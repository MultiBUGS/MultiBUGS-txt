(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsInterface;


	

	IMPORT
		Dialog, Files, Kernel, Services, Strings, Stores := Stores64,
		BugsCPCompiler, BugsData, BugsEvaluate, BugsGraph, BugsIndex,
		BugsMappers, BugsMsg, BugsNames, BugsNodes, BugsParser, BugsRandnum,
		BugsVariables,
		GraphDeviance, GraphKernel, GraphLogical, GraphNodes, GraphStochastic,
		MathRandnum,
		MonitorMonitors,
		UpdaterActions, UpdaterHMC, UpdaterMethods, UpdaterUpdaters;

	TYPE
		DistributeHook* = POINTER TO ABSTRACT RECORD
			numChains-, workersPerChain-: INTEGER;
			writeTime*, linkTime*, masterMemory*: INTEGER;
			fileSize*: LONGINT;
			rank*, setupTime*, memory*: POINTER TO ARRAY OF INTEGER;
			modules*: POINTER TO ARRAY OF Files.Name
		END;

		Command* = ARRAY 5 OF INTEGER;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		hook-: DistributeHook;
		useHMC-: BOOLEAN;
		numSteps-, warmUpPeriod-: INTEGER;
		stepSize-: REAL;

	CONST
		update* = 0;
		terminate* = 1;
		recvMonitorInfo* = 2;
		sendMCMCState* = 3;
		setWAIC* = 4;
		clearWAIC* = 5;
		checkPoint* = 6;
		restart* = 7;
		updateHMC* = 8;

	PROCEDURE (h: DistributeHook) Clear-, NEW, ABSTRACT;

	PROCEDURE (h: DistributeHook) Deviance- (chain: INTEGER): REAL, NEW, ABSTRACT;

	PROCEDURE (h: DistributeHook) Distribute-, NEW, ABSTRACT;

	PROCEDURE (h: DistributeHook) RecvMCMCState-, NEW, ABSTRACT;

	PROCEDURE (h: DistributeHook) RecvMutable- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (h: DistributeHook) MonitorChanged-, NEW, ABSTRACT;

	PROCEDURE (h: DistributeHook) SendCommand- (IN command: Command), NEW, ABSTRACT;

	PROCEDURE (h: DistributeHook) SendMutable- (VAR rd: Stores.Reader), NEW, ABSTRACT;

		PROCEDURE (h: DistributeHook) Update- (thin, iteration: INTEGER; overRelax: BOOLEAN;
	OUT res: SET; VAR updater, worker, endOfAdating: INTEGER), NEW, ABSTRACT;

	PROCEDURE DeleteFiles;
		VAR
			loc: Files.Locator;
			fileInfo: Files.FileInfo;
	BEGIN
		loc := Files.dir.This("Dynamic");
		loc := loc.This("Code");
		fileInfo := Files.dir.FileList(loc);
		WHILE fileInfo # NIL DO
			Files.dir.Delete(loc, fileInfo.name);
			fileInfo := fileInfo.next
		END;
	END DeleteFiles;

	PROCEDURE Error (errorNum: INTEGER; name: ARRAY OF CHAR);
		VAR
			errorMsg: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
			numToString: ARRAY 8 OF CHAR;
	BEGIN
		Strings.IntToString(errorNum, numToString);
		p[0] := name$;
		BugsMsg.LookupParam("BugsInterface" + numToString, p, errorMsg);
		BugsMsg.StoreError(errorMsg)
	END Error;

	PROCEDURE LoadValues (chain: INTEGER);
	BEGIN
		GraphStochastic.LoadValues(chain);
		GraphLogical.LoadValues(chain);
		GraphKernel.LoadValues(chain);
	END LoadValues;

	PROCEDURE OptimizeLinear;
		VAR
			i, num: INTEGER;
			logicals: GraphLogical.Vector;
			p: GraphLogical.Node;
			parents: GraphNodes.List;
			node: GraphNodes.Node;
			constDiffs: BOOLEAN;
		CONST
			all = TRUE;
	BEGIN
		logicals := GraphLogical.nodes;
		IF logicals # NIL THEN
			num := LEN(logicals);
			i := 0;
			WHILE i < num DO
				p := logicals[i];
				IF GraphLogical.linear IN p.props THEN
					EXCL(p.props, GraphLogical.linear);
					parents := p.Parents(all);
					constDiffs := TRUE;
					WHILE (parents # NIL) & constDiffs DO
						node := parents.node;
						WITH node: GraphLogical.Node DO
							constDiffs := GraphLogical.constDiffs IN node.props
						ELSE
						END;
						parents := parents.next
					END;
					IF constDiffs THEN INCL(p.props, GraphLogical.constDiffs) END
				END;
				INC(i)
			END
		END
	END OptimizeLinear;

	PROCEDURE ReplaceSampler (node: GraphStochastic.Node; fact: UpdaterUpdaters.Factory; OUT ok: BOOLEAN);
		VAR
			i, size: INTEGER;
			match: BOOLEAN;
			p: GraphStochastic.Node;
			newUpdater, oldUpdater: UpdaterUpdaters.Updater;
		CONST
			chain = 0;
	BEGIN
		ok := FALSE;
		IF ~(UpdaterUpdaters.enabled IN fact.props) THEN RETURN END;
		oldUpdater := UpdaterActions.FindSampler(chain, node);
		IF oldUpdater # NIL THEN
			size := oldUpdater.Size();
			i := 0;
			WHILE i < size DO
				p := oldUpdater.Prior(i);
				EXCL(p.props, GraphStochastic.update);
				INC(i)
			END;
			IF fact.CanUpdate(node) THEN
				newUpdater := fact.New(node);
			ELSE
				newUpdater := NIL
			END;
			i := 0;
			WHILE i < size DO
				p := oldUpdater.Prior(i);
				INCL(p.props, GraphStochastic.update);
				INC(i)
			END;
			IF (newUpdater # NIL) & (newUpdater.Size() = size) THEN
				i := 0;
				match := TRUE;
				WHILE	(i < size) & match DO
					match := oldUpdater.Prior(i) = newUpdater.Prior(i);
					INC(i)
				END;
				IF match THEN
					ok := TRUE;
					fact.SetProps(fact.props + {UpdaterUpdaters.active});
					UpdaterActions.ReplaceUpdater(newUpdater);
				END
			END
		END
	END ReplaceSampler;

	PROCEDURE StoreValues (chain: INTEGER);
	BEGIN
		GraphStochastic.StoreValues(chain);
		GraphLogical.StoreValues(chain);
		GraphKernel.StoreValues(chain)
	END StoreValues;

	PROCEDURE AllocatedMemory* (): INTEGER;
	BEGIN
		Services.Collect;
		RETURN Kernel.Allocated()
	END AllocatedMemory;

	PROCEDURE ChangeSampler* (IN string: ARRAY OF CHAR; factIndex: INTEGER; OUT ok: BOOLEAN);
		VAR
			var: BugsParser.Variable;
			offsets: POINTER TO ARRAY OF INTEGER;
			name: BugsNames.Name;
			i, size: INTEGER;
			fact: UpdaterUpdaters.Factory;
			node: GraphNodes.Node;
			ok1: BOOLEAN;
	BEGIN
		ok := FALSE;
		IF (factIndex # - 1) & (factIndex < LEN(UpdaterMethods.factories)) THEN
			fact := UpdaterMethods.factories[factIndex];
			var := BugsParser.StringToVariable(string);
			IF var # NIL THEN
				offsets := BugsEvaluate.Offsets(var);
				IF offsets # NIL THEN
					name := var.name;
					size := LEN(offsets);
					i := 0;
					ok := TRUE;
					WHILE i < size DO
						node := name.components[offsets[i]];
						IF node IS GraphStochastic.Node THEN
							ReplaceSampler(node(GraphStochastic.Node), fact, ok1);
							ok := ok & ok1
						END;
						INC(i)
					END;
					IF BugsGraph.IsAdapting(BugsRandnum.numberChains) THEN
						UpdaterActions.SetAdaption(UpdaterActions.iteration, MAX(INTEGER))
					END
				END
			END
		END
	END ChangeSampler;

	PROCEDURE CheckPoint* (VAR wr: Stores.Writer);
		VAR
			command: Command;
	BEGIN
		IF hook # NIL THEN
			command[0] := checkPoint;
			command[1] := - 1;
			command[2] := - 1;
			command[3] := - 1;
			command[4] := - 1;
			hook.SendCommand(command);
			hook.RecvMutable(wr)
		END
	END CheckPoint;

	PROCEDURE Clear*;
	BEGIN
		IF hook # NIL THEN hook.Clear; hook := NIL END;
		BugsMsg.Clear;
		DeleteFiles;
		GraphStochastic.Clear;
		GraphLogical.Clear;
		GraphKernel.Clear;
		BugsParser.Clear;
		BugsVariables.Clear;
		UpdaterActions.Clear;
		MonitorMonitors.Clear;
		GraphNodes.SetFactory(NIL);
		BugsRandnum.Clear;
		BugsCPCompiler.Clear;
		useHMC := FALSE
	END Clear;

	PROCEDURE Distribute* (workersPerChain, numChains: INTEGER);
	BEGIN
		ASSERT(hook # NIL, 20);
		IF workersPerChain = 0 THEN hook := NIL; RETURN END;
		hook.numChains := numChains;
		hook.workersPerChain := workersPerChain;
		hook.Distribute
	END Distribute;

	PROCEDURE GenerateInitsForChain* (chain: INTEGER; fixFounder: BOOLEAN; OUT ok: BOOLEAN);
		CONST
			maxTrials = 100;
		VAR
			res: SET;
			trials: INTEGER;
			updater: UpdaterUpdaters.Updater;
			p: GraphStochastic.Node;
			msg, name, name1: ARRAY 128 OF CHAR;
	BEGIN
		res := {};
		trials := 0;
		MathRandnum.SetGenerator(BugsRandnum.generators[chain]);
		LOOP
			BugsGraph.LoadInits(chain);
			LoadValues(chain);
			UpdaterActions.GenerateInits(chain, fixFounder, res, updater);
			IF res = {} THEN
				BugsGraph.StoreInits(chain);
				GraphLogical.EvaluateAllDiffs;
				OptimizeLinear;
				StoreValues(chain);
				BugsNodes.Checks(ok);
				EXIT
			ELSE	(*	try again	*)
				BugsGraph.LoadInits(chain);
				LoadValues(chain);
				INC(trials);
				IF trials > maxTrials THEN
					p := updater.Prior(0);
					BugsIndex.FindGraphNode(p, name);
					name[0] := " ";
					name[LEN(name$) - 1] := " ";
					msg := name + "with updater of type ";
					updater.Install(name);
					BugsMsg.Lookup(name, name1);
					msg := msg + name1;
					Error(1, msg);
					ok := FALSE;
					BugsGraph.LoadInits(chain);
					LoadValues(chain);
					EXIT
				END
			END
		END;
		MathRandnum.SetGenerator(BugsRandnum.generators[0]);
		BugsGraph.SetInitialized
	END GenerateInitsForChain;

	PROCEDURE GenerateInits* (numChains: INTEGER; fixFounders: BOOLEAN; OUT ok: BOOLEAN);
		VAR
			chain: INTEGER;
	BEGIN
		chain := 0;
		ok := TRUE;
		WHILE (chain < numChains) & ok DO
			GenerateInitsForChain(chain, fixFounders, ok);
			IF ~ok THEN RETURN END;
			INC(chain)
		END;
		UpdaterActions.SetInitialized
	END GenerateInits;

	PROCEDURE IsAdapting* (): BOOLEAN;
	BEGIN
		RETURN UpdaterActions.endOfAdapting > UpdaterActions.iteration
	END IsAdapting;

	PROCEDURE IsCompiled* (): BOOLEAN;
	BEGIN
		RETURN UpdaterActions.NumberUpdaters() # 0
	END IsCompiled;

	PROCEDURE IsDistributed* (): BOOLEAN;
	BEGIN
		RETURN hook # NIL
	END IsDistributed;

	PROCEDURE IsInitialized* (): BOOLEAN;
	BEGIN
		RETURN UpdaterActions.initialized
	END IsInitialized;

	PROCEDURE IsParsed* (): BOOLEAN;
	BEGIN
		RETURN BugsParser.model # NIL
	END IsParsed;

	PROCEDURE LoadData* (VAR s: BugsMappers.Scanner; OUT ok: BOOLEAN);
		VAR
			pos: INTEGER;
			protocol: ARRAY 120 OF CHAR;
			fact: BugsData.Factory;
			loader: BugsData.Loader;
	BEGIN
		ASSERT(IsParsed(), 21);
		BugsData.SetFactory(NIL);
		pos := s.Pos();
		s.Scan;
		IF s.type = BugsMappers.function THEN
			protocol := "BugsSplusData.Install"
		ELSE
			protocol := "BugsRectData.Install"
		END;
		fact := BugsData.InstallFactory(protocol);
		loader := fact.New();
		s.SetPos(pos);
		loader.Data(s, ok)
	END LoadData;

	PROCEDURE LoadDeviance* (chain: INTEGER);
		VAR
			node: GraphNodes.Node;
			name: BugsNames.Name;
			value: REAL;
	BEGIN
		name := BugsIndex.Find("deviance");
		IF name # NIL THEN
			node := name.components[0];
			IF hook # NIL THEN
				value := hook.Deviance(chain);
			ELSE
				value := GraphDeviance.DevianceValue(node);
				BugsGraph.devianceValues[chain] := value
			END;
			GraphDeviance.SetValue(node, value)
		END
	END LoadDeviance;

	PROCEDURE LoadGenerator* (chain: INTEGER);
	BEGIN
		MathRandnum.SetGenerator(BugsRandnum.generators[chain]);
	END LoadGenerator;

	PROCEDURE LoadInits* (VAR s: BugsMappers.Scanner; chain: INTEGER; OUT ok: BOOLEAN);
		VAR
			pos: INTEGER;
			protocol: ARRAY 120 OF CHAR;
			fact: BugsData.Factory;
			loader: BugsData.Loader;
	BEGIN
		BugsData.SetFactory(NIL);
		BugsGraph.LoadInits(chain);
		GraphStochastic.LoadValues(chain);
		pos := s.Pos();
		s.Scan;
		IF s.type = BugsMappers.function THEN
			protocol := "BugsSplusData.Install"
		ELSE
			protocol := "BugsRectData.Install"
		END;
		fact := BugsData.InstallFactory(protocol);
		loader := fact.New();
		s.SetPos(pos);
		loader.Inits(s, ok);
		IF ~ok THEN RETURN END;
		BugsGraph.StoreInits(chain);
		GraphStochastic.StoreValues(chain);
		IF BugsGraph.IsInitialized(chain) THEN
			GraphLogical.EvaluateAllDiffs;
			OptimizeLinear;
			StoreValues(chain);
			BugsNodes.Checks(ok);
			IF ~ok THEN RETURN END;
		END;
		BugsGraph.SetInitialized
	END LoadInits;

	PROCEDURE MarkMonitored*;
	BEGIN
		GraphStochastic.ClearMarks(GraphStochastic.nodes, {GraphStochastic.mark});
		GraphLogical.ClearMarks(GraphLogical.nodes, {GraphStochastic.mark});
		MonitorMonitors.MarkMonitored
	END MarkMonitored;

	PROCEDURE MonitorChanged*;
	BEGIN
		IF hook # NIL THEN
			hook.MonitorChanged
		END
	END MonitorChanged;

	PROCEDURE NumberChains* (): INTEGER;
	BEGIN
		RETURN UpdaterActions.NumberChains()
	END NumberChains;

	PROCEDURE ParseModel* (VAR s: BugsMappers.Scanner; OUT ok: BOOLEAN);
		VAR
			pos: INTEGER;
	BEGIN
		pos := s.Pos();
		REPEAT
			s.Scan
		UNTIL ((s.type = BugsMappers.char) & (s.char = "{")) OR s.eot;
		BugsVariables.StoreLoops(s, ok);
		IF ~ok THEN RETURN END;
		s.SetPos(pos);
		REPEAT
			s.Scan
		UNTIL ((s.type = BugsMappers.char) & (s.char = "{")) OR s.eot;
		BugsVariables.StoreNames(s, ok);
		IF ~ok THEN RETURN END;
		s.SetPos(pos);
		REPEAT
			s.Scan
		UNTIL ((s.type = BugsMappers.char) & (s.char = "{")) OR s.eot;
		BugsParser.ParseModel(s);
		BugsParser.MarkVariables;
		ok := ~BugsParser.error;
		IF ~ok THEN BugsParser.Clear END
	END ParseModel;

	PROCEDURE RecvMCMCState*;
	BEGIN
		IF hook # NIL THEN hook.RecvMCMCState END
	END RecvMCMCState;

	PROCEDURE Restart* (VAR rd: Stores.Reader);
		VAR
			command: Command;
	BEGIN
		IF hook # NIL THEN
			command[0] := restart;
			command[1] := - 1;
			command[2] := - 1;
			command[3] := - 1;
			command[4] := - 1;
			hook.SendCommand(command);
			hook.SendMutable(rd)
		END
	END Restart;

	PROCEDURE SendCommand* (IN command: Command);
	BEGIN
		IF hook # NIL THEN hook.SendCommand(command) END
	END SendCommand;

	PROCEDURE SetDistributeHook* (h: DistributeHook);
	BEGIN
		hook := h
	END SetDistributeHook;

	PROCEDURE SetNumChainsMonitors* (numChains: INTEGER);
	BEGIN
		MonitorMonitors.SetNumChainsMonitors(numChains)
	END SetNumChainsMonitors;

	PROCEDURE UpdateError* (res: SET; updater: UpdaterUpdaters.Updater);
		VAR
			i, pos: INTEGER;
			error, name, msg, install: Dialog.String;
			p: GraphNodes.Node;
	BEGIN
		p := updater.Prior(0);
		BugsIndex.FindGraphNode(p, name);
		IF name[0] = "<" THEN name[0] := " " END;
		Strings.Find(name, ">", 0, pos); IF pos # - 1 THEN name[pos] := " " END;
		updater.Install(install);
		BugsMsg.Lookup(install, install);
		msg := "update error for node " + name + " algorithm " + install + " error";
		i := 0;
		WHILE i < MAX(SET) DO
			IF i IN res THEN
				Strings.IntToString(i, error);
				error := "UpdaterError" + error;
				BugsMsg.Lookup(error, error);
				msg := msg + " " + error
			END;
			INC(i)
		END;
		BugsMsg.StoreError(msg)
	END UpdateError;

	PROCEDURE UpdateModel* (numChains, thin: INTEGER; overRelax: BOOLEAN; OUT ok: BOOLEAN);
		VAR
			res: SET;
			chain, endOfAdapting, iteration, j, updater, worker: INTEGER;
			u: UpdaterUpdaters.Updater;
	BEGIN
		ok := TRUE;
		IF hook = NIL THEN	(*	model is not distributed	*)
			chain := 0;
			WHILE (chain < numChains) & ok DO
				MathRandnum.SetGenerator(BugsRandnum.generators[chain]);
				LoadValues(chain);
				j := 0;
				WHILE (j < thin) & ok DO
					UpdaterActions.Sample(overRelax, chain, res, updater);
					ok := res = {};
					INC(j)
				END;
				IF ok THEN
					StoreValues(chain); INC(chain)
				ELSE
					u := UpdaterActions.updaters[0, updater]; UpdateError(res, u)
				END
			END;
			MathRandnum.SetGenerator(BugsRandnum.generators[0]);
			IF ok THEN
				IF (UpdaterActions.endOfAdapting > UpdaterActions.iteration) & 
					~BugsGraph.IsAdapting(numChains) THEN
					UpdaterActions.SetAdaption(UpdaterActions.iteration + 1, UpdaterActions.iteration + 1)
				ELSE
					UpdaterActions.SetAdaption(UpdaterActions.iteration + 1, UpdaterActions.endOfAdapting)
				END
			END
		ELSE (*	model is distributed	*)
			iteration := UpdaterActions.iteration;
			endOfAdapting := UpdaterActions.endOfAdapting;
			hook.Update(thin, iteration, overRelax, res, updater, worker, endOfAdapting);
			UpdaterActions.SetAdaption(UpdaterActions.iteration + 1, endOfAdapting);
			IF res # {} THEN
				ok := FALSE; u := UpdaterActions.updaters[0, updater]; UpdateError(res, u)
			END
		END
	END UpdateModel;

	PROCEDURE UpdateModelHMC* (numChains: INTEGER; OUT ok: BOOLEAN);
		VAR
			chain, iteration, updater: INTEGER;
			res: SET;
			u: UpdaterUpdaters.Updater;
	BEGIN
		chain := 0;
		res := {};
		WHILE (chain < numChains) & (res = {}) DO
			MathRandnum.SetGenerator(BugsRandnum.generators[chain]);
			LoadValues(chain);
			iteration := UpdaterActions.iteration + 1;
			UpdaterHMC.Update(numSteps, iteration, warmUpPeriod, chain, stepSize, res, updater);
			ok := res = {};
			IF ok THEN
				StoreValues(chain);
			ELSE
				LoadValues(chain);
				u := UpdaterActions.updaters[0, updater]; UpdateError(res, u)
			END;
			INC(chain)
		END;
		MathRandnum.SetGenerator(BugsRandnum.generators[0]);
		IF UpdaterActions.iteration = UpdaterHMC.startIt + warmUpPeriod THEN
			UpdaterActions.SetAdaption(UpdaterActions.iteration + 1, UpdaterActions.iteration + 1)
		ELSE
			UpdaterActions.SetAdaption(UpdaterActions.iteration + 1, UpdaterActions.endOfAdapting)
		END
	END UpdateModelHMC;

	PROCEDURE UpdateMonitors* (numChains: INTEGER);
		VAR
			chain: INTEGER;
	BEGIN
		chain := 0;
		WHILE chain < numChains DO
			(*GraphStochastic.LoadValues(chain);
			GraphLogical.LoadValues(chain);
			GraphKernel.LoadValues(chain);*)
			LoadValues(chain);
			IF MonitorMonitors.devianceMonitored THEN LoadDeviance(chain) END;
			MonitorMonitors.UpdateMonitors(chain);
			INC(chain)
		END
	END UpdateMonitors;

	PROCEDURE SetHMCParams* (num, warmUp: INTEGER; eps: REAL);
	BEGIN
		useHMC := TRUE;
		numSteps := num;
		warmUpPeriod := warmUp;
		stepSize := eps
	END SetHMCParams;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		Clear
	END Init;

BEGIN
	Init
END BugsInterface.


