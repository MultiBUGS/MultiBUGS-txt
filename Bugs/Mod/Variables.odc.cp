(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	   *)

MODULE BugsVariables;


	

	IMPORT
		Strings,
		BugsIndex, BugsMappers, BugsMsg, BugsNames;

	TYPE
		Loop = POINTER TO RECORD
			name: ARRAY 60 OF CHAR;
			next: Loop
		END;

	VAR
		loops: Loop;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Error (errorNum: INTEGER);
		VAR
			numToString: ARRAY 8 OF CHAR;
			errorMes: ARRAY 1024 OF CHAR;
	BEGIN
		Strings.IntToString(errorNum, numToString);
		BugsMsg.Lookup("BugsVariables" + numToString, errorMes);
		BugsMsg.Store(errorMes)
	END Error;

	PROCEDURE IsLoopIndex (IN name: ARRAY OF CHAR): BOOLEAN;
		VAR
			cursor: Loop;
	BEGIN
		cursor := loops;
		WHILE (cursor # NIL) & (cursor.name # name) DO
			cursor := cursor.next
		END;
		RETURN cursor # NIL
	END IsLoopIndex;

	PROCEDURE StoreLoop* (IN name: ARRAY OF CHAR);
		VAR
			cursor: Loop;
	BEGIN
		IF ~IsLoopIndex(name) THEN
			NEW(cursor);
			cursor.name := name$;
			cursor.next := loops;
			loops := cursor
		END
	END StoreLoop;

	PROCEDURE StoreLoops* (VAR s: BugsMappers.Scanner; OUT ok: BOOLEAN);
		VAR
			depth: INTEGER;
	BEGIN
		ok := TRUE;
		depth := 1;
		WHILE (depth # 0) & ok & ~s.eot DO
			s.Scan;
			IF ((s.type = BugsMappers.string) OR (s.type = BugsMappers.function)) & (s.string = "for") THEN
				INC(depth);
				s.Scan;
				s.Scan;
				IF s.type = BugsMappers.string THEN
					StoreLoop(s.string)
				ELSE
					ok := FALSE;
					Error(1) (*	loop index must be a name	*)
				END
			ELSIF (s.type = BugsMappers.char) & (s.char = "}") THEN
				DEC(depth)
			END
		END
	END StoreLoops;

	PROCEDURE CountSlots (VAR s: BugsMappers.Scanner; OUT indices: INTEGER; OUT ok: BOOLEAN);
		VAR
			brakets: INTEGER;
	BEGIN
		ok := TRUE;
		brakets := 1;
		indices := 1;
		WHILE (brakets # 0) & ~s.eot DO
			s.Scan;
			IF s.type = BugsMappers.char THEN
				IF s.char = "[" THEN
					INC(brakets)
				ELSIF s.char = "]" THEN
					DEC(brakets)
				ELSIF (s.char = ",") & (brakets = 1) THEN
					INC(indices)
				END
			END
		END;
		IF brakets # 0 THEN
			ok := FALSE;
			Error(2); (*	missing closing square braket	*)
		END
	END CountSlots;

	PROCEDURE StoreNode (VAR s: BugsMappers.Scanner; OUT ok: BOOLEAN);
		VAR
			slots: INTEGER;
			string: ARRAY 80 OF CHAR;
			name: BugsNames.Name;
	BEGIN
		ASSERT(s.type = BugsMappers.string, 31);
		ok := TRUE;
		string := s.string$;
		name := BugsIndex.Find(string);
		IF name = NIL THEN
			s.Scan;
			IF (s.type = BugsMappers.char) & (s.char = "[") THEN
				CountSlots(s, slots, ok);
				IF ~ok THEN RETURN END
			ELSE
				slots := 0
			END;
			name := BugsNames.New(string, slots);
			BugsIndex.Store(name)
		END
	END StoreNode;

	PROCEDURE StoreNames* (VAR s: BugsMappers.Scanner; OUT ok: BOOLEAN);
		VAR
			depth, pos: INTEGER;
	BEGIN
		ok := TRUE;
		depth := 1;
		WHILE (depth # 0) & ~s.eot DO
			s.Scan;
			IF (s.type = BugsMappers.string) & (s.string # "in") & (s.string # "for") THEN
				IF ~IsLoopIndex(s.string) THEN
					pos := s.Pos();
					StoreNode(s, ok);
					IF ~ok THEN RETURN END;
					s.SetPos(pos)
				END
			ELSIF s.type = BugsMappers.char THEN
				IF s.char = "{" THEN
					INC(depth)
				ELSIF s.char = "}" THEN
					DEC(depth)
				END
			END
		END
	END StoreNames;

	PROCEDURE StoreName* (IN name: ARRAY OF CHAR);
		VAR
			s: BugsMappers.Scanner;
			ok: BOOLEAN;
	BEGIN
		s.ConnectToString(name);
		s.SetPos(0);
		StoreNames(s, ok)
	END StoreName;
	
	PROCEDURE Clear*;
	BEGIN
		loops := NIL;
		BugsIndex.Clear
	END Clear;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		loops := NIL
	END Init;

BEGIN
	Init
END BugsVariables.
