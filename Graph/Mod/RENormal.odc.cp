(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphRENormal;


	

	IMPORT
		Stores := Stores64,
		GraphMRF,
		GraphMultivariate, GraphNodes, GraphRules, GraphStochastic, GraphUVGMRF,
		MathFunc;

	TYPE
		Node = POINTER TO ABSTRACT RECORD(GraphUVGMRF.Node) END;

		StdNode = POINTER TO RECORD(Node) END;

		ConsNode = POINTER TO RECORD(Node) END;

		StdFactory = POINTER TO RECORD (GraphMultivariate.Factory) END;

		ConsFactory = POINTER TO RECORD (GraphMultivariate.Factory) END;

	VAR
		factStd-, factCons-: GraphMultivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) CheckUVMRF (): SET;
	BEGIN
		RETURN {}
	END CheckUVMRF;

	PROCEDURE (node: Node) ClassifyLikelihoodUVMRF (parent: GraphStochastic.Node): INTEGER;
	BEGIN
		RETURN GraphRules.unif
	END ClassifyLikelihoodUVMRF;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.normal
	END ClassifyPrior;

	PROCEDURE (node: Node) ExternalizeUVMRF (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeUVMRF;

	PROCEDURE (node: Node) InitUVMRF;
	BEGIN
	END InitUVMRF;

	PROCEDURE (node: Node) InternalizeUVMRF (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeUVMRF;

	PROCEDURE (node: Node) ParentsUVMRF (all: BOOLEAN): GraphNodes.List;
	BEGIN
		RETURN NIL
	END ParentsUVMRF;

	PROCEDURE (node: Node) SetUVMRF (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {}
	END SetUVMRF;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
		VAR
			y: REAL;
	BEGIN
		ASSERT(as = GraphRules.gamma, 21);
		p0 := 0.50;
		y := node.value;
		p1 := 0.5 * y * y;
		x := node.tau
	END LikelihoodForm;

	PROCEDURE (node: Node) LogDet (): REAL;
	BEGIN
		RETURN 0.0
	END LogDet;

	PROCEDURE (node: Node) MatrixElements (OUT elements: ARRAY OF REAL);
		VAR
			tau: REAL;
			i, size: INTEGER;
	BEGIN
		tau := node.tau.Value();
		i := 0;
		size := node.Size();
		WHILE i < size DO
			elements[i] := tau;
			INC(i)
		END
	END MatrixElements;

	PROCEDURE (node: Node) MatrixInfo (OUT type, size: INTEGER);
	BEGIN
		type := GraphMRF.diagonal;
		size := node.Size()
	END MatrixInfo;

	PROCEDURE (node: Node) MatrixMap (OUT rowInd, colPtr: ARRAY OF INTEGER);
		VAR
			i, size: INTEGER;
	BEGIN
		size := node.Size();
		i := 0;
		WHILE i < size DO
			rowInd[i] := i;
			colPtr[i] := i;
			INC(i)
		END;
	END MatrixMap;

	PROCEDURE (node: Node) MVPriorForm (OUT p0: ARRAY OF REAL;
	OUT p1: ARRAY OF ARRAY OF REAL);
	BEGIN
	END MVPriorForm;

	PROCEDURE (node: Node) NumberNeighbours (): INTEGER;
	BEGIN
		RETURN 0
	END NumberNeighbours;

	PROCEDURE (prior: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		ASSERT(as = GraphRules.normal, 21);
		p0 := 0.0;
		p1 := prior.tau.Value()
	END PriorForm;

	PROCEDURE (node: StdNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphRENormal.StdInstall"
	END Install;

	PROCEDURE (node: StdNode) NumberConstraints (): INTEGER;
	BEGIN
		RETURN 0
	END NumberConstraints;

	PROCEDURE (node: ConsNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphRENormal.ConsInstall"
	END Install;

	PROCEDURE (node: ConsNode) NumberConstraints (): INTEGER;
	BEGIN
		RETURN 1
	END NumberConstraints;

	PROCEDURE (f: StdFactory) New (): GraphMultivariate.Node;
		VAR
			node: StdNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: StdFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "s"
	END Signature;

	PROCEDURE (f: ConsFactory) New (): GraphMultivariate.Node;
		VAR
			node: ConsNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: ConsFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "s"
	END Signature;

	PROCEDURE StdInstall*;
	BEGIN
		GraphNodes.SetFactory(factStd)
	END StdInstall;

	PROCEDURE ConsInstall*;
	BEGIN
		GraphNodes.SetFactory(factCons)
	END ConsInstall;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			fStd: StdFactory;
			fCons: ConsFactory;
	BEGIN
		Maintainer;
		NEW(fStd);
		factStd := fStd;
		NEW(fCons);
		factCons := fCons
	END Init;

BEGIN
	Init
END GraphRENormal.

