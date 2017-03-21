(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



  *)

MODULE MathTaucsImp;

	

	IMPORT
		Math,
		MathSparsematrix, MathTaucsLib;

	CONST
		lower = MathTaucsLib.lower;
		symmetric = MathTaucsLib.symmetric;
		double = MathTaucsLib.double;

	TYPE
		Matrix = POINTER TO RECORD(MathSparsematrix.Matrix)
			taucsMatrix: MathTaucsLib.Matrix
		END;

		LLT = POINTER TO RECORD(MathSparsematrix.LLT)
			taucsMatrix: MathTaucsLib.Matrix
		END;

		Factory = POINTER TO RECORD (MathSparsematrix.Factory) END;

	VAR
		fact: Factory;
		version-: INTEGER;
		maintainer-: ARRAY 20 OF CHAR;

	PROCEDURE (m: Matrix) FINALIZE;
	BEGIN
		IF m.taucsMatrix # NIL THEN
			MathTaucsLib.Free(m.taucsMatrix);
			m.taucsMatrix := NIL
		END
	END FINALIZE;

	PROCEDURE (m: Matrix) AddDiagonals (IN diags: ARRAY OF REAL; size: INTEGER);
		VAR
			i: INTEGER;
	BEGIN
		size := m.taucsMatrix.n;
		i := 0;
		WHILE i < size DO
			m.taucsMatrix.values[m.taucsMatrix.colptr[i]] := m.taucsMatrix.values[m.taucsMatrix.colptr[i]] + diags[i];
			INC(i)
		END;
	END AddDiagonals;

	(*PROCEDURE (m: Matrix) LLTFactor (): MathSparsematrix.LLT;
	VAR
	supernodalFactor: ANYPTR;
	mFactor: LLT;
	BEGIN
	supernodalFactor := MathTaucsLib.SupernodalFactorLLT(m.taucsMatrix);
	ASSERT(supernodalFactor # NIL, 100);
	NEW(mFactor);
	mFactor.taucsMatrix := MathTaucsLib.SupernodalToCCS(supernodalFactor);
	ASSERT(mFactor.taucsMatrix # NIL, 100);
	MathTaucsLib.FreeSupernodalFactor(supernodalFactor);
	RETURN mFactor
	END LLTFactor;*)

	PROCEDURE (m: Matrix) LLTFactor (): MathSparsematrix.LLT;
		VAR
			mFactor: LLT;
	BEGIN
		NEW(mFactor);
		mFactor.taucsMatrix := MathTaucsLib.FactorLLT(m.taucsMatrix, 0, 0);
		ASSERT(mFactor.taucsMatrix # NIL, 100);
		RETURN mFactor
	END LLTFactor;

	PROCEDURE (m: Matrix) Multiply (IN x: ARRAY OF REAL; size: INTEGER;
	OUT y: ARRAY OF REAL);
	BEGIN
		MathTaucsLib.MatrixTimesVector(m.taucsMatrix, x, y);
	END Multiply;

	PROCEDURE (m: Matrix) SetMap (IN rowInd, colPtr: ARRAY OF INTEGER);
		VAR
			i, n, nElements: INTEGER;
	BEGIN
		i := 0;
		n := LEN(colPtr);
		nElements := LEN(rowInd);
		WHILE i < n DO
			m.taucsMatrix.colptr[i] := colPtr[i];
			INC(i)
		END;
		m.taucsMatrix.colptr[n] := nElements;
		i := 0;
		WHILE i < nElements DO
			m.taucsMatrix.rowind[i] := rowInd[i];
			INC(i)
		END;
	END SetMap;

	PROCEDURE (m: Matrix) SetElements (IN elements: ARRAY OF REAL);
		VAR
			i, nElements: INTEGER;
	BEGIN
		i := 0;
		nElements := LEN(elements);
		i := 0;
		WHILE i < nElements DO
			m.taucsMatrix.values[i] := elements[i];
			INC(i)
		END;
	END SetElements;

	PROCEDURE (m: Matrix) GetElements (OUT elements: ARRAY OF REAL);
		VAR
			i: INTEGER;
	BEGIN
		i := 0;
		WHILE i < m.taucsMatrix.colptr[m.taucsMatrix.n] DO
			elements[i] := m.taucsMatrix.values[i]; INC(i)
		END;
	END GetElements;

	PROCEDURE (m: Matrix) GetMap (OUT rowInd, colPtr: ARRAY OF INTEGER);
		VAR
			i: INTEGER;
	BEGIN
		i := 0;
		WHILE i < m.taucsMatrix.m DO
			colPtr[i] := m.taucsMatrix.colptr[i]; INC(i)
		END;
		i := 0;
		WHILE i < m.taucsMatrix.colptr[m.taucsMatrix.n] DO
			rowInd[i] := m.taucsMatrix.rowind[i]; INC(i)
		END;
	END GetMap;

	PROCEDURE (m: Matrix) NumElements (): INTEGER;
	BEGIN
		RETURN m.taucsMatrix.colptr[m.taucsMatrix.n]
	END NumElements;

	PROCEDURE (m: Matrix) Size (): INTEGER;
	BEGIN
		RETURN m.taucsMatrix.m
	END Size;

	PROCEDURE (m: LLT) BackSub (VAR x: ARRAY OF REAL; size: INTEGER);
		VAR
			y: POINTER TO ARRAY OF REAL;
			i, j, jp: INTEGER;
	BEGIN
		size := m.taucsMatrix.n;
		NEW(y, size);
		i := size - 1;
		WHILE i >= 0 DO
			jp := m.taucsMatrix.colptr[i] + 1;
			WHILE jp < m.taucsMatrix.colptr[i + 1] DO
				j := m.taucsMatrix.rowind[jp];
				x[i] := x[i] - y[j] * m.taucsMatrix.values[jp];
				INC(jp);
			END;
			jp := m.taucsMatrix.colptr[i];
			j := m.taucsMatrix.rowind[jp];
			y[i] := x[i] / m.taucsMatrix.values[jp];
			DEC(i);
		END;
		i := 0;
		WHILE i < size DO
			x[i] := y[i];
			INC(i);
		END;
	END BackSub;

	PROCEDURE (m: LLT) ForwardSub (VAR x: ARRAY OF REAL; size: INTEGER);
		VAR
			y: REAL;
			i, j, ip: INTEGER;
	BEGIN
		size := m.taucsMatrix.n;
		j := 0;
		WHILE j < size DO
			ip := m.taucsMatrix.colptr[j];
			i := m.taucsMatrix.rowind[ip];
			ASSERT(i = j);
			y := x[j] / m.taucsMatrix.values[ip];
			INC(ip);
			WHILE ip < m.taucsMatrix.colptr[j + 1] DO
				i := m.taucsMatrix.rowind[ip];
				x[i] := x[i] - y * m.taucsMatrix.values[ip];
				INC(ip);
			END;
			x[j] := y;
			INC(j);
		END;
	END ForwardSub;

	PROCEDURE (m: LLT) Free;
	BEGIN
		IF m.taucsMatrix # NIL THEN
			MathTaucsLib.Free(m.taucsMatrix)
		END;
		m.taucsMatrix := NIL
	END Free;

	PROCEDURE (m: LLT) FINALIZE;
	BEGIN
		m.Free
	END FINALIZE;

	PROCEDURE (m: LLT) Multiply (IN x: ARRAY OF REAL; size: INTEGER;
	OUT y: ARRAY OF REAL);
	BEGIN
		MathTaucsLib.MatrixTimesVector(m.taucsMatrix, x, y);
	END Multiply;

	PROCEDURE (m: LLT) LogDet (): REAL;
		VAR
			diagIndex, i, n: INTEGER;
			logDet: REAL;
	BEGIN
		logDet := 0;
		i := 0;
		n := m.taucsMatrix.n;
		WHILE i < n DO
			diagIndex := m.taucsMatrix.colptr[i];
			logDet := logDet + Math.Ln(m.taucsMatrix.values[diagIndex]);
			INC(i)
		END;
		RETURN 2 * logDet
	END LogDet;

	PROCEDURE (m: LLT) NumElements (): INTEGER;
	BEGIN
		RETURN m.taucsMatrix.colptr[m.taucsMatrix.n]
	END NumElements;

	PROCEDURE (f: Factory) New (n, nElements: INTEGER): Matrix;
		VAR
			m: Matrix;
	BEGIN
		NEW(m);
		m.taucsMatrix := MathTaucsLib.Create(n, n, nElements, {lower, symmetric, double});
		RETURN m
	END New;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "Jussi Alho"
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
END MathTaucsImp.

