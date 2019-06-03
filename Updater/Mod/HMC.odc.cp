MODULE UpdaterHMC;

	

	IMPORT
		Dialog, Math, Services, Strings,
		BugsFiles, BugsIndex, BugsInterface, BugsNames, BugsRandnum,
		GraphDeviance, GraphMultivariate, GraphNodes, GraphStochastic,
		MathRandnum,
		MonitorMonitors, MonitorSummary,
		UpdaterActions, UpdaterUpdaters;

	VAR
		mean, mean2: POINTER TO ARRAY OF POINTER TO ARRAY OF REAL;
		oldVals, p, sigma: POINTER TO ARRAY OF REAL;
		data: GraphStochastic.Vector;
		iterations*, L*, warmUpPeriod*: INTEGER;
		delta*: REAL;

	PROCEDURE LogLikelihood (nodes: GraphStochastic.Vector): REAL;
		VAR
			i, numNodes: INTEGER;
			likeLihood: REAL;
	BEGIN
		likeLihood := 0.0;
		IF nodes # NIL THEN
			numNodes := LEN(nodes);
			i := 0;
			WHILE i < numNodes DO
				IF GraphStochastic.data IN nodes[i].props THEN
					likeLihood := likeLihood + nodes[i].LogLikelihood()
				END;
				INC(i)
			END
		END;
		RETURN likeLihood
	END LogLikelihood;

	PROCEDURE WarmUp*;
		VAR
			stochastics: GraphStochastic.Vector;
			burnIn, chain, i, j, numChains, numStochastics, sampleSize: INTEGER;
			res: SET;
			updater: UpdaterUpdaters.Updater;
			mapVal, mu, skew, exKur, lower, median, upper, x: REAL;
			name: BugsNames.Name;
			deviance: GraphNodes.Node;
			time: LONGINT;
			string, number: Dialog.String;
		CONST
			overRelax = FALSE;
	BEGIN
		time := Services.Ticks();
		numChains := UpdaterActions.NumberChains();
		stochastics := GraphStochastic.stochastics;
		numStochastics := GraphStochastic.numStochastics;
		NEW(mean, numChains);
		NEW(mean2, numChains);
		chain := 0;
		WHILE chain < numChains DO
			NEW(mean[chain], numStochastics);
			NEW(mean2[chain], numStochastics);
			INC(chain)
		END;
		NEW(oldVals, numStochastics);
		NEW(p, numStochastics);
		NEW(sigma, numStochastics);
		name := BugsIndex.Find("deviance");
		deviance := name.components[0];
		data := GraphDeviance.DevianceTerms(deviance);
		burnIn := warmUpPeriod DIV 2;
		i := 0;
		WHILE i < burnIn DO
			chain := 0;
			WHILE chain < numChains DO
				MathRandnum.SetGenerator(BugsRandnum.generators[chain]);
				UpdaterActions.LoadSamples(chain);
				UpdaterActions.Sample(overRelax, chain, res, updater);
				UpdaterActions.StoreSamples(chain);
				INC(chain)
			END;
			INC(i)
		END;
		chain := 0;
		WHILE chain < numChains DO
			i := 0;
			WHILE i < numStochastics DO
				mean[chain, i] := 0.0;
				mean2[chain, i] := 0.0;
				INC(i)
			END;
			INC(chain)
		END;
		sampleSize := 1;
		i := 0;
		WHILE i < warmUpPeriod DO
			chain := 0;
			WHILE chain < numChains DO
				MathRandnum.SetGenerator(BugsRandnum.generators[chain]);
				UpdaterActions.LoadSamples(chain);
				UpdaterActions.Sample(overRelax, chain, res, updater);
				j := 0;
				WHILE j < numStochastics DO
					x := stochastics[j].value;
					mapVal := stochastics[j].Map();
					mean[chain, j] := (1.0 - 1.0 / sampleSize) * mean[chain, j] + (mapVal / sampleSize);
					mean2[chain, j] := (1.0 - 1.0 / sampleSize) * mean2[chain, j] + (mapVal * mapVal / sampleSize);
					INC(j)
				END;
				UpdaterActions.StoreSamples(chain);
				INC(chain)
			END;
			INC(sampleSize);
			INC(i)
		END;
		MathRandnum.SetGenerator(BugsRandnum.generators[0]);
		UpdaterActions.SetAdaption(0, 0);
		time := Services.Ticks() - time;
		Strings.IntToString(warmUpPeriod, number);
		string := number + " warm up updates took ";
		Strings.IntToString(time DIV Services.resolution, number);
		string := string + number + ".";
		Strings.IntToString(time MOD Services.resolution, number);
		string := string + number + "s";
		BugsFiles.ShowStatus(string)
	END WarmUp;

	PROCEDURE LeapFrog*;
		VAR
			diff, kineticNew, kineticOld, logAlpha, logDataLikeNew, logDataLikeOld,
			logDetJacobianNew, logDetJacobianOld,
			logStochasticLikeNew, logStochasticLikeOld, x: REAL;
			i, j, numStochastics: INTEGER;
			stochastics: GraphStochastic.Vector;
			stoch: GraphStochastic.Node;
			reject: BOOLEAN;
	BEGIN
		stochastics := GraphStochastic.stochastics;
		numStochastics := GraphStochastic.numStochastics;
		logDataLikeOld := LogLikelihood(data);
		(*	generate momentum and calculate energy	*)
		kineticOld := 0.0;
		logStochasticLikeOld := 0.0;
		logDetJacobianOld := 0.0;
		i := 0;
		WHILE i < numStochastics DO
			p[i] := MathRandnum.StandardNormal();
			stoch := stochastics[i];
			oldVals[i] := stoch.value;
			WITH stoch: GraphMultivariate.Node DO
				IF stoch.index = 0 THEN logStochasticLikeOld := logStochasticLikeOld + stoch.LogLikelihood() END
			ELSE
				logStochasticLikeOld := logStochasticLikeOld + stoch.LogLikelihood()
			END;
			logDetJacobianOld := logDetJacobianOld + stoch.LogDetJacobian();
			kineticOld := kineticOld + 0.5 * p[i] * p[i];
			INC(i)
		END;
		(*	L leapfrog steps	*)
		i := 0;
		WHILE i < numStochastics DO
			diff := stochastics[i].DiffLogConditional(); p[i] := p[i] + 0.5 * sigma[i] * delta * diff;
			INC(i)
		END;
		j := 0;
		WHILE j < L - 1 DO
			i := 0;
			WHILE i < numStochastics DO
				stoch := stochastics[i]; x := stoch.Map(); x := x + sigma[i] * delta * p[i]; stoch.InvMap(x);
				INC(i)
			END;
			i := 0;
			WHILE i < numStochastics DO
				diff := stochastics[i].DiffLogConditional(); p[i] := p[i] + sigma[i] * delta * diff;
				INC(i)
			END;
			INC(j);
		END;
		i := 0;
		WHILE i < numStochastics DO
			stoch := stochastics[i]; x := stoch.Map(); x := x + sigma[i] * delta * p[i]; stoch.InvMap(x);
			INC(i)
		END;
		logDataLikeNew := LogLikelihood(data);
		logStochasticLikeNew := 0.0;
		logDetJacobianNew := 0.0;
		kineticNew := 0.0;
		(*	calculate final  momentum and energy	*)
		i := 0;
		WHILE i < numStochastics DO
			stoch := stochastics[i];
			diff := stoch.DiffLogConditional(); p[i] := p[i] + 0.5 * sigma[i] * delta * diff;
			kineticNew := kineticNew + 0.5 * p[i] * p[i];
			WITH stoch: GraphMultivariate.Node DO
				IF stoch.index = 0 THEN logStochasticLikeNew := logStochasticLikeOld + stoch.LogLikelihood() END
			ELSE
				logStochasticLikeNew := logStochasticLikeNew + stoch.LogLikelihood()
			END;
			logDetJacobianNew := logDetJacobianNew + stoch.LogDetJacobian();
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

	PROCEDURE Update*;
		VAR
			chain, i, j, numChains, numStochastics: INTEGER;
			time: LONGINT;
			string, number: Dialog.String;
	BEGIN
		time := Services.Ticks();
		numChains := UpdaterActions.NumberChains();
		numStochastics := GraphStochastic.numStochastics;
		i := 0;
		WHILE i < iterations DO
			chain := 0;
			WHILE chain < numChains DO
				j := 0;
				WHILE j < numStochastics DO
					sigma[j] := Math.Sqrt(mean2[chain, j] - mean[chain, j] * mean[chain, j]);
					INC(j)
				END;
				MathRandnum.SetGenerator(BugsRandnum.generators[chain]);
				UpdaterActions.LoadSamples(chain);
				LeapFrog;
				IF MonitorMonitors.devianceMonitored THEN
					BugsInterface.LoadDeviance(chain)
				END;
				MonitorMonitors.UpdateMonitors(chain);
				UpdaterActions.StoreSamples(chain);
				INC(chain)
			END;
			INC(i)
		END;
		time := Services.Ticks() - time;
		Strings.IntToString(iterations, number);
		string := number + " HMC updates took ";
		Strings.IntToString(time DIV Services.resolution, number);
		string := string + number + ".";
		Strings.IntToString(time MOD Services.resolution, number);
		string := string + number + "s";
		BugsFiles.ShowStatus(string)
	END Update;

BEGIN
	L := 10;
	warmUpPeriod := 1000;
	iterations := 10000;
	delta := 0.25
END UpdaterHMC.

