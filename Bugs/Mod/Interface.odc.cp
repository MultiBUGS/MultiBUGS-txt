(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsInterface;


	

	IMPORT
		Dialog, Files, Kernel, Services, Strings, BugsCPCompiler, 
		BugsData, BugsEvaluate, BugsGraph, BugsIndex,
		BugsMappers, BugsMsg, BugsNames, BugsNodes, BugsParser, BugsRandnum,
		BugsVariables, GraphDeviance,
		GraphNodes, GraphStochastic, MathRandnum,
		MonitorMonitors,
		UpdaterActions, UpdaterMethods, UpdaterUpdaters;

	TYPE
		DistributeHook* = POINTER TO ABSTRACT RECORD
			numChains-, workersPerChain-: INTEGER;
			writeTime*, linkTime*, fileSize*, masterMemory*: INTEGER;
			rank*, setupTime*, memory*: POINTER TO ARRAY OF INTEGER;
			modules*: POINTER TO ARRAY OF Files.Name
		END;

		Command* = ARRAY 5 OF INTEGER;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		hook-: DistributeHook;

	PROCEDURE (h: DistributeHook) Clear-, NEW, ABSTRACT;

	PROCEDURE (h: DistributeHook) Deviance- (chain: INTEGER): REAL, NEW, ABSTRACT;

	PROCEDURE (h: DistributeHook) Distribute-, NEW, ABSTRACT;

	PROCEDURE (h: DistributeHook) RecvMCMCState-, NEW, ABSTRACT;

	PROCEDURE (h: DistributeHook) MonitorChanged-, NEW, ABSTRACT;

	PROCEDURE (h: DistributeHook) SendCommand- (IN command: Command), NEW, ABSTRACT;

	PROCEDURE (h: DistributeHook) Update- (thin, iteration: INTEGER; overRelax: BOOLEAN;
	VAR endOfAdating: INTEGER), NEW, ABSTRACT;

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

	PROCEDURE Clear*;
	BEGIN
		IF hook # NIL THEN
			hook.Clear;
			hook := NIL
		END;
		BugsMsg.Clear;
		DeleteFiles;
		GraphStochastic.Clear;
		BugsParser.Clear;
		BugsVariables.Clear;
		UpdaterActions.Clear;
		MonitorMonitors.Clear;
		GraphNodes.SetFactory(NIL);
		BugsRandnum.Clear;
		BugsCPCompiler.Clear;
	END Clear;

	PROCEDURE Distribute* (workersPerChain, numChains: INTEGER);
	BEGIN
		ASSERT(hook # NIL, 20);
		IF workersPerChain = 0 THEN
			hook := NIL;
			RETURN
		END;
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
		LOOP
			MathRandnum.SetGenerator(BugsRandnum.generators[chain]);
			UpdaterActions.LoadSamples(chain);
			UpdaterActions.GenerateInits(chain, fixFounder, res, updater);
			UpdaterActions.StoreSamples(chain);
			IF res = {} THEN
				BugsNodes.Checks(ok);
				EXIT
			ELSE
				UpdaterActions.LoadSamples(chain);
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
					EXIT
				END
			END
		END;
		MathRandnum.SetGenerator(BugsRandnum.generators[0])
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

	PROCEDURE RecvMCMCState*;
	BEGIN
		IF hook # NIL THEN
			hook.RecvMCMCState
		END
	END RecvMCMCState;

	PROCEDURE HasInits (numChains: INTEGER): BOOLEAN;
		VAR
			hasInits: BOOLEAN;
			chain: INTEGER;
	BEGIN
		chain := 0;
		hasInits := TRUE;
		WHILE (chain < numChains) & hasInits DO
			hasInits := UpdaterActions.IsInitialized(chain);
			INC(chain)
		END;
		RETURN hasInits
	END HasInits;

	PROCEDURE LoadInits* (VAR s: BugsMappers.Scanner; chain, numChains: INTEGER;
	fixFounder: BOOLEAN; OUT ok: BOOLEAN);
		VAR
			pos: INTEGER;
			hasInits: BOOLEAN;
			protocol: ARRAY 120 OF CHAR;
			fact: BugsData.Factory;
			loader: BugsData.Loader;
	BEGIN
		BugsData.SetFactory(NIL);
		UpdaterActions.LoadSamples(chain);
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
		UpdaterActions.StoreSamples(chain);
		IF UpdaterActions.IsInitialized(chain) THEN
			(*	generate inits for hidden variables	*)
			GenerateInitsForChain(chain, fixFounder, ok);
			IF ~ok THEN; RETURN END;
			hasInits := HasInits(numChains);
			IF hasInits THEN
				BugsNodes.Checks(ok);
				UpdaterActions.StoreSamples(chain);
				IF ~ok THEN RETURN END;
				UpdaterActions.SetInitialized
			END
		END
	END LoadInits;

	PROCEDURE MarkMonitored*;
	BEGIN
		GraphStochastic.ClearMarks(GraphStochastic.stochastics, {GraphStochastic.mark});
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

	PROCEDURE SendCommand* (IN command: Command);
	BEGIN
		IF hook # NIL THEN
			hook.SendCommand(command)
		END
	END SendCommand;

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

	PROCEDURE LoadDeviance* (chain: INTEGER);
		VAR
			node: GraphNodes.Node;
			name: BugsNames.Name;
			value: REAL;
	BEGIN
		IF hook # NIL THEN
			name := BugsIndex.Find("deviance");
			IF name # NIL THEN
				node := name.components[0];
				value := hook.Deviance(chain);
				GraphDeviance.SetValue(node, value)
			END
		END
	END LoadDeviance;

	PROCEDURE SetDistributeHook* (h: DistributeHook);
	BEGIN
		hook := h
	END SetDistributeHook;

	PROCEDURE UpdateModel* (numChains, thin: INTEGER; overRelax: BOOLEAN; OUT ok: BOOLEAN);
		VAR
			res: SET;
			chain, endOfAdapting, i, iteration, j, len: INTEGER;
			error, name, name1, msg, install: Dialog.String;
			updater: UpdaterUpdaters.Updater;
			p: GraphNodes.Node;
	BEGIN
		ok := TRUE;
		IF hook # NIL THEN
			iteration := UpdaterActions.iteration;
			endOfAdapting := UpdaterActions.endOfAdapting;
			hook.Update(thin, iteration, overRelax, endOfAdapting);
			UpdaterActions.SetAdaption(UpdaterActions.iteration + 1, endOfAdapting);
		ELSE
			chain := 0;
			WHILE (chain < numChains) & ok DO
				MathRandnum.SetGenerator(BugsRandnum.generators[chain]);
				UpdaterActions.LoadSamples(chain);
				j := 0;
				WHILE (j < thin) & ok DO
					UpdaterActions.Sample(overRelax, chain, res, updater);
					ok := res = {};
					INC(j)
				END;
				IF ok THEN
					UpdaterActions.StoreSamples(chain);
					INC(chain)
				ELSE
					UpdaterActions.LoadSamples(chain);
					p := updater.Prior(0);
					updater.Install(name);
					BugsMsg.Lookup(name, name1);
					BugsIndex.FindGraphNode(p, name);
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
					BugsMsg.StoreError(msg);
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
		END
	END UpdateModel;

	PROCEDURE UpdateMonitors* (numChains: INTEGER);
		VAR
			chain: INTEGER;
	BEGIN
		chain := 0;
		WHILE chain < numChains DO
			UpdaterActions.LoadSamples(chain);
			IF MonitorMonitors.devianceMonitored THEN
				LoadDeviance(chain)
			END;
			MonitorMonitors.UpdateMonitors(chain);
			INC(chain)
		END
	END UpdateMonitors;

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
				p.SetProps(p.props - {GraphStochastic.update});
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
				p.SetProps(p.props + {GraphStochastic.update});
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
					oldUpdater.LoadSample();
					newUpdater.StoreSample();
				END
			END
		END
	END ReplaceSampler;

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

	PROCEDURE LoadGenerator* (chain: INTEGER);
	BEGIN
		MathRandnum.SetGenerator(BugsRandnum.generators[chain]);
	END LoadGenerator;

	PROCEDURE AllocatedMemory* (): INTEGER;
	BEGIN
		Services.Collect;
		RETURN Kernel.Allocated()
	END AllocatedMemory;

	PROCEDURE SetNumChainsMonitors* (numChains: INTEGER);
	BEGIN
		MonitorMonitors.SetNumChainsMonitors(numChains)
	END SetNumChainsMonitors;

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


