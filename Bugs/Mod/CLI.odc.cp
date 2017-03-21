(* 		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



  *)

MODULE BugsCLI;

	

	IMPORT
		Console, Strings,
		BugsMappers, BugsScripting;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		first: BOOLEAN;

	PROCEDURE Loop*;
		VAR
			str: ARRAY 1025 OF CHAR;
			whereOut: INTEGER;
			i, pos: INTEGER;
	BEGIN
		whereOut := BugsMappers.whereOut;
		BugsMappers.SetDest(BugsMappers.log);
		Console.Open;
		IF first THEN
			first := FALSE;
			Console.WriteStr("OpenBUGS version 4.0.0 rev 000");
			Console.WriteLn;
			Console.WriteStr("type 'modelQuit()' to quit"); Console.WriteLn;
		END;
		LOOP
			Console.WriteStr("OpenBUGS> ");
			Console.ReadLn(str);
			(* ReadLn preserves newline, so 0X indicates EOF in input script *)
			IF (str[0] = 0X) THEN EXIT END;
			i := 0;
			WHILE (i < LEN(str)) & (str[i] # "#") DO INC(i) END;
			IF i < LEN(str) THEN str[i] := 0X END;
			(* if string is just newline, then strip it, to avoid "unknown command" *)
			IF (str[0] = 0AX) OR (str[0] = 0DX) THEN str[0] := 0X END;
			Strings.Find(str, "modelQuit()", 0, pos);
			IF pos # - 1 THEN EXIT END;
			Strings.Find(str, "modelQuit('y')", 0, pos);
			IF pos # - 1 THEN EXIT END;
			Strings.Find(str, "modelQuit('yes')", 0, pos);
			IF pos # - 1 THEN EXIT END;
			Strings.Find(str, "modelQuit('Y')", 0, pos);
			IF pos # - 1 THEN EXIT END;
			Strings.Find(str, "modelQuit('YES')", 0, pos);
			IF pos # - 1 THEN EXIT END;
			BugsScripting.EmbedCommand(str);
		END;
		Console.Close;
		BugsMappers.SetDest(whereOut)
	END Loop;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		first := TRUE
	END Init;

BEGIN
	Init
END BugsCLI.

