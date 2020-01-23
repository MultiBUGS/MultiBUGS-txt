(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterSlice;

	

	IMPORT
		BugsRegistry,
		GraphRules, GraphStochastic,
		UpdaterSlicebase, UpdaterUpdaters;

	TYPE
		Updater = POINTER TO RECORD(UpdaterSlicebase.Updater) END;

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

	PROCEDURE (updater: Updater) LogLikelihoodOpt (): REAL;
		VAR
			logLike: REAL;
	BEGIN
		updater.prior.Evaluate;
		logLike := updater.LogLikelihood();
		RETURN logLike
	END LogLikelihoodOpt;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterSlice.Install"
	END Install;

	PROCEDURE (updater: Updater) Setup;
	BEGIN
		UpdaterSlicebase.adaptivePhase := fact.adaptivePhase;
		UpdaterSlicebase.maxIterations := fact.iterations
	END Setup;
	
	PROCEDURE (updater: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterSlice.Install"
	END Install;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF prior.ClassifyPrior() = GraphRules.wishart THEN RETURN FALSE END;
		IF prior.ClassifyPrior() = GraphRules.dirichlet THEN RETURN FALSE END;
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
