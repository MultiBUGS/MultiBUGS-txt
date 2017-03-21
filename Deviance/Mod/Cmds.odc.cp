(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE DevianceCmds;


	

	IMPORT
		Dialog, Ports,
		BugsDialog, BugsGraph, BugsIndex, BugsInterface, BugsMappers, BugsMsg, BugsTexts,
		DevianceFormatted, DevianceIndex, DevianceInterface, DevianceMonitors, DeviancePlugin,
		DeviancePluginD, DeviancePluginS,
		TextModels;


	TYPE
		DialogBox* = POINTER TO RECORD(BugsDialog.DialogBox)
			parents*: INTEGER
		END;

	VAR
		dialog*: DialogBox; (* dialog for node monitoring *)

		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	CONST
		direct* = 0;
		stochastic* = 1;
		non* = 2;

	PROCEDURE Notifier;
	BEGIN
		CASE dialog.parents OF
		|direct:
			DeviancePluginD.Install
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
		plugin := DevianceIndex.Plugin();
		IF plugin = NIL THEN
			dialog.parents := direct;
		ELSE
			dialog.parents := plugin.Type();
		END;
		Notifier
	END SetFactory;

	PROCEDURE Clear*;
	BEGIN
		DevianceInterface.Clear;
		dialog.parents := direct;
		DeviancePluginD.Install;
	END Clear;

	PROCEDURE Set*;
	BEGIN
		DevianceInterface.Set;
	END Set;

	PROCEDURE Stats*;
		VAR
			i, numTabs: INTEGER;
			tabs: POINTER TO ARRAY OF INTEGER;
			f: BugsMappers.Formatter;
			t: TextModels.Model;
	BEGIN
		t := TextModels.dir.New();
		BugsTexts.ConnectFormatter(f, t);
		f.SetPos(0);
		IF ~BugsInterface.IsDistributed() THEN
			IF DevianceInterface.PluginSet() THEN numTabs := 7 ELSE numTabs := 4 END;
			NEW(tabs, numTabs);
			i := 0;
			WHILE i < numTabs DO
				tabs[i] := 20 * Ports.mm * (i + 1);
				INC(i)
			END;
			f.WriteRuler(tabs);
			DevianceFormatted.Stats(f)
		ELSE
			numTabs := 3;
			NEW(tabs, numTabs);
			i := 0;
			WHILE i < numTabs DO
				tabs[i] := 20 * Ports.mm * (i + 1);
				INC(i)
			END;
			DevianceFormatted.DistributedWAIC(f)
		END;
		f.Register("Information Criterion")
	END Stats;

	PROCEDURE (dialog: DialogBox) Init-;
	BEGIN
		SetFactory;
	END Init;

	PROCEDURE (dialog: DialogBox) Update-;
	BEGIN
		SetFactory;
	END Update;

	PROCEDURE SetGuard* (OUT ok: BOOLEAN);
		VAR
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		ok := BugsIndex.Find("deviance") # NIL;
		IF ~ok THEN
			BugsMsg.MapMsg("DevianceEmbed:NoDeviance", msg);
			BugsTexts.ShowMsg(msg);
			RETURN
		END;
		ok := BugsInterface.IsInitialized();
		IF ~ok THEN
			BugsMsg.MapMsg("DevianceEmbed:NotInitialized", msg);
			BugsTexts.ShowMsg(msg);
			RETURN
		END;
		ok := ~BugsInterface.IsAdapting();
		IF ~ok THEN
			BugsMsg.MapMsg("DevianceEmbed:Adapting", msg);
			BugsTexts.ShowMsg(msg);
			RETURN
		END;
		ok := DevianceIndex.Plugin() = NIL;
		IF ~ok THEN
			BugsMsg.MapMsg("DevianceEmbed:AlreadyMonitored", msg);
			BugsTexts.ShowMsg(msg);
			RETURN
		END
	END SetGuard;

	PROCEDURE StatsGuard* (OUT ok: BOOLEAN);
		VAR
			variable: Dialog.String;
			msg: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		ok := DevianceInterface.IsUpdated();
		IF ~ok THEN
			BugsMsg.MapMsg("DevianceEmbed:NoIterations", msg);
			BugsTexts.ShowMsg(msg);
			RETURN
		END
	END StatsGuard;

	PROCEDURE SetGuardWin* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := FALSE;
		IF BugsIndex.Find("deviance") = NIL THEN
			par.disabled := TRUE;
			RETURN
		END;
		IF ~BugsInterface.IsInitialized() THEN
			par.disabled := TRUE;
			RETURN
		END;
		IF BugsInterface.IsAdapting() THEN
			par.disabled := TRUE;
			RETURN
		END
	END SetGuardWin;

	PROCEDURE StatsGuardWin* (VAR par: Dialog.Par);
		VAR
			variable: Dialog.String;
	BEGIN
		par.disabled := FALSE;
		IF ~DevianceInterface.IsUpdated() THEN
			par.disabled := TRUE;
			RETURN
		END
	END StatsGuardWin;

	PROCEDURE ParentsGuard* (VAR par: Dialog.Par);
	BEGIN
		par.readOnly := DevianceIndex.Plugin() # NIL
	END ParentsGuard;

	PROCEDURE ParentsNotifier* (op, from, to: INTEGER);
	BEGIN
		IF op = Dialog.changed THEN Notifier END
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
