(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



		  *)

MODULE GraphWeight;


	

	IMPORT
		Stores := Stores64,
		GraphNodes, GraphRules, GraphScalar, GraphStochastic;

	TYPE

		Node = POINTER TO RECORD(GraphScalar.Node)
			cutParent, weight: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD(GraphScalar.Factory) END;

	VAR
		fact-: GraphScalar.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		removeWeights: BOOLEAN;

	PROCEDURE (node: Node) Check (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			class: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		p := node.weight;
		class := GraphStochastic.ClassFunction(p, parent);
		IF class # GraphRules.const THEN  RETURN GraphRules.other END;
		p := node.cutParent;
		class := GraphStochastic.ClassFunction(p, parent);
		RETURN class
	END ClassFunction;

	PROCEDURE (node: Node) ExternalizeLogical (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.cutParent, wr);
		GraphNodes.Externalize(node.weight, wr)
	END ExternalizeLogical;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.cutParent := NIL;
		node.weight := NIL;
		removeWeights := FALSE
	END InitLogical;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphWeight.Install"
	END Install;

	PROCEDURE (node: Node) InternalizeLogical (VAR rd: Stores.Reader);
	BEGIN
		node.cutParent := GraphNodes.Internalize(rd);
		node.weight := GraphNodes.Internalize(rd)
	END InternalizeLogical;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		p := node.cutParent;
		p.AddParent(list);
		p := node.weight;
		p.AddParent(list);
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.cutParent := args.scalars[0];
			node.weight := args.scalars[1]
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
		signature := "ss"
	END Signature;

	PROCEDURE GetNode* (node: GraphNodes.Node): GraphNodes.Node;
	BEGIN
		IF (node IS Node) & removeWeights THEN
			node := node(Node).cutParent
		END;
		RETURN node
	END GetNode;

	PROCEDURE GetWeight* (node: GraphNodes.Node): GraphNodes.Node;
	BEGIN
		RETURN node(Node).weight
	END GetWeight;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact);
		removeWeights := FALSE
	END Install;

	PROCEDURE IsWeight* (node: GraphNodes.Node): BOOLEAN;
	BEGIN
		RETURN node IS Node
	END IsWeight;

	PROCEDURE RemoveWeights*;
	BEGIN
		removeWeights := TRUE
	END RemoveWeights;

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
		removeWeights := FALSE
	END Init;

BEGIN
	Init
END GraphWeight.
