(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE ModelsCmds;

	

	IMPORT
		Dialog, Ports, 
		BugsCmds, BugsDialog, BugsFiles, BugsIndex, BugsInterface, BugsMsg, ModelsFormatted,
		ModelsIndex, ModelsInterface, ModelsMonitors, 
		TextMappers, TextModels;

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
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		name := dialog.node.item$;
		IF ModelsInterface.IsStar(name) THEN
			Dialog.GetOK("This will delete all set monitors", "", "", "", {Dialog.ok, Dialog.cancel}, res);
			IF res # Dialog.ok THEN RETURN END
		END;
		ModelsInterface.Clear(name, ok);
		IF ~ok THEN
			p[0] := name$;
			errorMes := BugsMsg.message$;
			BugsMsg.LookupParam(errorMes, p, msg);
			BugsFiles.ShowStatus(msg);
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
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		name := dialog.node.item$;
		ModelsInterface.Clear(name, ok);
		IF ~ok THEN
			p[0] := name$;
			errorMes := BugsMsg.message$;
			BugsMsg.LookupParam(errorMes, p, msg);
			BugsFiles.ShowStatus(msg);
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
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		name := dialog.node.item$;
		ModelsInterface.Set(name, ok);
		IF ~ok THEN
			p[0] := name$;
			errorMes := BugsMsg.message$;
			BugsMsg.LookupParam(errorMes, p, msg);
			BugsFiles.ShowStatus(msg);
			RETURN
		END;
		UpdateNames;
		Dialog.UpdateList(dialog.node);
		Dialog.Update(dialog)
	END Set;

	PROCEDURE ComponentProbs*;
		VAR
			numTabs, pos: INTEGER;
			tabs: POINTER TO ARRAY OF INTEGER;
			f: TextMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		numTabs := 3;
		NEW(tabs, numTabs);
		tabs[0] := 0;
		tabs[1] := 30 * Ports.mm;
		tabs[2] := tabs[1] + 40 * Ports.mm;
		BugsFiles.WriteRuler(tabs, f);
		pos := f.Pos();
		ModelsFormatted.ComponentProbs(dialog.node.item, f);
		IF f.Pos() > pos THEN BugsFiles.Open("Model component statistics", text) END
	END ComponentProbs;

	PROCEDURE ModelProbs*;
		VAR
			numTabs, pos: INTEGER;
			tabs: POINTER TO ARRAY OF INTEGER;
			f: TextMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		numTabs := 5;
		NEW(tabs, numTabs);
		tabs[0] := 0;
		tabs[1] := 20 * Ports.mm;
		tabs[2] := tabs[1] + 30 * Ports.mm;
		tabs[3] := tabs[2] + 30 * Ports.mm;
		tabs[4] := tabs[3] + 50 * Ports.mm;
		BugsFiles.WriteRuler(tabs, f);
		pos := f.Pos();
		ModelsFormatted.ModelProbs(dialog.node.item, dialog.cumulative, dialog.minProb, dialog.maxNum, f);
		IF f.Pos() > pos THEN BugsFiles.Open("Model probabilities statistics", text) END
	END ModelProbs;

	PROCEDURE SetVariable* (var: ARRAY OF CHAR);
	BEGIN
		dialog.node.item := var$
	END SetVariable;

	(*	Guards	*)

	PROCEDURE SetGuard* (VAR par: Dialog.Par);
		VAR
			variable: Dialog.String;
			p: ARRAY 1 OF Dialog.String;
	BEGIN
		par.disabled := FALSE;
		IF ~BugsInterface.IsInitialized() THEN 
			par.disabled := TRUE;
			IF BugsCmds.script THEN BugsMsg.Lookup("ModelsCmds:NotInitialized", par.label) END
		ELSE
			IF BugsInterface.IsAdapting() THEN
				par.disabled := TRUE;
				IF BugsCmds.script THEN BugsMsg.Lookup("ModelsCmds:Adapting", par.label) END
			ELSE
				variable := dialog.node.item;
				IF BugsIndex.Find(variable) = NIL THEN
					par.disabled := TRUE;
					IF BugsCmds.script THEN 
						p[0] := variable$;
						BugsMsg.LookupParam("ModelsCmds:NotVariable", p, par.label)
					END
				ELSE
					IF BugsIndex.Find(variable).numSlots # 1 THEN
						par.disabled := TRUE;
						IF BugsCmds.script THEN
							p[0] := variable$;
							BugsMsg.LookupParam("ModelsCmds:NotVector", p, par.label)
						END
					ELSE
						IF ModelsIndex.Find(variable) # NIL THEN
							par.disabled := TRUE;
							par.disabled := TRUE;
							IF BugsCmds.script THEN
								p[0] := variable$;
								BugsMsg.LookupParam("ModelsCmds:AlreadySet", p, par.label)
							END
						END
					END
				END
			END
		END
	END SetGuard;

	PROCEDURE StatsGuard* (VAR par: Dialog.Par);
		VAR
			numMonitors: INTEGER;
			variable: Dialog.String;
			p: ARRAY 1 OF Dialog.String;
	BEGIN
		par.disabled := FALSE;
		IF ~BugsInterface.IsInitialized() THEN
			par.disabled := TRUE;
			IF BugsCmds.script THEN BugsMsg.Lookup("ModelsCmds:NotInitialized", par.label) END
		ELSE
			variable := dialog.node.item;
			IF~ModelsInterface.IsStar(variable) THEN
				IF ModelsIndex.Find(variable) = NIL THEN
					par.disabled := TRUE;
					IF BugsCmds.script THEN
						p[0] := variable$;
						BugsMsg.LookupParam("ModelsCmds:NotSet", p, par.label)
					END
				END
			ELSE
				numMonitors := ModelsIndex.NumberOfMonitors();
				IF numMonitors = 0 THEN
					par.disabled := TRUE;
					IF BugsCmds.script THEN
						BugsMsg.Lookup("ModelsCmds:NoMonitors", par.label)
					END
				END
			END
		END
	END StatsGuard;

	PROCEDURE InitDialog;
	BEGIN
		dialog.cumulative := 1.0;
		dialog.minProb := 0.0;
		dialog.maxNum := 1000000;
	END InitDialog;

	PROCEDURE (dialogBox0: DialogBox) Init-;
	BEGIN
		ModelsInterface.ClearNI("*");
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
