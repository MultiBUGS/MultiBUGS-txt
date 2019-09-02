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

	PROCEDURE DiffLogConditional* (node: GraphStochastic.Node): REAL;
		VAR
			diff, value: REAL;
			children: GraphStochastic.Vector;
			i, num: INTEGER;
	BEGIN
		node.EvaluateDiffs;
		diff := node.DiffLogPrior();
		children := node.children;
		IF children # NIL THEN
			num := LEN(children);
			i := 0;
			WHILE i < num DO
				diff := diff + children[i].DiffLogLikelihood(node);
				INC(i)
			END
		END;
		node.ClearDiffs;
		RETURN diff
	END DiffLogConditional;

	PROCEDURE Hessian* (priors: GraphStochastic.Vector; VAR matrix: ARRAY OF ARRAY OF REAL;
	OUT error: BOOLEAN);
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
			priors[i].value := val + delta;
			priors[i].Evaluate;
			j := 0;
			WHILE j < dim DO
				matrix[i, j] := DiffLogConditional(priors[j]);
				INC(j)
			END;
			priors[i].value := val - delta;
			priors[i].Evaluate;
			j := 0;
			WHILE j < dim DO
				matrix[i, j] := (matrix[i, j] - DiffLogConditional(priors[j])) / (2 * delta);
				INC(j)
			END;
			priors[i].value := val;
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
		END;
		i := 0; error := FALSE; WHILE (i < dim) & ~error DO error := matrix[i, i] > 0.0; INC(i) END;
	END Hessian;

	PROCEDURE Converged (priors: GraphStochastic.Vector; tol: REAL): BOOLEAN;
		VAR
			i, dim: INTEGER;
	BEGIN
		i := 0;
		dim := LEN(priors);
		WHILE i < dim DO
			deriv[i] := DiffLogConditional(priors[i]);
			IF ABS(deriv[i]) > tol THEN RETURN FALSE END;
			INC(i)
		END;
		RETURN TRUE
	END Converged;


	PROCEDURE MaximizeBounded (prior: GraphStochastic.Node);
		VAR
			diff, x, y, x0, x1, y0, y1: REAL;
			count: INTEGER;
		CONST
			eps = 1.0E-6;
			tol = 1.0E-6;
	BEGIN
		diff := DiffLogConditional(prior);
		IF ABS(diff) < eps THEN
			RETURN
		ELSE
			prior.Bounds(x0, x1);
			x0 := x0 + eps;
			x1 := x1 - eps;
			prior.value := x0;
			prior.Evaluate;
			y0 := DiffLogConditional(prior);
			prior.value := x1;
			prior.Evaluate;
			y1 := DiffLogConditional(prior);
			(*	pegasus solver	*)
			count := 0;
			LOOP
				x := x1 - y1 * (x1 - x0) / (y1 - y0);
				prior.value := x;
				prior.Evaluate;
				y := DiffLogConditional(prior);
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
		diff := DiffLogConditional(prior);
		x := prior.value;
		LOOP
			prior.value := x;
			prior.Evaluate;
			diff := DiffLogConditional(prior);
			IF ABS(diff) < tol THEN EXIT END;
			prior.value := x + delta;
			prior.Evaluate;
			diffPlus := DiffLogConditional(prior);
			prior.value := x - delta;
			prior.Evaluate;
			diffMinus := DiffLogConditional(prior);
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

	PROCEDURE MAP* (priors: GraphStochastic.Vector; OUT error: BOOLEAN);
		VAR
			i, j, dim, count, icm: INTEGER;
			lower, upper, x: REAL;
			converged: BOOLEAN;
		CONST
			tol = 1.0E-6;
			maxIts = 100;
			numICM = 100;
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
			converged := Converged(priors, tol);
			INC(icm)
		UNTIL (icm = numICM) OR converged; 
		(*	do some block Newton updates with hessian matrix	*)
		count := 0;
		error := FALSE;
		WHILE ~converged & (count < maxIts) DO
			Hessian(priors, tau, error); IF error THEN RETURN END;
			i := 0;
			WHILE i < dim DO
				deriv[i] := DiffLogConditional(priors[i]);
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
				priors[i].value := x;
				INC(i)
			END;
			INC(count);
			converged := Converged(priors, tol);
		END;
		error := ~converged
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
