(*			(* PKBugs Version 2.0 *)



license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)


MODULE PharmacoInputs;


	IMPORT
		Services,
		GraphNodes;

	CONST
		row* = 0; reset* = 1; input* = 2; level* = 3;
		amount* = 4; time* = 5; ss* = 6; addl* = 7; omega* = 8;
		nCol* = 12; nKnown* = 9;
		trap = 77;

	TYPE
		Input* = POINTER TO RECORD
			id-, nPar-, nCon-, cum-: INTEGER;
			name-: ARRAY 80 OF CHAR;
			labels-: POINTER TO ARRAY OF ARRAY 80 OF CHAR;
			next-: Input
		END;

		Summary* = POINTER TO ARRAY OF Input;

		Reset* = POINTER TO ARRAY OF INTEGER;

	VAR
		inputs-: Input;
		errorOff-: INTEGER;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE SetErrorOff* (off: INTEGER);
	BEGIN
		errorOff := off
	END SetErrorOff;

	PROCEDURE IdToInput* (id: INTEGER; list: Input): Input;
		VAR
			cursor, new: Input; i: INTEGER;
	BEGIN
		cursor := list;
		WHILE (cursor # NIL) & (cursor.id # id) DO cursor := cursor.next END;
		IF cursor = NIL THEN
			new := NIL
		ELSE
			NEW(new); new.next := NIL;
			new.name := cursor.name; new.id := cursor.id;
			new.nPar := cursor.nPar; new.nCon := cursor.nCon; new.cum := cursor.cum;
			IF new.nPar = 0 THEN new.labels := NIL ELSE NEW(new.labels, new.nPar) END;
			i := 0; WHILE i < new.nPar DO new.labels[i] := cursor.labels[i]; INC(i) END
		END;
		RETURN new
	END IdToInput;

	PROCEDURE NameToInput* (IN name: ARRAY OF CHAR): Input;
		VAR
			cursor, new: Input; i: INTEGER;
	BEGIN
		cursor := inputs;
		WHILE (cursor # NIL) & (cursor.name # name) DO cursor := cursor.next END;
		IF cursor = NIL THEN
			new := NIL
		ELSE
			NEW(new); new.next := NIL;
			new.name := cursor.name; new.id := cursor.id;
			new.nPar := cursor.nPar; new.nCon := cursor.nCon; new.cum := cursor.cum;
			IF new.nPar = 0 THEN new.labels := NIL ELSE NEW(new.labels, new.nPar) END;
			i := 0; WHILE i < new.nPar DO new.labels[i] := cursor.labels[i]; INC(i) END
		END;
		RETURN new
	END NameToInput;

	PROCEDURE AddInput* (new: Input; VAR list: Input);
		VAR
			cursor: Input;
	BEGIN
		IF (list = NIL) OR (new.id < list.id) THEN
			new.next := list; list := new
		ELSE
			cursor := list;
			WHILE (cursor.next # NIL) & (new.id >= cursor.next.id) DO
				cursor := cursor.next
			END;
			IF new.id # cursor.id THEN
				new.next := cursor.next; cursor.next := new
			END
		END
	END AddInput;

	PROCEDURE RegisterInput* (IN name: ARRAY OF CHAR; id, nPar, nCon: INTEGER);
		VAR
			new: Input;
			i: INTEGER;
	BEGIN
		NEW(new);
		new.name := name$;
		new.id := id;
		new.nPar := nPar;
		new.nCon := nCon;
		new.cum := 0;
		IF nPar # 0 THEN NEW(new.labels, nPar) ELSE new.labels := NIL END;
		i := 0;
		WHILE i < nPar DO new.labels[i] := ""; INC(i) END;
		AddInput(new, inputs)
	END RegisterInput;

	PROCEDURE AddInputLabel* (IN name, label: ARRAY OF CHAR; labelNum: INTEGER);
		VAR
			cursor: Input;
	BEGIN
		cursor := inputs;
		WHILE (cursor # NIL) & (inputs.name # name) DO cursor := cursor.next END;
		IF cursor # NIL THEN
			cursor.labels[labelNum] := label$
		END
	END AddInputLabel;

	PROCEDURE RealToInt* (node: GraphNodes.Node): INTEGER;
		CONST
			eps = 1.0E-20;
	BEGIN
		RETURN SHORT(ENTIER(node.value + eps));
	END RealToInt;

	PROCEDURE IsInteger* (node: GraphNodes.Node): BOOLEAN;
		CONST
			eps = 1.0E-20;
		VAR
			x: REAL;
			int: INTEGER;
	BEGIN
		x := node.value;
		int := SHORT(ENTIER(x + eps));
		RETURN ABS(x - int) < eps
	END IsInteger;

	PROCEDURE IsConstant* (node: GraphNodes.Node): BOOLEAN;
		VAR
			install: ARRAY 80 OF CHAR;
	BEGIN
		IF node # NIL THEN
			node.Install(install);
			RETURN install = "GraphConstant.Install"
		ELSE
			RETURN FALSE
		END
	END IsConstant;

	PROCEDURE CheckHist (hist: GraphNodes.SubVector): BOOLEAN;
	BEGIN
		ASSERT(hist.components # NIL, trap); ASSERT(hist.start >= 0, trap);
		ASSERT(hist.nElem > 0, trap); ASSERT(hist.step > 0, trap);
		RETURN (hist.start MOD nCol = 0) & (hist.nElem MOD nCol = 0) & (hist.step = 1)
	END CheckHist;

	PROCEDURE CheckTheta* (theta: GraphNodes.SubVector; summary: Summary;
	nComp: INTEGER): BOOLEAN;
		VAR
			nLev, i, size: INTEGER;
			cursor: Input;
	BEGIN
		ASSERT(theta.components # NIL, trap); ASSERT(theta.start >= 0, trap);
		ASSERT(theta.nElem > 0, trap); ASSERT(theta.step > 0, trap);
		nLev := LEN(summary);
		size := nLev - 1 + 2 * nComp;
		i := 0;
		WHILE i < nLev DO
			cursor := summary[i];
			WHILE cursor # NIL DO
				INC(size, cursor.nPar);
				cursor := cursor.next
			END;
			INC(i)
		END;
		RETURN theta.nElem = size
		(* size of parameter vector inconsistent with dosing matrix and/or no. of compartments *)
	END CheckTheta;

	PROCEDURE LevelExists (hist: GraphNodes.SubVector; lev: INTEGER): BOOLEAN;
		VAR
			exists: BOOLEAN;
			nRec, i, start, resInt, levInt: INTEGER;
			ok: BOOLEAN;
	BEGIN
		exists := FALSE;
		nRec := hist.nElem DIV nCol;
		i := 0;
		WHILE (i < nRec) & ~exists DO
			start := hist.start + i * nCol;
			ASSERT(IsConstant(hist.components[start + reset]), trap);
			ok := IsInteger(hist.components[start + reset]); ASSERT(ok, trap);
			resInt := RealToInt(hist.components[start + reset]);
			ASSERT((resInt = 0) OR (resInt = 1), trap);
			IF resInt = 0 THEN
				ASSERT(IsConstant(hist.components[start + level]), trap);
				ok := IsInteger(hist.components[start + level]); ASSERT(ok, trap);
				levInt := RealToInt(hist.components[start + level]);
				IF levInt = lev THEN exists := TRUE END
			END;
			INC(i)
		END;
		RETURN exists
	END LevelExists;

	PROCEDURE GetLevels (hist: GraphNodes.SubVector; OUT first, last, nRes: INTEGER; OUT res: SET);
		VAR
			nRec, i, start, rowInt, resInt, levInt: INTEGER;
			prev: REAL;
			p: GraphNodes.Node;
	BEGIN
		res := {};
		nRec := hist.nElem DIV nCol;
		i := 0; nRes := 0; first := 2; last := - 1; prev := MIN(REAL);
		WHILE i < nRec DO
			start := hist.start + i * nCol;
			p := hist.components[start + row];
			errorOff := start + row;
			IF ~IsConstant(p) THEN (* all row fields in dosing matrix must be constants *)
				res := {GraphNodes.arg4, GraphNodes.notData}; RETURN
			END;
			IF ~IsInteger(p) THEN
				res := {GraphNodes.arg4, GraphNodes.integer}; RETURN
			END;
			rowInt := RealToInt(p);
			IF rowInt # i THEN	(* row fields in each section of dosing matrix must be 0, 1, 2, ... *)
				res := {GraphNodes.arg4, GraphNodes.invalidValue}; RETURN
			END;
			errorOff := start + reset;
			p := hist.components[start + reset];
			IF ~IsConstant(p) THEN	(* all reset fields in dosing matrix must be constants *)
				res := {GraphNodes.arg4, GraphNodes.notData}; RETURN
			END;
			IF ~IsInteger(p) THEN
				res := {GraphNodes.arg4, GraphNodes.integer}; RETURN
			END;
			resInt := RealToInt(p);
			IF (resInt # 0) & (resInt # 1) THEN (* all reset fields in dosing matrix must be 0 or 1 *)
				res := {GraphNodes.arg4, GraphNodes.invalidValue}; RETURN
			END;
			errorOff := start + time;
			p := hist.components[start + time];
			IF ~IsConstant(p) THEN (* all time fields in dosing matrix must be constants *)
				res := {GraphNodes.arg4, GraphNodes.notData}; RETURN
			END;
			IF (resInt = 0) & (p.value < prev) THEN (* each section of dosing matrix must be chronological *)
				res := {GraphNodes.arg4, GraphNodes.invalidValue}; RETURN
			END;
			prev := p.value;
			IF resInt = 0 THEN
				errorOff := start + level;
				p := hist.components[start + level];
				IF ~IsConstant(p) THEN (* level fields in dosing matrix must be constants when reset = 0 *)
					res := {GraphNodes.arg4, GraphNodes.notData}; RETURN
				END;
				IF ~IsInteger(p) THEN (* level fields in dosing matrix must be integers when reset = 0 *)
					res := {GraphNodes.arg4, GraphNodes.integer}; RETURN
				END;
				levInt := RealToInt(p);
				first := MIN(first, levInt); last := MAX(last, levInt)
			ELSE
				INC(nRes)
			END;
			INC(i)
		END;
		IF (first # 0) & (first # 1) THEN (* lowest level in each section of dosing matrix must be 0 or 1 *)
			res := {GraphNodes.arg4, GraphNodes.invalidValue}; RETURN
		END;
		ASSERT(last >= first, trap);
		errorOff := hist.start + level;
		i := first;
		WHILE i <= last DO
			(* all levels between lowest and highest in each section of dosing matrix must be present *)
			IF ~LevelExists(hist, i) THEN
				res := {GraphNodes.arg4, GraphNodes.invalidValue}; RETURN
			END;
			INC(i)
		END
	END GetLevels;

	PROCEDURE SetCum (VAR summary: Summary);
		VAR
			len, i, cum: INTEGER;
			cursor: Input;
	BEGIN
		len := LEN(summary);
		i := 0; cum := 0;
		WHILE i < len DO
			cursor := summary[i];
			WHILE cursor # NIL DO
				cursor.cum := cum; INC(cum, cursor.nPar);
				cursor := cursor.next
			END;
			INC(i)
		END
	END SetCum;

	PROCEDURE GetSummary* (hist: GraphNodes.SubVector; OUT summary: Summary;
	OUT resInfo: Reset; OUT first, last: INTEGER; OUT res: SET);
		VAR
			inpVar: Input;
			p: GraphNodes.Node;
			len, i, j, nRec, nRes, start, resInt, levInt, inpInt: INTEGER;
	BEGIN
		IF ~CheckHist(hist) THEN
			errorOff := - 1;
			res := {GraphNodes.arg4, GraphNodes.length}; RETURN
		END;
		GetLevels(hist, first, last, nRes, res); IF res # {} THEN RETURN END;
		IF nRes = 0 THEN resInfo := NIL ELSE NEW(resInfo, nRes) END;
		len := last - first + 1; NEW(summary, len);
		i := 0; WHILE i < len DO summary[i] := NIL; INC(i) END;
		nRec := hist.nElem DIV nCol;
		i := 0; j := 0;
		WHILE i < nRec DO
			start := hist.start + i * nCol;
			p := hist.components[start + reset];
			ASSERT(IsConstant(p), trap); ASSERT(IsInteger(p), trap);
			resInt := RealToInt(p);
			ASSERT((resInt = 0) OR (resInt = 1), trap);
			IF resInt = 0 THEN
				p := hist.components[start + level];
				ASSERT(IsConstant(p), trap); ASSERT(IsInteger(p), trap);
				levInt := RealToInt(p);
				errorOff := start + input;
				p := hist.components[start + input];
				IF ~IsConstant(p) THEN (* input fields in dosing matrix must be constants when reset = 0 *)
					res := {GraphNodes.arg4, GraphNodes.notData}; RETURN
				END;
				IF ~IsInteger(p) THEN (* input fields in dosing matrix must be integers when reset = 0 *)
					res := {GraphNodes.integer}; RETURN
				END;
				inpInt := RealToInt(p);
				inpVar := IdToInput(inpInt, inputs);
				IF inpVar = NIL THEN (* specified input(s) in dosing matrix not found in Pharmaco/Rsrc/Inputs.odc *)
					res := {GraphNodes.arg4, GraphNodes.invalidValue}; RETURN
				END;
				AddInput(inpVar, summary[levInt - first])
			ELSE
				resInfo[j] := i;
				INC(j)
			END;
			INC(i)
		END;
		SetCum(summary)
	END GetSummary;

	PROCEDURE GetStart* (resInfo: Reset; pos: INTEGER; OUT start: INTEGER; OUT res: SET);
		VAR
			nRes, i: INTEGER;
			found: BOOLEAN;
	BEGIN
		res := {};
		IF resInfo = NIL THEN
			start := 0
		ELSE
			nRes := LEN(resInfo);
			i := nRes - 1; found := FALSE;
			WHILE (i >= 0) & ~found DO
				IF resInfo[i] = pos THEN (* pos argument must not correspond to a reset event *)
					res := {GraphNodes.arg4, GraphNodes.invalidValue}; RETURN
				END;
				IF resInfo[i] < pos THEN found := TRUE; start := resInfo[i] + 1 END;
				DEC(i)
			END;
			IF ~found THEN start := 0 END
		END
	END GetStart;

	PROCEDURE CountDoses* (hist: GraphNodes.SubVector; beg, end: INTEGER; tobs: REAL;
	OUT n: INTEGER; OUT res: SET);
		VAR
			interval, s: REAL; p: GraphNodes.Node;
			i, j, off, resInt, addlInt: INTEGER;
	BEGIN
		res := {};
		i := beg;
		n := 0;
		WHILE i <= end DO
			off := hist.start + i * nCol;
			ASSERT(IsInteger(hist.components[off + reset]), trap);
			ASSERT(RealToInt(hist.components[off + reset]) = 0, trap);
			s := hist.components[off + time].value; ASSERT(tobs >= s, trap);
			INC(n);
			errorOff := off + addl;
			p := hist.components[off + addl];
			IF IsConstant(p) THEN
				IF ~IsInteger(p) THEN (* addl values in dosing matrix must be integers *)
					res := {GraphNodes.arg4, GraphNodes.integer}; RETURN
				END;
				addlInt := RealToInt(p);
				IF addlInt < 1 THEN (* addl values in dosing matrix must be > 0 *)
					res := {GraphNodes.arg4, GraphNodes.invalidValue}; RETURN
				END;
				errorOff := off + omega;
				p := hist.components[off + omega];
				(* dosing interval fields in dosing matrix must be constants when addl specified *)
				IF ~IsConstant(p) THEN
					res := {GraphNodes.arg4, GraphNodes.notData}; RETURN
				END;
				interval := p.value;
				IF interval <= 0 THEN (* dosing interval values in dosing matrix must be > 0 *)
					res := {GraphNodes.arg4, GraphNodes.invalidValue}; RETURN
				END;
				j := 0; s := s + interval;
				WHILE (j < addlInt) & (s <= tobs) DO
					INC(n); INC(j); s := s + interval
				END
			END;
			INC(i)
		END
	END CountDoses;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "D.Lunn"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		inputs := NIL
	END Init;

BEGIN
	Init
END PharmacoInputs.
