(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphPi;

	(*	function to return value of pi	*)

	

	IMPORT
		GraphConstant, GraphNodes, 
		Math;

	TYPE

		Factory = POINTER TO RECORD(GraphNodes.Factory) END;

	VAR
		fact-: GraphNodes.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		piNode: GraphNodes.Node;

	PROCEDURE (f: Factory) New (): GraphNodes.Node;
	BEGIN
		RETURN piNode
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
			pi: REAL;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f;
		pi := Math.Pi();
		piNode := GraphConstant.New(pi);
		piNode.Init
	END Init;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

BEGIN
	Init
END GraphPi.
