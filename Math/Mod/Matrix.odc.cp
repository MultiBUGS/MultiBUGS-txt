(*		GNU General Public Licence  *)

MODULE MathMatrix;

	(*
	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"
	*)

	(*	Some of these procedures require symmetric positive definite matrices	*)

	

	IMPORT
		Math,
		MathSort;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 20 OF CHAR;
		z: POINTER TO ARRAY OF REAL;
		workMatrix: POINTER TO ARRAY OF ARRAY OF REAL;

		(*	Forward substitution to solve Ax = b where A is lower triangular. See, for example,
		http://en.wikipedia.org/wiki/Triangular_matrix#Forward_and_back_substitution
		The x variable plays the role of "b" on input and of the solution "x" on output.   *)

	PROCEDURE ForwardSub* (IN a: ARRAY OF ARRAY OF REAL;
	VAR x: ARRAY OF REAL; size: INTEGER);
		VAR
			i, j: INTEGER;
			sum: REAL;
	BEGIN
		ASSERT(LEN(a, 0) >= size, 20);
		ASSERT(LEN(a, 1) >= size, 21);
		ASSERT(LEN(x) >= size, 22);
		i := 0;
		WHILE i < size DO
			sum := x[i];
			j := 0;
			WHILE j < i DO
				sum := sum - a[i, j] * x[j];
				INC(j)
			END;
			x[i] := sum / a[i, i];
			INC(i)
		END
	END ForwardSub;

	(*	Back substitution to solve Ax = b where A is upper triangular. The algorithm is exactly
	the same as in forward substitution but with the loops running in the opposite direction.
	For convenience of use in "Invert", the input matrix is the transpose of A.	*)

	PROCEDURE BackSub* (IN a: ARRAY OF ARRAY OF REAL;
	VAR x: ARRAY OF REAL; size: INTEGER);
		VAR
			i, j: INTEGER;
			sum: REAL;
	BEGIN
		ASSERT(LEN(a, 0) >= size, 20);
		ASSERT(LEN(a, 1) >= size, 21);
		ASSERT(LEN(x) >= size, 22);
		i := size - 1;
		WHILE i >= 0 DO
			sum := x[i];
			j := size - 1;
			WHILE j > i DO
				sum := sum - a[j, i] * x[j];
				DEC(j)
			END;
			x[i] := sum / a[i, i];
			DEC(i)
		END
	END BackSub;

	(*	Implementation of the Cholesky–Banachiewicz algorithm, as described at:
	http://en.wikipedia.org/wiki/Cholesky_decomposition
	The L matrix of the decomposition is computed in the lower triangle of A.
	The elements of symmetric A are initially retained in the upper triangle.
	The upper triangle is set to zero at the end of the procedure.	*)

	PROCEDURE Cholesky* (VAR a: ARRAY OF ARRAY OF REAL; size: INTEGER);
		VAR
			i, j, k: INTEGER;
			sum: REAL;
	BEGIN
		ASSERT(LEN(a, 0) >= size, 20);
		ASSERT(LEN(a, 1) >= size, 21);
		i := 0;
		WHILE i < size DO
			j := 0;
			WHILE j <= i DO
				sum := a[j, i];
				k := 0;
				WHILE k < j DO
					sum := sum - a[i, k] * a[j, k];
					INC(k)
				END;
				IF j = i THEN a[j, j] := Math.Sqrt(sum)
				ELSE a[i, j] := sum / a[j, j]
				END;
				INC(j)
			END;
			INC(i)
		END;
		j := 0;
		WHILE j < size DO
			i := 0; WHILE i < j DO a[i, j] := 0.0; INC(i) END;
			INC(j)
		END
	END Cholesky;

	(*	Matrix inverse computed using Cholesky decomposition followed by forward
	and back substitution. The symmetric matrix A is decomposed to LL'. We then solve
	LL'x = z for each column z of the identity matrix, which gives the columns, x,
	of A^{-1}. We obtain each column by solving Ly = z (forward substitution) and then
	L'x = y (back substitution).	*)

	PROCEDURE Invert* (VAR a: ARRAY OF ARRAY OF REAL; size: INTEGER);
		VAR
			i, j: INTEGER;
	BEGIN
		ASSERT(LEN(a, 0) >= size, 20);
		ASSERT(LEN(a, 1) >= size, 21);
		IF LEN(z) < size THEN NEW(z, size) END;
		IF LEN(workMatrix, 0) < size THEN NEW(workMatrix, size, size) END;
		Cholesky(a, size);
		i := 0;
		WHILE i < size DO
			j := 0;
			WHILE j < size DO
				IF j = i THEN z[j] := 1 ELSE z[j] := 0 END;
				INC(j)
			END;
			ForwardSub(a, z, size);
			BackSub(a, z, size);
			j := 0; WHILE j < size DO workMatrix[i, j] := z[j]; INC(j) END;
			INC(i)
		END;
		i := 0;
		WHILE i < size DO
			j := 0; WHILE j < size DO a[i, j] := workMatrix[i, j]; INC(j) END;
			INC(i)
		END
	END Invert;

	PROCEDURE DotProduct* (IN x, y: ARRAY OF REAL; size: INTEGER): REAL;
		VAR
			i: INTEGER;
			dotProduct: REAL;
	BEGIN
		ASSERT(LEN(x) >= size, 20);
		ASSERT(LEN(y) >= size, 21);
		i := 0;
		dotProduct := 0.0;
		WHILE i < size DO
			dotProduct := dotProduct + x[i] * y[i];
			INC(i)
		END;
		RETURN dotProduct
	END DotProduct;

	PROCEDURE LogDet* (VAR a: ARRAY OF ARRAY OF REAL; size: INTEGER): REAL;
		VAR
			i: INTEGER;
			log: REAL;
	BEGIN
		ASSERT(LEN(a, 0) >= size, 20);
		ASSERT(LEN(a, 1) >= size, 21);
		Cholesky(a, size);
		log := 0.0;
		i := 0;
		WHILE i < size DO
			log := log + Math.Ln(a[i, i]);
			INC(i)
		END;
		RETURN 2.0 * log
	END LogDet;

	(*
	The following 6 procedures are functions for the jacobi method for eigenvalues of a real
	symmetric matrix. The procedures SymsChur2, Apply_jacobi_L, Apply_jacobi_R,
	Norm, and Jacobi are taken from eigen/jacobi.c in the GNU Scientific Library, whereas
	Hypot is trivial.
	*)

	PROCEDURE Hypot (x, y: REAL): REAL;
	BEGIN
		RETURN Math.Sqrt(x * x + y * y);
	END Hypot;

	PROCEDURE SymsChur2 (VAR a: ARRAY OF ARRAY OF REAL;
	p, q: INTEGER;
	VAR c, s: REAL): REAL;
		VAR
			apq, app, aqq, tau, t, c1: REAL;
	BEGIN
		apq := a[p][q];

		IF apq # 0.0 THEN
			app := a[p][p];
			aqq := a[q][q];
			tau := (aqq - app) / (2.0 * apq);
			IF tau >= 0.0 THEN
				t := 1.0 / (tau + Hypot(1.0, tau));
			ELSE
				t :=  - 1.0 / ( - tau + Hypot(1.0, tau));
			END;
			c1 := 1.0 / Hypot(1.0, t);
			c := c1;
			s := c1 * t;
		ELSE
			c := 1.0;
			s := 0.0;
		END;
		RETURN ABS(apq);
	END SymsChur2;

	PROCEDURE Apply_jacobi_L (VAR a: ARRAY OF ARRAY OF REAL; p, q: INTEGER;
	c, s: REAL; size: INTEGER);
		VAR
			j: INTEGER;
			apj, aqj: REAL;
	BEGIN
		j := 0;
		WHILE j < size DO
			apj := a[p][j];
			aqj := a[q][j];
			a[p][j] := apj * c - aqj * s;
			a[q][j] := apj * s + aqj * c;
			INC(j);
		END;
	END Apply_jacobi_L;

	PROCEDURE Apply_jacobi_R (VAR a: ARRAY OF ARRAY OF REAL; p, q: INTEGER;
	c, s: REAL; size: INTEGER);
		VAR
			i: INTEGER;
			aip, aiq: REAL;
	BEGIN
		i := 0;
		WHILE i < size DO
			aip := a[i][p];
			aiq := a[i][q];
			a[i][p] := aip * c - aiq * s;
			a[i][q] := aip * s + aiq * c;
			INC(i);
		END;
	END Apply_jacobi_R;

	PROCEDURE Norm (a: ARRAY OF ARRAY OF REAL; size: INTEGER): REAL;
		VAR
			i, j: INTEGER;
			sum, scale, ssq, Aij, ax: REAL;
	BEGIN
		sum := 0.0;
		scale := 0.0;
		ssq := 0.0;
		i := 0;
		WHILE i < size DO
			j := 0;
			WHILE j < size DO
				Aij := a[i][j];
				IF Aij # 0.0 THEN
					ax := ABS(Aij);
					IF scale < ax THEN
						ssq := 1.0 + ssq * (scale / ax) * (scale / ax);
						scale := ax;
					ELSE
						ssq := ssq + (ax / scale) * (ax / scale);
					END;
				END;
				INC(j);
			END;
			INC(i);
		END;
		sum := scale * Math.Sqrt(ssq);
		RETURN sum;
	END Norm;

	(*	Output is eigenvalues only, eigenvalues sorted	*)
	PROCEDURE Jacobi* (VAR a: ARRAY OF ARRAY OF REAL;
	OUT ev: ARRAY OF REAL; size: INTEGER);
		CONST
			max_rot = 50;
		VAR
			i, p, q: INTEGER;
			red, redsum, nrm, c, s: REAL;
	BEGIN
		ASSERT(LEN(a, 0) >= size, 20);
		ASSERT(LEN(a, 1) >= size, 21);
		ASSERT(LEN(ev) >= size, 22);
		(* initialise the array to store eigenvalues *)
		i := 0;
		WHILE i < size DO
			ev[i] := 0.0;
			INC(i);
		END;
		redsum := 0.0;
		i := 0;
		LOOP
			nrm := Norm(a, size);
			IF (i > max_rot) OR (nrm = 0.0) THEN
				EXIT;
			END;
			INC(i);
			p := 0;
			WHILE p < size DO
				q := p + 1;
				WHILE q < size DO
					red := SymsChur2(a, p, q, c, s);
					redsum := redsum + red;
					Apply_jacobi_L(a, p, q, c, s, size);
					Apply_jacobi_R(a, p, q, c, s, size);
					INC(q);
				END;
				INC(p);
			END;
		END; 
		i := 0;
		WHILE i < size DO
			ev[i] := a[i, i];
			INC(i);
		END;
		MathSort.HeapSort(ev, size);
	END Jacobi;

	(*	Out put is eigenvalues and eigenvectors, eigenvalues not sorted	*)
	PROCEDURE JacobiVectors* (VAR a, evec: ARRAY OF ARRAY OF REAL;
	OUT ev: ARRAY OF REAL; size: INTEGER);
		CONST
			max_rot = 50;
		VAR
			i, j, p, q: INTEGER;
			red, redsum, nrm, c, s: REAL;
	BEGIN
		ASSERT(LEN(a, 0) >= size, 20);
		ASSERT(LEN(a, 1) >= size, 21);
		ASSERT(LEN(ev) >= size, 22);
		ASSERT(LEN(evec, 0) >= size, 23);
		ASSERT(LEN(evec, 1) >= size, 24);
		(*	initialise the arrays to store eigenvalues and eigenvectors	*)
		i := 0;
		WHILE i < size DO
			ev[i] := 0.0;
			j := 0; WHILE j < size DO evec[i, j] := 0.0; INC(j) END;
			evec[i, i] := 1.0;
			INC(i);
		END;
		redsum := 0.0;
		i := 0;
		LOOP
			nrm := Norm(a, size);
			IF (i > max_rot) OR (nrm = 0.0) THEN
				EXIT;
			END;
			INC(i);
			p := 0;
			WHILE p < size DO
				q := p + 1;
				WHILE q < size DO
					red := SymsChur2(a, p, q, c, s);
					redsum := redsum + red;
					Apply_jacobi_L(a, p, q, c, s, size);
					Apply_jacobi_R(a, p, q, c, s, size);
					(*	calculate eigen vectors	*)
					Apply_jacobi_R (evec, p, q, c, s, size);
					INC(q);
				END;
				INC(p);
			END;
		END; 
		i := 0;
		WHILE i < size DO
			ev[i] := a[i, i];
			INC(i);
		END;
	END JacobiVectors;

	PROCEDURE SVD* (IN w: ARRAY OF ARRAY OF REAL; 
	OUT svdMatrix: ARRAY OF ARRAY OF REAL; OUT svdVector: ARRAY OF REAL);
		VAR
			i, ii, j, k, estColRank, rotCount, sweepCount, slimit, nRow, nCol, dim0, dim1: INTEGER;
			eps, e2, tol, vt, p, x0, y0, q, r, c0, s0, d1, d2: REAL;
	BEGIN
		(* Based on Algorithm 1 in Nash, J. C. (1990). Compact Numerical Methods FOR Computers:
		Linear algebra and function minimisation. 2nd edition, CRC Press, pp. 36-38 *)
		nRow := LEN(w, 0);
		nCol := LEN(w, 1);
		dim0 := MAX(LEN(workMatrix, 0), 2 * nRow);
		dim1 := MAX(LEN(workMatrix, 1), nCol);
		IF (dim0 > LEN(workMatrix, 0)) OR (dim1 > LEN(workMatrix, 1)) THEN
			NEW(workMatrix, dim0, dim1)
		END;
		FOR i := 0 TO(nRow - 1) DO
			FOR j := 0 TO(nCol - 1) DO
				workMatrix[i, j] := w[i, j];
				IF i = j THEN
					workMatrix[nRow + i, j] := 1.0;
				ELSE
					workMatrix[nRow + i, j] := 0.0;
				END;
			END;
		END;
		(* approximate machine precision *)
		eps := 1.0;
		WHILE (1.0 + 0.5 * eps) # 1.0 DO
			eps := 0.5 * eps;
		END;
		slimit := MAX(6, nCol DIV 4);
		estColRank := nCol;
		sweepCount := 0;
		e2 := 10.0 * nRow * eps * eps;
		tol := eps * 0.1;
		REPEAT
			rotCount := (estColRank * (estColRank - 1)) DIV 2;
			sweepCount := sweepCount + 1;
			FOR j := 0 TO(estColRank - 2) DO
				FOR k := (j + 1) TO(estColRank - 1) DO
					p := 0.0;
					q := 0.0;
					r := 0.0;
					FOR i := 0 TO(nRow - 1) DO
						x0 := workMatrix[i, j];
						y0 := workMatrix[i, k];
						p := p + x0 * y0;
						q := q + x0 * x0;
						r := r + y0 * y0;
					END;
					svdVector[j] := q;
					svdVector[k] := r;
					IF q >= r THEN
						IF (q <= e2 * svdVector[1]) OR (ABS(p) <= tol * q) THEN
							rotCount := rotCount - 1;
						ELSE
							p := p / q;
							r := 1.0 - r / q;
							vt := Math.Sqrt(4 * p * p + r * r);
							c0 := Math.Sqrt(0.5 * (1 + r / vt));
							s0 := p / (vt * c0);
							(* rotate *)
							FOR ii := 0 TO(nRow + nCol - 1) DO
								d1 := workMatrix[ii, j];
								d2 := workMatrix[ii, k];
								workMatrix[ii, j] := d1 * c0 + d2 * s0;
								workMatrix[ii, k] :=  - d1 * s0 + d2 * c0;
							END;
						END;
					ELSE
						p := p / r;
						q := q / r - 1.0;
						vt := Math.Sqrt(4 * p * p + q * q);
						s0 := Math.Sqrt(0.5 * (1.0 - q / vt));
						IF p < 0 THEN
							s0 :=  - s0;
						END;
						c0 := p / (vt * s0);
						(* rotate *)
						FOR ii := 0 TO(nRow + nCol - 1) DO
							d1 := workMatrix[ii, j];
							d2 := workMatrix[ii, k];
							workMatrix[ii, j] := d1 * c0 + d2 * s0;
							workMatrix[ii, k] :=  - d1 * s0 + d2 * c0;
						END;
					END;
				END;
			END;
		UNTIL (rotCount = 0) OR (sweepCount > slimit);

		(* ToDo: IF (sweepCount > slimit) THEN error("Sweep limit exceeded"); *)

		(* return svdMat and svdMatrix *)
		FOR i := 0 TO(nRow - 1) DO
			FOR j := 0 TO(nCol - 1) DO
				svdMatrix[i, j] := workMatrix[nRow + i, j];
			END;
			svdVector[i] := Math.Sqrt(svdVector[i]);
		END
	END SVD;

	PROCEDURE Maintainer;
	BEGIN
		version := 323;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		NEW(z, 2);
		NEW(workMatrix, 2, 2)
	END Init;

BEGIN
	Init
END MathMatrix.

