(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE PKBugsParse;

		(* PKBugs Version 1.1 *)

	IMPORT
		PKBugsData,
		BugsIndex, BugsNames,
		GraphConstant,
		PharmacoInputs;

	TYPE
		LevInfo = RECORD
			iv: BOOLEAN;
			inputs: PharmacoInputs.Input
		END;

		Date = RECORD
			year*, month*, day*: INTEGER
		END;

	VAR
		events-: POINTER TO ARRAY OF INTEGER;
		summary: POINTER TO ARRAY OF LevInfo;
		offHist-, offData-: POINTER TO ARRAY OF INTEGER;
		history, time, pos: BugsNames.Name;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	(*	steal the following two functions from Dates module because do not have unix version	*)
	PROCEDURE ValidDate (IN d: Date): BOOLEAN;
		VAR y, m, d1: INTEGER;
	BEGIN
		IF (d.year < 1) OR (d.year > 9999) OR (d.month < 1) OR (d.month > 12) OR (d.day < 1) THEN
			RETURN FALSE
		ELSE
			y := d.year; m := d.month;
			IF m = 2 THEN
				IF (y < 1583) & (y MOD 4 = 0)
					OR (y MOD 4 = 0) & ((y MOD 100 # 0) OR (y MOD 400 = 0)) THEN
					d1 := 29
				ELSE d1 := 28
				END
			ELSIF m IN {1, 3, 5, 7, 8, 10, 12} THEN d1 := 31
			ELSE d1 := 30
			END;
			IF (y = 1582) & (m = 10) & (d.day > 4) & (d.day < 15) THEN RETURN FALSE END;
			RETURN d.day <= d1
		END
	END ValidDate;

	PROCEDURE Day (IN d: Date): INTEGER;
		VAR y, m, n: INTEGER;
	BEGIN
		y := d.year; m := d.month - 3;
		IF m < 0 THEN INC(m, 12); DEC(y) END;
		n := y * 1461 DIV 4 + (m * 153 + 2) DIV 5 + d.day - 306;
		IF n > 577737 THEN n := n - (y DIV 100 * 3 - 5) DIV 4 END;
		RETURN n
	END Day;

	PROCEDURE GetEvents;
		VAR len, i: INTEGER;
	BEGIN
		ASSERT(PKBugsData.dv # NIL, 25); ASSERT(PKBugsData.id # NIL, 25);
		ASSERT(PKBugsData.time # NIL, 25); ASSERT(PKBugsData.amt # NIL, 25);
		len := LEN(PKBugsData.time); NEW(events, len);
		IF PKBugsData.evid # NIL THEN
			i := 0; WHILE i < len DO events[i] := PKBugsData.evid[i]; INC(i) END
		ELSIF PKBugsData.mdv # NIL THEN
			i := 0;
			WHILE i < len DO
				IF ~PKBugsData.mdv[i] THEN events[i] := PKBugsData.obs
				ELSIF PKBugsData.amt[i].missing THEN events[i] := PKBugsData.pred
				ELSE events[i] := PKBugsData.dose
				END;
				INC(i)
			END
		ELSE
			i := 0;
			WHILE i < len DO
				IF ~PKBugsData.amt[i].missing THEN events[i] := PKBugsData.dose
				ELSIF PKBugsData.dv[i].missing THEN events[i] := PKBugsData.pred
				ELSE events[i] := PKBugsData.obs
				END;
				INC(i)
			END
		END
	END GetEvents;

	PROCEDURE GetDim (OUT nInd, nHist, nData, res, ind: INTEGER);
		VAR len, i, ni, hi, last: INTEGER;
	BEGIN
		res := 0; ind := 0;
		ASSERT(events # NIL, 25); len := LEN(events);
		ASSERT(PKBugsData.id # NIL, 25); ASSERT(LEN(PKBugsData.id) = len, 25);
		nInd := 1; nHist := 0; nData := 0;
		i := 0; ni := 0; hi := 0; last := PKBugsData.id[0];
		WHILE i < len DO
			IF PKBugsData.id[i] # last THEN
				IF ni = 0 THEN res := 5300; ind := nInd; RETURN END; 	(* dose and/or reset events only *)
				IF hi = 0 THEN res := 5314; ind := nInd; RETURN END; 	(* no dosing history *)
				INC(nInd); INC(nData, ni); ni := 0; INC(nHist, hi); hi := 0
			END;
			last := PKBugsData.id[i];
			CASE events[i] OF
			|PKBugsData.obs, PKBugsData.pred: INC(ni)
			|PKBugsData.dose: INC(hi);
				IF (PKBugsData.ss # NIL) & ~PKBugsData.ss[i].missing & (PKBugsData.ss[i].value = 1) THEN
					INC(hi)	(* reset event implied *)
				END
			|PKBugsData.reset: INC(hi)
			|PKBugsData.resDose: INC(hi, 2)
			END;
			INC(i)
		END;
		IF ni = 0 THEN res := 5300; ind := nInd; RETURN END; 	(* dose and/or reset events only *)
		IF hi = 0 THEN res := 5314; ind := nInd; RETURN END; 	(* no dosing history *)
		INC(nData, ni); INC(nHist, hi)
	END GetDim;

	PROCEDURE InitNames (nInd, nHist, nData: INTEGER);
	BEGIN
		ASSERT(nInd > 0, 25); ASSERT(nHist > 0, 25); ASSERT(nData > 0, 25);
		history := BugsNames.New("hist", 2);
		history.SetRange(0, nHist); history.SetRange(1, PharmacoInputs.nCol);
		history.AllocateNodes;
		time := BugsNames.New("time", 1); time.SetRange(0, nData);
		time.AllocateNodes;
		pos := BugsNames.New("pos", 1); pos.SetRange(0, nData);
		pos.AllocateNodes;
		NEW(offHist, nInd + 1); NEW(offData, nInd + 1); offHist[0] := 1; offData[0] := 1
	END InitNames;

	PROCEDURE GetInputName (i: INTEGER; OUT name: ARRAY OF CHAR);
	BEGIN
		IF (PKBugsData.rate # NIL) & ~PKBugsData.rate[i].missing THEN
			IF PKBugsData.rate[i].value > 0 THEN name := "IVinf"
			ELSE IF (PKBugsData.lag # NIL) & PKBugsData.lag[i] THEN
					name := "ZOlag"
				ELSE name := "ZO"
				END
			END
		ELSE
			IF (PKBugsData.abs # NIL) & PKBugsData.abs[i] THEN
				IF (PKBugsData.lag # NIL) & PKBugsData.lag[i] THEN name := "FOlag" ELSE name := "FO" END
			ELSE name := "IVbol"
			END
		END
	END GetInputName;

	PROCEDURE StoreInput (j, i: INTEGER; OUT res: INTEGER; OUT inp: ARRAY OF CHAR);
		VAR
			name: ARRAY 80 OF CHAR;
			new: PharmacoInputs.Input;
	BEGIN
		res := 0; inp := "";
		GetInputName(i, name);
		IF (name = "IVbol") OR (name = "IVinf") THEN summary[j].iv := TRUE
		ELSE
			new := PharmacoInputs.NameToInput(name);
			IF new = NIL THEN res := 5316; inp := name$; RETURN END;
			(* input not found in PKLink/Rsrc/Inputs.odc *)
			PharmacoInputs.AddInput(new, summary[j].inputs)
		END
	END StoreInput;

	PROCEDURE GetSummary (nInd, len: INTEGER; OUT res, ind: INTEGER; OUT inp: ARRAY OF CHAR);
		VAR i, j, last: INTEGER;
	BEGIN
		res := 0; ind := 0; inp := "";
		NEW(summary, nInd);
		j := 0; WHILE j < nInd DO summary[j].iv := FALSE; summary[j].inputs := NIL; INC(j) END;
		i := 0; j := 0; last := PKBugsData.id[0];
		WHILE i < len DO
			IF PKBugsData.id[i] # last THEN INC(j) END; last := PKBugsData.id[i];
			IF (events[i] = PKBugsData.dose) OR (events[i] = PKBugsData.resDose) THEN
				StoreInput(j, i, res, inp); IF res # 0 THEN ind := j + 1; RETURN END
			END;
			INC(i)
		END
	END GetSummary;

	PROCEDURE RelTime (IN d, rd: PKBugsData.Date; t, rt: REAL; OUT hours: REAL; OUT res: INTEGER);
		CONST
			missing = 0; day = 1; dm = 2; dmy = 3; notLeap = 1999;
		VAR
			format: INTEGER; d0, d1: Date;

		PROCEDURE Format (IN date: PKBugsData.Date): INTEGER;
			VAR type: INTEGER;
		BEGIN
			IF date.day.missing & date.month.missing & date.year.missing THEN type := missing
			ELSE
				IF date.month.missing THEN type := day
				ELSIF date.year.missing THEN type := dm
				ELSE type := dmy
				END
			END;
			RETURN type
		END Format;

	BEGIN
		res := 0;
		format := Format(rd);
		IF Format(d) # format THEN res := 5301; RETURN END; 	(* inconsistent date formats *)
		CASE format OF
		|missing: hours := t - rt
		|day: hours := 24 * (d.day.value - rd.day.value) + t - rt
		|dm:
			d0.day := rd.day.value; d0.month := rd.month.value; d0.year := notLeap;
			d1.day := d.day.value; d1.month := d.month.value; d1.year := notLeap;
			IF ~ValidDate(d0) OR ~ValidDate(d1) THEN res := 5302; RETURN END;
			(* invalid date(s) *)
			hours := 24 * (Day(d1) - Day(d0)) + t - rt
		|dmy:
			d0.day := rd.day.value; d0.month := rd.month.value; d0.year := rd.year.value;
			d1.day := d.day.value; d1.month := d.month.value; d1.year := d.year.value;
			IF ~ValidDate(d0) OR ~ValidDate(d1) THEN res := 5302; RETURN END;
			(* invalid date(s) *)
			hours := 24 * (Day(d1) - Day(d0)) + t - rt
		END
	END RelTime;

	PROCEDURE AddResetRow (row, p: INTEGER; evTime: REAL);
		VAR s, index, len: INTEGER;
	BEGIN
		ASSERT(history.components # NIL, 25); len := LEN(history.components);
		s := row * PharmacoInputs.nCol;
		index := s + PharmacoInputs.row; ASSERT(index < len, 25);
		history.components[index] := GraphConstant.New(p + 1);
		index := s + PharmacoInputs.reset; ASSERT(index < len, 25);
		history.components[index] := GraphConstant.New(1);
		index := s + PharmacoInputs.time; ASSERT(index < len, 25);
		history.components[index] := GraphConstant.New(evTime)
	END AddResetRow;

	PROCEDURE GetLevel (id, ind: INTEGER; OUT lev: INTEGER);
		VAR
			cursor: PharmacoInputs.Input; found: BOOLEAN;
	BEGIN
		cursor := summary[ind - 1].inputs; ASSERT(cursor # NIL, 25);
		found := FALSE; lev := 1;
		WHILE (cursor # NIL) & ~found DO
			IF cursor.id = id THEN found := TRUE ELSE INC(lev) END;
			cursor := cursor.next
		END;
		ASSERT(found, 25)
	END GetLevel;

	PROCEDURE AddDoseRow (i, ind, row, p: INTEGER; evTime: REAL;
	OUT res: INTEGER; OUT inp: ARRAY OF CHAR);
		VAR
			name: ARRAY 80 OF CHAR;
			s, index, len, lev: INTEGER;
			new: PharmacoInputs.Input;
	BEGIN
		res := 0; inp := "";
		ASSERT(history.components # NIL, 25); len := LEN(history.components);
		s := row * PharmacoInputs.nCol;
		index := s + PharmacoInputs.row; ASSERT(index < len, 25);
		history.components[index] := GraphConstant.New(p + 1);
		index := s + PharmacoInputs.reset; ASSERT(index < len, 25);
		history.components[index] := GraphConstant.New(0);
		index := s + PharmacoInputs.time; ASSERT(index < len, 25);
		history.components[index] := GraphConstant.New(evTime);
		GetInputName(i, name); new := PharmacoInputs.NameToInput(name);
		IF new = NIL THEN
			res := 5316; res := - res; inp := name$; RETURN	(* input not found in PKLink/Rsrc/Inputs.odc *)
		END;
		index := s + PharmacoInputs.input; ASSERT(index < len, 25);
		history.components[index] := GraphConstant.New(new.id);
		IF (name = "IVbol") OR (name = "IVinf") THEN lev := 0 ELSE GetLevel(new.id, ind, lev) END;
		index := s + PharmacoInputs.level; ASSERT(index < len, 25);
		history.components[index] := GraphConstant.New(lev);
		IF PKBugsData.amt[i].missing THEN res := 5303; RETURN END;
		(* amt missing on dose event record *)
		index := s + PharmacoInputs.amount; ASSERT(index < len, 25);
		history.components[index] := GraphConstant.New(PKBugsData.amt[i].value);
		IF (PKBugsData.ss # NIL) & ~PKBugsData.ss[i].missing THEN
			ASSERT(PKBugsData.ii # NIL, 25);
			IF PKBugsData.ii[i].missing THEN res := 5304; RETURN END; 	(* ii missing for steady state dose *)
			index := s + PharmacoInputs.ss; ASSERT(index < len, 25);
			history.components[index] := GraphConstant.New(1);
			index := s + PharmacoInputs.omega; ASSERT(index < len, 25);
			history.components[index] := GraphConstant.New(PKBugsData.ii[i].value)
		ELSE
			index := s + PharmacoInputs.ss; ASSERT(index < len, 25);
			history.components[index] := GraphConstant.New(0)
		END;
		IF (PKBugsData.addl # NIL) & ~PKBugsData.addl[i].missing THEN
			ASSERT(PKBugsData.ii # NIL, 25);
			IF PKBugsData.ii[i].missing THEN res := 5306; RETURN END; 	(* ii missing for implied doses *)
			index := s + PharmacoInputs.addl; ASSERT(index < len, 25);
			history.components[index] := GraphConstant.New(PKBugsData.addl[i].value);
			index := s + PharmacoInputs.omega; ASSERT(index < len, 25);
			IF history.components[index] = NIL THEN
				history.components[index] := GraphConstant.New(PKBugsData.ii[i].value)
			END
		END;
		IF name = "IVinf" THEN
			ASSERT((PKBugsData.rate # NIL) & ~PKBugsData.rate[i].missing
			 & (PKBugsData.rate[i].value > 0), 25);
			index := s + PharmacoInputs.nKnown; ASSERT(index < len, 25);
			history.components[index] := GraphConstant.New(PKBugsData.amt[i].value / 
			PKBugsData.rate[i].value)
		END
	END AddDoseRow;

	PROCEDURE BuildHistory* (OUT res, ind: INTEGER; OUT inp: ARRAY OF CHAR);
		VAR
			nInd, nHist, nData, len, i, last, j, dose, obs, p: INTEGER;
			reset, dates, ssRes: BOOLEAN; refTime, evTime, prev: REAL;
			refDate: PKBugsData.Date;

		PROCEDURE InitVars;
		BEGIN
			dates := PKBugsData.date # NIL;
			j := 1; dose := 0; obs := 0;
			refTime := PKBugsData.time[0]; IF dates THEN refDate := PKBugsData.date[0] END;
			prev := 0; p := - 1; reset := TRUE; last := PKBugsData.id[0]
		END InitVars;

	BEGIN
		res := 0; ind := 0; inp := "";
		GetEvents;
		GetDim(nInd, nHist, nData, res, ind); IF res # 0 THEN RETURN END;
		InitNames(nInd, nHist, nData);
		ASSERT(events # NIL, 25); len := LEN(events);
		GetSummary(nInd, len, res, ind, inp); IF res # 0 THEN res := - res; RETURN END;
		InitVars;
		i := 0;
		WHILE i < len DO
			IF PKBugsData.id[i] # last THEN
				offHist[j] := dose + 1; offData[j] := obs + 1;
				refTime := PKBugsData.time[i]; IF dates THEN refDate := PKBugsData.date[i] END;
				INC(j); prev := 0; p := - 1; reset := TRUE
			END;
			last := PKBugsData.id[i];
			ssRes := (events[i] = PKBugsData.dose) & (PKBugsData.ss # NIL)
			 & ~PKBugsData.ss[i].missing & (PKBugsData.ss[i].value = 1);
			IF (events[i] = PKBugsData.reset) OR (events[i] = PKBugsData.resDose) OR ssRes THEN
				refTime := PKBugsData.time[i]; IF dates THEN refDate := PKBugsData.date[i] END; prev := 0
			END;
			IF dates THEN
				RelTime(PKBugsData.date[i], refDate, PKBugsData.time[i], refTime, evTime, res);
				IF res # 0 THEN ind := j; RETURN END
			ELSE evTime := PKBugsData.time[i] - refTime
			END;
			IF evTime < prev THEN res := 5307; ind := j; RETURN END; 	(* data not in chronological order *)
			prev := evTime;
			CASE events[i] OF
			|PKBugsData.obs, PKBugsData.pred:
				IF reset THEN res := 5315; ind := j; RETURN END; 	(* no dose events before obs/pred event *)
				ASSERT(p >= 0, 25);
				time.components[obs] := GraphConstant.New(evTime);
				pos.components[obs] := GraphConstant.New(p);
				INC(obs)
			|PKBugsData.dose:
				IF ssRes THEN AddResetRow(dose, p, evTime); INC(dose); INC(p) END;
				AddDoseRow(i, j, dose, p, evTime, res, inp); IF res # 0 THEN ind := j; RETURN END;
				INC(dose); INC(p); reset := FALSE
			|PKBugsData.reset: AddResetRow(dose, p, evTime); INC(dose); INC(p)
			|PKBugsData.resDose:
				AddResetRow(dose, p, evTime); INC(dose); INC(p);
				AddDoseRow(i, j, dose, p, evTime, res, inp); IF res # 0 THEN ind := j; RETURN END;
				INC(dose); INC(p); reset := FALSE
			END;
			INC(i)
		END;
		offHist[j] := dose + 1; offData[j] := obs + 1
	END BuildHistory;

	PROCEDURE CheckInputs* (OUT res, ind: INTEGER);
		VAR
			nInd, j: INTEGER;
			cursor0, cursor1: PharmacoInputs.Input;
	BEGIN
		res := 0; ind := 0;
		ASSERT(summary # NIL, 25); nInd := LEN(summary);
		IF ~summary[0].iv THEN ASSERT(summary[0].inputs # NIL, 25) END;
		j := 1;
		WHILE j < nInd DO
			IF summary[j].iv # summary[0].iv THEN res := 5310; ind := j + 1; RETURN END;
			(* drug inputs not identical *)
			IF ~summary[j].iv THEN ASSERT(summary[j].inputs # NIL, 25) END;
			cursor0 := summary[0].inputs; cursor1 := summary[j].inputs;
			WHILE cursor0 # NIL DO
				IF (cursor1 = NIL) OR (cursor1.id # cursor0.id) THEN
					res := 5310; ind := j + 1; RETURN	(* drug inputs not identical *)
				END;
				cursor0 := cursor0.next; cursor1 := cursor1.next
			END;
			IF cursor1 # NIL THEN res := 5310; ind := j + 1; RETURN END; 	(* drug inputs not identical *)
			INC(j)
		END
	END CheckInputs;

	PROCEDURE StoreHist*;
	BEGIN
		ASSERT(history # NIL, 25); BugsIndex.Store(history);
		ASSERT(time # NIL, 25); BugsIndex.Store(time);
		ASSERT(pos # NIL, 25); BugsIndex.Store(pos)
	END StoreHist;

	PROCEDURE GetParLabels* (OUT bio, abs: POINTER TO ARRAY OF ARRAY OF CHAR);
		VAR
			nBio, nAbs, a, b, i: INTEGER;
			base, string: ARRAY 80 OF CHAR;
			cursor: PharmacoInputs.Input;
	BEGIN
		ASSERT(summary # NIL, 25);
		IF summary[0].iv THEN base := "IV"; string := "F / (1 - F) {"
		ELSE ASSERT(summary[0].inputs # NIL, 25); base := summary[0].inputs.name$; string := "F {"
		END;
		nBio := - 1; nAbs := 0; IF summary[0].iv THEN INC(nBio) END;
		cursor := summary[0].inputs;
		WHILE cursor # NIL DO
			INC(nBio); INC(nAbs, cursor.nPar);
			cursor := cursor.next
		END;
		ASSERT(nBio >= 0, 25);
		IF nBio = 0 THEN bio := NIL ELSE NEW(bio, nBio, 80) END;
		IF nAbs = 0 THEN abs := NIL ELSE NEW(abs, nAbs, 80) END;
		b := - 1; a := 0; IF summary[0].iv THEN INC(b) END;
		cursor := summary[0].inputs;
		WHILE cursor # NIL DO
			IF b >= 0 THEN bio[b] := string + cursor.name + ":" + base + "}" END; INC(b);
			i := 0;
			WHILE i < cursor.nPar DO abs[a] := cursor.labels[i] + " {" + cursor.name + "}"; INC(a); INC(i) END;
			cursor := cursor.next
		END
	END GetParLabels;

	PROCEDURE NumInd* (): INTEGER;
	BEGIN
		ASSERT(summary # NIL, 25); RETURN LEN(summary)
	END NumInd;

	PROCEDURE NumData* (): INTEGER;
	BEGIN
		ASSERT((time # NIL) & (time.components # NIL), 25);
		RETURN LEN(time.components)
	END NumData;

	PROCEDURE Reset*;
	BEGIN
		events := NIL; summary := NIL;
		offHist := NIL; offData := NIL;
		history := NIL; time := NIL; pos := NIL
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
END PKBugsParse.
