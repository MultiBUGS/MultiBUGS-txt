(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE RanksCmds;

	

	IMPORT
		Dialog, Ports, Strings,
		BugsCmds, BugsDialog, BugsFiles, BugsIndex, BugsInterface, BugsMsg, 
		PlotsViews,
		RanksDensity, RanksFormatted, RanksIndex, RanksInterface, RanksMonitors, 
		TextMappers, TextModels;

	TYPE

		DialogBox* = POINTER TO RECORD(BugsDialog.DialogBox)	(* dialog for node monitoring *)
			node*: Dialog.Combo;
			percentiles*: Dialog.Selection
		END;

	VAR
		dialog*: DialogBox;

		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE GetPercentiles (): POINTER TO ARRAY OF REAL;
		VAR
			i, j, len, numFrac, res: INTEGER;
			fractions: POINTER TO ARRAY OF REAL;
			string: Dialog.String;
	BEGIN
		len := dialog.percentiles.len;
		numFrac := 0;
		i := 0;
		WHILE i < len DO
			IF dialog.percentiles.In(i) THEN
				INC(numFrac)
			END;
			INC(i)
		END;
		IF numFrac = 0 THEN
			fractions := NIL;
			RETURN NIL
		END;
		NEW(fractions, numFrac);
		i := 0;
		j := 0;
		WHILE i < len DO
			IF dialog.percentiles.In(i) THEN
				dialog.percentiles.GetItem(i, string);
				IF string = "median" THEN
					fractions[j] := 50.0
				ELSE
					Strings.StringToReal(string, fractions[j], res)
				END;
				INC(j)
			END;
			INC(i)
		END;
		RETURN fractions
	END GetPercentiles;

	PROCEDURE NumFractions (): INTEGER;
		VAR
			i, len, numFrac: INTEGER;
	BEGIN
		len := dialog.percentiles.len;
		numFrac := 0;
		i := 0;
		WHILE i < len DO
			IF dialog.percentiles.In(i) THEN
				INC(numFrac)
			END;
			INC(i)
		END;
		RETURN numFrac
	END NumFractions;

	PROCEDURE UpdateNames*;
		VAR
			i, num: INTEGER;
			monitors: POINTER TO ARRAY OF RanksMonitors.Monitor;
	BEGIN
		monitors := RanksIndex.GetMonitors();
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
		IF RanksInterface.IsStar(name) THEN
			Dialog.GetOK("This will delete all set monitors", "", "", "", {Dialog.ok, Dialog.cancel}, res);
			IF res # Dialog.ok THEN RETURN END
		END;
		RanksInterface.Clear(name, ok);
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
		RanksInterface.Clear(name, ok);
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

	PROCEDURE MakePlots (IN name: ARRAY OF CHAR): POINTER TO ARRAY OF PlotsViews.View;
		VAR
			i, numBins, sampleSize: INTEGER;
			string: Dialog.String;
			histogram: POINTER TO ARRAY OF INTEGER;
			monitor: RanksMonitors.Monitor;
			plots: POINTER TO ARRAY OF PlotsViews.View;
	BEGIN
		sampleSize := RanksInterface.SampleSize(name);
		IF sampleSize = 0 THEN
			RETURN NIL
		END;
		monitor := RanksIndex.Find(name);
		numBins := monitor.Name().Size();
		NEW(histogram, numBins);
		NEW(plots, numBins);
		i := 0;
		WHILE i < numBins DO
			monitor.Histogram(i, histogram);
			monitor.Name().Indices(i, string);
			string := monitor.Name().string + string;
			plots[i] := RanksDensity.New(string, histogram, sampleSize);
			INC(i)
		END;
		RETURN plots
	END MakePlots;

	PROCEDURE FormatPlots (IN name: ARRAY OF CHAR; VAR f: TextMappers.Formatter);
		VAR
			h, i, num, w: INTEGER;
			plots: POINTER TO ARRAY OF PlotsViews.View;
	BEGIN
		plots := MakePlots(name);
		IF plots # NIL THEN
			num := LEN(plots)
		ELSE
			num := 0
		END;
		i := 0;
		WHILE i < num DO
			plots[i].MinSize(w, h);
			f.WriteView(plots[i]);
			INC(i)
		END
	END FormatPlots;

	PROCEDURE Draw*;
		VAR
			i, len: INTEGER;
			name: Dialog.String;
			monitors: POINTER TO ARRAY OF RanksMonitors.Monitor;
			f: TextMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		name := dialog.node.item$;
		IF RanksInterface.IsStar(name) THEN
			monitors := RanksIndex.GetMonitors();
			i := 0;
			IF monitors # NIL THEN
				len := LEN(monitors)
			ELSE
				len := 0
			END;
			WHILE i < len DO
				FormatPlots(monitors[i].Name().string, f);
				INC(i)
			END
		ELSE
			FormatPlots(name, f)
		END;
		IF f.Pos() > 0 THEN BugsFiles.Open("Rank histograms", text) END
	END Draw;

	PROCEDURE Set*;
		VAR
			ok: BOOLEAN;
			errorMes: ARRAY 1024 OF CHAR;
			name: ARRAY 128 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		name := dialog.node.item$;
		RanksInterface.Set(name, ok);
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

	PROCEDURE Stats*;
		VAR
			i, numFrac, numTabs, pos: INTEGER;
			tabs: POINTER TO ARRAY OF INTEGER;
			fractions: POINTER TO ARRAY OF REAL;
			f: TextMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		numFrac := NumFractions();
		numTabs := numFrac + 3;
		NEW(tabs, numTabs);
		tabs[0] := 0;
		tabs[1] := 25 * Ports.mm;
		i := 2;
		WHILE i < numTabs DO
			tabs[i] := tabs[1] + 15 * Ports.mm * (i - 1);
			INC(i)
		END;
		BugsFiles.WriteRuler(tabs, f);
		fractions := GetPercentiles();
		pos := f.Pos();
		RanksFormatted.Stats(dialog.node.item, fractions, f);
		IF f.Pos() > pos THEN BugsFiles.Open("Rank statistics", text) END
	END Stats;

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
			IF BugsCmds.script THEN BugsMsg.Lookup("RanksCmds:NotInitialized", par.label) END
		ELSE
			IF BugsInterface.IsAdapting() THEN
				par.disabled := TRUE;
				IF BugsCmds.script THEN BugsMsg.Lookup("RanksCmds:Adapting", par.label) END
			ELSE
				variable := dialog.node.item;
				IF BugsIndex.Find(variable) = NIL THEN
					par.disabled := TRUE;
					IF BugsCmds.script THEN 
						BugsMsg.LookupParam("RanksCmds:NotVariable", p, par.label)
					END
				ELSE
					IF BugsIndex.Find(variable).numSlots # 1 THEN
						par.disabled := TRUE;
						IF BugsCmds.script THEN 
							p[0] := variable$;
							BugsMsg.LookupParam("RanksCmds:NotVector", p, par.label)
						END
					ELSE
						IF RanksIndex.Find(variable) # NIL THEN
							par.disabled := TRUE;
							IF BugsCmds.script THEN 
								p[0] := variable$;
								BugsMsg.LookupParam("RanksCmds:AlreadySet", p, par.label)
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
			IF BugsCmds.script THEN BugsMsg.Lookup("RanksCmds:NotInitialized", par.label) END
		ELSE
			variable := dialog.node.item;
			IF~RanksInterface.IsStar(variable) THEN
				IF RanksIndex.Find(variable) = NIL THEN
					par.disabled := TRUE;
					IF BugsCmds.script THEN
						p[0] := variable$;
						BugsMsg.LookupParam("RanksCmds:NotSet", p, par.label);
					END
				END
			ELSE
				numMonitors := RanksIndex.NumberOfMonitors();
				IF numMonitors = 0 THEN
					par.disabled := TRUE;
					IF BugsCmds.script THEN
						BugsMsg.Lookup("RanksCmds:NoMonitors", par.label)
					END
				END
			END
		END
	END StatsGuard;

	PROCEDURE (dialogBox0: DialogBox) Init-;
	BEGIN
		dialog.node.item := "";
		dialog.node.SetLen(1);
		dialog.node.SetItem(0, " ")
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
		dialog.percentiles.SetResources("#Ranks:percentiles");
		dialog.percentiles.Incl(0, 0);
		dialog.percentiles.Incl(4, 4);
		dialog.percentiles.Incl(8, 8);
		BugsDialog.AddDialog(dialog);
		BugsDialog.UpdateDialogs
	END Init;

BEGIN
	Init
END RanksCmds.
