(* 	

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterHMC;

	

	IMPORT
		Math,
		BugsIndex, BugsNames, 
		GraphDeviance, GraphMultivariate, GraphNodes, GraphRules, GraphStochastic,
		MathRandnum,
		UpdaterActions, UpdaterUpdaters;

	VAR
		mean, mean2, sigma: POINTER TO ARRAY OF POINTER TO ARRAY OF REAL;
		oldVals, p: POINTER TO ARRAY OF REAL;
		data, stochastics: GraphStochastic.Vector;
		numStochastics: INTEGER;
		otherUpdaters: POINTER TO ARRAY OF INTEGER;
		setUp-: BOOLEAN;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	CONST
		nonHMC = {GraphRules.wishart, GraphRules.dirichlet, GraphRules.general};

	PROCEDURE EstimateMeans (iteration, warmUpPeriod, chain: INTEGER);
		VAR
			j, sampleSize: INTEGER;
			res: SET;
			updater: UpdaterUpdaters.Updater;
			mapVal: REAL;
		CONST
			overRelax = FALSE;
	BEGIN
		UpdaterActions.Sample(overRelax, chain, res, updater);
		IF iteration >= warmUpPeriod DIV 2 THEN
			sampleSize := iteration - (warmUpPeriod DIV 2) + 1;
			j := 0;
			WHILE j < numStochastics DO
				mapVal := stochastics[j].Map();
				mean[chain, j] := mean[chain, j] + (mapVal - mean[chain, j]) / sampleSize;
				mean2[chain, j] := mean2[chain, j] + (mapVal * mapVal - mean2[chain, j]) / sampleSize;
				INC(j)
			END
		END
	END EstimateMeans;

	PROCEDURE LeapFrog (numSteps, chain: INTEGER; eps: REAL; OUT reject: BOOLEAN);
		VAR
			diff, kineticNew, kineticOld, logAlpha, logDataLikeNew, logDataLikeOld,
			logDetJacobianNew, logDetJacobianOld,
			logStochasticLikeNew, logStochasticLikeOld, x: REAL;
			i, steps: INTEGER;
			stoch: GraphStochastic.Node;

		PROCEDURE LogLikelihood (nodes: GraphStochastic.Vector): REAL;
			VAR
				index, numNodes: INTEGER;
				likelihood: REAL;
		BEGIN
			likelihood := 0.0;
			IF nodes # NIL THEN
				numNodes := LEN(nodes);
				index := 0;
				WHILE index < numNodes DO
					IF GraphStochastic.data IN nodes[index].props THEN
						likelihood := likelihood + nodes[index].LogLikelihood()
					END;
					INC(index)
				END
			END;
			RETURN likelihood
		END LogLikelihood;

	BEGIN
		(*	generate momentum and calculate kinetic energy and jacobeans, store old values	*)
		kineticOld := 0.0;
		logDetJacobianOld := 0.0;
		i := 0;
		WHILE i < numStochastics DO
			stoch := stochastics[i];
			p[i] := MathRandnum.StandardNormal();
			oldVals[i] := stoch.value;
			kineticOld := kineticOld + 0.5 * p[i] * p[i];
			logDetJacobianOld := logDetJacobianOld + stoch.LogDetJacobian();
			INC(i)
		END;
		(*	calculate potential energy	*)
		logDataLikeOld := LogLikelihood(data);
		logStochasticLikeOld := 0.0;
		i := 0;
		WHILE i < LEN(stochastics) DO
			stoch := stochastics[i];
			WITH stoch: GraphMultivariate.Node DO
				IF stoch.index = 0 THEN logStochasticLikeOld := logStochasticLikeOld + stoch.LogLikelihood() END
			ELSE
				logStochasticLikeOld := logStochasticLikeOld + stoch.LogLikelihood()
			END;
			INC(i)
		END;
		(*	leapfrog steps	*)
		i := 0;
		WHILE i < numStochastics DO
			stoch := stochastics[i]; diff := stoch.DiffLogConditional(); p[i] := p[i] + 0.5 * sigma[chain, i] * eps * diff;
			INC(i)
		END;
		steps := 0;
		WHILE steps < numSteps - 1 DO
			i := 0;
			WHILE i < numStochastics DO
				stoch := stochastics[i]; x := stoch.Map(); x := x + sigma[chain, i] * eps * p[i]; stoch.InvMap(x);
				(*IF ABS(eps * p[i]) > 10 THEN reject := TRUE; res := {}; RETURN END;*)
				INC(i)
			END;
			i := 0;
			WHILE i < numStochastics DO
				stoch := stochastics[i]; diff := stoch.DiffLogConditional(); p[i] := p[i] + sigma[chain, i] * eps * diff;
				INC(i)
			END;
			INC(steps);
		END;
		i := 0;
		WHILE i < numStochastics DO
			stoch := stochastics[i]; x := stoch.Map(); x := x + sigma[chain, i] * eps * p[i]; stoch.InvMap(x);
			INC(i)
		END;
		(*	calculate final  momentum and kinetic energy	*)
		kineticNew := 0.0;
		logDetJacobianNew := 0.0;
		i := 0;
		WHILE i < numStochastics DO
			stoch := stochastics[i]; diff := stoch.DiffLogConditional(); p[i] := p[i] + 0.5 * sigma[chain, i] * eps * diff;
			kineticNew := kineticNew + 0.5 * p[i] * p[i];
			logDetJacobianNew := logDetJacobianNew + stoch.LogDetJacobian();
			INC(i)
		END;
		(*	calculate final potentail energy	*)
		logDataLikeNew := LogLikelihood(data);
		logStochasticLikeNew := 0.0;
		i := 0;
		WHILE i < LEN(stochastics) DO
			stoch := stochastics[i];
			WITH stoch: GraphMultivariate.Node DO
				IF stoch.index = 0 THEN logStochasticLikeNew := logStochasticLikeNew + stoch.LogLikelihood() END
			ELSE
				logStochasticLikeNew := logStochasticLikeNew + stoch.LogLikelihood()
			END;
			INC(i)
		END;
		logAlpha := - (kineticNew - kineticOld) + logDataLikeNew - logDataLikeOld + 
		logStochasticLikeNew - logStochasticLikeOld + logDetJacobianNew - logDetJacobianOld;
		reject := logAlpha < Math.Ln(MathRandnum.Rand());
		IF reject THEN
			i := 0;
			WHILE i < numStochastics DO
				stochastics[i].SetValue(oldVals[i]);
				INC(i)
			END;
		END;
	END LeapFrog;

	PROCEDURE Clear*;
	BEGIN
		stochastics := NIL;
		otherUpdaters := NIL;
		data := NIL;
		setUp := FALSE
	END Clear;

	PROCEDURE Setup* (numChains: INTEGER);
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
		name := BugsIndex.Find("deviance");
		deviance := name.components[0];
		data := GraphDeviance.DevianceTerms(deviance);
		otherUpdaters := NIL;
		stochastics := GraphStochastic.stochastics;
		numStochastics := GraphStochastic.numStochastics;
		numOtherUpdater := 0;
		(*	mark auxillary nodes prior to removal	*)
		auxillary := GraphStochastic.auxillary;
		WHILE auxillary # NIL DO
			stoch := auxillary.node;
			stoch.SetProps(stoch.props + {GraphNodes.mark});
			auxillary := auxillary.next
		END;
		(*	remove conjugate wishart and conjugate dirichlet etc	*)
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
		auxillary := GraphStochastic.auxillary;
		WHILE auxillary # NIL DO
			stoch := auxillary.node;
			stoch.SetProps(stoch.props - {GraphNodes.mark});
			auxillary := auxillary.next
		END;
		NEW(mean, numChains);
		NEW(mean2, numChains);
		NEW(sigma, numChains);
		chain := 0;
		WHILE chain < numChains DO
			NEW(mean[chain], numStochastics);
			NEW(mean2[chain], numStochastics);
			NEW(sigma[chain], numStochastics);
			i := 0;
			WHILE i < numStochastics DO
				mean[chain, i] := 0.0;
				mean2[chain, i] := 0.0;
				INC(i)
			END;
			INC(chain)
		END;
		NEW(oldVals, numStochastics);
		NEW(p, numStochastics);
	END Setup;

	PROCEDURE Update* (numSteps, iteration, warmUpPeriod, chain: INTEGER; eps: REAL;
	OUT reject: BOOLEAN);
		VAR
			j, numOtherUpdater: INTEGER;
			res: SET;
		CONST
			overRelax = FALSE;
	BEGIN
		ASSERT(setUp, 21);
		reject := FALSE;
		res := {};
		ASSERT(iteration # 0, 33);
		IF iteration = warmUpPeriod THEN
			j := 0;
			WHILE j < numStochastics DO
				sigma[chain, j] := Math.Sqrt(mean2[chain, j] - mean[chain, j] * mean[chain, j]);
				INC(j)
			END
		END;
		IF iteration < warmUpPeriod THEN
			EstimateMeans(iteration, warmUpPeriod, chain)
		ELSE
			LeapFrog(numSteps, chain, eps, reject);
			IF otherUpdaters # NIL THEN
				numOtherUpdater := LEN(otherUpdaters);
				j := 0;
				WHILE j < numOtherUpdater DO;
					UpdaterActions.updaters[chain, otherUpdaters[j]].Sample(overRelax, res);
					INC(j)
				END;
			END
		END;
		ASSERT(res = {}, 77);
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

