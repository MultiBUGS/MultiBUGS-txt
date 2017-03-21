(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE PKBugsPriors;

		(* PKBugs Version 1.1 *)

	IMPORT
		BugsIndex, BugsInterface, BugsNames,
		GraphStochastic,
		MathMatrix, MathRandnum,
		UpdaterActions;

	TYPE
		Summary = RECORD
			mean, cv: REAL
		END;

	CONST
		defaultMean* = 1.0;
		defaultCV* = 10.0;

	VAR
		priors-: POINTER TO ARRAY OF Summary;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE InitPriors* (nParams: INTEGER);
		VAR
			i: INTEGER;
	BEGIN
		NEW(priors, nParams);
		i := 0;
		WHILE i < nParams DO
			priors[i].mean := defaultMean; priors[i].cv := defaultCV; INC(i)
		END
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

	PROCEDURE WriteMu (u: REAL);
		VAR
			len, i: INTEGER;
			value: REAL;
			mean, mu: BugsNames.Name;
			node: GraphStochastic.Node;
	BEGIN
		mean := BugsIndex.Find("mu.prior.mean"); ASSERT(mean # NIL, 25);
		mu := BugsIndex.Find("mu"); ASSERT(mean # NIL, 25);
		len := LEN(mean.components);
		i := 0;
		WHILE i < len DO
			value := u * mean.components[i].Value();
			node := mu.components[i](GraphStochastic.Node);
			node.SetValue(value);
			node.SetProps(node.props + {GraphStochastic.initialized});
			INC(i)
		END
	END WriteMu;

	PROCEDURE WriteOmegaInv;
		VAR
			nPar, i, j: INTEGER;
			matrix, dof, omegaInv: BugsNames.Name;
			node: GraphStochastic.Node;
			inv: POINTER TO ARRAY OF ARRAY OF REAL;
			value: REAL;
	BEGIN
		matrix := BugsIndex.Find("omega.inv.matrix"); ASSERT(matrix # NIL, 25);
		dof := BugsIndex.Find("omega.inv.dof"); ASSERT(dof # NIL, 25);
		omegaInv := BugsIndex.Find("omega.inv"); ASSERT(dof # NIL, 25);
		ASSERT((matrix.slotSizes # NIL) & (LEN(matrix.slotSizes) = 2), 25);
		nPar := matrix.slotSizes[0];
		ASSERT((matrix.slotSizes[1] = nPar) & (LEN(matrix.components) = nPar * nPar), 25);
		NEW(inv, nPar, nPar);
		i := 0;
		WHILE i < nPar DO
			j := 0;
			WHILE j < nPar DO
				inv[i, j] := matrix.components[i * nPar + j].Value(); INC(j)
			END;
			INC(i)
		END;
		MathMatrix.Invert(inv, nPar);
		i := 0;
		WHILE i < nPar DO
			j := 0;
			WHILE j < nPar DO
				value := dof.components[0].Value() * inv[i, j];
				node := omegaInv.components[i * nPar + j](GraphStochastic.Node);
				node.SetValue(value);
				node.SetProps(node.props + {GraphStochastic.initialized});
				INC(j)
			END;
			INC(i)
		END
	END WriteOmegaInv;

	PROCEDURE WriteTheta;
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
		theta := BugsIndex.Find("theta"); ASSERT(thetaMean # NIL, 25);
		i := 0;
		WHILE i < nInd DO
			j := 0;
			WHILE j < nPar DO
				value := thetaMean.components[i * nPar + j].Value();
				node := theta.components[i * nPar + j](GraphStochastic.Node);
				node.SetValue(value);
				node.SetProps(node.props + {GraphStochastic.initialized});
				INC(j)
			END;
			INC(i)
		END
	END WriteTheta;

	PROCEDURE GenerateInitsForChain* (chain: INTEGER; u: REAL);
		VAR
			node: GraphStochastic.Node;
			name: BugsNames.Name;
			ok: BOOLEAN;
		CONST
			fixFounder = FALSE;
	BEGIN
		UpdaterActions.LoadSamples(chain);
		name := BugsIndex.Find("tau"); ASSERT(name # NIL, 25);
		node := name.components[0](GraphStochastic.Node);
		node.SetValue(1.0);
		node.SetProps(node.props + {GraphStochastic.initialized});
		(*	if more than one chain jigle initial values	*)
		WriteMu(u);
		WriteOmegaInv;
		WriteTheta;
		UpdaterActions.StoreSamples(chain);
		BugsInterface.GenerateInitsForChain(chain, fixFounder, ok);
		UpdaterActions.StoreSamples(chain);
		ASSERT(ok, 100)
	END GenerateInitsForChain;

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
