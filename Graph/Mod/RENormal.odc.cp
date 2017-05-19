(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphRENormal;


	

	IMPORT
		GraphMRF, GraphUVGMRF, GraphMultivariate, GraphNodes, GraphRules, GraphStochastic;

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
		start, step: INTEGER;
		
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
			i, size: INTEGER;
			b: GraphStochastic.Vector;
			y: REAL;
	BEGIN
		ASSERT(as = GraphRules.gamma, 21);
		ASSERT(node.index = 0, 21);
		size := node.Size();
		b := node.components;
		p0 := 0.50 * size;
		p1 := 0.0;
		i := 0;
		WHILE i < size DO
			y := b[i].value;
			p1 := p1 + 0.5 * y * y;
			INC(i)
		END;
		x := node.tau
	END LikelihoodForm;

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

	PROCEDURE (prior: Node) ThinLikelihood (first, thin: INTEGER);
	BEGIN
		start := first;
		step := thin
	END ThinLikelihood;
	
	PROCEDURE (node: StdNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphRENormal.StdInstall"
	END Install;

	PROCEDURE (node: StdNode) Modify (): GraphStochastic.Node;
		VAR
			p: StdNode;
	BEGIN
		NEW(p);
		p^ := node^;
		p.TransformParams;
		RETURN p
	END Modify;

	PROCEDURE (node: StdNode) NumberConstraints (): INTEGER;
	BEGIN
		RETURN 0
	END NumberConstraints;

	PROCEDURE (node: ConsNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphRENormal.ConsInstall"
	END Install;

	PROCEDURE (node: ConsNode) Modify (): GraphStochastic.Node;
		VAR
			p: ConsNode;
	BEGIN
		NEW(p);
		p^ := node^;
		p.TransformParams;
		RETURN p
	END Modify;

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
		start := 0;
		step := 1;
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
		start := 0;
		step := 1;
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
		factCons := fCons;
		start := 0;
		step := 1
	END Init;

BEGIN
	Init
END GraphRENormal.

