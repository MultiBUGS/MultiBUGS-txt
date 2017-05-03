(*	

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

	When used as a prior on the precision is equivalent to a half T prior on the standard deviation
	
	Implemented as a mixture of gamma distributions
	
	x ~ dgamma(r, lambda)     lambda ~ dgamma(a, b)
	
*)


MODULE GraphHalfT;

	

	IMPORT
		Math, Stores, 
		GraphConjugateUV, GraphFlat, GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathRandnum, 
		UpdaterActions, UpdaterAuxillary, UpdaterUpdaters;

	TYPE
		Node = POINTER TO RECORD(GraphConjugateUV.Node)
			k, tau: GraphNodes.Node;
			y: GraphUnivariate.Node
		END;

		Auxillary = POINTER TO RECORD(UpdaterAuxillary.UpdaterUV) END;

		Factory = POINTER TO RECORD(GraphUnivariate.Factory) END;

		AuxillaryFactory = POINTER TO RECORD(UpdaterUpdaters.Factory) END;

	VAR
		fact-: GraphUnivariate.Factory;
		auxillaryFact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		
	PROCEDURE (updater: Auxillary) Clone (): Auxillary;
		VAR
			u: Auxillary;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Auxillary) CopyFromAuxillary (source: UpdaterUpdaters.Updater);
	BEGIN
	END CopyFromAuxillary;

	PROCEDURE (auxillary: Auxillary) ExternalizeAuxillary (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeAuxillary;

	PROCEDURE (auxillary: Auxillary) InternalizeAuxillary (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeAuxillary;

	PROCEDURE (auxillary: Auxillary) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphHalfT.AuxillaryInstall"
	END Install;

	PROCEDURE (auxillary: Auxillary) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			node: Node;
			k, tau, a, b, r, x, y: REAL;
	BEGIN
		res := {};
		node := auxillary.node(Node);
		k := node.k.Value();
		tau := node.tau.Value();
		r := 0.5 * k;
		a := 0.5;
		b := tau * k;
		x := node.value;
		IF node.likelihood # NIL THEN
			y :=  MathRandnum.Gamma(a + r, k * k * x + b)
		ELSE
			y := MathRandnum.Gamma(a, b)
		END;
		node.y.SetValue(y)
	END Sample;
	
	PROCEDURE (auxillary: Auxillary) UpdatedBy (index: INTEGER): GraphStochastic.Node;
	BEGIN
		RETURN auxillary.node(Node).y
	END UpdatedBy;

	PROCEDURE (f: AuxillaryFactory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: AuxillaryFactory) Create (): UpdaterUpdaters.Updater;
		VAR
			auxillary: Auxillary;
	BEGIN
		NEW(auxillary);
		RETURN auxillary
	END Create;

	PROCEDURE (f: AuxillaryFactory) GetDefaults;
	BEGIN
	END GetDefaults;

	PROCEDURE (f: AuxillaryFactory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphHalfT.AuxillaryInstall"
	END Install;

	PROCEDURE (node: Node) BoundsUnivariate (OUT lower, upper: REAL);
	BEGIN
		lower :=  0;
		upper := INF
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
		CONST
			eps = 1.0E-10;
		VAR
			k, tau: REAL;
	BEGIN
		k := node.k.Value();
		IF k < 2 THEN
			RETURN {GraphNodes.invalidValue, GraphNodes.arg1}
		END;
		IF ~(GraphNodes.data IN node.k.props) THEN
			RETURN {GraphNodes.notData, GraphNodes.arg1}
		END;
		tau := node.tau.Value();
		IF tau <  - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg2}
		END;
		IF ~(GraphNodes.data IN node.tau.props) THEN
			RETURN {GraphNodes.notData, GraphNodes.arg2}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
	BEGIN
		HALT(0);
		RETURN -1
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.gamma
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (r: REAL): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.k, wr);
		GraphNodes.Externalize(node.tau, wr);
		GraphNodes.Externalize(node.y, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.noPDF, GraphStochastic.noCDF,
		GraphStochastic.noMean});
		node.k := NIL;
		node.tau := NIL;
		node.y := NIL;
	END InitUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
		VAR
			p: GraphNodes.Node;
	BEGIN
		node.k := GraphNodes.Internalize(rd);
		node.tau := GraphNodes.Internalize(rd);
		p := GraphNodes.Internalize(rd);
		node.y := p(GraphUnivariate.Node)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphHalfT.Install"
	END Install;

	PROCEDURE (node: Node) Location (): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END Location;
	
	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node; OUT p0, p1: REAL);
	BEGIN
		HALT(0)
	END LikelihoodForm;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			logL, k, lambda, x, y, r: REAL;
	BEGIN
		x := node.value;
		y := node.y.value;
		k := node.k.Value();
		r := 0.5 * k;
		lambda := y * k* k;
		logL := (r - 1.0) * Math.Ln(x) - lambda * x;
		RETURN logL
	END LogPrior;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.y.AddParent(list);
		RETURN list
	END ParentsUnivariate;
	
	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
		VAR
			r, k, y: REAL;
	BEGIN
		k := node.k.Value();
		r := 0.5 * k;
		y := node.y.value;
		p0 := r;
		p1 := y * k * k
	END PriorForm;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			k, lambda, r, x, y: REAL;
	BEGIN
		res := {};
		k := node.k.Value();
		r := 0.5 * k;
		y := node.y.value;
		lambda := y * k * k;
		x := MathRandnum.Gamma(r, lambda);
		node.SetValue(x) 
	END Sample;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			auxillary: UpdaterUpdaters.Updater;
			y: GraphUnivariate.Node;
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			ASSERT(args.scalars[1] # NIL, 21);
			node.tau := args.scalars[0];
			node.k := args.scalars[1];
			y := GraphFlat.fact.New();
			(*	need to set y	*)
			y.Set(args, res);
			node.y := y;
			y.SetValue(1.0);
			y.SetProps(y.props + {GraphNodes.hidden, GraphStochastic.initialized});
			auxillary := auxillaryFact.New(node);
			UpdaterActions.RegisterUpdater(auxillary)
		END
	END SetUnivariate;


	PROCEDURE (node: Node) ModifyUnivariate (): GraphUnivariate.Node;
		VAR
			p: Node;
	BEGIN
		NEW(p);
		p^ := node^;
		RETURN p
	END ModifyUnivariate;

	PROCEDURE (f: Factory) New (): GraphUnivariate.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "ss"
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE AuxillaryInstall*;
	BEGIN
		UpdaterUpdaters.SetFactory(auxillaryFact)
	END AuxillaryInstall;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory;
			fAuxillary: AuxillaryFactory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f;
		NEW(fAuxillary);
		auxillaryFact := fAuxillary
	END Init;

BEGIN
	Init
END GraphHalfT.
