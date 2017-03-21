(*		GNU General Public Licence	 *)

(*
license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"
*)

MODULE MathIntegrate;

	

	IMPORT
		MathFunctional;

	TYPE
		Romberg = POINTER TO RECORD(MathFunctional.Functional) END;

		FactoryRomberg = POINTER TO RECORD(MathFunctional.Factory) END;

	VAR
		factRomberg-: MathFunctional.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		up, down: POINTER TO ARRAY OF REAL;

	PROCEDURE PolyInterp (IN x, y: ARRAY OF REAL; z: REAL; num: INTEGER;
	OUT value, error: REAL);
		VAR
			i, m, nearest, index, split: INTEGER;
			dist, minDist, delta, factor: REAL;
	BEGIN
		IF LEN(down) < num THEN NEW(down, num); NEW(up, num) END;
		minDist := INF;
		i := 0;
		WHILE i < num DO
			down[i] := y[i];
			up[i] := y[i];
			dist := ABS(z - x[i]);
			IF dist < minDist THEN
				nearest := i;
				minDist := dist
			END;
			INC(i)
		END;
		index := nearest;
		value := y[nearest];
		m := 1;
		WHILE m < num DO
			i := 0;
			WHILE (i < num - m) DO
				delta := x[i] - x[i + m];
				factor := (down[i + 1] - up[i]) / delta;
				up[i] := factor * (x[i + m] - z);
				down[i] := factor * (x[i] - z);
				INC(i)
			END;
			split := (num - m + 1) DIV 2;
			IF index < split THEN
				error := down[index]
			ELSE
				DEC(index);
				error := up[index]
			END;
			value := value + error;
			INC(m)
		END
	END PolyInterp;

	PROCEDURE (integrator: Romberg) Trapezoid (x0, x1: REAL; IN theta: ARRAY OF REAL; n: INTEGER): REAL, NEW;
		VAR
			i, numPoints: INTEGER;
			delta, start, value: REAL;
	BEGIN
		IF n = - 1 THEN
			value := 0.5 * (integrator.function.Value(x0, theta) + integrator.function.Value(x1, theta)) * (x1 - x0)
		ELSE
			i := 0;
			numPoints := ASH(1, n);
			delta := (x1 - x0) / numPoints;
			start := x0 - 0.5 * delta;
			value := 0.0;
			FOR i := 1 TO numPoints DO
				value := value + integrator.function.Value(start + i * delta, theta)
			END;
			value := value * (x1 - x0) / numPoints
		END;
		RETURN value
	END Trapezoid;

	PROCEDURE (integrator: Romberg) Value (x0, x1, tol: REAL; IN theta: ARRAY OF REAL): REAL;
		CONST
			order = 6;
			max = order - 1;
		VAR
			i, n: INTEGER;
			error, value: REAL;
			step, trapezoid: ARRAY order OF REAL;
	BEGIN
		IF x0 > x1 THEN
			value := x0;
			x0 := x1;
			x1 := value
		END;
		FOR i := 0 TO max DO
			trapezoid[i] := integrator.Trapezoid(x0, x1, theta, i - 1);
			step[i] := ASH(1, 2 * (max - i))
		END;
		n := max;
		LOOP
			PolyInterp(step, trapezoid, 0, max, value, error);
			IF ABS(error) < tol THEN EXIT END;
			FOR i := 1 TO max DO
				trapezoid[i - 1] := trapezoid[i]
			END;
			trapezoid[max] := integrator.Trapezoid(x0, x1, theta, n);
			INC(n)
		END;
		RETURN value
	END Value;

	PROCEDURE (f: FactoryRomberg) New (): MathFunctional.Functional;
		VAR
			integrator: Romberg;
	BEGIN
		NEW(integrator);
		RETURN integrator
	END New;

	PROCEDURE InstallRomberg*;
	BEGIN
		MathFunctional.SetFactory(factRomberg)
	END InstallRomberg;

	PROCEDURE Maintainer;
	BEGIN
		version := 323;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		CONST
			minSize = 10;
		VAR
			fRomberg: FactoryRomberg;
	BEGIN
		Maintainer;
		NEW(fRomberg);
		factRomberg := fRomberg;
		NEW(up, minSize);
		NEW(down, minSize)
	END Init;

BEGIN
	Init
END MathIntegrate.
