(* 		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



  *)

MODULE MathJacobi;

	

	IMPORT
		Math, 
		MathFunc, MathMatrix;
		
	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		matrix, evec: POINTER TO ARRAY OF ARRAY OF REAL;
		
	PROCEDURE QuadratureRule* (OUT abscisa, weights: ARRAY OF REAL;  alpha, beta: REAL; size: INTEGER);
		VAR
			i, j: INTEGER;
			l, hm, norm: REAL;
	BEGIN
		IF size > LEN(matrix, 0) THEN NEW(matrix, size, size); NEW(evec, size, size) END;
		i := 0; WHILE i < size DO j := 0; WHILE j < size DO matrix[i, j] := 0.0; INC(j) END; INC(i) END;
		matrix[0, 0] := (beta - alpha) / (alpha + beta + 2);
		matrix[0, 1] := 2 / (alpha + beta + 2);
		i := 1;
		WHILE i < size - 1 DO
			l := 2 * i + alpha + beta;
			matrix[i, i - 1] := 2 * i * (i + alpha + beta) / (l * (l + 1));
			matrix[i, i] := (beta * beta  - alpha * alpha) / (l * (l + 2));
			matrix[i, i + 1] := 2 * (i + alpha + 1) * (i + beta + 1) / ((l + 1) * (l + 2));
			INC(i)
		END;
		l := 2 * (size - 1) + alpha + beta;
		matrix[size - 1, size - 2] := 2 * (size - 1 + alpha) * (size  - 1 + beta) / (l * (l + 1));
		matrix[size - 1, size - 1] := (beta * beta - alpha * alpha) / (l * (l + 2)); 
		(*	make matrix symetric does not effect eigenvalues but does effect eigenvectors	*)
		i := 0;
		WHILE i < size - 1 DO
			hm := Math.Sqrt(matrix[i, i + 1] * matrix[i + 1, i]);
			matrix[i, i + 1] := hm;
			matrix[i + 1, i] := hm;
			INC(i)
		END;
		MathMatrix.JacobiVectors(matrix, evec, abscisa, size);
		norm := Math.Power(2, alpha + beta + 1) * Math.Exp(MathFunc.LogGammaFunc(alpha + 1) * 
		MathFunc.LogGammaFunc(beta + 1) / MathFunc.LogGammaFunc(alpha + beta + 1)) / (alpha + beta + 1);
		i := 0;
		WHILE i < size DO
			weights[i] := evec[0, i] * evec[0, i] * norm; 
			INC(i)
		END;
	END QuadratureRule;
	
	PROCEDURE Test* (size: INTEGER); (*	should be gauss (- legendre) rules	*)
		VAR
			abscisa, weights: POINTER TO ARRAY OF REAL;
			i: INTEGER;
			integral: REAL;
	BEGIN
		NEW(abscisa, size); NEW(weights, size);
		QuadratureRule(abscisa, weights, 0.0, 0.5, size);
		i := 0;
		integral := 0.0;
		WHILE i < size DO integral := integral + weights[i]; INC(i) END;
		HALT(0)
	END Test;
	
	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		CONST
			size = 4;
	BEGIN
		Maintainer;
		NEW(matrix, size, size);
		NEW(evec, size, size)
	END Init;
	
BEGIN
	Init
END MathJacobi.

'MathJacobi.Test(8)'


