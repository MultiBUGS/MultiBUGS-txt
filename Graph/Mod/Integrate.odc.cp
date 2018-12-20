(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphIntegrate;

	

	IMPORT
		MathFunctional, MathIntegrate,
		GraphFunctional, GraphLogical, GraphNodes, GraphScalar;

	TYPE
		Factory = POINTER TO RECORD(GraphScalar.Factory) END;

	VAR
		fact-: GraphScalar.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (f: Factory) New (): GraphScalar.Node;
		VAR
			node: GraphScalar.Node;
			functional: MathFunctional.Functional;
	BEGIN
		functional := MathIntegrate.factRomberg.New();
		node := GraphFunctional.New(functional);
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
