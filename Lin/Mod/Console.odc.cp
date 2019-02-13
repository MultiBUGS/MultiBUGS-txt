(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE LinConsole;

	

	IMPORT
		SYSTEM,
		Console,
		LinLibc;

	TYPE
		LinCons = POINTER TO RECORD (Console.Console) END;

	CONST
		strLen = 1024;

	VAR
		s: ARRAY strLen OF CHAR;
		ss: ARRAY strLen OF SHORTCHAR;
		linCons: LinCons;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (cons: LinCons) ReadLn (OUT text: ARRAY OF CHAR);
		VAR
			i: INTEGER;
			str: POINTER TO ARRAY[untagged] OF SHORTCHAR;
	BEGIN
		str := LinLibc.fgets(ss, strLen, LinLibc.stdin);
		IF (str = NIL) THEN
			(* if end of file, then ss is not changed by fgets and NIL is returned.
			We return an empty string here *)
			text[0] := 0X;
			RETURN
		END;
		i := 0;
		REPEAT
			text[i] := ss[i];
			INC(i)
		UNTIL (ss[i] = 0X) OR (i = LEN(text) - 1);
		text[i] := 0X
	END ReadLn;

	PROCEDURE Printf;
		VAR res: INTEGER;
	BEGIN
		res := LinLibc.printf(ss);
		res := LinLibc.fflush(LinLibc.NULL)
	END Printf;

	PROCEDURE (cons: LinCons) WriteChar (c: CHAR);
	BEGIN
		s[0] := c;
		s[1] := 0X;
		ss := SHORT(s);
		Printf()
	END WriteChar;

	PROCEDURE (cons: LinCons) WriteStr (IN text: ARRAY OF CHAR);
	BEGIN
		ss := SHORT(text);
		Printf()
	END WriteStr;

	PROCEDURE (cons: LinCons) WriteLn;
	BEGIN
		ss[0] := 0AX;
		ss[1] := 0X;
		Printf()
	END WriteLn;

	PROCEDURE (cons: LinCons) Open;
	BEGIN
	END Open;

	PROCEDURE (cons: LinCons) Close;
	BEGIN
	END Close;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		NEW(linCons);
		Console.SetConsole(linCons)
	END Init;

BEGIN
	Init
END LinConsole.
