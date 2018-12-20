(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsInterpreter;

	

	IMPORT
		Dialog, Meta, Strings,
		BugsMappers, BugsMsg;

	CONST
		notModule* = 1;
		invalidArgument* = 2;
		invalidObjectType* = 3;
		failedGuard* = 4;
		
	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	(*	
		strips semicolons ";", brackets "(" and ")" out of string. IF  period "." not within quotes or
		bracket pair then stripped
	*)
	PROCEDURE Strip (VAR string: ARRAY OF CHAR);
		VAR
			paren, inString: BOOLEAN;
			quoteChar: CHAR;
			i: INTEGER;
	BEGIN
		i := 0;
		paren := FALSE;
		inString := FALSE;
		WHILE string[i] # 0X DO
			IF ~inString THEN
				IF (string[i] = '"') OR (string[i] = "'") THEN
					quoteChar := string[i];
					inString := TRUE
				END;
				IF string[i] = "(" THEN
					paren := TRUE;
					string[i] := " "
				ELSIF string[i] = ")" THEN
					paren := FALSE;
					string[i] := " "
				ELSIF string[i] = ";" THEN
					string[i] := " "
				ELSIF (string[i] = ":") & (string[i + 1] = "=") THEN
					string[i] := " ";
					string[i + 1] := " "
				ELSIF (string[i] = ".") & ~paren & ~inString THEN
					string[i] := " "
				END
			ELSE
				inString := string[i] # quoteChar
			END;
			INC(i)
		END
	END Strip;

	PROCEDURE  CmdInterpreter* (command: ARRAY OF CHAR; OUT res: INTEGER);
		VAR
			ok: BOOLEAN;
			par: Dialog.Par;
			string: ARRAY 1024 OF CHAR;
			s: BugsMappers.Scanner;
			cmd: RECORD(Meta.Value) Do: PROCEDURE END;
			stringCmd: RECORD(Meta.Value) Do: PROCEDURE (arg: ARRAY OF CHAR) END;
			realCmd: RECORD(Meta.Value) Do: PROCEDURE (arg: REAL) END;
			intCmd: RECORD(Meta.Value) Do: PROCEDURE (arg: INTEGER) END;
			boolCmd: RECORD(Meta.Value) Do: PROCEDURE (arg: BOOLEAN) END;
			guard: RECORD(Meta.Value) Do: PROCEDURE (VAR arg: Dialog.Par) END;
			item, item0, item1: Meta.Item;
			msg: Dialog.String;
	BEGIN
		res := 0;
		Strip(command);
		string := command + " $";
		s.ConnectToString(string);
		s.SetPos(0);
		s.Scan;
		WHILE (s.type = BugsMappers.string) & (res = 0) DO
			Meta.Lookup(s.string, item);
			IF item.obj = Meta.modObj THEN
				s.Scan;
				item.Lookup(s.string, item0);
				IF item0.obj = Meta.procObj THEN
					item0.GetVal(cmd, ok);
					IF ok THEN
						cmd.Do 
					ELSE
						item0.GetVal(stringCmd, ok);
						IF ok THEN
							s.Scan;
							IF s.type # BugsMappers.string THEN
								res := invalidArgument; RETURN
							END;
							stringCmd.Do(s.string)
						ELSE
							item0.GetVal(intCmd, ok);
							IF ok THEN
								s.Scan;
								IF s.type # BugsMappers.int THEN 
									res := invalidArgument; RETURN
								END;
								intCmd.Do(s.int)
							ELSE
								item0.GetVal(realCmd, ok);
								IF ok THEN
									s.Scan;
									IF s.type # BugsMappers.real THEN
										res := invalidArgument; RETURN
									END;
									realCmd.Do(s.real)
								ELSE
									item0.GetVal(boolCmd, ok);
									IF ok THEN
										s.Scan;
										IF s.type # BugsMappers.string THEN
											res := invalidArgument; RETURN
										END;
										Strings.ToUpper(s.string, string);
										IF (string = "T") OR (string = "TRUE") THEN
											boolCmd.Do(TRUE)
										ELSIF (string = "F") OR (string = "FALSE") THEN
											boolCmd.Do(FALSE)
										ELSE
											res := invalidArgument; RETURN
										END
									ELSE
										item0.GetVal(guard, ok); 
										IF ok THEN
											par.disabled := FALSE;
											guard.Do(par);
											IF par.disabled THEN 
												BugsMsg.Lookup(par.label, msg);
												BugsMsg.StoreError(msg);
												res := failedGuard; 
												RETURN 
											END
										ELSE
											res := invalidArgument; RETURN
										END
									END
								END
							END
						END
					END
				ELSIF item0.obj = Meta.varObj THEN
					IF item0.typ = Meta.ptrTyp THEN
						item0.Deref(item0)
					END;
					IF item0.typ = Meta.recTyp THEN
						s.Scan;
						item0.Lookup(s.string, item1);
						s.Scan;
						IF item1.typ = Meta.intTyp THEN
							IF s.type # BugsMappers.int THEN
								res := invalidArgument; RETURN
							END;
							item1.PutIntVal(s.int)
						ELSIF item1.typ = Meta.realTyp THEN
							IF s.type = BugsMappers.real THEN
								item1.PutRealVal(s.real)
							ELSIF s.type = BugsMappers.int THEN
								item1.PutRealVal(s.int)
							ELSE
								res := invalidArgument; RETURN
							END
						ELSIF item1.typ = Meta.boolTyp THEN
							s.Scan;
							IF s.type = BugsMappers.int THEN
								item1.PutBoolVal(s.int # 0)
							ELSIF s.type = BugsMappers.string THEN
								Strings.ToUpper(s.string, string);
								IF (string = "TRUE") OR (string = "T") THEN
									item1.PutBoolVal(TRUE)
								ELSIF (string = "FALSE") OR (string = "F") THEN
									item1.PutBoolVal(FALSE)
								ELSE
									res := invalidArgument; RETURN
								END
							END
						END
					ELSIF item0.typ = Meta.intTyp THEN
						s.Scan;
						IF s.type # BugsMappers.int THEN
							res := invalidArgument; RETURN
						END;
						item0.PutIntVal(s.int)
					ELSIF item0.typ = Meta.realTyp THEN
						s.Scan;
						IF s.type = BugsMappers.real THEN
							item0.PutRealVal(s.real)
						ELSIF s.type = BugsMappers.int THEN
							item0.PutRealVal(s.int)
						ELSE
							res := invalidArgument; RETURN
						END
					ELSIF item0.typ = Meta.boolTyp THEN
						s.Scan;
						IF s.type = BugsMappers.int THEN
							item0.PutBoolVal(s.int # 0)
						ELSIF s.type = BugsMappers.string THEN
							Strings.ToUpper(s.string, string);
							IF (string = "TRUE") OR (string = "T") THEN
								item0.PutBoolVal(TRUE)
							ELSIF (string = "FALSE") OR (string = "F") THEN
								item0.PutBoolVal(FALSE)
							ELSE
								res := invalidArgument; RETURN
							END
						ELSE
							res := invalidArgument; RETURN
						END
					END
				ELSE
					res := invalidObjectType; RETURN
				END
			ELSE
				res := notModule; RETURN
			END;
			s.Scan
		END
	END CmdInterpreter;

	PROCEDURE CharArray* (procedure: ARRAY OF CHAR; VAR x: ARRAY OF CHAR;
	OUT res: INTEGER);
		VAR
			ok: BOOLEAN;
			s: BugsMappers.Scanner;
			item, item0: Meta.Item;
			arrayVALProc: RECORD(Meta.Value) Do: PROCEDURE (x: ARRAY OF CHAR) END;
			arrayVARProc: RECORD(Meta.Value) Do: PROCEDURE (VAR x: ARRAY OF CHAR) END;
			arrayINProc: RECORD(Meta.Value) Do: PROCEDURE (IN x: ARRAY OF CHAR) END;
			arrayOUTProc: RECORD(Meta.Value) Do: PROCEDURE (OUT x: ARRAY OF CHAR) END;
	BEGIN
		res := 0;
		Strip(procedure);
		s.ConnectToString(procedure);
		s.SetPos(0);
		s.Scan;
		Meta.Lookup(s.string, item);
		IF item.obj = Meta.modObj THEN
			s.Scan;
			item.Lookup(s.string, item0);
			IF item0.obj = Meta.procObj THEN
				item0.GetVal(arrayVALProc, ok);
				IF ok THEN
					arrayVALProc.Do(x)
				ELSE
					item0.GetVal(arrayVARProc, ok);
					IF ok THEN
						arrayVARProc.Do(x)
					ELSE
						item0.GetVal(arrayINProc, ok);
						IF ok THEN
							arrayINProc.Do(x)
						ELSE
							item0.GetVal(arrayOUTProc, ok);
							IF ok THEN
								arrayOUTProc.Do(x)
							ELSE
								res := invalidObjectType;
								RETURN
							END
						END
					END
				END
			ELSE
				res := invalidObjectType;
				RETURN
			END
		ELSE
			res := notModule;
			RETURN
		END
	END CharArray;

	PROCEDURE Integer* (procedure: ARRAY OF CHAR; OUT x, res: INTEGER);
		VAR
			ok: BOOLEAN;
			s: BugsMappers.Scanner;
			item, item0, item1: Meta.Item;
			intProc: RECORD(Meta.Value) Do: PROCEDURE (): INTEGER END;
			boolProc: RECORD(Meta.Value) Do: PROCEDURE (): BOOLEAN END;
	BEGIN
		res := 0;
		x := MIN(INTEGER);
		Strip(procedure);
		s.ConnectToString(procedure);
		s.SetPos(0);
		s.Scan;
		Meta.Lookup(s.string, item);
		IF item.obj = Meta.modObj THEN
			s.Scan;
			item.Lookup(s.string, item0);
			IF item0.obj = Meta.procObj THEN
				item0.GetVal(intProc, ok);
				IF ok THEN
					x := intProc.Do()
				ELSE
					item0.GetVal(boolProc, ok);
					IF ok THEN
						IF boolProc.Do() THEN x := 1 ELSE x := 0 END
					ELSE
						res := invalidObjectType;
						RETURN
					END
				END
			ELSIF item0.obj = Meta.varObj THEN
				IF item0.typ = Meta.ptrTyp THEN
					item0.Deref(item0)
				END;
				IF item0.typ = Meta.recTyp THEN
					s.Scan;
					item0.Lookup(s.string, item1);
					IF item1.typ = Meta.intTyp THEN
						x := item1.IntVal()
					ELSIF item1.typ = Meta.boolTyp THEN
						IF item1.BoolVal() THEN x := 1 ELSE x := 0 END
					END
				ELSIF item0.typ = Meta.intTyp THEN
					x := item0.IntVal()
				ELSIF item0.typ = Meta.boolTyp THEN
					IF item0.BoolVal() THEN
						x := 1
					ELSE
						x := 0
					END
				ELSE
					res := invalidObjectType;
					RETURN
				END
			ELSE
				res := invalidObjectType;
				RETURN
			END
		ELSE
			res := notModule;
			RETURN
		END
	END Integer;

	PROCEDURE IntegerArray* (procedure: ARRAY OF CHAR; VAR x: ARRAY OF INTEGER;
	OUT res: INTEGER);
		VAR
			ok: BOOLEAN;
			s: BugsMappers.Scanner;
			item, item0: Meta.Item;
			arrayVARProc: RECORD(Meta.Value) Do: PROCEDURE (VAR x: ARRAY OF INTEGER) END;
			arrayINProc: RECORD(Meta.Value) Do: PROCEDURE (IN x: ARRAY OF INTEGER) END;
			arrayOUTProc: RECORD(Meta.Value) Do: PROCEDURE (OUT x: ARRAY OF INTEGER) END;
	BEGIN
		res := 0;
		Strip(procedure);
		s.ConnectToString(procedure);
		s.SetPos(0);
		s.Scan;
		Meta.Lookup(s.string, item);
		IF item.obj = Meta.modObj THEN
			s.Scan;
			item.Lookup(s.string, item0);
			IF item0.obj = Meta.procObj THEN
				item0.GetVal(arrayVARProc, ok);
				IF ok THEN
					arrayVARProc.Do(x)
				ELSE
					item0.GetVal(arrayINProc, ok);
					IF ok THEN
						arrayINProc.Do(x)
					ELSE
						item0.GetVal(arrayOUTProc, ok);
						IF ok THEN
							arrayOUTProc.Do(x)
						ELSE
							res := invalidObjectType;
							RETURN
						END
					END
				END
			END
		ELSE
			res := notModule;
			RETURN
		END
	END IntegerArray;

	PROCEDURE Real* (procedure: ARRAY OF CHAR; x: REAL; OUT y: REAL;
	OUT res: INTEGER);
		VAR
			ok: BOOLEAN;
			s: BugsMappers.Scanner;
			item, item0: Meta.Item;
			realProc: RECORD(Meta.Value) Do: PROCEDURE (x: REAL): REAL END;
	BEGIN
		res := 0;
		y :=  - INF;
		Strip(procedure);
		s.ConnectToString(procedure);
		s.SetPos(0);
		s.Scan;
		Meta.Lookup(s.string, item);
		IF item.obj = Meta.modObj THEN
			s.Scan;
			item.Lookup(s.string, item0);
			IF item0.obj = Meta.procObj THEN
				item0.GetVal(realProc, ok);
				IF ok THEN
					y := realProc.Do(x)
				ELSE
					res := invalidObjectType;
					RETURN
				END
			END
		ELSE
			res := notModule;
			RETURN
		END
	END Real;

	PROCEDURE RealArray* (procedure: ARRAY OF CHAR; VAR x: ARRAY OF REAL;
	OUT res: INTEGER);
		VAR
			ok: BOOLEAN;
			s: BugsMappers.Scanner;
			item, item0: Meta.Item;
			arrayVARProc: RECORD(Meta.Value) Do: PROCEDURE (VAR x: ARRAY OF REAL) END;
			arrayINProc: RECORD(Meta.Value) Do: PROCEDURE (IN x: ARRAY OF REAL) END;
			arrayOUTProc: RECORD(Meta.Value) Do: PROCEDURE (OUT x: ARRAY OF REAL) END;
	BEGIN
		res := 0;
		Strip(procedure);
		s.ConnectToString(procedure);
		s.SetPos(0);
		s.Scan;
		Meta.Lookup(s.string, item);
		IF item.obj = Meta.modObj THEN
			s.Scan;
			item.Lookup(s.string, item0);
			IF item0.obj = Meta.procObj THEN
				item0.GetVal(arrayVARProc, ok);
				IF ok THEN
					arrayVARProc.Do(x)
				ELSE
					item0.GetVal(arrayINProc, ok);
					IF ok THEN
						arrayINProc.Do(x)
					ELSE
						item0.GetVal(arrayOUTProc, ok);
						IF ok THEN
							arrayOUTProc.Do(x)
						ELSE
							res := invalidObjectType;
							RETURN
						END
					END
				END
			END
		ELSE
			res := notModule;
			RETURN
		END
	END RealArray;

	PROCEDURE Guard* (procedure: ARRAY OF CHAR; OUT ok: BOOLEAN; OUT res: INTEGER);
		VAR
			s: BugsMappers.Scanner;
			item, item0: Meta.Item;
			guardProc: RECORD(Meta.Value) Do: PROCEDURE (OUT x: BOOLEAN) END;
	BEGIN
		res := 0;
		Strip(procedure);
		s.ConnectToString(procedure);
		s.SetPos(0);
		s.Scan;
		Meta.Lookup(s.string, item);
		IF item.obj = Meta.modObj THEN
			s.Scan;
			item.Lookup(s.string, item0);
			IF item0.obj = Meta.procObj THEN
				item0.GetVal(guardProc, ok);
				IF ok THEN
					guardProc.Do(ok)
				ELSE
					res := invalidObjectType;
					RETURN
				END
			END
		ELSE
			res := notModule;
			RETURN
		END
	END Guard;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END BugsInterpreter.



