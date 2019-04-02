(*	

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



		  *)

MODULE GraphCut;


	

	IMPORT
		Stores,
		GraphLogical, GraphNodes, GraphScalar, GraphStochastic;

	TYPE

		Node = POINTER TO RECORD(GraphScalar.Node)
			cutParent: GraphNodes.Node
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
		VAR
			class: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		p := node.cutParent;
		class := GraphStochastic.ClassFunction(p, parent);
		RETURN class
	END ClassFunction;

	PROCEDURE (node: Node) ExternalizeLogical (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.cutParent, wr)
	END ExternalizeLogical;

	PROCEDURE (node: Node) InternalizeLogical (VAR rd: Stores.Reader);
	BEGIN
		node.cutParent := GraphNodes.Internalize(rd)
	END InternalizeLogical;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.cutParent := NIL
	END InitLogical;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphCut.Install"
	END Install;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		IF all THEN
			p := node.cutParent;
			p.AddParent(list)
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.cutParent := args.scalars[0]
		END
	END Set;

	PROCEDURE (node: Node) Value (): REAL;
		VAR
			value: REAL;
			p: GraphNodes.Node;
	BEGIN
		p := node.cutParent;
		value := p.Value();
		RETURN value
	END Value;

	PROCEDURE (node: Node) ValDiff (x: GraphNodes.Node; OUT val, diff: REAL);
		VAR
			p: GraphNodes.Node;
	BEGIN
		p := node.cutParent;
		p.ValDiff(x, val, diff)
	END ValDiff;

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
END GraphCut.
