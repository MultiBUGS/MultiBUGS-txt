(*	

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



  *)

(*

This is the BUGS scripting language

There are two vesions of the CP code in the scripting language

The first is used within the GUI interface, the second with Linux and ClassicBUGS

The GUI version of the script language manipulates the GUI's dialog box and can be used in
conjunction with the GUI

The commands in the script language can take integer valued parameters denoted by i and string
value parameters denoted by s. If there are more than one parameter of one type the posfix 0, 1 etc
is added in a left to right order

*)


MODULE BugsScripts;

	

	IMPORT
		BugsScripting;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		Command: PROCEDURE (IN com, actionW, actionEmbed: ARRAY OF CHAR);
		loaded: BOOLEAN;

	PROCEDURE Load*;
	BEGIN

		IF loaded THEN RETURN END;
		loaded := TRUE; 	(*__________________________________________________________________________________________________________

		Model menu
		*)

		(*	sets default directory actually prepends file names with a directory path	*)
		Command("modelSetWD(s)",
		"BugsFiles.SetWorkingDir('^0')",
		"BugsFiles.SetWorkingDir('^0')");

		(*	gets default directory actually what prepends file names	*)
		Command("modelGetWD()",
		"BugsCmds.GetWD()",
		"BugsEmbed.GetWD()");

		(*	check model in file s	*)
		Command("modelCheck(s)",
		"BugsCmds.SetFilePath('^0' ); BugsCmds.ParseGuard; BugsCmds.Parse",
		"BugsEmbed.SetFilePath( '^0' ); BugsEmbed.ParseGuard; BugsEmbed.Parse");

		(*	load data in files s	*)
		Command("modelData(s)",
		"BugsCmds.SetFilePath( '^0' ); BugsCmds.LoadDataGuard; BugsCmds.LoadData",
		"BugsEmbed.SetFilePath( '^0' ); BugsEmbed.LoadDataGuard; BugsEmbed.LoadData");

		(*	compile model using 1 chain	*)
		Command("modelCompile()",
		"BugsCmds.CompileGuard; BugsCmds.specificationDialog.numChains := 1; BugsCmds.Compile",
		"BugsEmbed.CompileGuard; BugsEmbed.numChains := 1; BugsEmbed.Compile");

		(*	compile model using i chains	*)
		Command("modelCompile(i)",
		"BugsCmds.CompileGuard;BugsCmds.specificationDialog.numChains  := ^0; BugsCmds.Compile",
		"BugsEmbed.CompileGuard; BugsEmbed.numChains := ^0; BugsEmbed.Compile");

		(*	load initial values for current chain from file s	*)
		Command("modelInits(s)",
		"BugsCmds.SetFilePath( '^0' ); BugsCmds.LoadInitsGuard; BugsCmds.LoadInits",
		"BugsEmbed.SetFilePath('^0' ); BugsEmbed.LoadInitsGuard; BugsEmbed.LoadInits");

		(*	load initial values for chaini  from file s	*)
		Command("modelInits(s,i)",
		"BugsCmds.SetFilePath( '^0' );BugsCmds.LoadInitsGuard; BugsCmds.specificationDialog.chain := ^1; BugsCmds.LoadInits",
		"BugsEmbed.SetFilePath( '^0' ); BugsEmbed.LoadInitsGuard ; BugsEmbed.chain := ^1; BugsEmbed.LoadInits");

		(*	generate initial values	*)
		Command("modelGenInits()",
		"BugsCmds.GenerateInitsGuard;BugsCmds.GenerateInits",
		"BugsEmbed.GenerateInitsGuard;BugsEmbed.GenerateInits");

		(*	generate initial values	*)
		Command("modelGenInits(s)",
		"BugsCmds.GenerateInitsGuard;BugsCmds.specificationDialog.fixFounder = ^0; BugsCmds.GenerateInits",
		"BugsEmbed.GenerateInitsGuard;BugsEmbed.fixFounder := ^0; BugsEmbed.GenerateInits");

		(*	distribute model using i cores	*)
		Command("modelDistribute(i)",
		"BugsCmds.UpdateGuard;BugsCmds.specificationDialog.numProc  := ^0;BugsMaster.Install('MPImsimp');BugsCmds.Distribute('MPImsimp')",
		"BugsEmbed.UpdateGuard ; BugsEmbed.numProc := ^0; BugsMaster.Install('MPImsimp');BugsEmbed.Distribute('MPImsimp')");

		(*	update model i iterations	*)
		Command("modelUpdate(i)",
		"BugsCmds.UpdateGuard;BugsCmds.updateDialog.updates  := ^0;BugsCmds.Update",
		"BugsEmbed.UpdateGuard ; BugsEmbed.updates := ^0; BugsEmbed.Update");

		(*	update model i0 iteration with prospective thin of i1	*)
		Command("modelUpdate(i, i)",
		"BugsCmds.UpdateGuard ;BugsCmds.updateDialog.updates  := ^0; BugsCmds.updateDialog.thin := ^1; BugsMPIl.Update",
		"BugsEmbed.UpdateGuard ; BugsEmbed.updates := ^0 ; BugsEmbed.thin  := ^1; BugsEmbed.Update");

		(*	update model i0 iteration with prospective thin of i1 and screen refresh rate of i2 *)
		(* refresh rate only used in Windows GUI *)
		Command("modelUpdate(i, i, i)",
		"BugsCmds.UpdateGuard ;BugsCmds.updateDialog.updates  := ^0; BugsCmds.updateDialog.thin := ^1; BugsCmds.updateDialog.refresh := ^2;  BugsCmds.Update",
		"BugsEmbed.UpdateGuard ; BugsEmbed.updates := ^0 ; BugsEmbed.thin  := ^1; BugsEmbed.Update");

		(*	update model i0 iteration with prospective thin of i1 and screen refresh rate of i2 *)
		(* refresh rate only used in Windows GUI *)
		Command("modelUpdate(i, i, i, s)",
		"BugsCmds.UpdateGuard ;BugsCmds.updateDialog.updates  := ^0;BugsCmds.updateDialog.thin := ^1; BugsCmds.updateDialog.refresh := ^2;  BugsCmds.updateDialog.overRelax := ^3; BugsCmds.Update",
		"BugsEmbed.UpdateGuard ; BugsEmbed.updates := ^0; BugsEmbed.thin  := ^1; BugsEmbed.overRelax  := ^3; BugsEmbed.Update");

		(*	update model i iteration with over-relaxation s	*)
		Command("modelUpdate(i, s)",
		"BugsParallel.UpdateGuard ;BugsCmds.updateDialog.updates  := ^0; BugsCmds.updateDialog.overRelax := ^1; BugsCmds.Update",
		"BugsEmbed.UpdateGuard ; BugsEmbed.updates := ^0 ; BugsEmbed.overRelax  := ^1; BugsEmbed.Update");

		(*	update model i0 iteration with prospective thin of i1 and over-relaxation s	*)
		Command("modelUpdate(i, i, s)",
		"BugsCmds.UpdateGuard ;BugsCmds.updateDialog.updates  := ^0;BugsCmds.updateDialog.thin := ^1; BugsCmds.updateDialog.overRelax := ^2; BugsParallel.Update",
		"BugsEmbed.UpdateGuard ; BugsEmbed.updates := ^0; BugsEmbed.thin  := ^1; BugsEmbed.overRelax  := ^2; BugsEmbed.Update");

		(*	update model i iterations	*)
		Command("modelUpdateNI(i, i, s)",
		"BugsCmds.UpdateGuard ;BugsCmds.updateDialog.updates  := ^0;BugsCmds.updateDialog.thin := ^1; BugsCmds.updateDialog.overRelax := ^2; BugsParallel.UpdateNI",
		"BugsEmbed.UpdateGuard ; BugsEmbed.updates := ^0; BugsEmbed.thin  := ^1; BugsEmbed.overRelax  := ^2; BugsEmbed.Update");

		(*	writes out current state of sampler, disabled on non-Windows OS*)
		Command("modelSaveState()",
		"BugsEmbed.UpdateGuard ; BugsCmds.WriteChains",
		"");

		(*	writes out current state of sampler to files with stem name s	*)
		Command("modelSaveState(s)",
		"BugsEmbed.UpdateGuard ;BugsEmbed.WriteChains('^0')",
		"BugsEmbed.UpdateGuard ;BugsEmbed.WriteChains('^0')");

		(*	externalizes the model to file s .bug	*)
		Command("modelExternalize(s)",
		"BugsEmbed.UpdateGuard ;BugsCmds.ExternalizeModel('^0')",
		"BugsEmbed.UpdateGuard ;BugsEmbed.ExternalizeModel('^0')");

		(*	internalizes the model from file s .bug	*)
		Command("modelInternalize(s)",
		"BugsCmds.InternalizeModel('^0')",
		"BugsEmbed.InternalizeModel('^0')");

		(*	gets the state of the random number generator	*)
		Command("modelGetRN()",
		"BugsCmds.GetRNState",
		"BugsEmbed.GetRNState");

		(*	sets the the random number generator to predefined state i	*)
		Command("modelSetRN(i)",
		"BugsCmds.SetRNGuard; BugsCmds.SetRNState(^0)",
		"BugsEmbed.SetRNGuard; BugsEmbed.SetRNState(^0)");

		(*	sets where output is sent to s, disabled on non-Windows OS	*)
		Command("modelDisplay(s)",
		"BugsCmds.SetDisplay('^0')",
		"");

		(*	sets the precision to which real numbers are output	*)
		Command("modelPrecision(i)",
		"BugsCmds.displayDialog.precision :=  ^0; BugsMappers.SetPrec(^0)",
		"BugsMappers.SetPrec(^0)");

		(*	disables factory s creating updaters	*)
		Command("modelDisable(s)",
		"UpdaterMethods.SetFactory('^0');BugsCmds.FactoryGuard;UpdaterMethods.Disable",
		"UpdaterMethods.SetFactory('^0');BugsEmbed.FactoryGuard;UpdaterMethods.Disable");

		(*	enables factory s to create updaters	*)
		Command("modelEnable(s)",
		"UpdaterMethods.SetFactory('^0');BugsCmds.FactoryGuard;UpdaterMethods.Enable",
		"UpdaterMethods.SetFactory('^0');BugsEmbed.FactoryGuard;UpdaterMethods.Enable");

		(*	sets the adaptive phase of updaters created by factory s to i	*)
		Command("modelSetAP(s, i)",
		"UpdaterMethods.SetFactory('^0');BugsCmds.AdaptivePhaseGuard;UpdaterMethods.SetAdaptivePhase(^1)",
		"UpdaterMethods.SetFactory('^0');BugsEmbed.AdaptivePhaseGuard;UpdaterMethods.SetAdaptivePhase(^1)");

		(*	sets the number of iterations allowed in updaters created by factory s to i	*)
		Command("modelSetIts(s, i)",
		"UpdaterMethods.SetFactory('^0');BugsCmds.IterationsGuard;UpdaterMethods.SetIterations(^1)",
		"UpdaterMethods.SetFactory('^0');BugsEmbed.IterationsGuard;UpdaterMethods.SetIterations(^1)");

		(*	sets the amount of over-relaxation used by updaters created by factory s to i	*)
		Command("modelSetOR(s, i)",
		"UpdaterMethods.SetFactory('^0');BugsCmds.OverRelaxationGuard;UpdaterMethods.SetOverRelaxation(^1)",
		"UpdaterMethods.SetFactory('^0');BugsEmbed.OverRelaxationGuard;UpdaterMethods.SetOverRelaxation(^1)");

		(*	sets updater parameters	*)
		Command("modelSetParam(s, i, i, i)",
		"UpdaterMethods.SetFactory('^0');UpdaterSettings.SetAdaptivePhase(^1);UpdaterSettings.SetIterations(^2);UpdaterSettings.SetOverRelaxation(^3)",
		"");

		(*	tries to change sampling method for node	*)
		Command("modelChangeSampler(s, s)",
		"UpdaterMethods.SetFactory('^1');BugsCmds.FactoryGuard;BugsCmds.ChangeSampler('^0')",
		"UpdaterMethods.SetFactory('^1');BugsEmbed.FactoryGuard;BugsEmbed.ChangeSampler('^0')");

		(*___________________________________________________________________________________________

		File Menu

		*)

		(*	causes the BUGS program to close (with modal confirmation dialog in Windows)	*)
		Command("modelQuit()",
		"HostMenus.Exit",
		"BugsEmbed.Quit");

		(*	causes the BUGS program to close if s = "yes" or "y" (without modal confirmation dialog	in Windows)	*)
		Command("modelQuit(s)",
		"BugsCmds.Quit('^0')",
		"BugsEmbed.Quit");

		(*	saves a log of output to file s	*)
		Command("modelSaveLog(s)",
		"BugsCmds.SaveLog('^0' )",
		"");

		(*___________________________________________________________________________________________

		Inference Menu
		*)

		(*	sets the begin iteration for sample monitors to i	*)
		Command("samplesBeg(i)",
		"SamplesCmds.dialog.beg :=  ^0",
		"SamplesEmbed.beg :=  ^0");

		(*	sets the end iteration for sample monitors to i	*)
		Command("samplesEnd(i)",
		"SamplesCmds.dialog.end := ^0",
		"SamplesEmbed.end := ^0");

		(*	sets the retrospective thin for sample monitors to i	*)
		Command("samplesThin(i)",
		"SamplesCmds.dialog.thin :=  ^0",
		"SamplesEmbed.thin :=  ^0");

		(*	sets the first chain for sample monitors to i	*)
		Command("samplesFirstChain(i)",
		"SamplesCmds.dialog.firstChain :=  ^0",
		"SamplesEmbed.firstChain :=  ^0");

		(*	sets the last chain for sample monitors to i	*)
		Command("samplesLastChain(i)",
		"SamplesCmds.dialog.lastChain := ^0",
		"SamplesEmbed.lastChain := ^0");

		(*	sets a sample monitor for variable s	*)
		Command("samplesSet(s)",
		"SamplesCmds.SetVariable('^0');SamplesCmds.SetGuard;SamplesCmds.Set",
		"SamplesEmbed.SetVariable('^0');SamplesEmbed.SetGuard;SamplesEmbed.Set");

		(*	clears the sample monitor for variable s	*)
		Command("samplesClear(s)",
		"SamplesCmds.SetVariable('^0');SamplesCmds.HistoryGuard;SamplesCmds.ClearNI",
		"SamplesEmbed.SetVariable('^0');SamplesEmbed.HistoryGuard;SamplesEmbed.Clear");

		(*	displays summary statistics for sample monitor s	*)
		Command("samplesStats(s)",
		"SamplesCmds.SetVariable('^0');SamplesCmds.StatsGuard;SamplesCmds.Stats",
		"SamplesEmbed.SetVariable('^0');SamplesEmbed.StatsGuard;SamplesEmbed.Stats");

		(*	displays CODA output for sample monitor s, disabled on non-Windows OS	*)
		Command("samplesCoda(s)",
		"SamplesCmds.SetVariable('^0');SamplesCmds.StatsGuard;SamplesCmds.CODA()",
		"");

		(*	writes CODA output for sample monitor s0 to files with stem name s1	*)
		Command("samplesCoda(s, s)",
		"SamplesCmds.SetVariable('^0');SamplesCmds.StatsGuard;SamplesCmds.CODAFiles('^1')",
		"SamplesEmbed.SetVariable('^0');SamplesEmbed.StatsGuard;SamplesEmbed.CODA ('^1')");

		(*	displays density estimates for sample monitor s, disabled on non-Windows OS	*)
		Command("samplesDensity(s)",
		"SamplesCmds.SetVariable('^0' );SamplesCmds.StatsGuard;SamplesDensity.Install;SamplesCmds.Plot",
		"");

		(*	displays auto correlation for sample monitor s , disabled on non-Windows OS	*)
		Command("samplesAutoC(s)",
		"SamplesCmds.SetVariable('^0');SamplesCmds.StatsGuard;SamplesCorrelat.Install;SamplesCmds.Plot",
		"");

		(*	displays dynamic traces for sample monitor s, disabled on non-Windows OS	*)
		Command("samplesTrace(s)",
		"SamplesCmds.SetVariable('^0');SamplesCmds.StatsGuard;SamplesTrace.Install;SamplesCmds.Plot",
		"");

		(*	displays history plots for sample monitor s, disabled on non-Windows OS	*)
		Command("samplesHistory(s)",
		"SamplesCmds.SetVariable('^0');SamplesCmds.StatsGuard;SamplesHistory.Install;SamplesCmds.Plot",
		"");

		(*	displays cumulative quantile plots for sample monitor s , disabled on non-Windows OS	*)
		Command("samplesQuantiles(s)",
		"SamplesCmds.SetVariable('^0');SamplesCmds.StatsGuard;SamplesQuantiles.Install;SamplesCmds.Plot",
		"");

		(*	displays bgr convergence diagnostic plots for sample monitor s, disabled on non-Windows OS	*)
		Command("samplesBgr(s)",
		"SamplesCmds.SetVariable('^0');SamplesCmds.BGRGuard;SamplesDiagnostics.Install;SamplesCmds.Plot",
		"");

		(*	sets summary monitor for variable s	*)
		Command("summarySet(s)",
		"SummaryCmds.SetVariable('^0');SummaryCmds.SetGuard;SummaryCmds.Set",
		"SummaryEmbed.SetVariable('^0');SummaryEmbed.SetGuard;SummaryEmbed.Set");

		(*	displays summary statistics for summary monitor s	*)
		Command("summaryStats(s)",
		"SummaryCmds.SetVariable('^0' );SummaryCmds.StatsGuard;SummaryCmds.Stats",
		"SummaryEmbed.SetVariable('^0' );SummaryEmbed.StatsGuard;SummaryEmbed.Stats");

		(*	displays mean for summary monitor s	*)
		Command("summaryMean(s)",
		"SummaryCmds.SetVariable('^0');SummaryCmds.StatsGuard;SummaryCmds.Means",
		"SummaryEmbed.SetVariable('^0');SummaryEmbed.StatsGuard;SummaryEmbed.Means");

		(*	clears summary monitor s	*)
		Command("summaryClear(s)",
		"SummaryCmds.SetVariable('^0');SummaryCmds.StatsGuard;SummaryCmds.ClearNI",
		"SummaryEmbed.SetVariable('^0');SummaryEmbed.StatsGuard;SummaryEmbed.Clear");

		(*	sets ranks monitor for variable s	*)
		Command("ranksSet(s)",
		"RanksCmds.SetVariable('^0');RanksCmds.SetGuard;RanksCmds.Set",
		"RanksEmbed.SetVariable('^0');RanksEmbed.SetGuard;RanksEmbed.Set");

		(*	displays rank information for ranks monitor for variable s	*)
		Command("ranksStats(s)",
		"RanksCmds.SetVariable('^0' );RanksCmds.StatsGuard;RanksCmds.Stats",
		"RanksEmbed.SetVariable('^0' );RanksEmbed.StatsGuard;RanksEmbed.Stats");

		(*	clears ranks monitor s	*)
		Command("ranksClear(s)",
		"RanksCmds.SetVariable('^0');RanksCmds.StatsGuard;RanksCmds.Clear",
		"RanksEmbed.SetVariable('^0');RanksEmbed.StatsGuard;RanksEmbed.Clear");

		(*	displays histograms for ranks monitor for variable s, disabled on non-Windows OS *)
		Command("ranksHistogram(s)",
		"RanksCmds.SetVariable('^0');RanksDialog.StatsGuard;RanksCmds.Draw",
		"");

		(*	sets model monitor for variable s	*)
		Command("modelsSet(s)",
		"ModelsCmds.SetVariable('^0');ModelsCmds.SetGuard;ModelsCmds.Set",
		"ModelsEmbed.SetVariable('^0');ModelsEmbed.SetGuard;ModelsEmbed.Set");

		(*	displays component probs for  model monitor for variable s	*)
		Command("modelsSet(s)",
		"ModelsCmds.SetVariable('^0');ModelsCmds.StatsGuard;ModelsCmds.ComponentProbs",
		"ModelsEmbed.SetVariable('^0');ModelsEmbed.StatsGuard;ModelsEmbed.ComponentProbs");

		(*	displays model probs for  model monitor for variable s	*)
		Command("modelsSet(s)",
		"ModelsCmds.SetVariable('^0');ModelsCmds.StatsGuard;ModelsCmds.ModelProbs",
		"ModelsEmbed.SetVariable('^0');ModelsEmbed.StatsGuard;ModelsEmbed.ModelProbs");

		(*	clears model monitor for variable s	*)
		Command("modelsClear(s)",
		"ModelsCmds.SetVariable('^0');ModelsCmds.StatsGuard;ModelsCmds.Clear",
		"ModelsEmbed.SetVariable('^0');ModelsEmbed.StatsGuard;ModelsEmbed.Clear");

		(*	sets monitor for DIC	*)
		Command("dicSet()",
		"DevianceCmds.SetVariable('*'); DevianceCmds.SetGuard; DevianceCmds.Set",
		"DevianceEmbed.SetVariable('*'); DevianceEmbed.SetGuard ; DevianceEmbed.Set");

		(*	sets monitor for DIC with direct parents should only run on worker	*)
		Command("dicSetD()",
		"DeviancePluginD.Install;DevianceCmds.SetGuard; DevianceCmds.Set",
		"DeviancePluginD.Install;DevianceEmbed.SetGuard ; DevianceEmbed.Set");

		(*	sets monitor for DIC with stochastic parents should only run on worker	*)
		Command("dicSetS()",
		"DeviancePluginS.Install;DevianceCmds.SetGuard; DevianceCmds.Set",
		"DeviancePluginS.Install;DevianceEmbed.SetGuard ; DevianceEmbed.Set");

		(*	clears monitor for DIC	*)
		Command("dicClear()",
		"DevianceCmds.SetVariable('*'); DevianceCmds.StatsGuard ; DevianceCmds.Clear",
		"DevianceEmbed.SetVariable('*'); DevianceEmbed.StatsGuard ; DevianceEmbed.Clear");

		(*	displays DIC statistics	*)
		Command("dicStats()",
		"DevianceCmds.StatsGuard; DevianceCmds.Stats",
		"DevianceEmbed.StatsGuard; DevianceEmbed.Stats");

		(*	clears monitor for DIC	*)
		Command("dicClear ()",
		"DevianceCmds.StatsGuard ; DevianceCmds.Clear",
		"DevianceEmbed.StatsGuard ; DevianceEmbed.Clear");

		(*___________________________________________________________________________________________

		Info Menu
		*)

		(*	display node values	*)
		Command("infoNodeValue(s)",
		"BugsCmds.SetNode('^0'); BugsCmds.Values",
		"BugsEmbed.SetNode('^0'); BugsEmbed.Values");

		(*	display node methods	*)
		Command("infoNodeMethods(s)",
		"BugsCmds.SetNode('^0'); BugsDebug.Methods",
		"BugsEmbed.SetNode('^0'); BugsEmbed.Methods");

		(*	display node types	*)
		Command("infoNodeTypes(s)",
		"BugsCmds.SetNode('^0'); BugsDebug.Types",
		"BugsEmbed.SetNode('^0'); BugsEmbed.Types");

		(*	display updaters that are not initialized	*)
		Command("infoUnitializedUpdaters()",
		"BugsCmds.NotCompiledGuard; BugsCmds.WriteUninitNodes",
		"");

		(*	display updaters sorted by node name	*)
		Command("infoUpdatersbyName()",
		"BugsCmds.NotCompiledGuard; BugsCmds.UpdatersByName",
		"BugsEmbed.NotCompiledGuard; BugsEmbed.UpdatersByName");

		(*	display updaters sorted by node depth in graph	*)
		Command("infoUpdatersbyDepth()",
		"BugsCmds.NotCompiledGuard; BugsCmds.UpdatersByDepth",
		"BugsEmbed.NotCompiledGuard; BugsEmbed.UpdatersByDepth");

		(*	displays the modules of the OpenBUGS software currently loaded	*)
		Command("infoModules()",
		"BugsCmds.Modules",
		"BugsEmbed.Modules");

		(*	displays the amount of memory used by software	*)
		Command("infoMemory()",
		"BugsCmds.AllocatedMemory",
		"BugsEmbed.AllocatedMemory");

	END Load;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		Command := BugsScripting.LoadCommand;
		loaded := FALSE
	END Init;

BEGIN
	Init
END BugsScripts.


