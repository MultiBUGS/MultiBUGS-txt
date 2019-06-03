(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterUnivariateT;


	

	IMPORT
		Stores := Stores64,
		BugsRegistry,
		GraphNodes, GraphStochastic,
		UpdaterUnivariate, UpdaterUpdaters;


	TYPE
		Updater = POINTER TO RECORD(UpdaterUnivariate.Updater)
			(*	put any required fields in here	*)
		END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

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
	BEGIN
	END CopyFromUnivariate;

	PROCEDURE (updater: Updater) GenerateInit (fixFounder: BOOLEAN; OUT res: SET);
		VAR
			prior: GraphStochastic.Node;
		CONST
			univariate = FALSE;
	BEGIN
		prior := updater.prior;
		IF ~prior.CanSample(univariate) THEN res := {GraphNodes.lhs}; RETURN END;
		prior.Sample(res);
		IF res # {} THEN RETURN END;
		prior.SetProps(prior.props + {GraphStochastic.initialized})
	END GenerateInit;

	PROCEDURE (updater: Updater) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeUnivariate;

	PROCEDURE (updater: Updater) InitializeUnivariate;
	BEGIN
	END InitializeUnivariate;

	PROCEDURE (updater: Updater) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeUnivariate;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterUnivariateT.Install"
	END Install;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		(*	calculate if sampler is adapting at present	*)
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			rand: REAL;
			prior: GraphStochastic.Node;
	BEGIN
		res := {};
		prior := updater.prior;
		(*	generate new random number here and call it rand	*)
		prior.SetValue(rand)
	END Sample;

	PROCEDURE (f: Factory) GetDefaults;
		VAR
			name: ARRAY 256 OF CHAR;
			props: SET;
			iterations, res: INTEGER;
	BEGIN
		f.Install(name);
		(*	get default parameters, if any, from the BUGS registry	*)
		BugsRegistry.ReadInt(name + ".iterations", iterations, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterUnivariateT.Install"
	END Install;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		(*	if not able to (or unwilling to) update node prior return NIL	*)
		IF prior.children = NIL THEN RETURN FALSE END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: Factory) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: Updater;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE Maintainer;
	BEGIN
		(*	version number and name of maintainer	*)
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
		fact := f;
		(*	indicate which parameters are valid for updater factory	*)
		fact.SetProps({UpdaterUpdaters.iterations});
		f.Install(name);
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
			(*	updater factory already in BUGS registry	*)
		ELSE
			(*	set updater factory parameters to inital default values in the BUGS registry	*)
			BugsRegistry.WriteInt(name + ".iterations", 100000);
			BugsRegistry.WriteSet(name + ".props", f.props)
		END;
		(*	read updater factory parameters from BUGS registry	*)
		f.GetDefaults
	END Init;

	PROCEDURE Install*;
	BEGIN
		UpdaterUpdaters.SetFactory(fact);
		fact.GetDefaults
	END Install;

BEGIN
	Init
END UpdaterUnivariateT.
