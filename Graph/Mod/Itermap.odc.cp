(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphItermap;


	

	IMPORT
		Stores := Stores64,
		GraphLogical, GraphNodes, GraphRules, GraphStochastic, GraphVector;

	TYPE
		Node = POINTER TO RECORD(GraphVector.Node)
			function, x, x0: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD(GraphVector.Factory) END;

	VAR
		fact-: GraphVector.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) Check (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			form0, form1: INTEGER;
			p: GraphNodes.Node;
			stochastic: GraphStochastic.Node;
	BEGIN
		stochastic := parent(GraphStochastic.Node);
		form1 := GraphRules.const;
		p := node.function;
		form0 := GraphStochastic.ClassFunction(p, stochastic);
		IF form0 # GraphRules.const THEN
			form1 := GraphRules.other
		END;
		p := node.x0;
		form0 := GraphStochastic.ClassFunction(p, stochastic);
		IF form0 # GraphRules.const THEN
			form1 := GraphRules.other
		END;
		RETURN form1
	END ClassFunction;

	PROCEDURE (node: Node) Evaluate;
		VAR
			i, size: INTEGER;
			x0: REAL;
			stochastic: GraphStochastic.Node;
	BEGIN
		IF node.index = 0 THEN
			size := node.Size();
			x0 := node.x0.value;
			i := 0;
			stochastic := node.x(GraphStochastic.Node);
			WHILE i < size DO
				stochastic.value := x0;
				node.components[i].value := node.function.value;
				x0 := node.function.value;
				INC(i)
			END
		END
	END Evaluate;

	PROCEDURE (node: Node) EvaluateDiffs;
	BEGIN
		HALT(126)
	END EvaluateDiffs;

	PROCEDURE (node: Node) ExternalizeVector (VAR wr: Stores.Writer);
	BEGIN
		IF node.index = 0 THEN
			GraphNodes.Externalize(node.function, wr);
			GraphNodes.Externalize(node.x, wr);
			GraphNodes.Externalize(node.x0, wr)
		END
	END ExternalizeVector;

	PROCEDURE (node: Node) InternalizeVector (VAR rd: Stores.Reader);
		VAR
			p: Node;
	BEGIN
		IF node.index = 0 THEN
			node.function := GraphNodes.Internalize(rd);
			node.x := GraphNodes.Internalize(rd);
			node.x0 := GraphNodes.Internalize(rd);
		END;
		p := node.components[0](Node);
		node.function := p.function;
		node.x := p.x;
		node.x0 := p.x0;
	END InternalizeVector;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.function := NIL;
		node.x := NIL;
		node.x0 := NIL
	END InitLogical;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphItermap.Install"
	END Install;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		p := node.function;
		p.AddParent(list);
		p := node.x0;
		p.AddParent(list);
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			node.function := args.scalars[0];
			node.x := args.scalars[1];
			node.x0 := args.scalars[2];
			node.x.props := node.x.props + {GraphStochastic.hidden, GraphStochastic.initialized}
		END
	END Set;

	PROCEDURE (f: Factory) New (): GraphVector.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;


	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "Fs"
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
		fact := f
	END Init;

BEGIN
	Init
END GraphItermap.

