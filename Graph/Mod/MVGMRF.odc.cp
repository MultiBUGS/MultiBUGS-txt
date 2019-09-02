(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphMVGMRF;

	

	IMPORT
		Math, Stores := Stores64,
		GraphMRF, GraphMultivariate, GraphNodes, GraphRules, GraphStochastic,
		MathMatrix, MathRandnum, MathSparsematrix;

	TYPE
		Node* = POINTER TO ABSTRACT RECORD(GraphMRF.Node)
			tau-: GraphNodes.Vector;
			dim-, tauStart-, tauStep-: INTEGER
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		mean: POINTER TO ARRAY OF REAL;
		prec: POINTER TO ARRAY OF ARRAY OF REAL;

	PROCEDURE (node: Node) BlockSize* (): INTEGER;
	BEGIN
		RETURN node.dim
	END BlockSize;

	PROCEDURE (node: Node) MVLikelihoodForm* (as: INTEGER; OUT x: GraphNodes.Vector;
	OUT start, step: INTEGER;
	OUT p0: ARRAY OF REAL; OUT p1: ARRAY OF ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE (node: Node) CheckMVGMRF- (): SET, NEW, ABSTRACT;

	PROCEDURE (node: Node) CheckChain- (): SET;
	BEGIN
		RETURN node.CheckMVGMRF()
	END CheckChain;

	PROCEDURE (node: Node) ClassifyLikelihood* (parent: GraphStochastic.Node): INTEGER;
		VAR
			class, f, i, j, numConst, numIdent, numProd, tauStart, tauStep: INTEGER;
			tau: GraphNodes.Node;
	BEGIN
		numConst := 0;
		numIdent := 0;
		numProd := 0;
		tauStart := node.tauStart;
		tauStep := node.tauStep;
		WITH parent: GraphMultivariate.Node DO
			i := 0;
			WHILE i < node.dim DO
				j := 0;
				WHILE j < node.dim DO
					tau := node.tau[i * node.dim + j];
					f := GraphStochastic.ClassFunction(tau, parent.components[tauStart + (i * node.dim + j) * tauStep]);
					IF f = GraphRules.const THEN INC(numConst)
					ELSIF f = GraphRules.ident THEN INC(numIdent)
					ELSIF f = GraphRules.prod THEN INC(numProd)
					END;
					INC(j)
				END;
				INC(i)
			END;
			IF numIdent = node.dim * node.dim THEN
				class := GraphRules.wishart
			ELSE
				class := GraphRules.invalid
			END
		ELSE
			class := GraphRules.general
		END;
		RETURN class
	END ClassifyLikelihood;

	PROCEDURE (node: Node) ClassifyPrior* (): INTEGER;
	BEGIN
		RETURN GraphRules.mVN
	END ClassifyPrior;

	PROCEDURE (node: Node) Constraints* (OUT constraints: ARRAY OF ARRAY OF REAL);
		VAR
			dim, i, j, size: INTEGER;
	BEGIN
		dim := node.dim;
		size := node.Size() DIV dim;
		i := 0;
		WHILE i < dim DO
			j := 0; WHILE j < dim * size DO constraints[i, j] := 0.0; INC(j) END;
			j := 0; WHILE j < size DO constraints[i, i + dim * j] := 1.0; INC(j) END;
			INC(i)
		END;
	END Constraints;

	PROCEDURE (node: Node) Deviance* (): REAL;
	BEGIN
		HALT(126);
		RETURN 0
	END Deviance;

	PROCEDURE (node: Node) DiffLogLikelihood* (x: GraphStochastic.Node): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior* (): REAL;
		VAR
			dim, i, j, index: INTEGER;
			sum: REAL;
	BEGIN
		node.MVPriorForm(mean, prec);
		dim := node.dim;
		i := 0;
		index := node.index;
		index := (index DIV dim) * dim;
		WHILE i < dim DO
			mean[i] := mean[i] - node.components[index].value;
			INC(index);
			INC(i)
		END;
		index := node.index MOD dim;
		sum := 0.0;
		j := 0;
		WHILE j < dim DO
			sum := sum + prec[index, j] * mean[j];
			INC(j)
		END;
		RETURN - sum
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeMVGMRF- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (node: Node) ExternalizeChain- (VAR wr: Stores.Writer);
		VAR
			dim: INTEGER;
			v: GraphNodes.SubVector;
	BEGIN
		IF node.index = 0 THEN
			dim := node.dim;
			wr.WriteInt(node.dim);
			v.Init;
			v.components := node.tau;
			v.start := node.tauStart; v.nElem := dim * dim; v.step := node.tauStep;
			GraphNodes.ExternalizeSubvector(v, wr);
		END;
		node.ExternalizeMVGMRF(wr)
	END ExternalizeChain;

	PROCEDURE (node: Node) InitMVGMRF-, NEW, ABSTRACT;

	PROCEDURE (node: Node) InitStochastic-;
	BEGIN
		node.dim := 0;
		node.tau := NIL;
		node.tauStart := - 1;
		node.tauStep := 0;
		INCL(node.props, GraphStochastic.noMean);
		node.InitMVGMRF
	END InitStochastic;

	PROCEDURE (node: Node) InternalizeMVGMRF- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (node: Node) InternalizeChain- (VAR rd: Stores.Reader);
		VAR
			dim: INTEGER;
			v: GraphNodes.SubVector;
			p: Node;
	BEGIN
		IF node.index = 0 THEN
			rd.ReadInt(dim);
			node.dim := dim;
			IF (mean = NIL) OR (dim > LEN(mean)) THEN
				NEW(mean, dim);
				NEW(prec, dim, dim);
			END;
			GraphNodes.InternalizeSubvector(v, rd);
			node.tau := v.components;
			node.tauStart := v.start;
			node.tauStep := v.step
		ELSE
			p := node.components[0](Node);
			node.tau := p.tau;
			node.tauStart := p.tauStart;
			node.tauStep := p.tauStep
		END;
		node.InternalizeMVGMRF(rd)
	END InternalizeChain;

	PROCEDURE (node: Node) LikelihoodForm* (as: INTEGER; VAR x: GraphNodes.Node; OUT p0, p1: REAL);
	BEGIN
	END LikelihoodForm;

	PROCEDURE (node: Node) LogLikelihood* (): REAL;
		VAR
			as, dim, i, j, tauStart, tauStep: INTEGER;
			x: GraphNodes.Vector;
			p0: ARRAY 1 OF REAL;
			tau, trace, likelihood, k: REAL;
	BEGIN
		as := GraphRules.wishart;
		node.MVLikelihoodForm(as, x, tauStart, tauStep, p0, prec);
		k := p0[0];
		dim := node.dim;
		i := 0;
		trace := 0.0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				tau := node.tau[tauStart + (i + j * dim) * tauStep].value;
				trace := trace + prec[i, j] * tau;
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				prec[i, j] := node.tau[tauStart + (i + j * dim) * tauStep].value;
				INC(j)
			END;
			INC(i)
		END;
		likelihood := 0.5 * (k - dim - 1) * MathMatrix.LogDet(prec, dim) + trace;
		RETURN likelihood
	END LogLikelihood;

	PROCEDURE (node: Node) Location* (): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END Location;

	PROCEDURE (node: Node) LogMVPrior* (): REAL;
		VAR
			dim, i, j, index: INTEGER;
			sum: REAL;
	BEGIN
		node.MVPriorForm(mean, prec);
		dim := node.dim;
		i := 0;
		index := node.index;
		index := (index DIV dim) * dim;
		WHILE i < dim DO
			mean[i] := mean[i] - node.components[index].value;
			INC(index);
			INC(i)
		END;
		i := 0;
		sum := 0.0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				sum := sum + mean[i] * prec[i, j] * mean[j];
				INC(j)
			END;
			INC(i)
		END;
		RETURN - 0.5 * sum
	END LogMVPrior;

	PROCEDURE (node: Node) LogPrior* (): REAL;
	BEGIN
		RETURN node.LogMVPrior()
	END LogPrior;

	PROCEDURE (node: Node) MVSample* (OUT res: SET);
		VAR
			i, j, k, numConstraints, nElements, size, type: INTEGER;
			rowInd, colPtr: POINTER TO ARRAY OF INTEGER;
			elements, c, eps, x: POINTER TO ARRAY OF REAL;
			A, U, V, Winverse: POINTER TO ARRAY OF ARRAY OF REAL;
			m: MathSparsematrix.Matrix;
			llt: MathSparsematrix.LLT;
	BEGIN
		res := {};
		size := node.Size();
		node.MatrixInfo(type, nElements);
		numConstraints := node.NumberConstraints();
		NEW(colPtr, size);
		NEW(rowInd, nElements);
		NEW(elements, nElements);
		NEW(x, size);
		NEW(eps, size);
		NEW(Winverse, numConstraints, numConstraints);
		NEW(c, numConstraints);
		NEW(A, numConstraints, size);
		NEW(U, numConstraints, size);
		NEW(V, numConstraints, size);
		node.MatrixMap(rowInd, colPtr);
		node.MatrixElements(elements);
		m := MathSparsematrix.New(size, nElements);
		MathSparsematrix.SetMap(m, rowInd, colPtr);
		MathSparsematrix.SetElements(m, elements);
		i := 0; WHILE i < size DO eps[i] := 1.0E-6; INC(i) END;
		MathSparsematrix.AddDiagonals(m, eps, size);
		llt := MathSparsematrix.LLTFactor(m);
		i := 0;
		WHILE i < size DO
			x[i] := MathRandnum.StandardNormal();
			INC(i)
		END;
		MathSparsematrix.BackSub(llt, x, size);
		node.Constraints(A);
		j := 0;
		WHILE j < numConstraints DO
			i := 0; WHILE i < size DO V[j, i] := A[j, i]; INC(i) END;
			MathSparsematrix.ForwardSub(llt, V[j], size);
			MathSparsematrix.BackSub(llt, V[j], size);
			INC(j)
		END;
		i := 0;
		WHILE i < numConstraints DO
			j := 0;
			WHILE j < numConstraints DO
				Winverse[i, j] := MathMatrix.DotProduct(A[i], V[j], size);
				INC(j)
			END;
			INC(i)
		END;
		MathMatrix.Invert(Winverse, numConstraints);
		i := 0;
		WHILE i < numConstraints DO
			j := 0; WHILE j < size DO U[i, j] := 0.0; INC(j) END;
			INC(i)
		END;
		i := 0;
		WHILE i < numConstraints DO
			j := 0;
			WHILE j < size DO
				k := 0; WHILE k < numConstraints DO U[i, j] := U[i, j] + Winverse[i, k] * V[k, j]; INC(k) END;
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < numConstraints DO
			c[i] := 0;
			j := 0;
			WHILE j < size DO
				c[i] := c[i] + A[i, j] * x[j];
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < size DO
			j := 0;
			WHILE j < numConstraints DO x[i] := x[i] - U[j, i] * c[j]; INC(j) END;
			INC(i)
		END;
		i := 0;
		WHILE i < size DO
			node.components[i].value := x[i]; INC(i)
		END;
	END MVSample;

	PROCEDURE (node: Node) Parents* (all: BOOLEAN): GraphNodes.List;
		VAR
			dim, i, tauStart, tauStep: INTEGER;
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		dim := node.dim;
		IF node.index MOD dim = 0 THEN
			tauStart := node.tauStart;
			tauStep := node.tauStep;
			i := 0;
			WHILE i < dim * dim DO
				p := node.tau[tauStart + i * tauStep];
				IF p # NIL THEN p.AddParent(list) END;
				INC(i)
			END;
			GraphNodes.ClearList(list)
		END;
		RETURN list
	END Parents;

	PROCEDURE (node: Node) PriorForm* (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
	END PriorForm;

	PROCEDURE (node: Node) Sample* (OUT res: SET);
	BEGIN
		res := {GraphNodes.lhs}
	END Sample;

	PROCEDURE (node: Node) SetMVGMRF- (IN args: GraphNodes.Args; OUT res: SET), NEW, ABSTRACT;

	PROCEDURE (node: Node) Set* (IN args: GraphNodes.Args; OUT res: SET);
		CONST
			eps = 1.0E-10;
		VAR
			dim, i, last, start, step: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			last := args.numVectors - 1;
			ASSERT(args.vectors[last].components # NIL, 20);
			ASSERT(args.vectors[last].start >= 0, 21);
			ASSERT(args.vectors[last].nElem > 0, 22);
			dim := SHORT(ENTIER(Math.Sqrt(args.vectors[last].nElem) + eps));
			IF dim > LEN(mean) THEN
				NEW(mean, dim); NEW(prec, dim, dim)
			END;
			node.dim := dim;
			node.tau := args.vectors[last].components;
			node.tauStart := args.vectors[last].start;
			node.tauStep := args.vectors[last].step;
			i := 0;
			start := node.tauStart;
			step := node.tauStep;
			WHILE i < dim * dim DO
				p := node.tau[start + i * step];
				IF p = NIL THEN
					res := {GraphNodes.arg1, GraphNodes.nil}; RETURN
				END;
				INC(i)
			END
		END;
		node.SetMVGMRF(args, res)
	END Set;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		CONST
			minSize = 10;
	BEGIN
		Maintainer;
		NEW(mean, minSize);
		NEW(prec, minSize, minSize);
	END Init;

BEGIN
	Init
END GraphMVGMRF.
