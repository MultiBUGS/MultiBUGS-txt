(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	  *)

MODULE BugsScripting;

	

	IMPORT 
		Files, Strings,
		BugsFiles, BugsInterpreter, BugsMappers, BugsMsg, BugsStrings;

	TYPE
		ScriptCommand = POINTER TO RECORD;
			name, pascal, embedPascal: POINTER TO ARRAY OF CHAR;
			numPar: INTEGER;
			next: ScriptCommand
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		scriptCommands: ScriptCommand;

		PROCEDURE ParseCommand (VAR s: BugsMappers.Scanner;
	OUT command: ARRAY OF CHAR; OUT p: ARRAY OF ARRAY OF CHAR);
		VAR
			argNum, i, integer, res: INTEGER;
			indices, char: ARRAY 1024 OF CHAR;
	BEGIN
		command := "";
		i := 0;
		WHILE i < LEN(p) DO
			p[i] := "";
			INC(i)
		END;
		IF (s.type # BugsMappers.string) & (s.type # BugsMappers.function) THEN
			RETURN
		END;
		command := s.string$;
		s.Scan;
		IF (s.type # BugsMappers.char) OR (s.char # "(") THEN
			RETURN
		END;
		command := command + "(";
		argNum := 0;
		REPEAT
			s.Scan;
			(*	read script command parameters into p vector 	*)
			IF (s.type = BugsMappers.string) OR (s.type = BugsMappers.int) THEN
				p[argNum] := s.string$;
				s.Scan;
				IF (s.type = BugsMappers.char) & (s.char = "[") & ~s.eot THEN
					indices := "[";
					REPEAT
						s.Scan;
						IF (s.type = BugsMappers.string) OR (s.type = BugsMappers.int) THEN
							indices := indices + s.string
						ELSIF s.type = BugsMappers.char THEN
							char[0] := s.char;
							char[1] := 0X;
							indices := indices + char
						END
					UNTIL ((s.type = BugsMappers.char) & (s.char = "]")) OR s.eot;
					p[argNum] := p[argNum] + indices;
					IF ~s.eot THEN
						s.Scan
					END
				END;
				INC(argNum)
			END
		UNTIL ((s.type = BugsMappers.char) & (s.char = ")")) OR s.eot;
		i := 0;
		WHILE i < argNum DO
			Strings.StringToInt(p[i], integer, res);
			IF res = 0 THEN
				command := command + "i"
			ELSE
				command := command + "s"
			END;
			INC(i)
		END;
		IF (s.type = BugsMappers.char) & (s.char = ")") THEN
			command := command + ")"
		END;
		IF ~s.eot THEN
			s.Scan
		END
	END ParseCommand;

	PROCEDURE LookUpCommand (name: ARRAY OF CHAR): ScriptCommand;
		VAR
			cursor: ScriptCommand;
	BEGIN
		cursor := scriptCommands;
		WHILE (cursor # NIL) & (cursor.name^ # name) DO
			cursor := cursor.next
		END;
		RETURN cursor
	END LookUpCommand;

	PROCEDURE Script* (VAR s: BugsMappers.Scanner);
		VAR
			pos, res: INTEGER;
			msg, name, pascal, pat: ARRAY 1024 OF CHAR;
			p: ARRAY 10 OF ARRAY 1024 OF CHAR;
			i, numPar: INTEGER;
			scriptCommand: ScriptCommand;
	BEGIN
		ParseCommand(s, name, p);
		scriptCommand := LookUpCommand(name);
		IF (scriptCommand # NIL) & (scriptCommand.pascal^ # "") THEN
			i := 0;
			numPar := scriptCommand.numPar;
			pascal := scriptCommand.pascal$;
			WHILE i < numPar DO
				Strings.IntToString(i, pat);
				pat := "^" + pat;
				pos := 0;
				WHILE pos # - 1 DO
					Strings.Find(pascal, pat, 0, pos);
					IF pos # - 1 THEN
						Strings.Replace(pascal, pos, LEN(pat$), p[i])
					END
				END;
				INC(i)
			END;
			BugsInterpreter.CmdInterpreter(pascal, res);
			ASSERT(res = 0, 20);
		ELSE
			BugsMsg.MapMsg("BugsScript6", msg);
			BugsFiles.ShowMsg(msg)
		END
	END Script;

	PROCEDURE EmbedScript* (VAR s: BugsMappers.Scanner);
		VAR
			pos, res: INTEGER;
			msg, name, pascal, pat: ARRAY 1024 OF CHAR;
			p: ARRAY 10 OF ARRAY 1024 OF CHAR;
			i, numPar: INTEGER;
			scriptCommand: ScriptCommand;
	BEGIN
		ParseCommand(s, name, p);
		scriptCommand := LookUpCommand(name);
		IF (scriptCommand # NIL) & (scriptCommand.embedPascal^ # "") THEN
			i := 0;
			numPar := scriptCommand.numPar;
			pascal := scriptCommand.embedPascal$;
			WHILE i < numPar DO
				Strings.IntToString(i, pat);
				pat := "^" + pat;
				pos := 0;
				WHILE pos # - 1 DO
					Strings.Find(pascal, pat, 0, pos);
					IF pos # - 1 THEN
						Strings.Replace(pascal, pos, LEN(pat$), p[i])
					END
				END;
				INC(i)
			END;
			BugsInterpreter.CmdInterpreter(pascal, res);
			ASSERT(res = 0, 60);
		ELSE
			BugsMsg.MapMsg("BugsScript6", msg);
			BugsFiles.ShowMsg(msg)
		END
	END EmbedScript;

	PROCEDURE ProofCommand (VAR command: ARRAY OF CHAR; OUT res: INTEGER);
		VAR
			i, nOpenBraket, nOpenDQ, nOpenSB, nOpenSQ: INTEGER;
	BEGIN
		res := 0;
		nOpenBraket := 0;
		nOpenDQ := 0;
		nOpenSB := 0;
		nOpenSQ := 0;
		i := 0;
		WHILE (command[i] # 0X) & (command[i] # "#") DO
			CASE command[i] OF
			|"(": INC(nOpenBraket)
			|")": DEC(nOpenBraket)
			|'"': INC(nOpenSQ)
			|"'": INC(nOpenDQ)
			|"[": INC(nOpenSB)
			|"]": DEC(nOpenSB)
			ELSE
			END;
			INC(i)
		END;
		IF command[i] = "#" THEN command[i] := 0X END;
		IF nOpenBraket # 0 THEN
			res := 1
		ELSIF ODD(nOpenSQ) THEN
			res := 2
		ELSIF ODD(nOpenDQ) THEN
			res := 3
		ELSIF nOpenSB # 0 THEN
			res := 4
		ELSIF (nOpenDQ > 0) & (nOpenSQ > 0) THEN
			res := 5
		END
	END ProofCommand;

	PROCEDURE Command* (command: ARRAY OF CHAR);
		VAR
			beg, res: INTEGER;
			msg, pat: ARRAY 1024 OF CHAR;
			s: BugsMappers.Scanner;
	BEGIN
		IF command = "" THEN RETURN END;
		ProofCommand(command, res);
		IF command = "" THEN RETURN END;
		IF res # 0 THEN
			Strings.IntToString(res, pat);
			pat := "#Bugs:Script" + pat;
			BugsMsg.MapMsg(pat, msg);
			BugsFiles.ShowMsg(msg);
			RETURN
		END;
		BugsStrings.ConnectScanner(s, command);
		beg := 0;
		s.SetPos(beg);
		s.Scan;
		IF s.eot THEN RETURN END;
		Script(s)
	END Command;

	PROCEDURE EmbedCommand* (command: ARRAY OF CHAR);
		VAR
			beg, res: INTEGER;
			msg, pat: ARRAY 1024 OF CHAR;
			s: BugsMappers.Scanner;
	BEGIN
		IF command = "" THEN RETURN END;
		ProofCommand(command, res);
		IF command = "" THEN RETURN END;
		IF res # 0 THEN
			Strings.IntToString(res, pat);
			pat := "#Bugs:Script" + pat;
			BugsMsg.MapMsg(pat, msg);
			BugsFiles.ShowMsg(msg);
			RETURN
		END;
		BugsStrings.ConnectScanner(s, command);
		beg := 0;
		s.SetPos(beg);
		s.Scan;
		EmbedScript(s)
	END EmbedCommand;

	PROCEDURE StripComment (VAR s: BugsMappers.Scanner);
	BEGIN
		IF (s.type = BugsMappers.char) & (s.char = "(") & (s.nextCh = "*") THEN
			REPEAT s.Scan UNTIL (s.type = BugsMappers.char) & (s.char = "*") & (s.nextCh = ")");
			s.Scan;
			s.Scan;
			StripComment(s)
		END;
	END StripComment;

	PROCEDURE LoadCommand* (IN com, actionWindows, actionEmbed: ARRAY OF CHAR);
		VAR
			len: INTEGER;
			command: ScriptCommand;
			s: BugsMappers.Scanner;
			name: ARRAY 1024 OF CHAR;
	BEGIN
		NEW(command);
		command.numPar := 0;
		command.next := scriptCommands;
		scriptCommands := command;
		BugsStrings.ConnectScanner(s, com);
		s.SetPos(0);
		s.Scan;
		name := s.string$;
		s.Scan;
		name := name + "(";
		REPEAT
			s.Scan;
			IF s.type = BugsMappers.string THEN
				name := name + s.string;
				INC(command.numPar)
			END
		UNTIL (s.type = BugsMappers.char) & (s.char = ")");
		name := name + ")";
		len := LEN(name$);
		NEW(command.name, len + 1);
		command.name^ := name$;
		len := LEN(actionWindows$);
		NEW(command.pascal, len + 1);
		command.pascal^ := actionWindows$;
		len := LEN(actionEmbed$);
		NEW(command.embedPascal, len + 1);
		command.embedPascal^ := actionEmbed$;
	END LoadCommand;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		scriptCommands := NIL;
	END Init;

BEGIN
	Init
END BugsScripting.
