(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE SamplesCmds;

	

	IMPORT
		Dialog, Files, Ports, Strings,
		BugsCmds,
		BugsDialog, BugsFiles, BugsIndex, BugsInterface, BugsMappers, BugsMsg,
		BugsTexts, SamplesFormatted,
		SamplesIndex, SamplesInterface, SamplesMonitors, SamplesPlots, TextModels,
		UpdaterActions;

	TYPE
		DialogBox* = POINTER TO RECORD(BugsDialog.DialogBox)
			node*: Dialog.Combo;
			options*: Dialog.Selection;
			beg*, end*, thin*, firstChain*, lastChain*: INTEGER
		END;

	VAR
		dialog*: DialogBox;

		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE InitDialog;
	BEGIN
		dialog.options.Excl(0, dialog.options.len - 1);
		dialog.options.Incl(SamplesFormatted.medianOpt, SamplesFormatted.medianOpt);
		dialog.options.Incl(SamplesFormatted.quant0Opt, SamplesFormatted.quant0Opt);
		dialog.options.Incl(SamplesFormatted.quant7Opt, SamplesFormatted.quant7Opt);
		dialog.options.Incl(SamplesFormatted.essOpt, SamplesFormatted.essOpt);
		dialog.node.item := "";
		dialog.beg := 1;
		dialog.end := 10000000;
		dialog.thin := 1;
		dialog.firstChain := 1;
		dialog.lastChain := BugsCmds.specificationDialog.numChains;
		dialog.node.SetLen(1);
		dialog.node.SetItem(0, " ")
	END InitDialog;

	PROCEDURE UpdateNames*;
		VAR
			i, len: INTEGER;
			monitors: POINTER TO ARRAY OF SamplesMonitors.Monitor;
	BEGIN
		monitors := SamplesIndex.GetMonitors();
		i := 0;
		IF monitors # NIL THEN
			len := LEN(monitors)
		ELSE
			len := 0
		END;
		WHILE i < len DO
			dialog.node.SetItem(i, monitors[i].Name().string);
			INC(i)
		END;
		dialog.node.SetLen(len);
		dialog.node.item := ""
	END UpdateNames;

	PROCEDURE (dialog: DialogBox) Init-;
	BEGIN
		InitDialog
	END Init;

	PROCEDURE (dialog: DialogBox) Update-;
	BEGIN
		UpdateNames;
		Dialog.UpdateList(dialog.node);
		Dialog.UpdateList(dialog.options);
		dialog.lastChain := BugsCmds.specificationDialog.numChains;
		Dialog.Update(dialog)
	END Update;

	PROCEDURE GetFractions (OUT fractions: ARRAY OF REAL);
		VAR
			i, j, k, len, len1, res: INTEGER;
			string, string1: Dialog.String;
			c: CHAR;
	BEGIN
		len := dialog.options.len;
		i := SamplesFormatted.quant0Opt;
		WHILE i < len DO
			dialog.options.GetItem(i, string);
			j := 0;
			k := 0;
			len1 := LEN(string$);
			WHILE j < len1 DO
				c := string[j];
				CASE c OF
					"0".."9", ".": string1[k] := c; INC(k)
				ELSE
				END;
				INC(j);
			END;
			string1[k] := 0X;
			Strings.StringToReal(string1, fractions[i - SamplesFormatted.quant0Opt], res);
			INC(i)
		END
	END GetFractions;

	PROCEDURE Clear*;
		VAR
			res: INTEGER;
			ok: BOOLEAN;
			string: Dialog.String;
			msg: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		string := dialog.node.item$;
		IF SamplesInterface.IsStar(string) THEN
			Dialog.GetOK("This will delete all set monitors", "", "", "", {Dialog.ok, Dialog.cancel}, res);
			IF res # Dialog.ok THEN RETURN END;
		END;
		SamplesInterface.Clear(string, ok);
		IF ~ok THEN
			p[0] := string$;
			BugsMsg.GetError(msg);
			BugsMsg.ParamMsg(msg, p, msg);
			BugsTexts.ShowMsg(msg);
			RETURN
		END;
		UpdateNames;
		Dialog.UpdateList(dialog.node);
		Dialog.Update(dialog)
	END Clear;

	PROCEDURE ClearNI*;
		VAR
			ok: BOOLEAN;
			string: Dialog.String;
			msg: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		string := dialog.node.item$;
		SamplesInterface.Clear(string, ok);
		IF ~ok THEN
			p[0] := string$;
			BugsMsg.GetError(msg);
			BugsMsg.ParamMsg(msg, p, msg);
			BugsTexts.ShowMsg(msg);
			RETURN
		END;
		UpdateNames;
		Dialog.UpdateList(dialog.node);
		Dialog.Update(dialog)
	END ClearNI;

	PROCEDURE CODA*;
		VAR
			beg, end, firstChain, i, lastChain, numChains, oldWhereOut, thin: INTEGER;
			numAsString: ARRAY 8 OF CHAR;
			string: Dialog.String;
			msg: ARRAY 1024 OF CHAR;
			f: POINTER TO ARRAY OF BugsMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		oldWhereOut := BugsCmds.displayDialog.whereOut;
		BugsMappers.SetDest(BugsMappers.window);
		string := dialog.node.item;
		beg := dialog.beg - 1;
		end := dialog.end;
		thin := dialog.thin;
		firstChain := dialog.firstChain;
		lastChain := dialog.lastChain;
		numChains := lastChain - firstChain + 1;
		NEW(f, numChains + 1);
		i := 0;
		WHILE i <= numChains DO
			text := TextModels.dir.New();
			BugsTexts.ConnectFormatter(f[i], text);
			f[i].SetPos(0);
			INC(i)
		END;
		SamplesFormatted.CODA(string, beg, end, thin, firstChain, lastChain, f);
		f[0].Register("CODA index");
		i := 1;
		WHILE i <= numChains DO
			Strings.IntToString(i + firstChain - 1, numAsString);
			f[i].Register("CODAchain " + numAsString);
			INC(i)
		END;
		BugsCmds.displayDialog.whereOut := oldWhereOut;
		BugsMsg.MapMsg("SamplesCmds:CODAFilesWritten", msg);
		BugsTexts.ShowMsg(msg)
	END CODA;

	PROCEDURE CODAFiles* (stemName: ARRAY OF CHAR);
		VAR
			beg, end, firstChain, i, lastChain, numChains, thin: INTEGER;
			numAsString: ARRAY 8 OF CHAR;
			msg: ARRAY 1024 OF CHAR;
			name: Files.Name;
			string: Dialog.String;
			f: POINTER TO ARRAY OF BugsMappers.Formatter;
			file: Files.File;
			loc: Files.Locator;
	BEGIN
		string := dialog.node.item;
		beg := dialog.beg - 1;
		end := dialog.end;
		thin := dialog.thin;
		firstChain := dialog.firstChain;
		lastChain := dialog.lastChain;
		numChains := lastChain - firstChain + 1;
		NEW(f, numChains + 1);
		BugsFiles.PathToFileSpec(stemName, loc, name);
		i := 0;
		WHILE i <= numChains DO
			file := Files.dir.New(loc, Files.dontAsk);
			BugsFiles.ConnectFormatter(f[i], file);
			f[i].SetPos(0);
			INC(i)
		END;
		SamplesFormatted.CODA(string, beg, end, thin, firstChain, lastChain, f);
		f[0].Register(name + "CODAindex");
		i := 1;
		WHILE i <= numChains DO
			Strings.IntToString(i + firstChain - 1, numAsString);
			f[i].Register(name + "CODAchain" + numAsString);
			INC(i)
		END;
		BugsMsg.MapMsg("SamplesCmds:CODAFilesWritten", msg);
		BugsTexts.ShowMsg(msg)
	END CODAFiles;

	PROCEDURE PlotMarkov*;
		VAR
			beg, end, thin, firstChain, lastChain, numChains: INTEGER;
			string, title: Dialog.String;
			f: BugsMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		(*	only include values stored after end of adapting period	*)
		beg := MAX(dialog.beg, UpdaterActions.endOfAdapting) - 1;
		end := dialog.end;
		thin := dialog.thin;
		firstChain := dialog.firstChain;
		lastChain := dialog.lastChain;
		numChains := BugsCmds.specificationDialog.numChains;
		string := dialog.node.item;
		text := TextModels.dir.New();
		BugsTexts.ConnectFormatter(f, text);
		f.SetPos(0);
		SamplesPlots.Draw(string, beg, end, thin, firstChain, lastChain, numChains, f);
		IF f.views # 0 THEN
			SamplesPlots.Title(title);
			f.Register(title)
		END
	END PlotMarkov;

	PROCEDURE Plot*;
		VAR
			beg, end, thin, firstChain, lastChain, numChains: INTEGER;
			string, title: Dialog.String;
			f: BugsMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		string := dialog.node.item;
		beg := dialog.beg - 1;
		end := dialog.end;
		thin := dialog.thin;
		firstChain := dialog.firstChain;
		lastChain := dialog.lastChain;
		numChains := BugsCmds.specificationDialog.numChains;
		text := TextModels.dir.New();
		BugsTexts.ConnectFormatter(f, text);
		f.SetPos(0);
		SamplesPlots.Draw(string, beg, end, thin, firstChain, lastChain, numChains, f);
		IF f.views # 0 THEN
			SamplesPlots.Title(title);
			f.Register(title)
		END
	END Plot;

	PROCEDURE Set*;
		VAR
			beg, numChains: INTEGER;
			ok: BOOLEAN;
			string: Dialog.String;
			msg: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		string := dialog.node.item;
		numChains := BugsCmds.specificationDialog.numChains;
		beg := UpdaterActions.iteration;
		SamplesInterface.Set(string, beg, numChains, ok);
		IF ~ok THEN
			p[0] := string$;
			BugsMsg.GetError(msg);
			BugsMsg.ParamMsg(msg, p, msg);
			BugsTexts.ShowMsg(msg);
			RETURN
		END;
		UpdateNames;
		Dialog.UpdateList(dialog.node);
		Dialog.Update(dialog)
	END Set;

	PROCEDURE Stats*;
		VAR
			beg, end, firstChain, i, lastChain, len, numTabs, thin: INTEGER;
			string: Dialog.String;
			tabs: POINTER TO ARRAY OF INTEGER;
			fractions: ARRAY 10 OF REAL;
			f: BugsMappers.Formatter;
			text: TextModels.Model;
			options: SET;
	BEGIN
		text := TextModels.dir.New();
		BugsTexts.ConnectFormatter(f, text);
		f.SetPos(0);
		i := 0;
		len := dialog.options.len;
		numTabs := 7;
		options := {};
		WHILE i < len DO
			IF dialog.options.In(i) THEN
				INCL(options, i);
				INC(numTabs)
			END;
			INC(i)
		END;
		IF SamplesFormatted.hpd0Opt IN options THEN INC(numTabs) END;
		IF SamplesFormatted.hpd1Opt IN options THEN INC(numTabs) END;
		GetFractions(fractions);
		NEW(tabs, numTabs);
		tabs[0] := 0;
		tabs[1] := 20 * Ports.mm;
		i := 2;
		WHILE i < numTabs DO
			tabs[i] := tabs[1] + 16 * Ports.mm * (i - 1);
			INC(i)
		END;
		f.WriteRuler(tabs);
		string := dialog.node.item;
		(*	only include values stored after end of adapting period	*)
		beg := MAX(dialog.beg, UpdaterActions.endOfAdapting) - 1;
		end := dialog.end;
		thin := dialog.thin;
		firstChain := dialog.firstChain;
		lastChain := dialog.lastChain;
		SamplesFormatted.StatsSummary(string, beg, end, thin, firstChain, lastChain, options, fractions, f);
		IF f.lines > 1 THEN
			f.Register("Node statistics")
		END
	END Stats;

	PROCEDURE SetVariable* (var: ARRAY OF CHAR);
	BEGIN
		dialog.node.item := var$
	END SetVariable;

	PROCEDURE SetGuard* (OUT ok: BOOLEAN);
		VAR
			i: INTEGER;
			msg: ARRAY 1024 OF CHAR;
			var: Dialog.String;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		ok := BugsInterface.IsInitialized();
		IF ~ok THEN
			BugsMsg.MapMsg("SamplesCmds:NotInitialized", msg);
			BugsFiles.ShowMsg(msg)
		ELSE
			var := dialog.node.item;
			i := 0;
			WHILE (var[i] # 0X) & (var[i] # "[") DO
				INC(i)
			END;
			var[i] := 0X;
			ok := BugsIndex.Find(var) # NIL;
			IF ~ok THEN
				p[0] := var$;
				BugsMsg.MapParamMsg("SamplesCmds:NotVariable", p, msg);
				BugsFiles.ShowMsg(msg)
			END
		END
	END SetGuard;

	PROCEDURE StatsGuard* (OUT ok: BOOLEAN);
		VAR
			i, numMonitors: INTEGER;
			msg: ARRAY 1024 OF CHAR;
			var: Dialog.String;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		ok := BugsInterface.IsInitialized();
		IF ~ok THEN
			BugsMsg.MapMsg("SamplesCmds:NotInitialized", msg);
			BugsFiles.ShowMsg(msg)
		ELSE
			ok := ~BugsInterface.IsAdapting();
			IF ~ok THEN
				BugsMsg.MapMsg("SamplesCmds:Adapting", msg);
				BugsFiles.ShowMsg(msg)
			ELSE
				var := dialog.node.item;
				IF~SamplesInterface.IsStar(var) THEN
					i := 0;
					WHILE (var[i] # 0X) & (var[i] # "[") DO
						INC(i)
					END;
					var[i] := 0X;
					ok := SamplesIndex.Find(var) # NIL;
					IF ~ok THEN
						p[0] := var$;
						BugsMsg.MapParamMsg("SamplesCmds:NotSet", p, msg);
						BugsFiles.ShowMsg(msg)
					END
				ELSE
					numMonitors := SamplesIndex.NumberOfMonitors();
					ok := numMonitors # 0;
					IF ~ok THEN
						BugsMsg.MapMsg("SamplesCmds:NoMonitors", msg);
						BugsFiles.ShowMsg(msg)
					END
				END
			END
		END
	END StatsGuard;

	PROCEDURE BGRGuard* (OUT ok: BOOLEAN);
		VAR
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		IF BugsCmds.specificationDialog.numChains = 1 THEN
			ok := FALSE;
			BugsMsg.MapMsg("SamplesCmds:OnlyOneChain", msg);
			BugsFiles.ShowMsg(msg)
		ELSE
			StatsGuard(ok)
		END
	END BGRGuard;

	PROCEDURE HistoryGuard* (OUT ok: BOOLEAN);
		VAR
			i, numMonitors: INTEGER;
			msg: ARRAY 1024 OF CHAR;
			var: Dialog.String;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		ok := BugsInterface.IsInitialized();
		IF ~ok THEN
			BugsMsg.MapMsg("SamplesCmds:NotInitialized", msg);
			BugsFiles.ShowMsg(msg)
		ELSE
			var := dialog.node.item;
			IF~SamplesInterface.IsStar(var) THEN
				i := 0;
				WHILE (var[i] # 0X) & (var[i] # "[") DO
					INC(i)
				END;
				var[i] := 0X;
				ok := SamplesIndex.Find(var) # NIL;
				IF ~ok THEN
					p[0] := var$;
					BugsMsg.MapParamMsg("SamplesCmds:NotSet", p, msg);
					BugsFiles.ShowMsg(msg)
				END
			ELSE
				numMonitors := SamplesIndex.NumberOfMonitors();
				ok := numMonitors # 0;
				IF ~ok THEN
					BugsMsg.MapMsg("SamplesCmds:NoMonitors", msg);
					BugsFiles.ShowMsg(msg)
				END
			END
		END
	END HistoryGuard;

	PROCEDURE SetGuardWin* (VAR par: Dialog.Par);
		VAR
			i: INTEGER;
			var: Dialog.String;
	BEGIN
		par.disabled := FALSE;
		IF ~BugsInterface.IsInitialized() THEN
			par.disabled := TRUE
		ELSE
			var := dialog.node.item;
			i := 0;
			WHILE (var[i] # 0X) & (var[i] # "[") DO
				INC(i)
			END;
			var[i] := 0X;
			IF BugsIndex.Find(var) = NIL THEN
				par.disabled := TRUE
			END
		END
	END SetGuardWin;

	PROCEDURE StatsGuardWin* (VAR par: Dialog.Par);
		VAR
			i, numMonitors: INTEGER;
			var: Dialog.String;
	BEGIN
		par.disabled := dialog.lastChain < dialog.firstChain;
		IF ~BugsInterface.IsInitialized() THEN
			par.disabled := TRUE
		ELSE
			IF BugsInterface.IsAdapting() THEN
				par.disabled := TRUE
			ELSE
				var := dialog.node.item;
				IF~SamplesInterface.IsStar(var) THEN
					i := 0;
					WHILE (var[i] # 0X) & (var[i] # "[") DO
						INC(i)
					END;
					var[i] := 0X;
					IF SamplesIndex.Find(var) = NIL THEN
						par.disabled := TRUE
					END
				ELSE
					numMonitors := SamplesIndex.NumberOfMonitors();
					IF numMonitors = 0 THEN
						par.disabled := TRUE
					END
				END
			END
		END
	END StatsGuardWin;

	PROCEDURE BGRGuardWin* (VAR par: Dialog.Par);
	BEGIN
		IF BugsCmds.specificationDialog.numChains = 1 THEN
			par.disabled := TRUE
		ELSE
			par.disabled := dialog.lastChain <= dialog.firstChain;
			IF ~par.disabled THEN
				StatsGuardWin(par)
			END
		END
	END BGRGuardWin;

	PROCEDURE HistoryGuardWin* (VAR par: Dialog.Par);
		VAR
			i, numMonitors: INTEGER;
			var: Dialog.String;
	BEGIN
		par.disabled := dialog.lastChain < dialog.firstChain;
		IF ~BugsInterface.IsInitialized() THEN
			par.disabled := TRUE
		ELSE
			var := dialog.node.item;
			IF~SamplesInterface.IsStar(var) THEN
				i := 0;
				WHILE (var[i] # 0X) & (var[i] # "[") DO
					INC(i)
				END;
				var[i] := 0X;
				IF SamplesIndex.Find(var) = NIL THEN
					par.disabled := TRUE
				END
			ELSE
				numMonitors := SamplesIndex.NumberOfMonitors();
				IF numMonitors = 0 THEN
					par.disabled := TRUE
				END
			END
		END
	END HistoryGuardWin;

	PROCEDURE FirstNotifier* (op, from, to: INTEGER);
	BEGIN
		dialog.firstChain := MAX(1, dialog.firstChain);
		dialog.firstChain := MIN(dialog.firstChain, BugsCmds.specificationDialog.numChains)
	END FirstNotifier;

	PROCEDURE LastNotifier* (op, from, to: INTEGER);
	BEGIN
		dialog.lastChain := MAX(1, dialog.lastChain);
		dialog.lastChain := MIN(dialog.lastChain, BugsCmds.specificationDialog.numChains)
	END LastNotifier;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		NEW(dialog);
		dialog.options.SetResources("#Samples:options");
		InitDialog;
		BugsDialog.AddDialog(dialog);
		BugsDialog.UpdateDialogs
	END Init;

BEGIN
	Init
END SamplesCmds.
