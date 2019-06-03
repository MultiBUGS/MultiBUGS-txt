(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphSentinel;

	

	IMPORT
		Stores := Stores64,
		GraphLogical, GraphNodes, GraphRules, GraphScalar;

	TYPE

		Node = POINTER TO RECORD(GraphScalar.Node) END;
		
	VAR
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
		RETURN class
	END ClassFunction;
	
	PROCEDURE (node: Node) ExternalizeLogical (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeLogical;
	
	PROCEDURE (node: Node) InitLogical;
	BEGIN
	END InitLogical;
		
	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphSentinel.Install"
	END Install;
	
	PROCEDURE (node: Node) InternalizeLogical (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeLogical;
	
	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
	BEGIN
		RETURN NIL
	END Parents;
	
	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
	END Set;
	
	PROCEDURE (node: Node) ValDiff (x: GraphNodes.Node; OUT val, diff: REAL);
	BEGIN
	END ValDiff;
	
	PROCEDURE (node: Node) Value (): REAL;
	BEGIN
		RETURN 0.0
	END Value;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE New* (): GraphLogical.Node;
		VAR
			s: Node;
	BEGIN
		NEW(s);
		RETURN s
	END New;

BEGIN
	Maintainer		
END GraphSentinel.
