(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsSerialize;


	

	IMPORT 
		Files := Files64, Services, Stores := Stores64, 
		BugsCPCompiler, BugsGraph, BugsIndex, BugsParser, BugsRandnum,
		DeviancePlugin,
		GraphNodes,
		MonitorMonitors,
		UpdaterActions, UpdaterMethods;

	VAR
		restartLoc-: Files.Locator;

		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE SetRestartLoc* (loc: Files.Locator);
	BEGIN
		restartLoc := loc;
	END SetRestartLoc;

	PROCEDURE GetRestartLoc* (): Files.Locator;
	BEGIN
		RETURN restartLoc;
	END GetRestartLoc;

	PROCEDURE ExternalizeModel* (VAR wr: Stores.Writer);
	BEGIN
		BugsIndex.ExternalizeNames(wr);
		BugsParser.Externalize(BugsParser.model, wr);
		BugsCPCompiler.Externalize(wr)
	END ExternalizeModel;

	PROCEDURE InternalizeModel* (VAR rd: Stores.Reader);
		VAR
			model: BugsParser.Statement;
	BEGIN
		BugsIndex.InternalizeNames(rd);
		model := BugsParser.Internalize(rd);
		BugsParser.SetModel(model);
		BugsCPCompiler.Internalize(rd)
	END InternalizeModel;

	(*	immutable	*)
	PROCEDURE ExternalizeGraph* (VAR wr: Stores.Writer);
		VAR
			num: INTEGER;
	BEGIN
		GraphNodes.BeginExternalize(wr);
		UpdaterActions.ExternalizeParamPointers(wr);
		num := BugsIndex.NumberNodes();
		wr.WriteInt(num);
		BugsIndex.ExternalizePointers(wr);
		UpdaterActions.ExternalizeParamData(wr);
		BugsIndex.ExternalizeData(wr);
		UpdaterActions.ExternalizeUpdaterPointers(wr);
		GraphNodes.EndExternalize(wr);
		wr.WriteInt(GraphNodes.maxDepth);
		wr.WriteInt(GraphNodes.maxStochDepth)
	END ExternalizeGraph;

	(*	immutable	*)
	PROCEDURE InternalizeGraph* (VAR rd: Stores.Reader);
		VAR
			maxDepth, maxStochDepth, num: INTEGER;
		CONST
			nChains = MAX(INTEGER);
	BEGIN
		GraphNodes.BeginInternalize(rd);
		rd.ReadInt(num); 
		GraphNodes.InternalizePointers(num, rd);
		rd.ReadInt(num);
		BugsIndex.InternalizePointers(rd); 
		GraphNodes.InternalizeNodeData(rd);
		UpdaterActions.InternalizeUpdaterPointers(nChains, rd);
		GraphNodes.EndInternalize(rd);
		BugsGraph.CreateDeviance;
		rd.ReadInt(maxDepth);
		rd.ReadInt(maxStochDepth); 
		GraphNodes.SetDepth(maxDepth, maxStochDepth)
	END InternalizeGraph;

	(*	mutable	*)
	PROCEDURE ExternalizeMutable* (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteInt(UpdaterActions.iteration);
		wr.WriteInt(UpdaterActions.endOfAdapting);
		BugsRandnum.ExternalizeRNGenerators(wr);
		UpdaterActions.ExternalizeUpdaterData(wr);
		UpdaterMethods.ExternalizeProperties(wr);
	END ExternalizeMutable;

	(*	mutable	*)
	PROCEDURE InternalizeMutable* (VAR rd: Stores.Reader);
		VAR
			chain, iteration, endOfAdapting, numChains: INTEGER;
			pos: LONGINT;
	BEGIN
		rd.ReadInt(iteration);
		rd.ReadInt(endOfAdapting);
		UpdaterActions.SetAdaption(iteration, endOfAdapting);
		BugsRandnum.InternalizeRNGenerators(rd);
		pos := rd.Pos();
		chain := 0;
		numChains := UpdaterActions.NumberChains(); 
		WHILE chain < numChains DO
			rd.SetPos(pos);
			UpdaterActions.InternalizeUpdaterData(chain, rd);
			INC(chain)
		END;
		UpdaterMethods.InternalizeProperties(rd);
	END InternalizeMutable;

	(*	mutable	*)
	PROCEDURE ExternalizeMonitors* (VAR wr: Stores.Writer);
	BEGIN
		DeviancePlugin.Externalize(wr);
		MonitorMonitors.ExternalizeMonitors(wr)
	END ExternalizeMonitors;

	(*	mutable	*)
	PROCEDURE InternalizeMonitors* (VAR rd: Stores.Reader);
	BEGIN
		DeviancePlugin.Internalize(rd);
		MonitorMonitors.InternalizeMonitors(rd)
	END InternalizeMonitors;

	PROCEDURE Externalize* (numWorker: INTEGER; VAR wr: Stores.Writer);
	BEGIN
		wr.WriteInt(numWorker);
		ExternalizeModel(wr);
		ExternalizeGraph(wr);
		ExternalizeMutable(wr);
		ExternalizeMonitors(wr)
	END Externalize;

	PROCEDURE Internalize* (OUT numWorker: INTEGER; VAR rd: Stores.Reader);
	BEGIN
		rd.ReadInt(numWorker);
		InternalizeModel(rd); 
		InternalizeGraph(rd);
		InternalizeMutable(rd);
		InternalizeMonitors(rd);
		UpdaterActions.StoreStochastics;
		Services.Collect
	END Internalize;

	PROCEDURE ExternalizeResults* (fileName: ARRAY OF CHAR);
		VAR
			f: Files.File;
			res: INTEGER;
			wr: Stores.Writer;
	BEGIN
		f := Files.dir.New(restartLoc, Files.dontAsk);
		wr.ConnectTo(f);
		wr.SetPos(0);
		ExternalizeMonitors(wr);
		f.Flush;
		wr.ConnectTo(NIL);
		Services.Collect;
		f.Register(fileName$, "bug", Files.dontAsk, res);
		ASSERT(res = 0, 99)
	END ExternalizeResults;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		restartLoc := Files.dir.This("Restart")
	END Init;

BEGIN
	Init
END BugsSerialize.

