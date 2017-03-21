(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphODElangRK45;

	

	IMPORT
		GraphNodes, GraphODElang, GraphVector,
		MathODE, MathRungeKutta45;

	TYPE
		Node = POINTER TO RECORD(GraphODElang.Node) END;

		Factory = POINTER TO RECORD(GraphVector.Factory) END;

	VAR
		fact-: GraphVector.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphODElangRK45.Install"
	END Install;

	PROCEDURE (f: Factory) New (): GraphVector.Node;
		VAR
			node: Node;
			solver: MathODE.Solver;
	BEGIN
		NEW(node);
		node.Init;
		solver := MathRungeKutta45.fact.New();
		node.SetSolver(solver);
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
