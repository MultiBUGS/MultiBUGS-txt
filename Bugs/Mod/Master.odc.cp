(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)

MODULE BugsMaster;

	

	IMPORT
		MPI,
		SYSTEM, Files := Files64, Dialog, Kernel, MPImaster, Meta, Services, Stores := Stores64, Strings,
		HostFiles,
		StdLog,
		TextModels,
		BugsComponents, BugsGraph, BugsIndex, BugsInterface, BugsMsg, BugsNames, BugsParallel,
		DevCommanders,
		DevianceInterface,
		GraphLogical, GraphNodes, GraphStochastic,
		MonitorMonitors;

	TYPE
		Hook = POINTER TO RECORD(BugsInterface.DistributeHook)
			monitorChanged: BOOLEAN
		END;

	VAR
		globalValues, monitorValues, devianceValues: POINTER TO ARRAY OF REAL;
		col, row, leader: POINTER TO ARRAY OF INTEGER;
		logicals: GraphLogical.Vector;
		platform: Dialog.String;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		mpiImp: Dialog.String;

	CONST
		fileStemName = "Bugs";

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
			Dialog.Call("DevLinker.LinkExe", "WindowsLinker", res)
		ELSE
			Dialog.Call("Dev2Linker1.LinkElfExe", "LinuxLinker", res)
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
				MPImaster.RecvReals(monitorValues, leader[chain]);
				BugsParallel.RecvMonitored(monitorValues, col, row);
				GraphLogical.Evaluate(logicals);
				GraphStochastic.StoreValues(chain);
				GraphLogical.StoreValues(chain);
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

	PROCEDURE (h: Hook) Clear;
		VAR
			command: BugsInterface.Command;
	BEGIN
		globalValues := NIL;
		leader := NIL;
		col := NIL;
		row := NIL;
		monitorValues := NIL;
		logicals := NIL;
		command[0] := BugsInterface.terminate;
		command[1] := - 1;
		command[2] := - 1;
		command[3] := - 1;
		command[4] := - 1;
		BugsInterface.SendCommand(command);
		MPImaster.CommDisconnect;
	END Clear;

	PROCEDURE (h: Hook) Deviance (chain: INTEGER): REAL;
	BEGIN
		RETURN devianceValues[chain]
	END Deviance;

	PROCEDURE (h: Hook) Distribute;
		VAR
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
		BugsComponents.WriteModel(f, workersPerChain, numChains);
		h.fileSize := f.Length();
		f.Register(bugFile$, "bug", Files.dontAsk, res);
		ASSERT(res = 0, 77);
		numStochastics := BugsParallel.NumStochastics();
		NEW(globalValues, numStochastics);
		(*	find deviance and mark it as distributed if it exists	*)
		name := BugsIndex.Find("deviance");
		IF name # NIL THEN deviance := name.components[0]; ELSE deviance := NIL END;
		IF deviance # NIL THEN INCL(deviance.props, GraphStochastic.distributed) END;
		endTime := Services.Ticks();
		h.writeTime := SHORT(endTime - startTime);
		startTime := endTime;
		h.modules := BugsComponents.Modules(platform);
		len0 := StdLog.text.Length();
		(*	link the worker program	*)
		LinkModules(executable, h.modules);
		len1 := StdLog.text.Length();
		StdLog.text.Delete(len0, len1);
		endTime := Services.Ticks();
		h.linkTime := SHORT(endTime - startTime);
		Strings.IntToString(numWorkers, string); 
		fileList := Files.dir.FileList(loc);
		IF Dialog.platform # Dialog.linux THEN executable := executable + ".exe" END;
		(*	check that executable does not get deleted by anti virus software	*)
		WHILE (fileList # NIL) & (fileList.name # executable) DO fileList := fileList.next END;
		IF fileList = NIL THEN
			BugsInterface.SetDistributeHook(NIL);
			BugsMsg.StoreError("BugsMaster:LinkingFailure");
			RETURN
		END;
		path := loc(HostFiles.Locator).path$;
		IF Dialog.platform # Dialog.linux THEN
			worker := path + "\" + executable;
		ELSE
			worker := path + "/" + executable;
		END;
		(*	distribute the worker program	*)
		MPImaster.Spawn(SHORT(worker), numWorkers);
		size := MPImaster.CommRemoteSize();
		ASSERT(size = numWorkers, 66);
		rank := 0; chain := 0;
		WHILE rank < numWorkers DO
			MPImaster.RecvIntegers(resultParams, rank);
			IF resultParams[2] = 0 THEN leader[chain] := rank; INC(chain) END;
			h.setupTime[rank] := resultParams[0];
			h.memory[rank] := resultParams[1];
			h.rank[rank] := resultParams[2];
			INC(rank)
		END;
		Services.Collect;
		h.masterMemory := Kernel.Allocated();
		h.monitorChanged := TRUE;
		IF DevianceInterface.state = DevianceInterface.set THEN
			DevianceInterface.Clear; DevianceInterface.Set
		END
	END Distribute;

	PROCEDURE (h: Hook) MonitorChanged;
	BEGIN
		h.monitorChanged := TRUE
	END MonitorChanged;

	PROCEDURE (h: Hook) RecvMCMCState;
		VAR
			chain, numChains: INTEGER;
			command: BugsInterface.Command;
			buffer: ARRAY 4 OF REAL;
	BEGIN
		command[0] := BugsInterface.sendMCMCState;
		command[1] := 0; command[2] := 0; command[3] := 0; command[4] := 0;
		numChains := h.numChains;
		BugsInterface.SendCommand(command);
		chain := 0;
		WHILE chain < numChains DO
			MPImaster.RecvReals(globalValues, leader[chain]);
			BugsParallel.RecvSample(globalValues);
			GraphLogical.EvaluateAll;
			GraphStochastic.StoreValues(chain);
			GraphLogical.StoreValues(chain);
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

	PROCEDURE (h: Hook) RecvMutable (VAR wr: Stores.Writer);
		VAR
			i, j, numWorker: INTEGER;
			mutableSizes: POINTER TO ARRAY OF INTEGER;
			buffer: POINTER TO ARRAY OF BYTE;
	BEGIN
		numWorker := h.workersPerChain * h.numChains;
		NEW(mutableSizes, numWorker);
		i := 0;
		WHILE i < numWorker DO
			mutableSizes[i] := MPImaster.RecvInteger(i); wr.WriteInt(mutableSizes[i]); INC(i)
		END;
		i := 0;
		WHILE i < numWorker DO
			NEW(buffer, mutableSizes[i]);
			MPImaster.RecvBytes(buffer, i);
			j := 0; WHILE j < mutableSizes[i] DO wr.WriteByte(buffer[j]); INC(j) END;
			INC(i)
		END
	END RecvMutable;

	PROCEDURE (h: Hook) SendCommand (IN cmds: BugsInterface.Command);
		VAR
			numChains, numWorkers, workersPerChain, worker: INTEGER;
	BEGIN
		workersPerChain := h.workersPerChain; numChains := h.numChains;
		numWorkers := workersPerChain * numChains;
		worker := 0; WHILE worker < numWorkers DO MPImaster.SendIntegers(cmds, worker); INC(worker) END
	END SendCommand;

	PROCEDURE (h: Hook) SendMutable (VAR rd: Stores.Reader);
		VAR
			i, j, numWorker: INTEGER;
			mutableSizes: POINTER TO ARRAY OF INTEGER;
			buffer: POINTER TO ARRAY OF BYTE;
	BEGIN
		numWorker := h.workersPerChain * h.numChains;
		NEW(mutableSizes, numWorker);
		i := 0;
		WHILE i < numWorker DO
			rd.ReadInt(mutableSizes[i]); MPImaster.SendInteger(mutableSizes[i], i); INC(i)
		END;
		i := 0;
		WHILE i < numWorker DO
			NEW(buffer, mutableSizes[i]);
			j := 0; WHILE j < mutableSizes[i] DO rd.ReadByte(buffer[j]); INC(j) END;
			MPImaster.SendBytes(buffer, i);
			INC(i)
		END
	END SendMutable;

	PROCEDURE (h: Hook) Update (thin, iteration: INTEGER; overRelax: BOOLEAN;
	OUT res: SET; VAR updater, worker, endOfAdapting: INTEGER);
		VAR
			command: BugsInterface.Command;
			status: ARRAY 3 OF INTEGER;
			end, i, numChains, numWorkers, workersPerChain: INTEGER;
	BEGIN
		updater := - 1;
		workersPerChain := h.workersPerChain;
		numChains := h.numChains;
		numWorkers := workersPerChain * numChains;
		IF h.monitorChanged THEN
			h.monitorChanged := FALSE;
			(*	recalculate which stochastic nodes are required to be monitored	*)
			BugsInterface.MarkMonitored;
			BugsParallel.MonitoredNodes(col, row, logicals);
			IF col # NIL THEN NEW(monitorValues, LEN(col)) END;
			command[0] := BugsInterface.recvMonitorInfo;
			IF col # NIL THEN command[1] := LEN(col) ELSE command[1] := 0 END;
			IF MonitorMonitors.devianceMonitored THEN command[2] := 1 ELSE command[2] := 0 END;
			command[3] := 0; command[4] := 0;
			BugsInterface.SendCommand(command);
			(*	send changed monitors to lead workers	*)
			IF col # NIL THEN
				i := 0;
				WHILE i < numChains DO
					MPImaster.SendIntegers(col, leader[i]); MPImaster.SendIntegers(row, leader[i]); INC(i)
				END
			END
		END;
		(*	now do update	*)
		IF BugsInterface.useHMC THEN
			command[0] := BugsInterface.updateHMC; command[1] := BugsInterface.warmUpPeriod;
			command[2] := iteration; command[3] := BugsInterface.numSteps;
			command[4] := SYSTEM.VAL(INTEGER, SHORT(BugsInterface.stepSize))
		ELSE
			command[0] := BugsInterface.update; command[1] := thin;
			command[2] := iteration; command[3] := endOfAdapting;
			IF overRelax THEN command[4] := 1 ELSE command[4] := 0 END
		END;
		BugsInterface.SendCommand(command);
		endOfAdapting := 0;
		i := 0;
		WHILE i < numWorkers DO
			MPImaster.RecvIntegers(status, i);
			end := status[0];
			res := BITS(status[1]);
			IF (res # {}) & (updater = - 1) THEN
				updater := status[2]; worker := i; updater := BugsParallel.MapUpdater(worker, updater);
			END;
			endOfAdapting := MAX(endOfAdapting, end);
			INC(i)
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
		VAR
			item: Meta.Item;
	BEGIN
		Maintainer;
		globalValues := NIL;
		Meta.Lookup("MPIimp", item);
		platform := "Windows";
		Dialog.MapString("#Bugs:MPI" + platform, mpiImp);
		Meta.Lookup(mpiImp, item);
		IF MPI.hook = NIL THEN
			(* ABI Compatibility Initiative, Fedora 29, CentOS 7 *)
			platform := "ABICI";
			Dialog.MapString("#Bugs:MPI" + platform, mpiImp);
			Meta.Lookup(mpiImp, item)
		END;
		IF MPI.hook = NIL THEN
			(* CentOS 6, Debian 8/9/10, Ubuntu 16.04, Ubuntu 19.04 *)
			platform := "Debian";
			Dialog.MapString("#Bugs:MPI" + platform, mpiImp);
			Meta.Lookup(mpiImp, item)
		END;
		IF MPI.hook = NIL THEN
			(* Ubuntu 18.04 *)
			platform := "Ubuntu1804";
			Dialog.MapString("#Bugs:MPI" + platform, mpiImp);
			Meta.Lookup(mpiImp, item)
		END;
		IF MPI.hook = NIL THEN
			(* Ubuntu 14.04 *)
			platform := "Ubuntu1404";
			Dialog.MapString("#Bugs:MPI" + platform, mpiImp);
			Meta.Lookup(mpiImp, item)
		END;
		(*	handle eror	*)
		MPImaster.Install
	END Init;

BEGIN
	Init
END BugsMaster.
