(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphHalf;

	

	IMPORT
		Stores,
		GraphLogical, GraphNodes, GraphRules, GraphScalar, GraphStochastic;

	TYPE

		Node = POINTER TO RECORD(GraphScalar.Node)
			x: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD(GraphNodes.Factory) END;
		
	VAR
		fact: GraphNodes.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
	
	PROCEDURE (node: Node) Check (): SET;
	BEGIN
		RETURN {}
	END Check;
	
	PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			class: INTEGER;
	BEGIN
		class := GraphRules.other;
		IF node.x IS GraphLogical.Node THEN
			class := node.x(GraphLogical.Node).ClassFunction(parent);
			IF class = GraphRules.ident THEN class := GraphRules.prod END
		ELSIF node.x IS GraphStochastic.Node THEN
			IF node.x = parent THEN
				class := GraphRules.prod
			ELSE
				class := GraphRules.const
			END
		END;
		RETURN class
	END ClassFunction;
	
	PROCEDURE (node: Node) ExternalizeLogical (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.x, wr)
	END ExternalizeLogical;
	
	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.x := NIL
	END InitLogical;
		
	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphHalf.Install"
	END Install;
	
	PROCEDURE (node: Node) InternalizeLogical (VAR rd: Stores.Reader);
	BEGIN
		node.x := GraphNodes.Internalize(rd)
	END InternalizeLogical;
	
	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		IF node.x IS GraphStochastic.Node THEN
			list := NIL;
			node.x.AddParent(list)
		ELSE
			list := node.x.Parents(all)
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;
	
	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
	
	END Set;
	
	PROCEDURE (node: Node) ValDiff (x: GraphNodes.Node; OUT val, diff: REAL);
	BEGIN
		IF node.x IS GraphStochastic.Node THEN
			val := 0.5 * node.x.Value();
			IF node.x = x THEN
				diff := 0.5
			ELSE
				diff := 0.0
			END
		ELSE
			node.x.ValDiff(x, val, diff);
			val := 0.5 * val;
			diff := 0.5 * diff
		END
	END ValDiff;
	
	PROCEDURE (node: Node) Value (): REAL;
	BEGIN
		RETURN 0.5 * node.x.Value()
	END Value;

	PROCEDURE (f: Factory) New (): GraphScalar.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: Factory) NumParam (): INTEGER;
	BEGIN
		RETURN 0
	END NumParam;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := ""
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
	
	PROCEDURE New* (x: GraphNodes.Node): GraphNodes.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		node.x := x;
		RETURN node
	END New;

BEGIN
	Init		
END GraphHalf.
