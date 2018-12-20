(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE DevianceCmds;


	

	IMPORT
		Dialog, Ports,
		TextMappers, TextModels, BugsDialog, BugsFiles, BugsIndex,
		BugsCmds, BugsInterface, BugsMsg, 
		DevianceFormatted, DevianceIndex, DevianceInterface, DeviancePlugin, DeviancePluginS;

	TYPE
		DialogBox* = POINTER TO RECORD(BugsDialog.DialogBox)
			parents*: INTEGER
		END;

	VAR
		dialog*: DialogBox; (* dialog for node monitoring *)

		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	CONST
		stochastic = 1;
		non = 2;

	PROCEDURE Notifier;
	BEGIN
		CASE dialog.parents OF
		|stochastic:
			DeviancePluginS.Install
		|non:
			DeviancePlugin.SetFact(NIL)
		END
	END Notifier;

	PROCEDURE SetFactory;
		VAR
			plugin: DeviancePlugin.Plugin;
	BEGIN
		IF ~BugsInterface.IsDistributed() THEN
			plugin := DevianceIndex.Plugin();
			IF plugin = NIL THEN
				dialog.parents := stochastic;
			ELSE
				dialog.parents := plugin.Type();
			END
		ELSE
			dialog.parents := non
		END;
		Notifier
	END SetFactory;

	PROCEDURE Clear*;
	BEGIN
		DevianceInterface.Clear;
		IF ~BugsInterface.IsDistributed() THEN
			dialog.parents := stochastic;
			DeviancePluginS.Install
		ELSE
			dialog.parents := non
		END;
	END Clear;

	PROCEDURE Set*;
	BEGIN
		DevianceInterface.Set;
	END Set;

	PROCEDURE Stats*;
		VAR
			i, numTabs, pos: INTEGER;
			tabs: POINTER TO ARRAY OF INTEGER;
			f: TextMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		IF ~BugsInterface.IsDistributed() THEN
			IF DevianceInterface.PluginSet() THEN numTabs := 7 ELSE numTabs := 6 END;
			NEW(tabs, numTabs);
			i := 0;
			WHILE i < numTabs DO
				tabs[i] := 20 * Ports.mm * (i + 1);
				INC(i)
			END;
			BugsFiles.WriteRuler(tabs, f);
			pos := f.Pos();
			DevianceFormatted.Stats(f)
		ELSE
			numTabs := 7;
			NEW(tabs, numTabs);
			tabs[0] := Ports.mm;
			i := 1;
			WHILE i < numTabs DO
				tabs[i] := tabs[i - 1] + 20 * Ports.mm;
				INC(i)
			END;
			BugsFiles.WriteRuler(tabs, f);
			pos := f.Pos();
			DevianceFormatted.DistributedStats(f)
		END;
		IF f.Pos() > pos THEN BugsFiles.Open("Information Criterion", text) END
	END Stats;

	PROCEDURE (dialog: DialogBox) Init-;
	BEGIN
		SetFactory;
		Clear
	END Init;

	PROCEDURE (dialog: DialogBox) Update-;
	BEGIN
		SetFactory;
	END Update;

	PROCEDURE SetGuard* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := FALSE;
		IF BugsIndex.Find("deviance") = NIL THEN
			par.disabled := TRUE;
			IF BugsCmds.script THEN BugsMsg.Lookup("DevianceCmds:NoDeviance", par.label) END;
			RETURN
		END;
		IF ~BugsInterface.IsInitialized() THEN
			par.disabled := TRUE;
			IF BugsCmds.script THEN BugsMsg.Lookup("DevianceCmds:NotInitialized", par.label) END;
			RETURN
		END;
		IF BugsInterface.IsAdapting() THEN
			par.disabled := TRUE;
			IF BugsCmds.script THEN BugsMsg.Lookup("DevianceCmds:Adapting", par.label) END;
			IF par.disabled & BugsCmds.script THEN
				BugsMsg.Lookup("DevianceCmds:AlreadyMonitored", par.label)
			END;
			RETURN
		END;
		par.disabled := DevianceInterface.state # DevianceInterface.notSet
	END SetGuard;

	PROCEDURE StatsGuard* (VAR par: Dialog.Par);
		VAR
			variable: Dialog.String;
	BEGIN
		par.disabled := FALSE;
		IF ~DevianceInterface.IsUpdated() THEN
			par.disabled := TRUE;
			IF BugsCmds.script THEN BugsMsg.Lookup("DevianceCmds:NoIterations", par.label) END;
			RETURN
		END
	END StatsGuard;

	PROCEDURE ParentsGuard* (VAR par: Dialog.Par);
	BEGIN
		par.readOnly := DevianceInterface.state # DevianceInterface.notSet;
		IF ~par.readOnly THEN
			par.readOnly := BugsInterface.IsDistributed()
		END
	END ParentsGuard;

	PROCEDURE ParentsNotifier* (op, from, to: INTEGER);
	BEGIN
		IF dialog.parents = stochastic THEN
			DeviancePluginS.Install
		ELSE
			DeviancePlugin.SetFact(NIL)
		END;
		Dialog.Update(dialog)
	END ParentsNotifier;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		NEW(dialog);
		SetFactory;
		BugsDialog.AddDialog(dialog)
	END Init;

BEGIN
	Init
END DevianceCmds.
