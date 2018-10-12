(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterSlice;

	

	IMPORT
		Math, Stores, 
		BugsRegistry,
		GraphNodes, GraphRules, GraphStochastic,
		MathRandnum,
		UpdaterContinuous, UpdaterUpdaters;

	CONST
		batch = 25;

		(*	internal states of sampling algorithm	*)
		left = 0; right = 1; sample = 2;

		leftBounds = {GraphStochastic.leftNatural, GraphStochastic.leftImposed};
		rightBounds = {GraphStochastic.rightNatural, GraphStochastic.rightImposed};

	TYPE
		Updater = POINTER TO RECORD(UpdaterContinuous.Updater)
			unimodal: BOOLEAN;
			iteration: INTEGER;
			meanStep, step, left, leftBound, right, rightBound, oldX, h: REAL;
		END;

		Factory = POINTER TO RECORD(UpdaterUpdaters.Factory) END;


	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
	
	PROCEDURE (updater: Updater) Clone (): Updater;
		VAR
			u: Updater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Updater) CopyFromUnivariate (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
	BEGIN
		s := source(Updater);
		updater.unimodal := s.unimodal;
		updater.iteration := s.iteration;
		updater.meanStep := s.meanStep;
		updater.step := s.step;
		updater.left := s.left;
		updater.leftBound := s.leftBound;
		updater.right := s.right;
		updater.rightBound := s.rightBound;
		updater.oldX := s.oldX;
		updater.h := s.h
	END CopyFromUnivariate;

	PROCEDURE (updater: Updater) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteInt(updater.iteration);
		wr.WriteReal(updater.meanStep);
		wr.WriteReal(updater.step);
	END ExternalizeUnivariate;

	PROCEDURE (updater: Updater) InitializeUnivariate;
		VAR
			prior: GraphStochastic.Node;
	BEGIN
		updater.iteration := 0;
		updater.step := 0.1;
		updater.meanStep := 0.0;
		prior := updater.prior;
		CASE prior.classConditional OF
		|GraphRules.logCon, GraphRules.logReg, GraphRules.cloglogReg,
			GraphRules.logitReg, GraphRules.probitReg, GraphRules.normal, GraphRules.mVN:
			updater.unimodal := TRUE
		ELSE
			updater.unimodal := FALSE
		END
	END InitializeUnivariate;

	PROCEDURE (updater: Updater) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadInt(updater.iteration);
		rd.ReadReal(updater.meanStep);
		rd.ReadReal(updater.step);
	END InternalizeUnivariate;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
		VAR
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.prior;
		IF (leftBounds * prior.props # {}) & (rightBounds * prior.props # {}) THEN
			RETURN FALSE
		END;
		RETURN ~updater.unimodal & (updater.iteration < fact.adaptivePhase + 1)
	END IsAdapting;

	(*	Pegasus solver for algebraic equations	*)
	PROCEDURE (updater: Updater) Solve (x0, x1, tol: REAL): REAL, NEW;
		VAR
			count: INTEGER;
			x, y, y0, y1: REAL;
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.prior;
		prior.SetValue(x0);
		y0 := updater.LogConditional() - updater.h;
		prior.SetValue(x1);
		y1 := updater.LogConditional() - updater.h;
		count := 0;
		LOOP
			x := x1 - y1 * (x1 - x0) / (y1 - y0);
			prior.SetValue(x);
			y := updater.LogConditional() - updater.h;
			IF y * y1 > 0 THEN
				y0 := y0 * y1 / (y1 + y)
			ELSE
				x0 := x1;
				y0 := y1
			END;
			x1 := x;
			y1 := y;
			IF ABS(x1 - x0) < tol THEN
				EXIT
			END;
			INC(count);
			ASSERT(count < 2000, 70)
		END;
		RETURN x
	END Solve;

	(*	changes internal state of sample usually depending on the value of logCond	*)
	PROCEDURE (updater: Updater) State (logCond: REAL; VAR state, attempts: INTEGER), NEW;
		VAR
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.prior;
		CASE state OF
		|left:
			IF logCond < updater.h THEN
				state := right;
				updater.left := prior.value;
				prior.SetValue(updater.oldX);
				attempts := 0
			END
		|right:
			IF logCond < updater.h THEN
				state := sample;
				updater.right := prior.value;
				attempts := 0
			END
		|sample:
			IF logCond > updater.h THEN (*	have new MCMC sample	*)
				state := - state;
				INC(updater.iteration);
				IF updater.unimodal OR (updater.iteration < fact. adaptivePhase) THEN
					updater.meanStep := updater.meanStep + ABS(prior.value - updater.oldX);
					IF updater.iteration MOD batch = 0 THEN
						updater.step := 3.25 * updater.meanStep / batch;
						updater.meanStep := 0.0
					END
				END
			ELSIF prior.value < updater.oldX THEN
				updater.left := prior.value
			ELSE
				updater.right := prior.value
			END
		END
	END State;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			attempts, rightIts, state: INTEGER;
			prior: GraphStochastic.Node;
			logCond, p, rand, x, lower, upper: REAL;
		CONST
			tol = 1.0E-6;
	BEGIN
		res := {};
		prior := updater.prior;
		p := MathRandnum.Rand();
		rand := MathRandnum.Rand();
		rightIts := MathRandnum.DiscreteUniform(0, fact.iterations);
		updater.oldX := prior.value;
		prior.Bounds(updater.leftBound, updater.rightBound);
		attempts := 0;
		logCond := updater.LogConditional();
		updater.h := logCond + Math.Ln(rand);
		IF(leftBounds * prior.props # {}) & (rightBounds * prior.props # {}) THEN
			state := sample;
			updater.left := updater.leftBound;
			updater.right := updater.rightBound
		ELSE
			state := left
		END;
		LOOP
			CASE state OF
			|left:
				INC(attempts);
				IF attempts > fact.iterations - rightIts THEN
					prior.SetValue(updater.oldX);
					EXIT
				END;
				IF attempts = 1 THEN
					x := prior.value - updater.step * (1 - p)
				ELSE
					x := prior.value - updater.step
				END;
				IF (leftBounds * prior.props # {}) & (x < updater.leftBound) THEN
					updater.left := updater.leftBound;
					state := right;
					prior.SetValue(updater.oldX);
					attempts := 0
				ELSE
					prior.SetValue(x);
					logCond := updater.LogConditional();
					updater.State(logCond, state, attempts)
				END
			|right:
				INC(attempts);
				IF attempts > rightIts THEN
					prior.SetValue(updater.oldX);
					EXIT
				END;
				IF attempts = 1 THEN
					x := prior.value + updater.step * p
				ELSE
					x := prior.value + updater.step
				END;
				IF (rightBounds * prior.props # {}) & (x > updater.rightBound) THEN
					updater.right := updater.rightBound;
					state := sample;
					attempts := 0
				ELSE
					prior.SetValue(x);
					logCond := updater.LogConditional();
					updater.State(logCond, state, attempts)
				END
			|sample:
				IF overRelax & (updater.iteration MOD fact.overRelaxation = 0) THEN
					INC(updater.iteration);
					lower := updater.Solve(updater.left, updater.oldX, tol);
					upper := updater.Solve(updater.oldX, updater.right, tol);
					x := upper + lower - updater.oldX;
					prior.SetValue(x);
					IF ~updater.unimodal & (updater.LogConditional() < updater.h) THEN
						prior.SetValue(updater.oldX)
					END;
					EXIT
				END;
				INC(attempts);
				IF attempts > fact.iterations THEN
					res := {GraphNodes.lhs, GraphNodes.tooManyIts}; 
					EXIT
				END;
				ASSERT(updater.left < updater.oldX, 66);
				ASSERT(updater.right > updater.oldX, 77);
				x := MathRandnum.Uniform(updater.left, updater.right);
				prior.SetValue(x);
				logCond := updater.LogConditional();
				updater.State(logCond, state, attempts);
				IF state = - sample THEN EXIT END
			END
		END
	END Sample;

	PROCEDURE (updater: Updater) Install* (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterSlice.Install"
	END Install;

	PROCEDURE (updater: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterSlice.Install"
	END Install;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
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
			adaptivePhase, iterations, overRelaxation, res: INTEGER;
			props: SET;
			name: ARRAY 256 OF CHAR;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadInt(name + ".iterations", iterations, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadInt(name + ".adaptivePhase", adaptivePhase, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadInt(name + ".overRelaxation", overRelaxation, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetParameter(iterations, UpdaterUpdaters.iterations);
		f.SetParameter(adaptivePhase, UpdaterUpdaters.adaptivePhase);
		f.SetParameter(overRelaxation, UpdaterUpdaters.overRelaxation);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE Install*;
	BEGIN
		UpdaterUpdaters.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			isRegistered: BOOLEAN;
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		f.Install(name);
		f.SetProps({UpdaterUpdaters.iterations, UpdaterUpdaters.overRelaxation,
		UpdaterUpdaters.adaptivePhase, UpdaterUpdaters.enabled});
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteInt(name + ".iterations", 100000);
			BugsRegistry.WriteInt(name + ".adaptivePhase", 500);
			BugsRegistry.WriteInt(name + ".overRelaxation", 4);
			BugsRegistry.WriteSet(name + ".props", f.props)
		END;
		f.GetDefaults;
		fact := f
	END Init;

BEGIN
	Init
END UpdaterSlice.
