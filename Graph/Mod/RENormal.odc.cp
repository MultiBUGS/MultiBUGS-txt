(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphRENormal;


	

	IMPORT
		Stores,
		GraphMRF,
		GraphMultivariate, GraphNodes, GraphRules, GraphStochastic, GraphUVMRF,
		MathFunc;

	TYPE
		Node = POINTER TO ABSTRACT RECORD(GraphUVMRF.Normal) END;

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

	PROCEDURE (node: Node) ExternalizeChainUVMRF (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeChainUVMRF;

	PROCEDURE (node: Node) InitStochasticUVMRF;
	BEGIN
	END InitStochasticUVMRF;

	PROCEDURE (node: Node) InternalizeChainUVMRF (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeChainUVMRF;

	PROCEDURE (node: Node) ParentsUVMRF (all: BOOLEAN): GraphNodes.List;
	BEGIN
		RETURN NIL
	END ParentsUVMRF;

	PROCEDURE (node: Node) SetUVMRF (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {}
	END SetUVMRF;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			tau, x: REAL;
	BEGIN
		x := node.value;
		tau := node.tau.Value();
		RETURN - tau * x
	END DiffLogPrior;

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

	PROCEDURE (node: Node) LogLikelihood (): REAL;
		VAR
			as: INTEGER;
			lambda, logLikelihood, logTau, r, tau: REAL;
			x: GraphNodes.Node;
	BEGIN
		as := GraphRules.gamma;
		node.LikelihoodForm(as, x, r, lambda);
		tau := x.Value();
		logTau := MathFunc.Ln(tau);
		logLikelihood := r * logTau - tau * lambda;
		RETURN logLikelihood
	END LogLikelihood;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			tau, x: REAL;
	BEGIN
		x := node.value;
		tau := node.tau.Value();
		RETURN - 0.50 * tau * x * x
	END LogPrior;

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

