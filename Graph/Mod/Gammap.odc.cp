(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphGammap;


	

	IMPORT
		Math, Stores := Stores64,
		GraphLogical, GraphNodes, GraphRules, GraphScalar, GraphStochastic,
		MathFunc;

	TYPE

		Node = POINTER TO RECORD(GraphScalar.Node)
			a, x: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD(GraphScalar.Factory) END;

	CONST
		eps = 1.0E-10;

	VAR
		fact-: GraphScalar.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) Check (): SET;
		VAR
			a, x: REAL;
	BEGIN
		a := node.a.value;
		IF a < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg1}
		END;
		x := node.x.value;
		IF x < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg2}
		END;
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			f: INTEGER;
	BEGIN
		f := GraphStochastic.ClassFunction(node.a, parent);
		IF f # GraphRules.const THEN
			RETURN GraphRules.other
		END;
		f := GraphStochastic.ClassFunction(node.x, parent);
		IF f # GraphRules.const THEN
			RETURN GraphRules.other
		END;
		RETURN f
	END ClassFunction;

	PROCEDURE (node: Node) Evaluate;
		VAR
			a, x, value: REAL;
	BEGIN
		a := node.a.value;
		x := node.x.value;
		node.value := MathFunc.GammaP(a, x);
	END Evaluate;

	PROCEDURE (node: Node) EvaluateDiffs;
	BEGIN
		HALT(126)
	END EvaluateDiffs;

	PROCEDURE (node: Node) ExternalizeScalar (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.a, wr);
		GraphNodes.Externalize(node.x, wr)
	END ExternalizeScalar;

	PROCEDURE (node: Node) InternalizeScalar (VAR rd: Stores.Reader);
		VAR
			p: GraphNodes.Node;
	BEGIN
		p := GraphNodes.Internalize(rd);
		node.a := p;
		p := GraphNodes.Internalize(rd);
		node.x := p;
	END InternalizeScalar;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.a := NIL;
		node.x := NIL
	END InitLogical;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphGammap.Install"
	END Install;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		p := node.a;
		p.AddParent(list);
		p := node.x;
		p.AddParent(list);
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.a := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			node.x := args.scalars[1]
		END
	END Set;

	PROCEDURE (f: Factory) New (): GraphScalar.Node;
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

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

BEGIN
	Init
END GraphGammap.



