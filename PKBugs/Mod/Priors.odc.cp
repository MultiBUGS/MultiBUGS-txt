(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE PKBugsPriors;

		(* PKBugs Version 1.1 *)

	IMPORT
		BugsGraph, BugsIndex, BugsInterface, BugsNames,
		GraphLogical, GraphStochastic,
		MathMatrix, MathRandnum,
		UpdaterActions;

	TYPE
		Summary = RECORD
			mean, cv: REAL
		END;

	CONST
		defaultMean* = 1.0;
		defaultCV* = 20.0;

	VAR
		priors-: POINTER TO ARRAY OF Summary;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE InitPriors* (nParams: INTEGER);
		VAR
			i: INTEGER;
	BEGIN
		NEW(priors, nParams);
		i := 0; WHILE i < nParams DO priors[i].mean := defaultMean; priors[i].cv := defaultCV; INC(i) END
	END InitPriors;

	PROCEDURE StoreSummary* (parameter: INTEGER; mean, cv: REAL);
	BEGIN
		ASSERT(parameter < LEN(priors), 25);
		priors[parameter].mean := mean; priors[parameter].cv := cv
	END StoreSummary;

	PROCEDURE GetSummary* (parameter: INTEGER; OUT mean, cv: REAL);
	BEGIN
		ASSERT(parameter < LEN(priors), 25);
		mean := priors[parameter].mean; cv := priors[parameter].cv
	END GetSummary;

	PROCEDURE CheckPriors* (OUT res, par: INTEGER);
		VAR len, i: INTEGER;
	BEGIN
		res := 0; par := 0;
		ASSERT(priors # NIL, 25); len := LEN(priors);
		i := 0;
		WHILE i < len DO
			IF ~(priors[i].mean > 0) THEN res := 5602; par := i; RETURN END;
			(* population mean must be > 0 *)
			IF ~(priors[i].cv > 0) THEN res := 5604; par := i; RETURN END; 	(* inter-individual cv must be > 0 *)
			INC(i)
		END
	END CheckPriors;

	PROCEDURE InitializeMu (u: REAL);
		VAR
			len, i: INTEGER;
			value: REAL;
			mean, mu: BugsNames.Name;
			node: GraphStochastic.Node;
	BEGIN
		mean := BugsIndex.Find("mu.prior.mean"); ASSERT(mean # NIL, 25);
		mu := BugsIndex.Find("mu"); ASSERT(mu # NIL, 25);
		len := LEN(mean.components);
		i := 0;
		WHILE i < len DO
			value := u * mean.components[i].value;
			node := mu.components[i](GraphStochastic.Node);
			node.value := value;
			INCL(node.props, GraphStochastic.initialized);
			INC(i)
		END;
		i := 0;
		WHILE i < len DO
			node := mu.components[i](GraphStochastic.Node);
			GraphLogical.Evaluate(node.dependents);
			INC(i)
		END
	END InitializeMu;

	PROCEDURE InitializeOmegaInv;
		VAR
			nPar, i, j: INTEGER;
			matrix, dof, omegaInv: BugsNames.Name;
			node: GraphStochastic.Node;
			inv: POINTER TO ARRAY OF ARRAY OF REAL;
			value: REAL;
	BEGIN
		matrix := BugsIndex.Find("omega.inv.matrix"); ASSERT(matrix # NIL, 25);
		dof := BugsIndex.Find("omega.inv.dof"); ASSERT(dof # NIL, 25);
		omegaInv := BugsIndex.Find("omega.inv"); ASSERT(omegaInv # NIL, 25);
		ASSERT((matrix.slotSizes # NIL) & (LEN(matrix.slotSizes) = 2), 25);
		nPar := matrix.slotSizes[0];
		ASSERT((matrix.slotSizes[1] = nPar) & (LEN(matrix.components) = nPar * nPar), 25);
		NEW(inv, nPar, nPar);
		i := 0;
		WHILE i < nPar DO
			j := 0; WHILE j < nPar DO inv[i, j] := matrix.components[i * nPar + j].value; INC(j) END;
			INC(i)
		END;
		MathMatrix.Invert(inv, nPar);
		i := 0;
		WHILE i < nPar DO
			j := 0;
			WHILE j < nPar DO
				value := dof.components[0].value * inv[i, j];
				node := omegaInv.components[i * nPar + j](GraphStochastic.Node);
				node.value := value;
				INCL(node.props, GraphStochastic.initialized);
				INC(j)
			END;
			INC(i)
		END
	END InitializeOmegaInv;

	PROCEDURE InitializeTheta;
		VAR
			nInd, nPar, i, j: INTEGER;
			thetaMean, theta: BugsNames.Name;
			node: GraphStochastic.Node;
			value: REAL;
	BEGIN
		thetaMean := BugsIndex.Find("theta.mean"); ASSERT(thetaMean # NIL, 25);
		ASSERT((thetaMean.slotSizes # NIL) & (LEN(thetaMean.slotSizes) = 2), 25);
		nInd := thetaMean.slotSizes[0]; nPar := thetaMean.slotSizes[1];
		ASSERT(LEN(thetaMean.components) = nInd * nPar, 25);
		theta := BugsIndex.Find("theta"); ASSERT(theta # NIL, 25);
		i := 0;
		WHILE i < nInd DO
			j := 0;
			WHILE j < nPar DO
				value := thetaMean.components[i * nPar + j].value;
				node := theta.components[i * nPar + j](GraphStochastic.Node);
				node.value := value;
				INCL(node.props, GraphStochastic.initialized);
				INC(j)
			END;
			INC(i)
		END
	END InitializeTheta;

	PROCEDURE InitializeChain* (chain: INTEGER; u: REAL; OUT ok: BOOLEAN);
		VAR
			node: GraphStochastic.Node;
			name: BugsNames.Name;
		CONST
			fixFounder = FALSE;
	BEGIN
		BugsGraph.LoadInits(chain);
		GraphStochastic.LoadValues(chain);
		name := BugsIndex.Find("tau"); ASSERT(name # NIL, 25);
		node := name.components[0](GraphStochastic.Node);
		node.value := 1.0; INCL(node.props, GraphStochastic.initialized);
		(*	if more than one chain jigle initial values	*)
		InitializeMu(u);
		InitializeOmegaInv;
		InitializeTheta;
		BugsGraph.StoreInits(chain);
		GraphStochastic.StoreValues(chain);
		BugsInterface.GenerateInitsForChain(chain, fixFounder, ok)
	END InitializeChain;

	PROCEDURE Reset*;
	BEGIN
		priors := NIL
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
END PKBugsPriors.
