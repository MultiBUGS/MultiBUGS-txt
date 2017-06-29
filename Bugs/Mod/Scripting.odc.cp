(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	  *)

MODULE BugsScripting;

	

	IMPORT 
		Strings, StdLog,
		BugsInterpreter, BugsMappers, BugsMsg, BugsScripts;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

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

	PROCEDURE Script* (VAR s: BugsMappers.Scanner; OUT res: INTEGER);
		VAR
			pos, pos1, i, numPar: INTEGER;
			bugsCommand, pascalCommand, pat: ARRAY 1024 OF CHAR;
			p: ARRAY 10 OF ARRAY 1024 OF CHAR;
	BEGIN
		ParseCommand(s, bugsCommand, p);
		BugsScripts.FindKey(bugsCommand, pascalCommand);
		IF pascalCommand # "" THEN
			i := 0;
			Strings.Find(bugsCommand, "(", 0, pos);
			Strings.Find(bugsCommand, ")", 0, pos1);
			numPar := pos1 - pos - 1;
			WHILE i < numPar DO
				Strings.IntToString(i, pat);
				pat := "^" + pat;
				pos := 0;
				WHILE pos # - 1 DO
					Strings.Find(pascalCommand, pat, 0, pos);
					IF pos # - 1 THEN
						Strings.Replace(pascalCommand, pos, LEN(pat$), p[i])
					END
				END;
				INC(i)
			END;
			StdLog.Ln;StdLog.String(pascalCommand); StdLog.Ln;
			BugsInterpreter.CmdInterpreter(pascalCommand, res); 
		ELSE
			BugsMsg.Show("BugsScript6");
		END
	END Script;

	PROCEDURE Command* (command: ARRAY OF CHAR; OUT res: INTEGER);
		VAR
			beg: INTEGER;
			pat: ARRAY 1024 OF CHAR;
			s: BugsMappers.Scanner;
	BEGIN
		IF command = "" THEN RETURN END;
		s.ConnectToString(command);
		beg := 0;
		s.SetPos(beg);
		s.Scan;
		IF s.eot THEN res := -2; RETURN END;
		Script(s, res)
	END Command;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END BugsScripting.
