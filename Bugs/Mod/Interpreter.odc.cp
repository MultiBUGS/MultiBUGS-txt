(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	  *)

MODULE BugsInterpreter;

	

	CONST
		notModule* = 1;
		invalidArgument* = 2;
		invalidObjectType* = 3;

	TYPE
		Interpreter* = POINTER TO ABSTRACT RECORD END;

	VAR
		interpreter: Interpreter;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		PROCEDURE (i: Interpreter) CharArray- (procedure: ARRAY OF CHAR; VAR x: ARRAY OF CHAR;
	OUT res: INTEGER), NEW, ABSTRACT;

		PROCEDURE (i: Interpreter) CmdInterpreter- (command: ARRAY OF CHAR; OUT res: INTEGER),
	NEW, ABSTRACT;

		PROCEDURE (i: Interpreter) Guard- (procedure: ARRAY OF CHAR; OUT ok: BOOLEAN;
	OUT res: INTEGER), NEW, ABSTRACT;

		PROCEDURE (i: Interpreter) Integer- (procedure: ARRAY OF CHAR; OUT x, res: INTEGER),
	NEW, ABSTRACT;

		PROCEDURE (i: Interpreter) IntegerArray- (procedure: ARRAY OF CHAR; VAR x: ARRAY OF INTEGER;
	OUT res: INTEGER), NEW, ABSTRACT;

		PROCEDURE (i: Interpreter) Real- (procedure: ARRAY OF CHAR; x: REAL; OUT y: REAL;
	OUT res: INTEGER), NEW, ABSTRACT;

		PROCEDURE (i: Interpreter) RealArray- (procedure: ARRAY OF CHAR; VAR x: ARRAY OF REAL;
	OUT res: INTEGER), NEW, ABSTRACT;

	PROCEDURE CmdInterpreter* (command: ARRAY OF CHAR; OUT res: INTEGER);
	BEGIN
		ASSERT(interpreter # NIL, 20);
		interpreter.CmdInterpreter(command, res)
	END CmdInterpreter;

	PROCEDURE CharArray* (procedure: ARRAY OF CHAR; VAR x: ARRAY OF CHAR; OUT res: INTEGER);
	BEGIN
		ASSERT(interpreter # NIL, 20);
		interpreter.CharArray(procedure, x, res)
	END CharArray;

	PROCEDURE Integer* (procedure: ARRAY OF CHAR; OUT x, res: INTEGER);
	BEGIN
		ASSERT(interpreter # NIL, 20);
		interpreter.Integer(procedure, x, res)
	END Integer;

	PROCEDURE IntegerArray* (procedure: ARRAY OF CHAR; VAR x: ARRAY OF INTEGER;
	OUT res: INTEGER);
	BEGIN
		ASSERT(interpreter # NIL, 20);
		interpreter.IntegerArray(procedure, x, res)
	END IntegerArray;

	PROCEDURE Real* (procedure: ARRAY OF CHAR; x: REAL; OUT y: REAL; OUT res: INTEGER);
	BEGIN
		ASSERT(interpreter # NIL, 20);
		interpreter.Real(procedure, x, y, res)
	END Real;

	PROCEDURE RealArray* (procedure: ARRAY OF CHAR; VAR x: ARRAY OF REAL; OUT res: INTEGER);
	BEGIN
		ASSERT(interpreter # NIL, 20);
		interpreter.RealArray(procedure, x, res)
	END RealArray;

	PROCEDURE Guard* (procedure: ARRAY OF CHAR; OUT ok: BOOLEAN; OUT res: INTEGER);
	BEGIN
		ASSERT(interpreter # NIL, 20);
		interpreter.Guard(procedure, ok, res)
	END Guard;

	PROCEDURE SetInterpreter* (i: Interpreter);
	BEGIN
		interpreter := i
	END SetInterpreter;

	PROCEDURE Maintainer;
	BEGIN
		version:= 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		interpreter := NIL
	END Init;

BEGIN
	Init
END BugsInterpreter.



