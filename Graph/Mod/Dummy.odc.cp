(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	  *)

MODULE GraphDummy;


	

	IMPORT
		Stores,
		GraphConjugateUV, GraphNodes, GraphRules, GraphStochastic, GraphUnivariate;

	TYPE
		Node = POINTER TO RECORD(GraphConjugateUV.Node) END;

		Factory = POINTER TO RECORD(GraphUnivariate.Factory) END;

	VAR
		fact-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
	BEGIN
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
	BEGIN
		RETURN GraphRules.unif
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.unif
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (x: REAL): REAL;
	BEGIN
		RETURN 0.0
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
	BEGIN
		RETURN 0.0
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
	BEGIN
		RETURN 0.0
	END DiffLogPrior;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphDummy.Install"
	END Install;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
	BEGIN
		RETURN 0.0
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
	BEGIN
		RETURN 0.0
	END LogPrior;

	PROCEDURE (node: Node) Location (): REAL;
	BEGIN
		RETURN 0.0
	END Location;

	PROCEDURE (node: Node) BoundsUnivariate (OUT left, upper: REAL);
	BEGIN
		left :=  - INF;
		upper := INF
	END BoundsUnivariate;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.noMean, GraphStochastic.initialized,
		GraphStochastic.hidden, GraphStochastic.update})
	END InitUnivariate;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		HALT(0)
	END LikelihoodForm;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
	BEGIN
		RETURN NIL
	END ParentsUnivariate;

	PROCEDURE (prior: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		ASSERT(as = GraphRules.normal, 21);
		p0 := 0.0;
		p1 := 0.0
	END PriorForm;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {}
	END SetUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
	BEGIN
		HALT(0)
	END Sample;

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
		signature := ""
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f;
	END Init;

BEGIN
	Init
END GraphDummy.

