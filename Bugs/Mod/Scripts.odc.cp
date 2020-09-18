(*	

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



  *)

(*

This is the BUGS scripting language

The script language manipulates the GUI's dialog box and can be used in
conjunction with the GUI

The commands in the script language can take integer valued parameters denoted by i and string
value parameters denoted by s. If there are more than one parameter of one type the posfix 0, 1 etc
is added in a left to right order

*)


MODULE BugsScripts;

	

	TYPE
		ScriptCommand = POINTER TO RECORD;
			bugsCommand, pascalCommand: POINTER TO ARRAY OF CHAR;
			next: ScriptCommand
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		scriptCommands: ScriptCommand;
		loaded: BOOLEAN;

	PROCEDURE FindKey* (IN bugsCommand: ARRAY OF CHAR; OUT pascalCommand: ARRAY OF CHAR);
			VAR
			cursor: ScriptCommand;
	BEGIN
		cursor := scriptCommands;
		WHILE (cursor # NIL) & (cursor.bugsCommand^ # bugsCommand) DO
			cursor := cursor.next
		END;
		IF cursor # NIL THEN
			pascalCommand := cursor.pascalCommand$
		ELSE
			pascalCommand := ""
		END
	END FindKey;
	
	PROCEDURE StoreKey (IN bugsCommand, pascalCommand: ARRAY OF CHAR);
		VAR
			len: INTEGER;
			command: ScriptCommand;
	BEGIN
		NEW(command);
		command.next := scriptCommands;
		len := LEN(bugsCommand$);
		NEW(command.bugsCommand, len + 1);
		command.bugsCommand^ := bugsCommand$;
		len := LEN(pascalCommand$);
		NEW(command.pascalCommand, len + 1);
		command.pascalCommand^ := pascalCommand$;
		command.next := scriptCommands;
		scriptCommands := command
	END StoreKey;

	PROCEDURE Load*;
	BEGIN

		IF loaded THEN RETURN END;
		loaded := TRUE; 	(*__________________________________________________________________________________________________________

		Model menu
		*)

		(*	sets default directory actually prepends file names with a directory path	*)
		StoreKey("modelSetWD(s)",
		"BugsFiles.SetWorkingDir('^0')");

		(*	check model in file s	*)
		StoreKey("modelCheck(s)",
		"BugsCmds.Parse('^0')");

		(*	load data in files s	*)
		StoreKey("modelData(s)",
		"BugsCmds.LoadDataGuard; BugsCmds.LoadData('^0')");

		(*	compile model using 1 chain	*)
		StoreKey("modelCompile()",
		"BugsCmds.CompileGuard; MathTT800.Install; BugsCmds.specificationDialog.numChains := 1; BugsCmds.Compile");

		(*	compile model using i chains	*)
		StoreKey("modelCompile(i)",
		"BugsCmds.CompileGuard; MathTT800.Install; BugsCmds.specificationDialog.numChains  := ^0; BugsCmds.Compile");

		(*	load initial values for current chain from file s	*)
		StoreKey("modelInits(s)",
		"BugsCmds.LoadInitsGuard; BugsCmds.LoadInits('^0')");

		(*	load initial values for chaini  from file s	*)
		StoreKey("modelInits(si)",
		"BugsCmds.LoadInitsGuard; BugsCmds.specificationDialog.chain := ^1; BugsCmds.LoadInits('^0')");

		(*	generate initial values	*)
		StoreKey("modelGenInits()",
		"BugsCmds.GenerateInits");

		(*	generate initial values	*)
		StoreKey("modelGenInits(s)",
		"BugsCmds.specificationDialog.fixFounder = ^0; BugsCmds.GenerateInits");

		(*	distribute model using i cores	*)
		StoreKey("modelDistribute(i)",
		"BugsCmds.DistributeGuard; BugsCmds.specificationDialog.workersPerChain  := ^0; BugsMaster.Install; BugsCmds.Distribute");

		(*	update model i iterations	*)
		StoreKey("modelUpdate(i)",
		"BugsCmds.UpdateGuard; BugsCmds.updateDialog.updates  := ^0; BugsCmds.Update");

		(*	update model i0 iteration with prospective thin of i1	*)
		StoreKey("modelUpdate(ii)",
		"BugsCmds.UpdateGuard ;BugsCmds.updateDialog.updates  := ^0; BugsCmds.updateDialog.thin := ^1; BugsCmds.Update");

		(*	update model i0 iteration with prospective thin of i1 and screen refresh rate of i2 *)
		(* refresh rate only used in Windows GUI *)
		StoreKey("modelUpdate(iii)",
		"BugsCmds.UpdateGuard ;BugsCmds.updateDialog.updates  := ^0; BugsCmds.updateDialog.thin := ^1; BugsCmds.updateDialog.refresh := ^2;  BugsCmds.Update");

		(*	update model i0 iteration with prospective thin of i1 and screen refresh rate of i2 *)
		(* refresh rate only used in Windows GUI *)
		StoreKey("modelUpdate(iiis)",
		"BugsCmds.UpdateGuard ;BugsCmds.updateDialog.updates  := ^0;BugsCmds.updateDialog.thin := ^1; BugsCmds.updateDialog.refresh := ^2;  BugsCmds.updateDialog.overRelax := ^3; BugsCmds.Update");

		(*	update model i iteration with over-relaxation s	*)
		StoreKey("modelUpdate(is)",
		"BugsCmds.UpdateGuard; BugsCmds.updateDialog.updates  := ^0; BugsCmds.updateDialog.overRelax := ^1; BugsCmds.Update");

		(*	update model i0 iteration with prospective thin of i1 and over-relaxation s	*)
		StoreKey("modelUpdate(iis)",
		"BugsCmds.UpdateGuard; BugsCmds.updateDialog.updates  := ^0; BugsCmds.updateDialog.thin := ^1; BugsCmds.updateDialog.overRelax := ^2; BugsCmds.Update");

		(*	update model i iterations	*)
		StoreKey("modelUpdateNI(iis)",
		"BugsCmds.UpdateGuard; BugsCmds.updateDialog.updates  := ^0;BugsCmds.updateDialog.thin := ^1; BugsCmds.updateDialog.overRelax := ^2; BugsCmds.UpdateNI");

		(*	externalizes the model to file s .bug	*)
		StoreKey("modelExternalize(s)",
		"BugsCmds.UpdateGuard; BugsCmds.ExternalizeModel('^0')");

		(*	internalizes the model from file s .bug	*)
		StoreKey("modelInternalize(s)",
		"BugsCmds.InternalizeModel('^0')");

		(*	sets the the random number generator to predefined state i	*)
		StoreKey("modelSetRN(i)",
		"BugsCmds.SetRNGuard; BugsCmds.SetRNState(^0)");

		(*	sets where output is sent to s, disabled on non-Windows OS	*)
		StoreKey("modelDisplay(s)",
		"BugsCmds.SetDisplay('^0')");

		(*	sets the precision to which real numbers are output	*)
		StoreKey("modelPrecision(i)",
		"BugsCmds.displayDialog.precision :=  ^0; BugsFiles.SetPrec(^0)");

		(*	disables factory s creating updaters	*)
		StoreKey("modelDisable(s)",
		"UpdaterMethods.SetFactory('^0'); BugsCmds.FactoryGuard; UpdaterMethods.Disable");

		(*	enables factory s to create updaters	*)
		StoreKey("modelEnable(s)",
		"UpdaterMethods.SetFactory('^0'); BugsCmds.FactoryGuard; UpdaterMethods.Enable");

		(*	sets the adaptive phase of updaters created by factory s to i	*)
		StoreKey("modelSetAP(si)",
		"UpdaterMethods.SetFactory('^0'); BugsCmds.AdaptivePhaseGuard; UpdaterMethods.SetAdaptivePhase(^1)");

		(*	sets the number of iterations allowed in updaters created by factory s to i	*)
		StoreKey("modelSetIts(si)",
		"UpdaterMethods.SetFactory('^0'); BugsCmds.IterationsGuard; UpdaterMethods.SetIterations(^1)");

		(*	sets the amount of over-relaxation used by updaters created by factory s to i	*)
		StoreKey("modelSetOR(si)",
		"UpdaterMethods.SetFactory('^0'); BugsCmds.OverRelaxationGuard; UpdaterMethods.SetOverRelaxation(^1)");

		(*	sets updater parameters	*)
		StoreKey("modelSetParam(siii)",
		"UpdaterMethods.SetFactory('^0'); UpdaterSettings.SetAdaptivePhase(^1); UpdaterSettings.SetIterations(^2); UpdaterSettings.SetOverRelaxation(^3)");

		(*	tries to change sampling method for node	*)
		StoreKey("modelChangeSampler(ss)",
		"UpdaterMethods.SetFactory('^1'); BugsCmds.FactoryGuard; BugsCmds.ChangeSampler('^0')");

		(*___________________________________________________________________________________________

		File Menu

		*)

		(*	causes the BUGS program to close (with modal confirmation dialog in Windows)	*)
		StoreKey("modelQuit()",
		"BugsCmds.Quit('no')");

		(*	causes the BUGS program to close if s = "yes" or "y" (without modal confirmation dialog	in Windows)	*)
		StoreKey("modelQuit(s)",
		"BugsCmds.Quit('^0')");

		(*	saves a log of output to file s	*)
		StoreKey("modelSaveLog(s)",
		"BugsCmds.SaveLog('^0' )");

		(*___________________________________________________________________________________________

		Inference Menu
		*)

		(*	sets the begin iteration for sample monitors to i	*)
		StoreKey("samplesBeg(i)",
		"SamplesCmds.dialog.beg :=  ^0");

		(*	sets the end iteration for sample monitors to i	*)
		StoreKey("samplesEnd(i)",
		"SamplesCmds.dialog.end := ^0");

		(*	sets the retrospective thin for sample monitors to i	*)
		StoreKey("samplesThin(i)",
		"SamplesCmds.dialog.thin :=  ^0");

		(*	sets the first chain for sample monitors to i	*)
		StoreKey("samplesFirstChain(i)",
		"SamplesCmds.dialog.firstChain :=  ^0");

		(*	sets the last chain for sample monitors to i	*)
		StoreKey("samplesLastChain(i)",
		"SamplesCmds.dialog.lastChain := ^0");

		(*	sets a sample monitor for variable s	*)
		StoreKey("samplesSet(s)",
		"SamplesCmds.SetVariable('^0'); SamplesCmds.SetGuard; SamplesCmds.Set");

		(*	clears the sample monitor for variable s	*)
		StoreKey("samplesClear(s)",
		"SamplesCmds.SetVariable('^0'); SamplesCmds.HistoryGuard; SamplesCmds.ClearNI");

		(*	displays summary statistics for sample monitor s	*)
		StoreKey("samplesStats(s)",
		"SamplesCmds.SetVariable('^0'); SamplesCmds.StatsGuard; SamplesCmds.Stats");

		(*	displays CODA output for sample monitor s, disabled on non-Windows OS	*)
		StoreKey("samplesCoda(s)",
		"SamplesCmds.SetVariable('^0'); SamplesCmds.HistoryGuard; SamplesCmds.CODA");

		(*	writes CODA output for sample monitor s0 to files with stem name s1	*)
		StoreKey("samplesCoda(ss)",
		"SamplesCmds.SetVariable('^0'); SamplesCmds.HistoryGuard; SamplesCmds.CODAFiles('^1')");

		(*	displays density estimates for sample monitor s, disabled on non-Windows OS	*)
		StoreKey("samplesDensity(s)",
		"SamplesCmds.SetVariable('^0' ); SamplesCmds.StatsGuard; SamplesDensity.Install; SamplesCmds.Plot");

		(*	displays auto correlation for sample monitor s , disabled on non-Windows OS	*)
		StoreKey("samplesAutoC(s)",
		"SamplesCmds.SetVariable('^0'); SamplesCmds. StatsGuard; SamplesCorrelat.Install; SamplesCmds.Plot");

		(*	displays dynamic traces for sample monitor s, disabled on non-Windows OS	*)
		StoreKey("samplesTrace(s)",
		"SamplesCmds.SetVariable('^0'); SamplesCmds.HistoryGuard; SamplesTrace.Install; SamplesCmds.Plot");

		(*	displays history plots for sample monitor s, disabled on non-Windows OS	*)
		StoreKey("samplesHistory(s)",
		"SamplesCmds.SetVariable('^0'); SamplesCmds.HistoryGuard; SamplesHistory.Install; SamplesCmds.Plot");

		(*	displays cumulative quantile plots for sample monitor s , disabled on non-Windows OS	*)
		StoreKey("samplesQuantiles(s)",
		"SamplesCmds.SetVariable('^0'); SamplesCmds.StatsGuard; SamplesQuantiles.Install; SamplesCmds.Plot");

		(*	displays bgr convergence diagnostic plots for sample monitor s, disabled on non-Windows OS	*)
		StoreKey("samplesBgr(s)",
		"SamplesCmds.SetVariable('^0'); SamplesCmds.BGRGuard; SamplesDiagnostics.Install; SamplesCmds.Plot");

		(*	alters options in Samples dialog	*)
		StoreKey("samplesOptionIncl(i)", "SamplesCmds.OptionsIncl(^0)");
		StoreKey("samplesOptionExcl(i)", "SamplesCmds.OptionsExcl(^0)");
		
		(*	sets summary monitor for variable s	*)
		StoreKey("summarySet(s)",
		"SummaryCmds.SetVariable('^0'); SummaryCmds.SetGuard; SummaryCmds.Set");

		(*	displays summary statistics for summary monitor s	*)
		StoreKey("summaryStats(s)",
		"SummaryCmds.SetVariable('^0' ); SummaryCmds.StatsGuard; SummaryCmds.Stats");

		(*	displays mean for summary monitor s	*)
		StoreKey("summaryMean(s)",
		"SummaryCmds.SetVariable('^0'); SummaryCmds.StatsGuard; SummaryCmds.Means");

		(*	clears summary monitor s	*)
		StoreKey("summaryClear(s)",
		"SummaryCmds.SetVariable('^0'); SummaryCmds.StatsGuard; SummaryCmds.ClearNI");

		(*	sets ranks monitor for variable s	*)
		StoreKey("ranksSet(s)",
		"RanksCmds.SetVariable('^0'); RanksCmds.SetGuard; RanksCmds.Set");

		(*	displays rank information for ranks monitor for variable s	*)
		StoreKey("ranksStats(s)",
		"RanksCmds.SetVariable('^0' ); RanksCmds.StatsGuard; RanksCmds.Stats");

		(*	clears ranks monitor s	*)
		StoreKey("ranksClear(s)",
		"RanksCmds.SetVariable('^0'); RanksCmds.StatsGuard; RanksCmds.Clear");

		(*	displays histograms for ranks monitor for variable s *)
		StoreKey("ranksHistogram(s)",
		"RanksCmds.SetVariable('^0'); RanksDialog.StatsGuard; RanksCmds.Draw");

		(*	sets model monitor for variable s	*)
		StoreKey("modelsSet(s)",
		"ModelsCmds.SetVariable('^0'); ModelsCmds.SetGuard; ModelsCmds.Set");

		(*	displays component probs for  model monitor for variable s	*)
		StoreKey("modelsComp(s)",
		"ModelsCmds.SetVariable('^0'); ModelsCmds.StatsGuard; ModelsCmds.ComponentProbs");

		(*	displays model probs for  model monitor for variable s	*)
		StoreKey("modelsProbs(s)",
		"ModelsCmds.SetVariable('^0'); ModelsCmds.StatsGuard; ModelsCmds.ModelProbs");

		(*	clears model monitor for variable s	*)
		StoreKey("modelsClear(s)",
		"ModelsCmds.SetVariable('^0'); ModelsCmds.StatsGuard; ModelsCmds.Clear");

		(*	sets monitor for IC with stochastic parents should only use plugin on non distributed models 	*)
		StoreKey("icSet()",
		"DeviancePluginS.Install; DevianceCmds.SetGuard; DevianceCmds.Set");
		
		(*	dic versions are needed for backwards compatibility with MultiBUGS 1.0	*)
		StoreKey("dicSetS()",
		"DeviancePluginS.Install; DevianceCmds.SetGuard; DevianceCmds.Set");

		(*	displays IC statistics	*)
		StoreKey("icStats()",
		"DevianceCmds.StatsGuard; DevianceCmds.Stats");
		StoreKey("dicStats()",
		"DevianceCmds.StatsGuard; DevianceCmds.Stats");

		(*	clears monitor for IC	*)
		StoreKey("icClear()",
		"DevianceCmds.StatsGuard; DevianceCmds.Clear");
		StoreKey("dicClear()",
		"DevianceCmds.StatsGuard; DevianceCmds.Clear");
		(*___________________________________________________________________________________________

		Info Menu
		*)
		
		(*	display updaters that are not initialized	*)
		StoreKey("infoUnitializedUpdaters()", "BugsCmds.NotCompiledGuard; BugsCmds.WriteUninitNodes");

		(*	display updaters sorted by node name	*)
		StoreKey("infoUpdatersByName()", "BugsCmds.NotCompiledGuard; BugsCmds.UpdatersByName");

		(*	display updaters sorted by node depth in graph	*)
		StoreKey("infoUpdatersByDepth()", "BugsCmds.NotCompiledGuard; BugsCmds.UpdatersByDepth");

		(*	display model	*)
		StoreKey("infoModel()", "BugsCmds.ParsedGuard; BugsCmds.PrintModel");

		(*	display latex	*)
		StoreKey("infoLatex()", "BugsCmds.ParsedGuard; BugsCmds.PrintLatex");

		(*	display data	*)
		StoreKey("infoData()", "BugsCmds.UpdateGuard; BugsCmds.WriteData");

		(*	display state	*)
		StoreKey("infoState()", "BugsCmds.UpdateGuard; BugsCmds.WriteChains");

		(*	writes out current state of sampler to files with stem name s	*)
		StoreKey("infoSaveState(s)", "BugsCmds.UpdateGuard; BugsCmds.WriteChainsToFile('^0')");
		
		(*	distribution of samplers	*)
		StoreKey("infoDistSamp(^0)", "BugsCmds.specificationDialog.numProc := ^0; BugsCmds.ShowDistribution");
		
		(*	distribution of deviance	*)
		StoreKey("infoDistDeviance(^0)", "BugsCmds.specificationDialog.numProc := ^0; BugsCmds.ShowDistributionDeviance");
		
		(*	display node values	*)
		StoreKey("infoNodeValue(s)", "BugsCmds.SetNode('^0'); BugsCmds.Values");

		(*	display node methods	*)
		StoreKey("infoNodeMethods(s)", "BugsCmds.SetNode('^0'); BugsInfodebug.Methods");

		(*	display node types	*)
		StoreKey("infoNodeTypes(s)", "BugsCmds.SetNode('^0'); BugsInfodebug.Types");

		(*	model metrics	*)
		StoreKey("infoMetrics()", "BugsCmds.Metrics");
		
		(*	displays the modules of the BUGS software currently loaded	*)
		StoreKey("infoModules()", "BugsCmds.Modules");

		(*	displays the amount of memory used by software	*)
		StoreKey("infoMemory()", "BugsCmds.AllocatedMemory");

		(*	info on distribution	*)
		StoreKey("infoDistribution()", "BugsCmds.DistributeInfo");
		
		(*	info on version	*)
		StoreKey("infoVersion()", "BugsCmds.Version")
		
	END Load;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		loaded := FALSE;
		scriptCommands := NIL
	END Init;

BEGIN
	Init
END BugsScripts.


