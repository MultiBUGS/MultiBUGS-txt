(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)

(*	MPIimp loads an implementation of MPI and sets MPI.hook to a non NIL value;	*)

MODULE BugsMaster;

	

	IMPORT
		Dialog, Files, Kernel, MPImaster, Services, Strings,
		HostFiles,
		StdLog,
		TextModels,
		BugsCPCompiler, BugsComponents, BugsGraph, BugsIndex,
		BugsInterface, BugsMsg, BugsNames, BugsParallel,
		DevCommanders,
		DevianceInterface,
		GraphNodes, GraphStochastic,
		MonitorMonitors,
		UpdaterActions;

	TYPE
		Hook = POINTER TO RECORD(BugsInterface.DistributeHook)
			monitorChanged: BOOLEAN
		END;

	VAR
		globalValues, devianceValues: POINTER TO ARRAY OF REAL;
		col, row, leader: POINTER TO ARRAY OF INTEGER;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	CONST
		fileStemName = "Bugs";
		update = 0;
		terminate = 1;
		sendMonitorInfo = 2;
		requestMCMCState = 3;

	PROCEDURE WriteString (VAR wr: TextModels.Writer; IN string: ARRAY OF CHAR);
		VAR
			i, len: INTEGER;
	BEGIN
		i := 0;
		len := LEN(string);
		WHILE (i < len) & (string[i] # 0X) DO wr.WriteChar(string[i]); INC(i) END
	END WriteString;

	PROCEDURE LinkModules (exeFile: ARRAY OF CHAR; modules: POINTER TO ARRAY OF Files.Name);
		VAR
			wr: TextModels.Writer;
			text: TextModels.Model;
			i, len, res: INTEGER;
	BEGIN
		text := TextModels.dir.New();
		wr := text.NewWriter(NIL);
		IF Dialog.platform = Dialog.linux THEN WriteString(wr, "Linux ") END;
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
		IF Dialog.platform # Dialog.linux THEN
			Dialog.Call("DevLinker.LinkExe", "", res)
		ELSE
			Dialog.Call("Dev2Linker1.LinkElfExe", "", res)
		END;
		DevCommanders.par := NIL;
	END LinkModules;

	PROCEDURE RecvMonitoredValues (numChains: INTEGER);
		VAR
			chain: INTEGER;
	BEGIN
		IF col # NIL THEN
			chain := 0;
			WHILE chain < numChains DO
				MPImaster.RecvReals(globalValues, leader[chain]);
				BugsParallel.RecvMonitored(globalValues, col, row);
				UpdaterActions.StoreSamples(chain);
				INC(chain)
			END
		END;
		IF MonitorMonitors.devianceMonitored THEN
			chain := 0;
			WHILE chain < numChains DO
				devianceValues[chain] := MPImaster.RecvReal(leader[chain]);
				INC(chain)
			END
		END;
	END RecvMonitoredValues;

	PROCEDURE (h: Hook) SendCommand (IN ints: BugsInterface.Command);
		VAR
			numChains, numWorkers, workersPerChain, worker: INTEGER;
	BEGIN
		workersPerChain := h.workersPerChain;
		numChains := h.numChains;
		numWorkers := workersPerChain * numChains;
		worker := 0;
		WHILE worker < numWorkers DO
			MPImaster.SendIntegers(ints, worker);
			INC(worker)
		END
	END SendCommand;

	PROCEDURE (h: Hook) Deviance (chain: INTEGER): REAL;
	BEGIN
		RETURN devianceValues[chain]
	END Deviance;

	PROCEDURE (h: Hook) MonitorChanged;
	BEGIN
		h.monitorChanged := TRUE
	END MonitorChanged;

	PROCEDURE (h: Hook) Clear;
		VAR
			command: BugsInterface.Command;
	BEGIN
		globalValues := NIL;
		leader := NIL;
		col := NIL;
		row := NIL;
		command[0] := terminate;
		command[1] := - 1;
		command[2] := - 1;
		command[3] := - 1;
		command[4] := - 1;
		BugsInterface.SendCommand(command);
		MPImaster.CommDisconnect;
	END Clear;

	PROCEDURE (h: Hook) Distribute;
		VAR
			ok: BOOLEAN;
			len0, len1, numStochastics, chain, numChains, numWorkers, workersPerChain,
			size, res, rank: INTEGER;
			endTime, startTime: LONGINT;
			worker, path, string, executable, bugFile: ARRAY 1024 OF CHAR;
			loc: Files.Locator;
			fileList: Files.FileInfo;
			f: Files.File;
			resultParams: ARRAY 3 OF INTEGER;
			deviance: GraphNodes.Node;
			name: BugsNames.Name;
	BEGIN
		loc := Files.dir.This("");
		numChains := h.numChains;
		workersPerChain := h.workersPerChain;
		numWorkers := workersPerChain * numChains;
		NEW(devianceValues, numChains);
		NEW(h.rank, numWorkers);
		NEW(h.setupTime, numWorkers);
		NEW(h.memory, numWorkers);
		NEW(leader, numChains);
		executable := fileStemName + "Worker";
		bugFile := fileStemName;
		f := Files.dir.New(loc, Files.dontAsk);
		startTime := Services.Ticks();
		BugsComponents.WriteModel(f, workersPerChain, numChains, ok);
		IF ~ok THEN
			BugsInterface.SetDistributeHook(NIL);
			BugsMsg.StoreError("BugsMaster.NoGraphFile");
			RETURN
		END;
		h.fileSize := f.Length();
		f.Register(bugFile$, "bug", Files.dontAsk, res);
		ASSERT(res = 0, 77);
		numStochastics := BugsParallel.NumStochastics();
		NEW(globalValues, numStochastics);
		(*	find deviance and mark it as distributed if it exists	*)
		name := BugsIndex.Find("deviance");
		IF name # NIL THEN
			deviance := name.components[0];
		ELSE
			deviance := NIL
		END;
		IF deviance # NIL THEN
			deviance.SetProps(deviance.props + {GraphStochastic.distributed});
		END;
		endTime := Services.Ticks();
		h.writeTime := SHORT(endTime - startTime);
		startTime := endTime;
		h.modules := BugsComponents.Modules();
		len0 := StdLog.text.Length();
		(*	link the worker program	*)
		LinkModules(executable, h.modules); 
		len1 := StdLog.text.Length();
		StdLog.text.Delete(len0, len1);
		endTime := Services.Ticks();
		h.linkTime := SHORT(endTime - startTime);
		Strings.IntToString(numWorkers, string);
		fileList := Files.dir.FileList(loc);
		IF Dialog.platform # Dialog.linux THEN
			executable := executable + ".exe"
		END;
		(*	check that executable does not get deleted by anit virus software	*)
		WHILE (fileList # NIL) & (fileList.name # executable) DO
			fileList := fileList.next
		END;
		IF fileList = NIL THEN
			BugsInterface.SetDistributeHook(NIL);
			BugsMsg.StoreError("BugsMaster.LinkingFailure");
			RETURN
		END;
		path := loc(HostFiles.Locator).path$;
		worker := path + "\" + executable;
		(*	distribute the worker program	*)
		MPImaster.Spawn(SHORT(worker), numWorkers);
		size := MPImaster.CommRemoteSize();
		ASSERT(size = numWorkers, 66);
		rank := 0;
		chain := 0;
		WHILE rank < numWorkers DO
			MPImaster.RecvIntegers(resultParams, rank);
			IF resultParams[2] = 0 THEN
				leader[chain] := rank;
				INC(chain)
			END;
			h.setupTime[rank] := resultParams[0];
			h.memory[rank] := resultParams[1];
			h.rank[rank] := resultParams[2];
			INC(rank)
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
			chain, numChains: INTEGER;
			command: BugsInterface.Command;
			buffer: ARRAY 4 OF REAL;
	BEGIN
		command[0] := requestMCMCState;
		command[1] := 0;
		command[2] := 0;
		command[3] := 0;
		command[4] := 0;
		numChains := h.numChains;
		BugsInterface.SendCommand(command);
		chain := 0;
		WHILE chain < numChains DO
			MPImaster.RecvReals(globalValues, leader[chain]);
			BugsParallel.RecvSample(globalValues);
			UpdaterActions.StoreSamples(chain);
			IF BugsGraph.devianceExists THEN
				devianceValues[chain] := MPImaster.RecvReal(leader[chain])
			END;
			IF DevianceInterface.state = DevianceInterface.setDistributed THEN
				MPImaster.RecvReals(buffer, leader[chain]);
				DevianceInterface.StoreStatistics(buffer[0], buffer[1], buffer[2], buffer[3], chain)
			END;
			INC(chain)
		END
	END RecvMCMCState;

	PROCEDURE (h: Hook) Update (thin, iteration: INTEGER; overRelax: BOOLEAN;
	VAR endOfAdapting: INTEGER);
		VAR
			command: BugsInterface.Command;
			end, i, numChains, numWorkers, workersPerChain, worker: INTEGER;
	BEGIN
		workersPerChain := h.workersPerChain;
		numChains := h.numChains;
		numWorkers := workersPerChain * numChains;
		IF h.monitorChanged THEN
			h.monitorChanged := FALSE;
			(*	recalculate which stochastic nodes are monitored	*)
			BugsInterface.MarkMonitored;
			BugsParallel.MonitoredNodes(col, row);
			command[0] := sendMonitorInfo;
			IF col # NIL THEN command[1] := LEN(col) ELSE command[1] := 0 END;
			IF MonitorMonitors.devianceMonitored THEN
				command[2] := 1
			ELSE
				command[2] := 0
			END;
			command[3] := 0;
			command[4] := 0;
			BugsInterface.SendCommand(command);
			(*	send changed monitors to lead workers	*)
			IF col # NIL THEN
				i := 0;
				WHILE i < numChains DO
					MPImaster.SendIntegers(col, leader[i]);
					MPImaster.SendIntegers(row, leader[i]);
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
		WHILE worker < numWorkers DO
			end := MPImaster.RecvInteger(worker);
			endOfAdapting := MAX(endOfAdapting, end);
			INC(worker)
		END;
		RecvMonitoredValues(numChains)
	END Update;

	PROCEDURE Install*;
		VAR
			h: Hook;
	BEGIN
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
		globalValues := NIL
	END Init;

BEGIN
	Init
END BugsMaster.
