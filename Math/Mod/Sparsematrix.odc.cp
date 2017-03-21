(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



  *)

MODULE MathSparsematrix;

	

	IMPORT
		Math,
		MathMatrix;

	TYPE
		Matrix* = POINTER TO ABSTRACT RECORD END;

		StdMatrix = POINTER TO RECORD(Matrix)
			elements: POINTER TO ARRAY OF REAL;
			colPtr, rowInd: POINTER TO ARRAY OF INTEGER
		END;

		LLT* = POINTER TO ABSTRACT RECORD END;

		StdLLT = POINTER TO RECORD(LLT)
			a: POINTER TO ARRAY OF ARRAY OF REAL
		END;

		Factory* = POINTER TO ABSTRACT RECORD END;

		StdFactory = POINTER TO RECORD(Factory) END;

	VAR
		fact-, stdFact-: Factory;
		version-: INTEGER;
		maintainer-: ARRAY 20 OF CHAR;

	PROCEDURE (m: Matrix) AddDiagonals- (IN diags: ARRAY OF REAL; size: INTEGER), NEW, ABSTRACT;

		PROCEDURE (m: Matrix) Multiply- (IN x: ARRAY OF REAL; size: INTEGER;
	OUT y: ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE (m: Matrix) SetMap- (IN rowInd, colPtr: ARRAY OF INTEGER), NEW, ABSTRACT;

	PROCEDURE (m: Matrix) SetElements- (IN elements: ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE (m: Matrix) GetMap- (OUT rowInd, colPtr: ARRAY OF INTEGER), NEW, ABSTRACT;

	PROCEDURE (m: Matrix) GetElements- (OUT elements: ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE (m: Matrix) NumElements- (): INTEGER, NEW, ABSTRACT;

	PROCEDURE (m: Matrix) Size- (): INTEGER, NEW, ABSTRACT;

	PROCEDURE (m: Matrix) LLTFactor- (): LLT, NEW, ABSTRACT;

	PROCEDURE (m: LLT) Free*, NEW, ABSTRACT;

	PROCEDURE (m: LLT) BackSub- (VAR x: ARRAY OF REAL; size: INTEGER), NEW, ABSTRACT;

	PROCEDURE (m: LLT) ForwardSub- (VAR x: ARRAY OF REAL; size: INTEGER), NEW, ABSTRACT;

	PROCEDURE (m: LLT) LogDet- (): REAL, NEW, ABSTRACT;

		PROCEDURE (m: LLT) Multiply- (IN x: ARRAY OF REAL; size: INTEGER;
	OUT y: ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE (m: LLT) NumElements- (): INTEGER, NEW, ABSTRACT;

	PROCEDURE (f: Factory) New- (size, nElements: INTEGER): Matrix, NEW, ABSTRACT;

	PROCEDURE (m: StdMatrix) AddDiagonals (IN diags: ARRAY OF REAL; size: INTEGER);
		VAR
			col, i: INTEGER;
	BEGIN
		i := 0;
		WHILE i < size DO
			col := m.colPtr[i];
			m.elements[col] := m.elements[col] + diags[i];
			INC(i)
		END
	END AddDiagonals;

	PROCEDURE (m: StdMatrix) Multiply (IN x: ARRAY OF REAL; size: INTEGER;
	OUT y: ARRAY OF REAL);
		VAR
			i, j, k, nElements: INTEGER;
			a: POINTER TO ARRAY OF ARRAY OF REAL;
	BEGIN
		NEW(a, size, size);
		i := 0;
		WHILE i < size DO
			j := 0;
			WHILE j < size DO
				a[i, j] := 0.0;
				INC(j)
			END;
			INC(i)
		END;
		k := 0;
		nElements := LEN(m.elements);
		WHILE k < nElements DO
			i := m.rowInd[k];
			j := size;
			WHILE m.colPtr[j] > k DO DEC(j); END;
			a[i, j] := m.elements[k];
			a[j, i] := a[i, j];
			INC(k);
		END;
		i := 0;
		WHILE i < size DO
			y[i] := 0.0;
			j := 0;
			WHILE j < size DO
				y[i] := y[i] + a[i, j] * x[j];
				INC(j)
			END;
			INC(i)
		END
	END Multiply;

	PROCEDURE (m: StdMatrix) SetMap (IN rowInd, colPtr: ARRAY OF INTEGER);
		VAR
			i, n, nElements: INTEGER;
	BEGIN
		i := 0;
		n := LEN(m.colPtr) - 1;
		nElements := LEN(m.rowInd);
		WHILE i < n DO m.colPtr[i] := colPtr[i]; INC(i) END;
		m.colPtr[n] := nElements;
		i := 0;
		WHILE i < nElements DO m.rowInd[i] := rowInd[i]; INC(i) END
	END SetMap;

	PROCEDURE (m: StdMatrix) SetElements (IN elements: ARRAY OF REAL);
		VAR
			i, nElements: INTEGER;
	BEGIN
		nElements := LEN(m.rowInd);
		i := 0;
		WHILE i < nElements DO m.elements[i] := elements[i]; INC(i) END;
		INC(i)
	END SetElements;

	PROCEDURE (m: StdMatrix) GetElements (OUT elements: ARRAY OF REAL);
		VAR
			i, nElements: INTEGER;
	BEGIN
		i := 0;
		nElements := LEN(m.elements);
		WHILE i < nElements DO
			elements[i] := m.elements[i];
			INC(i);
		END
	END GetElements;

	PROCEDURE (m: StdMatrix) GetMap (OUT rowInd, colPtr: ARRAY OF INTEGER);
		VAR
			i, nElements, nCols: INTEGER;
	BEGIN
		i := 0;
		nCols := LEN(m.colPtr);
		nElements := LEN(m.elements);
		WHILE i < nCols DO
			colPtr[i] := m.colPtr[i];
			INC(i);
		END;
		i := 0;
		WHILE i < nElements DO
			rowInd[i] := m.rowInd[i];
			INC(i);
		END
	END GetMap;

	PROCEDURE (m: StdMatrix) NumElements (): INTEGER;
	BEGIN
		RETURN LEN(m.elements)
	END NumElements;

	PROCEDURE (m: StdMatrix) Size (): INTEGER;
	BEGIN
		RETURN LEN(m.colPtr) - 1
	END Size;

	PROCEDURE (m: StdMatrix) LLTFactor (): LLT;
		VAR
			i, j, k, n, nElements: INTEGER;
			llt: StdLLT;
	BEGIN
		n := LEN(m.colPtr) - 1;
		nElements := LEN(m.rowInd);
		NEW(llt);
		NEW(llt.a, n, n);
		i := 0;
		WHILE i < n DO
			j := 0;
			WHILE j < n DO
				llt.a[i, j] := 0.0; INC(j)
			END;
			INC(i)
		END;
		k := 0;
		WHILE k < nElements DO
			i := m.rowInd[k];
			j := n - 1;
			WHILE m.colPtr[j] > k DO DEC(j); END;
			llt.a[i, j] := m.elements[k];
			llt.a[j, i] := llt.a[i, j];
			INC(k);
		END;
		MathMatrix.Cholesky(llt.a, n);
		RETURN llt
	END LLTFactor;

	PROCEDURE (m: StdLLT) Free;
	BEGIN
	END Free;

	PROCEDURE (m: StdLLT) BackSub (VAR x: ARRAY OF REAL; size: INTEGER);
		VAR
			n: INTEGER;
	BEGIN
		n := LEN(m.a, 0);
		MathMatrix.BackSub(m.a, x, n)
	END BackSub;

	PROCEDURE (m: StdLLT) ForwardSub (VAR x: ARRAY OF REAL; size: INTEGER);
		VAR
			n: INTEGER;
	BEGIN
		n := LEN(m.a, 0);
		MathMatrix.ForwardSub(m.a, x, n)
	END ForwardSub;

	PROCEDURE (m: StdLLT) LogDet (): REAL;
		VAR
			i, n: INTEGER;
			logDet: REAL;
	BEGIN
		logDet := 0.0;
		i := 0;
		n := LEN(m.a, 0);
		WHILE i < n DO
			logDet := logDet + Math.Ln(m.a[i, i]);
			INC(i)
		END;
		RETURN 2 * logDet
	END LogDet;

	PROCEDURE (m: StdLLT) Multiply (IN x: ARRAY OF REAL; size: INTEGER; OUT y: ARRAY OF REAL);
		VAR
			i, j: INTEGER;
			temp: REAL;
	BEGIN
		i := 0;
		WHILE i < size DO
			j := i;
			temp := 0.0;
			WHILE j < size DO
				temp := temp + m.a[i, j] * x[j];
				INC(j)
			END;
			y[i] := temp;
			INC(i)
		END
	END Multiply;

	PROCEDURE (m: StdLLT) NumElements (): INTEGER;
		VAR
			n: INTEGER;
	BEGIN
		n := LEN(m.a, 0);
		RETURN ((n + 1) * n) DIV 2
	END NumElements;

	PROCEDURE (f: StdFactory) New (size, nElements: INTEGER): Matrix;
		VAR
			m: StdMatrix;
	BEGIN
		NEW(m);
		NEW(m.colPtr, size + 1);
		NEW(m.rowInd, nElements);
		NEW(m.elements, nElements);
		RETURN m
	END New;

	PROCEDURE AddDiagonals* (m: Matrix; IN diag: ARRAY OF REAL; size: INTEGER);
	BEGIN
		m.AddDiagonals(diag, size)
	END AddDiagonals;

	PROCEDURE Multiply* (m: Matrix; IN x: ARRAY OF REAL; size: INTEGER; OUT y: ARRAY OF REAL);
	BEGIN
		m.Multiply(x, size, y)
	END Multiply;

	PROCEDURE SetMap* (m: Matrix; IN rowInd, colPtr: ARRAY OF INTEGER);
	BEGIN
		m.SetMap(rowInd, colPtr)
	END SetMap;

	PROCEDURE SetElements* (m: Matrix; IN values: ARRAY OF REAL);
	BEGIN
		m.SetElements(values)
	END SetElements;

	PROCEDURE GetMap* (m: Matrix; OUT rowInd, colPtr: ARRAY OF INTEGER);
	BEGIN
		m.GetMap(rowInd, colPtr);
	END GetMap;

	PROCEDURE GetElements* (m: Matrix; OUT values: ARRAY OF REAL);
	BEGIN
		m.GetElements(values);
	END GetElements;

	PROCEDURE NumMatrixElements* (m: Matrix): INTEGER;
	BEGIN
		RETURN m.NumElements();
	END NumMatrixElements;

	PROCEDURE Size* (m: Matrix): INTEGER;
	BEGIN
		RETURN m.Size();
	END Size;

	PROCEDURE LLTFactor* (m: Matrix): LLT;
	BEGIN
		RETURN m.LLTFactor()
	END LLTFactor;

	PROCEDURE BackSub* (m: LLT; VAR x: ARRAY OF REAL; size: INTEGER);
	BEGIN
		m.BackSub(x, size)
	END BackSub;

	PROCEDURE ForwardSub* (m: LLT; VAR x: ARRAY OF REAL; size: INTEGER);
	BEGIN
		m.ForwardSub(x, size)
	END ForwardSub;

	PROCEDURE LogDet* (m: LLT): REAL;
	BEGIN
		RETURN m.LogDet()
	END LogDet;

	PROCEDURE MultiplyLLT* (m: LLT; VAR x: ARRAY OF REAL; size: INTEGER; OUT y: ARRAY OF REAL);
	BEGIN
		m.Multiply(x, size, y)
	END MultiplyLLT;

	PROCEDURE NumElements* (m: LLT): INTEGER;
	BEGIN
		RETURN m.NumElements()
	END NumElements;

	PROCEDURE New* (size, nElements: INTEGER): Matrix;
	BEGIN
		IF fact # NIL THEN
			RETURN fact.New(size, nElements)
		ELSE
			RETURN NIL
		END
	END New;

	PROCEDURE SetFactory* (f: Factory);
	BEGIN
		fact := f
	END SetFactory;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			stdF: StdFactory;
	BEGIN
		Maintainer;
		NEW(stdF);
		stdFact := stdF;
		fact := stdFact
	END Init;

BEGIN
	Init
END MathSparsematrix.
