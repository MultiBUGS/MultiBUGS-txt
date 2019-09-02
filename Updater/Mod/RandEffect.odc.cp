(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"


Univariate version of Eliptical slice sampler with conditioning

*)

MODULE UpdaterRandEffect;


	

	IMPORT
		Math, Stores := Stores64,
		BugsRegistry,
		GraphConjugateUV, GraphRules, GraphStochastic,
		MathRandnum,
		UpdaterContinuous, UpdaterUpdaters;

	TYPE
		Updater = POINTER TO RECORD(UpdaterContinuous.Updater) END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		maintainer-: ARRAY 40 OF CHAR;
		version-: INTEGER;

	PROCEDURE LogNormalLikelihood (muTau, tau1, value: REAL): REAL;
	BEGIN
		RETURN -0.5 * tau1 * value * value + muTau * value
	END LogNormalLikelihood;

	PROCEDURE (updater: Updater) Clone (): Updater;
		VAR
			u: Updater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Updater) CopyFromUnivariate (source: UpdaterUpdaters.Updater);
	BEGIN
	END CopyFromUnivariate;

	PROCEDURE (updater: Updater) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeUnivariate;

	PROCEDURE (updater: Updater) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeUnivariate;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) InitializeUnivariate;
	BEGIN
	END InitializeUnivariate;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterRandEffect.Install"
	END Install;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			cos, sin, mu, nu, oldX, tau, twoPi, theta, thetaMax, thetaMin, x, y,
			derivFirst, derivSecond, eta0, f, fMinus, fPlus, muTau, muTau0, tau1: REAL;
			prior: GraphStochastic.Node;
			children: GraphStochastic.Vector;
		CONST
			as = GraphRules.normal;
			delta = 0.0001;
	BEGIN
		res := {};
		twoPi := 2.0 * Math.Pi();
		prior := updater.prior;
		oldX := prior.value;
		children := prior.children;
		f := updater.LogLikelihood();
		prior.value := oldX + delta;
		fPlus := updater.LogLikelihood();
		prior.value := oldX - delta;
		fMinus := updater.LogLikelihood();
		prior.value := oldX;
		derivFirst := (fPlus - fMinus) / (2 * delta);
		derivSecond := (fPlus - 2.0 * f + fMinus) / (delta * delta);
		derivSecond := MIN(derivSecond, 0);
		tau1 := - derivSecond;
		muTau := derivFirst + tau1 * oldX;
		prior(GraphConjugateUV.Node).PriorForm(as, mu, tau);
		muTau0 := mu * tau;
		tau := tau + tau1;
		mu := (muTau + muTau0) / tau;
		nu := MathRandnum.Normal(0.0, tau);
		thetaMax := twoPi * MathRandnum.Rand();
		thetaMin := thetaMax - twoPi;
		y := updater.LogLikelihood() - LogNormalLikelihood(muTau, tau1, oldX) +
									Math.Ln(MathRandnum.Rand());
		LOOP
			theta := MathRandnum.Uniform(thetaMin, thetaMax);
			cos := Math.Cos(theta);
			sin := Math.Sin(theta);
			x := mu + (oldX - mu) * cos + nu * sin;
			prior.value := x;
			IF updater.LogLikelihood() - LogNormalLikelihood(muTau, tau1, x) > y THEN EXIT
			ELSIF theta < 0.0 THEN thetaMin := theta
			ELSE thetaMax := theta
			END
		END;
	END Sample;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF prior.ClassifyPrior() # GraphRules.normal THEN RETURN FALSE END;
		IF ~(prior IS GraphConjugateUV.Node) THEN RETURN FALSE END;
		IF prior.depth = 1 THEN RETURN FALSE END;
		IF ~(prior.classConditional IN {GraphRules.general, GraphRules.genDiff}) THEN RETURN FALSE END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: Factory) GetDefaults;
		VAR
			overRelaxation, res: INTEGER;
			props: SET;
			name: ARRAY 256 OF CHAR;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadInt(name + ".overRelaxation", overRelaxation, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetParameter(overRelaxation, UpdaterUpdaters.overRelaxation);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterElipticalD.Install"
	END Install;

	PROCEDURE (f: Factory) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: Updater;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

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
			f: Factory; isRegistered: BOOLEAN;
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
	BEGIN
		Maintainer;
		NEW(f);
		f.Install(name);
		f.SetProps({UpdaterUpdaters.overRelaxation, UpdaterUpdaters.enabled});
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN
			ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteInt(name + ".overRelaxation", 16);
			BugsRegistry.WriteSet(name + ".props", f.props)
		END;
		f.GetDefaults;
		fact := f
	END Init;

BEGIN
	Init
END UpdaterRandEffect.
