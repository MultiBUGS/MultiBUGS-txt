(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsMaster;

	

	IMPORT 
		MPI, SYSTEM, Dialog, Files, Kernel, Meta, Services, Stores, Strings, StdLog,
		BugsCPCompiler, BugsComponents, BugsIndex, BugsInterface, BugsMsg, BugsNames, 
		DevCommanders, DevLinker, 
		DevianceInterface,
		GraphNodes, GraphStochastic, 
		HostDialog, HostFiles,
		MonitorMonitors,
		TextModels, 
		UpdaterActions, UpdaterParallel, UpdaterUpdaters;

	TYPE
		Hook = POINTER TO RECORD(BugsInterface.DistributeHook)
			monitorChanged: BOOLEAN;
			numMonitored: INTEGER
		END;

	VAR
		mpiInit: BOOLEAN;
		devianceValues: POINTER TO ARRAY OF REAL;
		exeFileName, timeStamp: ARRAY 64 OF CHAR;
		values: POINTER TO ARRAY OF REAL;
		monitored, leader: POINTER TO ARRAY OF INTEGER;
		monitoredA, portA, valuesA: INTEGER;
		port: ARRAY MPI.MAX_PORT_NAME OF SHORTCHAR;
		intercomm: MPI.Comm;
		restartLoc-: Files.Locator;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	CONST
		fileStemName = "Bugs";
		update = 0;
		terminate = 1;
		getMonitors = 2;
		getSamples = 3;
		
	PROCEDURE DeleteFiles;
		VAR
			loc: Files.Locator;
			fileInfo: Files.FileInfo;
			pos: INTEGER;
	BEGIN
		(*	see if exe file left over from previous model	*)
		IF exeFileName # "" THEN
			loc := Files.dir.This("");
			Files.dir.Delete(loc, exeFileName$);
			exeFileName := ""
		END;
		(*	delete any appropiate .bug files	*)
		loc := restartLoc;
		fileInfo := Files.dir.FileList(loc);
		WHILE fileInfo # NIL DO
			Strings.Find(fileInfo.name, "_" + timeStamp, 0, pos);
			IF pos #  - 1 THEN
				Files.dir.Delete(restartLoc, fileInfo.name)
			END;
			fileInfo := fileInfo.next
		END;
		(*	delete the appropiate BugsWorker.exe file	*)
		loc := Files.dir.This("");
		fileInfo := Files.dir.FileList(loc);
		pos :=  - 1;
		WHILE (fileInfo # NIL) & (pos =  - 1) DO
			Strings.Find(fileInfo.name, "_" + timeStamp, 0, pos);
			IF pos #  - 1 THEN
				exeFileName := fileInfo.name$;
				Files.dir.Delete(loc, exeFileName$);
				IF loc.res = 0 THEN
					exeFileName := ""
				END
			END;
			fileInfo := fileInfo.next
		END
	END DeleteFiles;

	PROCEDURE InternalizeMutable (h: BugsInterface.DistributeHook);
		VAR
			i, chain, numChains, numRanks, numUpdaters, numWorker, rank, worker: INTEGER;
			fileName: ARRAY 128 OF CHAR;
			updaters: POINTER TO ARRAY OF UpdaterUpdaters.Vector;
			id: POINTER TO ARRAY OF INTEGER;
			u: UpdaterUpdaters.Updater;
			rd: Stores.Reader;
			f: Files.File;
	BEGIN
		numChains := h.numChains;
		numWorker := h.numWorker;
		numRanks := numWorker DIV numChains;
		chain := 0;
		WHILE chain < numChains DO
			rank := 0;
			UpdaterParallel.DistributeUpdaters(numRanks, chain, updaters, id);
			WHILE rank < numRanks DO
				worker := chain * numRanks; (*how to invert RankToChain?*)
				Strings.IntToString(worker, fileName);
				fileName := fileStemName + fileName + "_" + timeStamp + ".bug";
				f := Files.dir.Old(restartLoc, fileName$, Files.shared);
				ASSERT(f # NIL, 33);
				rd.ConnectTo(f);
				numUpdaters := LEN(updaters[0]);
				i := 0;
				WHILE i < numUpdaters DO
					u := updaters[rank, i];
					UpdaterUpdaters.Internalize(u, rd);
					INC(i)
				END;
				rd.ConnectTo(NIL);
				INC(rank)
			END;
			INC(chain)
		END;
		f.Close;
	END InternalizeMutable;

	PROCEDURE LinuxModules (VAR modules: ARRAY OF Files.Name);
		VAR
			i, len: INTEGER;
	BEGIN
		i := 0;
		len := LEN(modules);
		WHILE i < len DO
			IF (modules[i] = "Kernel+") OR (modules[i] = "HostFiles") THEN
				modules[i] := "Lin" + modules[i]
			END;
			INC(i)
		END
	END LinuxModules;

	PROCEDURE WriteString (VAR wr: TextModels.Writer; IN string: ARRAY OF CHAR);
		VAR
			i, len: INTEGER;
	BEGIN
		i := 0;
		len := LEN(string);
		WHILE (i < len) & (string[i] # 0X) DO wr.WriteChar(string[i]); INC(i) END
	END WriteString;

	PROCEDURE LinkModules (exeFile, mpiImplementation: ARRAY OF CHAR): POINTER TO ARRAY OF Files.Name;
		VAR
			wr: TextModels.Writer;
			text: TextModels.Model;
			i, len: INTEGER;
			modules: POINTER TO ARRAY OF Files.Name;
	BEGIN
		BugsCPCompiler.CreateTimeStamp;
		modules := BugsComponents.Modules(mpiImplementation);
		text := TextModels.dir.New();
		wr := text.NewWriter(NIL);
		WriteString(wr, exeFile); WriteString(wr, " := "); wr.WriteChar(TextModels.line);
		i := 0; len := LEN(modules);
		WHILE i < len DO
			WriteString(wr, modules[i]); wr.WriteChar(" "); INC(i);
			IF i MOD 4 = 0 THEN wr.WriteChar(TextModels.line) END
		END;
		NEW(DevCommanders.par);
		DevCommanders.par.text := text;
		DevCommanders.par.beg := 0;
		DevCommanders.par.end := text.Length();
		DevLinker.LinkExe;
		DevCommanders.par := NIL;
		RETURN modules
	END LinkModules;

	PROCEDURE RecvMonitoredValues (numChains, numMonitored: INTEGER);
		VAR
			chain: INTEGER;
	BEGIN
		IF numMonitored > 0 THEN
			chain := 0;
			WHILE chain < numChains DO
				MPI.Recv(valuesA, numMonitored, MPI.DOUBLE, leader[chain],
				2, intercomm, MPI.STATUS_IGNORE);
				MonitorMonitors.RecvMonitored(values, monitored, numMonitored);
				UpdaterActions.StoreSamples(chain);
				INC(chain)
			END
		END;
		IF MonitorMonitors.devianceMonitored THEN
			chain := 0;
			WHILE chain < numChains DO
				MPI.Recv(SYSTEM.ADR(devianceValues[chain]), 1, MPI.DOUBLE, leader[chain],
				2, intercomm, MPI.STATUS_IGNORE);
				INC(chain)
			END
		END;
	END RecvMonitoredValues;

	PROCEDURE (h: Hook) SendCommand (IN ints: BugsInterface.Command);
		VAR
			numWorker, worker: INTEGER;
	BEGIN
		numWorker := h.numWorker;
		worker := 0;
		WHILE worker < numWorker DO
			MPI.Send(SYSTEM.ADR(ints[0]), LEN(ints), MPI.INT, worker, 1, intercomm);
			INC(worker)
		END
	END SendCommand;

	PROCEDURE TerminateWorkers;
		VAR
			command: BugsInterface.Command;
	BEGIN
		command[0] := terminate;
		command[1] :=  - 1;
		command[2] :=  - 1;
		command[3] :=  - 1;
		command[4] := -1;
		BugsInterface.SendCommand(command)
	END TerminateWorkers;

	PROCEDURE (h: Hook) Deviance (chain: INTEGER): REAL;
	BEGIN
		RETURN devianceValues[chain]
	END Deviance;

	PROCEDURE (h: Hook) MonitorChanged;
	BEGIN
		h.monitorChanged := TRUE
	END MonitorChanged;

	PROCEDURE (h: Hook) Clear;
	BEGIN
		values := NIL;
		leader := NIL;
		monitored := NIL;
		valuesA := 0;
		monitoredA := 0;
		TerminateWorkers;
		DeleteFiles
	END Clear;

	PROCEDURE (h: Hook) Distribute (mpiImplementation: ARRAY OF CHAR);
		VAR
			ok: BOOLEAN;
			numStochastics, chain, numChains, numWorker, size, res, worker, len0, len1: INTEGER;
			endTime, startTime: LONGINT;
			cmd, path, string, executable, bugFile: ARRAY 1024 OF CHAR;
			loc: Files.Locator;
			fileList: Files.FileInfo;
			f: Files.File;
			resultParams: ARRAY 3 OF INTEGER;
			deviance: GraphNodes.Node;
			name: BugsNames.Name;
	BEGIN
		numStochastics := GraphStochastic.numStochastics;
		NEW(values, numStochastics);
		NEW(monitored, numStochastics);
		numChains := h.numChains;
		numWorker := h.numWorker;
		NEW(devianceValues, numChains);
		NEW(h.rank, numWorker);
		NEW(h.setupTime, numWorker);
		NEW(h.memory, numWorker);
		valuesA := SYSTEM.ADR(values[0]);
		monitoredA := SYSTEM.ADR(monitored[0]);
		NEW(leader, numChains);
		Strings.IntToString(GraphNodes.timeStamp, timeStamp);
		(*executable := fileStemName + "Worker_" + timeStamp;*)
		executable := fileStemName + "Worker" ;
		bugFile := fileStemName + "_" + timeStamp;
		f := Files.dir.New(restartLoc, Files.dontAsk);
		startTime := Services.Ticks();
		BugsComponents.WriteModel(f, numChains, port, ok);
		IF ~ok THEN
			BugsInterface.SetDistributeHook(NIL);
			BugsMsg.Store("unable to write graph file for worker");
			RETURN
		END;
		len0 := StdLog.text.Length();
		StdLog.Ln;
		StdLog.String("port name:");
		StdLog.Ln;
		StdLog.String(LONG(port)); 
		StdLog.Ln;
		(*	find deviance and mark it as distributed if it exists	*)
		name := BugsIndex.Find("deviance");
		deviance := name.components[0];
		IF deviance # NIL THEN
			deviance.SetProps(deviance.props + {GraphStochastic.distributed});
		END;
		h.fileSize := f.Length();
		f.Register(bugFile$, "bug", Files.dontAsk, res);
		endTime := Services.Ticks();
		h.writeTime := SHORT(endTime - startTime);
		ASSERT(res = 0, 77);
		startTime := endTime;
		h.modules := LinkModules(executable, mpiImplementation);
		endTime := Services.Ticks();
		h.linkTime := SHORT(endTime - startTime);
		Strings.IntToString(numWorker, string);
		cmd := "mpiexec -n " + string; 
		loc := Files.dir.This("");
		fileList := Files.dir.FileList(loc);
		WHILE (fileList # NIL) & (fileList.name #  executable + ".exe") DO
			fileList := fileList.next 
		END;
		IF fileList = NIL THEN
			BugsInterface.SetDistributeHook(NIL);
			BugsMsg.Store("unable to link BugsWorker executable");
			RETURN
		END;
		path := loc(HostFiles.Locator).path$;
		(*	need quotes round command line for case of space in file path	*)
		executable := executable + '"';
		path := '"' + path;
		cmd := cmd + " " + path + "\" + executable;
		(*HostDialog.hideExtRunWindow := TRUE; *)
		StdLog.Ln;
		StdLog.String("mpi command:");
		StdLog.Ln;
		StdLog.String(cmd);
		StdLog.Ln;
		len1 := StdLog.text.Length();
		IF ~BugsCPCompiler.debug THEN StdLog.text.Delete(len0, len1) END;
		Dialog.RunExternal(cmd);
		MPI.Comm_accept(portA, MPI.INFO_NULL, 0, MPI.COMM_WORLD, intercomm);
		MPI.Comm_remote_size(intercomm, size);
		ASSERT(size = numWorker, 66);
		worker := 0;
		chain := 0;
		WHILE worker < numWorker DO
			MPI.Recv(SYSTEM.ADR(resultParams[0]), 3, MPI.INT, worker, 0,
			intercomm, MPI.STATUS_IGNORE);
			IF resultParams[2] = 0 THEN
				leader[chain] := worker;
				INC(chain)
			END;
			h.setupTime[worker] := resultParams[0];
			h.memory[worker] := resultParams[1];
			h.rank[worker] := resultParams[2];
			INC(worker)
		END;
		Services.Collect;
		h.masterMemory := Kernel.Allocated();
		h.monitorChanged := TRUE;
		IF DevianceInterface.state = DevianceInterface.set THEN
			DevianceInterface.Clear;
			DevianceInterface.Set
		END
	END Distribute;

	PROCEDURE (h: Hook) RecvMCMCState;
		VAR
			chain, len, numChains: INTEGER;
			command: BugsInterface.Command;
			buffer: ARRAY 4 OF REAL;
	BEGIN
		command[0] := getSamples;
		command[1] := 0;
		command[2] := 0;
		command[3] := 0;
		command[4] := 0;
		numChains := h.numChains;
		BugsInterface.SendCommand(command);
		len := GraphStochastic.numStochastics;
		chain := 0;
		WHILE chain < numChains DO
			MPI.Recv(valuesA, len, MPI.DOUBLE, leader[chain], 2, intercomm, MPI.STATUS_IGNORE);
			GraphStochastic.ReadSample(values);
			UpdaterActions.StoreSamples(chain);
			MPI.Recv(SYSTEM.ADR(devianceValues[chain]), 1, MPI.DOUBLE, leader[chain],
			2, intercomm, MPI.STATUS_IGNORE);
			IF DevianceInterface.state = DevianceInterface.setDistributed THEN
				MPI.Recv(SYSTEM.ADR(buffer), 4, MPI.DOUBLE, leader[chain],
				2, intercomm, MPI.STATUS_IGNORE);
				DevianceInterface.StoreStatistics(buffer[0], buffer[1], buffer[2], buffer[3], chain)
			END;
			INC(chain)
		END
	END RecvMCMCState;

	PROCEDURE (h: Hook) Update (thin, iteration: INTEGER; overRelax: BOOLEAN; VAR endOfAdapting: INTEGER);
		VAR
			command: BugsInterface.Command;
			end, i, numChains, numWorker, worker: INTEGER;
	BEGIN
		numWorker := h.numWorker;
		numChains := h.numChains;
		IF h.monitorChanged THEN
			h.monitorChanged := FALSE;
			(*	recalculate which stochastic nodes are monitored	*)
			BugsInterface.MarkMonitored;
			MonitorMonitors.MonitoredNodes(monitored, h.numMonitored);
			command[0] := getMonitors;
			command[1] := h.numMonitored;
			IF MonitorMonitors.devianceMonitored THEN
				command[2] := 1
			ELSE
				command[2] := 0
			END;
			command[3] := 0;
			command[4] := 0;
			BugsInterface.SendCommand(command);
			(*	send changed monitors to lead workers	*)
			IF h.numMonitored > 0 THEN
				i := 0;
				WHILE i < numChains DO
					MPI.Send(monitoredA, h.numMonitored, MPI.INT, leader[i], 1, intercomm);
					INC(i)
				END
			END
		END;
		(*	now do update	*)
		command[0] := update;
		command[1] := thin;
		command[2] := iteration;
		command[3] := endOfAdapting;
		IF overRelax THEN command[4] := 1 ELSE command[4] := 0 END;
		BugsInterface.SendCommand(command);
		worker := 0;
		endOfAdapting := 0;
		WHILE worker < numWorker DO
			MPI.Recv(SYSTEM.ADR(end), 1, MPI.INT, worker, 0, intercomm, MPI.STATUS_IGNORE);
			endOfAdapting := MAX(endOfAdapting, end);
			INC(worker)
		END;
		RecvMonitoredValues(h.numChains, h.numMonitored)
	END Update;
	
	PROCEDURE Install* (mpiImplementation: ARRAY OF CHAR);
		VAR
			h: Hook;
			nargs: INTEGER;
			args: POINTER TO ARRAY[untagged] OF SHORTCHAR;
			mod: Meta.Item;
	BEGIN
		IF ~mpiInit THEN
			mpiInit := TRUE;
			(*	load this mpi implementation for use by master	*)
			Meta.Lookup(mpiImplementation, mod);
			MPI.Init(nargs, args);
			portA := SYSTEM.ADR(port[0]);
			MPI.Open_port(MPI.INFO_NULL, portA)
		END;
		NEW(h);
		h.monitorChanged := FALSE;
		h.numMonitored := 0;
		BugsInterface.SetDistributeHook(h)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		values := NIL;
		restartLoc := Files.dir.This("Restart");
		mpiInit := FALSE;
		exeFileName := ""
	END Init;

	PROCEDURE Finalize;
		VAR
			loc: Files.Locator;
	BEGIN
		BugsInterface.Clear;
		IF mpiInit THEN
			MPI.Close_port(portA);
			MPI.Finalize
		END;
		(*	see if exe file left over from previous model	*)
		IF exeFileName # "" THEN
			loc := Files.dir.This("");
			Files.dir.Delete(loc, exeFileName$);
			exeFileName := ""
		END
	END Finalize;

BEGIN
	Init
CLOSE
	Finalize
END BugsMaster.
