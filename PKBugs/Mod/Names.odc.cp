(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	  *)

MODULE PKBugsNames;

		(* PKBugs Version 1.1 *)

	IMPORT
		PKBugsScanners,
		Strings;

	CONST
		nNames = 24;
		dv* = 0; id* = 1; mdv* = 2; time* = 3; evid* = 4; amt* = 5; rate* = 6; ss* = 7;
		ii* = 8; date* = 9; dat1* = 10; dat2* = 11; dat3* = 12; addl* = 13; abs* = 14;
		lag* = 15; lower* = 16; upper* = 17; drop* = 18; other* = 19;
		centred = 20; cat = 21;

	TYPE
		ItemList* = POINTER TO RECORD
			name-, alias-: ARRAY 80 OF CHAR;
			type-: INTEGER;
			centred-, cat-: BOOLEAN;
			next-: ItemList
		END;

	VAR
		types: ARRAY nNames OF RECORD name: ARRAY 80 OF CHAR; type: INTEGER END;
		itemList-: ItemList;
		nItems-, nOthers-: INTEGER;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE IsPresent* (type: INTEGER): BOOLEAN;
		VAR cursor: ItemList; found: BOOLEAN;
	BEGIN
		cursor := itemList; found := FALSE;
		WHILE (cursor # NIL) & ~found DO IF (cursor.type = type) THEN 
			found := TRUE END; cursor := cursor.next 
		END;
		RETURN found
	END IsPresent;

	PROCEDURE CheckNames* (OUT res: INTEGER);
		VAR dateCount: INTEGER;
	BEGIN
		res := 0;
		IF (itemList = NIL) THEN res := 5101; RETURN END; 	(* no names scanned *)
		IF ~IsPresent(dv) OR ~IsPresent(id) OR ~IsPresent(time) OR ~IsPresent(amt) THEN
			res := 5102; RETURN	(* id, dv, time, and amt must be present *)
		END;
		IF (IsPresent(ss) OR IsPresent(addl)) & ~IsPresent(ii) THEN
			res := 5103; RETURN	(* ii must be present if ss or addl present *)
		END;
		dateCount := 0;
		IF IsPresent(date) THEN INC(dateCount) END; IF IsPresent(dat1) THEN INC(dateCount) END;
		IF IsPresent(dat2) THEN INC(dateCount) END; IF IsPresent(dat3) THEN INC(dateCount) END;
		IF (dateCount > 1) THEN res := 5104; RETURN END	(* only one date item permitted *)
	END CheckNames;

	PROCEDURE ItemType (string: ARRAY OF CHAR): INTEGER;
		VAR s: ARRAY 80 OF CHAR; i, type: INTEGER;
	BEGIN
		Strings.ToLower(string, s); type := other;
		i := 0; WHILE (i < nNames) DO IF s = types[i].name THEN type := types[i].type END; INC(i) END;
		RETURN type
	END ItemType;

	PROCEDURE AddToList (name, alias: ARRAY OF CHAR; type: INTEGER; OUT res: INTEGER);
		VAR n, a: ARRAY 80 OF CHAR; cursor, item: ItemList;
	BEGIN
		res := 0;
		Strings.ToLower(name, n); Strings.ToLower(alias, a);
		cursor := itemList;
		WHILE cursor # NIL DO
			IF n # "" THEN
				IF (n = cursor.name) OR (n = cursor.alias) THEN HALT(0); res := 5003; RETURN END	
				(* name already present *)
			END;
			IF a # "" THEN
				IF (a = cursor.name) OR (a = cursor.alias) THEN res := 5004; RETURN END	
				(* alias already present *)
			END;
			cursor := cursor.next
		END;
		INC(nItems); IF (type = other) OR (type = centred) OR (type = cat) THEN INC(nOthers) END;
		NEW(item); item.name := n$; item.alias := a$; item.next := NIL;
		IF (type = centred) THEN item.type := other; item.centred := TRUE; item.cat := FALSE
		ELSIF (type = cat) THEN item.type := other; item.centred := FALSE; item.cat := TRUE
		ELSE item.type := type; item.centred := FALSE; item.cat := FALSE
		END;
		cursor := itemList;
		IF cursor = NIL THEN itemList := item
		ELSE
			WHILE cursor.next # NIL DO cursor := cursor.next END;
			cursor.next := item
		END
	END AddToList;

	PROCEDURE ReadNames* (VAR s: PKBugsScanners.Scanner; OUT pos, res: INTEGER);
		VAR
			string: ARRAY 80 OF CHAR;
			type0, type1, pos0: INTEGER;

		PROCEDURE IsDrop (string: ARRAY OF CHAR): BOOLEAN;
			VAR s: ARRAY 80 OF CHAR;
		BEGIN
			Strings.ToLower(string, s); RETURN (s = "drop") OR (s = "skip")
		END IsDrop;

		PROCEDURE IsCentred (string: ARRAY OF CHAR): BOOLEAN;
			VAR s: ARRAY 80 OF CHAR;
		BEGIN
			Strings.ToLower(string, s); RETURN s = "centred"
		END IsCentred;

		PROCEDURE IsCat (string: ARRAY OF CHAR): BOOLEAN;
			VAR s: ARRAY 80 OF CHAR;
		BEGIN
			Strings.ToLower(string, s); RETURN s = "cat"
		END IsCat;

	BEGIN
		pos := 0; res := 0;
		s.SetPos(pos);
		WHILE ~s.eot DO
			pos0 := s.Pos(); pos := s.Pos(); s.Scan;
			IF (s.type = PKBugsScanners.string) THEN
				string := s.string$;
				IF IsDrop(string) THEN
					AddToList("", "", drop, res)
				ELSE
					type0 := ItemType(string);
					IF s.eot THEN AddToList(string, "", type0, res); RETURN END;
					pos := s.Pos(); s.Scan;
					IF (type0 = other) THEN
						CASE s.type OF
						|PKBugsScanners.string:
							s.SetPos(pos); AddToList(string, "", type0, res); 
							IF res # 0 THEN pos := pos0; RETURN END
						|PKBugsScanners.char:
							CASE s.char OF
							|0X: AddToList(string, "", type0, res); IF res # 0 THEN pos := pos0 END; RETURN
							|"=": pos := s.Pos();
								IF s.eot THEN
									res := 5002; RETURN	(* expected standard name, drop, skip, centred, or cat *)
								END;
								s.Scan;
								IF (s.type = PKBugsScanners.string) THEN
									IF IsDrop(s.string) THEN
										AddToList(string, "", drop, res); IF res # 0 THEN pos := pos0; RETURN END
									ELSIF IsCentred(s.string) THEN
										AddToList(string, "", centred, res); IF res # 0 THEN pos := pos0; RETURN END
									ELSIF IsCat(s.string) THEN
										AddToList(string, "", cat, res); IF res # 0 THEN pos := pos0; RETURN END
									ELSE
										type1 := ItemType(s.string);
										IF (type1 = other) THEN
											res := 5002; RETURN	
											(* expected standard name, drop, skip, centred, or cat *)
										END;
										AddToList(s.string, string, type1, res);
										IF (res = 5003) THEN RETURN
										ELSIF (res = 5004) THEN pos := pos0; RETURN
										END
									END
								ELSE res := 5002; RETURN	(* expected standard name, drop, skip, centred, or cat *)
								END
							ELSE res := 5006; RETURN	(* unexpected token scanned *)
							END
						ELSE res := 5001; RETURN	(* unexpected or invalid token scanned *)
						END
					ELSE
						CASE s.type OF
						|PKBugsScanners.string:
							s.SetPos(pos); AddToList(string, "", type0, res); 
							IF res # 0 THEN pos := pos0; RETURN END
						|PKBugsScanners.char:
							CASE s.char OF
							|0X: AddToList(string, "", type0, res); IF res # 0 THEN pos := pos0 END; RETURN
							|"=": pos := s.Pos();
								IF s.eot THEN res := 5005; RETURN END; 	(* expected alias, drop, or skip *)
								s.Scan;
								IF (s.type = PKBugsScanners.string) THEN
									IF IsCentred(s.string) THEN res := 5009; RETURN	
									(* only covariates may be centred *)
									ELSIF IsCat(s.string) THEN res := 5010; RETURN	
									(* cat only appropriate for covariates *)
									ELSIF IsDrop(s.string) THEN
										CASE type0 OF
										|dv, id, time, amt, date, dat1, dat2, dat3:
											AddToList(string, "", type0, res); IF res # 0 THEN pos := pos0; RETURN END
										ELSE
											AddToList(string, "", drop, res); IF res # 0 THEN pos := pos0; RETURN END
										END
									ELSE
										type1 := ItemType(s.string);
										IF (type1 # other) THEN res := 5005; RETURN END; 	
										(* expected alias, drop, or skip *)
										AddToList(string, s.string, type0, res);
										IF (res = 5004) THEN RETURN
										ELSIF (res = 5003) THEN pos := pos0; RETURN
										END
									END
								ELSE res := 5005; RETURN	(* expected alias, drop, or skip *)
								END
							ELSE res := 5006; RETURN	(* unexpected token scanned *)
							END
						ELSE res := 5001; RETURN	(* unexpected or invalid token scanned *)
						END
					END
				END
			ELSIF (s.type = PKBugsScanners.char) & (s.char = 0X) THEN RETURN
			ELSE res := 5001; RETURN	(* unexpected or invalid token scanned *)
			END
		END
	END ReadNames;

	PROCEDURE Reset*;
	BEGIN
		itemList := NIL; nItems := 0; nOthers := 0
	END Reset;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "D.Lunn"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer; 
		Reset;
		types[0].name := "dv"; types[0].type := dv;
		types[1].name := "id"; types[1].type := id;
		types[2].name := "mdv"; types[2].type := mdv;
		types[3].name := "time"; types[3].type := time;
		types[4].name := "evid"; types[4].type := evid;
		types[5].name := "amt"; types[5].type := amt;
		types[6].name := "rate"; types[6].type := rate;
		types[7].name := "ss"; types[7].type := ss;
		types[8].name := "ii"; types[8].type := ii;
		types[9].name := "date"; types[9].type := date;
		types[10].name := "dat1"; types[10].type := dat1;
		types[11].name := "dat2"; types[11].type := dat2;
		types[12].name := "dat3"; types[12].type := dat3;
		types[13].name := "addl"; types[13].type := addl;
		types[14].name := "abs"; types[14].type := abs;
		types[15].name := "lag"; types[15].type := lag;
		types[16].name := "lower"; types[16].type := lower;
		types[17].name := "upper"; types[17].type := upper;
		types[18].name := "l2"; types[18].type := drop;
		types[19].name := "cmt"; types[19].type := drop;
		types[20].name := "pcmt"; types[20].type := drop;
		types[21].name := "call"; types[21].type := drop;
		types[22].name := "l1"; types[22].type := drop;
		types[23].name := "cont"; types[23].type := drop
	END Init;

BEGIN
	Init
END PKBugsNames.
