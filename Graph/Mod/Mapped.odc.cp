(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphMapped;

	

	IMPORT
		Stores := Stores64,
		GraphMultivariate, GraphNodes, GraphRules, GraphScalar, GraphStochastic;

	TYPE

		Node = POINTER TO RECORD(GraphScalar.Node)
			x: GraphStochastic.Node
		END;

		Factory = POINTER TO RECORD(GraphScalar.Factory) END;

	VAR
		fact-: GraphScalar.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) Check (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;
	BEGIN
		RETURN GraphRules.other
	END ClassFunction;

	PROCEDURE (node: Node) Evaluate;
		VAR
			value: REAL;
			x: GraphStochastic.Node;
	BEGIN
		x := node.x;
		WITH x: GraphMultivariate.Node DO
			value := x.components[0].Map(); (*	needed for side effects	*)
		ELSE
		END;
		node.value := x.Map();
	END Evaluate;

	PROCEDURE (node: Node) EvaluateDiffs ;
	BEGIN
		HALT(126)
	END EvaluateDiffs;

	PROCEDURE (node: Node) ExternalizeScalar (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.x, wr)
	END ExternalizeScalar;

	PROCEDURE (node: Node) InternalizeScalar (VAR rd: Stores.Reader);
		VAR
			p: GraphNodes.Node;
	BEGIN
		p := GraphNodes.Internalize(rd);
		node.x := p(GraphStochastic.Node);
	END InternalizeScalar;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.x := NIL
	END InitLogical;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphMapped.Install"
	END Install;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		p := node.x;
		p.AddParent(list);
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			p: GraphNodes.Node;
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			p := args.scalars[0];
			ASSERT(p # NIL, 21);
			WITH p: GraphStochastic.Node DO
				IF GraphNodes.data IN p.props THEN
					res := {GraphNodes.arg1, GraphNodes.notStochastic}
				ELSIF GraphStochastic.integer IN p.props THEN
					res := {GraphNodes.arg1, GraphNodes.invalidValue}
				ELSE
					node.x := p
				END
			ELSE
				res := {GraphNodes.arg1, GraphNodes.notStochastic}
			END
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
		signature := "s"
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
END GraphMapped.



