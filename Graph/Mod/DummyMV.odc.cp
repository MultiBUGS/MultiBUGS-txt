(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	  *)

MODULE GraphDummyMV;


	

	IMPORT
		Stores := Stores64,
		GraphMultivariate, GraphNodes, GraphRules, GraphStochastic;

	TYPE
		Node = POINTER TO RECORD(GraphMultivariate.Node) END;

		Factory = POINTER TO RECORD(GraphMultivariate.Factory) END;

	VAR
		fact-: GraphMultivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) Bounds (OUT left, upper: REAL);
	BEGIN
		left := - INF;
		upper := INF
	END Bounds;

	PROCEDURE (node: Node) CanSample (multiVar: BOOLEAN): BOOLEAN;
	BEGIN
		RETURN TRUE
	END CanSample;

	PROCEDURE (node: Node) Check (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassifyLikelihood (parent: GraphStochastic.Node): INTEGER;
	BEGIN
		RETURN GraphRules.unif
	END ClassifyLikelihood;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.unif
	END ClassifyPrior;

	PROCEDURE (node: Node) Deviance (): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END Deviance;
	
	PROCEDURE (node: Node) DiffLogConditional (): REAL;
	BEGIN
		RETURN 0.0
	END DiffLogConditional;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
	BEGIN
		RETURN 0.0
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
	BEGIN
		RETURN 0.0
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeMultivariate (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeMultivariate;

	PROCEDURE (node: Node) InternalizeMultivariate (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeMultivariate;

	PROCEDURE (node: Node) InitStochastic;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.noMean, GraphStochastic.initialized,
		GraphStochastic.hidden, GraphStochastic.update})
	END InitStochastic;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphDummyMV.Install"
	END Install;

	PROCEDURE (node: Node) InvMap (y: REAL);
	BEGIN
		node.SetValue(y)
	END InvMap;

	PROCEDURE (node: Node) LogDetJacobian (): REAL;
	BEGIN
		RETURN 0.0
	END LogDetJacobian;

	PROCEDURE (node: Node) LogLikelihood (): REAL;
	BEGIN
		RETURN 0.0
	END LogLikelihood;

	PROCEDURE (node: Node) LogPrior (): REAL;
	BEGIN
		RETURN 0.0
	END LogPrior;

	PROCEDURE (node: Node) Location (): REAL;
	BEGIN
		RETURN 0.0
	END Location;

	PROCEDURE (node: Node) Map (): REAL;
	BEGIN
		RETURN node.value
	END Map;

	PROCEDURE (node: Node) MVPriorForm (OUT p0: ARRAY OF REAL;
	OUT p1: ARRAY OF ARRAY OF REAL);
	BEGIN
	END MVPriorForm;
	
	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
	BEGIN
		RETURN NIL
	END Parents;

	PROCEDURE (node: Node) Representative (): GraphStochastic.Node;
	BEGIN
		RETURN node
	END Representative;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {}
	END Set;

	PROCEDURE (node: Node) Sample (OUT res: SET);
	BEGIN
		HALT(0)
	END Sample;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node; OUT p0, p1: REAL);
	BEGIN
		HALT(0)
	END LikelihoodForm;

	PROCEDURE (node: Node) LogMVPrior (): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END LogMVPrior;

	PROCEDURE (node: Node) MVSample (OUT res: SET);
	BEGIN
		HALT(0);
	END MVSample;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		HALT(0);
	END PriorForm;

	PROCEDURE (f: Factory) New (): GraphMultivariate.Node;
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
END GraphDummyMV.

