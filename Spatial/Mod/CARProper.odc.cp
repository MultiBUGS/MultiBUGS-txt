(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE SpatialCARProper;

	

	IMPORT
		Math, Stores,
		GraphMRF, GraphMultivariate, GraphNodes, GraphParamtrans,
		GraphRules, GraphStochastic,
		MathFunc, MathMatrix, MathRandnum, MathSparsematrix;

	TYPE

		Node = POINTER TO RECORD (GraphMRF.Node)
			adj, num, offset: POINTER TO ARRAY OF INTEGER;
			c, m, eigenvalues: POINTER TO ARRAY OF REAL;
			mu: GraphNodes.Vector;
			tau, gamma: GraphNodes.Node;
			gammaMax, gammaMin: REAL;
			muStart, muStep: INTEGER
		END;

		Factory = POINTER TO RECORD (GraphMultivariate.Factory) END;

	VAR
		fact-: GraphMultivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		v0, v1, v2: POINTER TO ARRAY OF REAL;
		log2Pi: REAL;
		start, step: INTEGER;
		
		PROCEDURE EigenValues (IN c, m: ARRAY OF REAL; IN adj, cols: ARRAY OF INTEGER;
	OUT eigenvalues: ARRAY OF REAL);
		VAR
			mcm: POINTER TO ARRAY OF REAL; row, j, k, len, n, col: INTEGER; mRow, mCol: REAL;
	BEGIN
		len := LEN(c); n := LEN(cols);
		NEW(mcm, len);
		row := 0; j := 0;
		WHILE row < n DO
			mRow := m[row];
			k := j + cols[row];
			WHILE j < k DO
				col := adj[j];
				mCol := m[col];
				mcm[j] := (1.0 / Math.Sqrt(mRow)) * c[j] * Math.Sqrt(mCol);
				INC(j)
			END;
			INC(row)
		END;
		MathMatrix.SparseEigenvalues(adj, cols, mcm, n, eigenvalues)
	END EigenValues;

	PROCEDURE LogDet (node: Node): REAL;
		VAR
			gamma, logDet: REAL;
			i, nElem: INTEGER;
	BEGIN
		logDet := 0.0;
		gamma := node.gamma.Value();
		i := 0;
		nElem := node.Size();
		WHILE i < nElem DO
			logDet := logDet + Math.Ln(1.0 - gamma * node.eigenvalues[i]); INC(i)
		END;
		RETURN logDet
	END LogDet;

	PROCEDURE QuadraticForm (node: Node): REAL;
		CONST
			eps = 1.0E-10;
		VAR
			quadForm, gamma, nodeMu: REAL;
			i, nElem, muStart, muStep: INTEGER;
	BEGIN
		gamma := node.gamma.Value();
		IF gamma > node.gammaMax + eps THEN RETURN MathFunc.logOfZero END;
		IF gamma < node.gammaMin - eps THEN RETURN MathFunc.logOfZero END;
		i := 0;
		nElem := node.Size();
		IF LEN(v0) < nElem THEN
			NEW(v0, nElem); NEW(v1, nElem); NEW(v2, nElem)
		END;
		muStart := node.muStart;
		muStep := node.muStep;
		WHILE i < nElem DO
			nodeMu := node.mu[muStart + i * muStep].Value();
			v0[i] := node.components[i].value - nodeMu;
			v1[i] := v0[i] / node.m[i];
			INC(i)
		END;
		quadForm := 0.50 * MathMatrix.DotProduct(v1, v0, nElem);
		ASSERT(quadForm > 0, 66);
		MathMatrix.SparseMultiply(node.adj, node.num, node.c, v0, nElem, v2);
		quadForm := quadForm - 0.50 * gamma * MathMatrix.DotProduct(v1, v2, nElem);
		ASSERT(quadForm > 0, 66);
		RETURN quadForm
	END QuadraticForm;

	PROCEDURE (node: Node) Bounds (OUT lower, upper: REAL);
	BEGIN
		lower :=  - INF;
		upper := INF
	END Bounds;

	PROCEDURE (node: Node) Check (): SET;
		CONST
			eps = 1.0E-40;
		VAR
			tau, gamma: REAL;
	BEGIN
		tau := node.tau.Value();
		IF tau < eps THEN RETURN {GraphNodes.arg6, GraphNodes.invalidPosative} END;
		gamma := node.gamma.Value();
		IF gamma < node.gammaMin - eps THEN RETURN {GraphNodes.arg7, GraphNodes.invalidValue} END;
		IF gamma > node.gammaMax + eps THEN RETURN {GraphNodes.arg7, GraphNodes.invalidValue} END;
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassifyLikelihood (parent: GraphStochastic.Node): INTEGER;
		CONST
			linear = {GraphRules.const, GraphRules.ident, GraphRules.prod, GraphRules.linear};
		VAR
			f0, f1, density, i, nElem, muStart, muStep: INTEGER;
	BEGIN
		f0 := GraphStochastic.ClassFunction(node.gamma, parent);
		IF f0 # GraphRules.const THEN
			density := GraphRules.general
		ELSE
			f0 := GraphStochastic.ClassFunction(node.tau, parent);
			density := GraphRules.ClassifyPrecision(f0);
			f1 := GraphRules.const;
			i := 0; nElem := node.Size(); muStart := node.muStart; muStep := node.muStep;
			WHILE (i < nElem) & (f1 IN linear) DO
				f1 := GraphStochastic.ClassFunction(node.mu[muStart + i * muStep], parent);
				INC(i)
			END;
			IF ~(f1 IN linear) THEN
				density := GraphRules.general
			ELSIF f1 # GraphRules.const THEN
				IF f0 # GraphRules.const THEN
					density := GraphRules.general
				ELSE
					density := GraphRules.normal
				END
			END
		END;
		RETURN density
	END ClassifyLikelihood;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.normal
	END ClassifyPrior;

	PROCEDURE (node: Node) Constraints (OUT constraints: ARRAY OF ARRAY OF REAL);
	BEGIN
		HALT(126)
	END Constraints;

	PROCEDURE (node: Node) Deviance (): REAL;
	BEGIN
		ASSERT(node.index = 0, 21);
		RETURN - 2.0 * node.LogLikelihood() + log2Pi * node.Size()
	END Deviance;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			differential, diffTau, p0, p1, tau: REAL;
			y: GraphNodes.Node;
	BEGIN
		node.LikelihoodForm(GraphRules.gamma, y, p0, p1);
		node.tau.ValDiff(x, tau, diffTau);
		differential := diffTau * (p0 / tau - p1);
		RETURN differential
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			x, mu, tau, differential: REAL;
	BEGIN
		x := node.value;
		node.PriorForm(GraphRules.normal, mu, tau);
		differential :=  - tau * (x - mu);
		RETURN differential
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeChain (VAR wr: Stores.Writer);
		VAR
			i, len, nElem: INTEGER;
			v: GraphNodes.SubVector;
	BEGIN
		IF node.index = 0 THEN
			nElem := node.Size();
			GraphNodes.Externalize(node.tau, wr);
			GraphNodes.Externalize(node.gamma, wr);
			wr.WriteReal(node.gammaMax);
			wr.WriteReal(node.gammaMin);
			v := GraphNodes.NewVector();
			v.components := node.mu;
			v.start := node.muStart; v.nElem := nElem; v.step := node.muStep;
			GraphNodes.ExternalizeSubvector(v, wr);
			len := LEN(node.adj);
			wr.WriteInt(len);
			i := 0;
			WHILE i < len DO
				wr.WriteInt(node.adj[i]);
				wr.WriteReal(node.c[i]);
				INC(i)
			END;
			i := 0;
			WHILE i < nElem DO
				wr.WriteInt(node.num[i]);
				wr.WriteInt(node.offset[i]);
				wr.WriteReal(node.m[i]);
				wr.WriteReal(node.eigenvalues[i]);
				INC(i)
			END
		END
	END ExternalizeChain;

	PROCEDURE (node: Node) InitStochastic;
	BEGIN
		node.mu := NIL;
		node.tau := NIL;
		node.gamma := NIL;
		node.m := NIL;
		node.c := NIL;
		node.eigenvalues := NIL
	END InitStochastic;

	PROCEDURE (node: Node) InternalizeChain (VAR rd: Stores.Reader);
		VAR
			i, len, nElem: INTEGER;
			v: GraphNodes.SubVector;
			p: Node;
	BEGIN
		IF node. index = 0 THEN
			nElem := node.Size();
			node.tau := GraphNodes.Internalize(rd);
			node.gamma := GraphNodes.Internalize(rd);
			rd.ReadReal(node.gammaMax);
			rd.ReadReal(node.gammaMin);
			GraphNodes.InternalizeSubvector(v, rd);
			node.mu := v.components;
			node.muStart := v.start;
			node.muStep := v.step;
			rd.ReadInt(len);
			NEW(node.adj, len);
			NEW(node.c, len);
			i := 0;
			WHILE i < len DO
				rd.ReadInt(node.adj[i]);
				rd.ReadReal(node.c[i]);
				INC(i)
			END;
			NEW(node.num, nElem);
			NEW(node.offset, nElem);
			NEW(node.m, nElem);
			NEW(node.eigenvalues, nElem);
			i := 0;
			WHILE i < nElem DO
				rd.ReadInt(node.num[i]);
				rd.ReadInt(node.offset[i]);
				rd.ReadReal(node.m[i]);
				rd.ReadReal(node.eigenvalues[i]);
				INC(i)
			END
		ELSE
			p := node.components[0](Node);
			node.adj := p.adj;
			node.num := p.num;
			node.offset := p.offset;
			node.c := p.c;
			node.m := p.m;
			node.eigenvalues := p.eigenvalues;
			node.mu := p.mu;
			node.tau := p.tau;
			node.gamma := p.gamma;
			node.gammaMax := p.gammaMax;
			node.gammaMin := p.gammaMin;
			node.muStart := p.muStart;
			node.muStep := p.muStep
		END
	END InternalizeChain;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "SpatialCARProper.Install"
	END Install;

	PROCEDURE (likelihood: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		ASSERT(as = GraphRules.gamma, 21); ASSERT(likelihood.index = 0, 21);
		p0 := 0.50 * likelihood.Size();
		p1 := QuadraticForm(likelihood);
		x := likelihood.tau
	END LikelihoodForm;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			start, step, index: INTEGER;
			mean: REAL;
	BEGIN
		index := node.index;
		start := node.muStart;
		step := node.muStep;
		mean := node.mu[start + index * step].Value();
		RETURN mean
	END Location;

	PROCEDURE (node: Node) LogLikelihood (): REAL;
		VAR
			logLikelihood, tau, logTau: REAL;
	BEGIN
		tau := node.tau.Value();
		logTau := MathFunc.Ln(tau);
		logLikelihood := 0.50 * logTau * node.Size() + 0.50 * LogDet(node) - tau * QuadraticForm(node);
		RETURN logLikelihood
	END LogLikelihood;

	PROCEDURE (node: Node) LogMVPrior (): REAL;
		VAR
			logPrior, tau: REAL;
	BEGIN
		tau := node.tau.Value();
		logPrior :=  - tau * QuadraticForm(node);
		RETURN logPrior
	END LogMVPrior;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			x, mu, tau, prior: REAL;
	BEGIN
		x := node.value;
		node.PriorForm(GraphRules.normal, mu, tau);
		prior :=  - 0.50 * tau * (x - mu) * (x - mu);
		RETURN prior
	END LogPrior;

	PROCEDURE (node: Node) MatrixElements (OUT values: ARRAY OF REAL);
		VAR
			diagIndex, i, index, j, nnz, numNeighs, nElem, start: INTEGER;
			gamma, mInverse, tau: REAL;
	BEGIN
		gamma := node.gamma.Value();
		tau := node.tau.Value();
		i := 0;
		nnz := 0;
		nElem := node.Size();
		WHILE i < nElem DO
			mInverse := 1.0 / node.m[i];
			values[nnz] := tau * mInverse;
			INC(nnz);
			start := node.offset[i];
			numNeighs := node.num[i];
			j := 0;
			WHILE j < numNeighs DO
				index := node.adj[start + j];
				IF index > i THEN
					values[nnz] :=  - tau * gamma * mInverse * node.c[start + j]; INC(nnz)
				END;
				INC(j)
			END;
			INC(i)
		END
	END MatrixElements;

	PROCEDURE (node: Node) MatrixInfo (OUT type, nElements: INTEGER);
		VAR
			nElem: INTEGER;
	BEGIN
		type := GraphMRF.sparse;
		nElem := node.Size();
		nElements := LEN(node.c);
		ASSERT(~ODD(nElements), 66);
		nElements := nElem + nElements DIV 2
	END MatrixInfo;

	PROCEDURE (node: Node) MatrixMap (OUT rowInd, colPtr: ARRAY OF INTEGER);
		VAR
			i, index, j, nnz, numNeighs, nElem, start: INTEGER;
	BEGIN
		i := 0;
		nnz := 0;
		nElem := node.Size();
		WHILE i < nElem DO
			colPtr[i] := nnz;
			rowInd[nnz] := i;
			INC(nnz);
			start := node.offset[i];
			numNeighs := node.num[i];
			j := 0;
			WHILE j < numNeighs DO
				index := node.adj[start + j];
				IF index > i THEN
					rowInd[nnz] := index; INC(nnz)
				END;
				INC(j)
			END;
			INC(i)
		END;
	END MatrixMap;

	PROCEDURE (node: Node) MVSample (OUT res: SET);
		VAR
			i, nElements, nElem, type: INTEGER;
			rowInd, colPtr: POINTER TO ARRAY OF INTEGER;
			m: MathSparsematrix.Matrix;
			llt: MathSparsematrix.LLT;
			elements, x: POINTER TO ARRAY OF REAL;
	BEGIN
		nElem := node.Size();
		node.MatrixInfo(type, nElements);
		NEW(colPtr, nElem);
		NEW(rowInd, nElements);
		NEW(elements, nElements);
		node.MatrixMap(rowInd, colPtr);
		node.MatrixElements(elements);
		m := MathSparsematrix.New(nElem, nElements);
		MathSparsematrix.SetMap(m, rowInd, colPtr);
		MathSparsematrix.SetElements(m, elements);
		llt := MathSparsematrix.LLTFactor(m);
		NEW(x, nElem);
		i := 0;
		WHILE i < nElem DO
			x[i] := MathRandnum.StandardNormal();
			INC(i)
		END;
		MathSparsematrix.BackSub(llt, x, nElem);
		i := 0;
		WHILE i < nElem DO
			node.components[i].SetValue(x[i]);
			INC(i)
		END
	END MVSample;

	PROCEDURE (node: Node) Modify (): GraphStochastic.Node;
		VAR
			p: Node;
	BEGIN
		NEW(p);
		p^ := node^;
		p.tau := GraphParamtrans.LogTransform(p.tau);
		p.gamma := GraphParamtrans.IdentTransform(p.gamma);
		RETURN p
	END Modify;

	PROCEDURE (node: Node) NumberConstraints (): INTEGER;
	BEGIN
		RETURN 0
	END NumberConstraints;

	PROCEDURE (node: Node) NumberNeighbours (): INTEGER;
	BEGIN
		RETURN node.num[node.index]
	END NumberNeighbours;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			i, nElem, muStart, muStep: INTEGER;
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		IF (node.index = 0) OR all THEN
			node.tau.AddParent(list);
			node.gamma.AddParent(list);
			i := 0;
			nElem := node.Size();
			muStart := node.muStart;
			muStep := node.muStep;
			WHILE i < nElem DO
				p := node.mu[muStart + i * muStep];
				p.AddParent(list);
				INC(i)
			END
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (prior: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
		VAR
			index, i, j, len, offset, muStart, muStep: INTEGER;
			gamma: REAL;
	BEGIN
		ASSERT(as IN {GraphRules.normal, GraphRules.mVN}, 21);
		index := prior.index;
		muStart := prior.muStart;
		muStep := prior.muStep;
		p0 := prior.mu[muStart + index * muStep].Value();
		IF as = GraphRules.normal THEN
			gamma := prior.gamma.Value();
			p1 := prior.tau.Value() / prior.m[index];
			i := 0;
			len := prior.num[index];
			offset := prior.offset[index];
			WHILE i < len DO
				j := prior.adj[offset + i];
				p0 := p0 + 
				gamma * prior.c[offset + i] * (prior.components[j].value - prior.mu[muStart + j * muStep].Value());
				INC(i)
			END
		ELSE
			p1 := 0.0
		END
	END PriorForm;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			mu, tau: REAL;
	BEGIN
		node.PriorForm(GraphRules.normal, mu, tau);
		node.SetValue(MathRandnum.Normal(mu, tau));
		res := {}
	END Sample;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
		CONST
			eps = 1.0E-20;
		VAR
			index, i, nElem: INTEGER;
			p: Node;
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.vectors[0].components # NIL, 21); ASSERT(args.vectors[0].start >= 0, 21);
			ASSERT(args.vectors[0].nElem > 0, 21);
			ASSERT(args.vectors[1].components # NIL, 21); ASSERT(args.vectors[1].start >= 0, 21);
			ASSERT(args.vectors[1].nElem > 0, 21);
			ASSERT(args.vectors[2].components # NIL, 21); ASSERT(args.vectors[2].start >= 0, 21);
			ASSERT(args.vectors[2].nElem > 0, 21);
			ASSERT(args.vectors[3].components # NIL, 21); ASSERT(args.vectors[3].start >= 0, 21);
			ASSERT(args.vectors[3].nElem > 0, 21);
			ASSERT(args.vectors[4].components # NIL, 21); ASSERT(args.vectors[4].start >= 0, 21);
			ASSERT(args.vectors[4].nElem > 0, 21);
			ASSERT(args.scalars[0] # NIL, 21); node.tau := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21); node.gamma := args.scalars[1];
			nElem := LEN(node.components);
			index := node.index;
			node.mu := args.vectors[0].components;
			node.muStart := args.vectors[0].start;
			node.muStep := args.vectors[0].step;
			IF index = 0 THEN
				NEW(node.c, args.vectors[1].nElem);
				NEW(node.adj, args.vectors[2].nElem);
				NEW(node.num, args.vectors[3].nElem);
				NEW(node.offset, args.vectors[3].nElem);
				NEW(node.m, nElem);
				NEW(node.eigenvalues, nElem);
				i := 0;
				WHILE i < args.vectors[1].nElem DO
					node.c[i] := args.vectors[1].components[i].Value();
					node.adj[i] := SHORT(ENTIER(args.vectors[2].components[i].Value() + eps) - 1);
					INC(i)
				END;
				i := 0;
				WHILE i < nElem DO
					node.num[i] := SHORT(ENTIER(args.vectors[3].components[i].Value() + eps));
					node.m[i] := args.vectors[4].components[i].Value();
					INC(i)
				END;
				node.offset[0] := 0;
				i := 1;
				WHILE i < nElem DO
					node.offset[i] := node.offset[i - 1] + node.num[i - 1];
					INC(i)
				END;
				EigenValues(node.c, node.m, node.adj, node.num, node.eigenvalues);
				node.gammaMax := 1.0 / node.eigenvalues[nElem - 1];
				node.gammaMin := 1.0 / node.eigenvalues[0]
			ELSE
				p := node.components[0](Node);
				node.adj := p.adj;
				node.num := p.num;
				node.offset := p.offset;
				node.c := p.c;
				node.m := p.m;
				node.eigenvalues := p.eigenvalues;
				node.mu := p.mu;
				node.tau := p.tau;
				node.gamma := p.gamma;
				node.gammaMax := p.gammaMax;
				node.gammaMin := p.gammaMin;
				node.muStart := p.muStart;
				node.muStep := p.muStep
			END
		END
	END Set;

	PROCEDURE (prior: Node) ThinLikelihood (first, thin: INTEGER);
	BEGIN
		start := first;
		step := thin
	END ThinLikelihood;

	PROCEDURE (f: Factory) New (): GraphMultivariate.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		start := 0;
		step := 1;
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vvvvvss"
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		CONST
			initSize = 100;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		log2Pi := Math.Ln(2.0 * Math.Pi());
		NEW(f); fact := f;
		NEW(v0, initSize);
		NEW(v1, initSize);
		NEW(v2, initSize);
		start := 0;
		step := 1
	END Init;

BEGIN
	Init
END SpatialCARProper.
