MODULE UpdaterDelayedDirectional1D;


	

	IMPORT
		Math, Stores := Stores64, 
		BugsRegistry,
		GraphMultivariate, GraphNodes, GraphRules, GraphStochastic,
		MathMatrix, MathRandnum,
		UpdaterMultivariate, UpdaterUpdaters;

	TYPE
		Matrix = POINTER TO ARRAY OF ARRAY OF REAL;
		Vector = POINTER TO ARRAY OF REAL;

		Updater = POINTER TO RECORD (UpdaterMultivariate.Updater)
			adapt: INTEGER;
			means, svdVector, precision: Vector;
			means2, svdMatrix, cov: Matrix;
			iterations: INTEGER;
			index, acceptCount, rejectCount: POINTER TO ARRAY OF INTEGER;
		END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		oldX, newX: Vector;

		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE FindNLBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
		VAR
			i, size: INTEGER;
			class: SET;
			components, block: GraphStochastic.Vector;
	BEGIN
		IF prior.ClassifyPrior() = GraphRules.mVN THEN
			components := prior(GraphMultivariate.Node).components;
			i := 0;
			size := prior.Size();
			WHILE (i < size) & ~(GraphNodes.data IN components[i].props) DO INC(i) END;
			IF i # size THEN
				block := NIL
			ELSE
				block := components
			END
		ELSE
			class := {prior.classConditional};
			block := UpdaterMultivariate.FixedEffects(prior, class, FALSE, FALSE);
			IF block = NIL THEN (*	try less restrictive block membership	*)
				INCL(class, GraphRules.normal);
				block := UpdaterMultivariate.FixedEffects(prior, class, FALSE, FALSE);
				IF block = NIL THEN
					block := UpdaterMultivariate.FixedEffects(prior, class, TRUE, FALSE);
					IF block = NIL THEN
						block := UpdaterMultivariate.FixedEffects(prior, class, TRUE, TRUE);
					END
				END
			END
		END;
		RETURN block
	END FindNLBlock;

	PROCEDURE CalculateSVD (updater: Updater);
		VAR
			i, j, size: INTEGER;
	BEGIN
		size := updater.Size();
		(* calculate covariance matrix *)
		i := 0;
		WHILE i < size DO
			j := 0;
			WHILE j < size DO
				updater.cov[i, j] := updater.means2[i, j] - updater.means[i] * updater.means[j];
				INC(j)
			END;
			INC(i)
		END;
		MathMatrix.SVD(updater.cov, updater.svdMatrix, updater.svdVector);
	END CalculateSVD;

	PROCEDURE UpdateSums (updater: Updater; start: INTEGER);
		VAR
			i, j, n, size: INTEGER;
			prior: GraphStochastic.Vector;
	BEGIN
		(* first 25% of burnin iterations iterations are not used *)
		n := updater.iterations - start;
		prior := updater.prior;
		size := updater.Size();
		i := 0;
		WHILE i < size DO
			updater.means[i] := (n * updater.means[i] + prior[i].value) / (n + 1); ;
			j := 0;
			WHILE j < size DO
				updater.means2[i, j] := (n * updater.means2[i, j] + 
				prior[i].value * prior[j].value) / (n + 1);
				INC(j)
			END;
			INC(i)
		END;
	END UpdateSums;

	(*	Target acceptance rate between 0.4 and 0.5 for standard 1D Metropolis *)
	PROCEDURE AdaptProposal (rate: REAL; VAR precision: REAL);
	BEGIN
		IF rate > 0.8 THEN
			precision := precision * 0.1
		ELSIF rate > 0.7 THEN
			precision := precision * 0.5
		ELSIF rate > 0.6 THEN
			precision := precision * 0.75
		ELSIF rate > 0.5 THEN
			precision := precision * 0.95
		ELSIF rate > 0.4 THEN
			precision := precision * 1.00
		ELSIF rate > 0.3 THEN
			precision := precision * 1.05
		ELSIF rate > 0.2 THEN
			precision := precision * 1.5
		ELSE
			precision := precision * 2.0
		END;
	END AdaptProposal;

	(* randomly shuffle the indices *)
	PROCEDURE Shuffle (VAR index: ARRAY OF INTEGER);
		VAR
			i, j, size, temp: INTEGER;
	BEGIN
		size := LEN(index);
		i := 0;
		WHILE i < size DO
			j := SHORT(ENTIER(MathRandnum.Rand() * (i + 1)));
			temp := index[j];
			index[j] := index[i];
			index[i] := temp;
			INC(i)
		END;
	END Shuffle;

	(* Perform standard 1D Metropolis update step*)
	PROCEDURE MwG (updater: Updater);
		VAR
			accept: BOOLEAN;
			i, j, v, size, var, temp: INTEGER;
			logAlpha, oldCond, newCond: REAL;
	BEGIN
		size := updater.Size();
		oldCond := updater.LogConditional();
		updater.GetValue(oldX);
		i := 0;
		WHILE i < size DO
			newX[i] := oldX[i]; INC(i)
		END;
		Shuffle(updater.index);
		(* loop over parameters *)
		v := 0;
		WHILE v < size DO
			(* new proposal *)
			var := updater.index[v];
			newX[var] := MathRandnum.Normal(oldX[var], updater.precision[var]);
			updater.SetValue(newX);
			newCond := updater.LogConditional();
			logAlpha := newCond - oldCond;
			accept := logAlpha > Math.Ln(MathRandnum.Rand());
			IF accept THEN
				INC(updater.acceptCount[var]);
				oldX[var] := newX[var];
				oldCond := newCond;
			ELSE
				INC(updater.rejectCount[var]);
				newX[var] := oldX[var];
			END;
		END;
		IF ~accept THEN
			updater.SetValue(oldX);
		END;
	END MwG;

	(* Perform delayed 1D Metropolis update step *)
	PROCEDURE DelayedMwG (updater: Updater);
		VAR
			accept: BOOLEAN;
			i, j, v, size, var, temp: INTEGER;
			prop1, prop2, logAlpha1, logAlpha2, oldCond, newCond1, newCond2, ry2y1, rxy1: REAL;
	BEGIN
		size := updater.Size();
		oldCond := updater.LogConditional();
		updater.GetValue(oldX);
		i := 0;
		WHILE i < size DO
			newX[i] := oldX[i]; INC(i)
		END;
		Shuffle(updater.index);
		(* loop over parameters *)
		v := 0;
		WHILE v < size DO
			(* new proposal *)
			var := updater.index[v];
			prop1 := MathRandnum.Normal(oldX[var], updater.precision[var]);
			updater.SetValue(newX);
			newCond1 := updater.LogConditional();
			logAlpha1 := newCond1 - oldCond;
			accept := logAlpha1 > Math.Ln(MathRandnum.Rand());
			(* accept/reject first proposal *)
			IF accept THEN
				INC(updater.acceptCount[var]);
				oldX[var] := newX[var];
				oldCond := newCond1;
			ELSE
				INC(updater.rejectCount[var]);
				(* calculate independent second proposal with 10x larger precision *)
				prop2 := MathRandnum.Normal(oldX[var], 10 * updater.precision[var]);
				updater.SetValue(newX);
				newCond2 := updater.LogConditional();
				IF newCond1 > newCond2 THEN
					(* reject *)
					accept := FALSE;
					newX[var] := oldX[var];
				ELSE
					(* calculate logAlpha2 *)
					ry2y1 := 1.0 - Math.Exp(newCond1 - newCond2);
					rxy1 := 1.0 - Math.Exp(newCond1 - oldCond);
					logAlpha2 := newCond2 - oldCond - 0.5 * updater.precision[var] * 
					((prop2 - prop1) * (prop2 - prop1) - (oldX[var] - prop1) * (oldX[var] - prop1)) + 
					Math.Ln(ry2y1) - Math.Ln(rxy1);
					(* accept/reject second proposal *)
					accept := logAlpha2 > Math.Ln(MathRandnum.Rand());
					IF accept THEN
						oldX[var] := newX[var];
						oldCond := newCond2;
					ELSE
						newX[var] := oldX[var];
					END;
				END
			END;
			INC(v)
		END;
		IF ~accept THEN
			updater.SetValue(oldX)
		END
	END DelayedMwG;

	(* Perform delayed 1D Metropolis update step *)
	PROCEDURE AntitheticMwG (updater: Updater);
		VAR
			accept: BOOLEAN;
			i, j, v, size, var, temp: INTEGER;
			delta, prop1, prop2, prop3, logAlpha1, logAlpha2, oldCond, newCond1, newCond2, newCond3,
			correction: REAL;
	BEGIN
		size := updater.Size();
		oldCond := updater.LogConditional();
		updater.GetValue(oldX);
		i := 0;
		WHILE i < size DO
			newX[i] := oldX[i]; INC(i)
		END;
		Shuffle(updater.index);
		(* loop over parameters *)
		v := 0;
		WHILE v < size DO
			(* new proposal *)
			var := updater.index[v];
			delta := MathRandnum.Normal(0, updater.precision[var]);
			prop1 := oldX[var] + delta;
			prop2 := oldX[var] - delta;
			prop3 := prop2 - delta;
			(* evaluate first proposal *)
			newX[var] := prop1;
			updater.SetValue(newX);
			newCond1 := updater.LogConditional();
			logAlpha1 := newCond1 - oldCond;
			accept := logAlpha1 > Math.Ln(MathRandnum.Rand());
			IF accept THEN
				INC(updater.acceptCount[var]);
				oldX[var] := newX[var];
				oldCond := newCond1;
			ELSE
				INC(updater.rejectCount[var]);
				newX[var] := prop3;
				updater.SetValue(newX);
				newCond3 := updater.LogConditional();
				newX[var] := prop2;
				updater.SetValue(newX);
				newCond2 := updater.LogConditional();
				correction := Math.Ln(1 - Math.Exp(MIN(0, newCond3 - newCond2))) - 
				Math.Ln(1 - Math.Exp(MIN(0, logAlpha1)));
				logAlpha2 := newCond2 - oldCond + correction;
				accept := logAlpha2 > Math.Ln(MathRandnum.Rand());
				IF accept THEN
					(* accept second proposal *)
					oldX[var] := newX[var];
					oldCond := newCond2;
				ELSE
					(* reject second proposal *)
					newX[var] := oldX[var];
				END;
				INC(v)
			END
		END;
		IF ~accept THEN
			updater.SetValue(oldX);
		END;
	END AntitheticMwG;

	(* Target acceptance rate between 0.3 and 0.4 for Directional Metropolis *)
	PROCEDURE AdaptDirectionalProposal (rate: REAL; VAR precision: REAL);
	BEGIN
		IF rate > 0.8 THEN
			precision := precision * 0.1
		ELSIF rate > 0.6 THEN
			precision := precision * 0.5
		ELSIF rate > 0.5 THEN
			precision := precision * 0.7
		ELSIF rate > 0.4 THEN
			precision := precision * 0.85
		ELSIF rate > 0.3 THEN
			precision := precision * 1.0
		ELSIF rate > 0.2 THEN
			precision := precision * 1.1
		ELSIF rate > 0.1 THEN
			precision := precision * 1.5
		ELSE
			precision := precision * 2.0
		END
	END AdaptDirectionalProposal;

	(* Perform Directional Metropolis update step - with accept/reject counts *)
	PROCEDURE DirectionalMwG (updater: Updater);
		VAR
			accept: BOOLEAN;
			i, j, v, size, dir, temp: INTEGER;
			logAlpha, oldCond, newCond, prop: REAL;
	BEGIN
		size := updater.Size();
		oldCond := updater.LogConditional();
		accept := TRUE;
		Shuffle(updater.index);
		(* loop over directions *)
		v := 0;
		WHILE v < size DO
			(* new proposal in random direction *)
			dir := updater.index[v];
			IF accept THEN
				(* only renew oldX if accepted *)
				updater.GetValue(oldX);
			END;
			prop := MathRandnum.Normal(0, updater.precision[0] / updater.svdVector[dir]);
			(* multiply proposal with appropriate column of SVD matrix *)
			i := 0;
			WHILE i < size DO
				newX[i] := oldX[i] + prop * updater.svdMatrix[i, dir]; INC(i)
			END;
			updater.SetValue(newX);
			newCond := updater.LogConditional();
			logAlpha := newCond - oldCond;
			accept := logAlpha > Math.Ln(MathRandnum.Rand());
			(* accept/reject *)
			IF accept THEN
				INC(updater.acceptCount[0]);
				oldCond := newCond;
			ELSE
				INC(updater.rejectCount[0]);
			END;
			INC(v)
		END;
		IF ~accept THEN
			updater.SetValue(oldX);
		END;
	END DirectionalMwG;

	(* Perform Directional Metropolis update step - with accept/reject counts *)
	PROCEDURE AntitheticDirectionalMwG (updater: Updater);
		VAR
			accept: BOOLEAN;
			i, j, v, size, dir, temp: INTEGER;
			delta, logAlpha1, logAlpha2, oldCond,
			newCond1, newCond2, newCond3, correction: REAL;
	BEGIN
		size := updater.Size();
		updater.GetValue(oldX);
		oldCond := updater.LogConditional();
		Shuffle(updater.index);
		(* loop over directions *)
		v := 0;
		WHILE v < size DO
			(* new proposal in random direction *)
			dir := updater.index[v];
			delta := MathRandnum.Normal(0, updater.precision[0] / updater.svdVector[dir]);
			(* multiply proposal with appropriate column of SVD matrix *)
			i := 0;
			WHILE i < size DO
				newX[i] := oldX[i] + delta * updater.svdMatrix[i, dir]; INC(i)
			END;
			updater.SetValue(newX);
			newCond1 := updater.LogConditional();
			logAlpha1 := newCond1 - oldCond;
			accept := logAlpha1 > Math.Ln(MathRandnum.Rand());
			(* accept/reject *)
			IF accept THEN
				INC(updater.acceptCount[0]);
				oldCond := newCond1;
				updater.GetValue(oldX);
			ELSE
				INC(updater.rejectCount[0]);
				(* shadow proposal 2 *)
				i := 0;
				WHILE i < size DO
					newX[i] := oldX[i] - 2 * delta * updater.svdMatrix[i, dir]; INC(i)
				END;
				updater.SetValue(newX);
				newCond3 := updater.LogConditional();
				(* proposal 2 *)
				i := 0;
				WHILE i < size DO
					newX[i] := oldX[i] - delta * updater.svdMatrix[i, dir]; INC(i)
				END;
				updater.SetValue(newX);
				newCond2 := updater.LogConditional();
				(* delayed rejection correction *)
				correction := Math.Ln(1 - Math.Exp(MIN(0, newCond3 - newCond2))) - 
				Math.Ln(1 - Math.Exp(MIN(0, logAlpha1)));
				logAlpha2 := newCond2 - oldCond + correction;
				accept := logAlpha2 > Math.Ln(MathRandnum.Rand());
				IF accept THEN
					(* accept second proposal *)
					oldCond := newCond2;
					updater.GetValue(oldX)
				END;
			END;
			INC(v)
		END;
		IF ~accept THEN
			updater.SetValue(oldX);
		END
	END AntitheticDirectionalMwG;

	PROCEDURE (updater: Updater) Clone (): Updater;
		VAR
			u: Updater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Updater) CopyFromMultivariate (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
			i, j, size, size1: INTEGER;
	BEGIN
		s := source(Updater);
		updater.adapt := s.adapt;
		updater.iterations := s.iterations;
		NEW(updater.means, LEN(s.means));
		NEW(updater.svdVector, LEN(s.svdVector));
		NEW(updater.precision, LEN(s.precision));
		NEW(updater.index, LEN(s.index));
		NEW(updater.acceptCount, LEN(s.acceptCount));
		NEW(updater.rejectCount, LEN(s.rejectCount));
		i := 0;
		size := updater.Size();
		WHILE i < size DO
			updater.means[i] := s.means[i];
			updater.svdVector[i] := s.svdVector[i];
			updater.precision[i] := s.precision[i];
			updater.index[i] := s.index[i];
			updater.acceptCount[i] := s.acceptCount[i];
			updater.rejectCount[i] := s.rejectCount[i];
			INC(i)
		END;
		i := 0;
		WHILE i < size DO
			j := 0;
			WHILE j < size DO
				updater.cov[i, j] := s.cov[i, j];
				updater.means2[i, j] := s.means2[i, j];
				INC(j)
			END;
			INC(i)
		END;
		size := LEN(updater.svdMatrix, 0);
		size1 := LEN(updater.svdMatrix, 1);
		i := 0;
		WHILE i < size DO
			j := 0;
			WHILE j < size1 DO
				updater.svdMatrix[i, j] := s.svdMatrix[i, j];
				INC(j)
			END;
			INC(i)
		END
	END CopyFromMultivariate;

	PROCEDURE (updater: Updater) ExternalizeMultivariate (VAR wr: Stores.Writer);
		VAR
			s: Updater;
			i, j, size: INTEGER;
	BEGIN
		size := updater.Size();
		wr.WriteInt(updater.adapt);
		wr.WriteInt(updater.iterations);
		i := 0;
		WHILE i < size DO
			wr.WriteReal(updater.means[i]);
			wr.WriteReal(updater.svdVector[i]);
			wr.WriteReal(updater.precision[i]);
			wr.WriteInt(updater.index[i]);
			wr.WriteInt(updater.acceptCount[i]);
			wr.WriteInt(updater.rejectCount[i]);
			INC(i)
		END;
		i := 0;
		WHILE i < size DO
			j := 0;
			WHILE j < size DO
				wr.WriteReal(updater.cov[i, j]);
				wr.WriteReal(updater.means2[i, j]);
				wr.WriteReal(updater.svdMatrix[i, j]);
				INC(j)
			END;
			INC(i)
		END;
	END ExternalizeMultivariate;

	PROCEDURE (updater: Updater) FindBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
	BEGIN
		RETURN FindNLBlock(prior)
	END FindBlock;

	PROCEDURE (updater: Updater) InitializeMultivariate;
		VAR
			i, j, size: INTEGER;
	BEGIN
		size := updater.Size();
		IF LEN(oldX) < size THEN
			NEW(oldX, size);
			NEW(newX, size);
		END;
		NEW(updater.index, size);
		NEW(updater.means, size);
		NEW(updater.svdVector, size);
		NEW(updater.precision, size);
		NEW(updater.acceptCount, size);
		NEW(updater.rejectCount, size);
		NEW(updater.means2, size, size);
		NEW(updater.svdMatrix, size, size);
		NEW(updater.cov, size, size);
		updater.adapt := 1;
		updater.iterations := 0;
		i := 0;
		WHILE i < size DO
			updater.index[i] := i;
			updater.means[i] := 0.0;
			updater.precision[i] := 1000.0;
			updater.acceptCount[i] := 0;
			updater.rejectCount[i] := 0;
			j := 0;
			WHILE j < size DO
				updater.means2[i, j] := 0.0;
				INC(j)
			END;
			INC(i)
		END
	END InitializeMultivariate;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterDelayedDirectional1D.Install"
	END Install;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		(* RETURN updater.iterations[chain] <= fact.adaptivePhase *)
		RETURN FALSE;
	END IsAdapting;

	PROCEDURE (updater: Updater) InternalizeMultivariate (VAR rd: Stores.Reader);
		VAR
			s: Updater;
			i, j, size, size1: INTEGER;
	BEGIN
		size := updater.Size();
		rd.ReadInt(updater.adapt);
		rd.ReadInt(updater.iterations);
		i := 0;
		NEW(updater.means, size);
		NEW(updater.svdVector, size);
		NEW(updater.precision, size);
		NEW(updater.index, size);
		NEW(updater.acceptCount, size);
		NEW(updater.rejectCount, size);
		NEW(updater.cov, size, size);
		NEW(updater.means2, size, size);
		NEW(updater.svdMatrix, size, size);
		WHILE i < size DO
			rd.ReadReal(updater.means[i]);
			rd.ReadReal(updater.svdVector[i]);
			rd.ReadReal(updater.precision[i]);
			rd.ReadInt(updater.index[i]);
			rd.ReadInt(updater.acceptCount[i]);
			rd.ReadInt(updater.rejectCount[i]);
			INC(i)
		END;
		i := 0;
		WHILE i < size DO
			j := 0;
			WHILE j < size DO
				rd.ReadReal(updater.cov[i, j]);
				rd.ReadReal(updater.means2[i, j]);
				rd.ReadReal(updater.svdMatrix[i, j]);
				INC(j)
			END;
			INC(i)
		END;
	END InternalizeMultivariate;

	PROCEDURE (updater: Updater) ParamsSize (): INTEGER;
	BEGIN
		RETURN 0
	END ParamsSize;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			acceptRate: REAL;
			i, var, iteration, size, adaptivePhase, learingCorr, learningSVD: INTEGER;
	BEGIN
		iteration := updater.iterations;
		adaptivePhase := (*fact.adaptivePhase*)5000;
		learingCorr := adaptivePhase DIV 4;
		learningSVD := adaptivePhase DIV 2;
		size := updater.Size();
		(*	A. directional MwG - without adaptation after 10,000 iterations	*)
		IF iteration >= adaptivePhase THEN
			(* directional Metropolis update *)
			AntitheticDirectionalMwG(updater);
			(*	B. directional MwG - with adaptation between iteration 5,001 and 10,000	*)
		ELSIF iteration >= learningSVD THEN
			AntitheticDirectionalMwG(updater);
			UpdateSums(updater, learingCorr);
			(*	adapt proposal distribution & SVD	*)
			IF iteration = (fact.adaptivePhase DIV 2) + 5 * Math.Power(2, updater.adapt) THEN
				acceptRate := updater.acceptCount[0] / (updater.acceptCount[0] + 																				 updater.rejectCount[0]);
				AdaptDirectionalProposal(acceptRate, updater.precision[0]);
				updater.acceptCount[0] := 0;
				updater.rejectCount[0] := 0;
				IF updater.adapt > 6 THEN
					CalculateSVD(updater)
				END;
				INC(updater.adapt)
			END;
			(*	final update of SVD at iteration 10,000	*)
			IF iteration = adaptivePhase - 1 THEN
				CalculateSVD(updater);
				(*	empty unused variables	*)
				updater.acceptCount[0] := 0;
				updater.acceptCount[0] := 0;
				updater.means[0] := 0.0;
				i := 0;
				WHILE i < size DO
					updater.means2[i, 0] := 0.0; INC(i)
				END
			END;
			(*	C. standard 1D  - with adaptation for initial 5,000 iterations	*)
		ELSE
			(*	perform standard 1D Metropolis update	*)
			AntitheticMwG(updater);
			(* updater.DelayedMwG(chain); *)
			(* initial 2,500 iterations are not used for covariance matrix calculation*)
			IF iteration >= learingCorr THEN
				UpdateSums(updater, learingCorr);
			END;
			(* adapt proposal distribution *)
			IF iteration = 5 * Math.Power(2, updater.adapt) THEN
				var := 0;
				WHILE var < size DO
					acceptRate := updater.acceptCount[var] / (updater.acceptCount[var] + 
					updater.rejectCount[var]);
					AdaptProposal(acceptRate, updater.precision[var]);
					updater.acceptCount[var] := 0;
					updater.rejectCount[var] := 0;
					INC(var)
				END;
				(* increment adapt count *)
				INC(updater.adapt);
			END;
			(* prepare for directional MwG at iteration learningSVD *)
			IF iteration = learningSVD - 1 THEN
				updater.adapt := 1;
				CalculateSVD(updater);
				updater.acceptCount[0] := 0;
				updater.rejectCount[0] := 0;
				i := 0;
				WHILE i < size DO
					(* replace updater.precision with average precision *)
					updater.precision[0] := updater.precision[0] + updater.precision[i];
					(* empty unused variables *)
					updater.acceptCount[i] := 0;
					updater.rejectCount[i] := 0;
					updater.precision[i] := 0.0;
					(* decrease precision in anticipation of larger step sizes with directional updating *)
					INC(i)
				END;
				updater.precision[0] := updater.precision[0] / (2.4 * size)
			END
		END;
		INC(updater.iterations);
		res := {}
	END Sample;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		VAR
			block: GraphStochastic.Vector;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF ~(prior.classConditional IN {GraphRules.general, GraphRules.genDiff}) THEN RETURN FALSE END;
		block := FindNLBlock(prior);
		IF block = NIL THEN RETURN FALSE END;
		IF GraphStochastic.IsBounded(block) THEN RETURN FALSE END;
		RETURN TRUE		
	END CanUpdate;

	PROCEDURE (f: Factory) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: Updater;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE (f: Factory) GetDefaults;
		VAR
			adaptivePhase, res: INTEGER;
			name: ARRAY 256 OF CHAR;
	BEGIN
	(*	f.Install(name);
		BugsRegistry.ReadInt(name + ".adaptivePhase", adaptivePhase, res); ASSERT(res = 0, 55);
		f.SetParameter(adaptivePhase, UpdaterUpdaters.adaptivePhase)*)
	END GetDefaults;

	PROCEDURE (f: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterDelayedDirectional1D.Install"
	END Install;

	PROCEDURE Install*;
	BEGIN
		UpdaterUpdaters.SetFactory(fact);
		(*fact.GetDefaults*)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 310;
		maintainer := "MF Jonker"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory;
			name: ARRAY 128 OF CHAR;
			res:
			INTEGER;
			isRegistered: BOOLEAN;
		CONST
			size = 1;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f;
		f.Install(name);
		f.SetProps({UpdaterUpdaters.adaptivePhase, UpdaterUpdaters.enabled});
	(*	BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN
			ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteInt(name + ".adaptivePhase", 5000)
		END;
		f.GetDefaults;*)
		NEW(oldX, size);
		NEW(newX, size)
	END Init;

BEGIN
	Init
END UpdaterDelayedDirectional1D.


