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
	
	PROCEDURE (node: Node) Evaluate;
	BEGIN
	END Evaluate;
	
	PROCEDURE (node: Node) EvaluateDiffs ;
	BEGIN
	END EvaluateDiffs;
	
	PROCEDURE (node: Node) ExternalizeScalar (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeScalar;
	
	PROCEDURE (node: Node) InitLogical;
	BEGIN
	END InitLogical;
		
	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphSentinel.Install"
	END Install;
	
	PROCEDURE (node: Node) InternalizeScalar (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeScalar;
	
	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
	BEGIN
		RETURN NIL
	END Parents;
	
	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
	END Set;

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
