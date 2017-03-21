(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	 *)

MODULE MathAESolver;


	

	IMPORT
		MathFunctional;

	TYPE
		Bisection = POINTER TO RECORD(MathFunctional.Functional) END;

		Pegasus = POINTER TO RECORD(MathFunctional.Functional) END;

		FactoryBisection = POINTER TO RECORD(MathFunctional.Factory) END;

		FactoryPegasus = POINTER TO RECORD(MathFunctional.Factory) END;

	VAR
		factBisection-, factPegasus-: MathFunctional.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (solver: Bisection) Value (x0, x1, tol: REAL; IN theta: ARRAY OF REAL): REAL;
		VAR
			x, y, y0, y1: REAL;
	BEGIN
		IF x0 > x1 THEN
			x := x0;
			x0 := x1;
			x1 := x
		END;
		y0 := solver.function.Value(x0, theta);
		y1 := solver.function.Value(x1, theta);
		ASSERT(y0 * y1 < 0, 60);
		LOOP
			x := 0.5 * (x0 + x1);
			y := solver.function.Value(x, theta);
			IF y * y0 > 0 THEN
				x0 := x;
				y0 := y
			ELSIF y * y0 < 0 THEN
				x1 := x;
				y1 := y
			ELSE
				EXIT
			END;
			IF ABS(x1 - x0) < tol THEN
				EXIT
			END
		END;
		RETURN x
	END Value;

	PROCEDURE (solver: Pegasus) Value (x0, x1, tol: REAL; IN theta: ARRAY OF REAL): REAL;
		VAR
			count: INTEGER;
			x, y, y0, y1: REAL;
	BEGIN
		count := 0;
		IF x0 > x1 THEN
			x := x0;
			x0 := x1;
			x1 := x
		END;
		y0 := solver.function.Value(x0, theta);
		y1 := solver.function.Value(x1, theta);
		ASSERT(y0 * y1 < 0, 60);
		LOOP
			x := x1 - y1 * (x1 - x0) / (y1 - y0);
			y := solver.function.Value(x, theta);
			IF y * y1 > 0 THEN
				y0 := y0 * y1 / (y1 + y)
			ELSE
				x0 := x1;
				y0 := y1
			END;
			x1 := x;
			y1 := y;
			IF ABS(x1 - x0) < tol THEN
				EXIT
			END;
			INC(count);
			ASSERT(count < 2000, 70)
		END;
		RETURN x
	END Value;

	PROCEDURE (f: FactoryBisection) New (): MathFunctional.Functional;
		VAR
			solver: Bisection;
	BEGIN
		NEW(solver);
		RETURN solver
	END New;

	PROCEDURE (f: FactoryPegasus) New (): MathFunctional.Functional;
		VAR
			solver: Pegasus;
	BEGIN
		NEW(solver);
		RETURN solver
	END New;

	PROCEDURE InstallBisection*;
	BEGIN
		MathFunctional.SetFactory(factBisection)
	END InstallBisection;

	PROCEDURE InstallPegasus*;
	BEGIN
		MathFunctional.SetFactory(factPegasus)
	END InstallPegasus;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			fBisection: FactoryBisection;
			fPegasus: FactoryPegasus;
	BEGIN
		Maintainer;
		NEW(fBisection);
		factBisection := fBisection;
		NEW(fPegasus);
		factPegasus := fPegasus
	END Init;

BEGIN
	Init
END MathAESolver.
