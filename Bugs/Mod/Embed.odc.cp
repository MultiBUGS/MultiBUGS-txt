(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsEmbed;

	

	IMPORT
		Files, Services, Strings,
		BugsFiles, BugsGraph, BugsInterface, BugsMappers, BugsMsg, 
		BugsRandnum, BugsScripting, BugsSerialize, 
		MathRandnum,
		UpdaterActions, UpdaterMethods;

	TYPE
		Globals* = POINTER TO ABSTRACT RECORD
			next: Globals
		END;

		Params = POINTER TO RECORD(Globals) END;

	VAR
		compileCP*, updaterByMethod*, overRelax*, isScript*, fixFounder*: BOOLEAN;
		chain*, index*, numChains*, numProc*, preSet*,
		thin*, updates*: INTEGER;
		startTime: LONGINT;
		filePath, node: ARRAY 1024 OF CHAR;
		globals: Globals;

		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (g: Globals) Init-, NEW, ABSTRACT;

	PROCEDURE (g: Globals) Update-, NEW, EMPTY;

	PROCEDURE (p: Params) Init;
	BEGIN
		chain := 1;
		index := 1;
		numChains := 1;
		numProc := 1;
		preSet := 1;
		thin := 1;
		overRelax := FALSE;
		compileCP := FALSE;
		fixFounder := TRUE;
		updates := 1000
	END Init;

	PROCEDURE InitGlobals;
		VAR
			cursor: Globals;
	BEGIN
		cursor := globals;
		WHILE cursor # NIL DO
			cursor.Init;
			cursor := cursor.next
		END
	END InitGlobals;

	PROCEDURE UpdateGlobals;
		VAR
			cursor: Globals;
	BEGIN
		cursor := globals;
		WHILE cursor # NIL DO
			cursor.Update;
			cursor := cursor.next
		END
	END UpdateGlobals;

	PROCEDURE AddGlobals* (g: Globals);
	BEGIN
		g.next := globals;
		globals := g
	END AddGlobals;

	PROCEDURE File (path: ARRAY OF CHAR): Files.File;
		VAR
			file: Files.File;
			name: Files.Name;
			loc: Files.Locator;
	BEGIN
		BugsFiles.PathToFileSpec(path, loc, name);
		ASSERT(loc # NIL, 100); ASSERT(loc.res = 0, 101);
		file := Files.dir.Old(loc, name, Files.shared);
		RETURN file
	END File;

	PROCEDURE Error;
		VAR
			errorMes: ARRAY 1024 OF CHAR;
	BEGIN
		BugsMsg.GetError(errorMes);
		BugsFiles.ShowMsg(errorMes)
	END Error;

	PROCEDURE FileError (VAR s: BugsMappers.Scanner);
		VAR
			beg, end, linesRead: INTEGER;
			errorMes, string: ARRAY 1024 OF CHAR;
	BEGIN
		end := s.Pos();
		CASE s.type OF
		|BugsMappers.int, BugsMappers.real, BugsMappers.string, BugsMappers.function:
			beg := end - s.len
		ELSE
			beg := end - 1
		END;
		linesRead := s.linesRead;
		BugsMsg.GetError(errorMes);
		errorMes := errorMes + " error pos ";
		Strings.IntToString(beg, string);
		errorMes := errorMes + string + " (error on line ";
		Strings.IntToString(linesRead + 1, string);
		errorMes := errorMes + string + ")";
		BugsFiles.ShowMsg(errorMes)
	END FileError;

	PROCEDURE SetFilePath* (path: ARRAY OF CHAR);
	BEGIN
		filePath := path$
	END SetFilePath;

	PROCEDURE SetNode* (var: ARRAY OF CHAR);
	BEGIN
		node := var$
	END SetNode;

	(* 	Command to parse BUGS model 	*)

	PROCEDURE Parse*;
		VAR
			s: BugsMappers.Scanner;
			file: Files.File;
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		BugsInterface.Clear;
		InitGlobals;
		file := File(filePath);
		IF file = NIL THEN RETURN END;
		BugsFiles.ConnectScanner(s, file);
		s.SetPos(0);
		BugsInterface.ParseModel(s);
		IF BugsInterface.IsParsed() THEN
			BugsMsg.MapMsg("BugsEmbed:OkSyntax", msg);
			BugsFiles.ShowMsg(msg)
		ELSE
			FileError(s)
		END;
		s.SetReader(NIL);
		file.Close;
		file := NIL
	END Parse;

	(*	Command to load data	*)

	PROCEDURE LoadData*;
		VAR
			ok: BOOLEAN;
			s: BugsMappers.Scanner;
			file: Files.File;
			msg, warning: ARRAY 1024 OF CHAR;
	BEGIN
		BugsMsg.StoreMsg("");
		IF ~BugsInterface.IsParsed() THEN
			RETURN
		END;
		file := File(filePath);
		IF file = NIL THEN RETURN END;
		BugsFiles.ConnectScanner(s, file);
		s.SetPos(0);
		BugsInterface.LoadData(s, ok);
		IF ok THEN
			BugsMsg.MapMsg("BugsEmbed:OkData", msg);
			BugsMsg.GetMsg(warning); (* If warning not encountered, will contain empty string *)
			BugsFiles.ShowMsg(msg + warning)
		ELSE
			FileError(s)
		END;
		s.SetReader(NIL);
		file.Close;
		file := NIL
	END LoadData;

	(*	Command to write model graph	*)

	PROCEDURE Compile*;
		VAR
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		IF ~BugsInterface.IsParsed() THEN RETURN END;
		BugsGraph.Compile(numChains, updaterByMethod);
		IF BugsInterface.IsCompiled() THEN
			UpdateGlobals;
			BugsMsg.MapMsg("BugsEmbed:OkCompile", msg);
			BugsFiles.ShowMsg(msg)
		END
	END Compile;

	(*	Command to load initial values	*)

	PROCEDURE LoadInits*;
		VAR
			s: BugsMappers.Scanner;
			file: Files.File;
			msg, warning: ARRAY 1024 OF CHAR;
			numberChains: INTEGER;
			ok: BOOLEAN;
	BEGIN
		IF ~BugsInterface.IsCompiled() THEN RETURN END;
		BugsMsg.StoreMsg("");
		numberChains := BugsRandnum.numberChains;
		file := File(filePath);
		IF file = NIL THEN RETURN END;
		BugsFiles.ConnectScanner(s, file);
		s.SetPos(0);
		BugsInterface.LoadInits(s, chain - 1, numberChains, fixFounder, ok);
		IF ok THEN
			IF BugsInterface.IsInitialized() THEN
				BugsMsg.MapMsg("BugsEmbed:OkInits", msg)
			ELSIF UpdaterActions.IsInitialized(chain - 1) THEN
				BugsMsg.MapMsg("BugsEmbed:UninitOther", msg)
			ELSE
				BugsMsg.MapMsg("BugsEmbed:NotInit", msg)
			END;
			BugsMsg.GetMsg(warning); (* If warning not encountered, will contain empty string *)
			BugsFiles.ShowMsg(msg + warning);
			chain := chain MOD numberChains;
			INC(chain)
		ELSE
			FileError(s)
		END;
		s.SetReader(NIL);
		file.Close;
		file := NIL
	END LoadInits;

	(*	Commands to generate initial values	*)

	PROCEDURE GenerateInits*;
		VAR
			msg: ARRAY 1024 OF CHAR;
			ok: BOOLEAN;
	BEGIN
		IF ~BugsInterface.IsCompiled() THEN RETURN END;
		BugsInterface.GenerateInits(BugsRandnum.numberChains, fixFounder, ok);
		IF ok THEN
			BugsMsg.MapMsg("BugsEmbed:OkGenInits", msg);
			BugsFiles.ShowMsg(msg)
		ELSE
			Error
		END
	END GenerateInits;

	PROCEDURE Update*;
		CONST
			eps = 1.0E-20;
		VAR
			ok: BOOLEAN;
			i, numberChains: INTEGER;
			elapsedTime: LONGINT;
			msg: ARRAY 1024 OF CHAR;
			p: ARRAY 2 OF ARRAY 1024 OF CHAR;
	BEGIN
		numberChains := BugsRandnum.numberChains;
		startTime := Services.Ticks();
		thin := MAX(1, thin);
		i := 0;
		ok := TRUE;
		WHILE(i < updates) & ok DO
			INC(i);
			BugsInterface.UpdateModel(numberChains, thin, overRelax, ok);
			IF ok THEN
				BugsInterface.UpdateMonitors(numberChains);
			END
		END;
		BugsInterface.RecvSamples;
		IF ok THEN
			elapsedTime := Services.Ticks() - startTime;
			elapsedTime := ENTIER(1.0 * elapsedTime / Services.resolution + eps);
			Strings.IntToString(updates, p[0]);
			Strings.IntToString(elapsedTime, p[1]);
			BugsMsg.MapParamMsg("BugsEmbed:UpdatesTook", p, msg);
			BugsFiles.ShowMsg(msg)
		ELSE
			Error
		END
	END Update;

	PROCEDURE Quit*;
	BEGIN
	END Quit;

	PROCEDURE Script* (path: ARRAY OF CHAR);
		VAR
			beg: INTEGER;
			s: BugsMappers.Scanner;
			file: Files.File;
			loc: Files.Locator;
			name: Files.Name;
	BEGIN
		BugsFiles.PathToFileSpec(path, loc, name);
		file := Files.dir.Old(loc, name, Files.dontAsk);
		BugsMappers.SetDest(BugsMappers.log);
		BugsFiles.ConnectScanner(s, file);
		beg := 0;
		s.SetPos(beg);
		s.Scan;
		WHILE ~s.eot & ((s.type = BugsMappers.string) OR (s.type = BugsMappers.function)) DO
			BugsScripting.EmbedScript(s)
		END;
		BugsFiles.ShowMsg("Script " + path + " run")
	END Script;

	PROCEDURE GetRNState*;
		VAR
			string: ARRAY 1024 OF CHAR;
	BEGIN
		Strings.IntToString(preSet, string);
		BugsFiles.ShowMsg("new state is " + string)
	END GetRNState;

	PROCEDURE SetRNState* (state: INTEGER);
		CONST
			numberPresetStates = 14;
	BEGIN
		IF state < 1 THEN
			state := 1
		ELSIF state > numberPresetStates THEN
			preSet := numberPresetStates
		END;
		MathRandnum.InitState(state - 1);
		preSet := state
	END SetRNState;

	PROCEDURE ChangeSampler*;
		VAR
			ok: BOOLEAN;
			s: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		BugsInterface.ChangeSampler(node, UpdaterMethods.index, ok);
		IF ~ok THEN
			p[0] := node$;
			BugsMsg.MapParamMsg("BugsEmbed:couldNotChangeUpdater", p, s);
			BugsFiles.ShowMsg(s)
		END
	END ChangeSampler;

	PROCEDURE AllocatedMemory*;
		VAR
			msg: ARRAY 128 OF CHAR;
			alloc: INTEGER;
	BEGIN
		alloc := BugsInterface.AllocatedMemory();
		Strings.IntToString(alloc, msg);
		msg := msg + " bytes of memory allocated";
		BugsFiles.ShowMsg(msg)
	END AllocatedMemory;

	PROCEDURE Distribute* (mpiImplementation: ARRAY OF CHAR);
		VAR
			msg: ARRAY 1024 OF CHAR;		
	BEGIN
		BugsInterface.Distribute(mpiImplementation, numProc, numChains);
		IF BugsInterface.IsDistributed() THEN
			BugsMsg.StoreMsg("model distributed")
		END;
		BugsMsg.GetError(msg);
		BugsFiles.ShowMsg(msg)
	END Distribute;

	PROCEDURE GetWD*;
	BEGIN
		IF BugsFiles.workingDir # "" THEN
			BugsFiles.ShowMsg(BugsFiles.workingDir)
		ELSE
			BugsFiles.ShowMsg("working directory not set")
		END
	END GetWD;

	PROCEDURE ExternalizeModel* (fileName: ARRAY OF CHAR);
		VAR
			name: Files.Name;
			loc, locOld: Files.Locator;
	BEGIN
		BugsFiles.PathToFileSpec(fileName, loc, name);
		locOld := BugsSerialize.GetRestartLoc();
		BugsSerialize.SetRestartLoc(loc);
		BugsSerialize.Externalize(name);
		BugsFiles.ShowMsg("model externalized to file ok");
		BugsSerialize.SetRestartLoc(locOld)
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
			BugsFiles.ShowMsg(mes);
			RETURN
		END;
		f.Close;
		BugsSerialize.Internalize(name);
		numChains := BugsRandnum.numberChains;
		UpdateGlobals;
		BugsFiles.ShowMsg("model internalized from file ok");
		BugsSerialize.SetRestartLoc(locOld)
	END InternalizeModel;

	PROCEDURE ParseGuard* (OUT ok: BOOLEAN);
		VAR
			s: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
			f: Files.File;
	BEGIN
		f := File(filePath);
		ok := f # NIL;
		IF ok THEN
			f.Close
		ELSE
			p[0] := filePath$;
			BugsMsg.MapParamMsg("BugsEmbed:NoFile", p, s);
			BugsFiles.ShowMsg(s)
		END
	END ParseGuard;

	PROCEDURE LoadDataGuard* (OUT ok: BOOLEAN);
		VAR
			s: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
			f: Files.File;
	BEGIN
		f := File(filePath);
		ok := f # NIL;
		IF ok THEN
			f.Close;
			ok := BugsInterface.IsParsed();
			IF ~ok THEN
				BugsMsg.MapMsg("BugsEmbed:NoCheckData", s);
				BugsFiles.ShowMsg(s)
			END
		ELSE
			p[0] := filePath$;
			BugsMsg.MapParamMsg("BugsEmbed:NoFile", p, s);
			BugsFiles.ShowMsg(s)
		END
	END LoadDataGuard;

	PROCEDURE CompileGuard* (OUT ok: BOOLEAN);
		VAR
			s: ARRAY 1024 OF CHAR;
	BEGIN
		ok := BugsInterface.IsParsed();
		IF ~ok THEN
			BugsMsg.MapMsg("BugsEmbed:NoCheckCompile", s);
			BugsFiles.ShowMsg(s)
		END
	END CompileGuard;

	PROCEDURE CompiledGuard* (OUT ok: BOOLEAN);
		VAR
			s: ARRAY 1024 OF CHAR;
	BEGIN
		ok := ~BugsInterface.IsCompiled();
		IF ~ok THEN
			BugsMsg.MapMsg("BugsCmds:AlreadyCompiled", s);
			BugsFiles.ShowMsg(s)
		END
	END CompiledGuard;

	PROCEDURE NotCompiledGuard* (OUT ok: BOOLEAN);
		VAR
			s: ARRAY 1024 OF CHAR;
	BEGIN
		ok := ~BugsInterface.IsCompiled();
		IF ~ok THEN
			BugsMsg.MapMsg("BugsCmds:NotCompiled", s);
			BugsFiles.ShowMsg(s)
		END
	END NotCompiledGuard;

	PROCEDURE LoadInitsGuard* (OUT ok: BOOLEAN);
		VAR
			s: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
			f: Files.File;
	BEGIN
		f := File(filePath);
		ok := f # NIL;
		IF ok THEN
			ok := BugsInterface.IsCompiled();
			IF ~ok THEN
				BugsMsg.MapMsg("BugsEmbed:NoCompileInits", s);
				BugsFiles.ShowMsg(s)
			END
		ELSE
			p[0] := filePath$;
			BugsMsg.MapParamMsg("BugsEmbed:NoFile", p, s);
			BugsFiles.ShowMsg(s)
		END
	END LoadInitsGuard;

	PROCEDURE GenerateInitsGuard* (OUT ok: BOOLEAN);
		VAR
			s: ARRAY 1024 OF CHAR;
	BEGIN
		ok := BugsInterface.IsCompiled();
		IF ok THEN
			ok := ~BugsInterface.IsInitialized();
			IF ~ok THEN
				BugsMsg.MapMsg("BugsEmbed:AlreadyInits", s);
				BugsFiles.ShowMsg(s)
			END
		END
	END GenerateInitsGuard;

	PROCEDURE UpdateGuard* (OUT ok: BOOLEAN);
		VAR
			s: ARRAY 1024 OF CHAR;
	BEGIN
		ok := BugsInterface.IsInitialized();
		IF ~ok THEN
			BugsMsg.MapMsg("BugsEmbed:NotInits", s);
			BugsFiles.ShowMsg(s)
		END
	END UpdateGuard;

	PROCEDURE SetRNGuard* (OUT ok: BOOLEAN);
		VAR
			s: ARRAY 1024 OF CHAR;
	BEGIN
		ok := BugsInterface.IsCompiled() OR (UpdaterActions.iteration # 0);
		IF ~ok THEN
			s := "model must have been compiled but not updated to be able to change RN generator";
			BugsFiles.ShowMsg(s)
		END
	END SetRNGuard;

	PROCEDURE FactoryGuard* (OUT ok: BOOLEAN);
		VAR
			s: ARRAY 1024 OF CHAR;
	BEGIN
		UpdaterMethods.FactoryGuard(ok);
		IF ~ok THEN
			BugsMsg.MapMsg("UpdaterMethods:notUpdateMethod", s);
			BugsFiles.ShowMsg(s)
		END
	END FactoryGuard;

	PROCEDURE AdaptivePhaseGuard* (OUT ok: BOOLEAN);
		VAR
			s: ARRAY 1024 OF CHAR;
	BEGIN
		UpdaterMethods.AdaptivePhaseGuard(ok);
		IF ~ok THEN
			BugsMsg.MapMsg("UpdaterMethods:notAdaptive", s);
			BugsFiles.ShowMsg(s)
		END
	END AdaptivePhaseGuard;

	PROCEDURE IterationsGuard* (OUT ok: BOOLEAN);
		VAR
			s: ARRAY 1024 OF CHAR;
	BEGIN
		UpdaterMethods.IterationsGuard(ok);
		IF ~ok THEN
			BugsMsg.MapMsg("UpdaterMethods.notIterations", s);
			BugsFiles.ShowMsg(s)
		END
	END IterationsGuard;

	PROCEDURE OverRelaxationGuard* (OUT ok: BOOLEAN);
		VAR
			s: ARRAY 1024 OF CHAR;
	BEGIN
		UpdaterMethods.OverRelaxationGuard(ok);
		IF ~ok THEN
			BugsMsg.MapMsg("UpdaterMethods:notOverRelax", s);
			BugsFiles.ShowMsg(s)
		END
	END OverRelaxationGuard;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			params: Params;
	BEGIN
		globals := NIL;
		NEW(params);
		params.Init;
		AddGlobals(params);
		compileCP := TRUE;
		updaterByMethod := TRUE;
		isScript := FALSE;
		fixFounder := TRUE;
		Maintainer
	END Init;

BEGIN
	Init
CLOSE
	BugsInterface.Clear
END BugsEmbed.


