(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterElliptical;


	

	IMPORT
		Math, Stores := Stores64,
		BugsRegistry,
		GraphChain, GraphConjugateMV, GraphMRF, GraphMultivariate, GraphRules, GraphStochastic,
		MathMatrix, MathRandnum,
		UpdaterMultivariate, UpdaterUpdaters;

	TYPE
		Updater = POINTER TO RECORD(UpdaterMultivariate.Updater) END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		mu, nu, oldX: POINTER TO ARRAY OF REAL;
		tau: POINTER TO ARRAY OF ARRAY OF REAL;
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
	BEGIN
		RETURN prior(GraphMultivariate.Node).components
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
			NEW(tau, size, size)
		END
	END InitializeMultivariate;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterElliptical.Install"
	END Install;

	PROCEDURE (updater: Updater) ParamsSize (): INTEGER;
	BEGIN
		RETURN 0
	END ParamsSize;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			i, size: INTEGER;
			cos, sin, twoPi, theta, thetaMax, thetaMin, x, y, logLikelihood: REAL;
			prior: GraphMultivariate.Node;
	BEGIN
		res := {};
		prior := updater.prior[0](GraphMultivariate.Node);
		size := updater.Size();
		twoPi := 2.0 * Math.Pi();
		i := 0; WHILE i < size DO oldX[i] := updater.prior[i].value; INC(i) END;
		prior.MVPriorForm(mu, tau);
		IF prior.ClassifyPrior() = GraphRules.mVNSigma THEN
			MathRandnum.MNormalCovar(tau, size, nu)
		ELSE
			MathMatrix.Cholesky(tau, size);
			MathRandnum.MNormalPrec(tau, size, nu)
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
		END
	END Sample;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		VAR
			class: INTEGER;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		class := prior.ClassifyPrior();
		IF (class # GraphRules.mVN) & (class # GraphRules.mVNLin) THEN RETURN FALSE END;
		IF ~(prior IS GraphConjugateMV.Node) & ~(prior IS GraphChain.Node) THEN
			RETURN FALSE
		END;
		IF prior IS GraphMRF.Node THEN RETURN FALSE END;
		IF prior.depth = 1 THEN RETURN FALSE END;
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
		install := "UpdaterElliptical.Install"
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
		NEW(oldX, size);
		NEW(tau, size, size)
	END Init;

BEGIN
	Init
END UpdaterElliptical.
