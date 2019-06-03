(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterDescreteSlice;


	

	IMPORT
		Math, Stores := Stores64,
		BugsRegistry,
		GraphNodes, GraphRules, GraphStochastic, GraphVD,
		MathRandnum,
		UpdaterUnivariate, UpdaterUpdaters;

	CONST
		batch = 25;

	TYPE
		Updater = POINTER TO ABSTRACT RECORD(UpdaterUnivariate.Updater) END;

		StdUpdater = POINTER TO RECORD(Updater)
			meanStep: REAL;
			iteration, step: INTEGER;
		END;

		Interval = POINTER TO RECORD(Updater) END;

		StdFactory = POINTER TO RECORD(UpdaterUpdaters.Factory) END;

		IntervalFactory = POINTER TO RECORD(UpdaterUpdaters.Factory) END;

	VAR
		factStd-, factInterval-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (updater: Updater) GenerateInit (fixFounder: BOOLEAN; OUT res: SET);
		VAR
			prior: GraphStochastic.Node;
			leftR, rightR: REAL;
			left, right, sample: INTEGER;
		CONST
			eps = 1.0E-10;
	BEGIN
		prior := updater.prior;
		IF GraphStochastic.initialized IN prior.props THEN RETURN END;
		prior.Bounds(leftR, rightR);
		left := SHORT(ENTIER(leftR + eps));
		right := SHORT(ENTIER(rightR + eps));
		REPEAT
			prior.Sample(res);
			IF res # {} THEN RETURN END;
			sample := SHORT(ENTIER(prior.value + eps));
		UNTIL (sample >= left) & (sample <= right);
		prior.SetProps(prior.props + {GraphStochastic.initialized})
	END GenerateInit;

	PROCEDURE (updater: StdUpdater) BracketSlice (h: REAL; x, step: INTEGER; OUT left, right: INTEGER;
	OUT res: SET), NEW;
		CONST
			eps = 1.0E-10;
		VAR
			iter, l: INTEGER;
			leftR, rightR, y: REAL;
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.prior;
		res := {};
		prior.Bounds(leftR, rightR);
		l := SHORT(ENTIER(leftR + eps));
		right := x + MathRandnum.DiscreteUniform(0, step);
		left := right - step;
		IF left > l THEN
			prior.SetValue(left); y := updater.LogConditional() - h;
			iter := factStd.iterations;
			LOOP
				DEC(iter); IF iter < 0 THEN res := {GraphNodes.lhs, GraphNodes.tooManyIts}; RETURN END;
				IF y < 0.0 THEN EXIT END;
				DEC(left, step);
				IF left <= l THEN left := l; EXIT
				ELSE prior.SetValue(left); y := updater.LogConditional() - h
				END
			END
		ELSE
			left := l
		END;
		prior.SetValue(right); y := updater.LogConditional() - h;
		iter := factStd.iterations;
		LOOP
			DEC(iter); IF iter < 0 THEN res := {GraphNodes.lhs, GraphNodes.tooManyIts}; RETURN END;
			IF y < 0.0 THEN EXIT END;
			INC(right, step);
			prior.SetValue(right); y := updater.LogConditional() - h
		END
	END BracketSlice;

	PROCEDURE (updater: StdUpdater) Clone (): StdUpdater;
		VAR
			u: StdUpdater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: StdUpdater) CopyFromUnivariate (source: UpdaterUpdaters.Updater);
		VAR
			s: StdUpdater;
	BEGIN
		s := source(StdUpdater);
		updater.meanStep := s.meanStep;
		updater.iteration := s.iteration;
		updater.step := s.step
	END CopyFromUnivariate;

	PROCEDURE (updater: StdUpdater) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteReal(updater.meanStep);
		wr.WriteInt(updater.iteration);
		wr.WriteInt(updater.step);
	END ExternalizeUnivariate;

	PROCEDURE (updater: StdUpdater) InitializeUnivariate;
	BEGIN
		updater.step := 1;
		updater.meanStep := 0.0;
		updater.iteration := 0;
	END InitializeUnivariate;

	PROCEDURE (updater: StdUpdater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterDescreteSlice.Install"
	END Install;

	PROCEDURE (updater: StdUpdater) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadReal(updater.meanStep);
		rd.ReadInt(updater.iteration);
		rd.ReadInt(updater.step);
	END InternalizeUnivariate;

	PROCEDURE (updater: StdUpdater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN updater.iteration < factStd.adaptivePhase + 1
	END IsAdapting;

	PROCEDURE (updater: StdUpdater) Sample (overRelax: BOOLEAN; OUT res: SET);
		CONST
			eps = 1.0E-10;
		VAR
			iter, left, oldX, rand, right: INTEGER;
			cond, h, logRand: REAL;
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.prior;
		oldX := SHORT(ENTIER(prior.value + eps));
		logRand := Math.Ln(MathRandnum.Rand());
		h := updater.LogConditional() + logRand;
		updater.BracketSlice(h, oldX, updater.step, left, right, res);
		IF res # {} THEN RETURN END;
		iter := factStd.iterations;
		LOOP
			DEC(iter);
			IF iter < 0 THEN res := {GraphNodes.lhs, GraphNodes.tooManyIts}; RETURN END;
			rand := MathRandnum.DiscreteUniform(left, right);
			prior.SetValue(rand);
			cond := updater.LogConditional();
			IF (h < cond) OR (left = right) THEN EXIT
			ELSIF rand <= oldX THEN left := rand
			ELSE right := rand
			END
		END;
		INC(updater.iteration);
		IF updater.iteration <= factStd.adaptivePhase THEN
			updater.meanStep := updater.meanStep + ABS(prior.value - oldX);
			IF updater.iteration MOD batch = 0 THEN
				updater.step := SHORT(ENTIER(eps + 2.0 * updater.meanStep / batch));
				updater.step := MAX(updater.step, 1);
				updater.step := MIN(updater.step, 50);
				updater.meanStep := 0.0
			END
		END
	END Sample;

	PROCEDURE (updater: Interval) Clone (): Interval;
		VAR
			u: Interval;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Interval) CopyFromUnivariate (source: UpdaterUpdaters.Updater);
	BEGIN
	END CopyFromUnivariate;

	PROCEDURE (updater: Interval) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeUnivariate;

	PROCEDURE (updater: Interval) InitializeUnivariate;
	BEGIN
	END InitializeUnivariate;

	PROCEDURE (updater: Interval) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterDescreteSlice.IntervalInstall"
	END Install;

	PROCEDURE (updater: Interval) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeUnivariate;

	PROCEDURE (updater: Interval) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Interval) Sample (overRelax: BOOLEAN; OUT res: SET);
		CONST
			eps = 1.0E-10;
		VAR
			iter, left, oldX, rand, right: INTEGER;
			cond, h, logRand, leftR, rightR: REAL;
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.prior;
		prior.Bounds(leftR, rightR);
		left := SHORT(ENTIER(leftR + eps));
		right := SHORT(ENTIER(rightR + eps));
		oldX := SHORT(ENTIER(prior.value + eps));
		logRand := Math.Ln(MathRandnum.Rand());
		h := updater.LogConditional() + logRand;
		iter := factInterval.iterations;
		res := {};
		LOOP
			DEC(iter);
			IF iter < 0 THEN res := {GraphNodes.lhs, GraphNodes.tooManyIts}; RETURN END;
			rand := MathRandnum.DiscreteUniform(left, right);
			prior.SetValue(rand);
			cond := updater.LogConditional();
			IF (h < cond) OR (left = right) THEN EXIT
			ELSIF rand < oldX THEN left := rand
			ELSE right := rand
			END
		END
	END Sample;

	PROCEDURE (f: StdFactory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		CONST
			rightBounds = {GraphStochastic.rightNatural, GraphStochastic.rightImposed};
	BEGIN
		IF ~(GraphStochastic.integer IN prior.props) THEN RETURN FALSE END;
		IF GraphVD.Block(prior) # NIL THEN RETURN FALSE END;
		IF prior.classConditional # GraphRules.descrete THEN RETURN FALSE END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: StdFactory) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: StdUpdater;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE (f: StdFactory) GetDefaults;
		VAR
			adaptivePhase, iterations, overRelaxation, res: INTEGER;
			props: SET;
			name: ARRAY 256 OF CHAR;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadInt(name + ".iterations", iterations, res); ASSERT(res = 0, 54);
		BugsRegistry.ReadInt(name + ".adaptivePhase", adaptivePhase, res); ASSERT(res = 0, 60);
		BugsRegistry.ReadInt(name + ".overRelaxation", overRelaxation, res); ASSERT(res = 0, 56);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetParameter(iterations, UpdaterUpdaters.iterations);
		f.SetParameter(adaptivePhase, UpdaterUpdaters.adaptivePhase);
		f.SetParameter(overRelaxation, UpdaterUpdaters.overRelaxation);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: StdFactory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterDescreteSlice.Install"
	END Install;

	PROCEDURE (f: IntervalFactory) GetDefaults;
		VAR
			adaptivePhase, iterations, overRelaxation, res: INTEGER;
			props: SET;
			name: ARRAY 256 OF CHAR;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadInt(name + ".iterations", iterations, res); ASSERT(res = 0, 54);
		BugsRegistry.ReadInt(name + ".adaptivePhase", adaptivePhase, res); ASSERT(res = 0, 60);
		BugsRegistry.ReadInt(name + ".overRelaxation", overRelaxation, res); ASSERT(res = 0, 56);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetParameter(iterations, UpdaterUpdaters.iterations);
		f.SetParameter(adaptivePhase, UpdaterUpdaters.adaptivePhase);
		f.SetParameter(overRelaxation, UpdaterUpdaters.overRelaxation);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: IntervalFactory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterDescreteSlice.IntervalInstall"
	END Install;

	PROCEDURE (f: IntervalFactory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		CONST
			rightBounds = {GraphStochastic.rightNatural, GraphStochastic.rightImposed};
	BEGIN
		IF ~(GraphStochastic.integer IN prior.props) THEN RETURN FALSE END;
		IF GraphVD.Block(prior) # NIL THEN RETURN FALSE END;
		IF prior.classConditional # GraphRules.catagorical THEN RETURN FALSE END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: IntervalFactory) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: Interval;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE Install*;
	BEGIN
		UpdaterUpdaters.SetFactory(factStd)
	END Install;

	PROCEDURE IntervalInstall*;
	BEGIN
		UpdaterUpdaters.SetFactory(factInterval)
	END IntervalInstall;

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
			fStd: StdFactory;
			fInterval: IntervalFactory;
	BEGIN
		Maintainer;
		NEW(fStd);
		factStd := fStd;
		fStd.Install(name);
		fStd.SetProps({UpdaterUpdaters.iterations, UpdaterUpdaters.overRelaxation,
		UpdaterUpdaters.adaptivePhase, UpdaterUpdaters.enabled});
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 57)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteInt(name + ".iterations", 100000);
			BugsRegistry.WriteInt(name + ".adaptivePhase", 500);
			BugsRegistry.WriteInt(name + ".overRelaxation", 4);
			BugsRegistry.WriteSet(name + ".props", fStd.props)
		END;
		fStd.GetDefaults;
		
		NEW(fInterval);
		factInterval := fInterval;
		fInterval.Install(name);
		fInterval.SetProps({UpdaterUpdaters.iterations, UpdaterUpdaters.overRelaxation,
		UpdaterUpdaters.adaptivePhase, UpdaterUpdaters.enabled});
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 57)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteInt(name + ".iterations", 100000);
			BugsRegistry.WriteInt(name + ".adaptivePhase", 500);
			BugsRegistry.WriteInt(name + ".overRelaxation", 4);
			BugsRegistry.WriteSet(name + ".props", fInterval.props)
		END;
		fInterval.GetDefaults;
	END Init;

BEGIN
	Init
END UpdaterDescreteSlice.
