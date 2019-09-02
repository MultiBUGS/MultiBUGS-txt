(* 	

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE ParallelHMC;

	

	IMPORT
		MPIworker,
		Math, Services, Strings,
		GraphLogical, GraphMultivariate, GraphRules, GraphStochastic,
		MathRandnum,
		ParallelActions, ParallelRandnum,
		UpdaterUpdaters;

	VAR
		mean, mean2, sigma, p: POINTER TO ARRAY OF REAL;
		notHMC: POINTER TO ARRAY OF BOOLEAN;
		data, stochastics: GraphStochastic.Vector;
		numStochastics, commSize, rank: INTEGER;
		logJointOld, logJointNew: REAL;
		startIt-: INTEGER;
		setUp: BOOLEAN;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	CONST
		nonHMC = {GraphRules.wishart, GraphRules.dirichlet, GraphRules.general};

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
			diff, kineticNew, kineticOld, logAlpha, x: REAL;
			i, numUpdaters, steps: INTEGER;
			stoch: GraphStochastic.Node;
			updaters: UpdaterUpdaters.Vector;
			res: SET;

		CONST
			overRelax = FALSE;

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
				j := 0;
				WHILE j < numStochastics DO
					globalStochs[k, j].value := globalValues[k * numStochastics + j];
					INC(j)
				END;
				INC(k)
			END
		END Allgather;

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
				IF ~(GraphStochastic.distributed IN stoch.props) OR (rank = 0) THEN
					WITH stoch: GraphMultivariate.Node DO
						IF stoch = stoch.Representative() THEN
							priorDensity := priorDensity + stoch.LogLikelihood()
						END
					ELSE
						priorDensity := priorDensity + stoch.LogLikelihood()
					END;
					IF index < numStochastics THEN logDetJac := logDetJac + stoch.LogDetJacobian() END
				END;
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
		(*	generate momentum and calculate kinetic energy and jacobeans	*)
		kineticOld := 0.0;
		i := 0;
		WHILE i < numStochastics DO
			stoch := stochastics[i];
			IF GraphStochastic.distributed IN stoch.props THEN
				ParallelRandnum.UseSameStream;
				p[i] := MathRandnum.StandardNormal();
				ParallelRandnum.UsePrivateStream;
				IF rank = 0 THEN kineticOld := kineticOld + 0.5 * p[i] * p[i] END
			ELSE
				p[i] := MathRandnum.StandardNormal();
				kineticOld := kineticOld + 0.5 * p[i] * p[i];
			END;
			INC(i)
		END;
		(*	calculate initial potential energy if it has changed	*)
		logJointOld := LogJoint();
		GraphLogical.EvaluateAllDiffs;
		(*	initial momentum step	*)
		i := 0;
		WHILE i < numStochastics DO
			stoch := stochastics[i];
			IF ~(GraphStochastic.mark IN stoch.props) THEN
				diff := stoch.DiffLogConditional();
				p[i] := p[i] + 0.5 * sigma[i] * eps * diff
			END;
			INC(i)
		END;
		(*	leapfrog steps	*)
		steps := 0;
		WHILE steps < numSteps - 1 DO
			i := 0;
			WHILE i < numStochastics DO
				stoch := stochastics[i];
				IF ~(GraphStochastic.mark IN stoch.props) THEN
					x := stoch.Map(); x := x + sigma[i] * eps * p[i]; stoch.InvMap(x)
				END;
				(*IF ABS(eps * p[i]) > 10 THEN reject := TRUE; res := {}; RETURN END;*)
				INC(i)
			END;
			IF ~seperable & (commSize > 1) THEN Allgather END;
			GraphLogical.EvaluateAllDiffs;
			i := 0;
			WHILE i < numStochastics DO
				stoch := stochastics[i];
				IF ~(GraphStochastic.mark IN stoch.props) THEN
					diff := stoch.DiffLogConditional();
					p[i] := p[i] + sigma[i] * eps * diff
				END;
				INC(i)
			END;
			INC(steps);
		END;
		(*	final position step	*)
		i := 0;
		WHILE i < numStochastics DO
			IF ~(GraphStochastic.mark IN stoch.props) THEN
				stoch := stochastics[i]; x := stoch.Map(); x := x + sigma[i] * eps * p[i]; stoch.InvMap(x)
			END;
			INC(i)
		END;
		IF ~seperable & (commSize > 1) THEN Allgather END;
		GraphLogical.EvaluateAllDiffs;
		(*	calculate final  momentum and kinetic energy	*)
		kineticNew := 0.0;
		i := 0;
		WHILE i < numStochastics DO
			stoch := stochastics[i];
			IF ~(GraphStochastic.mark IN stoch.props) THEN
				diff := stoch.DiffLogConditional();
				p[i] := p[i] + 0.5 * sigma[i] * eps * diff
			END;
			IF GraphStochastic.distributed IN stoch.props THEN
				IF rank = 0 THEN kineticNew := kineticNew + 0.5 * p[i] * p[i] END
			ELSE
				kineticNew := kineticNew + 0.5 * p[i] * p[i];
			END;
			INC(i)
		END;
		(*	calculate final potentail energy	*)
		logJointNew := LogJoint();
		logAlpha := - (kineticNew - kineticOld) + logJointNew - logJointOld;
		logAlpha := MPIworker.SumReal(logAlpha);
		ParallelRandnum.UseSameStream;
		reject := logAlpha < Math.Ln(MathRandnum.Rand());
		ParallelRandnum.UsePrivateStream;
		IF reject THEN	(*	restore old values	*)
			GraphStochastic.LoadValues(0);
			GraphLogical.LoadValues(0);
		ELSE
			IF seperable & (commSize > 1) THEN Allgather END;
			GraphStochastic.StoreValues(0);
			GraphLogical.StoreValues(0)
		END;
		(*	samples non HMC parameters	*)
		IF notHMC # NIL THEN
			updaters := ParallelActions.updaters;
			numUpdaters := LEN(updaters);
			i := 0;
			WHILE i < numUpdaters DO
				IF notHMC[i] THEN updaters[i].Sample(overRelax, res) END;
				INC(i)
			END
		END
	END LeapFrog;

	PROCEDURE Clear*;
	BEGIN
		stochastics := NIL;
		data := NIL;
		setUp := FALSE;
	END Clear;

	PROCEDURE Setup* (start: INTEGER);
		VAR
			i, j, numUpdaters, pos, size: INTEGER;
			stoch: GraphStochastic.Node;
			updaters: UpdaterUpdaters.Vector;
			u: UpdaterUpdaters.Updater;
			name: ARRAY 64 OF CHAR;
			allHMC: BOOLEAN;
	BEGIN
		IF setUp THEN RETURN ELSE setUp := TRUE END;
		startIt := start;
		commSize := MPIworker.commSize;
		rank := MPIworker.rank;
		data := ParallelActions.observations;
		stochastics := ParallelActions.globalStochs[rank];
		ASSERT(stochastics # NIL, 77);
		numStochastics := LEN(stochastics);
		NEW(mean, numStochastics);
		NEW(mean2, numStochastics);
		NEW(sigma, numStochastics);
		allHMC := TRUE;
		i := 0;
		WHILE i < numStochastics DO
			mean[i] := 0.0;
			mean2[i] := 0.0;
			stoch := stochastics[i];
			IF (stoch.classConditional IN nonHMC) OR (GraphStochastic.integer IN stoch.props) THEN
				INCL(stoch.props, GraphStochastic.mark); allHMC := FALSE
			END;
			INC(i)
		END;
		updaters := ParallelActions.updaters;
		numUpdaters := LEN(updaters);
		i := 0;
		WHILE i < numUpdaters DO
			u := updaters[i];
			u.Install(name);
			Strings.Find(name, "UpdaterAuxillary", 0, pos);
			IF pos # - 1 THEN
				size := u.Size();
				j := 0;
				WHILE j < size DO
					stoch := u.Prior(j);
					INCL(stoch.props, GraphStochastic.mark); allHMC := FALSE;
					INC(j)
				END
			END;
			INC(i)
		END;
		notHMC := NIL;
		IF ~allHMC THEN
			NEW(notHMC, numUpdaters);
			i := 0;
			WHILE i < numUpdaters DO
				notHMC[i] := FALSE;
				u := updaters[i];
				size := u.Size();
				j := 0;
				WHILE j < size DO
					stoch := u.Prior(j);
					IF GraphStochastic.mark IN stoch.props THEN notHMC[i] := TRUE END;
					INC(j)
				END;
				INC(i)
			END
		END;
		NEW(p, numStochastics)
	END Setup;

	PROCEDURE Update* (seperable: BOOLEAN; numSteps, iteration, warmUpPeriod: INTEGER; eps: REAL;
	OUT reject: BOOLEAN);
		VAR
			i: INTEGER;
	BEGIN
		ASSERT(setUp, 21);
		INC(iteration);
		reject := FALSE;
		IF iteration - startIt = warmUpPeriod THEN
			i := 0;
			WHILE i < numStochastics DO
				sigma[i] := Math.Sqrt(mean2[i] - mean[i] * mean[i]);
				INC(i)
			END;
			GraphLogical.EvaluateAllDiffs;
			GraphStochastic.StoreValues(0);
			GraphLogical.StoreValues(0)
		END;
		IF iteration - startIt < warmUpPeriod THEN
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

