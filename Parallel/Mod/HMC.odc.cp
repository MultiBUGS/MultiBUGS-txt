(* 	

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE ParallelHMC;

	

	IMPORT
		MPIworker,
		Math,
		GraphMultivariate, GraphStochastic,
		MathRandnum,
		ParallelActions, ParallelRandnum;

	VAR
		mean, mean2, sigma, oldVals, p: POINTER TO ARRAY OF REAL;
		data, stochastics: GraphStochastic.Vector;
		numStochastics, commSize, rank: INTEGER;
		setUp: BOOLEAN;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	CONST
		distributed = GraphStochastic.distributed;

	PROCEDURE EstimateMeans (seperable: BOOLEAN; iteration, warmUpPeriod: INTEGER);
		VAR
			j, sampleSize: INTEGER;
			res: SET;
			mapVal: REAL;
		CONST
			overRelax = FALSE;
	BEGIN
		ParallelActions.Update(seperable, overRelax, res);
		IF iteration >= warmUpPeriod DIV 2 THEN
			sampleSize := iteration - (warmUpPeriod DIV 2) + 1;
			j := 0;
			WHILE j < numStochastics DO
				mapVal := stochastics[j].Map();
				mean[j] := mean[j] + (mapVal - mean[j]) / sampleSize;
				mean2[j] := mean2[j] + (mapVal * mapVal - mean2[j]) / sampleSize;
				INC(j)
			END;
		END
	END EstimateMeans;

	PROCEDURE LeapFrog (seperable: BOOLEAN; numSteps: INTEGER; eps: REAL; OUT reject: BOOLEAN);
		VAR
			diff, kineticNew, kineticOld, logAlpha, logDLikeNew, logDLikeOld,
			logDetJacobianNew, logDetJacobianOld,
			logSLikeNew, logSLikeOld, x: REAL;
			i, steps: INTEGER;
			stoch: GraphStochastic.Node;

		PROCEDURE Allgather;
			VAR
				j, k: INTEGER;
				values, globalValues: POINTER TO ARRAY OF REAL;
				globalStochs: POINTER TO ARRAY OF GraphStochastic.Vector;
		BEGIN
			values := ParallelActions.values;
			globalValues := ParallelActions.globalValues;
			globalStochs := ParallelActions.globalStochs;
			j := 0; WHILE j < numStochastics DO values[j] := stochastics[j].value; INC(j) END;
			MPIworker.AllGather(values, numStochastics, globalValues);
			k := 0;
			WHILE k < commSize DO
				IF k # rank THEN
					j := 0;
					WHILE j < numStochastics DO
						globalStochs[k, j].SetValue(globalValues[k * numStochastics + j]);
						INC(j)
					END
				END;
				INC(k)
			END
		END Allgather;

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
		(*	generate momentum and calculate energy and jacobeans, store old values	*)
		kineticOld := 0.0;
		logDetJacobianOld := 0.0;
		logSLikeOld := 0.0;
		logDLikeOld := LogLikelihood(data);
		i := 0;
		WHILE i < numStochastics DO
			stoch := stochastics[i];
			oldVals[i] := stoch.value;
			IF distributed IN stoch.props THEN
				IF commSize > 1 THEN ParallelRandnum.UseSameStream END;
				p[i] := MathRandnum.StandardNormal();
				IF commSize > 1 THEN ParallelRandnum.UsePrivateStream END;
			ELSE
				p[i] := MathRandnum.StandardNormal();
			END;
			IF ~(distributed IN stoch.props) OR (rank = 0) THEN
				logDetJacobianOld := logDetJacobianOld + stoch.LogDetJacobian();
				WITH stoch: GraphMultivariate.Node DO
					IF stoch.index = 0 THEN logSLikeOld := logSLikeOld + stoch.LogLikelihood() END
				ELSE
					logSLikeOld := logSLikeOld + stoch.LogLikelihood()
				END;
				kineticOld := kineticOld + 0.5 * p[i] * p[i]
			END;
			INC(i)
		END;
		(*	leapfrog steps	*)
		i := 0;
		WHILE i < numStochastics DO
			stoch := stochastics[i]; diff := stoch.DiffLogConditional(); p[i] := p[i] + 0.5 * sigma[i] * eps * diff;
			INC(i)
		END;
		steps := 0;
		WHILE steps < numSteps - 1 DO
			i := 0;
			WHILE i < numStochastics DO
				stoch := stochastics[i]; x := stoch.Map(); x := x + sigma[i] * eps * p[i]; stoch.InvMap(x);
				(*IF ABS(eps * p[i]) > 10 THEN reject := TRUE; res := {}; RETURN END;*)
				INC(i)
			END;
			IF ~seperable THEN Allgather END;
			i := 0;
			WHILE i < numStochastics DO
				stoch := stochastics[i]; diff := stoch.DiffLogConditional(); p[i] := p[i] + sigma[i] * eps * diff;
				INC(i)
			END;
			INC(steps);
		END;
		i := 0;
		WHILE i < numStochastics DO
			stoch := stochastics[i]; x := stoch.Map(); x := x + sigma[i] * eps * p[i]; stoch.InvMap(x);
			INC(i)
		END;
		IF ~seperable THEN Allgather END;
		(*	calculate final  momentum and energy	*)
		kineticNew := 0.0;
		logDetJacobianNew := 0.0;
		logSLikeNew := 0.0;
		logDLikeNew := LogLikelihood(data);
		i := 0;
		WHILE i < numStochastics DO
			stoch := stochastics[i]; diff := stoch.DiffLogConditional(); p[i] := p[i] + 0.5 * sigma[i] * eps * diff;
			IF ~(distributed IN stoch.props) OR (rank = 0) THEN
				logDetJacobianNew := logDetJacobianNew + stoch.LogDetJacobian();
				WITH stoch: GraphMultivariate.Node DO
					IF stoch.index = 0 THEN logSLikeNew := logSLikeNew + stoch.LogLikelihood() END
				ELSE
					logSLikeNew := logSLikeNew + stoch.LogLikelihood()
				END;
				kineticNew := kineticNew + 0.5 * p[i] * p[i]
			END;
			INC(i)
		END;
		logAlpha := - (kineticNew - kineticOld) + logDLikeNew - logDLikeOld + 
		logSLikeNew - logSLikeOld + logDetJacobianNew - logDetJacobianOld;
		logAlpha := MPIworker.SumReal(logAlpha);
		IF commSize > 1 THEN ParallelRandnum.UseSameStream END;
		reject := logAlpha < Math.Ln(MathRandnum.Rand());
		IF commSize > 1 THEN ParallelRandnum.UsePrivateStream END;
		IF reject THEN
			i := 0;
			WHILE i < numStochastics DO
				stochastics[i].SetValue(oldVals[i]);
				INC(i)
			END;
		END;
		IF seperable & ~reject & (commSize > 1) THEN Allgather END
	END LeapFrog;

	PROCEDURE Clear*;
	BEGIN
		stochastics := NIL;
		data := NIL;
		setUp := FALSE
	END Clear;

	PROCEDURE Setup*;
		VAR
			i: INTEGER;
	BEGIN
		IF setUp THEN RETURN ELSE setUp := TRUE END;
		commSize := MPIworker.commSize;
		rank := MPIworker.rank;
		data := ParallelActions.observations;
		stochastics := ParallelActions.globalStochs[rank];
		ASSERT(stochastics # NIL, 77);
		numStochastics := LEN(stochastics);
		NEW(mean, numStochastics);
		NEW(mean2, numStochastics);
		NEW(sigma, numStochastics);
		i := 0;
		WHILE i < numStochastics DO
			mean[i] := 0.0;
			mean2[i] := 0.0;
			INC(i)
		END;
		NEW(oldVals, numStochastics);
		NEW(p, numStochastics)
	END Setup;

	PROCEDURE Update* (seperable: BOOLEAN; numSteps, iteration, warmUpPeriod: INTEGER; eps: REAL;
	OUT reject: BOOLEAN);
		VAR
			i: INTEGER;
	BEGIN
		INC(iteration); (*	try this	*)
		ASSERT(setUp, 21);
		reject := FALSE;
		IF iteration = warmUpPeriod THEN
			i := 0;
			WHILE i < numStochastics DO
				sigma[i] := Math.Sqrt(mean2[i] - mean[i] * mean[i]);
				INC(i)
			END
		END;
		IF iteration < warmUpPeriod THEN
			EstimateMeans(seperable, iteration, warmUpPeriod)
		ELSE
			LeapFrog(seperable, numSteps, eps, reject)
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
END ParallelHMC.

