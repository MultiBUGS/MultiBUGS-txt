(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	  *)

MODULE GraphConstant;


	

	IMPORT
		Stores,
		GraphNodes;

	TYPE
		Node = POINTER TO RECORD(GraphNodes.Node)
			value: REAL
		END;

		Factory = POINTER TO RECORD(GraphNodes.Factory) END;

	VAR
		fact-: GraphNodes.Factory;
		constant: GraphNodes.Node;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) Check (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: Node) InitNode;
	BEGIN
		node.SetProps({GraphNodes.data});
	END InitNode;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphConstant.Install"
	END Install;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
	BEGIN
		RETURN NIL
	END Parents;

	PROCEDURE (node: Node) Representative (): Node;
	BEGIN
		RETURN node
	END Representative;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {}
	END Set;

	PROCEDURE (node: Node) Size (): INTEGER;
	BEGIN
		RETURN 1
	END Size;

	PROCEDURE (node: Node) Value (): REAL;
	BEGIN
		RETURN node.value
	END Value;

	PROCEDURE (node: Node) ValDiff (x: GraphNodes.Node; OUT val, diff: REAL);
	BEGIN
		val := node.value;
		diff := 0.0
	END ValDiff;

	PROCEDURE (node: Node) ExternalizeNode (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteReal(node.value);
	END ExternalizeNode;

	PROCEDURE (node: Node) InternalizeNode (VAR rd: Stores.Reader);
	BEGIN
		node.SetProps({GraphNodes.data});
		rd.ReadReal(node.value);
	END InternalizeNode;

	PROCEDURE (f: Factory) New (): GraphNodes.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: Factory) NumParam* (): INTEGER;
	BEGIN
		RETURN 0
	END NumParam;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := ""
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE New* (value: REAL): GraphNodes.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		node.value := value;
		RETURN node
	END New;

	PROCEDURE Old* (value: REAL): GraphNodes.Node;
	BEGIN
		constant(Node).value := value;
		RETURN constant
	END Old;

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
		constant := fact.New()
	END Init;

BEGIN
	Init
END GraphConstant.
