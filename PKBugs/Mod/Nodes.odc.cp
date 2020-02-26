(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE PKBugsNodes;

		(* PKBugs Version 1.1 *)

	IMPORT
		Math,
		PKBugsCovts, PKBugsData, PKBugsParse, PKBugsPriors,
		BugsIndex, BugsNames,
		GraphConstant, GraphNodes, GraphStochastic;

	CONST
		eps = 1.0E-40;

		(* censoring and truncation of distribution *)
		std = 0;
		left = 1;
		right = 2;
		interval* = 3;

	VAR
		data, lower, upper: BugsNames.Name;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE InitData (nData: INTEGER);
	BEGIN
		data := BugsNames.New("data", 1); data.passByreference := TRUE;
		data.SetRange(0, nData); data.AllocateNodes;
		lower := BugsNames.New("lower", 1); lower.passByreference := TRUE;
		lower.SetRange(0, nData); lower.AllocateNodes;
		upper := BugsNames.New("upper", 1); upper.passByreference := TRUE;
		upper.SetRange(0, nData); upper.AllocateNodes
	END InitData;

	PROCEDURE GetBounds (i: INTEGER; log: BOOLEAN; OUT l, u: GraphNodes.Node;
	OUT bounds, res: INTEGER);
		VAR
			b: REAL;
	BEGIN
		res := 0;
		IF (PKBugsData.lower # NIL) & ~PKBugsData.lower[i].missing THEN
			b := PKBugsData.lower[i].value;
			IF b < 0 THEN res := 5311; RETURN END; 	(* lower bound < 0 *)
			IF log THEN
				IF b < eps THEN l := NIL
				ELSE l := GraphConstant.New(Math.Ln(b))
				END
			ELSE l := GraphConstant.New(b)
			END
		ELSE l := NIL
		END;
		IF (PKBugsData.upper # NIL) & ~PKBugsData.upper[i].missing THEN
			b := PKBugsData.upper[i].value;
			IF l = NIL THEN IF ~(b > 0) THEN res := 5312; RETURN END	(* upper bound < or = 0 *)
			ELSE
				ASSERT((PKBugsData.lower # NIL) & ~PKBugsData.lower[i].missing, 25);
				IF ~(b > PKBugsData.lower[i].value) THEN
					res := 5313; RETURN	(* upper bound < or = lower bound *)
				END
			END;
			IF log THEN u := GraphConstant.New(Math.Ln(b)) ELSE u := GraphConstant.New(b) END
		ELSE u := NIL
		END;
		IF l # NIL THEN
			IF u # NIL THEN bounds := interval
			ELSE bounds := left
			END
		ELSIF u # NIL THEN bounds := right
		ELSE bounds := std
		END
	END GetBounds;

	PROCEDURE CreateDataNodes* (log: BOOLEAN; OUT res, ind: INTEGER);
		VAR
			nData, len, i, j, last, obs, bounds: INTEGER;
			val: REAL;
	BEGIN
		res := 0; ind := 0;
		nData := PKBugsParse.NumData(); InitData(nData);
		ASSERT(PKBugsParse.events # NIL, 25); len := LEN(PKBugsParse.events);
		ASSERT(PKBugsData.id # NIL, 25); last := PKBugsData.id[0];
		i := 0; j := 1; obs := 0;
		WHILE i < len DO
			IF PKBugsData.id[i] # last THEN INC(j) END; last := PKBugsData.id[i];
			IF PKBugsParse.events[i] = PKBugsData.obs THEN
				IF PKBugsData.dv[i].missing OR (PKBugsData.dv[i].value < 0) THEN
					res := 5305; ind := j; RETURN	(* dv missing or < 0 on observation event record *)
				END;
				IF log & (PKBugsData.dv[i].value < eps) THEN
					res := 5503; ind := j; RETURN	(* dv = 0 not allowed with log-normal residuals *)
				END;
				val := PKBugsData.dv[i].value;
				IF log THEN val := Math.Ln(val) END;
				data.components[obs] := GraphConstant.New(val);
				lower.components[obs] := NIL; upper.components[obs] := NIL;
				INC(obs)
			ELSIF PKBugsParse.events[i] = PKBugsData.pred THEN
				GetBounds(i, log, lower.components[obs], upper.components[obs], bounds, res);
				IF res # 0 THEN ind := j; RETURN END;
				INC(obs)
			ELSE
			END;
			INC(i)
		END
	END CreateDataNodes;

	PROCEDURE UndefinedVector (v: GraphNodes.Vector): BOOLEAN;
		VAR
			i, len: INTEGER;
			undefined: BOOLEAN;
	BEGIN
		undefined := TRUE;
		len := LEN(v);
		i := 0; WHILE undefined & (i < len) DO undefined := v[i] = NIL; INC(i) END;
		RETURN undefined
	END UndefinedVector;

	PROCEDURE StoreData*;
		VAR
			big, small: GraphNodes.Node;
			i, nData: INTEGER;
	BEGIN
		ASSERT(data # NIL, 25); BugsIndex.Store(data);
		big := GraphConstant.New(1.0E+6);
		small := GraphConstant.New( - 1.0E+6);
		ASSERT(lower # NIL, 25);
		IF ~UndefinedVector(lower.components) THEN
			nData := LEN(lower.components);
			i := 0;
			WHILE i < nData DO 
				IF lower.components[i] = NIL THEN lower.components[i] := small END; INC(i)
			END;
			BugsIndex.Store(lower)
		END;
		ASSERT(upper # NIL, 25);
		IF ~UndefinedVector(upper.components) THEN
			nData := LEN(upper.components);
			i := 0;
			WHILE i < nData DO
				IF upper.components[i] = NIL THEN upper.components[i] := big END; INC(i)
			END;
			BugsIndex.Store(upper)
		END
	END StoreData;

	PROCEDURE StoreModel (nData: INTEGER);
		VAR
			model: BugsNames.Name;
	BEGIN
		model := BugsNames.New("model", 1); model.passByreference := TRUE;
		model.SetRange(0, nData); model.AllocateNodes;
		BugsIndex.Store(model)
	END StoreModel;

	PROCEDURE StoreTau;
		VAR
			tau, a, b, sigma, sigmaSq: BugsNames.Name;
	BEGIN
		tau := BugsNames.New("tau", 0); tau.passByreference := TRUE;
		tau.AllocateNodes;
		a := BugsNames.New("tau.a", 0); a.passByreference := TRUE;
		a.AllocateNodes;
		b := BugsNames.New("tau.b", 0); b.passByreference := TRUE;
		b.AllocateNodes;
		sigma := BugsNames.New("sigma", 0); sigma.passByreference := TRUE;
		sigma.AllocateNodes;
		sigmaSq := BugsNames.New("sigma.sq", 0); sigmaSq.passByreference := TRUE;
		sigmaSq.AllocateNodes;
		BugsIndex.Store(tau); BugsIndex.Store(a); BugsIndex.Store(b);
		BugsIndex.Store(sigma); BugsIndex.Store(sigmaSq)
	END StoreTau;

	PROCEDURE StoreNComp (n: INTEGER);
		VAR nComp: BugsNames.Name;
	BEGIN
		nComp := BugsNames.New("n.comp", 0); nComp.passByreference := TRUE;
		nComp.AllocateNodes; nComp.components[0] := GraphConstant.New(n);
		BugsIndex.Store(nComp)
	END StoreNComp;

	PROCEDURE StoreTheta (nInd, p: INTEGER);
		VAR
			theta: BugsNames.Name;
	BEGIN
		theta := BugsNames.New("theta", 2); theta.passByreference := TRUE;
		theta.SetRange(0, nInd); theta.SetRange(1, p);
		theta.AllocateNodes;
		BugsIndex.Store(theta)
	END StoreTheta;

	PROCEDURE StoreThetaMean (nInd, p: INTEGER);
		VAR
			thetaMean: BugsNames.Name;
	BEGIN
		thetaMean := BugsNames.New("theta.mean", 2); thetaMean.passByreference := TRUE;
		thetaMean.SetRange(0, nInd); thetaMean.SetRange(1, p);
		thetaMean.AllocateNodes;
		BugsIndex.Store(thetaMean)
	END StoreThetaMean;

	PROCEDURE StoreOmegaInv (p: INTEGER);
		VAR
			omegaInv, matrix, dof, omega: BugsNames.Name;
	BEGIN
		omegaInv := BugsNames.New("omega.inv", 2); omegaInv.passByreference := TRUE;
		omegaInv.SetRange(0, p); omegaInv.SetRange(1, p);
		omegaInv.AllocateNodes;
		matrix := BugsNames.New("omega.inv.matrix", 2); matrix.passByreference := TRUE;
		matrix.SetRange(0, p); matrix.SetRange(1, p);
		matrix.AllocateNodes;
		dof := BugsNames.New("omega.inv.dof", 0); dof.passByreference := TRUE;
		dof.AllocateNodes;
		omega := BugsNames.New("omega", 2); omega.passByreference := TRUE;
		omega.SetRange(0, p); omega.SetRange(1, p);
		omega.AllocateNodes;
		BugsIndex.Store(omegaInv); BugsIndex.Store(matrix); BugsIndex.Store(dof); BugsIndex.Store(omega)
	END StoreOmegaInv;

	PROCEDURE StoreMu (q: INTEGER);
		VAR
			mu, mean, prec: BugsNames.Name;
	BEGIN
		mu := BugsNames.New("mu", 1); mu.passByreference := TRUE;
		mu.SetRange(0, q); mu.AllocateNodes;
		mean := BugsNames.New("mu.prior.mean", 1); mean.passByreference := TRUE;
		mean.SetRange(0, q); mean.AllocateNodes;
		prec := BugsNames.New("mu.prior.precision", 2); prec.passByreference := TRUE;
		prec.SetRange(0, q); prec.SetRange(1, q);
		prec.AllocateNodes;
		BugsIndex.Store(mu); BugsIndex.Store(mean); BugsIndex.Store(prec)
	END StoreMu;

	PROCEDURE LenSet (set: SET): INTEGER;
		VAR len, i: INTEGER;
	BEGIN
		i := 0; len := 0; WHILE i <= MAX(SET) DO IF i IN set THEN INC(len) END; INC(i) END;
		RETURN len
	END LenSet;

	PROCEDURE StoreNodes* (nComp: INTEGER);
		VAR
			nData, nInd, p, q, i: INTEGER;
	BEGIN
		nData := PKBugsParse.NumData(); nInd := PKBugsParse.NumInd();
		ASSERT(PKBugsCovts.covariates # NIL, 25); p := LEN(PKBugsCovts.covariates);
		q := p; i := 0; WHILE i < p DO q := q + LenSet(PKBugsCovts.covariates[i]); INC(i) END;
		StoreModel(nData);
		StoreTau;
		StoreNComp(nComp);
		StoreTheta(nInd, p);
		StoreThetaMean(nInd, p);
		StoreOmegaInv(p);
		StoreMu(q)
	END StoreNodes;

	PROCEDURE SetConsts;
		VAR
			mu, name, omegaInv: BugsNames.Name;
			i, nInd, p, q: INTEGER;
	BEGIN
		omegaInv := BugsIndex.Find("omega.inv");
		p := omegaInv.slotSizes[0];
		mu := BugsIndex.Find("mu");
		q := mu.slotSizes[0];
		name := BugsNames.New("n.col", 0); name.passByreference := TRUE;
		name.AllocateNodes; name.components[0] := GraphConstant.New(12);
		nInd := PKBugsParse.NumInd();
		BugsIndex.Store(name);
		name := BugsNames.New("n.ind", 0); name.passByreference := TRUE;
		name.AllocateNodes; name.components[0] := GraphConstant.New(nInd);
		BugsIndex.Store(name);
		name := BugsNames.New("p", 0); name.passByreference := TRUE;
		name.AllocateNodes; name.components[0] := GraphConstant.New(p);
		BugsIndex.Store(name);
		name := BugsNames.New("q", 0); name.passByreference := TRUE;
		name.AllocateNodes; name.components[0] := GraphConstant.New(q);
		BugsIndex.Store(name);
		name := BugsNames.New("off.hist", 1); name.passByreference := TRUE;
		name.SetRange(0, nInd + 1); name.AllocateNodes;
		i := 0;
		WHILE i < nInd + 1 DO
			name.components[i] := GraphConstant.New(PKBugsParse.offHist[i]); INC(i)
		END;
		BugsIndex.Store(name);
		name := BugsNames.New("off.data", 1); name.passByreference := TRUE;
		name.SetRange(0, nInd + 1); name.AllocateNodes;
		i := 0;
		WHILE i < nInd + 1 DO
			name.components[i] := GraphConstant.New(PKBugsParse.offData[i]); INC(i)
		END;
		BugsIndex.Store(name);
	END SetConsts;

	PROCEDURE SetCov (cov: ARRAY OF CHAR; index: INTEGER);
		VAR
			i, j, nInd: INTEGER;
			name: BugsNames.Name;
	BEGIN
		IF BugsIndex.Find(cov) # NIL THEN RETURN END;
		i := 0; j := 0;
		WHILE j # index DO
			IF PKBugsCovts.covariateOK[i] THEN INC(j) END;
			INC(i)
		END;
		index := i;
		nInd := LEN(PKBugsCovts.data, 0);
		name := BugsNames.New(cov, 1); name.passByreference := TRUE;
		name.SetRange(0, nInd); name.AllocateNodes;
		i := 0;
		WHILE i < nInd DO
			name.components[i] := GraphConstant.New(PKBugsCovts.data[i, index]);
			INC(i)
		END;
		BugsIndex.Store(name)
	END SetCov;

	PROCEDURE SetCovariates;
		VAR
			i, j, nCov, nPar: INTEGER;
			name: ARRAY 80 OF CHAR;
	BEGIN
		nPar := LEN(PKBugsCovts.covariates);
		IF PKBugsCovts.covariateNames # NIL THEN
			nCov := LEN(PKBugsCovts.covariateNames)
		ELSE
			nCov := 0
		END;
		i := 0;
		WHILE i < nPar DO
			j := 0;
			WHILE j < nCov DO
				IF j IN PKBugsCovts.covariates[i] THEN
					ASSERT(PKBugsCovts.covariateOK[j], 25);
					name := PKBugsCovts.covariateNames[j]$;
					SetCov(name, j);
				END;
				INC(j)
			END;
			INC(i)
		END
	END SetCovariates;

	PROCEDURE SetPriorParams;
		VAR
			i, j, len, lenMu, nPar, offset: INTEGER;
			name: BugsNames.Name;
			cv, mean, value: REAL;
	BEGIN
		name := BugsIndex.Find("omega.inv.matrix"); ASSERT(name # NIL, 20);
		nPar := name.slotSizes[0];
		i := 0;
		WHILE i < nPar DO
			PKBugsPriors.GetSummary(i, mean, cv);
			j := 0;
			WHILE j < nPar DO
				IF j = i THEN
					value := nPar * Math.IntPower(Math.ArcSinh(cv / 100), 2);
				ELSE
					value := 0.0
				END;
				name.components[i * nPar + j] := GraphConstant.New(value);
				INC(j)
			END;
			INC(i)
		END;
		name := BugsIndex.Find("omega.inv.dof"); ASSERT(name # NIL, 20);
		name.components[0] := GraphConstant.New(nPar);
		name := BugsIndex.Find("mu.prior.mean"); ASSERT(name # NIL, 20);
		i := 0;
		offset := 0;
		WHILE i < nPar DO
			PKBugsPriors.GetSummary(i, mean, cv);
			value := Math.Ln(mean); 	(*	why log here?	*)
			name.components[offset] := GraphConstant.New(value);
			len := LenSet(PKBugsCovts.covariates[i]);
			j := 0;
			WHILE j < len DO
				INC(offset);
				name.components[offset] := GraphConstant.New(0.0);
				INC(j)
			END;
			INC(offset);
			INC(i)
		END;
		name := BugsIndex.Find("mu.prior.precision"); ASSERT(name # NIL, 20);
		lenMu := name.slotSizes[0];
		i := 0;
		WHILE i < lenMu DO
			j := 0;
			WHILE j < lenMu DO
				IF j = i THEN value := 0.0001 ELSE value := 0.0 END;
				name.components[i * lenMu + j] := GraphConstant.New(value);
				INC(j)
			END;
			INC(i)
		END;
		name := BugsIndex.Find("tau.a"); ASSERT(name # NIL, 20);
		name.components[0] := GraphConstant.New(0.001);
		name := BugsIndex.Find("tau.b"); ASSERT(name # NIL, 20);
		name.components[0] := GraphConstant.New(0.001)
	END SetPriorParams;

	PROCEDURE SetGraph*;
	BEGIN
		SetConsts; SetCovariates; SetPriorParams
	END SetGraph;

	PROCEDURE Reset*;
	BEGIN
		data := NIL; lower := NIL; upper := NIL
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
END PKBugsNodes.
