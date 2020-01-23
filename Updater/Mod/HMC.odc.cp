(* 	

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterHMC;

	

	IMPORT
		Math, 
		BugsIndex, BugsNames,
		GraphDeviance, GraphLogical, GraphMultivariate, GraphNodes, GraphRules, GraphStochastic,
		MathRandnum,
		UpdaterActions, UpdaterUpdaters;

	VAR
		mean, mean2, sigma: POINTER TO ARRAY OF POINTER TO ARRAY OF REAL;
		logJointOld, logJointNew, p: POINTER TO ARRAY OF REAL;
		data, stochastics: GraphStochastic.Vector;
		numStochastics: INTEGER;
		otherUpdaters: POINTER TO ARRAY OF INTEGER;
		first: POINTER TO ARRAY OF BOOLEAN;
		startIt-: INTEGER;
		setUp-: BOOLEAN;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	CONST
		nonHMC = {GraphRules.wishart, GraphRules.dirichlet, GraphRules.general};

	PROCEDURE EstimateMeans (iteration, warmUp, chain: INTEGER);
		VAR
			j, sampleSize, updater: INTEGER;
			res: SET;
			mapVal: REAL;
		CONST
			overRelax = FALSE;
	BEGIN
		UpdaterActions.Sample(overRelax, chain, res, updater);
		IF iteration >= warmUp DIV 2 THEN
			sampleSize := iteration - (warmUp DIV 2) + 1;
			j := 0;
			WHILE j < numStochastics DO
				mapVal := stochastics[j].Map();
				mean[chain, j] := mean[chain, j] + (mapVal - mean[chain, j]) / sampleSize;
				mean2[chain, j] := mean2[chain, j] + (mapVal * mapVal - mean2[chain, j]) / sampleSize;
				INC(j)
			END
		END
	END EstimateMeans;

	PROCEDURE HMC (numSteps, chain: INTEGER; eps: REAL);
		VAR
			diff, kineticNew, kineticOld, logAlpha, x: REAL;
			i, steps: INTEGER;
			reject: BOOLEAN;
			stoch: GraphStochastic.Node;

		PROCEDURE LogJoint (): REAL;
			VAR
				index, numNodes, num: INTEGER;
				dataDensity, logDetJac, priorDensity: REAL;
				stoch: GraphStochastic.Node;
		BEGIN
			priorDensity := 0.0;
			logDetJac := 0;
			index := 0;
			num := LEN(stochastics);
			WHILE index < num DO
				stoch := stochastics[index];
				WITH stoch: GraphMultivariate.Node DO
					IF stoch = stoch.Representative() THEN priorDensity := priorDensity + stoch.LogLikelihood() END
				ELSE
					priorDensity := priorDensity + stoch.LogLikelihood()
				END;
				(*	only include jacobian term for HMC type nodes	*)
				IF index < numStochastics THEN logDetJac := logDetJac + stoch.LogDetJacobian() END;
				INC(index)
			END;
			dataDensity := 0.0;
			IF data # NIL THEN
				numNodes := LEN(data);
				index := 0;
				WHILE index < numNodes DO
					IF GraphStochastic.data IN data[index].props THEN
						dataDensity := dataDensity + data[index].LogLikelihood()
					END;
					INC(index)
				END
			END;
			RETURN priorDensity + logDetJac + dataDensity
		END LogJoint;

	BEGIN
		(*	generate momentum and calculate kinetic energy	*)
		kineticOld := 0.0;
		i := 0;
		WHILE i < numStochastics DO
			p[i] := MathRandnum.StandardNormal();
			kineticOld := kineticOld + 0.5 * p[i] * p[i];
			INC(i)
		END;
		(*	calculate initial potential energy if it has changed or if non HMC nodes	*)
		IF first[chain] OR (otherUpdaters # NIL) THEN
			first[chain] := FALSE; logJointOld[chain] := LogJoint()
		END;
		GraphLogical.EvaluateAllDiffs;
		(*	leapfrog steps	*)
		i := 0;
		WHILE i < numStochastics DO
			stoch := stochastics[i];
			diff := stoch.DiffLogConditional();
			p[i] := p[i] + 0.5 * sigma[chain, i] * eps * diff;
			INC(i)
		END;
		steps := 0;
		WHILE steps < numSteps - 1 DO
			i := 0;
			WHILE i < numStochastics DO
				stoch := stochastics[i];
				x := stoch.Map(); x := x + sigma[chain, i] * eps * p[i]; stoch.InvMap(x);
				INC(i)
			END;
			GraphLogical.EvaluateAllDiffs;
			i := 0;
			WHILE i < numStochastics DO
				stoch := stochastics[i];
				diff := stoch.DiffLogConditional();
				p[i] := p[i] + sigma[chain, i] * eps * diff;
				INC(i)
			END;
			INC(steps);
		END;
		i := 0;
		WHILE i < numStochastics DO
			stoch := stochastics[i]; 
			x := stoch.Map(); x := x + sigma[chain, i] * eps * p[i]; stoch.InvMap(x);
			INC(i)
		END;
		GraphLogical.EvaluateAllDiffs;
		(*	calculate final  momentum and kinetic energy	*)
		kineticNew := 0.0;
		i := 0;
		WHILE i < numStochastics DO
			stoch := stochastics[i];
			diff := stoch.DiffLogConditional();
			p[i] := p[i] + 0.5 * sigma[chain, i] * eps * diff;
			kineticNew := kineticNew + 0.5 * p[i] * p[i];
			INC(i)
		END;
		(*	calculate final potentail energy	*)
		logJointNew[chain] := LogJoint();
		logAlpha := - (kineticNew - kineticOld) + logJointNew[chain] - logJointOld[chain];
		reject := logAlpha < Math.Ln(MathRandnum.Rand());
		IF reject THEN	(*	restore old values	*)
			GraphStochastic.LoadValues(chain);
			GraphLogical.LoadValues(chain)
		ELSE
			logJointOld[chain] := logJointNew[chain]
		END;
	END HMC;

	PROCEDURE Clear*;
	BEGIN
		stochastics := NIL;
		otherUpdaters := NIL;
		data := NIL;
		setUp := FALSE;
		first := NIL
	END Clear;

	PROCEDURE Setup* (numChains, start: INTEGER);
		VAR
			chain, i, j, k, numOtherUpdater, numOtherStoch, numUpdaters: INTEGER;
			vector: GraphStochastic.Vector;
			stoch: GraphStochastic.Node;
			u: UpdaterUpdaters.Updater;
			prior: GraphStochastic.Node;
			auxillary: GraphStochastic.List;
			name: BugsNames.Name;
			deviance: GraphNodes.Node;
	BEGIN
		IF setUp THEN RETURN ELSE setUp := TRUE END;
		startIt := start;
		name := BugsIndex.Find("deviance");
		deviance := name.components[0];
		data := GraphDeviance.DevianceTerms(deviance);
		otherUpdaters := NIL;
		stochastics := GraphStochastic.nodes;
		IF stochastics # NIL THEN numStochastics := LEN(stochastics) ELSE numStochastics := 0 END;
		numOtherUpdater := 0;
		(*	mark auxillary nodes prior to removal	*)
		auxillary := GraphStochastic.auxillary;
		WHILE auxillary # NIL DO
			stoch := auxillary.node; INCL(stoch.props, GraphNodes.mark); auxillary := auxillary.next
		END;
		(*	count conjugate wishart, conjugate dirichlet, non differentiable, integer and auxillary	*)
		numOtherStoch := 0;
		i := 0;
		WHILE i < numStochastics DO
			stoch := stochastics[i];
			IF (stoch.classConditional IN nonHMC) OR (GraphStochastic.integer IN stoch.props) OR
				(GraphNodes.mark IN stoch.props) THEN
				INC(numOtherStoch)
			END;
			INC(i)
		END;
		(*	place the non HMC nodes after the HMC nodes	*)
		IF numOtherStoch > 0 THEN
			NEW(vector, numStochastics);
			i := 0;
			j := 0;
			k := 0;
			WHILE i < numStochastics DO
				stoch := stochastics[i];
				IF (stoch.classConditional IN nonHMC) OR (GraphStochastic.integer IN stoch.props) OR
					(GraphNodes.mark IN stoch.props) THEN
					IF stoch = stoch.Representative() THEN INC(numOtherUpdater) END;
					vector[numStochastics - numOtherStoch + k] := stoch;
					INC(k)
				ELSE
					vector[j] := stoch;
					INC(j)
				END;
				INC(i)
			END;
			stochastics := vector;
			numStochastics := numStochastics - numOtherStoch;
			numUpdaters := UpdaterActions.NumberUpdaters();
			NEW(otherUpdaters, numOtherUpdater);
			i := 0;
			j := 0;
			WHILE i < numUpdaters DO
				u := UpdaterActions.updaters[0, i];
				prior := u.Prior(0);
				IF (prior.classConditional IN nonHMC) OR (GraphStochastic.integer IN prior.props) OR
					(GraphNodes.mark IN prior.props) THEN
					otherUpdaters[j] := i;
					INC(j)
				END;
				INC(i)
			END
		END;
		(*	clear mark on auxillary nodes	*)
		auxillary := GraphStochastic.auxillary;
		WHILE auxillary # NIL DO
			stoch := auxillary.node; EXCL(stoch.props, GraphNodes.mark); auxillary := auxillary.next
		END;
		NEW(mean, numChains);
		NEW(mean2, numChains);
		NEW(sigma, numChains);
		NEW(first, numChains);
		chain := 0;
		WHILE chain < numChains DO
			NEW(mean[chain], numStochastics);
			NEW(mean2[chain], numStochastics);
			NEW(sigma[chain], numStochastics);
			NEW(logJointOld, numChains);
			NEW(logJointNew, numChains);
			i := 0;
			WHILE i < numStochastics DO
				mean[chain, i] := 0.0;
				mean2[chain, i] := 0.0;
				INC(i)
			END;
			first[chain] := TRUE;
			INC(chain)
		END;
		NEW(p, numStochastics);
	END Setup;

	PROCEDURE Update* (numSteps, iteration, warmUp, chain: INTEGER; eps: REAL; 
				OUT res: SET; OUT updater: INTEGER);
		VAR
			j, numOtherUpdater: INTEGER;
		CONST
			overRelax = FALSE;
	BEGIN
		ASSERT(setUp, 21);
		res := {};
		ASSERT(iteration # 0, 33);
		IF iteration - startIt = warmUp THEN
			j := 0;
			WHILE j < numStochastics DO
				sigma[chain, j] := Math.Sqrt(mean2[chain, j] - mean[chain, j] * mean[chain, j]);
				INC(j)
			END
		END;
		IF iteration - startIt < warmUp THEN
			EstimateMeans(iteration, warmUp, chain);
		ELSE
			HMC(numSteps, chain, eps); 
			IF otherUpdaters # NIL THEN
				numOtherUpdater := LEN(otherUpdaters);
				j := 0;
				WHILE (j < numOtherUpdater) & (res = {}) DO;
					updater := otherUpdaters[j];
					UpdaterActions.updaters[chain, updater].Sample(overRelax, res);
					INC(j)
				END;
			END
		END
	END Update;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		Clear
	END Init;

BEGIN
	Init
END UpdaterHMC.

