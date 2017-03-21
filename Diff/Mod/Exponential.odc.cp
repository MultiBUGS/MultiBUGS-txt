(*

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"
*)

MODULE DiffExponential;

	

	IMPORT
		GraphNodes, GraphODEmath, GraphVector,
		MathODE, MathRungeKutta45;


	TYPE
		Equations = POINTER TO RECORD (MathODE.Equations) END;

		Factory = POINTER TO RECORD(GraphVector.Factory) END;


	CONST
		nEq = 1;

	VAR
		fact-: GraphNodes.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		
	PROCEDURE (e: Equations) Derivatives (IN parameters, C: ARRAY OF REAL; n: INTEGER;
		t: REAL; OUT dCdt: ARRAY OF REAL);
		VAR
			lambda: REAL;
	BEGIN
		lambda := parameters[0];
		dCdt[0] := - lambda * C[0];
	END Derivatives;

	PROCEDURE (e: Equations) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "DiffExponential.Install"
	END Install;

	PROCEDURE (e: Equations) SecondDerivatives (IN theta, x: ARRAY OF REAL;
	numEq: INTEGER;
	t: REAL;
	OUT d2xdt2: ARRAY OF REAL);
	BEGIN
		HALT(126)
	END SecondDerivatives;

	PROCEDURE (e: Equations) Jacobian (IN theta, x: ARRAY OF REAL; numEq: INTEGER;
	t: REAL; OUT jacob: ARRAY OF ARRAY OF REAL);
	BEGIN
		HALT(126)
	END Jacobian;

	PROCEDURE (f: Factory) New (): GraphVector.Node;
		VAR
			node: GraphVector.Node;
			equations: Equations;
			solver: MathODE.Solver;
	BEGIN
		NEW(equations);
		solver := MathRungeKutta45.fact.New();
		node := GraphODEmath.New(solver, equations, nEq);
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vvvsv"
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
END DiffExponential.
