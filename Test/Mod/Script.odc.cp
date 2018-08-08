(*	

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	*)

MODULE TestScript;

	

	IMPORT
		Dialog, Services,
		StdLog,
		BugsCmds, BugsFiles, BugsMaster,
		MapsCmds,
		SamplesCmds, SamplesDensity, SamplesDiagnostics,
		SummaryCmds;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Compile (model: ARRAY OF CHAR);
		VAR
			par: Dialog.Par;
	BEGIN
		BugsCmds.SetDisplay("log");
		StdLog.String("**********"); StdLog.Ln;
		StdLog.String("*  " + model); StdLog.Ln;
		StdLog.String("**********"); StdLog.Ln;
		BugsCmds.SetFilePath("Examples/" + model + "model");
		BugsCmds.Parse;
		BugsCmds.SetFilePath("Examples/" + model + "data");
		BugsCmds.LoadData;
		BugsCmds.specificationDialog.numChains := 2;
		BugsCmds.Compile;
		BugsCmds.SetFilePath("Examples/" + model + "inits");
		BugsCmds.LoadInits;
		BugsCmds.SetFilePath("Examples/" + model + "inits1");
		BugsCmds.LoadInits;
		BugsCmds.GenerateInitsGuardWin(par);
		IF ~par.disabled THEN BugsCmds.GenerateInits END;
		BugsCmds.specificationDialog.numProc := 4;
		BugsMaster.Install('MPImsimp');
		BugsCmds.Distribute('MPImsimp')
	END Compile;

	PROCEDURE CompileGeo (model: ARRAY OF CHAR);
		VAR
			par: Dialog.Par;
	BEGIN
		BugsCmds.SetDisplay("log");
		StdLog.String("**********"); StdLog.Ln;
		StdLog.String("*  " + model); StdLog.Ln;
		StdLog.String("**********"); StdLog.Ln;
		BugsCmds.SetFilePath("GeoBUGS/Examples/" + model + "model");
		BugsCmds.Parse;
		BugsCmds.SetFilePath("GeoBUGS/Examples/" + model + "data");
		BugsCmds.LoadData;
		BugsCmds.specificationDialog.numChains := 2;
		BugsCmds.Compile;
		BugsCmds.SetFilePath("GeoBUGS/Examples/" + model + "inits");
		BugsCmds.LoadInits;
		BugsCmds.SetFilePath("GeoBUGS/Examples/" + model + "inits1");
		BugsCmds.LoadInits;
		BugsCmds.GenerateInitsGuardWin(par);
		IF ~par.disabled THEN BugsCmds.GenerateInits END
	END CompileGeo;

	PROCEDURE CompileEco (model: ARRAY OF CHAR);
		VAR
			par: Dialog.Par;
	BEGIN
		BugsCmds.SetDisplay("log");
		StdLog.String("**********"); StdLog.Ln;
		StdLog.String("*  " + model); StdLog.Ln;
		StdLog.String("**********"); StdLog.Ln;
		BugsCmds.SetFilePath("Eco/" + model + "model");
		BugsCmds.Parse;
		BugsCmds.SetFilePath("Eco/" + model + "data");
		BugsCmds.LoadDataGuardWin(par);
		IF ~par.disabled THEN BugsCmds.LoadData END;
		BugsCmds.SetFilePath("Eco/" + model + "data1");
		BugsCmds.LoadDataGuardWin(par);
		IF ~par.disabled THEN BugsCmds.LoadData END;
		BugsCmds.SetFilePath("Eco/" + model + "data2");
		BugsCmds.LoadDataGuardWin(par);
		IF ~par.disabled THEN BugsCmds.LoadData END;
		BugsCmds.SetFilePath("Eco/" + model + "data3");
		BugsCmds.LoadDataGuardWin(par);
		IF ~par.disabled THEN BugsCmds.LoadData END;
		BugsCmds.specificationDialog.numChains := 2;
		BugsCmds.Compile;
		BugsCmds.SetFilePath("Eco/" + model + "inits1");
		BugsCmds.LoadInits;
		BugsCmds.SetFilePath("Eco/" + model + "inits2");
		BugsCmds.LoadInits;
		BugsCmds.GenerateInitsGuardWin(par);
		IF ~par.disabled THEN BugsCmds.GenerateInits END
	END CompileEco;

	PROCEDURE Restore;
	BEGIN
	(*
		BugsCmds.ExternalizeModel("junk");
		BugsCmds.Clear;
		BugsCmds.InternalizeModel("junk");
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;*)
	END Restore;

	PROCEDURE Air*;
	BEGIN
		Compile("Air");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('X'); SamplesCmds.Set;
		SamplesCmds.SetVariable('theta'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Air;

	PROCEDURE Aligators*;
	BEGIN
		Compile("Aligators");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('b'); SamplesCmds.Set;
		SamplesCmds.SetVariable('g'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Aligators;

	PROCEDURE Asia*;
	BEGIN
		Compile("Asia");
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('bronchitis'); SamplesCmds.Set;
		SamplesCmds.SetVariable('either'); SamplesCmds.Set;
		SamplesCmds.SetVariable('lung.cancer'); SamplesCmds.Set;
		SamplesCmds.SetVariable('smoking'); SamplesCmds.Set;
		SamplesCmds.SetVariable('tuberculosis'); SamplesCmds.Set;
		SamplesCmds.SetVariable('xray'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 100000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Asia;

	PROCEDURE Beetles*;
	BEGIN
		Compile("Beetles");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('alpha'); SamplesCmds.Set;
		SamplesCmds.SetVariable('beta'); SamplesCmds.Set;
		SamplesCmds.SetVariable('rhat'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Beetles;

	PROCEDURE Biopsies*;
	BEGIN
		Compile("Biopsies");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('error'); SamplesCmds.Set;
		SamplesCmds.SetVariable('p'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Biopsies;

	PROCEDURE BiRats*;
	BEGIN
		Compile("BiRats");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('mu.beta'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END BiRats;

	PROCEDURE Blockers*;
	BEGIN
		Compile("Blockers");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('d'); SamplesCmds.Set;
		SamplesCmds.SetVariable('delta.new'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Blockers;

	PROCEDURE Bones*;
	BEGIN
		Compile("Bones");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('theta'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Bones;

	PROCEDURE Camel*;
	BEGIN
		Compile("Camel");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('Sigma2'); SamplesCmds.Set;
		SamplesCmds.SetVariable('rho'); SamplesCmds.Set;
		SamplesCmds.SetVariable('tau'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 100000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Camel;

	PROCEDURE Cervix*;
	BEGIN
		Compile("Cervix");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('beta0C'); SamplesCmds.Set;
		SamplesCmds.SetVariable('gamma1'); SamplesCmds.Set;
		SamplesCmds.SetVariable('gamma2'); SamplesCmds.Set;
		SamplesCmds.SetVariable('phi'); SamplesCmds.Set;
		SamplesCmds.SetVariable('q'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Cervix;

	PROCEDURE Dogs*;
	BEGIN
		Compile("Dogs");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('A'); SamplesCmds.Set;
		SamplesCmds.SetVariable('B'); SamplesCmds.Set;
		SamplesCmds.SetVariable('alpha'); SamplesCmds.Set;
		SamplesCmds.SetVariable('beta'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Dogs;

	PROCEDURE Dugongs*;
	BEGIN
		Compile("Dugongs");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('U3'); SamplesCmds.Set;
		SamplesCmds.SetVariable('alpha'); SamplesCmds.Set;
		SamplesCmds.SetVariable('beta'); SamplesCmds.Set;
		SamplesCmds.SetVariable('gamma'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Dugongs;

	PROCEDURE Dyes*;
	BEGIN
		Compile("Dyes");
		BugsCmds.updateDialog.updates := 25000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('sigma2.btw'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma2.with'); SamplesCmds.Set;
		SamplesCmds.SetVariable('theta'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 100000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Dyes;

	PROCEDURE Endo*;
	BEGIN
		Compile("Endo");
		BugsCmds.updateDialog.updates := 5000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('beta'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Endo;

	PROCEDURE Epil*;
	BEGIN
		Compile("Epil");
		BugsCmds.updateDialog.updates := 5000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('alpha.Age'); SamplesCmds.Set;
		SamplesCmds.SetVariable('alpha.BT'); SamplesCmds.Set;
		SamplesCmds.SetVariable('alpha.Base'); SamplesCmds.Set;
		SamplesCmds.SetVariable('alpha.Trt'); SamplesCmds.Set;
		SamplesCmds.SetVariable('alpha.V4'); SamplesCmds.Set;
		SamplesCmds.SetVariable('alpha0'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma.b'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma.b1'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Epil;

	PROCEDURE Equiv*;
	BEGIN
		Compile("Equiv");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('equiv'); SamplesCmds.Set;
		SamplesCmds.SetVariable('mu'); SamplesCmds.Set;
		SamplesCmds.SetVariable('phi'); SamplesCmds.Set;
		SamplesCmds.SetVariable('pi'); SamplesCmds.Set;
		SamplesCmds.SetVariable('alpha.Trt'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma1'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma2'); SamplesCmds.Set;
		SamplesCmds.SetVariable('theta'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Equiv;

	PROCEDURE Eyes*;
	BEGIN
		Compile("Eyes");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('P'); SamplesCmds.Set;
		SamplesCmds.SetVariable('lambda'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Eyes;

	PROCEDURE Eyetracking*;
	BEGIN
		Compile("Eyetracking");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('A'); SamplesCmds.Set;
		SamplesCmds.SetVariable('B'); SamplesCmds.Set;
		SamplesCmds.SetVariable('K'); SamplesCmds.Set;
		SamplesCmds.SetVariable('deviance'); SamplesCmds.Set;
		SamplesCmds.SetVariable('mu[92]'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		SamplesCmds.SetVariable('mu[92]');
		SamplesDensity.Install; SamplesCmds.Plot;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Eyetracking;

	PROCEDURE Hearts*;
	BEGIN
		Compile("Hearts");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('alpha'); SamplesCmds.Set;
		SamplesCmds.SetVariable('beta'); SamplesCmds.Set;
		SamplesCmds.SetVariable('delta'); SamplesCmds.Set;
		SamplesCmds.SetVariable('theta'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Hearts;

	PROCEDURE Hepatitis*;
	BEGIN
		Compile("Hepatitis");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('alpha0'); SamplesCmds.Set;
		SamplesCmds.SetVariable('beta0'); SamplesCmds.Set;
		SamplesCmds.SetVariable('gamma'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Hepatitis;

	PROCEDURE HepatitisME*;
	BEGIN
		Compile("HepatitisME");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('alpha0'); SamplesCmds.Set;
		SamplesCmds.SetVariable('beta0'); SamplesCmds.Set;
		SamplesCmds.SetVariable('gamma'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END HepatitisME;

	PROCEDURE Ice*;
	BEGIN
		Compile("Ice");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('logRR'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Ice;

	PROCEDURE Inhalers*;
	BEGIN
		Compile("Inhalers");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('a'); SamplesCmds.Set;
		SamplesCmds.SetVariable('beta'); SamplesCmds.Set;
		SamplesCmds.SetVariable('kappa'); SamplesCmds.Set;
		SamplesCmds.SetVariable('log.sigma'); SamplesCmds.Set;
		SamplesCmds.SetVariable('pi'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Inhalers;

	PROCEDURE Jama*;
		VAR
			par: Dialog.Par;
		CONST
			model = "Jama";
	BEGIN
		BugsCmds.SetDisplay("log");
		StdLog.String("**********"); StdLog.Ln;
		StdLog.String("*  " + model); StdLog.Ln;
		StdLog.String("**********"); StdLog.Ln;
		BugsCmds.SetFilePath("Examples/" + model + "model");
		BugsCmds.Parse;
		BugsCmds.SetFilePath("Examples/" + model + "data1");
		BugsCmds.LoadData;
		BugsCmds.SetFilePath("Examples/" + model + "data2");
		BugsCmds.LoadData;
		BugsCmds.SetFilePath("Examples/RCdata");
		BugsCmds.LoadData;
		BugsCmds.specificationDialog.numChains := 2;
		BugsCmds.Compile;
		BugsCmds.SetFilePath("Examples/" + model + "inits");
		BugsCmds.LoadInits;
		BugsCmds.SetFilePath("Examples/" + model + "inits1");
		BugsCmds.LoadInits;
		BugsCmds.GenerateInitsGuardWin(par);
		IF ~par.disabled THEN BugsCmds.GenerateInits END;
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('alpha.desc'); SamplesCmds.Set;
		SamplesCmds.SetVariable('beta.desc'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*');
		SamplesDensity.Install; SamplesCmds.Plot;
		SamplesCmds.Stats;
		Restore;
		StdLog.Ln;
		BugsCmds.SetDisplay('window')
	END Jama;

	PROCEDURE Jaws*;
	BEGIN
		Compile("Jaws");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('Sigma'); SamplesCmds.Set;
		SamplesCmds.SetVariable('beta0'); SamplesCmds.Set;
		SamplesCmds.SetVariable('beta1'); SamplesCmds.Set;
		SamplesCmds.SetVariable('mu'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Jaws;

	PROCEDURE Kidney*;
	BEGIN
		Compile("Kidney");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('alpha'); SamplesCmds.Set;
		SamplesCmds.SetVariable('beta.dis'); SamplesCmds.Set;
		SamplesCmds.SetVariable('beta.sex'); SamplesCmds.Set;
		SamplesCmds.SetVariable('r'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Kidney;

	PROCEDURE Leuk*;
	BEGIN
		Compile("Leuk");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('S.placebo'); SamplesCmds.Set;
		SamplesCmds.SetVariable('S.treat'); SamplesCmds.Set;
		SamplesCmds.SetVariable('beta'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Leuk;

	PROCEDURE Leukfr*;
	BEGIN
		Compile("Leukfr");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('beta'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Leukfr;

	PROCEDURE Line*;
	BEGIN
		Compile("Line");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('alpha'); SamplesCmds.Set;
		SamplesCmds.SetVariable('beta'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Line;

	PROCEDURE Lsat*;
	BEGIN
		Compile("Lsat");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('a'); SamplesCmds.Set;
		SamplesCmds.SetVariable('beta'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Lsat;

	PROCEDURE Mice*;
	BEGIN
		Compile("Mice");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('median'); SamplesCmds.Set;
		SamplesCmds.SetVariable('pos.control'); SamplesCmds.Set;
		SamplesCmds.SetVariable('r'); SamplesCmds.Set;
		SamplesCmds.SetVariable('test.sub'); SamplesCmds.Set;
		SamplesCmds.SetVariable('veh.control'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Mice;

	PROCEDURE Otrees*;
	BEGIN
		Compile("Otrees");
		BugsCmds.updateDialog.updates := 15000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('mu'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigmaC'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Otrees;

	PROCEDURE OtreesMVN*;
	BEGIN
		Compile("OtreesMVN");
		BugsCmds.updateDialog.updates := 5000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('mu'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigmaC'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END OtreesMVN;

	PROCEDURE Oxford*;
	BEGIN
		Compile("Oxford");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('alpha'); SamplesCmds.Set;
		SamplesCmds.SetVariable('beta1'); SamplesCmds.Set;
		SamplesCmds.SetVariable('beta2'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Oxford;

	PROCEDURE Pigs*;
	BEGIN
		Compile("Pigs");
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('i'); SamplesCmds.Set;
		SamplesCmds.SetVariable('p'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 100000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Pigs;

	PROCEDURE Pigweights*;
	BEGIN
		Compile("Pigweights");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('Sm'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Pigweights;

	PROCEDURE Rats*;
	BEGIN
		Compile("Rats");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('alpha0'); SamplesCmds.Set;
		SamplesCmds.SetVariable('beta.c'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Rats;

	PROCEDURE Salm*;
	BEGIN
		Compile("Salm");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('alpha'); SamplesCmds.Set;
		SamplesCmds.SetVariable('beta'); SamplesCmds.Set;
		SamplesCmds.SetVariable('gamma'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Salm;

	PROCEDURE Schools*;
	BEGIN
		Compile("Schools");
		BugsCmds.updateDialog.updates := 100000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('beta'); SamplesCmds.Set;
		SamplesCmds.SetVariable('gamma'); SamplesCmds.Set;
		SamplesCmds.SetVariable('phi'); SamplesCmds.Set;
		SamplesCmds.SetVariable('theta'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Schools;

	PROCEDURE Seeds*;
	BEGIN
		Compile("Seeds");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('alpha0'); SamplesCmds.Set;
		SamplesCmds.SetVariable('alpha1'); SamplesCmds.Set;
		SamplesCmds.SetVariable('alpha12'); SamplesCmds.Set;
		SamplesCmds.SetVariable('alpha2'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Seeds;

	PROCEDURE Sixcomp*;
		CONST
			model = "Sixcomp";
	BEGIN
		BugsCmds.SetDisplay("log");
		StdLog.String("**********"); StdLog.Ln;
		StdLog.String("*  " + model); StdLog.Ln;
		StdLog.String("**********"); StdLog.Ln;
		BugsCmds.SetFilePath("Examples/" + model + "model");
		BugsCmds.Parse;
		BugsCmds.SetFilePath("Examples/" + model + "data");
		BugsCmds.LoadData;
		BugsCmds.specificationDialog.numChains := 1;
		BugsCmds.Compile;
		BugsCmds.SetNode('solution'); BugsCmds.Values;
		StdLog.Ln;
		Restore;
		BugsCmds.SetNode('solution'); BugsCmds.Values;
		BugsCmds.SetDisplay('window')
	END Sixcomp;

	PROCEDURE Stacks*;
	BEGIN
		Compile("Stacks");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('b'); SamplesCmds.Set;
		SamplesCmds.SetVariable('outlier'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Stacks;

	PROCEDURE Stagnantc*;
	BEGIN
		Compile("Stagnantc");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('alpha'); SamplesCmds.Set;
		SamplesCmds.SetVariable('beta'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma'); SamplesCmds.Set;
		SamplesCmds.SetVariable('x.change'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Stagnantc;

	PROCEDURE StVeit*;
		CONST
			model = "StVeit";
		VAR
			par: Dialog.Par;
	BEGIN
		BugsCmds.SetDisplay("log");
		StdLog.String("**********"); StdLog.Ln;
		StdLog.String("*  " + model); StdLog.Ln;
		StdLog.String("**********"); StdLog.Ln;
		BugsCmds.SetFilePath("Examples/" + model + "model");
		BugsCmds.Parse;
		BugsCmds.SetFilePath("Examples/" + model + "data");
		BugsCmds.LoadData;
		BugsCmds.SetFilePath("Examples/RCdata");
		BugsCmds.LoadData;
		BugsCmds.specificationDialog.numChains := 1;
		BugsCmds.Compile;
		BugsCmds.SetFilePath("Examples/" + model + "inits");
		BugsCmds.LoadInits;
		BugsCmds.GenerateInitsGuardWin(par);
		IF ~par.disabled THEN BugsCmds.GenerateInits END;
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('theta.smooth'); SamplesCmds.Set;
		SamplesCmds.SetVariable('within'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('theta.smooth'); SamplesCmds.Stats;
		SamplesDensity.Install; SamplesCmds.PlotMarkov;
		SamplesCmds.SetVariable('within'); SamplesCmds.Stats;
		Restore;
		StdLog.Ln;
		BugsCmds.SetDisplay('window')
	END StVeit;

	PROCEDURE Surgical*;
	BEGIN
		Compile("Surgical");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('p'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Surgical;

	PROCEDURE Surgicalrand*;
	BEGIN
		Compile("Surgicalrand");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('mu'); SamplesCmds.Set;
		SamplesCmds.SetVariable('p'); SamplesCmds.Set;
		SamplesCmds.SetVariable('pop.mean'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		Restore;
		BugsCmds.SetDisplay('window')
	END Surgicalrand;

	(************************************************************************************************************************)

	PROCEDURE Elevation*;
	BEGIN
		CompileGeo("Elevation");
		BugsCmds.updateDialog.updates := 2000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('phi'); SamplesCmds.Set;
		SamplesCmds.SetVariable('kappa'); SamplesCmds.Set;
		SamplesCmds.SetVariable('beta'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma2'); SamplesCmds.Set;
		SummaryCmds.SetVariable('height.pred'); SummaryCmds.Set;
		SamplesCmds.SetVariable('height.pred.multi'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		MapsCmds.SetMap("Elevation");
		MapsCmds.SetPalette(MapsCmds.grey);
		MapsCmds.SetQuantity("height.pred");
		MapsCmds.SetType(MapsCmds.summaryMean);
		MapsCmds.SetDefaultCuts;
		StdLog.Ln;
		MapsCmds.Plot;
		StdLog.Ln
	END Elevation;

	PROCEDURE Forest*;
	BEGIN
		CompileGeo("Forest");
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('alpha'); SamplesCmds.Set;
		SamplesCmds.SetVariable('beta'); SamplesCmds.Set;
		SamplesCmds.SetVariable('theta0'); SamplesCmds.Set;
		SamplesCmds.SetVariable('theta1'); SamplesCmds.Set;
		SamplesCmds.SetVariable('theta2'); SamplesCmds.Set;
		SamplesCmds.SetVariable('spatial.effect'); SamplesCmds.Set;
		SummaryCmds.SetVariable('density'); SummaryCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		MapsCmds.SetMap("Forest");
		MapsCmds.SetPalette(MapsCmds.blues);
		MapsCmds.SetQuantity("density");
		MapsCmds.SetType(MapsCmds.summaryMean);
		MapsCmds.SetDefaultCuts;
		StdLog.Ln;
		MapsCmds.Plot;
		StdLog.Ln
	END Forest;

	PROCEDURE Huddersfield*;
	BEGIN
		CompileGeo("Huddersfield");
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('PC.base'); SamplesCmds.Set;
		SamplesCmds.SetVariable('PC.latent'); SamplesCmds.Set;
		SamplesCmds.SetVariable('PC.no2'); SamplesCmds.Set;
		SamplesCmds.SetVariable('rate.base'); SamplesCmds.Set;
		SamplesCmds.SetVariable('rate.latent'); SamplesCmds.Set;
		SamplesCmds.SetVariable('rate.no2'); SamplesCmds.Set;
		SummaryCmds.SetVariable('LATENT'); SummaryCmds.Set;
		SummaryCmds.SetVariable('RATE'); SummaryCmds.Set;
		BugsCmds.updateDialog.updates := 20000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		MapsCmds.SetMap("Huddersfield_750m_grid");
		MapsCmds.SetPalette(MapsCmds.blues);
		MapsCmds.SetQuantity("LATENT");
		MapsCmds.SetType(MapsCmds.summaryMean);
		MapsCmds.SetDefaultCuts;
		StdLog.Ln;
		MapsCmds.Plot;
		StdLog.Ln;
		MapsCmds.SetQuantity("RATE");
		MapsCmds.SetType(MapsCmds.summaryMean);
		MapsCmds.SetDefaultCuts;
		StdLog.Ln;
		MapsCmds.Plot;
		StdLog.Ln;
	END Huddersfield;

	PROCEDURE LHA*;
	BEGIN
		CompileGeo("LHA");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('alpha'); SamplesCmds.Set;
		SamplesCmds.SetVariable('beta'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma.b'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma.h'); SamplesCmds.Set;
		SummaryCmds.SetVariable('RR'); SummaryCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		MapsCmds.SetMap("LHA");
		MapsCmds.SetPalette(MapsCmds.blues);
		MapsCmds.SetQuantity("RR");
		MapsCmds.SetType(MapsCmds.summaryMean);
		MapsCmds.SetDefaultCuts;
		StdLog.Ln;
		MapsCmds.Plot;
		StdLog.Ln
	END LHA;

	PROCEDURE MVCAR*;
	BEGIN
		CompileGeo("MVCAR");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('alpha'); SamplesCmds.Set;
		SamplesCmds.SetVariable('corr'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma'); SamplesCmds.Set;
		SummaryCmds.SetVariable('RR1'); SummaryCmds.Set;
		SummaryCmds.SetVariable('RR2'); SummaryCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		MapsCmds.SetMap("WestYorkshire");
		MapsCmds.SetPalette(MapsCmds.blues);
		MapsCmds.SetQuantity("RR1");
		MapsCmds.SetType(MapsCmds.summaryMean);
		MapsCmds.SetDefaultCuts;
		StdLog.Ln;
		MapsCmds.Plot;
		StdLog.Ln;
		MapsCmds.SetQuantity("RR2");
		MapsCmds.SetType(MapsCmds.summaryMean);
		MapsCmds.SetDefaultCuts;
		StdLog.Ln;
		MapsCmds.Plot;
		StdLog.Ln
	END MVCAR;

	PROCEDURE MVCARCon*;
	BEGIN
		CompileGeo("MVCARCon");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('alpha'); SamplesCmds.Set;
		SamplesCmds.SetVariable('corr'); SamplesCmds.Set;
		SamplesCmds.SetVariable('corr.U'); SamplesCmds.Set;
		SamplesCmds.SetVariable('corr.sum'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma.U'); SamplesCmds.Set;
		SummaryCmds.SetVariable('RR1'); SummaryCmds.Set;
		SummaryCmds.SetVariable('RR2'); SummaryCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		MapsCmds.SetMap("WestYorkshire");
		MapsCmds.SetPalette(MapsCmds.blues);
		MapsCmds.SetQuantity("RR1");
		MapsCmds.SetType(MapsCmds.summaryMean);
		MapsCmds.SetDefaultCuts;
		StdLog.Ln;
		MapsCmds.Plot;
		StdLog.Ln;
		MapsCmds.SetQuantity("RR2");
		MapsCmds.SetType(MapsCmds.summaryMean);
		MapsCmds.SetDefaultCuts;
		StdLog.Ln;
		MapsCmds.Plot;
		StdLog.Ln
	END MVCARCon;

	PROCEDURE Pollution*;
	BEGIN
		CompileGeo("Pollution");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('beta'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma.err'); SamplesCmds.Set;
		SummaryCmds.SetVariable('mu'); SummaryCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		SummaryCmds.SetVariable('*'); SummaryCmds.Stats
	END Pollution;

	PROCEDURE Rongelap*;
		VAR
			par: Dialog.Par;
	BEGIN
		BugsCmds.SetDisplay("log");
		StdLog.String("**********"); StdLog.Ln;
		StdLog.String("*  " + "Rongelap"); StdLog.Ln;
		StdLog.String("**********"); StdLog.Ln;
		BugsCmds.SetFilePath("GeoExamples/" + "Rongelap" + "model");
		BugsCmds.Parse;
		BugsCmds.SetFilePath("GeoExamples/" + "Rongelap" + "data");
		BugsCmds.LoadData;
		BugsCmds.specificationDialog.numChains := 2;
		BugsCmds.Compile;
		BugsCmds.SetFilePath("GeoExamples/" + "Rongelap" + "inits1");
		BugsCmds.LoadInits;
		BugsCmds.SetFilePath("GeoExamples/" + "Rongelap" + "inits2");
		BugsCmds.LoadInits;
		BugsCmds.GenerateInitsGuardWin(par);
		IF ~par.disabled THEN BugsCmds.GenerateInits END;
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('beta'); SamplesCmds.Set;
		SamplesCmds.SetVariable('phi'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma'); SamplesCmds.Set;
		SamplesCmds.SetVariable('max.contamination'); SamplesCmds.Set;
		SummaryCmds.SetVariable('pred'); SummaryCmds.Set;
		SummaryCmds.SetVariable('prob.exceeds.15'); SummaryCmds.Set;
		BugsCmds.updateDialog.updates := 4000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		MapsCmds.SetMap("Rongelap");
		MapsCmds.SetPalette(MapsCmds.blues);
		MapsCmds.SetQuantity("pred");
		MapsCmds.SetType(MapsCmds.summaryMean);
		MapsCmds.SetDefaultCuts;
		StdLog.Ln;
		MapsCmds.Plot;
		StdLog.Ln;
		MapsCmds.SetPalette(MapsCmds.blues);
		MapsCmds.SetQuantity("prob.exceeds.15");
		MapsCmds.SetType(MapsCmds.summaryMean);
		MapsCmds.SetDefaultCuts;
		StdLog.Ln;
		MapsCmds.Plot;
		StdLog.Ln;
	END Rongelap;

	PROCEDURE Scotland*;
	BEGIN
		CompileGeo("Scotland");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('alpha0'); SamplesCmds.Set;
		SamplesCmds.SetVariable('alpha1'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma'); SamplesCmds.Set;
		SummaryCmds.SetVariable('RR'); SummaryCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		MapsCmds.SetMap("Scotland");
		MapsCmds.SetPalette(MapsCmds.blues);
		MapsCmds.SetQuantity("RR");
		MapsCmds.SetType(MapsCmds.summaryMean);
		MapsCmds.SetDefaultCuts;
		StdLog.Ln;
		MapsCmds.Plot;
		StdLog.Ln
	END Scotland;

	PROCEDURE Scotland1*;
	BEGIN
		CompileGeo("Scotland1");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('alpha'); SamplesCmds.Set;
		SamplesCmds.SetVariable('gamma'); SamplesCmds.Set;
		SamplesCmds.SetVariable('sigma'); SamplesCmds.Set;
		SummaryCmds.SetVariable('RR'); SummaryCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		MapsCmds.SetMap("Scotland");
		MapsCmds.SetPalette(MapsCmds.blues);
		MapsCmds.SetQuantity("RR");
		MapsCmds.SetType(MapsCmds.summaryMean);
		MapsCmds.SetDefaultCuts;
		StdLog.Ln;
		MapsCmds.Plot;
		StdLog.Ln
	END Scotland1;

	PROCEDURE Shared*;
	BEGIN
		CompileGeo("Shared");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('RR.ratio'); SamplesCmds.Set;
		SamplesCmds.SetVariable('delta'); SamplesCmds.Set;
		SamplesCmds.SetVariable('frac.shared'); SamplesCmds.Set;
		SamplesCmds.SetVariable('var.shared'); SamplesCmds.Set;
		SamplesCmds.SetVariable('var.specific'); SamplesCmds.Set;
		SummaryCmds.SetVariable('specificRR1'); SummaryCmds.Set;
		SummaryCmds.SetVariable('specificRR2'); SummaryCmds.Set;
		SummaryCmds.SetVariable('sharedRR'); SummaryCmds.Set;
		BugsCmds.updateDialog.updates := 10000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		MapsCmds.SetMap("WestYorkshire");
		MapsCmds.SetPalette(MapsCmds.blues);
		MapsCmds.SetQuantity("specificRR1");
		MapsCmds.SetType(MapsCmds.summaryMean);
		MapsCmds.SetDefaultCuts;
		StdLog.Ln;
		MapsCmds.Plot;
		StdLog.Ln;
		MapsCmds.SetQuantity("specificRR2");
		MapsCmds.SetType(MapsCmds.summaryMean);
		MapsCmds.SetDefaultCuts;
		StdLog.Ln;
		MapsCmds.Plot;
		StdLog.Ln;
		MapsCmds.SetQuantity("sharedRR");
		MapsCmds.SetType(MapsCmds.summaryMean);
		MapsCmds.SetDefaultCuts;
		StdLog.Ln;
		MapsCmds.Plot;
		StdLog.Ln
	END Shared;

	(************************************************************************************************************************)

	PROCEDURE Eco2Agg*;
	BEGIN
		CompileEco("Eco2Agg");
		BugsCmds.updateDialog.updates := 4000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('base.mu'); SamplesCmds.Set;
		SamplesCmds.SetVariable('base.sig'); SamplesCmds.Set;
		SamplesCmds.SetVariable('or.carstairs'); SamplesCmds.Set;
		SamplesCmds.SetVariable('or.lincome'); SamplesCmds.Set;
		SamplesCmds.SetVariable('or.nonwhite'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 26000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		SamplesDiagnostics.Install; SamplesCmds.Plot;
		StdLog.Ln;
	END Eco2Agg;

	PROCEDURE Eco2Indiv*;
	BEGIN
		CompileEco("Eco2Indiv");
		BugsCmds.updateDialog.updates := 1000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('base.mu'); SamplesCmds.Set;
		SamplesCmds.SetVariable('base.sig'); SamplesCmds.Set;
		SamplesCmds.SetVariable('or.carstairs'); SamplesCmds.Set;
		SamplesCmds.SetVariable('or.lincome'); SamplesCmds.Set;
		SamplesCmds.SetVariable('or.nonwhite'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 29000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		SamplesDiagnostics.Install; SamplesCmds.Plot;
		StdLog.Ln;
	END Eco2Indiv;

	PROCEDURE Eco2AggIndiv*;
	BEGIN
		CompileEco("Eco2AggIndiv");
		BugsCmds.updateDialog.updates := 4000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('base.mu'); SamplesCmds.Set;
		SamplesCmds.SetVariable('base.sig'); SamplesCmds.Set;
		SamplesCmds.SetVariable('or.carstairs'); SamplesCmds.Set;
		SamplesCmds.SetVariable('or.lincome'); SamplesCmds.Set;
		SamplesCmds.SetVariable('or.nonwhite'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 26000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		SamplesDiagnostics.Install; SamplesCmds.Plot;
		StdLog.Ln;
	END Eco2AggIndiv;

	PROCEDURE Eco3AggIndiv*;
	BEGIN
		CompileEco("Eco3AggIndiv");
		BugsCmds.updateDialog.updates := 20000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('mumu'); SamplesCmds.Set;
		SamplesCmds.SetVariable('musig'); SamplesCmds.Set;
		SamplesCmds.SetVariable('rr.c.carstairs'); SamplesCmds.Set;
		SamplesCmds.SetVariable('rr.nocar'); SamplesCmds.Set;
		SamplesCmds.SetVariable('rr.nonwhite'); SamplesCmds.Set;
		SamplesCmds.SetVariable('rr.soclass'); SamplesCmds.Set;
		BugsCmds.updateDialog.updates := 50000;
		BugsCmds.UpdateNI;
		SamplesCmds.SetVariable('*'); SamplesCmds.Stats;
		StdLog.Ln;
		SamplesDiagnostics.Install; SamplesCmds.Plot;
		StdLog.Ln;
	END Eco3AggIndiv;

	(***************************************************************************************************************************)

	PROCEDURE AllModels*;
		VAR
			startTime, timeInterval: LONGINT;
			oldWhereOut: INTEGER;
	BEGIN
		startTime := Services.Ticks();
		oldWhereOut := BugsFiles.whereOut;
		StdLog.Clear;
		Air; Aligators; Asia; Beetles;
		Biopsies; BiRats; Blockers; Bones;
		Camel; Cervix; Dogs; Dugongs;
		Dyes; Endo; Epil; Equiv;
		Eyes; Eyetracking; Hearts; Hepatitis; HepatitisME;
		Ice; Inhalers; Jama; Jaws;
		Kidney; Leuk; Leukfr; Line;
		Lsat; Mice; Otrees; OtreesMVN;
		Oxford; Pigs; Pigweights; Rats;
		Salm; Schools; Seeds; Sixcomp;
		Stacks; Stagnantc; StVeit; Surgical;
		Surgicalrand;
		timeInterval := Services.Ticks() - startTime;
		StdLog.String("******************************"); StdLog.Ln;
		StdLog.String("Tests took "); StdLog.Int(timeInterval DIV Services.resolution);
		StdLog.String("s"); StdLog.Ln;
		BugsCmds.SaveLog("Test/AllModels");
		BugsFiles.SetDest(oldWhereOut)
	END AllModels;

	PROCEDURE AllGeo*;
		VAR
			startTime, timeInterval: LONGINT;
			oldWhereOut: INTEGER;
	BEGIN
		startTime := Services.Ticks();
		oldWhereOut := BugsFiles.whereOut;
		StdLog.Clear;
		Elevation; Forest; LHA; Huddersfield;
		MVCAR; MVCARCon; Pollution; Rongelap;
		Scotland; Scotland1; Shared;
		timeInterval := Services.Ticks() - startTime;
		StdLog.String("******************************"); StdLog.Ln;
		StdLog.String("Tests took "); StdLog.Int(timeInterval DIV Services.resolution);
		StdLog.String("s"); StdLog.Ln;
		BugsCmds.SaveLog("Test/GeoModels");
		BugsFiles.SetDest(oldWhereOut)
	END AllGeo;

	PROCEDURE AllEco*;
		VAR
			startTime, timeInterval: LONGINT;
			oldWhereOut: INTEGER;
	BEGIN
		startTime := Services.Ticks();
		oldWhereOut := BugsFiles.whereOut;
		StdLog.Clear;
		Eco2Agg; Eco2Indiv; Eco2AggIndiv; Eco3AggIndiv;
		timeInterval := Services.Ticks() - startTime;
		StdLog.String("******************************"); StdLog.Ln;
		StdLog.String("Tests took "); StdLog.Int(timeInterval DIV Services.resolution);
		StdLog.String("s"); StdLog.Ln;
		BugsCmds.SaveLog("Test/EcoModels");
		BugsFiles.SetDest(oldWhereOut)
	END AllEco;

	PROCEDURE EveryThing*;
	BEGIN
		AllModels;
		AllGeo;
		AllEco
	END EveryThing;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END TestScript.


(***************************************************************************************************************************)


TestScript.AllModels	TestScript.AllGeo	TestScript.AllEco



TestScript.EveryThing





(***************************************************************************************************************************)


TestScript.Air	TestScript.Aligators	TestScript.Asia

TestScript.Beetles	TestScript.Biopsies	TestScript.BiRats

TestScript.Blockers	TestScript.Bones	TestScript.Camel

TestScript.Cervix	TestScript.Dogs	TestScript.Dugongs

TestScript.Dyes	TestScript.Endo	TestScript.Epil

TestScript.Equiv	TestScript.Eyetracking	TestScript.Eyes

TestScript.Hearts	TestScript.Hepatitis	TestScript.HepatitisME

TestScript.Ice	TestScript.Inhalers	TestScript.Jama

TestScript.Jaws	TestScript.Kidney	TestScript.Leuk

TestScript.Leukfr	TestScript.Line	TestScript.Lsat

TestScript.Mice	TestScript.Otrees	TestScript.OtreesMVN

TestScript.Oxford	TestScript.Pigs	TestScript.Pigweights

TestScript.Rats	TestScript.Salm	TestScript.Schools

TestScript.Seeds	TestScript.Sixcomp	TestScript.Stacks

TestScript.Stagnantc	TestScript.StVeit	TestScript.Surgical

TestScript.Surgicalrand


(***************************************************************************************************************************)


TestScript.Elevation	TestScript.Forest	TestScript.Huddersfield

TestScript.LHA	TestScript.MVCAR	TestScript.MVCARCon

TestScript.Pollution	TestScript.Rongelap	TestScript.Scotland

TestScript.Scotland1	TestScript.Shared


(***************************************************************************************************************************)


TestScript.Eco2Agg	TestScript.Eco2Indiv	TestScript.Eco2AggIndiv

TestScript.Eco3AggIndiv
