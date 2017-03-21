(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	  *)

MODULE PKBugsData;

		(* PKBugs Version 1.1 *)

	IMPORT
		PKBugsNames, PKBugsScanners;

	CONST
		obs* = 0; dose* = 1; pred* = 2; reset* = 3; resDose* = 4;
		drop = 5; item = 6; mv = 7; eot = 8;

		tab = 09X; para = 0EX; line = 0DX;

	TYPE
		IntData* = RECORD value-: INTEGER; missing-: BOOLEAN END;
		RealData* = RECORD value-: REAL; missing-: BOOLEAN END;
		Date* = RECORD day-, month-, year-: IntData END;

	VAR
		id-, evid-: POINTER TO ARRAY OF INTEGER;
		time-: POINTER TO ARRAY OF REAL;
		mdv-, abs-, lag-: POINTER TO ARRAY OF BOOLEAN;
		ss-, addl-: POINTER TO ARRAY OF IntData;
		dv-, rate-, amt-, ii-, lower-, upper-: POINTER TO ARRAY OF RealData;
		date-: POINTER TO ARRAY OF Date;
		others-: POINTER TO ARRAY OF ARRAY OF RealData;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Equals (r: REAL; i: INTEGER): BOOLEAN;
		CONST eps = 1.0E-10;
	BEGIN
		RETURN ABS(r - i) < eps
	END Equals;

	PROCEDURE LongToInt (l: LONGINT; OUT i, res: INTEGER);
	BEGIN
		res := 0;
		IF (l < MIN(INTEGER)) OR (l > MAX(INTEGER)) THEN res := 5200; RETURN END; 	
		(* number too large *)
		i := SHORT(l)
	END LongToInt;

	PROCEDURE ScanDrop (VAR s: PKBugsScanners.Scanner; OUT scanned: INTEGER);
		VAR ch: CHAR;
	BEGIN
		ch := s.nextCh; s.FindToken(ch);
		CASE ch OF
		|0X: scanned := eot; RETURN	(* nothing scanned *)
		|",": s.ReadChar(ch); s.FindToken(ch);
			CASE ch OF
			|0X: scanned := mv; RETURN	(* missing value implied *)
			|",": scanned := mv; s.SetPos(s.Pos())	(* missing value scanned *)
			ELSE
				s.ReadChar(ch);
				LOOP
					CASE ch OF
					|" ", ",", tab, line, para: s.SetPos(s.Pos()); EXIT
					|0X: EXIT
					ELSE s.ReadChar(ch)
					END
				END;
				scanned := drop	(* drop scanned *)
			END
		ELSE
			s.ReadChar(ch);
			LOOP
				CASE ch OF
				|" ", ",", tab, line, para: s.SetPos(s.Pos()); EXIT
				|0X: EXIT
				ELSE s.ReadChar(ch)
				END
			END;
			scanned := drop	(* drop scanned *)
		END
	END ScanDrop;

	PROCEDURE ScanItem (VAR s: PKBugsScanners.Scanner; OUT scanned, pos, res: INTEGER);
	BEGIN
		pos := s.Pos(); res := 0; s.Scan;
		CASE s.type OF
		|PKBugsScanners.date, PKBugsScanners.clock, PKBugsScanners.real, PKBugsScanners.int:
			scanned := item	(* item scanned *)
		|PKBugsScanners.char:
			CASE s.char OF
			|".": scanned := mv	(* missing value scanned *)
			|",": pos := s.Pos();
				IF s.eot THEN scanned := mv; RETURN END; 	(* missing value implied *)
				s.Scan;
				CASE s.type OF
				|PKBugsScanners.date, PKBugsScanners.clock, PKBugsScanners.real, PKBugsScanners.int:
					scanned := item	(* item scanned *)
				|PKBugsScanners.char:
					CASE s.char OF
					|".": scanned := mv	(* missing value scanned *)
					|",": scanned := mv; s.SetPos(pos)	(* missing value scanned *)
					|0X: scanned := mv; RETURN	(* missing value implied *)
					ELSE res := 5006; RETURN	(* unexpected token scanned *)
					END
				ELSE res := 5001; RETURN	(* unexpected or invalid token scanned *)
				END
			|0X: scanned := eot; RETURN	(* nothing scanned *)
			ELSE res := 5006; RETURN	(* unexpected token scanned *)
			END
		ELSE res := 5001; RETURN	(* unexpected or invalid token scanned *)
		END
	END ScanItem;

	PROCEDURE CountData* (VAR s: PKBugsScanners.Scanner; OUT nData, pos, res: INTEGER);
		VAR
			scanned: INTEGER;
			cursor: PKBugsNames.ItemList; ch: CHAR;
	BEGIN
		ASSERT(PKBugsNames.itemList # NIL, 25);
		pos := 0; res := 0; nData := 0; cursor := PKBugsNames.itemList;
		s.SetPos(pos); ch := s.nextCh; s.FindToken(ch);
		IF (ch = ",") THEN INC(nData); cursor := cursor.next	(* missing value implied *)
		ELSIF (ch = 0X) THEN pos := s.Pos(); res := 5100; RETURN	(* no data *)
		END;
		s.SetPos(pos);
		WHILE ~s.eot DO
			IF (cursor = NIL) THEN cursor := PKBugsNames.itemList END;
			IF (cursor.type = PKBugsNames.drop) THEN
				ScanDrop(s, scanned);
				ASSERT((scanned = drop) OR (scanned = mv) OR (scanned = eot), 25);
				IF (scanned = eot) THEN RETURN ELSE INC(nData); cursor := cursor.next END
			ELSE
				ScanItem(s, scanned, pos, res);
				IF res # 0 THEN RETURN END;
				ASSERT((scanned = item) OR (scanned = mv) OR (scanned = eot), 25);
				IF (scanned = eot) THEN RETURN ELSE INC(nData); cursor := cursor.next END
			END
		END
	END CountData;

	PROCEDURE Missing (type, i: INTEGER; VAR j: INTEGER; OUT res: INTEGER);
	BEGIN
		res := 0;
		CASE type OF
		|PKBugsNames.dv: dv[i].missing := TRUE
		|PKBugsNames.id: res := 5106; RETURN	(* id must not have missing values *)
		|PKBugsNames.mdv: mdv[i] := FALSE
		|PKBugsNames.time: res := 5108; RETURN	(* time must not have missing values *)
		|PKBugsNames.evid: res := 5109; RETURN	(* evid must not have missing values *)
		|PKBugsNames.amt: amt[i].missing := TRUE
		|PKBugsNames.rate: rate[i].missing := TRUE
		|PKBugsNames.ss: ss[i].missing := TRUE
		|PKBugsNames.ii: ii[i].missing := TRUE
		|PKBugsNames.date, PKBugsNames.dat1, PKBugsNames.dat2, PKBugsNames.dat3:
			date[i].day.missing := TRUE; date[i].month.missing := TRUE; date[i].year.missing := TRUE
		|PKBugsNames.addl: addl[i].missing := TRUE
		|PKBugsNames.abs: abs[i] := FALSE
		|PKBugsNames.lag: lag[i] := FALSE
		|PKBugsNames.lower: lower[i].missing := TRUE
		|PKBugsNames.upper: upper[i].missing := TRUE
		|PKBugsNames.other: others[i, j].missing := TRUE; INC(j)
		ELSE	(* drop *)
		END
	END Missing;

	PROCEDURE StoreReal (IN s: PKBugsScanners.Scanner; VAR var: RealData; OUT res: INTEGER);
	BEGIN
		res := 0;
		IF (s.type = PKBugsScanners.real) OR (s.type = PKBugsScanners.int) THEN var.value := s.real
		ELSE res := 5114; RETURN	(* expected real or integer *)
		END
	END StoreReal;

	PROCEDURE StoreID (IN s: PKBugsScanners.Scanner; i: INTEGER; OUT res: INTEGER);
		VAR long: LONGINT; integer: INTEGER;
	BEGIN
		res := 0;
		IF ((s.type = PKBugsScanners.int) & (s.nFields = 1)) OR
			((s.type = PKBugsScanners.real) & (s.nFields = 2) & (s.field[1] = 0)) THEN
			long := s.field[0]; IF s.neg[0] THEN long := - long END;
			LongToInt(long, integer, res); IF res # 0 THEN RETURN ELSE id[i] := integer END
		ELSE res := 5110; RETURN	(* invalid id *)
		END
	END StoreID;

	PROCEDURE StoreTime (IN s: PKBugsScanners.Scanner; i: INTEGER; OUT res: INTEGER);
	BEGIN
		res := 0;
		IF (s.type = PKBugsScanners.clock) OR (s.type = PKBugsScanners.real) OR
			(s.type = PKBugsScanners.int) THEN
			IF (s.real < 0) THEN res := 5112; RETURN	(* time must be non-negative *)
			ELSE time[i] := s.real
			END
		ELSE res := 5116; RETURN	(* expected time *)
		END
	END StoreTime;

	PROCEDURE StoreEVID (IN s: PKBugsScanners.Scanner; i: INTEGER; OUT res: INTEGER);
	BEGIN
		res := 0;
		IF (s.type = PKBugsScanners.real) OR (s.type = PKBugsScanners.int) THEN
			IF Equals(s.real, 0) THEN evid[i] := obs
			ELSIF Equals(s.real, 1) THEN evid[i] := dose
			ELSIF Equals(s.real, 2) THEN evid[i] := pred
			ELSIF Equals(s.real, 3) THEN evid[i] := reset
			ELSIF Equals(s.real, 4) THEN evid[i] := resDose
			ELSE res := 5113; RETURN	(* evid must be 0, 1, 2, 3, or 4 *)
			END
		ELSE res := 5114; RETURN	(* expected real or integer *)
		END
	END StoreEVID;

	PROCEDURE StoreAMT (IN s: PKBugsScanners.Scanner; i: INTEGER; OUT res: INTEGER);
	BEGIN
		res := 0;
		IF (s.type = PKBugsScanners.real) OR (s.type = PKBugsScanners.int) THEN
			IF (s.real < 0) OR Equals(s.real, 0) THEN amt[i].missing := TRUE ELSE amt[i].value := s.real END
		ELSE res := 5114; RETURN	(* expected real or integer *)
		END
	END StoreAMT;

	PROCEDURE StoreRate (IN s: PKBugsScanners.Scanner; i: INTEGER; OUT res: INTEGER);
	BEGIN
		res := 0;
		IF (s.type = PKBugsScanners.real) OR (s.type = PKBugsScanners.int) THEN
			IF Equals(s.real, 0) THEN rate[i].missing := TRUE ELSE rate[i].value := s.real END
		ELSE res := 5114; RETURN	(* expected real or integer *)
		END
	END StoreRate;

	PROCEDURE StoreSS (IN s: PKBugsScanners.Scanner; i: INTEGER; OUT res: INTEGER);
	BEGIN
		res := 0;
		IF (s.type = PKBugsScanners.real) OR (s.type = PKBugsScanners.int) THEN
			IF Equals(s.real, 1) THEN ss[i].value := 1
			ELSIF Equals(s.real, 2) THEN ss[i].value := 2
			ELSE ss[i].missing := TRUE
			END
		ELSE res := 5114; RETURN	(* expected real or integer *)
		END
	END StoreSS;

	PROCEDURE StoreII (IN s: PKBugsScanners.Scanner; i: INTEGER; OUT res: INTEGER);
	BEGIN
		res := 0;
		IF (s.type = PKBugsScanners.clock) OR (s.type = PKBugsScanners.real) OR
			(s.type = PKBugsScanners.int) THEN
			IF (s.real < 0) OR Equals(s.real, 0) THEN ii[i].missing := TRUE ELSE ii[i].value := s.real END
		ELSE res := 5116; RETURN	(* expected time *)
		END
	END StoreII;

	PROCEDURE StoreDate (IN s: PKBugsScanners.Scanner; type, i: INTEGER; OUT res: INTEGER);
		CONST century = 1900;
		VAR long: LONGINT; int0, int1, int2: INTEGER;
	BEGIN
		res := 0;
		IF (s.nFields = 1) THEN
			long := s.field[0]; IF s.neg[0] THEN long := - long END; LongToInt(long, int0, res);
			IF res # 0 THEN RETURN END;
			date[i].day.value := int0; date[i].month.missing := TRUE; date[i].year.missing := TRUE
		ELSIF (s.nFields = 2) THEN
			IF s.neg[0] OR s.neg[1] THEN res := 5118; RETURN END; 	(* invalid date *)
			LongToInt(s.field[0], int0, res); IF res # 0 THEN RETURN END;
			LongToInt(s.field[1], int1, res); IF res # 0 THEN RETURN END;
			IF (type = PKBugsNames.date) OR (type = PKBugsNames.dat2) THEN
				date[i].day.value := int1; date[i].month.value := int0; date[i].year.missing := TRUE
			ELSE
				date[i].day.value := int0; date[i].month.value := int1; date[i].year.missing := TRUE
			END
		ELSE	(* s.nFields = 3 *)
			IF s.neg[0] OR s.neg[1] OR s.neg[2] THEN res := 5118; RETURN END; 	(* invalid date *)
			LongToInt(s.field[0], int0, res); IF res # 0 THEN RETURN END;
			LongToInt(s.field[1], int1, res); IF res # 0 THEN RETURN END;
			LongToInt(s.field[2], int2, res); IF res # 0 THEN RETURN END;
			IF (type = PKBugsNames.date) OR (type = PKBugsNames.dat1) THEN
				IF (s.nDigits[2] = 2) THEN int2 := int2 + century END
			ELSE
				IF (s.nDigits[0] = 2) THEN int0 := int0 + century END
			END;
			CASE type OF
			|PKBugsNames.date: date[i].day.value := int1; date[i].month.value := int0; date[i].year.value := int2
			|PKBugsNames.dat1: date[i].day.value := int0; date[i].month.value := int1; date[i].year.value := int2
			|PKBugsNames.dat2: date[i].day.value := int2; date[i].month.value := int1; date[i].year.value := int0
			ELSE date[i].day.value := int1; date[i].month.value := int2; date[i].year.value := int0
			END
		END
	END StoreDate;

	PROCEDURE StoreADDL (IN s: PKBugsScanners.Scanner; i: INTEGER; OUT res: INTEGER);
		VAR integer: INTEGER;
	BEGIN
		res := 0;
		IF (s.type = PKBugsScanners.real) OR (s.type = PKBugsScanners.int) THEN
			IF (s.real < 0) OR Equals(s.real, 0) THEN addl[i].missing := TRUE
			ELSE
				IF ((s.type = PKBugsScanners.int) & (s.nFields = 1)) OR
					((s.type = PKBugsScanners.real) & (s.nFields = 2) & (s.field[1] = 0)) THEN
					LongToInt(s.field[0], integer, res); IF res # 0 THEN RETURN ELSE addl[i].value := integer END
				ELSE res := 5117; RETURN	(* invalid addl *)
				END
			END
		ELSE res := 5114; RETURN	(* expected real or integer *)
		END
	END StoreADDL;

	PROCEDURE StoreBool (IN s: PKBugsScanners.Scanner; OUT var: BOOLEAN; OUT res: INTEGER);
	BEGIN
		res := 0;
		IF (s.type = PKBugsScanners.real) OR (s.type = PKBugsScanners.int) THEN var := Equals(s.real, 1)
		ELSE res := 5114; RETURN	(* expected real or integer *)
		END
	END StoreBool;

	PROCEDURE StoreOther (IN s: PKBugsScanners.Scanner; i: INTEGER; VAR j: INTEGER; OUT res: INTEGER);
	BEGIN
		res := 0;
		IF (s.type = PKBugsScanners.real) OR (s.type = PKBugsScanners.int) THEN
			others[i, j].value := s.real; INC(j)
		ELSE res := 5114; RETURN	(* expected real or integer *)
		END
	END StoreOther;

	PROCEDURE LoadData* (VAR s: PKBugsScanners.Scanner; OUT pos, res: INTEGER);
		VAR
			scanned, i, j: INTEGER;
			cursor: PKBugsNames.ItemList; ch: CHAR;
	BEGIN
		ASSERT(PKBugsNames.itemList # NIL, 25);
		pos := 0; res := 0; cursor := PKBugsNames.itemList; i := 0; j := 0;
		s.SetPos(pos); ch := s.nextCh; s.FindToken(ch);
		IF (ch = ",") THEN
			Missing(cursor.type, i, j, res);
			IF res # 0 THEN RETURN ELSE cursor := cursor.next END
		ELSIF (ch = 0X) THEN pos := s.Pos(); res := 5100; RETURN	(* no data *)
		END;
		s.SetPos(pos);
		WHILE ~s.eot DO
			IF (cursor = NIL) THEN cursor := PKBugsNames.itemList; INC(i); j := 0 END;
			IF (cursor.type = PKBugsNames.drop) THEN
				ScanDrop(s, scanned);
				ASSERT((scanned = drop) OR (scanned = mv) OR (scanned = eot), 25);
				IF (scanned = eot) THEN RETURN ELSE cursor := cursor.next END
			ELSE
				ScanItem(s, scanned, pos, res); IF res # 0 THEN RETURN END;
				ASSERT((scanned = item) OR (scanned = mv) OR (scanned = eot), 25);
				IF (scanned = eot) THEN RETURN END;
				IF (scanned = mv) THEN
					Missing(cursor.type, i, j, res);
					IF res # 0 THEN RETURN ELSE cursor := cursor.next END
				ELSE	(* scanned = item *)
					CASE cursor.type OF
					|PKBugsNames.dv: StoreReal(s, dv[i], res)
					|PKBugsNames.id: StoreID(s, i, res)
					|PKBugsNames.mdv: StoreBool(s, mdv[i], res)
					|PKBugsNames.time: StoreTime(s, i, res)
					|PKBugsNames.evid: StoreEVID(s, i, res)
					|PKBugsNames.amt: StoreAMT(s, i, res)
					|PKBugsNames.rate: StoreRate(s, i, res)
					|PKBugsNames.ss: StoreSS(s, i, res)
					|PKBugsNames.ii: StoreII(s, i, res)
					|PKBugsNames.date, PKBugsNames.dat1, PKBugsNames.dat2, PKBugsNames.dat3:
						StoreDate(s, cursor.type, i, res)
					|PKBugsNames.addl: StoreADDL(s, i, res)
					|PKBugsNames.abs: StoreBool(s, abs[i], res)
					|PKBugsNames.lag: StoreBool(s, lag[i], res)
					|PKBugsNames.lower: StoreReal(s, lower[i], res)
					|PKBugsNames.upper: StoreReal(s, upper[i], res)
					ELSE StoreOther(s, i, j, res)
					END;
					IF res # 0 THEN RETURN
					ELSE cursor := cursor.next
					END
				END
			END
		END
	END LoadData;

	PROCEDURE InitializeData* (nRecords: INTEGER);
		VAR i, j: INTEGER;
	BEGIN
		IF PKBugsNames.nOthers > 0 THEN
			NEW(others, nRecords, PKBugsNames.nOthers);
			i := 0;
			WHILE (i < nRecords) DO
				j := 0; WHILE (j < PKBugsNames.nOthers) DO others[i, j].missing := FALSE; INC(j) END;
				INC(i)
			END
		END;
		IF PKBugsNames.IsPresent(PKBugsNames.dv) THEN
			NEW(dv, nRecords); i := 0; WHILE (i < nRecords) DO dv[i].missing := FALSE; INC(i) END
		END;
		IF PKBugsNames.IsPresent(PKBugsNames.id) THEN NEW(id, nRecords) END;
		IF PKBugsNames.IsPresent(PKBugsNames.mdv) THEN NEW(mdv, nRecords) END;
		IF PKBugsNames.IsPresent(PKBugsNames.time) THEN NEW(time, nRecords) END;
		IF PKBugsNames.IsPresent(PKBugsNames.evid) THEN NEW(evid, nRecords) END;
		IF PKBugsNames.IsPresent(PKBugsNames.amt) THEN
			NEW(amt, nRecords); i := 0; WHILE (i < nRecords) DO amt[i].missing := FALSE; INC(i) END
		END;
		IF PKBugsNames.IsPresent(PKBugsNames.rate) THEN
			NEW(rate, nRecords); i := 0; WHILE (i < nRecords) DO rate[i].missing := FALSE; INC(i) END
		END;
		IF PKBugsNames.IsPresent(PKBugsNames.ss) THEN
			NEW(ss, nRecords); i := 0; WHILE (i < nRecords) DO ss[i].missing := FALSE; INC(i) END
		END;
		IF PKBugsNames.IsPresent(PKBugsNames.ii) THEN
			NEW(ii, nRecords); i := 0; WHILE (i < nRecords) DO ii[i].missing := FALSE; INC(i) END
		END;
		IF PKBugsNames.IsPresent(PKBugsNames.addl) THEN
			NEW(addl, nRecords); i := 0; WHILE (i < nRecords) DO addl[i].missing := FALSE; INC(i) END
		END;
		IF PKBugsNames.IsPresent(PKBugsNames.lower) THEN
			NEW(lower, nRecords); i := 0; WHILE (i < nRecords) DO lower[i].missing := FALSE; INC(i) END
		END;
		IF PKBugsNames.IsPresent(PKBugsNames.upper) THEN
			NEW(upper, nRecords); i := 0; WHILE (i < nRecords) DO upper[i].missing := FALSE; INC(i) END
		END;
		IF PKBugsNames.IsPresent(PKBugsNames.abs) THEN NEW(abs, nRecords) END;
		IF PKBugsNames.IsPresent(PKBugsNames.lag) THEN NEW(lag, nRecords) END;
		IF PKBugsNames.IsPresent(PKBugsNames.date) 
			OR PKBugsNames.IsPresent(PKBugsNames.dat1) 
			OR PKBugsNames.IsPresent(PKBugsNames.dat2) 
			OR PKBugsNames.IsPresent(PKBugsNames.dat3) THEN
			NEW(date, nRecords);
			i := 0;
			WHILE (i < nRecords) DO
				date[i].day.missing := FALSE; date[i].month.missing := FALSE; date[i].year.missing := FALSE;
				INC(i)
			END
		END
	END InitializeData;

	PROCEDURE Reset*;
	BEGIN
		id := NIL; mdv := NIL; evid := NIL; ss := NIL; addl := NIL; abs := NIL; lag := NIL; dv := NIL; amt := NIL;
		rate := NIL; ii := NIL; date := NIL; time := NIL; others := NIL; lower := NIL; upper := NIL
	END Reset;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "D.Lunn"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer; Reset
	END Init;

BEGIN
	Init
END PKBugsData.
