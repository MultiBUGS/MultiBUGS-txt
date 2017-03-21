(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



  *)

MODULE MathDiagmatrix;

	

	IMPORT
		Math,
		MathSparsematrix;

	TYPE
		Matrix = POINTER TO RECORD(MathSparsematrix.Matrix)
			matrix: POINTER TO ARRAY OF REAL
		END;

		LLT = POINTER TO RECORD(MathSparsematrix.LLT)
			matrix: POINTER TO ARRAY OF REAL
		END;

		Factory = POINTER TO RECORD (MathSparsematrix.Factory) END;

	VAR
		fact: Factory;
		version-: INTEGER;
		maintainer-: ARRAY 20 OF CHAR;

	PROCEDURE (m: Matrix) AddDiagonals (IN diags: ARRAY OF REAL; size: INTEGER);
		VAR
			i: INTEGER;
	BEGIN
		i := 0; WHILE i < size DO m.matrix[i] := m.matrix[i] + diags[i]; INC(i) END
	END AddDiagonals;

	PROCEDURE (m: Matrix) LLTFactor (): MathSparsematrix.LLT;
		VAR
			i, size: INTEGER;
			mFactor: LLT;
	BEGIN
		size := LEN(m.matrix);
		NEW(mFactor);
		NEW(mFactor.matrix, size);
		i := 0; WHILE i < size DO mFactor.matrix[i] := Math.Sqrt(m.matrix[i]); INC(i) END;
		RETURN mFactor
	END LLTFactor;

	PROCEDURE (m: Matrix) Multiply (IN x: ARRAY OF REAL; size: INTEGER;
	OUT y: ARRAY OF REAL);
		VAR
			i: INTEGER;
	BEGIN
		i := 0; WHILE i < size DO y[i] := m.matrix[i] * x[i]; INC(i) END
	END Multiply;

	PROCEDURE (m: Matrix) GetMap (OUT rowInd, colPtr: ARRAY OF INTEGER);
	BEGIN
	END GetMap;

	PROCEDURE (m: Matrix) GetElements (OUT elements: ARRAY OF REAL);
		VAR
			i, n: INTEGER;
	BEGIN
		i := 0;
		n := LEN(m.matrix);
		WHILE (i < n) DO
			elements[i] := m.matrix[i];
			INC(i)
		END;
	END GetElements;

	PROCEDURE (m: Matrix) SetMap (IN rowInd, colPtr: ARRAY OF INTEGER);
	BEGIN
	END SetMap;

	PROCEDURE (m: Matrix) SetElements (IN elements: ARRAY OF REAL);
		VAR
			i, size: INTEGER;
	BEGIN
		i := 0;
		size := LEN(elements);
		WHILE i < size DO
			m.matrix[i] := elements[i];
			INC(i)
		END;
	END SetElements;

	PROCEDURE (m: LLT) BackSub (VAR x: ARRAY OF REAL; size: INTEGER);
		VAR
			i: INTEGER;
	BEGIN
		i := 0; WHILE i < size DO x[i] := x[i] / m.matrix[i]; INC(i) END
	END BackSub;

	PROCEDURE (m: LLT) ForwardSub (VAR x: ARRAY OF REAL; size: INTEGER);
		VAR
			i: INTEGER;
	BEGIN
		i := 0; WHILE i < size DO x[i] := x[i] / m.matrix[i]; INC(i) END
	END ForwardSub;

	PROCEDURE (m: LLT) Free;
	BEGIN
	END Free;

	PROCEDURE (m: LLT) Multiply (IN x: ARRAY OF REAL; size: INTEGER;
	OUT y: ARRAY OF REAL);
		VAR
			i: INTEGER;
	BEGIN
		i := 0; WHILE i < size DO y[i] := m.matrix[i] * x[i]; INC(i) END
	END Multiply;

	PROCEDURE (m: LLT) LogDet (): REAL;
		VAR
			i, size: INTEGER;
			logDet: REAL;
	BEGIN
		logDet := 0;
		i := 0;
		size := LEN(m.matrix);
		WHILE i < size DO
			logDet := logDet + Math.Ln(m.matrix[i]);
			INC(i)
		END;
		RETURN 2 * logDet
	END LogDet;

	PROCEDURE (m: LLT) NumElements (): INTEGER;
	BEGIN
		RETURN LEN(m.matrix)
	END NumElements;

	PROCEDURE (m: Matrix) NumElements (): INTEGER;
	BEGIN
		RETURN LEN(m.matrix)
	END NumElements;

	PROCEDURE (m: Matrix) Size (): INTEGER;
	BEGIN
		RETURN LEN(m.matrix)
	END Size;

	PROCEDURE (f: Factory) New (n, nElements: INTEGER): Matrix;
		VAR
			m: Matrix;
	BEGIN
		ASSERT(n = nElements, 20);
		NEW(m);
		NEW(m.matrix, n);
		RETURN m
	END New;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		NEW(fact);
		MathSparsematrix.SetFactory(fact)
	END Init;

	PROCEDURE Install*;
	BEGIN
		MathSparsematrix.SetFactory(fact)
	END Install;

BEGIN
	Init
END MathDiagmatrix.

