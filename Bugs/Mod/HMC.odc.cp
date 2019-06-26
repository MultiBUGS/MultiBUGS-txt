(* 	

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsHMC;

	

	IMPORT
		Dialog, Services, Strings,
		BugsCmds, BugsDialog, BugsFiles, BugsInterface, BugsMsg, BugsRandnum,
		MathRandnum,
		MonitorMonitors,
		UpdaterActions, UpdaterHMC;

	TYPE
		Action = POINTER TO RECORD (Services.Action)
			updating: BOOLEAN;
			numReject, startIt, updates: INTEGER
		END;

		UpdateDialog* = POINTER TO RECORD(BugsDialog.DialogBox)
			iteration-: INTEGER; 	(*	current iteration number	*)
			refresh*: INTEGER; 	(*	return control to framework after resresh updates	*)
			updates*: INTEGER; 	(*	number of MCMC updates	*)
			numSteps*: INTEGER; 	(*	number of leapfrog steps	*)
			warmUpPeriod*: INTEGER; 	(*	warmup period	*)
			eps*: REAL; (*	step length	*)
		END;

	VAR
		startTime: LONGINT;
		action: Action;
		updateDialog*: UpdateDialog;

		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (a: Action) Do;
		VAR
			i, chain, numAccept, numChains, numSteps, refresh, updates, warmUpPeriod: INTEGER;
			eps: REAL;
			reject: BOOLEAN;
			elapsedTime: LONGINT;
			p: ARRAY 3 OF ARRAY 1024 OF CHAR;
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		IF ~a.updating THEN RETURN END;
		refresh := updateDialog.refresh;
		updates := updateDialog.updates;
		numChains := BugsCmds.specificationDialog.numChains;
		numSteps := updateDialog.numSteps;
		warmUpPeriod := updateDialog.warmUpPeriod;
		eps := updateDialog.eps;
		i := 0;
		WHILE (i < refresh) & (a.updates < updates) DO
			INC(i);
			INC(a.updates);
			chain := 0;
			WHILE chain < numChains DO
				MathRandnum.SetGenerator(BugsRandnum.generators[chain]);
				UpdaterActions.LoadSamples(chain);
				UpdaterHMC.Update(numSteps, updateDialog.iteration + i, warmUpPeriod, chain, eps, reject);
				IF reject THEN INC(a.numReject) END;
				UpdaterActions.StoreSamples(chain);
				INC(chain)
			END;
			BugsInterface.UpdateMonitors(numChains)
		END;
		MathRandnum.SetGenerator(BugsRandnum.generators[0]);
		UpdaterActions.SetAdaption(updateDialog.iteration, UpdaterActions.endOfAdapting);
		INC(updateDialog.iteration, i);
		MonitorMonitors.UpdateDrawers;
		Dialog.Update(updateDialog);
		IF a.updates < updates THEN
			Services.DoLater(a, Services.now)
		ELSE
			BugsInterface.RecvMCMCState;
			BugsInterface.LoadDeviance(0);
			a.updating := FALSE;
			numAccept := numChains * (updateDialog.iteration - a.startIt) - a.numReject;
			numAccept := numAccept DIV numChains;
			elapsedTime := Services.Ticks() - startTime;
			Strings.IntToString(updates, p[0]);
			Strings.IntToString(elapsedTime DIV Services.resolution, p[1]);
			Strings.IntToString(elapsedTime MOD Services.resolution, msg);
			p[1] := p[1] + "." + msg;
			Strings.IntToString(100 * numAccept DIV (updateDialog.iteration - a.startIt), p[2]);
			BugsMsg.LookupParam("BugsCmds:UpdatesTookHMC", p, msg);
			BugsFiles.ShowStatus(msg)
		END
	END Do;

	PROCEDURE Update*;
		VAR
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		IF ~UpdaterHMC.setUp THEN
			UpdaterActions.SetAdaption(updateDialog.iteration,
			updateDialog.warmUpPeriod + updateDialog.iteration + 1);
		END;
		UpdaterHMC.Setup(BugsCmds.specificationDialog.numChains);
		IF ~action.updating THEN
			action.updating := TRUE;
			startTime := Services.Ticks();
			BugsMsg.Lookup("BugsCmds:Updating", msg);
			BugsFiles.ShowStatus(msg);
			action.updates := 0;
			action.startIt := updateDialog.iteration;
			action.numReject := 0;
			Services.DoLater(action, Services.now)
		ELSE
			action.updating := FALSE
		END;
		Dialog.Update(updateDialog)
	END Update;


	PROCEDURE InitDialog;
	BEGIN
		updateDialog.iteration := 0;
		updateDialog.updates := 1000;
		updateDialog.refresh := 100;
		updateDialog.numSteps := 8;
		updateDialog.warmUpPeriod := 1000;
		updateDialog.eps := 0.25;
		UpdaterHMC.Clear
	END InitDialog;

	PROCEDURE (dialogBox: UpdateDialog) Update-;
	BEGIN
		Dialog.Update(dialogBox)
	END Update;

	PROCEDURE (dialogBox: UpdateDialog) Init-;
	BEGIN
		InitDialog
	END Init;

	PROCEDURE WarmUpGuard* (VAR par: Dialog.Par);
	BEGIN
		par.readOnly := UpdaterHMC.setUp
	END WarmUpGuard;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		NEW(action);
		NEW(updateDialog);
		InitDialog;
		BugsDialog.AddDialog(updateDialog);
		BugsDialog.UpdateDialogs
	END Init;

BEGIN
	Init
END BugsHMC.
