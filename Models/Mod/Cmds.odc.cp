(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE ModelsCmds;

	

	IMPORT
		Dialog, Ports, Strings,
		BugsDialog,
		BugsGraph, BugsIndex, BugsInterface, BugsMappers, BugsMsg, BugsTexts, ModelsFormatted,
		ModelsIndex, ModelsInterface, ModelsMonitors, TextModels;

	TYPE

		DialogBox* = POINTER TO RECORD(BugsDialog.DialogBox)	(* dialog for node monitoring *)
			node*: Dialog.Combo;
			cumulative*, minProb*: REAL;
			maxNum*: INTEGER
		END;

	VAR
		dialog*: DialogBox;

		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE UpdateNames*;
		VAR
			i, num: INTEGER;
			monitors: POINTER TO ARRAY OF ModelsMonitors.Monitor;
	BEGIN
		monitors := ModelsIndex.GetMonitors();
		IF monitors # NIL THEN
			num := LEN(monitors)
		ELSE
			num := 0
		END;
		i := 0;
		WHILE i < num DO
			dialog.node.SetItem(i, monitors[i].Name().string);
			INC(i)
		END;
		dialog.node.SetLen(num);
		dialog.node.item := ""
	END UpdateNames;

	PROCEDURE Clear*;
		VAR
			ok: BOOLEAN;
			res: INTEGER;
			errorMes: ARRAY 1024 OF CHAR;
			name: ARRAY 128 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		name := dialog.node.item$;
		IF ModelsInterface.IsStar(name) THEN
			Dialog.GetOK("This will delete all set monitors", "", "", "", {Dialog.ok, Dialog.cancel}, res);
			IF res # Dialog.ok THEN RETURN END
		END;
		ModelsInterface.Clear(name, ok);
		IF ~ok THEN
			p[0] := name$;
			BugsMsg.GetError(errorMes);
			BugsMsg.ParamMsg(errorMes, p, errorMes);
			BugsTexts.ShowMsg(errorMes);
			RETURN
		END;
		UpdateNames;
		Dialog.UpdateList(dialog.node);
		Dialog.Update(dialog)
	END Clear;

	PROCEDURE ClearNI*;
		VAR
			ok: BOOLEAN;
			errorMes: ARRAY 1024 OF CHAR;
			name: ARRAY 128 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		name := dialog.node.item$;
		ModelsInterface.Clear(name, ok);
		IF ~ok THEN
			p[0] := name$;
			BugsMsg.GetError(errorMes);
			BugsMsg.ParamMsg(errorMes, p, errorMes);
			BugsTexts.ShowMsg(errorMes);
			RETURN
		END;
		UpdateNames;
		Dialog.UpdateList(dialog.node);
		Dialog.Update(dialog)
	END ClearNI;

	PROCEDURE Set*;
		VAR
			ok: BOOLEAN;
			errorMes: ARRAY 1024 OF CHAR;
			name: ARRAY 128 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		name := dialog.node.item$;
		ModelsInterface.Set(name, ok);
		IF ~ok THEN
			p[0] := name$;
			BugsMsg.GetError(errorMes);
			BugsMsg.ParamMsg(errorMes, p, errorMes);
			BugsTexts.ShowMsg(errorMes);
			RETURN
		END;
		UpdateNames;
		Dialog.UpdateList(dialog.node);
		Dialog.Update(dialog)
	END Set;

	PROCEDURE ComponentProbs*;
		VAR
			numTabs: INTEGER;
			tabs: POINTER TO ARRAY OF INTEGER;
			f: BugsMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		text := TextModels.dir.New();
		BugsTexts.ConnectFormatter(f, text);
		f.SetPos(0);
		numTabs := 3;
		NEW(tabs, numTabs);
		tabs[0] := 0;
		tabs[1] := 30 * Ports.mm;
		tabs[2] := tabs[1] + 35 * Ports.mm;
		f.WriteRuler(tabs);
		ModelsFormatted.ComponentProbs(dialog.node.item, f);
		f.Register("Model component statistics")
	END ComponentProbs;

	PROCEDURE ModelProbs*;
		VAR
			i, numTabs: INTEGER;
			tabs: POINTER TO ARRAY OF INTEGER;
			f: BugsMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		text := TextModels.dir.New();
		BugsTexts.ConnectFormatter(f, text);
		f.SetPos(0);
		numTabs := 5;
		NEW(tabs, numTabs);
		tabs[0] := 0;
		tabs[1] := 20 * Ports.mm;
		tabs[2] := tabs[1] + 30 * Ports.mm;
		tabs[3] := tabs[2] + 30 * Ports.mm;
		tabs[4] := tabs[3] + 50 * Ports.mm;
		f.WriteRuler(tabs);
		ModelsFormatted.ModelProbs(dialog.node.item, dialog.cumulative, dialog.minProb, dialog.maxNum, f);
		f.Register("Model probabilities statistics")
	END ModelProbs;

	PROCEDURE SetVariable* (var: ARRAY OF CHAR);
	BEGIN
		dialog.node.item := var$
	END SetVariable;

	(*	Guards	*)

	PROCEDURE SetGuard* (OUT ok: BOOLEAN);
		VAR
			msg: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
			variable: Dialog.String;
	BEGIN
		ok := BugsInterface.IsInitialized();
		IF ~ok THEN
			BugsMsg.MapMsg("ModelsCmds:NotInitialized", msg);
			BugsTexts.ShowMsg(msg)
		ELSE
			variable := dialog.node.item;
			ok := ~BugsInterface.IsAdapting();
			IF ~ok THEN
				BugsMsg.MapMsg("ModelsCmds:Adapting", msg);
				BugsTexts.ShowMsg(msg)
			ELSE
				ok := BugsIndex.Find(variable) # NIL;
				IF ~ok THEN
					p[0] := variable$;
					BugsMsg.MapParamMsg("ModelsCmds:NotVariable", p, msg);
					BugsTexts.ShowMsg(msg)
				ELSE
					ok := BugsIndex.Find(variable).numSlots = 1;
					IF ~ok THEN
						p[0] := variable$;
						BugsMsg.MapParamMsg("ModelsCmds:NotVector", p, msg);
						BugsTexts.ShowMsg(msg)
					ELSE
						ok := ModelsIndex.Find(variable) = NIL;
						IF ~ok THEN
							p[0] := variable$;
							BugsMsg.MapParamMsg("ModelsCmds:AlreadySet", p, msg);
							BugsTexts.ShowMsg(msg)
						END
					END
				END
			END
		END
	END SetGuard;

	PROCEDURE StatsGuard* (OUT ok: BOOLEAN);
		VAR
			numMonitors: INTEGER;
			variable: Dialog.String;
			msg: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		ok := BugsInterface.IsInitialized();
		IF ~ok THEN
			BugsMsg.MapMsg("ModelsCmds:NotInitialized", msg);
			BugsTexts.ShowMsg(msg)
		ELSE
			variable := dialog.node.item;
			IF~ModelsInterface.IsStar(variable) THEN
				ok := ModelsIndex.Find(variable) # NIL;
				IF ~ok THEN
					p[0] := variable$;
					BugsMsg.MapParamMsg("ModelsCmds:NotSet", p, msg);
					BugsTexts.ShowMsg(msg)
				END
			ELSE
				numMonitors := ModelsIndex.NumberOfMonitors();
				ok := numMonitors # 0; ;
				IF ~ok THEN
					BugsMsg.MapMsg("ModelsCmds:NoMonitors", msg);
					BugsTexts.ShowMsg(msg)
				END
			END
		END
	END StatsGuard;

	PROCEDURE SetGuardWin* (VAR par: Dialog.Par);
		VAR
			variable: Dialog.String;
	BEGIN
		par.disabled := FALSE;
		IF ~BugsInterface.IsInitialized() THEN
			par.disabled := TRUE
		ELSE
			IF BugsInterface.IsAdapting() THEN
				par.disabled := TRUE
			ELSE
				variable := dialog.node.item;
				IF BugsIndex.Find(variable) = NIL THEN
					par.disabled := TRUE
				ELSE
					IF BugsIndex.Find(variable).numSlots # 1 THEN
						par.disabled := TRUE
					ELSE
						IF ModelsIndex.Find(variable) # NIL THEN
							par.disabled := TRUE
						END
					END
				END
			END
		END
	END SetGuardWin;

	PROCEDURE StatsGuardWin* (VAR par: Dialog.Par);
		VAR
			numMonitors: INTEGER;
			variable: Dialog.String;
	BEGIN
		par.disabled := FALSE;
		IF ~BugsInterface.IsInitialized() THEN
			par.disabled := TRUE
		ELSE
			variable := dialog.node.item;
			IF~ModelsInterface.IsStar(variable) THEN
				IF ModelsIndex.Find(variable) = NIL THEN
					par.disabled := TRUE
				END
			ELSE
				numMonitors := ModelsIndex.NumberOfMonitors();
				IF numMonitors = 0 THEN
					par.disabled := TRUE
				END
			END
		END
	END StatsGuardWin;

	PROCEDURE InitDialog;
	BEGIN
		dialog.cumulative := 1.0;
		dialog.minProb := 0.0;
		dialog.maxNum := 1000000;
	END InitDialog;

	PROCEDURE (dialogBox0: DialogBox) Init-;
	BEGIN
		dialog.node.item := "";
		dialog.node.SetLen(1);
		dialog.node.SetItem(0, " ");
		InitDialog
	END Init;

	PROCEDURE (dialogBox0: DialogBox) Update-;
	BEGIN
		UpdateNames;
		Dialog.UpdateList(dialog.node);
		Dialog.Update(dialog)
	END Update;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		NEW(dialog);
		InitDialog;
		BugsDialog.AddDialog(dialog);
		BugsDialog.UpdateDialogs
	END Init;

BEGIN
	Init
END ModelsCmds.
