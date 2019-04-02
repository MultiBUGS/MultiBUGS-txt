(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterEllipticalD;


	

	IMPORT
		Math, Stores,
		BugsRegistry,
		GraphConjugateUV, GraphRules, GraphStochastic,
		MathRandnum,
		UpdaterMultivariate, UpdaterUpdaters;

	TYPE
		Updater = POINTER TO RECORD(UpdaterMultivariate.Updater) END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		mu, nu, oldX: POINTER TO ARRAY OF REAL;
		fact-: UpdaterUpdaters.Factory;
		maintainer-: ARRAY 40 OF CHAR;
		version-: INTEGER;

	PROCEDURE (updater: Updater) Clone (): Updater;
		VAR
			u: Updater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Updater) CopyFromMultivariate (source: UpdaterUpdaters.Updater);
	BEGIN
	END CopyFromMultivariate;

	PROCEDURE (updater: Updater) ExternalizeMultivariate (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeMultivariate;

	PROCEDURE (updater: Updater) FindBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
		VAR
			class: SET;
	BEGIN
		class := {prior.classConditional};
		RETURN UpdaterMultivariate.FixedEffects(prior, class, FALSE, FALSE)
	END FindBlock;

	PROCEDURE (updater: Updater) InternalizeMultivariate (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeMultivariate;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) InitializeMultivariate;
		VAR
			size: INTEGER;
	BEGIN
		size := updater.Size();
		IF size > LEN(mu) THEN
			NEW(mu, size);
			NEW(nu, size);
			NEW(oldX, size);
		END
	END InitializeMultivariate;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterEllipticalD.Install"
	END Install;

	PROCEDURE (updater: Updater) ParamsSize (): INTEGER;
	BEGIN
		RETURN 0
	END ParamsSize;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			i, size: INTEGER;
			cos, sin, tau, twoPi, theta, thetaMax, thetaMin, x, y, logLikelihood: REAL;
			prior: GraphStochastic.Vector;
		CONST
			as = GraphRules.normal;
	BEGIN
		res := {};
		size := updater.Size();
		twoPi := 2.0 * Math.Pi();
		prior := updater.prior;
		i := 0;
		WHILE i < size DO
			oldX[i] := updater.prior[i].value;
			prior[i](GraphConjugateUV.Node).PriorForm(as, mu[i], tau);
			nu[i] := MathRandnum.Normal(0.0, tau);
			INC(i)
		END;
		thetaMax := twoPi * MathRandnum.Rand();
		thetaMin := thetaMax - twoPi;
		y := updater.LogLikelihood() + Math.Ln(MathRandnum.Rand());
		LOOP
			theta := MathRandnum.Uniform(thetaMin, thetaMax);
			cos := Math.Cos(theta);
			sin := Math.Sin(theta);
			i := 0;
			WHILE i < size DO
				x := mu[i] + (oldX[i] - mu[i]) * cos + nu[i] * sin;
				updater.prior[i].SetValue(x);
				INC(i)
			END;
			logLikelihood := updater.LogLikelihood();
			IF logLikelihood > y THEN EXIT
			ELSIF theta < 0.0 THEN thetaMin := theta
			ELSE thetaMax := theta
			END
		END;
	END Sample;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		VAR
			class: SET;
			block: GraphStochastic.Vector;
		CONST
			maxSize = 10;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF prior.ClassifyPrior() # GraphRules.normal THEN RETURN FALSE END;
		IF ~(prior IS GraphConjugateUV.Node) THEN RETURN FALSE END;
		IF prior.depth = 1 THEN RETURN FALSE END;
		IF ~(prior.classConditional IN {GraphRules.general, GraphRules.genDiff}) THEN RETURN FALSE END;
		class := {prior.classConditional};
		block := UpdaterMultivariate.FixedEffects(prior, class, FALSE, FALSE);
		IF block = NIL THEN  RETURN FALSE END;
		IF LEN(block) > maxSize THEN RETURN FALSE END;
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
		install := "UpdaterEllipticalD.Install"
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
		CONST
			size = 10;
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
		fact := f;
		NEW(mu, size);
		NEW(nu, size);
		NEW(oldX, size)
	END Init;

BEGIN
	Init
END UpdaterEllipticalD.
