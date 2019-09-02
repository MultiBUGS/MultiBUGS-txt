(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterPoisson;


	

	IMPORT
		Stores := Stores64,
		BugsRegistry,
		GraphConjugateUV, GraphNodes, GraphRules, GraphStochastic,
		MathRandnum,
		UpdaterUnivariate, UpdaterUpdaters;

	TYPE
		Updater = POINTER TO RECORD(UpdaterUnivariate.Updater) END;

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

	PROCEDURE (updater: Updater) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeUnivariate;

	(*	this is wrong because value must be greater than observed binomial count of liklelihood	*)
	PROCEDURE (updater: Updater) GenerateInit (fixFounder: BOOLEAN; OUT res: SET);
		VAR
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.prior;
		prior.Sample(res);
		IF res # {} THEN RETURN END;
		INCL(prior.props, GraphStochastic.initialized)
	END GenerateInit;

	PROCEDURE (updater: Updater) InitializeUnivariate;
	BEGIN
	END InitializeUnivariate;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterPoisson.Install"
	END Install;

	PROCEDURE (updater: Updater) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeUnivariate;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			lambda, q, n, value: REAL;
			prior, likelihood: GraphConjugateUV.Node;
			children: GraphStochastic.Vector;
			as: INTEGER;
			node: GraphNodes.Node;
	BEGIN
		prior := updater.prior(GraphConjugateUV.Node);
		as := GraphRules.poisson;
		prior.PriorForm(as, lambda, q);
		children := prior.children;
		likelihood := children[0](GraphConjugateUV.Node);
		likelihood.LikelihoodForm(as, node, q, n);
		lambda := lambda * q;
		value := MathRandnum.Poisson(lambda);
		prior.value := n + value; prior.Evaluate;
		res := {}
	END Sample;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		VAR
			children: GraphStochastic.Vector;
	BEGIN
		IF ~(GraphStochastic.integer IN prior.props) THEN
			RETURN FALSE
		END;
		IF prior.classConditional # GraphRules.poisson THEN
			RETURN FALSE
		END;
		children := prior.children;
		IF children = NIL THEN
			RETURN FALSE
		ELSIF LEN(children) # 1 THEN
			RETURN FALSE
		END;
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
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
			props: SET;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadSet(name + ".props", props, res);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterPoisson.Install"
	END Install;

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
		f.SetProps({UpdaterUpdaters.enabled});
		f.Install(name);
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteSet(name + ".props", f.props)
		END;
		f.GetDefaults;
		fact := f
	END Init;

BEGIN
	Init
END UpdaterPoisson.
