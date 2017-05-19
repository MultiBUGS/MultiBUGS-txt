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
		Meta, Strings,
		BugsFiles, BugsInterpreter, BugsMsg, BugsScripting;

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

	PROCEDURE[ccall] infoData* ();
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		command := "infoData()";
		BugsScripting.Command(command, res)
	END infoData;

	PROCEDURE[ccall] infoError* (VAR error: POINTER TO ARRAY[untagged] OF SHORTCHAR);
	BEGIN
		error^ := SHORT(BugsMsg.message$)
	END infoError;

	PROCEDURE[ccall] infoMetrics* ();
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		command := "infoMetrics()";
		BugsScripting.Command(command, res)
	END infoMetrics;

	PROCEDURE[ccall] infoModel* ();
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		command := "infoModel()";
		BugsScripting.Command(command, res)
	END infoModel;

	PROCEDURE[ccall] infoNodeMethods* (VAR varName: POINTER TO ARRAY[untagged] OF SHORTCHAR);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		command := "infoNodeMethods('" + varName + "')";
		BugsScripting.Command(command, res)
	END infoNodeMethods;

	PROCEDURE[ccall] infoNodeTypes* (VAR varName: POINTER TO ARRAY[untagged] OF SHORTCHAR);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		command := "infoNodeTypes('" + varName + "')";
		BugsScripting.Command(command, res)
	END infoNodeTypes;

	PROCEDURE[ccall] infoNodeValues* (VAR varName: POINTER TO ARRAY[untagged] OF SHORTCHAR);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		command := "infoNodeValues('" + varName + "')";
		BugsScripting.Command(command, res)
	END infoNodeValues;

	PROCEDURE[ccall] infoUnitializedUpdaters* ();
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		command := "infoUnitializedUpdaters()";
		BugsScripting.Command(command, res)
	END infoUnitializedUpdaters ;

	PROCEDURE[ccall] infoUpdatersByDepth* ();
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		command := "infoUpdatersByDepth()";
		BugsScripting.Command(command, res)
	END infoUpdatersByDepth ;

	PROCEDURE[ccall] infoUpdatersByName* ();
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		command := "infoUpdatersByDepth()";
		BugsScripting.Command(command, res)
	END infoUpdatersByName ;

	PROCEDURE[ccall] modelCheck* (VAR fileName: POINTER TO ARRAY[untagged] OF SHORTCHAR);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		command := "modelCheck('" + fileName + "')";
		BugsScripting.Command(command, res)
	END modelCheck;

	PROCEDURE[ccall] modelCompile* (VAR numChains: INTEGER);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		Strings.IntToString(numChains, command);
		command := "modelCompile('" + command + "')";
		BugsScripting.Command(command, res)
	END modelCompile;

	PROCEDURE[ccall] modelGenInits* ();
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		command := "modelGenInits()";
		BugsScripting.Command(command, res)
	END modelGenInits;
	
	PROCEDURE[ccall] modelLoadData* (VAR fileName: POINTER TO ARRAY[untagged] OF SHORTCHAR);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		command := "modelLoadData('" + fileName + "')";
		BugsScripting.Command(command, res)
	END modelLoadData;

	PROCEDURE[ccall] modelLoadInits* (VAR fileName: POINTER TO ARRAY[untagged] OF SHORTCHAR;
	VAR chain: INTEGER);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		Strings.IntToString(chain, command);
		command := "modelLoadInits('" + fileName +  "'," + command + ")";
		BugsScripting.Command(command, res)
	END modelLoadInits;

	PROCEDURE[ccall] modelUpdate* (VAR numUpdates: INTEGER);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		Strings.IntToString(numUpdates, command);
		command := "modelUpdate('" + command + "')";
		BugsScripting.Command(command, res)
	END modelUpdate;

	PROCEDURE[ccall] modelsClear* (VAR varName: POINTER TO ARRAY[untagged] OF SHORTCHAR);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		command := "modelsClear('" + varName + ")";
		BugsScripting.Command(command, res)
	END modelsClear;

	PROCEDURE[ccall] modelsComp* (VAR varName: POINTER TO ARRAY[untagged] OF SHORTCHAR);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		command := "modelsComp('" + varName + ")";
		BugsScripting.Command(command, res)
	END modelsComp;

	PROCEDURE[ccall] modelsProbs* (VAR varName: POINTER TO ARRAY[untagged] OF SHORTCHAR);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		command := "modelsProbs('" + varName + ")";
		BugsScripting.Command(command, res)
	END modelsProbs;

	PROCEDURE[ccall] modelsSet* (VAR varName: POINTER TO ARRAY[untagged] OF SHORTCHAR);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		command := "ranksSet('" + varName + ")";
		BugsScripting.Command(command, res)
	END modelsSet;

	PROCEDURE[ccall] ranksClear* (VAR varName: POINTER TO ARRAY[untagged] OF SHORTCHAR);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		command := "ranksClear('" + varName + ")";
		BugsScripting.Command(command, res)
	END ranksClear;

	PROCEDURE[ccall] ranksSet* (VAR varName: POINTER TO ARRAY[untagged] OF SHORTCHAR);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		command := "ranksSet('" + varName + ")";
		BugsScripting.Command(command, res)
	END ranksSet;

	PROCEDURE[ccall] ranksStats* (VAR varName: POINTER TO ARRAY[untagged] OF SHORTCHAR);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		command := "ranksStats('" + varName + ")";
		BugsScripting.Command(command, res)
	END ranksStats;

	PROCEDURE[ccall] samplesBeg* (VAR beg: INTEGER);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		Strings.IntToString(beg, command);
		command := "samplesBeg('" + command + ")";
		BugsScripting.Command(command, res)
	END samplesBeg;

	PROCEDURE[ccall] samplesClear* (VAR varName: POINTER TO ARRAY[untagged] OF SHORTCHAR);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		command := "samplesClear('" + varName + ")";
		BugsScripting.Command(command, res)
	END samplesClear;

	PROCEDURE[ccall] samplesEnd* (VAR end: INTEGER);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		Strings.IntToString(end, command);
		command := "samplesEnd('" + command + ")";
		BugsScripting.Command(command, res)
	END samplesEnd;
	
	PROCEDURE[ccall] samplesFirstChain* (VAR chain: INTEGER);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		Strings.IntToString(chain, command);
		command := "samplesFirstChain('" + command + ")";
		BugsScripting.Command(command, res)
	END samplesFirstChain;
	
	PROCEDURE[ccall] samplesLastChain* (VAR chain: INTEGER);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		Strings.IntToString(chain, command);
		command := "samplesLastChain('" + command + ")";
		BugsScripting.Command(command, res)
	END samplesLastChain;
	
	PROCEDURE[ccall] samplesOptionExcl* (VAR opt: INTEGER);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		Strings.IntToString(opt, command);
		command := "samplesOptionExcl('" + command + ")";
		BugsScripting.Command(command, res)
	END samplesOptionExcl;
	
	PROCEDURE[ccall] samplesOptionIncl* (VAR opt: INTEGER);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		Strings.IntToString(opt, command);
		command := "samplesOptionIncl('" + command + ")";
		BugsScripting.Command(command, res)
	END samplesOptionIncl;

	PROCEDURE[ccall] samplesSet* (VAR varName: POINTER TO ARRAY[untagged] OF SHORTCHAR);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		command := "samplesSet('" + varName + ")";
		BugsScripting.Command(command, res)
	END samplesSet;

	PROCEDURE[ccall] samplesStats* (VAR varName: POINTER TO ARRAY[untagged] OF SHORTCHAR);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		command := "samplesStats('" + varName + ")";
		BugsScripting.Command(command, res)
	END samplesStats;
	
	PROCEDURE[ccall] samplesThin* (VAR thin: INTEGER);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		Strings.IntToString(thin, command);
		command := "samplesThin('" + command + ")";
		BugsScripting.Command(command, res)
	END samplesThin;

	PROCEDURE[ccall] summaryClear* (VAR varName: POINTER TO ARRAY[untagged] OF SHORTCHAR);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		command := "summaryClear('" + varName + ")";
		BugsScripting.Command(command, res)
	END summaryClear;

	PROCEDURE[ccall] summarySet* (VAR varName: POINTER TO ARRAY[untagged] OF SHORTCHAR);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		command := "summarySet('" + varName + ")";
		BugsScripting.Command(command, res)
	END summarySet;

	PROCEDURE[ccall] summaryStats* (VAR varName: POINTER TO ARRAY[untagged] OF SHORTCHAR);
		VAR
			command: ARRAY 1024 OF CHAR;
			res: INTEGER;
	BEGIN
		command := "summaryStats('" + varName + ")";
		BugsScripting.Command(command, res)
	END summaryStats;
	
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



