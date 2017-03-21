(*

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"
*)

MODULE DiffChangePoints;

	

	IMPORT
		Math,
		GraphNodes, GraphODEBlockM, GraphVector,
		MathODE, MathRungeKutta45;

	TYPE
		Equations = POINTER TO RECORD (GraphODEBlockM.Equations) END;

		Factory = POINTER TO RECORD(GraphVector.Factory) END;

	CONST
		nEq = 3;

	VAR
		fact-: GraphNodes.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (e: Equations) Derivatives (IN parameters, C: ARRAY OF REAL; n: INTEGER; t: REAL;
	OUT dCdt: ARRAY OF REAL);
		VAR
			block: INTEGER;
			CL, V, TI, dose, R31: REAL;
	BEGIN
		block := e.Block();
		CL := Math.Exp(parameters[0]);
		V := Math.Exp(parameters[1]);
		TI := Math.Exp(parameters[2]);
		dose := parameters[3];
		CASE block OF
		|0: R31 := 0;
		|1: R31 := 0;
		|2: R31 := dose / TI;
		|3: R31 := 0;
		END;
		dCdt[0] := R31 - CL * C[0] / V;
		dCdt[1] := 0;
		dCdt[2] := - R31;
	END Derivatives;

	PROCEDURE (e: Equations) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "DiffChangePoints.Install"
	END Install;

	PROCEDURE (e: Equations) Adjust (IN theta: ARRAY OF REAL;
	VAR C: ARRAY OF REAL; n: INTEGER; t: REAL);
		VAR
			block: INTEGER;
	BEGIN
		block := e.Block();
		CASE block OF
		|1:
			C[0] := C[0] + C[1];
			C[1] := 0;
		|2:
		|3:
		END;
	END Adjust;

	PROCEDURE (equations: Equations) SecondDerivatives (IN theta, x: ARRAY OF REAL;
	numEq: INTEGER; t: REAL; OUT d2xdt2: ARRAY OF REAL);
	BEGIN
		HALT(126)
	END SecondDerivatives;

	PROCEDURE (equations: Equations) Jacobian (IN theta, x: ARRAY OF REAL; numEq: INTEGER; t: REAL; OUT jacob: ARRAY OF ARRAY OF REAL);
	BEGIN
		HALT(126)
	END Jacobian;

	PROCEDURE (f: Factory) New (): GraphODEBlockM.Node;
		VAR
			equations: Equations;
			node: GraphODEBlockM.Node;
			solver: MathODE.Solver;
	BEGIN
		NEW(equations);
		solver := MathRungeKutta45.fact.New();
		node := GraphODEBlockM.New(solver, equations, nEq);
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vvvvv";
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas";
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
END DiffChangePoints.
