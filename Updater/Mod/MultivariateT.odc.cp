(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterMultivariateT;


	

	IMPORT
		Stores,
		BugsRegistry,
		GraphStochastic,
		UpdaterMultivariate, UpdaterUpdaters;

	TYPE
		Matrix = ARRAY OF ARRAY OF REAL;

		Vector = ARRAY OF REAL;

		Updater = POINTER TO RECORD (UpdaterMultivariate.Updater)
			newValue: POINTER TO ARRAY OF REAL;
			(*	put any required field in here	*)
		END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE FindBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
	BEGIN
		(*	given the prior calculate a set of nodes that should be updated as block
		if no such set can be found return NIL	*)
		RETURN NIL
	END FindBlock;

	PROCEDURE (updater: Updater) ParamsSize (): INTEGER;
	BEGIN
		RETURN 0
	END ParamsSize;

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

	PROCEDURE (updater: Updater) FindBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
	BEGIN
		RETURN FindBlock(prior)
	END FindBlock;

	PROCEDURE (updater: Updater) ExternalizeMultivariate (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeMultivariate;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterMultivariateT.Install"
	END Install;

	PROCEDURE (updater: Updater) InitializeMultivariate;
		VAR
			size: INTEGER;
	BEGIN
		(*	initialize any special fields here	*)
		size := updater.Size();
		NEW(updater.newValue, size)
	END InitializeMultivariate;

	PROCEDURE (updater: Updater) InternalizeMultivariate (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeMultivariate;

	PROCEDURE (updater: Updater) IsAdapting* (): BOOLEAN;
	BEGIN
		(*	calculate if sampler is adapting at present	*)
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) Sample* (overRelax: BOOLEAN; OUT res: SET);
	BEGIN
		(*	generate a vector of new values and put them into updater.newValues	*)
		updater.SetValue(updater.newValue);
		res := {}
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
		install := "UpdaterMultivariateT.Install"
	END Install;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		(*	if not able to (or unwilling to) update node prior return NIL	*)
		IF prior.likelihood = NIL THEN RETURN FALSE END;
		IF FindBlock(prior) = NIL THEN RETURN FALSE END;
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
END UpdaterMultivariateT.
