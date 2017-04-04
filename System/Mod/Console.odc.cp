(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	  *)

MODULE Console;

	

	TYPE
		Console* = POINTER TO ABSTRACT RECORD END;

	VAR
		cons-, stdCon-: Console;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (console: Console) WriteStr- (IN text: ARRAY OF CHAR), NEW, ABSTRACT;

	PROCEDURE (console: Console) WriteChar- (c: CHAR), NEW, ABSTRACT;

	PROCEDURE (console: Console) WriteLn-, NEW, ABSTRACT;

	PROCEDURE (console: Console) ReadLn- (OUT text: ARRAY OF CHAR), NEW, ABSTRACT;

	PROCEDURE (console: Console) Open-, NEW, ABSTRACT;

	PROCEDURE (console: Console) Close-, NEW, ABSTRACT;

	PROCEDURE WriteStr* (IN text: ARRAY OF CHAR);
	BEGIN
		cons.WriteStr(text)
	END WriteStr;

	PROCEDURE WriteChar* (c: CHAR);
	BEGIN
		cons.WriteChar(c)
	END WriteChar;

	PROCEDURE WriteLn*;
	BEGIN
		cons.WriteLn
	END WriteLn;

	PROCEDURE ReadLn* (OUT text: ARRAY OF CHAR);
	BEGIN
		cons.ReadLn(text)
	END ReadLn;

	PROCEDURE Open*;
	BEGIN
		cons.Open
	END Open;

	PROCEDURE Close*;
	BEGIN
		cons.Close
	END Close;

	PROCEDURE SetConsole* (console: Console);
	BEGIN
		cons := console;
		IF stdCon = NIL THEN stdCon := console END
	END SetConsole;

	PROCEDURE Maintainer;
	BEGIN
		version:= 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		cons := NIL;
		stdCon := NIL
	END Init;

BEGIN
	Init
END Console.

