(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	 *)

MODULE MathODE;

	TYPE

		Equations* = POINTER TO ABSTRACT RECORD END;

		Solver* = POINTER TO ABSTRACT RECORD
			numEq-: INTEGER;
			scale, x, dxdt: POINTER TO ARRAY OF REAL;
			equations-: Equations
		END;

		SolverFactory* = POINTER TO ABSTRACT RECORD END;

	VAR
		solverFact-: SolverFactory;
		version-: INTEGER;
		maintainer-: ARRAY 20 OF CHAR;

	PROCEDURE (equations: Equations) Derivatives* (IN theta, x: ARRAY OF REAL;
	numEq: INTEGER; t: REAL; OUT dxdt: ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE (equations: Equations) Install* (OUT install: ARRAY OF CHAR), NEW, ABSTRACT;

	PROCEDURE (equations: Equations) SecondDerivatives* (IN theta, x: ARRAY OF REAL;
	numEq: INTEGER; t: REAL; OUT d2xdt2: ARRAY OF REAL), NEW, ABSTRACT;

	
	PROCEDURE (equations: Equations) Jacobian* (IN theta, x: ARRAY OF REAL;
	numEq: INTEGER; t: REAL; OUT jacob: ARRAY OF ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE (solver: Solver) Step* (IN theta, x0: ARRAY OF REAL;
	numEq, order: INTEGER; t0, step: REAL; OUT x1, error: ARRAY OF REAL), NEW, ABSTRACT;


	PROCEDURE (solver: Solver) TrialStep* (IN theta, x0, scale: ARRAY OF REAL;
		numEq: INTEGER; t0, step, tol: REAL; OUT actualStep, predStep: REAL;
	OUT x1: ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE (solver: Solver) AccurateStep* (IN theta, x0: ARRAY OF REAL;
	numEq: INTEGER; t0, step, tol: REAL; OUT x1: ARRAY OF REAL), NEW;
		CONST
			maxSteps = 100000;
			eps = 1.0E-15;
		VAR
			i, iter: INTEGER;
			actualStep, predStep, t, tStep: REAL;
			x, dxdt, scale: POINTER TO ARRAY OF REAL;
			equations: Equations;
	BEGIN
		x := solver.x;
		dxdt := solver.dxdt;
		scale := solver.scale;
		equations := solver.equations;
		t := t0;
		i := 0;
		WHILE i < numEq DO
			x[i] := x0[i];
			INC(i)
		END;
		tStep := t0 + step;
		step := MIN(step, 0.25);
		iter := 0;
		LOOP
			INC(iter); ASSERT(iter < maxSteps, 33);
			IF t + step > tStep THEN step := tStep - t END;
			equations.Derivatives(theta, x, numEq, t, solver.dxdt);
			i := 0;
			WHILE i < numEq DO
				scale[i] := ABS(x[i]) + ABS(dxdt[i] * step) + eps;
				INC(i)
			END;
			solver.TrialStep(theta, x, scale, numEq, t, step, tol, actualStep, predStep, x1);
			t := t + actualStep;
			IF ABS(tStep - t) < eps THEN EXIT END;
			step := predStep;
			i := 0;
			WHILE i < numEq DO
				x[i] := x1[i];
				INC(i)
			END
		END
	END AccurateStep;

	PROCEDURE (solver: Solver) InitStorage-, NEW, ABSTRACT;

	PROCEDURE (solver: Solver) Init* (equations: Equations; numEq: INTEGER), NEW;
	BEGIN
		solver.numEq := numEq;
		solver.equations := equations;
		NEW(solver.scale, numEq);
		NEW(solver.x, numEq);
		NEW(solver.dxdt, numEq);
		solver.InitStorage
	END Init;

	PROCEDURE (solver: Solver) Install* (OUT install: ARRAY OF CHAR), NEW, ABSTRACT;
	
	PROCEDURE (f: SolverFactory) New* (): Solver, NEW, ABSTRACT;

	PROCEDURE SetSolverFactory* (f: SolverFactory);
	BEGIN
		solverFact := f
	END SetSolverFactory;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer
	END Init;

BEGIN
	Init
END MathODE.
