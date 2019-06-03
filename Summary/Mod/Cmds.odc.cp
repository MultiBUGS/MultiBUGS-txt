(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)


MODULE SummaryCmds;

	

	IMPORT
		Dialog, Ports,
		TextMappers, TextModels,
		BugsCmds, BugsDialog, BugsFiles, BugsIndex, BugsInterface, BugsMsg, 
		SummaryFormatted, SummaryIndex, SummaryInterface, SummaryMonitors;

	TYPE
		DialogBox* = POINTER TO RECORD(BugsDialog.DialogBox)
			node*: Dialog.Combo;
			options*: Dialog.Selection
		END;

	VAR
		dialog*: DialogBox;

		version-: INTEGER;
		maintainer-: ARRAY 20 OF CHAR;

	PROCEDURE SetVariable* (var: ARRAY OF CHAR);
	BEGIN
		dialog.node.item := var$
	END SetVariable;

	PROCEDURE UpdateNames*;
		VAR
			i, num: INTEGER;
			monitors: POINTER TO ARRAY OF SummaryMonitors.Monitor;
	BEGIN
		monitors := SummaryIndex.GetMonitors();
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
		IF SummaryInterface.IsStar(name) THEN
			BugsMsg.Lookup("SummaryCmds:DeleteAll", msg);
			Dialog.GetOK(msg, "", "", "", {Dialog.ok, Dialog.cancel}, res);
			IF res # Dialog.ok THEN RETURN END
		END;
		SummaryInterface.Clear(name, ok);
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
		SummaryInterface.Clear(name, ok);
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

	PROCEDURE Means*;
		VAR
			f: TextMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		SummaryFormatted.Means(dialog.node.item, f);
		IF f.Pos() > 0 THEN BugsFiles.Open("Means", text) END
	END Means;

	PROCEDURE Set*;
		VAR
			ok: BOOLEAN;
			errorMes: ARRAY 1024 OF CHAR;
			name: ARRAY 128 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		name := dialog.node.item$;
		SummaryInterface.Set(name, ok);
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

	PROCEDURE Options (): SET;
		VAR
			options: SET;
			i, len: INTEGER;
	BEGIN
		len := dialog.options.len;
		options := {};
		i := 0;
		WHILE i < len DO
			IF dialog.options.In(i) THEN
				INCL(options, i);
			END;
			INC(i)
		END;
		RETURN options
	END Options;

	PROCEDURE NumTabs (options: SET): INTEGER;
		VAR
			i, numTabs: INTEGER;
	BEGIN
		numTabs := 5;
		i := 0;
		WHILE i < MAX(SET) DO
			IF i IN options THEN INC(numTabs) END;
			INC(i)
		END;
		RETURN numTabs;
	END NumTabs;

	PROCEDURE Stats*;
		VAR
			tabs: POINTER TO ARRAY OF INTEGER;
			i, numTabs, pos: INTEGER;
			f: TextMappers.Formatter;
			text: TextModels.Model;
			options: SET;
	BEGIN
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		options := Options();
		numTabs := NumTabs(options);
		NEW(tabs, numTabs);
		tabs[0] := 0;
		tabs[1] := 20 * Ports.mm;
		i := 2;
		WHILE i < numTabs DO
			tabs[i] := tabs[1] + 16 * (i - 1) * Ports.mm;
			INC(i)
		END;
		BugsFiles.WriteRuler(tabs, f);
		pos := f.Pos();
		SummaryFormatted.Stats(dialog.node.item, options, f);
		IF f.Pos() > pos THEN BugsFiles.Open("Summary statistics", text) END
	END Stats;

	(*	summary statistics with no percentiles	*)
	PROCEDURE StatsNoPercentiles*;
		CONST
			percentiles = {SummaryFormatted.medianOpt, SummaryFormatted.quant0Opt,
			SummaryFormatted.quant1Opt};
		VAR
			tabs: POINTER TO ARRAY OF INTEGER;
			i, numTabs, pos: INTEGER;
			f: TextMappers.Formatter;
			text: TextModels.Model;
			options: SET;
	BEGIN
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		options := Options() - percentiles;
		numTabs := 5;
		NEW(tabs, numTabs);
		tabs[0] := 0;
		tabs[1] := 25 * Ports.mm;
		i := 2;
		WHILE i < numTabs DO
			tabs[i] := tabs[1] + 20 * (i - 1) * Ports.mm;
			INC(i)
		END;
		BugsFiles.WriteRuler(tabs, f);
		pos := f.Pos();
		SummaryFormatted.Stats(dialog.node.item, options, f);
		IF f.Pos() > pos THEN BugsFiles.Open("Summary statistics", text) END
	END StatsNoPercentiles;

	PROCEDURE SetGuard* (VAR par: Dialog.Par);
		VAR
			variable: Dialog.String;
			p: ARRAY 1 OF Dialog.String;
	BEGIN
		par.disabled := FALSE;
		IF ~BugsInterface.IsInitialized() THEN
			par.disabled := TRUE;
			IF BugsCmds.script THEN BugsMsg.Lookup("SummaryCmds:NotInitialized", par.label) END
		ELSE
			IF BugsInterface.IsAdapting() THEN
				par.disabled := TRUE;
				IF BugsCmds.script THEN BugsMsg.Lookup("SummaryCmds:Adapting", par.label) END
			ELSE
				variable := dialog.node.item;
				IF BugsIndex.Find(variable) = NIL THEN
					par.disabled := TRUE;
					IF BugsCmds.script THEN
						p[0] := variable$;
						BugsMsg.LookupParam("SummaryCmds:NotVariable", p, par.label)
					END
				ELSE
					IF SummaryIndex.Find(variable) # NIL THEN
						par.disabled := TRUE;
						IF BugsCmds.script THEN
							p[0] := variable$;
							BugsMsg.LookupParam("SummaryCmds:AlreadySet", p, par.label)
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
			IF BugsCmds.script THEN BugsMsg.Lookup("SummaryCmds:NotInitialized", par.label) END
		ELSE
			variable := dialog.node.item;
			IF~SummaryInterface.IsStar(variable) THEN
				IF SummaryIndex.Find(variable) = NIL THEN
					par.disabled := TRUE;
					IF BugsCmds.script THEN 
						p[0] := variable$;
						BugsMsg.LookupParam("SummaryCmds:NotSet", p, par.label)
					END
				END
			ELSE
				numMonitors := SummaryIndex.NumberOfMonitors();
				IF numMonitors = 0 THEN
					par.disabled := TRUE;
					IF BugsCmds.script THEN 
						BugsMsg.Lookup("SummaryCmds:NoMonitors", par.label)
					END
				END
			END
		END
	END StatsGuard;

	PROCEDURE InitDialog;
	BEGIN
		dialog.node.item := "";
		dialog.node.SetLen(1);
		dialog.node.SetItem(0, " ");
		dialog.options.Excl(0, dialog.options.len - 1);
		dialog.options.Incl(SummaryFormatted.medianOpt, SummaryFormatted.medianOpt);
		dialog.options.Incl(SummaryFormatted.quant0Opt, SummaryFormatted.quant0Opt);
		dialog.options.Incl(SummaryFormatted.quant1Opt, SummaryFormatted.quant1Opt);
	END InitDialog;

	PROCEDURE (dialog: DialogBox) Init-;
	BEGIN
		InitDialog
	END Init;

	PROCEDURE (dialog: DialogBox) Update-;
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
		dialog.options.SetResources("#Summary:options");
		InitDialog;
		BugsDialog.AddDialog(dialog);
		BugsDialog.UpdateDialogs
	END Init;

BEGIN
	Init
END SummaryCmds.
