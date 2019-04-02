(* 	

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsCmds;

	

	IMPORT
		Dialog, Environment, Files, Meta, Ports, Services, Strings, Views, Windows, 
		HostMenus,
		StdLog, StdCmds,
		TextCmds, TextControllers, TextMappers, TextModels, TextViews,
		BugsCPCompiler, BugsDialog, BugsFiles, BugsGraph, BugsIndex, BugsInfo, BugsInterface,
		BugsInterpreter, BugsLatexprinter, BugsMAP, BugsMappers, BugsMsg, BugsNames, BugsParser,
		BugsPrettyprinter, BugsRandnum, BugsScripting, BugsSerialize, BugsVersion,
		DevDebug,
		GraphNodes, GraphRules, GraphStochastic,
		MathRandnum,
		MonitorMonitors,
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
			numChains*, workersPerChain*: INTEGER; 	(*	number of MCMC chains	*)
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
		
		script-: BOOLEAN;
		
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
		VAR
			res: INTEGER;
	BEGIN
		IF option = "log" THEN
			displayDialog.whereOut := BugsFiles.log;
			BugsFiles.SetDest(BugsFiles.log);
			Dialog.Call("StdLog.Open", "", res);
		ELSIF option = "window" THEN
			displayDialog.whereOut := BugsFiles.window;
			BugsFiles.SetDest(BugsFiles.window)
		ELSIF option = "file" THEN
			displayDialog.whereOut := BugsFiles.file;
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

	PROCEDURE GetText (IN filePath: ARRAY OF CHAR): TextModels.Model;
		VAR
			text: TextModels.Model;
			c: TextControllers.Controller;
			w: Windows.Window;
			v: Views.View;
	BEGIN
		IF filePath # "" THEN
			text := BugsFiles.FileToText(filePath)
		ELSIF Dialog.platform = Dialog.linux THEN (*	work round bug in Linux version of BB	*)
			w := Windows.dir.First();
			WHILE (w # NIL) & ~(w.doc.ThisView() IS TextViews.View) DO
				w := Windows.dir.Next(w)
			END;
			IF w # NIL THEN
				v := w.doc.ThisView();
				text := v(TextViews.View).ThisModel()
			ELSE
				text := NIL
			END
		ELSE
			c := TextControllers.Focus();
			IF c # NIL THEN text := c.text ELSE text := NIL END
		END;
		RETURN text
	END GetText;

	PROCEDURE GetPos (IN filePath: ARRAY OF CHAR): INTEGER;
		VAR
			beg, end, pos: INTEGER;
			c: TextControllers.Controller;
			text: TextModels.Model;
	BEGIN
		text := GetText(filePath);
		IF filePath = "" THEN
			text := GetText(filePath);
			IF text # NIL THEN
				c := TextControllers.Focus();
				c.GetSelection(beg, end);
				IF beg # end THEN
					pos := beg
				ELSE
					pos := 0
				END
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
		BugsFiles.ShowStatus(errorMes);
		TextViews.ShowRange(text, beg, end, TextViews.any);
		TextControllers.SetSelection(text, beg, end)
	END TextError;

	(* 	Command to parse BUGS model 	*)

	PROCEDURE Parse* (filePath: ARRAY OF CHAR);
		VAR
			ok: BOOLEAN;
			pos: INTEGER;
			msg: ARRAY 1024 OF CHAR;
			s: BugsMappers.Scanner;
			text: TextModels.Model;
			p: ARRAY 1 OF Dialog.String;
	BEGIN
		IF filePath = "" THEN
			NewModel(ok);
			IF ~ok THEN RETURN END
		END;
		Clear;
		BugsDialog.UpdateDialogs;
		text := GetText(filePath);
		IF text = NIL THEN 
			IF filePath # "" THEN
				p[0] := filePath$;
				BugsMsg.LookupParam("BugsCmds:NoFile", p, msg);
				BugsMsg.StoreError(msg);
				BugsFiles.ShowStatus(msg)
			END;
			RETURN 
		END;
		s.ConnectToText(text);
		pos := GetPos(filePath);
		s.SetPos(pos);
		BugsInterface.ParseModel(s, ok);
		IF ~BugsInterface.IsParsed() THEN TextError(s, text); RETURN END;
		BugsDialog.UpdateDialogs;
		BugsMsg.Lookup("BugsCmds:OkSyntax", msg);
		BugsFiles.ShowStatus(msg)
	END Parse;

	(*	Commands to load data	*)

	PROCEDURE LoadData*(filePath: ARRAY OF CHAR);
		VAR
			ok: BOOLEAN;
			pos: INTEGER;
			s: BugsMappers.Scanner;
			text: TextModels.Model;
			msg, warning: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF Dialog.String;
	BEGIN
		IF ~BugsInterface.IsParsed() THEN RETURN END;
		text := GetText(filePath);
		IF text = NIL THEN 
			IF filePath # "" THEN
				p[0] := filePath$;
				BugsMsg.LookupParam("BugsCmds:NoFile", p, msg);
				BugsMsg.StoreError(msg);
				BugsFiles.ShowStatus(msg)
			END;
			RETURN 
		END;
		s.ConnectToText(text);
		pos := GetPos(filePath);
		s.SetPos(pos);
		BugsMsg.Clear;
		BugsInterface.LoadData(s, ok);
		IF ~ok THEN TextError(s, text); RETURN END;
		warning := BugsMsg.message$;
		BugsMsg.Lookup("BugsCmds:OkData", msg);
		(* If warning not encountered, will contain empty string *)
		msg := msg + warning;
		BugsMsg.StoreMsg(msg);
		BugsFiles.ShowStatus(msg);
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
			msg: ARRAY 1024 OF CHAR;
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
		IF ~BugsCPCompiler.debug THEN StdLog.text.Delete(len0, len1) END;
		IF ok THEN
			SetAdaptingStatus;
			UpdaterSettings.MarkCompiled;
			UpdaterSettings.FillDialog;
			BugsDialog.UpdateDialogs;
			elapsedTime := Services.Ticks() - startTime;
			elapsedTime := ENTIER(1.0 * elapsedTime / Services.resolution + eps);
			Strings.IntToString(elapsedTime, p[0]);
			BugsMsg.LookupParam("BugsCmds:OkCompile", p, msg);
			BugsMsg.StoreMsg(msg);
			BugsFiles.ShowStatus(msg);
		ELSE
			BugsFiles.ShowStatus(BugsMsg.message)
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

	PROCEDURE LoadInits* (filePath: ARRAY OF CHAR);
		VAR
			ok, fixFounder: BOOLEAN;
			pos: INTEGER;
			chain, numChains: INTEGER;
			s: BugsMappers.Scanner;
			text: TextModels.Model;
			msg, warning: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF Dialog.String;
	BEGIN
		BugsMsg.Clear;
		NewInits(ok);
		IF ~ok THEN RETURN END;
		text := GetText(filePath);
		IF text = NIL THEN 
			IF filePath # "" THEN
				p[0] := filePath$;
				BugsMsg.LookupParam("BugsCmds:NoFile", p, msg);
				BugsMsg.StoreError(msg);
				BugsFiles.ShowStatus(msg)
			END;
			RETURN 
		END;
		s.ConnectToText(text);
		pos := GetPos(filePath);
		s.SetPos(pos);
		chain := specificationDialog.chain;
		numChains := specificationDialog.numChains;
		fixFounder := specificationDialog.fixFounder;
		BugsInterface.LoadInits(s, chain - 1, numChains, fixFounder, ok);
		IF ~ok THEN
			IF UpdaterActions.IsInitialized(chain - 1) THEN
				BugsFiles.ShowStatus(BugsMsg.message);
			ELSE
				TextError(s, text);
			END;
			RETURN
		END;
		warning := BugsMsg.message$;
		IF BugsInterface.IsInitialized() THEN
			BugsMsg.Lookup("BugsCmds:OkInits", msg);
		ELSIF UpdaterActions.IsInitialized(chain - 1) THEN
			BugsMsg.Lookup("BugsCmds:UninitOther", msg)
		ELSE
			BugsMsg.Lookup("BugsCmds:NotInit", msg)
		END;
		msg := msg + warning;
		BugsMsg.StoreMsg(msg);
		BugsFiles.ShowStatus(BugsMsg.message);
		SetChain(chain + 1);
		Dialog.Update(specificationDialog)
	END LoadInits;

	(*	Commands to generate initial values	*)

	PROCEDURE GenerateInits*;
		VAR
			numChains: INTEGER;
			fixFounder, ok: BOOLEAN;
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		IF ~BugsInterface.IsCompiled() THEN RETURN END;
		IF BugsInterface.IsInitialized() THEN RETURN END;
		numChains := specificationDialog.numChains;
		fixFounder := specificationDialog.fixFounder;
		BugsInterface.GenerateInits(numChains, fixFounder, ok);
		IF ok THEN
			BugsMsg.Lookup("BugsCmds:OkGenInits", msg);
			BugsFiles.ShowStatus(msg)
		ELSE
			BugsFiles.ShowStatus(BugsMsg.message);
			RETURN
		END
	END GenerateInits;

	PROCEDURE Distribute*;
		VAR
			workersPerChain, numChains, res: INTEGER;
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		numChains := specificationDialog.numChains;
		workersPerChain := specificationDialog.workersPerChain;
		Dialog.Call("BugsMaster.Install", "", res);
		BugsInterface.Distribute(workersPerChain, numChains);
		IF BugsInterface.IsDistributed() THEN
			BugsMsg.Lookup("BugsCmds:ModelDistibuted", msg);
			BugsFiles.ShowStatus(msg)
		ELSE
			BugsFiles.ShowStatus(BugsMsg.message)
		END
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
			msg: ARRAY 1024 OF CHAR;
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
			BugsMsg.Lookup(BugsMsg.message, msg);
			BugsFiles.ShowStatus(msg);
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
			BugsMsg.LookupParam("BugsCmds:UpdatesTook", p, msg);
			BugsFiles.ShowStatus(msg)
		END
	END Do;

	PROCEDURE Update*;
		VAR
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		IF ~action.updating THEN
			action.updating := TRUE;
			startTime := Services.Ticks();
			BugsMsg.Lookup("BugsCmds:Updating", msg);
			BugsFiles.ShowStatus(msg);
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
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		BugsMsg.Lookup("BugsCmds:Updating", msg);
		BugsFiles.ShowStatus(msg);
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
			BugsFiles.ShowStatus(BugsMsg.message);
			RETURN
		END;
		elapsedTime := Services.Ticks() - startTime;
		elapsedTime := ENTIER(1.0 * elapsedTime / Services.resolution + eps);
		Strings.IntToString(updates, p[0]);
		Strings.IntToString(elapsedTime, p[1]);
		BugsMsg.LookupParam("BugsCmds:UpdatesTook", p, msg);
		BugsFiles.ShowStatus(msg);
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
			bugsCommand, errorMsg: ARRAY 1024 OF CHAR;
			pos, res: INTEGER;
	BEGIN
		res := 0;
		IF IsUpdating() OR a.s.eot THEN
			(*	if updating wait, do not read next script command	*)
		ELSE
			script := TRUE;
			BugsScripting.Script(a.s, bugsCommand, res);
			script := FALSE
		END;
		Strings.Find(bugsCommand, "(", 0, pos);
		IF pos # - 1 THEN bugsCommand[pos] := 0X END;
		IF res = 0 THEN
			IF ~BugsMsg.error THEN
				Services.DoLater(a, Services.now)
			ELSE
				BugsMsg.Lookup("BugsCmds:CommandError", errorMsg);
				errorMsg := errorMsg + " " + bugsCommand + " failed";
				Dialog.ShowMsg(errorMsg)
			END
		ELSIF res = BugsInterpreter.failedGuard THEN
			errorMsg := BugsMsg.message$;
			errorMsg := errorMsg + " " + bugsCommand + " failed";
			Dialog.ShowMsg(errorMsg)
		ELSE
			BugsMsg.Lookup("BugsCmds:ScriptFailed", errorMsg); 
			Dialog.ShowMsg(errorMsg)
		END
	END Do;

	PROCEDURE Script*(fileName: ARRAY OF CHAR);
		VAR
			beg, end: INTEGER;
			s: BugsMappers.Scanner;
			text, text0: TextModels.Model;
			c: TextControllers.Controller;
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		text := GetText(fileName);
		IF text = StdLog.text THEN
			BugsMsg.Lookup("BugsCmds:ScripIsLog", msg);
			Dialog.ShowMsg(msg);
			RETURN
		END;
		(*	if have text with high-lighted region make a copy of high-lighted region	*)
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

	PROCEDURE MapValues*;
		VAR
			tabs: ARRAY 5 OF INTEGER;
			f: TextMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		tabs[0] := 0;
		tabs[1] := 30 * Ports.mm;
		tabs[2] := 60 * Ports.mm;
		tabs[3] := 90 * Ports.mm;
		tabs[4] := 110 * Ports.mm;
		BugsFiles.WriteRuler(tabs, f);
		BugsInfo.MapValues(f);
		BugsFiles.Open("Mapped values", text)
	END MapValues;

	PROCEDURE ShowDistribution*;
		VAR
			tabs: POINTER TO ARRAY OF INTEGER;
			f: TextMappers.Formatter;
			text: TextModels.Model;
			i, workersPerChain: INTEGER;
			string: ARRAY 128 OF CHAR;
	BEGIN
		workersPerChain := specificationDialog.workersPerChain;
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		NEW(tabs, workersPerChain + 2);
		tabs[0] := 10 * Ports.mm;
		i := 1;
		WHILE i < workersPerChain + 1 DO
			tabs[i] := tabs[0] + i * 40 * Ports.mm;
			INC(i)
		END;
		tabs[workersPerChain + 1] := tabs[workersPerChain] + 20 * Ports.mm;
		BugsFiles.WriteRuler(tabs, f);
		BugsInfo.Distribute(workersPerChain, f);
		Strings.IntToString(workersPerChain, string);
		string := "Updaters distributed over " + string + " workers";
		BugsFiles.Open(string, text)
	END ShowDistribution;

	PROCEDURE ShowDistributionDeviance*;
		VAR
			tabs: POINTER TO ARRAY OF INTEGER;
			f: TextMappers.Formatter;
			text: TextModels.Model;
			i, workersPerChain: INTEGER;
			string: ARRAY 128 OF CHAR;
	BEGIN
		workersPerChain := specificationDialog.workersPerChain;
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		NEW(tabs, workersPerChain + 1);
		tabs[0] := 10 * Ports.mm;
		i := 1;
		WHILE i < workersPerChain DO
			tabs[i] := tabs[0] + i * 40 * Ports.mm;
			INC(i)
		END;
		tabs[workersPerChain] := tabs[workersPerChain - 1] + 20 * Ports.mm;
		BugsFiles.WriteRuler(tabs, f);
		BugsInfo.DistributeDeviance(workersPerChain, f);
		Strings.IntToString(workersPerChain, string);
		string := "Deviance distributed over " + string + " processors";
		BugsFiles.Open(string, text)
	END ShowDistributionDeviance;

	PROCEDURE Values*;
		VAR
			i, max, numChains: INTEGER;
			tabs: POINTER TO ARRAY OF INTEGER;
			f: TextMappers.Formatter;
			text, header: TextModels.Model;
	BEGIN
		numChains := specificationDialog.numChains;
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		BugsInfo.Values(infoDialog.node.item, numChains, f);
		max := 2 + MIN(numChains, 10);
		NEW(tabs, max);
		i := 0;
		WHILE i < max DO
			tabs[i] := i * 25 * Ports.mm;
			INC(i)
		END;
		header := TextModels.dir.New();
		f.ConnectTo(header);
		f.SetPos(0);
		BugsFiles.WriteRuler(tabs, f);
		header.Append(text);
		text := header;
		BugsFiles.Open("Node values", text)
	END Values;

	PROCEDURE WriteChains*;
		VAR
			i, numChains: INTEGER;
			numAsString: ARRAY 8 OF CHAR;
			f: TextMappers.Formatter;
			text: TextModels.Model;
			msg: ARRAY 1024 OF CHAR;
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
		BugsMsg.Lookup("BugsCmds:ChainsWrittenOut", msg);
		BugsFiles.ShowStatus(msg)
	END WriteChains;

	PROCEDURE WriteChainsToFile* (stem: ARRAY OF CHAR);
		VAR
			i, numChains: INTEGER;
			numAsString: ARRAY 8 OF CHAR;
			f: TextMappers.Formatter;
			text: TextModels.Model;
			msg: ARRAY 1024 OF CHAR;
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
		BugsMsg.Lookup("BugsCmds:ChainsWrittenOut", msg);
		BugsFiles.ShowStatus(msg)
	END WriteChainsToFile;

	PROCEDURE WriteData*;
		CONST
			numTabs = 7;
		VAR
			f: TextMappers.Formatter;
			text: TextModels.Model;
			tabs: POINTER TO ARRAY OF INTEGER;
			i, oldPrec: INTEGER;
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		oldPrec := BugsFiles.prec;
		BugsFiles.SetPrec(MAX(8, oldPrec));
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
		BugsMsg.Lookup("BugsCmds:DataOut", msg);
		BugsFiles.ShowStatus(msg);
		BugsFiles.SetPrec(oldPrec)
	END WriteData;

	PROCEDURE WriteUninitNodes*;
		VAR
			i, numChains, pos, pos1: INTEGER;
			unInit: BOOLEAN;
			numAsString: ARRAY 8 OF CHAR;
			f: TextMappers.Formatter;
			text: TextModels.Model;
			tabs: ARRAY 2 OF INTEGER;
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		tabs[0] := 0;
		tabs[1] := 50 * Ports.mm;
		numChains := specificationDialog.numChains;
		i := 0;
		unInit := FALSE;
		WHILE i < numChains DO
			text := TextModels.dir.New();
			f.ConnectTo(text);
			f.SetPos(0);
			BugsFiles.WriteRuler(tabs, f);
			pos := f.Pos();
			BugsInfo.WriteUninitNodes(i, f);
			pos1 := f.Pos();
			IF pos # pos1 THEN
				Strings.IntToString(i + 1, numAsString);
				BugsFiles.Open("uninitialized nodes for chain " + numAsString, text);
				unInit := TRUE;
			END;
			INC(i)
		END;
		IF ~unInit THEN
			BugsMsg.Lookup("BugsCmds:UninitializedNodes", msg);
			BugsFiles.ShowStatus(msg)
		END
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
		BugsMsg.StoreMsg(msg);
		BugsFiles.ShowStatus(msg)
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
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		loc := BugsSerialize.restartLoc;
		f := Files.dir.Old(loc, fileName + ".bug", Files.shared);
		IF f = NIL THEN
			p[0] := fileName$;
			BugsMsg.LookupParam("BugsCmds:NoFile", p, msg);
			BugsFiles.ShowStatus(msg);
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
		BugsMsg.Lookup("BugsCmds:modelInternalized", msg);
		BugsFiles.ShowStatus(msg);
	END InternalizeModel;

	PROCEDURE Externalize*;
		VAR
			bugFileName: Dialog.String;
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		bugFileName := outBugDialog.name.item$;
		BugsSerialize.Externalize(bugFileName);
		BugsMsg.Lookup("BugsCmds:ModelExternalized ", msg);
		Dialog.ShowMsg(msg)
	END Externalize;

	PROCEDURE Internalize*;
		VAR
			ok: BOOLEAN;
			index: INTEGER;
			name: Dialog.String;
			msg: ARRAY 1024 OF CHAR;
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
		BugsMsg.Lookup("BugsCmds:ModelInternalized", msg);
		BugsFiles.ShowStatus(msg)
	END Internalize;

	PROCEDURE Quit* (option: ARRAY OF CHAR);
		VAR
			i: INTEGER;
	BEGIN
		i := 0; WHILE i < LEN(option) DO option[i] := CAP(option[i]); INC(i) END;
		IF (option = "YES") OR (option = "Y") THEN
			HostMenus.askQuit := FALSE
		ELSE
			HostMenus.askQuit := TRUE
		END;
		HostMenus.Exit
	END Quit;

	PROCEDURE About*;
		VAR
			aboutDialog, title: Dialog.String;
	BEGIN
		aboutDialog := "System/Rsrc/About";
		IF Environment.RunningUnderMPI() THEN
			aboutDialog := aboutDialog + "MultiBUGS";
			title := "About MultiBUGS"
		ELSE
			aboutDialog := aboutDialog + "OpenBUGS";
			title := "About OpenBUGS"		
		END;
		StdCmds.OpenToolDialog(aboutDialog, title)
	END About;
	
	PROCEDURE Debug*;
	BEGIN
		BugsCPCompiler.debug := ~BugsCPCompiler.debug;
		BugsMsg.debug := ~BugsMsg.debug
	END Debug;

	PROCEDURE ChangeSampler* (node: ARRAY OF CHAR);
		VAR
			ok: BOOLEAN;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		BugsInterface.ChangeSampler(node, UpdaterMethods.index, ok);
		IF ~ok THEN
			p[0] := node$;
			BugsMsg.LookupParam("BugsCmds:couldNotChangeUpdater", p, msg);
			BugsFiles.ShowStatus(msg)
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
		dialogBox.workersPerChain := 1;
		dialogBox.fixFounder := TRUE
	END Init;

	PROCEDURE (dialogBox: SpecificationDialog) Update-;
	BEGIN
		Dialog.Update(dialogBox)
	END Update;

	PROCEDURE OpenSpecificationDialog*;
	BEGIN
		IF Environment.RunningUnderMPI() THEN
			StdCmds.OpenToolDialog('Bugs/Rsrc/SpecificationDialogMultiBUGS', 'Specification Tool');
		ELSE
			StdCmds.OpenToolDialog('Bugs/Rsrc/SpecificationDialogOpenBUGS', 'Specification Tool');
		END;
	END OpenSpecificationDialog;

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
	
	PROCEDURE AboutGuard* (VAR par: Dialog.Par);
	BEGIN
		IF Environment.RunningUnderMPI() THEN
			par.label := "About MultiBUGS"
		ELSE
			par.label := "About OpenBUGS"
		END
	END AboutGuard;
	
	PROCEDURE CompileGuard* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~BugsInterface.IsParsed() OR BugsInterface.IsCompiled();
		IF par.disabled & script THEN BugsMsg.Lookup("BugsCmds:NoCheckCompile", par.label)  END
	END CompileGuard;

	PROCEDURE CompiledGuard* (VAR par: Dialog.Par);
	BEGIN
		par.readOnly := BugsInterface.IsCompiled()
	END CompiledGuard;

	PROCEDURE DebugGuard* (VAR par: Dialog.Par);
	BEGIN
		par.checked := BugsCPCompiler.debug
	END DebugGuard;

	PROCEDURE DevianceGuard* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~BugsInterface.IsInitialized() OR (BugsIndex.Find("deviance") = NIL);
		IF par.disabled & script THEN  BugsMsg.Lookup("BugsCmds:NoDeviance", par.label) END
	END DevianceGuard;

	PROCEDURE DistributeGuard* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := BugsInterface.IsDistributed() OR (~BugsInterface.IsInitialized());
		IF par.disabled & script THEN  BugsMsg.Lookup("BugsCmds:UnableToDistribute", par.label)END
	END DistributeGuard;

	PROCEDURE GenerateInitsGuard* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~BugsInterface.IsCompiled();
		IF par.disabled THEN
			IF script THEN BugsMsg.Lookup("BugsCmds:NoCompileGenInits", par.label) END
		ELSE
			par.disabled := BugsInterface.IsInitialized();
			IF par.disabled & script THEN BugsMsg.Lookup("BugsCmds:AlreadyInits", par.label) END
		END
	END GenerateInitsGuard;

	PROCEDURE LoadDataGuard* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~BugsInterface.IsParsed() OR BugsInterface.IsCompiled();
		IF par.disabled THEN
			IF script THEN BugsMsg.Lookup("BugsCmds:NoCheckData", par.label) END
		ELSE
			IF ~script THEN TextCmds.FocusGuard(par) END
		END
	END LoadDataGuard;

	PROCEDURE LoadInitsGuard* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~BugsInterface.IsCompiled() OR BugsInterface.IsDistributed();
		IF par.disabled THEN
			IF script THEN  BugsMsg.Lookup("BugsCmds:NoCompileInits", par.label) END
		ELSE
			IF ~script THEN TextCmds.FocusGuard(par) END
		END
	END LoadInitsGuard;

	PROCEDURE MAPGuard* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~BugsInterface.IsInitialized() OR (GraphNodes.maxStochDepth > 1)
	END MAPGuard;

	PROCEDURE NodeInfoGuard* (VAR par: Dialog.Par);
		VAR
			i: INTEGER;
			var: Dialog.String;
			name: BugsNames.Name;
	BEGIN
		var := infoDialog.node.item;
		i := 0;
		WHILE (var[i] # 0X) & (var[i] # "[") DO
			INC(i)
		END;
		var[i] := 0X;
		name := BugsIndex.Find(var);
		par.disabled := name = NIL;
		IF par.disabled & script THEN BugsMsg.Lookup("SamplesCmds:NotVariable", par.label) END
	END NodeInfoGuard;

	PROCEDURE NodeInfo1Guard* (VAR par: Dialog.Par);
		VAR
			i: INTEGER;
			var: Dialog.String;
			name: BugsNames.Name;
	BEGIN
		var := infoDialog.node.item;
		i := 0;
		WHILE (var[i] # 0X) & (var[i] # "[") DO
			INC(i)
		END;
		var[i] := 0X;
		name := BugsIndex.Find(var);
		par.disabled := name = NIL;
		IF par.disabled THEN
			IF script THEN BugsMsg.Lookup("SamplesCmds:NotVariable", par.label) END
		ELSE
			par.disabled := ~name.passByreference;
			IF par.disabled & script THEN BugsMsg.Lookup("SamplesCmds:NotVariable", par.label) END
		END
	END NodeInfo1Guard;

	PROCEDURE NotCompiledGuard* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~BugsInterface.IsCompiled();
		IF par.disabled & script THEN BugsMsg.Lookup("BugsCmds:NotCompiled", par.label) END
	END NotCompiledGuard;

	PROCEDURE ParseGuard* (VAR par: Dialog.Par);
	BEGIN
		IF ~script THEN TextCmds.FocusGuard(par) END
	END ParseGuard;

	PROCEDURE ParsedGuard* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~BugsInterface.IsParsed();
		IF par.disabled & script THEN  BugsMsg.Lookup("BugsCmds:NoCheckData", par.label) END
	END ParsedGuard;

	PROCEDURE SetLogGuard* (VAR par: Dialog.Par);
	BEGIN
		par.checked := displayDialog.whereOut = BugsFiles.log
	END SetLogGuard;

	PROCEDURE SetRNGuard* (VAR par: Dialog.Par);
	BEGIN
		par.readOnly := ~BugsInterface.IsCompiled() OR BugsInterface.IsInitialized()
	END SetRNGuard;
	
	PROCEDURE SetWindowGuard* (VAR par: Dialog.Par);
	BEGIN
		par.checked := displayDialog.whereOut = BugsFiles.window
	END SetWindowGuard;

	PROCEDURE UpdateGuard* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~BugsInterface.IsInitialized();
		IF par.disabled & script THEN  BugsMsg.Lookup("BugsCmds:NotInits", par.label) END
	END UpdateGuard;

	PROCEDURE AdaptivePhaseGuard* (VAR par: Dialog.Par);
		VAR
			ok: BOOLEAN;
	BEGIN
		UpdaterMethods.AdaptivePhaseGuard(ok);
		IF ~ok THEN
			par.disabled := TRUE;
			IF script THEN  BugsMsg.Lookup("UpdaterMethods:notAdaptive", par.label) END
		END
	END AdaptivePhaseGuard;

	PROCEDURE FactoryGuard* (VAR par: Dialog.Par);
		VAR
			ok: BOOLEAN;
	BEGIN
		UpdaterMethods.FactoryGuard(ok);
		IF ~ok THEN
			par.disabled := TRUE;
			IF script THEN BugsMsg.Lookup("UpdaterMethods:notUpdateMethod", par.label) END
		END
	END FactoryGuard;

	PROCEDURE IterationsGuard* (VAR par: Dialog.Par);
		VAR
			ok: BOOLEAN;
	BEGIN
		UpdaterMethods.IterationsGuard(ok);
		IF ~ok THEN
			par.disabled := TRUE;
			IF script THEN  BugsMsg.Lookup("UpdaterMethods:notIterations", par.label) END
		END
	END IterationsGuard;

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
END BugsCmds.

