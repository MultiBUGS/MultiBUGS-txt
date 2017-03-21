(*		GNU General Public Licence		 *)

(*
license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"
*)

MODULE MathRungeKutta45;

	

	IMPORT
		Math,
		MathODE;

	TYPE
		Solver = POINTER TO RECORD(MathODE.Solver)
			k1, k2, k3, k4, k5, k6, error: POINTER TO ARRAY OF REAL
		END;

		Factory = POINTER TO RECORD(MathODE.SolverFactory) END;

	VAR
		fact-: MathODE.SolverFactory;
		version-: INTEGER;
		maintainer-: ARRAY 20 OF CHAR;

	PROCEDURE (solver: Solver) Step (IN theta, x0: ARRAY OF REAL;
		numEq, order: INTEGER;
		t0, step: REAL;
	OUT x1, error: ARRAY OF REAL);
		CONST
			a2 = 1.0 / 5;
			a3 = 3.0 / 10;
			a4 = 3.0 / 5;
			a5 = 1;
			a6 = 7.0 / 8;
			b21 = 1.0 / 5;
			b31 = 3.0 / 40;
			b41 = 3.0 / 10;
			b51 = -11.0 / 54;
			b61 = 1631.0 / 55296;
			b32 = 9.0 / 40;
			b42 = -9.0 / 10;
			b52 = 5.0 / 2;
			b62 = 175.0 / 512;
			b43 = 6.0 / 5;
			b53 = -70.0 / 27;
			b63 = 575.0 / 13824;
			b54 = 35.0 / 27;
			b64 = 44275.0 / 110592;
			b65 = 253.0 / 4096;
			c1 = 37.0 / 378;
			c2 = 0;
			c3 = 250.0 / 621;
			c4 = 125.0 / 594;
			c5 = 0;
			c6 = 512.0 / 1771;
			e1 = c1 - 2825.0 / 27648;
			e2 = 0;
			e3 = c3 - 18575.0 / 48384;
			e4 = c4 - 13525.0 / 55296;
			e5 = c5 - 277.0 / 14336;
			e6 = c6 - 1.0 / 4;
		VAR
			i: INTEGER;
			t: REAL;
			k1, k2, k3, k4, k5, k6: POINTER TO ARRAY OF REAL;
			equations: MathODE.Equations;
	BEGIN
		k1 := solver.k1;
		k2 := solver.k2;
		k3 := solver.k3;
		k4 := solver.k4;
		k5 := solver.k5;
		k6 := solver.k6;
		equations := solver.equations;
		equations.Derivatives(theta, x0, numEq, t0, k1);
		t := t0 + a2 * step;
		i := 0;
		WHILE i < numEq DO
			k1[i] := step * k1[i];
			x1[i] := x0[i] + b21 * k1[i];
			INC(i)
		END;
		equations.Derivatives(theta, x1, numEq, t, k2);
		t := t0 + a3 * step;
		i := 0;
		WHILE i < numEq DO
			k2[i] := step * k2[i];
			x1[i] := x0[i] + b31 * k1[i] + b32 * k2[i];
			INC(i)
		END;
		equations.Derivatives(theta, x1, numEq, t, k3);
		t := t0 + a4 * step;
		i := 0;
		WHILE i < numEq DO
			k3[i] := step * k3[i];
			x1[i] := x0[i] + b41 * k1[i] + b42 * k2[i] + b43 * k3[i];
			INC(i)
		END;
		equations.Derivatives(theta, x1, numEq, t, k4);
		t := t0 + a5 * step;
		i := 0;
		WHILE i < numEq DO
			k4[i] := step * k4[i];
			x1[i] := x0[i] + b51 * k1[i] + b52 * k2[i] + b53 * k3[i] + b54 * k4[i];
			INC(i)
		END;
		equations.Derivatives(theta, x1, numEq, t, k5);
		t := t0 + a6 * step;
		i := 0;
		WHILE i < numEq DO
			k5[i] := step * k5[i];
			x1[i] := x0[i] + b61 * k1[i] + b62 * k2[i] + b63 * k3[i] + b64 * k4[i] + b65 * k5[i];
			INC(i)
		END;
		equations.Derivatives(theta, x1, numEq, t, k6);
		i := 0;
		WHILE i < numEq DO
			k6[i] := k6[i] * step;
			x1[i] := x0[i] + c1 * k1[i] + c2 * k2[i] + c3 * k3[i] + c4 * k4[i] + c5 * k5[i] + c6 * k6[i];
			error[i] := e1 * k1[i] + e2 * k2[i] + e3 * k3[i] + e4 * k4[i] + e5 * k5[i] + e6 * k6[i];
			INC(i)
		END
	END Step;

	PROCEDURE (solver: Solver) TrialStep (IN theta, x0, scale: ARRAY OF REAL;
	numEq: INTEGER; t0, step, tol: REAL;
	OUT actualStep, predStep: REAL;
	OUT x1: ARRAY OF REAL);
		CONST
			safe = 0.9;
		VAR
			i: INTEGER;
			maxError: REAL;
			error: POINTER TO ARRAY OF REAL;
	BEGIN
		error := solver.error;
		LOOP
			solver.Step(theta, x0, numEq, - 9, t0, step, x1, error);
			maxError := 0;
			i := 0;
			WHILE i < numEq DO
				maxError := MAX(maxError, ABS(error[i] / scale[i]));
				INC(i)
			END;
			maxError := maxError / tol;
			IF maxError <= 1 THEN
				actualStep := step;
				IF maxError > 0.00057665 THEN
					predStep := safe * step / Math.Power(maxError, 0.2);
				ELSE
					predStep := 4 * step
				END;
				EXIT
			ELSE
				step := safe * step / Math.Power(maxError, 0.25)
			END
		END
	END TrialStep;

	PROCEDURE (solver: Solver) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "MathRungeKutta45.Install"
	END Install;
	
	PROCEDURE (solver: Solver) InitStorage;
		VAR
			numEq: INTEGER;
	BEGIN
		numEq := solver.numEq;
		NEW(solver.k1, numEq);
		NEW(solver.k2, numEq);
		NEW(solver.k3, numEq);
		NEW(solver.k4, numEq);
		NEW(solver.k5, numEq);
		NEW(solver.k6, numEq);
		NEW(solver.error, numEq)
	END InitStorage;

	PROCEDURE (f: Factory) New (): MathODE.Solver;
		VAR
			solver: Solver;
	BEGIN
		NEW(solver);
		RETURN solver
	END New;

	PROCEDURE Install*;
	BEGIN
		MathODE.SetSolverFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 323;
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
END MathRungeKutta45.

