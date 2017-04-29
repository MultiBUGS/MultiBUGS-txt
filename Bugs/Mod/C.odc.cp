(*		GNU General Public Licence	*)


(*
	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"
*)

MODULE BugsC;

	(*

	This module is the interface to the BUGS software with C friendly procedure definitions. With the
	exception of initialization procedures the procedures are just wrappers round the procedures in
	BugsInterpreter.

	*)

	

	IMPORT
		SYSTEM,
		Meta,
		BugsFiles, BugsInterpreter, BugsScripting;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE[code] FCLEX 0DBH, 0E2H;
	PROCEDURE[code] FLDCW 0D9H, 06DH, 0FCH; 	(* -4, FP *)
	PROCEDURE[code] FSTCW 0D9H, 07DH, 0FCH; 	(* -4, FP *)

	PROCEDURE SetBBFpu;
		VAR
			cw: SET;
	BEGIN
		FSTCW;
		IF cw * {0..5, 8..11} # {1, 2, 3, 4, 5, 8, 9} THEN
			cw := cw - {0..5, 8..11} + {1, 2, 3, 4, 5, 8, 9};
			FCLEX;
			FLDCW
		END
	END SetBBFpu;

	PROCEDURE SetRFpu;
		VAR
			cw: SET;
	BEGIN
		FSTCW;
		cw := cw + {0};
		FCLEX;
		FLDCW
	END SetRFpu;

	PROCEDURE[ccall] BugsCmd* (VAR command: POINTER TO ARRAY[untagged] OF SHORTCHAR;
	VAR len, res: INTEGER);
	VAR
		i: INTEGER;
		string: POINTER TO ARRAY OF CHAR;
	BEGIN
		SetBBFpu;
		NEW(string, len + 1);
		i := 0; WHILE i < len DO string[i] := LONG(command[i]); INC(i) END; string[len] := 0X;
		BugsScripting.Command(string, res);
		SetRFpu
	END BugsCmd;
	
	PROCEDURE[ccall] CharArray* (VAR procedure: POINTER TO ARRAY[untagged] OF SHORTCHAR;
	VAR len: INTEGER; VAR x: POINTER TO ARRAY[untagged] OF SHORTCHAR;
	VAR lenX, res: INTEGER);
		VAR
			i: INTEGER;
			string, array: POINTER TO ARRAY OF CHAR;
	BEGIN
		SetBBFpu;
		NEW(string, len + 1);
		i := 0; WHILE i < len DO string[i] := LONG(procedure[i]); INC(i) END; string[len] := 0X;
		NEW(array, lenX + 1);
		i := 0; WHILE i < lenX DO array[i] := LONG(x[i]); INC(i) END;
		array[lenX] := 0X;
		BugsInterpreter.CharArray(string, array, res);
		i := 0; WHILE i < lenX DO x[i] := SHORT(array[i]); INC(i) END;
		SetRFpu
	END CharArray;

	PROCEDURE[ccall] CmdInterpreter* (VAR command: POINTER TO ARRAY[untagged] OF SHORTCHAR;
	VAR len, res: INTEGER);
	VAR
		i: INTEGER;
		string: POINTER TO ARRAY OF CHAR;
	BEGIN
		SetBBFpu;
		NEW(string, len + 1);
		i := 0; WHILE i < len DO string[i] := LONG(command[i]); INC(i) END; string[len] := 0X;
		BugsInterpreter.CmdInterpreter(string, res);
		SetRFpu
	END CmdInterpreter;

	PROCEDURE[ccall] Guard* (VAR procedure: POINTER TO ARRAY[untagged] OF SHORTCHAR;
	VAR len, x, res: INTEGER);
		VAR
			ok: BOOLEAN;
			i: INTEGER;
			string: POINTER TO ARRAY OF CHAR;
	BEGIN
		SetBBFpu;
		NEW(string, len + 1);
		i := 0; WHILE i < len DO string[i] := LONG(procedure[i]); INC(i) END; string[len] := 0X;
		BugsInterpreter.Guard(string, ok, res);
		IF (res = 0) & ok THEN x := 1 ELSE x := 0 END;
		SetRFpu
	END Guard;

	PROCEDURE[ccall] Integer* (VAR procedure: POINTER TO ARRAY[untagged] OF SHORTCHAR;
	VAR len, x, res: INTEGER);
		VAR
			i: INTEGER;
			string: POINTER TO ARRAY OF CHAR;
	BEGIN
		SetBBFpu;
		x := MIN(INTEGER);
		NEW(string, len + 1);
		i := 0; WHILE i < len DO string[i] := LONG(procedure[i]); INC(i) END; string[len] := 0X;
		BugsInterpreter.Integer(string, x, res);
		SetRFpu
	END Integer;

	PROCEDURE[ccall] IntegerArray* (VAR procedure: POINTER TO ARRAY[untagged] OF SHORTCHAR;
	VAR len: INTEGER;
	VAR x: ARRAY[untagged] OF INTEGER;
	VAR lenX, res: INTEGER);
		VAR
			i: INTEGER;
			string: POINTER TO ARRAY OF CHAR;
			array: POINTER TO ARRAY OF INTEGER;
	BEGIN
		SetBBFpu;
		NEW(string, len + 1);
		i := 0; WHILE i < len DO string[i] := LONG(procedure[i]); INC(i) END; string[len] := 0X;
		NEW(array, lenX);
		i := 0; WHILE i < lenX DO array[i] := x[i]; INC(i) END;
		BugsInterpreter.IntegerArray(string, array, res);
		i := 0; WHILE i < lenX DO x[i] := array[i]; INC(i) END;
		SetRFpu
	END IntegerArray;

	PROCEDURE[ccall] Real* (VAR procedure: POINTER TO ARRAY[untagged] OF SHORTCHAR;
	VAR len: INTEGER;
	VAR x, y: REAL; VAR res: INTEGER);
		VAR
			i: INTEGER;
			string: POINTER TO ARRAY OF CHAR;
	BEGIN
		SetBBFpu;
		NEW(string, len + 1);
		i := 0; WHILE i < len DO string[i] := LONG(procedure[i]); INC(i) END; string[len] := 0X;
		BugsInterpreter.Real(string, x, y, res);
		SetRFpu
	END Real;

	PROCEDURE[ccall] RealArray* (VAR procedure: POINTER TO ARRAY[untagged] OF SHORTCHAR;
	VAR len: INTEGER;
	VAR x: ARRAY[untagged] OF REAL;
	VAR lenX, res: INTEGER);
		VAR
			i: INTEGER;
			string: POINTER TO ARRAY OF CHAR;
			array: POINTER TO ARRAY OF REAL;
	BEGIN
		SetBBFpu;
		NEW(string, len + 1);
		i := 0; WHILE i < len DO string[i] := LONG(procedure[i]); INC(i) END; string[len] := 0X;
		NEW(array, lenX);
		i := 0; WHILE i < lenX DO array[i] := x[i]; INC(i) END;
		BugsInterpreter.RealArray(string, array, res);
		i := 0; WHILE i < lenX DO x[i] := array[i]; INC(i) END;
		SetRFpu
	END RealArray;

	PROCEDURE[ccall] SetWorkingDir* (VAR path: POINTER TO ARRAY[untagged] OF SHORTCHAR;
	VAR len: INTEGER);
		VAR
			i: INTEGER;
			string: POINTER TO ARRAY OF CHAR;
	BEGIN
		SetBBFpu;
		NEW(string, len + 1);
		i := 0; WHILE i < len DO string[i] := LONG(path[i]); INC(i) END; 
		string[len] := 0X;
		BugsFiles.SetWorkingDir(string);
		SetRFpu;
	END SetWorkingDir;

	PROCEDURE[ccall] SetTempDir* (VAR path: POINTER TO ARRAY[untagged] OF SHORTCHAR;
	VAR len: INTEGER);
		VAR
			i: INTEGER;
			string: POINTER TO ARRAY OF CHAR;
	BEGIN
		(*	set temp directory for buffer file	*)
		SetBBFpu;
		NEW(string, len + 1);
		i := 0; WHILE i < len DO string[i] := LONG(path[i]); INC(i) END; 
		string[len] := 0X;
		BugsFiles.SetTempDir(string);
		SetRFpu;
	END SetTempDir;
	
	PROCEDURE[ccall] UseBufferFile*;
	BEGIN
		BugsFiles.SetDest(BugsFiles.file)
	END UseBufferFile;

	PROCEDURE[ccall] UseConsole*;
	BEGIN
		BugsFiles.SetDest(BugsFiles.log)
	END UseConsole;
	
	PROCEDURE Maintainer;
	BEGIN
		version := 310;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	VAR
		ok: BOOLEAN;
		item: Meta.Item;
	BEGIN
		SetBBFpu;
		Maintainer;
		(*	Initialize subsystems	*)
		Meta.LookupPath("Startup.Setup", item);
		IF item.obj = Meta.procObj THEN
			item.Call(ok)
		END;
		BugsFiles.SetDest(BugsFiles.file);
		SetRFpu
	END Init;

BEGIN
	Init
END BugsC.



