(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphIntegrate;

	

	IMPORT
		MathFunctional, MathIntegrate,
		GraphFunctional, GraphLogical, GraphNodes;

	TYPE
		Node = POINTER TO RECORD(GraphFunctional.Node) END;

		Factory = POINTER TO RECORD(GraphLogical.Factory) END;

	VAR
		fact-: GraphLogical.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphIntegrate.Install"
	END Install;

	PROCEDURE (f: Factory) New (): GraphLogical.Node;
		VAR
			node: Node;
			functional: MathFunctional.Functional;
	BEGIN
		NEW(node);
		node.Init;
		functional := MathIntegrate.factRomberg.New();
		node.SetFunctional(functional);
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "Fsss"
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
END GraphIntegrate.
