(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsMaster;

	

	IMPORT
		MPI, SYSTEM, Console, Files, Kernel, Meta, Services, Stores, Strings, 
		BugsCPCompiler,
		BugsComponents, BugsIndex, BugsInterface, BugsMsg, BugsNames, 
		DevCommanders, DevLinker, 
		DevianceInterface, 
		GraphNodes, GraphStochastic, 
		HostFiles,
		MonitorMonitors,
		TextModels,
		UpdaterActions, UpdaterParallel, UpdaterUpdaters;

	TYPE
		Hook = POINTER TO RECORD(BugsInterface.DistributeHook)
			monitorChanged: BOOLEAN;
		END;

		Command = ARRAY 4 OF INTEGER;

	VAR
		mpiInit: BOOLEAN;
		devianceValues, waic, pW: POINTER TO ARRAY OF REAL;
		exeFileName, timeStamp: ARRAY 64 OF CHAR;
		values: POINTER TO ARRAY OF REAL;
		monitored, leader: POINTER TO ARRAY OF INTEGER;
		monitoredA, portA, valuesA: INTEGER;
		port: ARRAY MPI.MAX_PORT_NAME OF SHORTCHAR;
		intercomm: MPI.Comm;
		process: Console.Process;
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
				worker := chain * numRanks; (*how to invert RankToChain*)
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

	PROCEDURE IsLinux (): BOOLEAN;
		VAR
			i: Meta.Item;
			mod, type: Meta.Name;
	BEGIN
		Meta.GetItem(Console.cons, i);
		i.GetTypeName(mod, type);
		RETURN mod = "LinConsole"
	END IsLinux;

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
			isLinux: BOOLEAN;
			wr: TextModels.Writer;
			text: TextModels.Model;
			i, len: INTEGER;
			modules: POINTER TO ARRAY OF Files.Name;
	BEGIN
		BugsCPCompiler.CreateTimeStamp;
		isLinux := IsLinux();
		modules := BugsComponents.Modules(mpiImplementation);
		IF isLinux THEN
			LinuxModules(modules);
		END;
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

	PROCEDURE (h: Hook) SendCommand (IN ints: ARRAY OF INTEGER);
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
			command: Command;
	BEGIN
		command[0] := terminate;
		command[1] :=  - 1;
		command[2] :=  - 1;
		command[3] :=  - 1;
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
		IF process # NIL THEN
			Console.TerminateProcess(process);
			process := NIL
		END;
		DeleteFiles
	END Clear;

	PROCEDURE (h: Hook) Distribute (mpiImplementation: ARRAY OF CHAR);
		VAR
			ok: BOOLEAN;
			numStochastics, chain, numChains, numWorker, size, res, worker: INTEGER;
			endTime, startTime: LONGINT;
			cmd, path, string, executable, bugFile: ARRAY 1024 OF CHAR;
			loc: Files.Locator;
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
		NEW(waic, numChains);
		NEW(pW, numChains);
		NEW(h.rank, numWorker);
		NEW(h.setupTime, numWorker);
		NEW(h.memory, numWorker);
		valuesA := SYSTEM.ADR(values[0]);
		monitoredA := SYSTEM.ADR(monitored[0]);
		NEW(leader, numChains);
		IF process # NIL THEN
			Console.TerminateProcess(process);
			process := NIL
		END;
		Strings.IntToString(GraphNodes.timeStamp, timeStamp);
		executable := fileStemName + "Worker_" + timeStamp;
		bugFile := fileStemName + "_" + timeStamp;
		f := Files.dir.New(restartLoc, Files.dontAsk);
		startTime := Services.Ticks();
		BugsComponents.WriteModel(f, numChains, port, ok);
		IF ~ok THEN
			BugsInterface.SetDistributeHook(NIL);
			BugsMsg.StoreMsg("unable to write graph file for worker");
			RETURN
		END;
		(*	find deviance and mark it as distributed	*)
		name := BugsIndex.Find("deviance");
		deviance := name.components[0];
		deviance.SetProps(deviance.props + {GraphStochastic.distributed});
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
		path := loc(HostFiles.Locator).path$;
		(*	need quotes round command line for case of space in file path	*)
		executable := executable + '"';
		path := '"' + path;
		IF IsLinux() THEN
			(*	LinStub loads shared object library executable	*)
			cmd := cmd + " LinStub " + path + "\" + executable
		ELSE
			cmd := cmd + " " + path + "\" + executable
		END;
		process := Console.CreateProcess(cmd);
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
		(*	need to tell workers to calculate deviance and send value to master	*)
		Services.Collect;
		h.masterMemory := Kernel.Allocated();
		h.monitorChanged := TRUE
	END Distribute;

	PROCEDURE (h: Hook) RecvSamples;
		VAR
			chain, len, numChains: INTEGER;
			command: Command;
	BEGIN
		command[0] := getSamples;
		command[1] := 0;
		command[2] := 0;
		command[3] := 0;
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
			(*IF WAIC monitored THEN THIS stuff*)
			(*MPI.Recv(SYSTEM.ADR(waic[chain]), 1, MPI.DOUBLE, leader[chain],
			2, intercomm, MPI.STATUS_IGNORE);
			MPI.Recv(SYSTEM.ADR(pW[chain]), 1, MPI.DOUBLE, leader[chain],
			2, intercomm, MPI.STATUS_IGNORE);
			DevianceInterface.StoreWAIC(waic[chain], pW[chain], chain);*)
			
			INC(chain)
		END
	END RecvSamples;

	PROCEDURE (h: Hook) Update (thin, iteration: INTEGER; VAR endOfAdapting: INTEGER);
		VAR
			command: Command;
			end, i, numChains, numMonitored, numWorker, worker: INTEGER;
	BEGIN
		numWorker := h.numWorker;
		numChains := h.numChains;
		IF h.monitorChanged THEN
			h.monitorChanged := FALSE;
			(*	recalculate which stochastic nodes are monitored	*)
			BugsInterface.MarkMonitored;
			MonitorMonitors.MonitoredNodes(monitored, numMonitored);
			command[0] := getMonitors;
			command[1] := numMonitored;
			IF MonitorMonitors.devianceMonitored THEN
				command[2] := 1
			ELSE
				command[2] := 0
			END;
			command[3] := 0;
			BugsInterface.SendCommand(command);
			(*	send changed monitors to lead workers	*)
			IF numMonitored > 0 THEN
				i := 0;
				WHILE i < numChains DO
					MPI.Send(monitoredA, numMonitored, MPI.INT, leader[i], 1, intercomm);
					INC(i)
				END
			END
		END;
		(*	now do update	*)
		command[0] := update;
		command[1] := thin;
		command[2] := iteration;
		command[3] := endOfAdapting;
		BugsInterface.SendCommand(command);
		worker := 0;
		endOfAdapting := 0;
		WHILE worker < numWorker DO
			MPI.Recv(SYSTEM.ADR(end), 1, MPI.INT, worker, 0, intercomm, MPI.STATUS_IGNORE);
			endOfAdapting := MAX(endOfAdapting, end);
			INC(worker)
		END;
		RecvMonitoredValues(h.numChains, numMonitored)
	END Update;

	PROCEDURE Close*;
		VAR
			timeStamp: LONGINT;
	BEGIN
		timeStamp := GraphNodes.timeStamp;
		DeleteFiles;
		BugsInterface.Clear;
		GraphNodes.SetTimeStamp(timeStamp);
		DeleteFiles
	END Close;

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
		process := NIL;
		mpiInit := FALSE;
		exeFileName := ""
	END Init;

	PROCEDURE Finalize;
		VAR
			loc: Files.Locator;
	BEGIN
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
