(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE PKBugsCovts;

		(* PKBugs Version 1.1 *)

	IMPORT
		Math,
		PKBugsData, PKBugsNames, PKBugsParse;

	VAR
		covariates-: POINTER TO ARRAY OF SET;
		data-: POINTER TO ARRAY OF ARRAY OF REAL;
		covariateOK-: POINTER TO ARRAY OF BOOLEAN;
		covariateNames-: POINTER TO ARRAY OF ARRAY 80 OF CHAR;
		centres: POINTER TO ARRAY OF REAL;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE InitCovts* (nParams: INTEGER);
		VAR i: INTEGER;
	BEGIN
		NEW(covariates, nParams);
		i := 0; WHILE i < nParams DO covariates[i] := {}; INC(i) END
	END InitCovts;

	PROCEDURE IncludeCovts* (parameter, from, to: INTEGER);
		VAR i: INTEGER;
	BEGIN
		ASSERT(parameter < LEN(covariates), 25);
		i := from; WHILE i <= to DO INCL(covariates[parameter], i); INC(i) END
	END IncludeCovts;

	PROCEDURE ExcludeCovts* (parameter, from, to: INTEGER);
		VAR i: INTEGER;
	BEGIN
		ASSERT(parameter < LEN(covariates), 25);
		i := from; WHILE i <= to DO EXCL(covariates[parameter], i); INC(i) END
	END ExcludeCovts;

	PROCEDURE GetCovtNames;
		VAR
			i, j, len, numCovts: INTEGER;
			prefix: ARRAY 5 OF CHAR;
			cursor: PKBugsNames.ItemList;
	BEGIN
		IF covariateOK # NIL THEN len := LEN(covariateOK) ELSE len := 0 END;
		i := 0; numCovts := 0; prefix := "";
		WHILE i < len DO
			IF covariateOK[i] THEN INC(numCovts) END; INC(i)
		END;
		IF numCovts > 0 THEN NEW(covariateNames, numCovts) ELSE covariateNames := NIL END;
		cursor := PKBugsNames.itemList;
		i := 0; j := 0;
		WHILE (cursor # NIL) & (i < len) DO
			IF cursor.type = PKBugsNames.other THEN
				IF covariateOK[i] THEN
					covariateNames[j] := prefix + cursor.name$;
					INC(j)
				END;
				INC(i);
			END;
			cursor := cursor.next;
			IF cursor = NIL THEN cursor := PKBugsNames.itemList; prefix := "log." END;
		END
	END GetCovtNames;

	PROCEDURE IsCentred* (index: INTEGER): BOOLEAN;
		VAR isCentred, found: BOOLEAN; cursor: PKBugsNames.ItemList; nCov, i: INTEGER;
	BEGIN
		nCov := PKBugsNames.nOthers;
		IF index >= nCov THEN index := index - nCov END;
		cursor := PKBugsNames.itemList; ASSERT(cursor # NIL, 25);
		i := 0; found := FALSE;
		WHILE (cursor # NIL) & ~found DO
			IF cursor.type = PKBugsNames.other THEN
				IF i = index THEN found := TRUE; isCentred := cursor.centred
				ELSE INC(i)
				END
			END;
			cursor := cursor.next
		END;
		ASSERT(found, 25); RETURN isCentred
	END IsCentred;

	PROCEDURE IsCat* (index: INTEGER): BOOLEAN;
		VAR isCat, found: BOOLEAN; cursor: PKBugsNames.ItemList; nCov, i: INTEGER;
	BEGIN
		nCov := PKBugsNames.nOthers;
		IF index >= nCov THEN index := index - nCov END;
		cursor := PKBugsNames.itemList; ASSERT(cursor # NIL, 25);
		i := 0; found := FALSE;
		WHILE (cursor # NIL) & ~found DO
			IF cursor.type = PKBugsNames.other THEN
				IF i = index THEN found := TRUE; isCat := cursor.cat
				ELSE INC(i)
				END
			END;
			cursor := cursor.next
		END;
		ASSERT(found, 25); RETURN isCat
	END IsCat;

	PROCEDURE CentreCovts (nCov, nInd: INTEGER);
		VAR i, j: INTEGER;
	BEGIN
		NEW(centres, 2 * nCov);
		j := 0;
		WHILE j < 2 * nCov DO
			IF covariateOK[j] & IsCentred(j) THEN
				centres[j] := 0;
				i := 0; WHILE i < nInd DO centres[j] := centres[j] + data[i, j]; INC(i) END;
				centres[j] := centres[j] / nInd;
				i := 0; WHILE i < nInd DO data[i, j] := data[i, j] - centres[j]; INC(i) END
			END;
			INC(j)
		END
	END CentreCovts;

	PROCEDURE GetData* (OUT res, ind, cov: INTEGER);
		CONST
			eps = 1.0E-40;
		VAR
			i, j, k, nInd, nParams, nCov, nRecords, last, offInd: INTEGER;
			(*set: SET;*)
			missing: BOOLEAN;
	BEGIN
		res := 0; ind := 0; cov := 0;
		nInd := PKBugsParse.NumInd();
		(*nParams := LEN(covariates);*) nCov := PKBugsNames.nOthers;
		(*set := {}; i := 0; WHILE i < nParams DO set := set + covariates[i]; INC(i) END;*)
		IF nCov # 0 THEN
			GetCovtNames; (*	get all covariate names for use in error handling	*)
			NEW(data, nInd, 2 * nCov); NEW(covariateOK, 2 * nCov);
			j := 0; WHILE j < 2 * nCov DO covariateOK[j] := TRUE; INC(j) END;
			ASSERT(PKBugsData.id # NIL, 25); ASSERT(PKBugsData.others # NIL, 25);
			ASSERT(nCov = LEN(PKBugsData.others, 1), 25);
			nRecords := LEN(PKBugsData.id);
			j := 0;
			WHILE j < 2 * nCov DO
				IF j < nCov THEN k := j ELSE k := j - nCov END;
				i := 0; offInd := 0; last := PKBugsData.id[0]; missing := TRUE;
				WHILE i < nRecords DO
					IF PKBugsData.id[i] # last THEN
						IF missing THEN
							covariateOK[j] := FALSE;
							(*IF j IN set THEN
							res := 5801; ind := offInd + 1; cov := k; RETURN	(* cov missing for individual ind *)
							END*)
						END;
						INC(offInd); missing := TRUE
					END;
					IF missing & ~PKBugsData.others[i, k].missing THEN
						IF j < nCov THEN
							data[offInd, j] := PKBugsData.others[i, k].value;
							missing := FALSE
						ELSE
							IF PKBugsData.others[i, k].value < eps THEN
								covariateOK[j] := FALSE;
								(*IF j IN set THEN
								res := 5802; ind := offInd + 1; cov := k; RETURN	(* cannot log cov for individual ind *)
								END*)
							ELSE data[offInd, j] := Math.Ln(PKBugsData.others[i, k].value);
							END;
							missing := FALSE
						END
					END;
					last := PKBugsData.id[i]; INC(i)
				END;
				IF missing THEN
					covariateOK[j] := FALSE;
					(*IF j IN set THEN
					res := 5801; ind := offInd + 1; cov := k; RETURN	(* cov missing for individual ind *)
					END*)
				END;
				INC(j)
			END;
			CentreCovts(nCov, nInd);
			GetCovtNames (*	just the possible covariates	*)
		ELSE data := NIL; covariateOK := NIL; centres := NIL
		END
	END GetData;

	PROCEDURE Covariates* (param: INTEGER): POINTER TO ARRAY OF INTEGER;
		VAR
			len, i, j, k, off: INTEGER;
			set: SET;
			array: POINTER TO ARRAY OF INTEGER;
	BEGIN
		set := covariates[param];
		i := 0; len := 0; WHILE i <= MAX(SET) DO IF i IN set THEN INC(len) END; INC(i) END;
		IF len = 0 THEN RETURN NIL END;
		NEW(array, len);
		i := 0; 
		k := 0;
		WHILE i <= MAX(SET) DO 
			IF i IN set THEN 
				off := 0; 
				j := 0;
				WHILE j # i DO
					IF covariateOK[off] THEN INC(j) END;
					INC(off);
				END;
				array[k] := off;
				INC(k)
			END; 
			INC(i) 
		END;
		RETURN array
	END Covariates;

	PROCEDURE Reset*;
	BEGIN
		covariates := NIL; data := NIL; covariateOK := NIL; centres := NIL;
		covariateNames := NIL
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
END PKBugsCovts.
