(* 	

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsCmds;

	

	IMPORT
		Controllers, Converters, Dialog, Files, Meta, Models, Ports, Services, Strings, Views,
		BugsDialog, BugsFiles, BugsGraph, BugsIndex, BugsInfo, BugsInterface, BugsLatexprinter,
		BugsMAP, BugsMappers, BugsMsg, BugsNames, BugsParser, BugsPrettyprinter,
		BugsRandnum, BugsScripting, BugsSerialize, BugsTexts, BugsVersion,
		DevDebug,
		GraphNodes, GraphRules, GraphStochastic,
		HostMenus,
		MathRandnum,
		MonitorMonitors,
		StdLog,
		TextCmds, TextControllers, TextModels, TextViews,
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

	PROCEDURE SetDisplay* (option: ARRAY OF CHAR);
	BEGIN
		IF option = "log" THEN
			displayDialog.whereOut := BugsMappers.log;
			BugsMappers.SetDest(BugsMappers.log);
			StdLog.Open
		ELSIF option = "window" THEN
			displayDialog.whereOut := BugsMappers.window;
			BugsMappers.SetDest(BugsMappers.window)
		END
	END SetDisplay;

	PROCEDURE GetRNState*;
		VAR
			string: ARRAY 1024 OF CHAR;
	BEGIN
		Strings.IntToString(rnDialog.preSet, string);
		BugsTexts.ShowMsg("new state is " + string)
	END GetRNState;

	PROCEDURE SetRNState* (preSet: INTEGER);
		CONST
			numberPresetStates = 14;
	BEGIN
		IF preSet < 1 THEN
			preSet := 1
		ELSIF preSet > numberPresetStates THEN
			preSet := numberPresetStates
		END;
		MathRandnum.InitState(preSet - 1);
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

	PROCEDURE File (path: ARRAY OF CHAR): Files.File;
		VAR
			file: Files.File;
			name: Files.Name;
			loc: Files.Locator;
			pos: INTEGER;
	BEGIN
		BugsFiles.PathToFileSpec(path, loc, name);
		Strings.Find(name, ".txt", 0, pos);
		IF pos =  - 1 THEN
			Strings.Find(name, ".odc", 0, pos);
			IF pos =  - 1 THEN
				name := name + ".odc"
			END
		END;
		file := Files.dir.Old(loc, name, Files.shared);
		RETURN file
	END File;

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
			BugsFiles.PathToFileSpec(filePath, loc, name);
			Strings.Find(name, ".txt", 0, pos);
			IF pos =  - 1 THEN
				v := Views.OldView(loc, name)
			ELSE
				conv := Converters.list;
				WHILE (conv # NIL) & (conv.fileType # "txt") DO conv := conv.next END;
				v := Views.Old(Views.dontAsk, loc, name, conv)
			END;
			IF v # NIL THEN
				IF v IS TextViews.View THEN
					m := v.ThisModel();
					IF m # NIL THEN
						IF m IS TextModels.Model THEN
							text := m(TextModels.Model)
						ELSE
							text := NIL
						END
					ELSE
						text := NIL
					END
				ELSE
					text := NIL
				END
			ELSE
				text := NIL
			END
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
		BugsMsg.GetError(errorMes);
		BugsTexts.ShowMsg(errorMes);
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
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		IF filePath = "" THEN
			NewModel(ok);
			IF ~ok THEN filePath := ""; RETURN END
		END;
		Clear;
		BugsDialog.UpdateDialogs;
		text := GetText();
		IF text = NIL THEN filePath := ""; RETURN END;
		BugsTexts.ConnectScanner(s, text);
		pos := GetPos();
		s.SetPos(pos);
		BugsInterface.ParseModel(s);
		IF ~BugsInterface.IsParsed() THEN TextError(s, text); filePath := ""; RETURN END;
		BugsDialog.UpdateDialogs;
		BugsMsg.MapMsg("BugsCmds:OkSyntax", msg);
		BugsTexts.ShowMsg(msg);
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
		BugsMsg.StoreMsg("");
		IF ~BugsInterface.IsParsed() THEN filePath := ""; RETURN END;
		text := GetText();
		IF text = NIL THEN filePath := ""; RETURN END;
		BugsTexts.ConnectScanner(s, text);
		pos := GetPos();
		s.SetPos(pos);
		BugsInterface.LoadData(s, ok);
		IF ~ok THEN TextError(s, text); filePath := ""; RETURN END;
		BugsMsg.MapMsg("BugsCmds:OkData", msg);
		BugsMsg.GetMsg(warning);
		(* If warning not encountered, will contain empty string *)
		BugsTexts.ShowMsg(msg + warning);
		filePath := ""
	END LoadData;

	(*	Commands to write model graph	*)

	PROCEDURE SetAdaptingStatus*;
	BEGIN
		updateDialog.isAdapting := UpdaterActions.endOfAdapting = MAX(INTEGER)
	END SetAdaptingStatus;

	PROCEDURE Compile*;
		VAR
			updaterByMethod: BOOLEAN;
			numChains: INTEGER;
			msg: ARRAY 1024 OF CHAR;
			startTime, elapsedTime: LONGINT;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
		CONST
			eps = 1.0E-20;
	BEGIN
		IF ~BugsInterface.IsParsed() THEN RETURN END;
		numChains := specificationDialog.numChains;
		updaterByMethod := compileDialog.updaterByMethod;
		startTime := Services.Ticks();
		BugsGraph.Compile(numChains, updaterByMethod);
		IF BugsInterface.IsCompiled() THEN
			SetAdaptingStatus;
			UpdaterSettings.MarkCompiled;
			UpdaterSettings.FillDialog;
			BugsDialog.UpdateDialogs;
			elapsedTime := Services.Ticks() - startTime;
			elapsedTime := ENTIER(1.0 * elapsedTime / Services.resolution + eps);
			Strings.IntToString(elapsedTime, p[0]);
			BugsMsg.MapParamMsg("BugsCmds:OkCompile", p, msg);
			BugsTexts.ShowMsg(msg);
		END;
		IF ~BugsInterface.IsCompiled() THEN
			BugsMsg.GetError(msg); BugsTexts.ShowMsg(msg); RETURN
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
		BugsMsg.StoreMsg("");
		NewInits(ok);
		IF ~ok THEN filePath := ""; RETURN END;
		text := GetText();
		IF text = NIL THEN filePath := ""; RETURN END;
		BugsTexts.ConnectScanner(s, text);
		pos := GetPos();
		s.SetPos(pos);
		chain := specificationDialog.chain;
		numChains := specificationDialog.numChains;
		fixFounder := specificationDialog.fixFounder;
		BugsInterface.LoadInits(s, chain - 1, numChains, fixFounder, ok);
		IF ~ok THEN
			IF UpdaterActions.IsInitialized(chain - 1) THEN
				BugsMsg.GetError(msg);
				BugsTexts.ShowMsg(msg)
			ELSE
				TextError(s, text)
			END;
			filePath := ""; RETURN
		END;
		IF BugsInterface.IsInitialized() THEN
			BugsMsg.MapMsg("BugsCmds:OkInits", msg)
		ELSIF UpdaterActions.IsInitialized(chain - 1) THEN
			BugsMsg.MapMsg("BugsCmds:UninitOther", msg)
		ELSE
			BugsMsg.MapMsg("BugsCmds:NotInit", msg)
		END;
		BugsMsg.GetMsg(warning);
		BugsTexts.ShowMsg(msg + warning);
		SetChain(chain + 1);
		Dialog.Update(specificationDialog);
		filePath := ""
	END LoadInits;

	(*	Commands to generate initial values	*)

	PROCEDURE GenerateInits*;
		VAR
			msg: ARRAY 1024 OF CHAR;
			numChains: INTEGER;
			fixFounder, ok: BOOLEAN;
	BEGIN
		IF ~BugsInterface.IsCompiled() THEN RETURN END;
		numChains := specificationDialog.numChains;
		fixFounder := specificationDialog.fixFounder;
		BugsInterface.GenerateInits(numChains, fixFounder, ok);
		IF ok THEN
			BugsMsg.MapMsg("BugsCmds:OkGenInits", msg);
			BugsTexts.ShowMsg(msg)
		ELSE
			BugsMsg.GetError(msg);
			BugsTexts.ShowMsg(msg);
			RETURN
		END
	END GenerateInits;

	PROCEDURE Distribute* (mpiImplementation: ARRAY OF CHAR);
		VAR
			msg: ARRAY 1024 OF CHAR;
			numProc, numChains: INTEGER;
	BEGIN
		BugsTexts.ShowMsg("");
		numProc := specificationDialog.numProc;
		numChains := specificationDialog.numChains;
		BugsInterface.Distribute(mpiImplementation, numProc, numChains);
		IF BugsInterface.IsDistributed() THEN
			BugsMsg.StoreMsg("model distributed")
		END;
		BugsMsg.GetError(msg);
		BugsTexts.ShowMsg(msg)
	END Distribute;

	PROCEDURE DistributeInfo*;
		VAR
			tabs: ARRAY 6 OF INTEGER;
			f: BugsMappers.Formatter;
			text: TextModels.Model;
		CONST
			space = 30* Ports.mm;
	BEGIN
		text := TextModels.dir.New();
		BugsTexts.ConnectFormatter(f, text);
		f.SetPos(0);
		tabs[0] := 0;
		tabs[1] := 5 * Ports.mm;
		tabs[2] := tabs[1] + space;
		tabs[3] := tabs[2] + space;
		tabs[4] := tabs[3] + space;
		tabs[5] := tabs[4] + space;
		f.WriteRuler(tabs);
		BugsInfo.DistributeInfo(f);
		f.Register("Distribution info")
	END DistributeInfo;

	PROCEDURE (a: Action) Do;
		CONST
			eps = 1.0E-20;
		VAR
			ok, overRelax: BOOLEAN;
			i, numChains, refresh, thin, updates: INTEGER;
			elapsedTime: LONGINT;
			msg: ARRAY 1024 OF CHAR;
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
			BugsMsg.GetError(msg);
			BugsTexts.ShowMsg(msg);
			RETURN
		END;
		MonitorMonitors.UpdateDrawers;
		SetAdaptingStatus;
		Dialog.Update(updateDialog);
		IF a.updates < updates THEN
			Services.DoLater(a, Services.now)
		ELSE
			BugsInterface.RecvSamples;
			BugsInterface.LoadDeviance(0);
			a.updating := FALSE;
			elapsedTime := Services.Ticks() - startTime;
			elapsedTime := ENTIER(1.0 * elapsedTime / Services.resolution + eps);
			Strings.IntToString(updates, p[0]);
			Strings.IntToString(elapsedTime, p[1]);
			BugsMsg.MapParamMsg("BugsCmds:UpdatesTook", p, msg);
			BugsTexts.ShowMsg(msg)
		END
	END Do;

	PROCEDURE Update*;
		VAR
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		IF ~action.updating THEN
			action.updating := TRUE;
			startTime := Services.Ticks();
			BugsMsg.MapMsg("BugsCmds:Updating", msg);
			BugsTexts.ShowMsg(msg);
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
			msg: ARRAY 1024 OF CHAR;
			p: ARRAY 2 OF ARRAY 1024 OF CHAR;
	BEGIN
		BugsMsg.MapMsg("BugsCmds:Updating", msg);
		BugsTexts.ShowMsg(msg);
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
		BugsInterface.RecvSamples;
		SetAdaptingStatus;
		INC(updateDialog.iteration, i);
		IF ~ok THEN
			BugsMsg.GetError(msg); BugsTexts.ShowMsg(msg); RETURN
		END;
		elapsedTime := Services.Ticks() - startTime;
		elapsedTime := ENTIER(1.0 * elapsedTime / Services.resolution + eps);
		Strings.IntToString(updates, p[0]);
		Strings.IntToString(elapsedTime, p[1]);
		BugsMsg.MapParamMsg("BugsCmds:UpdatesTook", p, msg)		;
		BugsTexts.ShowMsg(msg);
		Dialog.Update(updateDialog)
	END UpdateNI;

	PROCEDURE IsUpdating* (): BOOLEAN;
	BEGIN
		RETURN action.updating
	END IsUpdating;

	PROCEDURE SaveLog* (path: ARRAY OF CHAR);
		VAR
			pos, res: INTEGER;
			loc: Files.Locator;
			name: Files.Name;
			v: Views.View;
			conv: Converters.Converter;
	BEGIN
		v := TextViews.dir.New(StdLog.text);
		BugsFiles.PathToFileSpec(path, loc, name);
		Strings.Find(name, ".txt", 0, pos);
		IF pos =  - 1 THEN
			Views.RegisterView(v, loc, name)
		ELSE
			conv := Converters.list;
			WHILE (conv # NIL) & (conv.fileType # "txt") DO conv := conv.next END;
			Views.Register(v, Views.dontAsk, loc, name, conv, res)
		END
	END SaveLog;

	PROCEDURE (a: ScriptAction) Do;
	BEGIN
		IF IsUpdating() OR a.s.eot THEN
			(*	if updating wait, do not read next script command	*)
		ELSE
			BugsScripting.Script(a.s)
		END;
		Services.DoLater(a, Services.now)
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
		BugsTexts.ConnectScanner(s, text);
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
		BugsFiles.PathToFileSpec(path, loc, name);
		v := Views.OldView(loc, name);
		IF v # NIL THEN
			WITH v: TextViews.View DO
				text := v.ThisModel();
				BugsTexts.ConnectScanner(s, text);
				beg := 0;
				s.SetPos(beg);
				s.Scan;
				scriptAction.s := s;
				Services.DoLater(scriptAction, Services.now)
			ELSE
			END
		END
	END ScriptFile;

	PROCEDURE Methods*;
		VAR
			tabs: ARRAY 3 OF INTEGER;
			f: BugsMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		text := TextModels.dir.New();
		BugsTexts.ConnectFormatter(f, text);
		f.SetPos(0);
		tabs[0] := 0;
		tabs[1] := 25 * Ports.mm;
		tabs[2] := 75 * Ports.mm;
		f.WriteRuler(tabs);
		BugsInfo.Methods(infoDialog.node.item, f);
		IF f.lines > 1 THEN
			f.Register("Node update methods")
		END
	END Methods;

	PROCEDURE Metrics*;
		VAR
			tabs: ARRAY 3 OF INTEGER;
			f: BugsMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		text := TextModels.dir.New();
		BugsTexts.ConnectFormatter(f, text);
		f.SetPos(0);
		tabs[0] := 0;
		tabs[1] := 25 * Ports.mm;
		tabs[2] := 75 * Ports.mm;
		f.WriteRuler(tabs);
		BugsInfo.ModelMetrics(f);
		f.Register("Model metrics")
	END Metrics;

	PROCEDURE Types*;
		VAR
			tabs: ARRAY 3 OF INTEGER;
			f: BugsMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		text := TextModels.dir.New();
		BugsTexts.ConnectFormatter(f, text);
		f.SetPos(0);
		tabs[0] := 0;
		tabs[1] := 25 * Ports.mm;
		tabs[2] := 75 * Ports.mm;
		f.WriteRuler(tabs);
		BugsInfo.Types(infoDialog.node.item, f);
		IF f.lines > 1 THEN
			f.Register("Node types")
		END
	END Types;

	PROCEDURE UpdatersByName*;
		VAR
			tabs: ARRAY 6 OF INTEGER;
			f: BugsMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		text := TextModels.dir.New();
		BugsTexts.ConnectFormatter(f, text);
		f.SetPos(0);
		tabs[0] := 0;
		tabs[1] := 25 * Ports.mm;
		tabs[2] := 75 * Ports.mm;
		tabs[3] := 85 * Ports.mm;
		tabs[4] := 95 * Ports.mm;
		tabs[5] := 105 * Ports.mm;
		f.WriteRuler(tabs);
		BugsInfo.UpdatersByName(f);
		IF f.lines > 1 THEN
			f.Register("Updater types")
		END
	END UpdatersByName;

	PROCEDURE UpdatersByDepth*;
		VAR
			tabs: ARRAY 6 OF INTEGER;
			f: BugsMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		text := TextModels.dir.New();
		BugsTexts.ConnectFormatter(f, text);
		f.SetPos(0);
		tabs[0] := 0;
		tabs[1] := 25 * Ports.mm;
		tabs[2] := 75 * Ports.mm;
		tabs[3] := 85 * Ports.mm;
		tabs[4] := 95 * Ports.mm;
		tabs[5] := 105 * Ports.mm;
		f.WriteRuler(tabs);
		BugsInfo.UpdatersByDepth(f);
		IF f.lines > 1 THEN
			f.Register("Updater types")
		END
	END UpdatersByDepth;

	PROCEDURE ShowDistribution*;
		VAR
			tabs: POINTER TO ARRAY OF INTEGER;
			f: BugsMappers.Formatter;
			text: TextModels.Model;
			i, numChains, numProc: INTEGER;
			string: ARRAY 128 OF CHAR;
	BEGIN
		numChains := specificationDialog.numChains;
		numProc := specificationDialog.numProc;
		numProc := numProc DIV numChains;
		numProc := MAX(numProc, 1);
		text := TextModels.dir.New();
		BugsTexts.ConnectFormatter(f, text);
		f.SetPos(0);
		NEW(tabs, numProc + 2);
		tabs[0] := 10 * Ports.mm;
		i := 1;
		WHILE i < numProc + 1 DO
			tabs[i] := tabs[0] + i * 40 * Ports.mm;
			INC(i)
		END;
		tabs[numProc + 1] := tabs[numProc] + 20 * Ports.mm;
		f.WriteRuler(tabs);
		BugsInfo.Distribute(numProc, f);
		IF f.lines > 1 THEN
			Strings.IntToString(numProc, string);
			string := "Updaters distributed over " + string + " processors";
			f.Register(string)
		END;
	END ShowDistribution;

	PROCEDURE ShowDistributionDeviance*;
		VAR
			tabs: POINTER TO ARRAY OF INTEGER;
			f: BugsMappers.Formatter;
			text: TextModels.Model;
			i, numChains, numProc: INTEGER;
			string: ARRAY 128 OF CHAR;
	BEGIN
		numChains := specificationDialog.numChains;
		numProc := specificationDialog.numProc;
		numProc := numProc DIV numChains;
		numProc := MAX(numProc, 1);
		text := TextModels.dir.New();
		BugsTexts.ConnectFormatter(f, text);
		f.SetPos(0);
		NEW(tabs, numProc + 1);
		tabs[0] := 10 * Ports.mm;
		i := 1;
		WHILE i < numProc DO
			tabs[i] := tabs[0] + i * 40 * Ports.mm;
			INC(i)
		END;
		tabs[numProc] := tabs[numProc - 1] + 20 * Ports.mm;
		f.WriteRuler(tabs);
		BugsInfo.DistributeDeviance(numProc, f);
		IF f.lines > 1 THEN
			Strings.IntToString(numProc, string);
			string := "Deviance distributed over " + string + " processors";
			f.Register(string)
		END;
	END ShowDistributionDeviance;

	PROCEDURE Values*;
		VAR
			i, max, numChains: INTEGER;
			tabs: POINTER TO ARRAY OF INTEGER;
			f: BugsMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		IF BugsInfo.IsConstant(infoDialog.node.item) THEN
			numChains := 1
		ELSE
			numChains := specificationDialog.numChains
		END;
		text := TextModels.dir.New();
		BugsTexts.ConnectFormatter(f, text);
		f.SetPos(0);
		max := 2 + MIN(numChains, 10);
		NEW(tabs, max);
		i := 0;
		WHILE i < max DO
			tabs[i] := i * 25 * Ports.mm;
			INC(i)
		END;
		f.WriteRuler(tabs);
		BugsInfo.Values(infoDialog.node.item, numChains, f);
		f.Register("Node values")
	END Values;

	PROCEDURE WriteChains*;
		VAR
			i, numChains: INTEGER;
			msg: ARRAY 1024 OF CHAR;
			numAsString: ARRAY 8 OF CHAR;
			f: BugsMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		numChains := specificationDialog.numChains;
		i := 0;
		WHILE i < numChains DO
			text := TextModels.dir.New();
			BugsTexts.ConnectFormatter(f, text);
			f.SetPos(0);
			BugsInfo.WriteChain(i, f);
			Strings.IntToString(i + 1, numAsString);
			f.Register("chain " + numAsString);
			INC(i)
		END;
		BugsMsg.MapMsg("BugsCmds:ChainsWrittenOut", msg);
		BugsTexts.ShowMsg(msg)
	END WriteChains;

	PROCEDURE WriteData*;
		CONST
			numTabs = 6;
		VAR
			msg: ARRAY 1024 OF CHAR;
			f: BugsMappers.Formatter;
			text: TextModels.Model;
			tabs: POINTER TO ARRAY OF INTEGER;
			i: INTEGER;
	BEGIN
		text := TextModels.dir.New();
		BugsTexts.ConnectFormatter(f, text);
		f.SetPos(0);
		NEW(tabs, numTabs);
		i := 0;
		WHILE i < numTabs DO
			tabs[i] := (10 + 20 * i) * Ports.mm;
			INC(i)
		END;
		f.WriteRuler(tabs);
		BugsInfo.WriteData(f);
		f.Register("Data");
		BugsMsg.MapMsg("BugsCmds:DataOut", msg);
		BugsTexts.ShowMsg(msg)
	END WriteData;

	PROCEDURE WriteUninitNodes*;
		VAR
			i, numChains: INTEGER;
			msg: ARRAY 1024 OF CHAR;
			numAsString: ARRAY 8 OF CHAR;
			f: BugsMappers.Formatter;
			text: TextModels.Model;
			tabs: ARRAY 2 OF INTEGER;
	BEGIN
		tabs[0] := 0;
		tabs[1] := 50 * Ports.mm;
		numChains := specificationDialog.numChains;
		i := 0;
		WHILE i < numChains DO
			text := TextModels.dir.New();
			BugsTexts.ConnectFormatter(f, text);
			f.SetPos(0);
			f.WriteRuler(tabs);
			BugsInfo.WriteUninitNodes(i, f);
			Strings.IntToString(i + 1, numAsString);
			f.Register("uninitialized nodes for chain " + numAsString);
			INC(i)
		END;
		BugsMsg.MapMsg("BugsCmds:UninitializedNodes", msg);
		BugsTexts.ShowMsg(msg)
	END WriteUninitNodes;

	PROCEDURE MAP*;
		VAR
			f: BugsMappers.Formatter;
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
		BugsTexts.ConnectFormatter(f, text);
		f.SetPos(0);
		f.WriteRuler(tabs);
		BugsMAP.Estimates(stochastics, f);
		f.Register("Maximum a posteri estimates");
	END MAP;

	PROCEDURE Modules*;
		CONST
			numTabs = 8;
		VAR
			i: INTEGER;
			tabs: ARRAY numTabs OF INTEGER;
			f: BugsMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		text := TextModels.dir.New();
		BugsTexts.ConnectFormatter(f, text);
		f.SetPos(0);
		tabs[0] := 0;
		tabs[1] := 40 * Ports.mm;
		i := 2;
		WHILE i < numTabs DO
			tabs[i] := tabs[1] + 20 * Ports.mm * i;
			INC(i)
		END;
		f.WriteRuler(tabs);
		BugsVersion.Modules(f);
		f.Register("modules")
	END Modules;

	PROCEDURE PrintModel*;
		VAR
			f: BugsMappers.Formatter;
			text: TextModels.Model;
			tabs: ARRAY 15 OF INTEGER;
			i: INTEGER;
	BEGIN
		text := TextModels.dir.New();
		BugsTexts.ConnectFormatter(f, text);
		f.SetPos(0);
		i := 0; WHILE i < LEN(tabs) DO tabs[i] := (i + 1) * 5 * Ports.mm; INC(i) END;
		tabs[i - 1] := 200 * Ports.mm;
		f.WriteRuler(tabs);
		BugsPrettyprinter.DisplayCode(BugsParser.model, f);
		f.Register("Bugs language model")
	END PrintModel;

	PROCEDURE PrintLatex*;
		VAR
			f: BugsMappers.Formatter;
			text: TextModels.Model;
			tabs: ARRAY 2 OF INTEGER;
	BEGIN
		text := TextModels.dir.New();
		BugsTexts.ConnectFormatter(f, text);
		f.SetPos(0);
		tabs[0] := 0; tabs[1] := 200 * Ports.mm;
		f.WriteRuler(tabs);
		BugsLatexprinter.DisplayCode(BugsParser.model, f);
		f.Register("model in latex")
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
		BugsTexts.ShowMsg(msg)
	END AllocatedMemory;

	PROCEDURE GetWD*;
	BEGIN
		IF BugsFiles.workingDir # "" THEN
			BugsTexts.ShowMsg(BugsFiles.workingDir)
		ELSE
			BugsTexts.ShowMsg("working directory not set")
		END
	END GetWD;

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
				IF pos =  - 1 THEN INC(num) END
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
				IF pos =  - 1 THEN
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
		VAR
			name: Files.Name;
			loc, locOld: Files.Locator;
	BEGIN
		BugsFiles.PathToFileSpec(fileName, loc, name);
		locOld := BugsSerialize.GetRestartLoc();
		BugsSerialize.SetRestartLoc(loc);
		BugsSerialize.Externalize(name);
		BugsSerialize.SetRestartLoc(locOld);
		UpdateBugFileNames;
		BugsTexts.ShowMsg("model externalized to file ok")
	END ExternalizeModel;

	PROCEDURE InternalizeModel* (fileName: ARRAY OF CHAR);
		VAR
			name: Files.Name;
			loc, locOld: Files.Locator;
			f: Files.File;
			mes: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		BugsFiles.PathToFileSpec(fileName, loc, name);
		locOld := BugsSerialize.GetRestartLoc();
		BugsSerialize.SetRestartLoc(loc);
		f := Files.dir.Old(loc, name + ".bug", Files.shared);
		IF f = NIL THEN
			p[0] := fileName$;
			BugsMsg.MapParamMsg("BugsCmds:NoFile", p, mes);
			BugsTexts.ShowMsg(mes);
			RETURN
		END;
		f.Close;
		BugsSerialize.Internalize(name);
		BugsSerialize.SetRestartLoc(locOld);
		specificationDialog.numChains := BugsRandnum.numberChains;
		SetIteration(UpdaterActions.iteration);
		updateDialog.isAdapting := UpdaterActions.endOfAdapting = MAX(INTEGER);
		UpdaterSettings.MarkCompiled;
		UpdaterSettings.FillDialog;
		BugsDialog.UpdateDialogs;
		BugsTexts.ShowMsg("model internalized from file ok")
	END InternalizeModel;

	PROCEDURE Externalize*;
		VAR
			bugFileName: Dialog.String;
	BEGIN
		bugFileName := outBugDialog.name.item$;
		BugsSerialize.Externalize(bugFileName);
		BugsTexts.ShowMsg("model externalized to file ok")
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
		BugsTexts.ShowMsg("model internalized from file ok")
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
			s: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		BugsInterface.ChangeSampler(node, UpdaterMethods.index, ok);
		IF ~ok THEN
			p[0] := node$;
			BugsMsg.MapParamMsg("BugsCmds:couldNotChangeUpdater", p, s);
			BugsTexts.ShowMsg(s)
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
		displayDialog.whereOut := BugsMappers.window;
		BugsMappers.SetDest(BugsMappers.window)
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
		CONST
			numberPresetStates = 14;
	BEGIN
		IF rnDialog.preSet < 1 THEN
			rnDialog.preSet := 1
		ELSIF rnDialog.preSet > numberPresetStates THEN
			rnDialog.preSet := numberPresetStates
		END;
		MathRandnum.InitState(rnDialog.preSet - 1);
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
		BugsMappers.SetPrec(displayDialog.precision)
	END PrecisionNotifier;

	PROCEDURE DisplayNotifier* (op, from, to: INTEGER);
	BEGIN
		BugsMappers.SetDest(displayDialog.whereOut)
	END DisplayNotifier;

	PROCEDURE CompileGuardWin* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~BugsInterface.IsParsed()
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
		par.disabled := ~BugsInterface.IsCompiled();
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
		par.checked := displayDialog.whereOut = BugsMappers.log
	END SetLogGuard;

	PROCEDURE SetWindowGuard* (VAR par: Dialog.Par);
	BEGIN
		par.checked := displayDialog.whereOut = BugsMappers.window
	END SetWindowGuard;

	PROCEDURE UpdateGuardWin* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~BugsInterface.IsInitialized()
	END UpdateGuardWin;

	PROCEDURE SetRNGuardWin* (VAR par: Dialog.Par);
	BEGIN
		par.readOnly := ~BugsInterface.IsCompiled() OR (UpdaterActions.iteration # 0)
	END SetRNGuardWin;

	PROCEDURE MAPGuardWin* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~BugsInterface.IsInitialized() OR (GraphNodes.maxStochDepth > 1)
	END MAPGuardWin;

	PROCEDURE ParseGuard* (OUT ok: BOOLEAN);
		VAR
			s: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		ok := File(filePath) # NIL;
		IF ~ok THEN
			p[0] := filePath$;
			BugsMsg.MapParamMsg("BugsCmds:NoFile", p, s);
			BugsTexts.ShowMsg(s)
		END
	END ParseGuard;

	PROCEDURE LoadDataGuard* (OUT ok: BOOLEAN);
		VAR
			s: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		ok := File(filePath) # NIL;
		IF ~ok THEN
			p[0] := filePath$;
			BugsMsg.MapParamMsg("BugsCmds:NoFile", p, s);
			BugsTexts.ShowMsg(s)
		ELSE
			ok := BugsInterface.IsParsed();
			IF ~ok THEN
				BugsMsg.MapMsg("BugsCmds:NoCheckData", s);
				BugsTexts.ShowMsg(s)
			END
		END
	END LoadDataGuard;

	PROCEDURE CompileGuard* (OUT ok: BOOLEAN);
		VAR
			s: ARRAY 1024 OF CHAR;
	BEGIN
		ok := BugsInterface.IsParsed();
		IF ~ok THEN
			BugsMsg.MapMsg("BugsCmds:NoCheckCompile", s);
			BugsTexts.ShowMsg(s)
		END
	END CompileGuard;

	PROCEDURE DistributeGuard* (OUT ok: BOOLEAN);
		VAR
			s: ARRAY 1024 OF CHAR;
	BEGIN
		ok := ~BugsInterface.IsDistributed() & BugsInterface.IsInitialized();
		IF ~ok THEN
			BugsMsg.MapMsg("BugsCmds:UnableToDistribute", s);
			BugsTexts.ShowMsg(s)
		END
	END DistributeGuard;

	PROCEDURE NotCompiledGuard* (OUT ok: BOOLEAN);
		VAR
			s: ARRAY 1024 OF CHAR;
	BEGIN
		ok := ~BugsInterface.IsCompiled();
		IF ~ok THEN
			BugsMsg.MapMsg("BugsCmds:NotCompiled", s);
			BugsTexts.ShowMsg(s)
		END
	END NotCompiledGuard;

	PROCEDURE LoadInitsGuard* (OUT ok: BOOLEAN);
		VAR
			s: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		ok := File(filePath) # NIL;
		IF ~ok THEN
			p[0] := filePath$;
			BugsMsg.MapParamMsg("BugsCmds:NoFile", p, s);
			BugsTexts.ShowMsg(s)
		ELSE
			ok := BugsInterface.IsCompiled();
			IF ~ok THEN
				BugsMsg.MapMsg("BugsCmds:NoCompileInits", s);
				BugsTexts.ShowMsg(s)
			END
		END
	END LoadInitsGuard;

	PROCEDURE GenerateInitsGuard* (OUT ok: BOOLEAN);
		VAR
			s: ARRAY 1024 OF CHAR;
	BEGIN
		ok := BugsInterface.IsCompiled();
		IF ~ok THEN
			BugsMsg.MapMsg("BugsCmds:NoCompileGenInits", s);
			BugsTexts.ShowMsg(s)
		ELSE
			ok := ~BugsInterface.IsInitialized();
			IF ~ok THEN
				BugsMsg.MapMsg("BugsCmds:AlreadyInits", s);
				BugsTexts.ShowMsg(s)
			END
		END
	END GenerateInitsGuard;

	PROCEDURE UpdateGuard* (OUT ok: BOOLEAN);
		VAR
			s: ARRAY 1024 OF CHAR;
	BEGIN
		ok := BugsInterface.IsInitialized();
		IF ~ok THEN
			BugsMsg.MapMsg("BugsCmds:NotInits", s);
			BugsTexts.ShowMsg(s)
		END
	END UpdateGuard;

	PROCEDURE SetRNGuard* (OUT ok: BOOLEAN);
		VAR
			s: ARRAY 1024 OF CHAR;
	BEGIN
		ok := BugsInterface.IsInitialized() & (UpdaterActions.iteration = 0);
		IF ~ok THEN
			s := "model must have been compiled but not updated to be able to change RN generator";
			BugsTexts.ShowMsg(s)
		END
	END SetRNGuard;

	PROCEDURE FactoryGuard* (OUT ok: BOOLEAN);
		VAR
			s: ARRAY 1024 OF CHAR;
	BEGIN
		UpdaterMethods.FactoryGuard(ok);
		IF ~ok THEN
			BugsMsg.MapMsg("UpdaterMethods:notUpdateMethod", s);
			BugsTexts.ShowMsg(s)
		END
	END FactoryGuard;

	PROCEDURE AdaptivePhaseGuard* (OUT ok: BOOLEAN);
		VAR
			s: ARRAY 1024 OF CHAR;
	BEGIN
		UpdaterMethods.AdaptivePhaseGuard(ok);
		IF ~ok THEN
			BugsMsg.MapMsg("UpdaterMethods:notAdaptive", s);
			BugsTexts.ShowMsg(s)
		END
	END AdaptivePhaseGuard;

	PROCEDURE IterationsGuard* (OUT ok: BOOLEAN);
		VAR
			s: ARRAY 1024 OF CHAR;
	BEGIN
		UpdaterMethods.IterationsGuard(ok);
		IF ~ok THEN
			BugsMsg.MapMsg("UpdaterMethods.notIterations", s);
			BugsTexts.ShowMsg(s)
		END
	END IterationsGuard;

	PROCEDURE OverRelaxationGuard* (OUT ok: BOOLEAN);
		VAR
			s: ARRAY 1024 OF CHAR;
	BEGIN
		UpdaterMethods.OverRelaxationGuard(ok);
		IF ~ok THEN
			BugsMsg.MapMsg("UpdaterMethods:notOverRelax", s);
			BugsTexts.ShowMsg(s)
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
		NEW(action);
		NEW(scriptAction);
		NEW(compileDialog);
		BugsDialog.AddDialog(compileDialog);
		compileDialog.updaterByMethod := TRUE;
		NEW(displayDialog);
		BugsDialog.AddDialog(displayDialog);
		displayDialog.whereOut := BugsMappers.whereOut;
		displayDialog.precision := BugsMappers.prec;
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

