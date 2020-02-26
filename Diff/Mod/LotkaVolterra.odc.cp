(*

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"
*)

MODULE DiffLotkaVolterra;

	

	IMPORT
		GraphNodes, GraphODEmath, GraphVector,
		MathODE, MathRungeKutta45;

	TYPE
		Equations = POINTER TO RECORD (MathODE.Equations) END;

		Factory = POINTER TO RECORD(GraphVector.Factory) END;

	CONST
		nEq = 2;

	VAR
		fact-: GraphVector.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		PROCEDURE (e: Equations) Derivatives (IN parameters, C: ARRAY OF REAL;
	n: INTEGER; t: REAL; OUT dCdt: ARRAY OF REAL);
		VAR
			alpha, beta, gamma, delta: REAL;
	BEGIN
		alpha := parameters[0];
		beta := parameters[1];
		gamma := parameters[2];
		delta := parameters[3];
		dCdt[0] := C[0] * (alpha - beta * C[1]);
		dCdt[1] :=  - C[1] * (gamma - delta * C[0])
	END Derivatives;

	PROCEDURE (e: Equations) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "DiffLotkaVolterra.Install"
	END Install;

	PROCEDURE (e: Equations) SecondDerivatives (IN theta,
	x: ARRAY OF REAL; numEq: INTEGER; t: REAL; OUT d2xdt2: ARRAY OF REAL);
	BEGIN
		HALT(126)
	END SecondDerivatives;

	PROCEDURE (e: Equations) Jacobian (IN theta, x: ARRAY OF REAL;
	numEq: INTEGER; t: REAL;
	OUT jacob: ARRAY OF ARRAY OF REAL);
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
		solver := MathRungeKutta45.New();
		node := GraphODEmath.New(solver, equations, nEq);
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vvvss"
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "Dave Lunn"
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
END DiffLotkaVolterra.
