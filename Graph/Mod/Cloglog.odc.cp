(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphCloglog;


	

	IMPORT
		Math,
		GraphLinkfunc, GraphNodes, GraphRules, GraphScalar, GraphStochastic;

	TYPE
		Node = POINTER TO RECORD(GraphLinkfunc.Node) END;

		Factory = POINTER TO RECORD(GraphLinkfunc.Factory) END;

	VAR
		fact-: GraphLinkfunc.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			class: INTEGER;
			predictor: GraphNodes.Node;
	BEGIN
		predictor := node.predictor;
		class := GraphStochastic.ClassFunction(predictor, parent);
		class := GraphRules.cloglogF[class];
		RETURN class
	END ClassFunction;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphCloglog.Install"
	END Install;

	PROCEDURE (node: Node) Evaluate;
		VAR
			value: REAL;
			predictor: GraphNodes.Node;
	BEGIN
		predictor := node.predictor;
		value := predictor.value;
		value := 1.0 - Math.Exp( - Math.Exp(value));
		node.value := value
	END Evaluate;

	PROCEDURE (node: Node) EvaluateDiffs;
		VAR
			predictor: GraphNodes.Node;
			x: GraphNodes.Vector;
			temp, temp1, val: REAL;
			i, N: INTEGER;
	BEGIN
		x := node.diffWRT;
		N := LEN(x);
		predictor := node.predictor;
		val := predictor.value;
		temp := Math.Exp(val);
		temp1 := Math.Exp( - temp);
		i := 0;
		WHILE i < N DO
			node.diffs[i] := predictor.Diff(x[i]) * temp * temp1;
			INC(i)
		END;
		node.value := 1.0 - temp1
	END EvaluateDiffs;

	PROCEDURE (f: Factory) New (): GraphLinkfunc.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

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
		fact := f
	END Init;

BEGIN
	Init
END GraphCloglog.
