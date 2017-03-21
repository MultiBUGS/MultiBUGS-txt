(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphMAP;


	

	IMPORT
		GraphStochastic, 
		MathMatrix;

	VAR
		tau, cholesky: POINTER TO ARRAY OF ARRAY OF REAL;
		deriv: POINTER TO ARRAY OF REAL;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Hessian* (priors: GraphStochastic.Vector; VAR matrix: ARRAY OF ARRAY OF REAL);
		VAR
			i, j, dim: INTEGER;
			val: REAL;
		CONST
			delta = 1.0E-3;
	BEGIN
		dim := LEN(priors);
		i := 0;
		WHILE i < dim DO
			val := priors[i].value;
			priors[i].SetValue(val + delta);
			j := 0;
			WHILE j < dim DO
				matrix[i, j] := priors[j].DiffLogConditional();
				INC(j)
			END;
			priors[i].SetValue(val - delta);
			j := 0;
			WHILE j < dim DO
				matrix[i, j] := (matrix[i, j] - priors[j].DiffLogConditional()) / (2 * delta);
				INC(j)
			END;
			priors[i].SetValue(val);
			INC(i)
		END;
		i := 0;
		WHILE i < dim DO
			j := i;
			WHILE j < dim DO
				matrix[i, j] := 0.5 * (matrix[i, j] + matrix[j, i]);
				matrix[j, i] := matrix[i, j];
				INC(j)
			END;
			INC(i)
		END
	END Hessian;

	PROCEDURE Converged (priors: GraphStochastic.Vector; tol: REAL): BOOLEAN;
		VAR
			i, dim: INTEGER;
			converged: BOOLEAN;
			deriv: REAL;
	BEGIN
		converged := TRUE;
		i := 0;
		dim := LEN(priors);
		WHILE (i < dim) & converged DO
			deriv := priors[i].DiffLogConditional();
			converged := converged & (ABS(deriv) < tol);
			INC(i)
		END;
		RETURN converged
	END Converged;


	PROCEDURE MaximizeBounded (prior: GraphStochastic.Node);
		VAR
			diff, x, y, x0, x1, y0, y1: REAL;
			count: INTEGER;
		CONST
			eps = 1.0E-6;
			tol = 1.0E-6;
	BEGIN
		diff := prior.DiffLogConditional();
		IF ABS(diff) < eps THEN
			RETURN
		ELSE
			prior.Bounds(x0, x1);
			x0 := x0 + eps;
			x1 := x1 - eps;
			prior.SetValue(x0);
			y0 := prior.DiffLogConditional();
			prior.SetValue(x1);
			y1 := prior.DiffLogConditional();
			(*	pegasus solver	*)
			count := 0;
			LOOP
				x := x1 - y1 * (x1 - x0) / (y1 - y0);
				prior.SetValue(x);
				y := prior.DiffLogConditional();
				IF ABS(y) < eps THEN EXIT END;
				IF y * y1 > 0 THEN
					y0 := y0 * y1 / (y1 + y)
				ELSE
					x0 := x1;
					y0 := y1
				END;
				x1 := x;
				y1 := y;
				IF ABS(x1 - x0) < tol THEN EXIT END;
				INC(count);
				ASSERT(count < 2000, 70)
			END
		END;
	END MaximizeBounded;

	PROCEDURE MaximizeUnbounded (prior: GraphStochastic.Node);
		VAR
			count: INTEGER;
			diff, diffMinus, diffPlus, diff2, lower, upper, x, x0: REAL;
		CONST
			maxIts = 100;
			delta = 1.0E-3;
			tol = 1.0E-6;
	BEGIN
		count := 0;
		prior.Bounds(lower, upper);
		diff := prior.DiffLogConditional();
		x := prior.value;
		LOOP
			prior.SetValue(x);
			diff := prior.DiffLogConditional();
			IF ABS(diff) < tol THEN EXIT END;
			prior.SetValue(x + delta);
			diffPlus := prior.DiffLogConditional();
			prior.SetValue(x - delta);
			diffMinus := prior.DiffLogConditional();
			diff2 := (diffPlus - diffMinus) / (2 * delta);
			IF diff2 > 0 THEN EXIT END;
			x0 := x;
			x := x0 - diff / diff2;
			IF x > upper THEN
				x := 0.5 * (x0 + upper); EXIT
			ELSIF x < lower THEN
				x := 0.5 * (lower + x0); EXIT
			END;
			INC(count);
			IF count = maxIts THEN HALT(0) END
		END
	END MaximizeUnbounded;

	PROCEDURE MAP1D* (prior: GraphStochastic.Node);
		CONST
			leftBounds = {GraphStochastic.leftNatural, GraphStochastic.leftImposed};
			rightBounds = {GraphStochastic.rightNatural, GraphStochastic.rightImposed};
	BEGIN
		IF (leftBounds * prior.props = {}) OR (rightBounds * prior.props = {}) THEN
			MaximizeUnbounded(prior)
		ELSE
			MaximizeBounded(prior)
		END
	END MAP1D;

	PROCEDURE MAP* (priors: GraphStochastic.Vector);
		VAR
			i, j, dim, count, icm: INTEGER;
			lower, upper, x: REAL;
		CONST
			tol = 1.0E-6;
			maxIts = 100;
			numICM = 25;
	BEGIN
		dim := LEN(priors);
		IF dim > LEN(tau, 0) THEN
			NEW(deriv, dim); NEW(tau, dim, dim); NEW(cholesky, dim, dim)
		END;
		(*	do iterative conditional mode	*)
		icm := 0;
		REPEAT
			i := 0;
			WHILE i < dim DO
				MAP1D(priors[i]);
				INC(i)
			END;
			INC(icm)
		UNTIL (icm = numICM) OR Converged(priors, tol);
		(*	do some block Newton updates with hessian matrix	*)
		count := 0;
		REPEAT
			Hessian(priors, tau);
			i := 0;
			WHILE i < dim DO
				deriv[i] := priors[i].DiffLogConditional();
				j := 0;
				WHILE j < dim DO
					cholesky[i, j] := - tau[i, j];
					INC(j)
				END;
				INC(i)
			END;
			MathMatrix.Cholesky(cholesky, dim);
			MathMatrix.ForwardSub(cholesky, deriv, dim);
			MathMatrix.BackSub(cholesky, deriv, dim);
			i := 0;
			WHILE i < dim DO
				x := priors[i].value;
				x := x + deriv[i];
				priors[i].Bounds(lower, upper);
				IF x < lower THEN
					x := 0.5 * (lower + priors[i].value);
				ELSIF x > upper THEN
					x := 0.5 * (upper + priors[i].value);
				END;
				priors[i].SetValue(x);
				INC(i)
			END;
			INC(count);
		UNTIL (count = maxIts) OR Converged(priors, tol)
	END MAP;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		CONST
			len = 1;
	BEGIN
		Maintainer;
		NEW(deriv, len);
		NEW(tau, len, len);
		NEW(cholesky, len, len)
	END Init;

BEGIN
	Init
END GraphMAP.
