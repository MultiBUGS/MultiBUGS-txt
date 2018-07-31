(* 	

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsCmds;

	

	IMPORT
		Controllers, Converters, Dialog, Files, Meta, Models, Ports, Services, Strings, Views,
		StdLog,
		BugsDialog, BugsFiles, BugsGraph, BugsIndex, BugsInfo, BugsInterface, BugsLatexprinter,
		BugsMAP, BugsMappers, BugsMsg, BugsNames, BugsParser, BugsPrettyprinter,
		BugsRandnum, BugsScripting, BugsSerialize, BugsVersion,
		DevDebug,
		GraphNodes, GraphRules, GraphStochastic,
		HostMenus,
		MathRandnum,
		MonitorMonitors,
		TextCmds, TextControllers, TextMappers, TextModels, TextViews,
		UpdaterActions, UpdaterMethods, UpdaterSettings;

	TYPE
		Action = POINTER TO RECORD (Services.Action)
			updating: BOOLEAN;
			updates: INTEGER
		END;

		ScriptAction = POINTER TO RECORD(Services.Action)
			s: BugsMappers.Scanner
		END;

		CompileDialog* = POINTER TO RECORD(BugsDialog.DialogBox)
			updaterByMethod*: BOOLEAN;
		END;

		DisplayDialog* = POINTER TO RECORD(BugsDialog.DialogBox)
			precision*: INTEGER;
			whereOut*: INTEGER
		END;

		InfoDialog* = POINTER TO RECORD(BugsDialog.DialogBox)
			node*: Dialog.Combo
		END;

		RNDialog* = POINTER TO RECORD(BugsDialog.DialogBox)
			preSet*: INTEGER
		END;

		SpecificationDialog* = POINTER TO RECORD(BugsDialog.DialogBox)
			chain*: INTEGER; 	(*	load inits for this chain	*)
			numChains*, numProc*: INTEGER; 	(*	number of MCMC chains	*)
			fixFounder*: BOOLEAN
		END;

		UpdateDialog* = POINTER TO RECORD(BugsDialog.DialogBox)
			iteration-: INTEGER; 	(*	current iteration number	*)
			refresh*: INTEGER; 	(*	return control to framework after resresh updates	*)
			thin*: INTEGER; 	(*	prespective thining	*)
			updates*: INTEGER; 	(*	number of MCMC updates	*)
			isAdapting-: BOOLEAN; 	(*	is sampler adapting	*)
			overRelax*: BOOLEAN; 	(*	over relax the gibbs sampler	 *)
		END;

		InBugDialog* = POINTER TO RECORD(BugsDialog.DialogBox)
			name*: Dialog.List
		END;

		OutBugDialog* = POINTER TO RECORD(BugsDialog.DialogBox)
			name*: Dialog.Combo
		END;

	VAR
		startTime: LONGINT;
		action: Action;
		scriptAction: ScriptAction;
		filePath: ARRAY 1024 OF CHAR;
		defaultNumChains, defaultRefresh: INTEGER;

		compileDialog*: CompileDialog;
		displayDialog*: DisplayDialog;
		infoDialog*: InfoDialog;
		rnDialog*: RNDialog;
		specificationDialog*: SpecificationDialog;
		updateDialog*: UpdateDialog;
		inBugDialog*: InBugDialog;
		outBugDialog*: OutBugDialog;

		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		numberPresets: INTEGER;

	PROCEDURE SetDisplay* (option: ARRAY OF CHAR);
	BEGIN
		IF option = "log" THEN
			displayDialog.whereOut := BugsFiles.log;
			BugsFiles.SetDest(BugsFiles.log);
			StdLog.Open
		ELSIF option = "window" THEN
			displayDialog.whereOut := BugsFiles.window;
			BugsFiles.SetDest(BugsFiles.window)
		ELSIF option = "file" THEN
			displayDialog.whereOut := - 1;
			BugsFiles.SetDest(BugsFiles.file)
		END
	END SetDisplay;

	PROCEDURE SetRNState* (preSet: INTEGER);
	BEGIN
		IF (BugsRandnum.generators # NIL) & (BugsRandnum.generators[0] # NIL) THEN
			numberPresets := MathRandnum.NumPresets(BugsRandnum.generators[0])
		END;
		IF preSet < 1 THEN
			preSet := 1
		ELSIF preSet > numberPresets THEN
			preSet := numberPresets
		END;
		BugsRandnum.SetShift(preSet - 1);
		BugsRandnum.CreateGenerators(BugsRandnum.numberChains);
		rnDialog.preSet := preSet;
		Dialog.Update(rnDialog)
	END SetRNState;

	PROCEDURE SetDefaultRefresh* (refresh: INTEGER);
	BEGIN
		defaultRefresh := refresh
	END SetDefaultRefresh;

	PROCEDURE SetDefaultNumChains* (numChains: INTEGER);
	BEGIN
		defaultNumChains := numChains
	END SetDefaultNumChains;

	(*	Command to reset system	*)

	PROCEDURE Clear*;
	BEGIN
		BugsInterface.Clear;
		BugsDialog.InitDialogs;
	END Clear;

	PROCEDURE SetChain* (chain: INTEGER);
	BEGIN
		IF chain <= specificationDialog.numChains THEN
			specificationDialog.chain := chain
		ELSE
			specificationDialog.chain := 1
		END
	END SetChain;

	PROCEDURE SetFilePath* (path: ARRAY OF CHAR);
	BEGIN
		filePath := path$
	END SetFilePath;

	PROCEDURE SetIteration* (iteration: INTEGER);
	BEGIN
		updateDialog.iteration := iteration
	END SetIteration;

	PROCEDURE SetNode* (node: ARRAY OF CHAR);
	BEGIN
		infoDialog.node.item := node$
	END SetNode;

	PROCEDURE NewModel* (OUT ok: BOOLEAN);
		VAR
			res: INTEGER;
	BEGIN
		IF BugsInterface.IsParsed() THEN
			Dialog.GetOK("#Bugs:CmdsNewModel", "", "", "", {Dialog.ok, Dialog.cancel}, res);
			ok := res = Dialog.ok
		ELSE
			ok := TRUE
		END
	END NewModel;

	PROCEDURE GetText (): TextModels.Model;
		VAR
			m: Models.Model;
			text: TextModels.Model;
			v: Views.View;
			name: Files.Name;
			loc: Files.Locator;
			conv: Converters.Converter;
			pos: INTEGER;
	BEGIN
		IF filePath # "" THEN
			text := BugsFiles.FileToText(filePath)
		ELSE
			m := Controllers.FocusModel();
			IF m # NIL THEN
				IF m IS TextModels.Model THEN
					text := m(TextModels.Model)
				ELSE
					text := NIL
				END
			ELSE
				text := NIL
			END
		END;
		RETURN text
	END GetText;

	PROCEDURE GetPos (): INTEGER;
		VAR
			beg, end, pos: INTEGER;
			c: TextControllers.Controller;
			text: TextModels.Model;
	BEGIN
		text := GetText();
		IF (text # NIL) & (filePath = "") THEN
			c := TextControllers.Focus();
			c.GetSelection(beg, end);
			IF beg # end THEN
				pos := beg
			ELSE
				pos := 0
			END
		ELSE
			pos := 0
		END;
		RETURN pos
	END GetPos;

	PROCEDURE TextError (s: BugsMappers.Scanner; text: TextModels.Model);
		VAR
			beg, end: INTEGER;
			errorMes: Dialog.String;
	BEGIN
		end := s.Pos();
		CASE s.type OF
		|BugsMappers.int, BugsMappers.real, BugsMappers.string, BugsMappers.function:
			beg := end - s.len
		ELSE
			beg := end - 1
		END;
		errorMes := BugsMsg.message$;
		BugsDialog.ShowMsg(errorMes);
		IF filePath = "" THEN
			TextViews.ShowRange(text, beg, end, TextViews.any);
			TextControllers.SetSelection(text, beg, end)
		END;
		Dialog.Beep
	END TextError;

	(* 	Command to parse BUGS model 	*)

	PROCEDURE Parse*;
		VAR
			ok: BOOLEAN;
			pos: INTEGER;
			s: BugsMappers.Scanner;
			text: TextModels.Model;
	BEGIN
		IF filePath = "" THEN
			NewModel(ok);
			IF ~ok THEN filePath := ""; RETURN END
		END;
		Clear;
		BugsDialog.UpdateDialogs;
		text := GetText();
		IF text = NIL THEN filePath := ""; RETURN END;
		s.ConnectToText(text);
		pos := GetPos();
		s.SetPos(pos);
		BugsInterface.ParseModel(s, ok);
		IF ~BugsInterface.IsParsed() THEN TextError(s, text); filePath := ""; RETURN END;
		BugsDialog.UpdateDialogs;
		BugsDialog.ShowMsg("BugsCmds:OkSyntax");
		filePath := ""
	END Parse;

	(*	Commands to load data	*)

	PROCEDURE LoadData*;
		VAR
			ok: BOOLEAN;
			pos: INTEGER;
			s: BugsMappers.Scanner;
			text: TextModels.Model;
			msg, warning: ARRAY 1024 OF CHAR;
	BEGIN
		BugsMsg.Store("");
		IF ~BugsInterface.IsParsed() THEN filePath := ""; RETURN END;
		text := GetText();
		IF text = NIL THEN filePath := ""; RETURN END;
		s.ConnectToText(text);
		pos := GetPos();
		s.SetPos(pos);
		BugsInterface.LoadData(s, ok);
		IF ~ok THEN TextError(s, text); filePath := ""; RETURN END;
		BugsMsg.Lookup("BugsCmds:OkData", msg);
		warning := BugsMsg.message$;
		(* If warning not encountered, will contain empty string *)
		msg := msg + warning;
		BugsDialog.ShowMsg(msg);
		filePath := ""
	END LoadData;

	(*	Commands to write model graph	*)

	PROCEDURE SetAdaptingStatus*;
	BEGIN
		updateDialog.isAdapting := UpdaterActions.endOfAdapting = MAX(INTEGER)
	END SetAdaptingStatus;

	PROCEDURE Compile*;
		VAR
			updaterByMethod, ok: BOOLEAN;
			numChains, len0, len1: INTEGER;
			startTime, elapsedTime: LONGINT;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
		CONST
			eps = 1.0E-20;
	BEGIN
		IF ~BugsInterface.IsParsed() THEN RETURN END;
		numChains := specificationDialog.numChains;
		updaterByMethod := compileDialog.updaterByMethod;
		startTime := Services.Ticks();
		len0 := StdLog.text.Length();
		BugsGraph.Compile(numChains, updaterByMethod, ok);
		len1 := StdLog.text.Length();
		StdLog.text.Delete(len0, len1);
		IF ok THEN
			SetAdaptingStatus;
			UpdaterSettings.MarkCompiled;
			UpdaterSettings.FillDialog;
			BugsDialog.UpdateDialogs;
			elapsedTime := Services.Ticks() - startTime;
			elapsedTime := ENTIER(1.0 * elapsedTime / Services.resolution + eps);
			Strings.IntToString(elapsedTime, p[0]);
			BugsDialog.ShowParamMsg("BugsCmds:OkCompile", p);
		ELSE
			BugsDialog.ShowMsg(BugsMsg.message);
			RETURN
		END
	END Compile;

	(*	Commands to load initial values	*)

	PROCEDURE NewInits (OUT ok: BOOLEAN);
		VAR
			res: INTEGER;
	BEGIN
		IF BugsInterface.IsInitialized() THEN
			Dialog.GetOK("#Bugs:CmdsNewInits", "", "", "", {Dialog.ok, Dialog.cancel}, res);
			ok := res = Dialog.ok
		ELSE
			ok := TRUE
		END
	END NewInits;

	PROCEDURE LoadInits*;
		VAR
			ok, fixFounder: BOOLEAN;
			pos: INTEGER;
			chain, numChains: INTEGER;
			s: BugsMappers.Scanner;
			text: TextModels.Model;
			msg, warning: ARRAY 1024 OF CHAR;
	BEGIN
		BugsMsg.Store("");
		NewInits(ok);
		IF ~ok THEN filePath := ""; RETURN END;
		text := GetText();
		IF text = NIL THEN filePath := ""; RETURN END;
		s.ConnectToText(text);
		pos := GetPos();
		s.SetPos(pos);
		chain := specificationDialog.chain;
		numChains := specificationDialog.numChains;
		fixFounder := specificationDialog.fixFounder;
		BugsInterface.LoadInits(s, chain - 1, numChains, fixFounder, ok);
		IF ~ok THEN
			IF UpdaterActions.IsInitialized(chain - 1) THEN
				BugsDialog.ShowMsg(BugsMsg.message);
			ELSE
				TextError(s, text);
			END;
			filePath := ""; RETURN
		END;
		IF BugsInterface.IsInitialized() THEN
			BugsMsg.Lookup("BugsCmds:OkInits", msg);
		ELSIF UpdaterActions.IsInitialized(chain - 1) THEN
			BugsMsg.Lookup("BugsCmds:UninitOther", msg)
		ELSE
			BugsMsg.Lookup("BugsCmds:NotInit", msg)
		END;
		warning := BugsMsg.message$;
		msg := msg + warning;
		BugsDialog.ShowMsg(msg);
		SetChain(chain + 1);
		Dialog.Update(specificationDialog);
		filePath := ""
	END LoadInits;

	(*	Commands to generate initial values	*)

	PROCEDURE GenerateInits*;
		VAR
			numChains: INTEGER;
			fixFounder, ok: BOOLEAN;
	BEGIN
		IF ~BugsInterface.IsCompiled() THEN RETURN END;
		numChains := specificationDialog.numChains;
		fixFounder := specificationDialog.fixFounder;
		BugsInterface.GenerateInits(numChains, fixFounder, ok);
		IF ok THEN
			BugsDialog.ShowMsg("BugsCmds:OkGenInits");
		ELSE
			BugsDialog.ShowMsg(BugsMsg.message);
			RETURN
		END
	END GenerateInits;

	PROCEDURE Distribute* (mpiImplementation: ARRAY OF CHAR);
		VAR
			msg: ARRAY 1024 OF CHAR;
			numProc, numChains: INTEGER;
	BEGIN
		numProc := specificationDialog.numProc;
		numChains := specificationDialog.numChains;
		BugsInterface.Distribute(mpiImplementation, numProc, numChains);
		IF BugsInterface.IsDistributed() THEN
			BugsMsg.Store("model distributed")
		END;
		BugsDialog.ShowMsg(BugsMsg.message)
	END Distribute;

	PROCEDURE DistributeInfo*;
		VAR
			tabs: ARRAY 6 OF INTEGER;
			f: TextMappers.Formatter;
			text: TextModels.Model;
		CONST
			space = 35 * Ports.mm;
	BEGIN
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		tabs[0] := 0;
		tabs[1] := 5 * Ports.mm;
		tabs[2] := tabs[1] + space;
		tabs[3] := tabs[2] + space;
		tabs[4] := tabs[3] + space;
		tabs[5] := tabs[4] + space;
		BugsFiles.WriteRuler(tabs, f);
		BugsInfo.DistributeInfo(f);
		BugsFiles.Open("Distribution info", text)
	END DistributeInfo;

	PROCEDURE (a: Action) Do;
		CONST
			eps = 1.0E-20;
		VAR
			ok, overRelax: BOOLEAN;
			i, numChains, refresh, thin, updates: INTEGER;
			elapsedTime: LONGINT;
			p: ARRAY 2 OF ARRAY 1024 OF CHAR;
	BEGIN
		IF ~a.updating THEN RETURN END;
		i := 0;
		refresh := updateDialog.refresh;
		updates := updateDialog.updates;
		thin := updateDialog.thin;
		overRelax := updateDialog.overRelax;
		numChains := specificationDialog.numChains;
		ok := TRUE;
		WHILE(i < refresh) & (a.updates < updates) & ok DO
			INC(i);
			INC(a.updates);
			BugsInterface.UpdateModel(numChains, thin, overRelax, ok);
			IF ok THEN
				BugsInterface.UpdateMonitors(numChains);
			END;
			SetAdaptingStatus
		END;
		SetIteration(updateDialog.iteration + i);
		IF ~ok THEN
			a.updating := FALSE;
			BugsDialog.ShowMsg(BugsMsg.message);
			RETURN
		END;
		MonitorMonitors.UpdateDrawers;
		SetAdaptingStatus;
		Dialog.Update(updateDialog);
		IF a.updates < updates THEN
			Services.DoLater(a, Services.now)
		ELSE
			BugsInterface.RecvMCMCState;
			BugsInterface.LoadDeviance(0);
			a.updating := FALSE;
			elapsedTime := Services.Ticks() - startTime;
			elapsedTime := ENTIER(1.0 * elapsedTime / Services.resolution + eps);
			Strings.IntToString(updates, p[0]);
			Strings.IntToString(elapsedTime, p[1]);
			BugsDialog.ShowParamMsg("BugsCmds:UpdatesTook", p);
		END
	END Do;

	PROCEDURE Update*;
		VAR
	BEGIN
		IF ~action.updating THEN
			action.updating := TRUE;
			startTime := Services.Ticks();
			BugsDialog.ShowMsg("BugsCmds:Updating");
			action.updates := 0;
			Services.DoLater(action, Services.now)
		ELSE
			action.updating := FALSE
		END;
		Dialog.Update(updateDialog)
	END Update;

	PROCEDURE UpdateNI*;
		CONST
			eps = 1.0E-20;
		VAR
			ok, overRelax: BOOLEAN;
			i, numChains, thin, updates: INTEGER;
			elapsedTime: LONGINT;
			p: ARRAY 2 OF ARRAY 1024 OF CHAR;
	BEGIN
		BugsDialog.ShowMsg("BugsCmds:Updating");
		startTime := Services.Ticks();
		i := 0;
		updates := updateDialog.updates;
		thin := updateDialog.thin;
		overRelax := updateDialog.overRelax;
		numChains := specificationDialog.numChains;
		ok := TRUE;
		WHILE(i < updates) & ok DO
			INC(i);
			BugsInterface.UpdateModel(numChains, thin, overRelax, ok);
			IF ok THEN
				BugsInterface.UpdateMonitors(numChains);
			END
		END;
		BugsInterface.RecvMCMCState;
		SetAdaptingStatus;
		INC(updateDialog.iteration, i);
		IF ~ok THEN
			BugsDialog.ShowMsg(BugsMsg.message);
			RETURN
		END;
		elapsedTime := Services.Ticks() - startTime;
		elapsedTime := ENTIER(1.0 * elapsedTime / Services.resolution + eps);
		Strings.IntToString(updates, p[0]);
		Strings.IntToString(elapsedTime, p[1]);
		BugsDialog.ShowParamMsg("BugsCmds:UpdatesTook", p)		;
		Dialog.Update(updateDialog)
	END UpdateNI;

	PROCEDURE IsUpdating* (): BOOLEAN;
	BEGIN
		RETURN action.updating
	END IsUpdating;

	PROCEDURE SaveLog* (path: ARRAY OF CHAR);
	BEGIN
		BugsFiles.Save(path, StdLog.text)
	END SaveLog;

	PROCEDURE (a: ScriptAction) Do;
		VAR
			res: INTEGER;
	BEGIN
		res := 0;
		IF IsUpdating() OR a.s.eot THEN
			(*	if updating wait, do not read next script command	*)
		ELSE
			BugsScripting.Script(a.s, res)
		END;
		IF res = 0 THEN
			Services.DoLater(a, Services.now)
		ELSE
			BugsDialog.ShowMsg("script failed")
		END
	END Do;

	PROCEDURE Script*;
		VAR
			beg, end: INTEGER;
			s: BugsMappers.Scanner;
			text, text0: TextModels.Model;
			c: TextControllers.Controller;
	BEGIN
		text := GetText();
		IF text = StdLog.text THEN
			Dialog.ShowMsg("#Bugs:ScriptFailed");
			RETURN
		END;
		(*	if have text with high lighted region make a copy of high lighted region	*)
		c := TextControllers.Focus();
		IF c # NIL THEN
			c.GetSelection(beg, end);
			IF beg # end THEN
				text0 := TextModels.dir.New();
				text0.InsertCopy(0, text, beg, end);
				text := text0
			END
		END;
		s.ConnectToText(text);
		beg := 0;
		s.SetPos(beg);
		s.Scan;
		scriptAction.s := s;
		Services.DoLater(scriptAction, Services.now)
	END Script;

	PROCEDURE ScriptFile* (path: ARRAY OF CHAR);
		VAR
			beg: INTEGER;
			s: BugsMappers.Scanner;
			v: Views.View;
			text: TextModels.Model;
			loc: Files.Locator;
			name: Files.Name;
	BEGIN
		text := BugsFiles.FileToText(path);
		IF text # NIL THEN
			s.ConnectToText(text);
			beg := 0;
			s.SetPos(beg);
			s.Scan;
			scriptAction.s := s;
			Services.DoLater(scriptAction, Services.now)
		END
	END ScriptFile;

	PROCEDURE Metrics*;
		VAR
			tabs: ARRAY 3 OF INTEGER;
			f: TextMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		tabs[0] := 0;
		tabs[1] := 25 * Ports.mm;
		tabs[2] := 75 * Ports.mm;
		BugsFiles.WriteRuler(tabs, f);
		BugsInfo.ModelMetrics(f);
		BugsFiles.Open("Model metrics", text)
	END Metrics;

	PROCEDURE UpdatersByName*;
		VAR
			tabs: ARRAY 6 OF INTEGER;
			f: TextMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		tabs[0] := 0;
		tabs[1] := 25 * Ports.mm;
		tabs[2] := 75 * Ports.mm;
		tabs[3] := 85 * Ports.mm;
		tabs[4] := 95 * Ports.mm;
		tabs[5] := 105 * Ports.mm;
		BugsFiles.WriteRuler(tabs, f);
		BugsInfo.UpdatersByName(f);
		BugsFiles.Open("Updater types", text)
	END UpdatersByName;

	PROCEDURE UpdatersByDepth*;
		VAR
			tabs: ARRAY 6 OF INTEGER;
			f: TextMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		tabs[0] := 0;
		tabs[1] := 25 * Ports.mm;
		tabs[2] := 75 * Ports.mm;
		tabs[3] := 85 * Ports.mm;
		tabs[4] := 95 * Ports.mm;
		tabs[5] := 105 * Ports.mm;
		BugsFiles.WriteRuler(tabs, f);
		BugsInfo.UpdatersByDepth(f);
		BugsFiles.Open("Updater types", text)
	END UpdatersByDepth;

	PROCEDURE ShowDistribution*;
		VAR
			tabs: POINTER TO ARRAY OF INTEGER;
			f: TextMappers.Formatter;
			text: TextModels.Model;
			i, numChains, numProc: INTEGER;
			string: ARRAY 128 OF CHAR;
	BEGIN
		numChains := specificationDialog.numChains;
		numProc := specificationDialog.numProc;
		numProc := numProc DIV numChains;
		numProc := MAX(numProc, 1);
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		NEW(tabs, numProc + 2);
		tabs[0] := 10 * Ports.mm;
		i := 1;
		WHILE i < numProc + 1 DO
			tabs[i] := tabs[0] + i * 40 * Ports.mm;
			INC(i)
		END;
		tabs[numProc + 1] := tabs[numProc] + 20 * Ports.mm;
		BugsFiles.WriteRuler(tabs, f);
		BugsInfo.Distribute(numProc, f);
		Strings.IntToString(numProc, string);
		string := "Updaters distributed over " + string + " processors";
		BugsFiles.Open(string, text)
	END ShowDistribution;

	PROCEDURE ShowDistributionDeviance*;
		VAR
			tabs: POINTER TO ARRAY OF INTEGER;
			f: TextMappers.Formatter;
			text: TextModels.Model;
			i, numChains, numProc: INTEGER;
			string: ARRAY 128 OF CHAR;
	BEGIN
		numChains := specificationDialog.numChains;
		numProc := specificationDialog.numProc;
		numProc := numProc DIV numChains;
		numProc := MAX(numProc, 1);
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		NEW(tabs, numProc + 1);
		tabs[0] := 10 * Ports.mm;
		i := 1;
		WHILE i < numProc DO
			tabs[i] := tabs[0] + i * 40 * Ports.mm;
			INC(i)
		END;
		tabs[numProc] := tabs[numProc - 1] + 20 * Ports.mm;
		BugsFiles.WriteRuler(tabs, f);
		BugsInfo.DistributeDeviance(numProc, f);
		Strings.IntToString(numProc, string);
		string := "Deviance distributed over " + string + " processors";
		BugsFiles.Open(string, text)
	END ShowDistributionDeviance;

	PROCEDURE Values*;
		VAR
			i, max, numChains: INTEGER;
			tabs: POINTER TO ARRAY OF INTEGER;
			f: TextMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		IF BugsInfo.IsConstant(infoDialog.node.item) THEN
			numChains := 1
		ELSE
			numChains := specificationDialog.numChains
		END;
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		max := 2 + MIN(numChains, 10);
		NEW(tabs, max);
		i := 0;
		WHILE i < max DO
			tabs[i] := i * 25 * Ports.mm;
			INC(i)
		END;
		BugsFiles.WriteRuler(tabs, f);
		BugsInfo.Values(infoDialog.node.item, numChains, f);
		BugsFiles.Open("Node values", text)
	END Values;

	PROCEDURE WriteChains*;
		VAR
			i, numChains: INTEGER;
			numAsString: ARRAY 8 OF CHAR;
			f: TextMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		numChains := specificationDialog.numChains;
		i := 0;
		WHILE i < numChains DO
			text := TextModels.dir.New();
			f.ConnectTo(text);
			f.SetPos(0);
			BugsInfo.WriteChain(i, f);
			Strings.IntToString(i + 1, numAsString);
			BugsFiles.Open("chain " + numAsString, text);
			INC(i)
		END;
		BugsDialog.ShowMsg("BugsCmds:ChainsWrittenOut");
	END WriteChains;

	PROCEDURE WriteChainsToFile* (stem: ARRAY OF CHAR);
		VAR
			i, numChains: INTEGER;
			numAsString: ARRAY 8 OF CHAR;
			f: TextMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		numChains := specificationDialog.numChains;
		i := 0;
		WHILE i < numChains DO
			text := TextModels.dir.New();
			f.ConnectTo(text);
			f.SetPos(0);
			BugsInfo.WriteChain(i, f);
			Strings.IntToString(i + 1, numAsString);
			BugsFiles.Save(stem + numAsString, text);
			INC(i)
		END;
		BugsDialog.ShowMsg("BugsCmds:ChainsWrittenOut");
	END WriteChainsToFile;

	PROCEDURE WriteData*;
		CONST
			numTabs = 6;
		VAR
			f: TextMappers.Formatter;
			text: TextModels.Model;
			tabs: POINTER TO ARRAY OF INTEGER;
			i: INTEGER;
	BEGIN
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		NEW(tabs, numTabs);
		i := 0;
		WHILE i < numTabs DO
			tabs[i] := (10 + 20 * i) * Ports.mm;
			INC(i)
		END;
		BugsFiles.WriteRuler(tabs, f);
		BugsInfo.WriteData(f);
		BugsFiles.Open("Data ", text);
		BugsDialog.ShowMsg("BugsCmds:DataOut");
	END WriteData;

	PROCEDURE WriteUninitNodes*;
		VAR
			i, numChains: INTEGER;
			numAsString: ARRAY 8 OF CHAR;
			f: TextMappers.Formatter;
			text: TextModels.Model;
			tabs: ARRAY 2 OF INTEGER;
	BEGIN
		tabs[0] := 0;
		tabs[1] := 50 * Ports.mm;
		numChains := specificationDialog.numChains;
		i := 0;
		WHILE i < numChains DO
			text := TextModels.dir.New();
			f.ConnectTo(text);
			f.SetPos(0);
			BugsFiles.WriteRuler(tabs, f);
			BugsInfo.WriteUninitNodes(i, f);
			Strings.IntToString(i + 1, numAsString);
			BugsFiles.Open("uninitialized nodes for chain " + numAsString, text);
			INC(i)
		END;
		BugsDialog.ShowMsg("BugsCmds:UninitializedNodes");
	END WriteUninitNodes;

	PROCEDURE MAP*;
		VAR
			f: TextMappers.Formatter;
			text: TextModels.Model;
			i, dim, numTabs: INTEGER;
			stochastics: GraphStochastic.Vector;
			tabs: POINTER TO ARRAY OF INTEGER;
	BEGIN
		stochastics := BugsGraph.ConditionalsOfClass({GraphRules.genDiff.. GraphRules.mVNLin});
		dim := LEN(stochastics);
		numTabs := dim + 5;
		NEW(tabs, numTabs);
		tabs[0] := 0;
		tabs[1] := 25 * Ports.mm;
		i := 2;
		WHILE i < numTabs DO
			tabs[i] := tabs[1] + 20 * (i - 1) * Ports.mm;
			INC(i)
		END;
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		BugsFiles.WriteRuler(tabs, f);
		BugsMAP.Estimates(stochastics, f);
		BugsFiles.Open("Maximum a posteri estimates", text)
	END MAP;

	PROCEDURE Modules*;
		CONST
			numTabs = 8;
		VAR
			i: INTEGER;
			tabs: ARRAY numTabs OF INTEGER;
			f: TextMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		tabs[0] := 0;
		tabs[1] := 40 * Ports.mm;
		i := 2;
		WHILE i < numTabs DO
			tabs[i] := tabs[1] + 20 * Ports.mm * i;
			INC(i)
		END;
		BugsFiles.WriteRuler(tabs, f);
		BugsVersion.Modules(f);
		BugsFiles.Open("Modules", text)
	END Modules;

	PROCEDURE PrintModel*;
		VAR
			f: TextMappers.Formatter;
			text: TextModels.Model;
			tabs: ARRAY 15 OF INTEGER;
			i: INTEGER;
	BEGIN
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		i := 0; WHILE i < LEN(tabs) DO tabs[i] := (i + 1) * 5 * Ports.mm; INC(i) END;
		tabs[i - 1] := 200 * Ports.mm;
		BugsFiles.WriteRuler(tabs, f);
		BugsPrettyprinter.DisplayCode(BugsParser.model, f);
		BugsFiles.Open("Bugs language model", text)
	END PrintModel;

	PROCEDURE PrintLatex*;
		VAR
			f: TextMappers.Formatter;
			text: TextModels.Model;
			tabs: ARRAY 2 OF INTEGER;
	BEGIN
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		tabs[0] := 0; tabs[1] := 200 * Ports.mm;
		BugsFiles.WriteRuler(tabs, f);
		BugsLatexprinter.DisplayCode(BugsParser.model, f);
		BugsFiles.Open("Bugs model in latex", text)
	END PrintLatex;

	PROCEDURE AllocatedMemory*;
		VAR
			msg, hundreds, thousands, millions: ARRAY 128 OF CHAR;
			alloc, int: INTEGER;
	BEGIN
		alloc := BugsInterface.AllocatedMemory();
		int := alloc MOD 1000;
		Strings.IntToString(int, hundreds);
		IF LEN(hundreds$) = 2 THEN hundreds := "0" + hundreds
		ELSIF LEN(hundreds$) = 1 THEN hundreds := "00" + hundreds
		END;
		int := alloc DIV 1000;
		int := int MOD 1000;
		Strings.IntToString(int, thousands);
		IF LEN(thousands$) = 2 THEN thousands := "0" + thousands
		ELSIF LEN(thousands$) = 1 THEN thousands := "00" + thousands
		END;
		int := alloc DIV 1000000;
		Strings.IntToString(int, millions);
		msg := millions + " " + thousands + " " + hundreds;
		msg := msg + " bytes of memory allocated";
		BugsDialog.ShowMsg(msg)
	END AllocatedMemory;

	PROCEDURE NumberOfBugFiles (): INTEGER;
		VAR
			loc: Files.Locator;
			fileList: Files.FileInfo;
			num, pos: INTEGER;
	BEGIN
		loc := BugsSerialize.GetRestartLoc();
		fileList := Files.dir.FileList(loc);
		num := 0;
		WHILE fileList # NIL DO
			IF fileList.type = "bug" THEN
				Strings.Find(fileList.name, "bug_", 0, pos);
				IF pos = - 1 THEN INC(num) END
			END;
			fileList := fileList.next
		END;
		RETURN num
	END NumberOfBugFiles;

	PROCEDURE GetBugFiles (): POINTER TO ARRAY OF Dialog.String;
		VAR
			i, len, num, pos: INTEGER;
			loc: Files.Locator;
			fileList: Files.FileInfo;
			bugFileNames: POINTER TO ARRAY OF Dialog.String;
	BEGIN
		num := NumberOfBugFiles();
		IF num > 0 THEN
			NEW(bugFileNames, num)
		ELSE
			bugFileNames := NIL
		END;
		loc := BugsSerialize.GetRestartLoc();
		fileList := Files.dir.FileList(loc);
		i := 0;
		WHILE fileList # NIL DO
			IF fileList.type = "bug" THEN
				Strings.Find(fileList.name, "bug_", 0, pos);
				IF pos = - 1 THEN
					bugFileNames[i] := fileList.name$;
					len := LEN(fileList.name$);
					bugFileNames[i, len - 4] := 0X;
					INC(i)
				END
			END;
			fileList := fileList.next
		END;
		RETURN bugFileNames
	END GetBugFiles;

	PROCEDURE UpdateBugFileNames;
		VAR
			i, num: INTEGER;
			bugFileNames: POINTER TO ARRAY OF Dialog.String;
	BEGIN
		bugFileNames := GetBugFiles();
		IF bugFileNames # NIL THEN
			num := LEN(bugFileNames, 0)
		ELSE
			num := 0
		END;
		i := 0;
		WHILE i < num DO
			inBugDialog.name.SetItem(i, bugFileNames[i]);
			outBugDialog.name.SetItem(i, bugFileNames[i]);
			INC(i)
		END;
		inBugDialog.name.SetLen(num);
		Dialog.UpdateList(inBugDialog);
		outBugDialog.name.SetLen(num);
		Dialog.UpdateList(outBugDialog)
	END UpdateBugFileNames;

	PROCEDURE ExternalizeModel* (fileName: ARRAY OF CHAR);
	BEGIN
		BugsSerialize.Externalize(fileName);
	END ExternalizeModel;

	PROCEDURE InternalizeModel* (fileName: ARRAY OF CHAR);
		VAR
			loc: Files.Locator;
			f: Files.File;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		loc := BugsSerialize.restartLoc;
		f := Files.dir.Old(loc, fileName + ".bug", Files.shared);
		IF f = NIL THEN
			p[0] := fileName$;
			BugsDialog.ShowParamMsg("BugsCmds:NoFile", p);
			RETURN
		END;
		f.Close;
		BugsSerialize.Internalize(fileName);
		specificationDialog.numChains := BugsRandnum.numberChains;
		SetIteration(UpdaterActions.iteration);
		updateDialog.isAdapting := UpdaterActions.endOfAdapting = MAX(INTEGER);
		UpdaterSettings.MarkCompiled;
		UpdaterSettings.FillDialog;
		BugsDialog.UpdateDialogs;
		BugsDialog.ShowMsg("modelInternalized");
	END InternalizeModel;

	PROCEDURE Externalize*;
		VAR
			bugFileName: Dialog.String;
	BEGIN
		bugFileName := outBugDialog.name.item$;
		BugsSerialize.Externalize(bugFileName);
		BugsDialog.ShowMsg("model externalized to file ok")
	END Externalize;

	PROCEDURE Internalize*;
		VAR
			ok: BOOLEAN;
			index: INTEGER;
			name: Dialog.String;
	BEGIN
		NewModel(ok);
		IF ~ok THEN RETURN END;
		Clear;
		index := inBugDialog.name.index;
		inBugDialog.name.GetItem(index, name);
		BugsSerialize.Internalize(name);
		specificationDialog.numChains := BugsRandnum.numberChains;
		SetIteration(UpdaterActions.iteration);
		updateDialog.isAdapting := UpdaterActions.endOfAdapting = MAX(INTEGER);
		UpdaterSettings.MarkCompiled;
		UpdaterSettings.FillDialog;
		BugsDialog.UpdateDialogs;
		BugsDialog.ShowMsg("model internalized from file ok")
	END Internalize;

	PROCEDURE Quit* (option: ARRAY OF CHAR);
		VAR
			i, res: INTEGER;
	BEGIN
		i := 0; WHILE i < LEN(option) DO option[i] := CAP(option[i]); INC(i) END;
		IF (option = "YES") OR (option = "Y") THEN
			HostMenus.Exit
		ELSE
			Dialog.GetOK("Quit OpenBUGS?", "", "", "", {Dialog.ok, Dialog.cancel}, res);
			IF res = Dialog.ok THEN
				HostMenus.Exit
			END
		END
	END Quit;


	PROCEDURE ChangeSampler* (node: ARRAY OF CHAR);
		VAR
			ok: BOOLEAN;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		BugsInterface.ChangeSampler(node, UpdaterMethods.index, ok);
		IF ~ok THEN
			p[0] := node$;
			BugsDialog.ShowParamMsg("BugsCmds:couldNotChangeUpdater", p);
		END
	END ChangeSampler;

	PROCEDURE SetFactory* (updateMethod: ARRAY OF CHAR);
		VAR
			ok: BOOLEAN;
	BEGIN
		UpdaterMethods.SetFactory(updateMethod);
		UpdaterMethods.FactoryGuard(ok);
		IF ok THEN
			UpdaterSettings.dialog.methods.index := UpdaterMethods.index;
			Dialog.UpdateList(UpdaterSettings.dialog.methods);
			Dialog.Update(UpdaterSettings.dialog)
		END
	END SetFactory;

	(*	User interface code	*)

	PROCEDURE (dialogBox: CompileDialog) Init-;
	BEGIN
	END Init;

	PROCEDURE (dialogBox: CompileDialog) Update-;
	BEGIN
		Dialog.Update(compileDialog)
	END Update;

	PROCEDURE (dialogBox: DisplayDialog) Init-;
	BEGIN
		displayDialog.whereOut := BugsFiles.window;
		BugsFiles.SetDest(BugsFiles.window)
	END Init;

	PROCEDURE (dialogBox: DisplayDialog) Update-;
	BEGIN
		Dialog.Update(displayDialog)
	END Update;

	PROCEDURE (dialogBox: InfoDialog) Init-;
	BEGIN
		infoDialog.node.item := "";
		infoDialog.node.SetLen(1);
		infoDialog.node.SetItem(0, " ")
	END Init;

	PROCEDURE UpdateNames;
		VAR
			i, len: INTEGER;
			names: POINTER TO ARRAY OF BugsNames.Name;
	BEGIN
		names := BugsIndex.GetNames();
		IF names # NIL THEN
			len := LEN(names)
		ELSE
			len := 0
		END;
		i := 0;
		WHILE i < len DO
			infoDialog.node.SetItem(i, names[i].string);
			INC(i)
		END;
		infoDialog.node.SetLen(len);
		infoDialog.node.item := ""
	END UpdateNames;

	PROCEDURE (dialogBox: InfoDialog) Update-;
	BEGIN
		UpdateNames;
		Dialog.UpdateList(dialogBox.node);
		Dialog.Update(dialogBox)
	END Update;

	PROCEDURE (dialogBox: RNDialog) Init-;
	BEGIN
		dialogBox.preSet := 1
	END Init;

	PROCEDURE (dialogBox: RNDialog) Update-;
	BEGIN
		Dialog.Update(rnDialog)
	END Update;

	PROCEDURE (dialogBox: SpecificationDialog) Init-;
	BEGIN
		dialogBox.numChains := defaultNumChains;
		dialogBox.chain := 1;
		dialogBox.numProc := 2;
		dialogBox.fixFounder := TRUE
	END Init;

	PROCEDURE (dialogBox: SpecificationDialog) Update-;
	BEGIN
		Dialog.Update(dialogBox)
	END Update;

	PROCEDURE (dialogBox: UpdateDialog) Update-;
	BEGIN
		Dialog.Update(dialogBox)
	END Update;

	PROCEDURE (dialogBox: UpdateDialog) Init-;
	BEGIN
		dialogBox.iteration := 0;
		dialogBox.updates := 1000;
		dialogBox.refresh := defaultRefresh;
		dialogBox.thin := 1;
		dialogBox.overRelax := FALSE;
		dialogBox.isAdapting := FALSE
	END Init;

	PROCEDURE (dialog: InBugDialog) Update-;
	BEGIN
		UpdateBugFileNames
	END Update;

	PROCEDURE (dialog: InBugDialog) Init-;
	BEGIN
		UpdateBugFileNames
	END Init;

	PROCEDURE (dialog: OutBugDialog) Update-;
	BEGIN
		UpdateBugFileNames
	END Update;

	PROCEDURE (dialog: OutBugDialog) Init-;
	BEGIN
		UpdateBugFileNames;
		dialog.name.item := ""
	END Init;

	PROCEDURE SetRNNotifier* (op, from, to: INTEGER);
	BEGIN
		IF (BugsRandnum.generators # NIL) & (BugsRandnum.generators[0] # NIL) THEN
			numberPresets := MathRandnum.NumPresets(BugsRandnum.generators[0])
		END;
		rnDialog.preSet := MAX(1, rnDialog.preSet);
		rnDialog.preSet := MIN(numberPresets, rnDialog.preSet);
		BugsRandnum.SetShift(rnDialog.preSet - 1);
		BugsRandnum.CreateGenerators(BugsRandnum.numberChains);
		Dialog.Update(rnDialog)
	END SetRNNotifier;

	PROCEDURE ChainNotifier* (op, from, to: INTEGER);
	BEGIN
		IF specificationDialog.chain < 1 THEN
			specificationDialog.chain := 1
		ELSIF specificationDialog.chain > specificationDialog.numChains THEN
			specificationDialog.chain := specificationDialog.numChains
		END
	END ChainNotifier;

	PROCEDURE PrecisionNotifier* (op, from, to: INTEGER);
	BEGIN
		displayDialog.precision := MAX(2, displayDialog.precision);
		displayDialog.precision := MIN(displayDialog.precision, 10);
		BugsFiles.SetPrec(displayDialog.precision)
	END PrecisionNotifier;

	PROCEDURE DisplayNotifier* (op, from, to: INTEGER);
	BEGIN
		BugsFiles.SetDest(displayDialog.whereOut)
	END DisplayNotifier;

	PROCEDURE CompileGuardWin* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~BugsInterface.IsParsed() OR BugsInterface.IsCompiled()
	END CompileGuardWin;

	PROCEDURE CompiledGuardWin* (VAR par: Dialog.Par);
	BEGIN
		par.readOnly := BugsInterface.IsCompiled()
	END CompiledGuardWin;

	PROCEDURE CompileCPGuardWin* (VAR par: Dialog.Par);
		VAR
			mod: Meta.Item;
	BEGIN
		Meta.Lookup("StdLoader", mod);
		par.readOnly := mod.obj # Meta.modObj
	END CompileCPGuardWin;

	PROCEDURE DevianceGuardWin* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~BugsInterface.IsInitialized() OR (BugsIndex.Find("deviance") = NIL)
	END DevianceGuardWin;

	PROCEDURE DistributeGuardWin* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := BugsInterface.IsDistributed() OR (~BugsInterface.IsInitialized())
	END DistributeGuardWin;

	PROCEDURE GenerateInitsGuardWin* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~BugsInterface.IsCompiled();
		par.disabled := par.disabled OR BugsInterface.IsInitialized()
	END GenerateInitsGuardWin;

	PROCEDURE LoadDataGuardWin* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~BugsInterface.IsParsed();
		IF ~par.disabled THEN
			TextCmds.FocusGuard(par)
		END
	END LoadDataGuardWin;

	PROCEDURE LoadInitsGuardWin* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~BugsInterface.IsCompiled() OR BugsInterface.IsDistributed();
		IF ~par.disabled THEN
			TextCmds.FocusGuard(par)
		END
	END LoadInitsGuardWin;

	PROCEDURE NotCompiledGuardWin* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~BugsInterface.IsCompiled()
	END NotCompiledGuardWin;

	PROCEDURE ParseGuardWin* (VAR par: Dialog.Par);
	BEGIN
		TextCmds.FocusGuard(par)
	END ParseGuardWin;

	PROCEDURE ParsedGuardWin* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~BugsInterface.IsParsed()
	END ParsedGuardWin;

	PROCEDURE SetLogGuard* (VAR par: Dialog.Par);
	BEGIN
		par.checked := displayDialog.whereOut = BugsFiles.log
	END SetLogGuard;

	PROCEDURE SetWindowGuard* (VAR par: Dialog.Par);
	BEGIN
		par.checked := displayDialog.whereOut = BugsFiles.window
	END SetWindowGuard;

	PROCEDURE UpdateGuardWin* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~BugsInterface.IsInitialized()
	END UpdateGuardWin;

	PROCEDURE SetRNGuardWin* (VAR par: Dialog.Par);
	BEGIN
		par.readOnly := ~BugsInterface.IsCompiled() OR BugsInterface.IsInitialized()
	END SetRNGuardWin;

	PROCEDURE MAPGuardWin* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~BugsInterface.IsInitialized() OR (GraphNodes.maxStochDepth > 1)
	END MAPGuardWin;

	PROCEDURE DevianceGuard* (OUT ok: BOOLEAN);
	BEGIN
		ok := BugsIndex.Find("deviance") # NIL;
		IF ~ok THEN
			BugsDialog.ShowMsg("BugsCmds:NoDeviance")
		END
	END DevianceGuard;

	PROCEDURE ParseGuard* (OUT ok: BOOLEAN);
		VAR
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		ok := BugsFiles.FileToText(filePath) # NIL;
		IF ~ok THEN
			p[0] := filePath$;
			BugsDialog.ShowParamMsg("BugsCmds:NoFile", p);
		END
	END ParseGuard;

	PROCEDURE ParsedGuard* (OUT ok: BOOLEAN);
	BEGIN
		ok := BugsInterface.IsParsed();
		IF ~ok THEN
			BugsDialog.ShowMsg("BugsCmds:NoCheckData")
		END
	END ParsedGuard;

	PROCEDURE LoadDataGuard* (OUT ok: BOOLEAN);
		VAR
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		ok := BugsFiles.FileToText(filePath) # NIL;
		IF ~ok THEN
			p[0] := filePath$;
			BugsDialog.ShowParamMsg("BugsCmds:NoFile", p);
		ELSE
			ok := BugsInterface.IsParsed();
			IF ~ok THEN
				BugsDialog.ShowMsg("BugsCmds:NoCheckData");
			END
		END
	END LoadDataGuard;

	PROCEDURE CompileGuard* (OUT ok: BOOLEAN);
	BEGIN
		ok := BugsInterface.IsParsed();
		IF ~ok THEN
			BugsDialog.ShowMsg("BugsCmds:NoCheckCompile");
		END
	END CompileGuard;

	PROCEDURE DistributeGuard* (OUT ok: BOOLEAN);
	BEGIN
		ok := ~BugsInterface.IsDistributed() & BugsInterface.IsInitialized();
		IF ~ok THEN
			BugsDialog.ShowMsg("BugsCmds:UnableToDistribute");
		END
	END DistributeGuard;

	PROCEDURE NotCompiledGuard* (OUT ok: BOOLEAN);
	BEGIN
		ok := ~BugsInterface.IsCompiled();
		IF ~ok THEN
			BugsDialog.ShowMsg("BugsCmds:NotCompiled");
		END
	END NotCompiledGuard;

	PROCEDURE LoadInitsGuard* (OUT ok: BOOLEAN);
		VAR
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		ok := BugsFiles.FileToText(filePath) # NIL;
		IF ~ok THEN
			p[0] := filePath$;
			BugsDialog.ShowParamMsg("BugsCmds:NoFile", p);
		ELSE
			ok := BugsInterface.IsCompiled();
			IF ~ok THEN
				BugsDialog.ShowMsg("BugsCmds:NoCompileInits");
			END
		END
	END LoadInitsGuard;

	PROCEDURE GenerateInitsGuard* (OUT ok: BOOLEAN);
	BEGIN
		ok := BugsInterface.IsCompiled();
		IF ~ok THEN
			BugsDialog.ShowMsg("BugsCmds:NoCompileGenInits");
		ELSE
			ok := ~BugsInterface.IsInitialized();
			IF ~ok THEN
				BugsDialog.ShowMsg("BugsCmds:AlreadyInits");
			END
		END
	END GenerateInitsGuard;

	PROCEDURE UpdateGuard* (OUT ok: BOOLEAN);
	BEGIN
		ok := BugsInterface.IsInitialized();
		IF ~ok THEN
			BugsDialog.ShowMsg("BugsCmds:NotInits");
		END
	END UpdateGuard;

	PROCEDURE SetRNGuard* (OUT ok: BOOLEAN);
		VAR
			s: ARRAY 1024 OF CHAR;
	BEGIN
		ok := BugsInterface.IsCompiled() & ~BugsInterface.IsInitialized();
		IF ~ok THEN
			s := "model must have been compiled but not initialized to be able to change RN generator";
			BugsDialog.ShowMsg(s)
		END
	END SetRNGuard;

	PROCEDURE FactoryGuard* (OUT ok: BOOLEAN);
	BEGIN
		UpdaterMethods.FactoryGuard(ok);
		IF ~ok THEN
			BugsDialog.ShowMsg("UpdaterMethods:notUpdateMethod");
		END
	END FactoryGuard;

	PROCEDURE AdaptivePhaseGuard* (OUT ok: BOOLEAN);
	BEGIN
		UpdaterMethods.AdaptivePhaseGuard(ok);
		IF ~ok THEN
			BugsDialog.ShowMsg("UpdaterMethods:notAdaptive");
		END
	END AdaptivePhaseGuard;

	PROCEDURE IterationsGuard* (OUT ok: BOOLEAN);
	BEGIN
		UpdaterMethods.IterationsGuard(ok);
		IF ~ok THEN
			BugsDialog.ShowMsg("UpdaterMethods.notIterations");
		END
	END IterationsGuard;

	PROCEDURE OverRelaxationGuard* (OUT ok: BOOLEAN);
	BEGIN
		UpdaterMethods.OverRelaxationGuard(ok);
		IF ~ok THEN
			BugsDialog.ShowMsg("UpdaterMethods:notOverRelax");
		END
	END OverRelaxationGuard;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			mod: Meta.Item;
	BEGIN
		Maintainer;
		numberPresets := 1;
		NEW(action);
		NEW(scriptAction);
		NEW(compileDialog);
		BugsDialog.AddDialog(compileDialog);
		compileDialog.updaterByMethod := TRUE;
		NEW(displayDialog);
		BugsDialog.AddDialog(displayDialog);
		displayDialog.whereOut := BugsFiles.whereOut;
		displayDialog.precision := BugsFiles.prec;
		NEW(infoDialog);
		BugsDialog.AddDialog(infoDialog);
		NEW(rnDialog);
		BugsDialog.AddDialog(rnDialog);
		NEW(specificationDialog);
		SetDefaultNumChains(1);
		BugsDialog.AddDialog(specificationDialog);
		SetDefaultRefresh(100);
		NEW(updateDialog);
		BugsDialog.AddDialog(updateDialog);
		NEW(inBugDialog);
		BugsDialog.AddDialog(inBugDialog);
		NEW(outBugDialog);
		BugsDialog.AddDialog(outBugDialog);
		BugsDialog.InitDialogs;
		BugsDialog.UpdateDialogs;
		DevDebug.MapHex := BugsIndex.MapGraphAddress;
		(*	load simplified trap handler if present	*)
		Meta.Lookup("BugsTraphandler", mod);
		TextCmds.find.ignoreCase := FALSE;
		TextCmds.find.find := ""
	END Init;

BEGIN
	Init
CLOSE
	BugsInterface.Clear
END BugsCmds.

