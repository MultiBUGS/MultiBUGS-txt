(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphODElangRK45;

	

	IMPORT
		GraphNodes, GraphODElang, GraphVector,
		MathODE, MathRungeKutta45;

	TYPE
		Factory = POINTER TO RECORD(GraphVector.Factory) END;

	VAR
		fact-: GraphVector.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (f: Factory) New (): GraphVector.Node;
		VAR
			node: GraphVector.Node;
			solver: MathODE.Solver;
	BEGIN
		solver := MathRungeKutta45.New();
		node := GraphODElang.New(solver);
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vvDss"
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
END GraphODElangRK45.
