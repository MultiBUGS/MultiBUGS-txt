(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterSCDE;


	

	IMPORT
		Math, Stores := Stores64,
		BugsRegistry,
		GraphRules, GraphStochastic,
		MathRandnum,
		UpdaterActions, UpdaterMetropolisUV, UpdaterUpdaters;

	CONST
		batch = 100;

	TYPE
		Updater = POINTER TO ABSTRACT RECORD (UpdaterMetropolisUV.Updater)
			chain, index: INTEGER;
		END;


		UpdaterMet = POINTER TO RECORD(Updater) END;


		UpdaterMAP = POINTER TO RECORD(Updater) END;

		FactoryMet = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

		FactoryMAP = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		factMet-, factMAP-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE BuildProposal (updater: Updater);
		VAR
			i, numStoch: INTEGER;
			stochastics: GraphStochastic.Vector;
	BEGIN
		UpdaterActions.FindUpdater(updater, updater.chain, i);
		stochastics := GraphStochastic.nodes;
		IF stochastics # NIL THEN
			numStoch := LEN(stochastics);
			i := 0;
			WHILE (i < numStoch) & (updater.prior # stochastics[i]) DO INC(i) END;
			updater.index := i
		END
	END BuildProposal;
	
	PROCEDURE SampleProposal (updater: Updater);
		VAR
			chain, r1, r2, numChains: INTEGER;
			gamma, x, xMap, xR1, xR2: REAL;
			prior: GraphStochastic.Node;
		CONST
			b = 1.0E-4;
	BEGIN
		prior := updater.prior;
		numChains := UpdaterActions.NumberChains();
		chain := updater.chain;
		ASSERT(numChains >= 3, 21);
		gamma := 2.38 / Math.Sqrt(2);
		IF updater.iteration MOD 10 = 0 THEN gamma := gamma / 10.0 END;
		prior := updater.prior;
		xMap := prior.Map();
		REPEAT r1 := MathRandnum.DiscreteUniform(0, numChains - 1) UNTIL r1 # chain;
		REPEAT r2 := MathRandnum.DiscreteUniform(0, numChains - 1) UNTIL (r2 # chain) & (r2 # r1);
		prior.value := GraphStochastic.values[r1, updater.index];
		xR1 := prior.Map();
		prior.value := GraphStochastic.values[r2, updater.index];
		xR2 := prior.Map();
		x := xMap + gamma * (xR1 - xR2) + MathRandnum.Normal(0, b);
		prior.InvMap(x);
	END SampleProposal;

	PROCEDURE (updater: Updater) CopyFromMetropolisUV (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
	BEGIN
		s := source(Updater);
		updater.chain := s.chain;
		updater.index := s.index
	END CopyFromMetropolisUV;

	PROCEDURE (updater: Updater) ExternalizeMetropolis (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteInt(updater.chain);
		wr.WriteInt(updater.index)
	END ExternalizeMetropolis;

	PROCEDURE (updater: Updater) InternalizeMetropolis (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadInt(updater.chain);
		rd.ReadInt(updater.index)
	END InternalizeMetropolis;

	PROCEDURE (updater: Updater) InitializeMetropolis;
	BEGIN
		updater.chain :=  - 1;
		updater.index := -1
	END InitializeMetropolis;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) MetTest (): REAL, NEW, ABSTRACT;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			logAlpha, newLogDen, newVal, oldLogDen: REAL;
			prior: GraphStochastic.Node;
	BEGIN
		IF updater.index =  -1 THEN BuildProposal(updater) END;
		prior := updater.prior;
		updater.Store;
		oldLogDen := updater.LogConditional() + prior.LogDetJacobian();
		SampleProposal(updater);
		prior.Evaluate;
		newLogDen := updater.LogConditional() + prior.LogDetJacobian();
		logAlpha := newLogDen - oldLogDen;
		IF logAlpha < updater.MetTest() THEN
			updater.Restore;
			INC(updater.rejectCount);
		END;
		INC(updater.iteration);
		IF updater.iteration MOD batch = 0 THEN
			updater.rejectCount := 0
		END;
		res := {}
	END Sample;

	PROCEDURE (updater: UpdaterMet) Clone (): UpdaterMet;
		VAR
			u: UpdaterMet;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: UpdaterMet) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterSCDE.InstallMet"
	END Install;

	PROCEDURE (updater: UpdaterMet) MetTest (): REAL;
	BEGIN
		RETURN Math.Ln(MathRandnum.Rand())
	END MetTest;

	PROCEDURE (updater: UpdaterMAP) Clone (): UpdaterMAP;
		VAR
			u: UpdaterMAP;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: UpdaterMAP) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterSCDE.InstallMAP"
	END Install;

	PROCEDURE (updater: UpdaterMAP) MetTest (): REAL;
	BEGIN
		RETURN 0.0
	END MetTest;

	PROCEDURE (f: FactoryMet) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF GraphStochastic.bounds * prior.props # {} THEN RETURN FALSE END;
		IF prior.ClassifyPrior() = GraphRules.wishart THEN RETURN FALSE END;
		IF prior.ClassifyPrior() = GraphRules.dirichlet THEN RETURN FALSE END;
		IF prior.children = NIL THEN RETURN FALSE END;
		IF ~(prior.classConditional IN {GraphRules.general, GraphRules.genDiff}) THEN RETURN FALSE END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: FactoryMet) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: UpdaterMet;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE (f: FactoryMet) GetDefaults;
		VAR
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
			props: SET;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadSet(name + ".props", props, res);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: FactoryMet) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterSCDE.InstallMet"
	END Install;

	PROCEDURE (f: FactoryMAP) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF prior.children = NIL THEN RETURN FALSE END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: FactoryMAP) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: UpdaterMAP;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE (f: FactoryMAP) GetDefaults;
		VAR
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
			props: SET;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadSet(name + ".props", props, res);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: FactoryMAP) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterSCDE.InstallMAP"
	END Install;

	PROCEDURE InstallMet*;
	BEGIN
		UpdaterUpdaters.SetFactory(factMet)
	END InstallMet;

	PROCEDURE InstallMAP*;
	BEGIN
		UpdaterUpdaters.SetFactory(factMAP)
	END InstallMAP;

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
			fMet: FactoryMet;
			fMAP: FactoryMAP;
	BEGIN
		Maintainer;
		NEW(fMet);
		fMet.SetProps({});
		fMet.Install(name);
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteSet(name + ".props", fMet.props)
		END;
		fMet.GetDefaults;
		factMet := fMet;
		NEW(fMAP);
		fMAP.SetProps({});
		fMAP.Install(name);
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteSet(name + ".props", fMAP.props)
		END;
		fMAP.GetDefaults;
		factMAP := fMAP
	END Init;

BEGIN
	Init
END UpdaterSCDE.
