(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterForward;


	

	IMPORT
		Stores := Stores64,
		BugsRegistry,
		GraphLogical, GraphMultivariate, GraphNodes, GraphStochastic, GraphUnivariate,
		UpdaterMultivariate, UpdaterUnivariate, UpdaterUpdaters;

	TYPE
		UpdaterUV = POINTER TO RECORD(UpdaterUnivariate.Updater) END;

		UpdaterMV = POINTER TO RECORD(UpdaterMultivariate.Updater) END;

		FactoryUV = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

		FactoryMV = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		factUV-, factMV-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (updater: UpdaterUV) Clone (): UpdaterUV;
		VAR
			u: UpdaterUV;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: UpdaterUV) CopyFromUnivariate (source: UpdaterUpdaters.Updater);
	BEGIN
	END CopyFromUnivariate;

	PROCEDURE (updater: UpdaterUV) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeUnivariate;

	PROCEDURE (updater: UpdaterUV) GenerateInit (fixFounder: BOOLEAN; OUT res: SET);
		VAR
			prior: GraphStochastic.Node;
		CONST
			univariate = FALSE;
	BEGIN
		prior := updater.prior;
		IF GraphStochastic.initialized IN prior.props THEN RETURN END;
		ASSERT(prior.CanSample(univariate), 21);
		prior.Sample(res);
		IF res # {} THEN RETURN END;
		INCL(prior.props, GraphStochastic.initialized)
	END GenerateInit;

	PROCEDURE (updater: UpdaterUV) InitializeUnivariate;
	BEGIN
	END InitializeUnivariate;

	PROCEDURE (updater: UpdaterUV) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeUnivariate;

	PROCEDURE (updater: UpdaterUV) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterForward.InstallUV"
	END Install;

	PROCEDURE (updater: UpdaterUV) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: UpdaterUV) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.prior;
		prior.Sample(res);
		prior.Evaluate
	END Sample;

	PROCEDURE (updater: UpdaterMV) Clone (): UpdaterMV;
		VAR
			u: UpdaterMV;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: UpdaterMV) CopyFromMultivariate (source: UpdaterUpdaters.Updater);
	BEGIN
	END CopyFromMultivariate;

	PROCEDURE (updater: UpdaterMV) ExternalizeMultivariate (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeMultivariate;

	PROCEDURE (updater: UpdaterMV) FindBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
	BEGIN
		RETURN prior(GraphMultivariate.Node).components
	END FindBlock;

	PROCEDURE (updater: UpdaterMV) InitializeMultivariate;
	BEGIN
	END InitializeMultivariate;

	PROCEDURE (updater: UpdaterMV) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterForward.InstallMV"
	END Install;

	PROCEDURE (updater: UpdaterMV) InternalizeMultivariate (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeMultivariate;

	PROCEDURE (updater: UpdaterMV) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: UpdaterMV) ParamsSize (): INTEGER;
	BEGIN
		RETURN 0
	END ParamsSize;

	PROCEDURE (updater: UpdaterMV) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			prior: GraphMultivariate.Node;
	BEGIN
		prior := updater.prior[0](GraphMultivariate.Node);
		prior.MVSample(res);
		GraphLogical.Evaluate(updater.dependents)
	END Sample;

	PROCEDURE (f: FactoryUV) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		IF prior.children # NIL THEN RETURN FALSE END;
		IF prior IS GraphMultivariate.Node THEN RETURN FALSE END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: FactoryUV) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: UpdaterUV;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE (f: FactoryUV) GetDefaults;
		VAR
			res: INTEGER;
			props: SET;
			name: ARRAY 256 OF CHAR;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: FactoryUV) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterForward.InstallUV"
	END Install;

	PROCEDURE (f: FactoryMV) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		VAR
			components: GraphStochastic.Vector;
			i, size: INTEGER;
	BEGIN
		IF prior.children # NIL THEN RETURN FALSE END;
		IF prior IS GraphUnivariate.Node THEN RETURN FALSE END;
		i := 0;
		size := prior.Size();
		components := prior(GraphMultivariate.Node).components;
		WHILE (i < size) & (components[i].children = NIL) & ~(GraphNodes.data IN components[i].props) DO
			INC(i)
		END;
		IF i # size THEN RETURN FALSE END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: FactoryMV) GetDefaults;
		VAR
			res: INTEGER;
			props: SET;
			name: ARRAY 256 OF CHAR;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: FactoryMV) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterForward.InstallMV"
	END Install;

	PROCEDURE (f: FactoryMV) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: UpdaterMV;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE InstallUV*;
	BEGIN
		UpdaterUpdaters.SetFactory(factUV)
	END InstallUV;

	PROCEDURE InstallMV*;
	BEGIN
		UpdaterUpdaters.SetFactory(factMV)
	END InstallMV;

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
			f: FactoryUV;
			fMV: FactoryMV;
	BEGIN
		Maintainer;
		NEW(f);
		f.SetProps({UpdaterUpdaters.enabled, UpdaterUpdaters.hidden});
		f.Install(name);
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteSet(name + ".props", f.props)
		END;
		f.GetDefaults;
		factUV := f;
		NEW(fMV);
		fMV.SetProps({UpdaterUpdaters.enabled, UpdaterUpdaters.hidden});
		fMV.Install(name);
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteSet(name + ".props", fMV.props)
		END;
		fMV.GetDefaults;
		factMV := fMV
	END Init;

BEGIN
	Init
END UpdaterForward.
