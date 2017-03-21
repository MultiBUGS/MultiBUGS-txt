(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE SpatialMVCAR;

	

	IMPORT
		Math, Stores,
		GraphGMRF, GraphMultivariate, GraphNodes, GraphRules, GraphStochastic,
		MathMatrix, MathRandnum, MathSparsematrix;

	TYPE
		Node = POINTER TO RECORD(GraphGMRF.Node)
			neighs: POINTER TO ARRAY OF INTEGER;
			weights: POINTER TO ARRAY OF REAL;
			tau: GraphNodes.Vector;
			dim, numIslands, tauStart, tauStep: INTEGER
		END;

		Factory = POINTER TO RECORD (GraphMultivariate.Factory) END;

	CONST
		halt = 126; trap = 55;

	VAR
		fact-: GraphMultivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		mean: POINTER TO ARRAY OF REAL;
		prec, tau: POINTER TO ARRAY OF ARRAY OF REAL;

	PROCEDURE MarkNeighs (node: Node);
		VAR
			i, numNeigh: INTEGER;
			car: Node;
	BEGIN
		IF GraphNodes.mark IN node.props THEN RETURN END;
		node.SetProps(node.props + {GraphNodes.mark});
		numNeigh := LEN(node.neighs);
		i := 0;
		WHILE i < numNeigh DO
			car := node.components[node.neighs[i]](Node);
			IF ~(GraphNodes.mark IN car.props) THEN
				MarkNeighs(car)
			END;
			INC(i)
		END
	END MarkNeighs;

	PROCEDURE NumIslands (node: Node): INTEGER;
		VAR
			dim, i, numAreas, nElem, numIslands: INTEGER;
			new: BOOLEAN;
			car: Node;
	BEGIN
		numIslands := 0;
		nElem := node.Size();
		dim := node.dim;
		numAreas := nElem DIV dim;
		i := 0;
		REPEAT
			new := FALSE;
			WHILE (i < numAreas) & (GraphNodes.mark IN node.components[i].props) DO
				INC(i)
			END;
			IF i < numAreas THEN
				new := TRUE;
				INC(numIslands);
				car := node.components[i](Node);
				MarkNeighs(car)
			END;
			INC(i)
		UNTIL ~new;
		i := 0;
		WHILE i < numAreas DO
			car := node.components[i](Node);
			car.SetProps(car.props - {GraphNodes.mark});
			INC(i)
		END;
		RETURN numIslands
	END NumIslands;

	PROCEDURE MVLikelihoodForm (node: Node; as: INTEGER; OUT x: GraphNodes.Vector;
	OUT start, step: INTEGER; OUT p0: ARRAY OF REAL; OUT p1: ARRAY OF ARRAY OF REAL);
		VAR
			dim, i, j, k, l, nElem, numAreas, numNeigh: INTEGER;
			factor0, factor1, weight: REAL;
			area: Node;
	BEGIN
		ASSERT(as = GraphRules.wishart, trap);
		nElem := node.Size();
		dim := node.dim;
		numAreas := nElem DIV dim;
		p0[0] := numAreas - node.numIslands;
		x := node.tau;
		start := node.tauStart;
		step := node.tauStep;
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				p1[i, j] := 0;
				k := 0;
				WHILE k < numAreas DO
					area := node.components[k](Node);
					numNeigh := LEN(area.neighs);
					l := 0;
					WHILE l < numNeigh DO
						weight := area.weights[l];
						factor0 := weight * (node.components[i * numAreas + k].value
						 - node.components[i * numAreas + area.neighs[l]].value);
						factor1 := weight * (node.components[j * numAreas + k].value
						 - node.components[j * numAreas + area.neighs[l]].value);
						p1[i, j] := p1[i, j] + 0.5 * factor0 * factor1;
						INC(l)
					END;
					INC(k)
				END;
				INC(j)
			END;
			INC(i)
		END
	END MVLikelihoodForm;

	PROCEDURE (node: Node) Bounds (OUT lower, upper: REAL);
	BEGIN
		lower :=  - INF;
		upper := INF
	END Bounds;

	PROCEDURE (node: Node) Check (): SET;
		CONST
			eps = 1.0E-10;
		VAR
			i, j, len0, len1, n, nElem, numAreas, numConstraints, thisArea: INTEGER;
			res: SET;
			sum, weight: REAL;
			neigh: Node;
			constraints: POINTER TO ARRAY OF ARRAY OF REAL;
	BEGIN
		res := {};
		ASSERT(LEN(node.tau) = node.dim * node.dim, trap);
		IF node.index #  - 1 THEN (*	not special case of singleton islands	*)
			nElem := node.Size();
			numAreas := nElem DIV node.dim;
			thisArea := node.index MOD numAreas;
			IF node.neighs # NIL THEN len0 := LEN(node.neighs) ELSE len0 := 0 END;
			i := 0;
			WHILE i < len0 DO
				weight := node.weights[i];
				n := node.neighs[i]; ASSERT(n < numAreas, trap);
				neigh := node.components[n](Node);
				IF neigh.neighs # NIL THEN len1 := LEN(neigh.neighs) ELSE len1 := 0 END;
				j := 0; WHILE (j < len1) & (neigh.neighs[j] # thisArea) DO INC(j) END;
				IF j = len1 THEN
					RETURN {GraphNodes.arg1, GraphNodes.notSymmetric}
				END;
				IF ABS(neigh.weights[j] - weight) > eps THEN
					RETURN {GraphNodes.arg3, GraphNodes.notSymmetric}
				END;
				INC(i)
			END;
			IF node = node.components[0] THEN
				numConstraints := node.NumberConstraints();
				NEW(constraints, numConstraints, nElem);
				node.Constraints(constraints);
				i := 0;
				WHILE i < numConstraints DO
					j := 0;
					sum := 0.0;
					WHILE j < nElem DO
						sum := sum + node.components[j].value * constraints[i, j];
						INC(j)
					END;
					IF ABS(sum) > eps THEN
						RETURN {GraphNodes.lhs, GraphNodes.invalidValue}
					END;
					INC(i)
				END
			END
		END;
		RETURN res
	END Check;

	PROCEDURE (node: Node) ClassifyLikelihood (parent: GraphStochastic.Node): INTEGER;
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
			i := 0;
			class := GraphRules.unif;
			WHILE i < node.dim * node.dim DO
				tau := node.tau[tauStart + i * tauStep];
				f := GraphStochastic.ClassFunction(tau, parent);
				IF f # GraphRules.const THEN
					class := GraphRules.general
				END;
				INC(i)
			END;
		END;
		RETURN class
	END ClassifyLikelihood;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.normal
	END ClassifyPrior;

	PROCEDURE (node: Node) Constraints (OUT constraints: ARRAY OF ARRAY OF REAL);
		VAR
			dim, end, i, j, numAreas, size: INTEGER;
	BEGIN
		dim := node.dim;
		size := node.Size();
		numAreas := size DIV dim;
		i := 0;
		WHILE i < node.dim DO
			j := 0;
			WHILE j < size DO constraints[i, j] := 0.0; INC(j) END;
			j := i * numAreas;
			end := j + numAreas;
			WHILE j < end DO constraints[i, j] := 1.0; INC(j) END;
			INC(i)
		END
	END Constraints;

	PROCEDURE (node: Node) Deviance (): REAL;
	BEGIN
		HALT(126);
		RETURN 0
	END Deviance;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeChain (VAR wr: Stores.Writer);
		VAR
			dim, j, nElem, numNeigh: INTEGER;
			v: GraphNodes.SubVector;
	BEGIN
		IF node.index = 0 THEN
			dim := node.dim;
			nElem := node.Size();
			wr.WriteInt(node.dim);
			wr.WriteInt(node.numIslands);
			v := GraphNodes.NewVector();
			v.components := node.tau;
			v.start := node.tauStart; v.nElem := dim * dim; v.step := node.tauStep;
			GraphNodes.ExternalizeSubvector(v, wr);
		END;
		IF (node.index #  - 1) & (node.index MOD node.dim = 0) THEN
			numNeigh := LEN(node.neighs);
			wr.WriteInt(numNeigh);
			j := 0;
			WHILE j < numNeigh DO
				wr.WriteInt(node.neighs[j]);
				wr.WriteReal(node.weights[j]);
				INC(j)
			END;
		END
	END ExternalizeChain;

	PROCEDURE (node: Node) InitStochastic;
	BEGIN
		node.dim := 0;
		node.numIslands := 0;
		node.neighs := NIL;
		node.weights := NIL;
		node.tau := NIL;
		node.tauStart :=  - 1;
		node.tauStep := 0;
		node.SetProps(node.props + {GraphStochastic.noMean})
	END InitStochastic;

	PROCEDURE (node: Node) InternalizeChain (VAR rd: Stores.Reader);
		VAR
			dim, i, j, numAreas, numNeigh, nElem: INTEGER;
			p, r: Node;
			v: GraphNodes.SubVector;
	BEGIN
		IF node.index = 0 THEN
			rd.ReadInt(dim);
			node.dim := dim;
			IF (mean = NIL) OR (dim > LEN(mean)) THEN
				NEW(mean, dim);
				NEW(prec, dim, dim);
				NEW(tau, dim, dim)
			END;
			rd.ReadInt(node.numIslands);
			GraphNodes.InternalizeSubvector(v, rd);
			node.tau := v.components;
			node.tauStart := v.start;
			node.tauStep := v.step;
		END;
		IF node.index =  - 1 THEN
			node.Init;
			node.SetComponent(NIL,  - 1);
			node.SetProps({GraphStochastic.nR});
			node.SetValue(0.0)
		END;
		IF node.index #  - 1 THEN
			p := node.components[0](Node);
			node.dim := p.dim;
			node.numIslands := p.numIslands;
			node.tauStart := p.tauStart;
			node.tauStep := node.tauStep;
			node.tau := p.tau;
			nElem := node.Size();
			numAreas := nElem DIV dim;
			i := 0;
			WHILE i < numAreas DO
				p := node.components[i](Node);
				rd.ReadInt(numNeigh);
				NEW(p.neighs, numNeigh);
				NEW(p.weights, numNeigh);
				j := 0;
				WHILE j < numNeigh DO
					rd.ReadInt(p.neighs[j]);
					rd.ReadReal(p.weights[j]);
					INC(j)
				END;
				(*	replicate the neighbourhood structure	*)
				j := 1;
				WHILE j < dim DO
					r := node.components[i + j * numAreas](Node);
					r.neighs := p.neighs;
					r.weights := p.weights;
					INC(j)
				END;
				INC(i)
			END;
		END
	END InternalizeChain;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "SpatialMVCAR.Install"
	END Install;

	(*	use this to get parameters of Wishart	*)
	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node; OUT p0, p1: REAL);
		VAR
			p0Vec: ARRAY 1 OF REAL;
			xVec: GraphNodes.Vector;
			tauStart, tauStep, i, j, size: INTEGER;
		CONST
			eps = 1.0E-6;
	BEGIN
		ASSERT(as = GraphRules.wishart, 21);
		WITH x: GraphMultivariate.Node DO
			IF x.index = 0 THEN
				MVLikelihoodForm(node, as, xVec, tauStart, tauStep, p0Vec, prec);
				ASSERT(x = xVec[0], 66);
				p0 := p0Vec[0];
				p1 := prec[0, 0]
			ELSE
				size := x.Size();
				size := SHORT(ENTIER(Math.Sqrt(size + eps)));
				i := node.index DIV size;
				j := node.index MOD size;
				p0 :=  - 1;
				p1 := prec[i, j]
			END
		END
	END LikelihoodForm;

	PROCEDURE (node: Node) LogLikelihood (): REAL;
		VAR
			as, dim, i, j, tauStart, tauStep: INTEGER;
			x: GraphNodes.Vector;
			p0: ARRAY 1 OF REAL;
			trace, likelihood, k: REAL;
	BEGIN
		as := GraphRules.wishart;
		MVLikelihoodForm(node, as, x, tauStart, tauStep, p0, prec);
		k := p0[0];
		dim := node.dim;
		i := 0;
		trace := 0.0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				tau[j, i] := node.tau[tauStart + (i + j * dim) * tauStep].Value();
				trace := trace + prec[i, j] * tau[j, i];
				INC(j)
			END;
			INC(i)
		END;
		likelihood := 0.5 * (k - dim - 1) * MathMatrix.LogDet(tau, dim) + trace;
		RETURN likelihood
	END LogLikelihood;

	PROCEDURE (node: Node) Location (): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END Location;

	PROCEDURE (node: Node) LogMVPrior (): REAL;
		VAR
			as, dim, i, j, tauStart, tauStep: INTEGER;
			x: GraphNodes.Vector;
			p0: ARRAY 1 OF REAL;
			trace: REAL;
	BEGIN
		as := GraphRules.wishart;
		MVLikelihoodForm(node, as, x, tauStart, tauStep, p0, prec);
		dim := node.dim;
		i := 0;
		trace := 0.0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				trace := trace + prec[i, j] * node.tau[tauStart + (i + j * dim) * tauStep].Value();
				INC(j)
			END;
			INC(i)
		END;
		RETURN - 0.5 * trace
	END LogMVPrior;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			logPrior, mu, tau, x: REAL;
	BEGIN
		node.PriorForm(GraphRules.normal, mu, tau);
		x := node.value;
		logPrior :=  - 0.5 * tau * (x - mu) * (x - mu);
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) MatrixElements (OUT values: ARRAY OF REAL);
		VAR
			blockI, diagIndex, dim, i, j, k, label, nElem, nnz, nRegion, numNeighs, shift, tauStart, tauStep: INTEGER;
			sumWeights, tau: REAL;
			p: Node;
	BEGIN
		i := 0;
		nElem := node.Size();
		dim := node.dim;
		nRegion := nElem DIV dim;
		tauStart := node.tauStart;
		tauStep := node.tauStep;
		nnz := 0;
		WHILE i < nElem DO
			blockI := i DIV nRegion;
			shift := blockI * nRegion;
			p := node.components[i](Node);
			numNeighs := LEN(p.neighs);
			(*	the diagonal block, only keep elements one side of diagonal	*)
			tau := node.tau[tauStart + (blockI + dim * blockI) * tauStep].Value();
			diagIndex := nnz; INC(nnz);
			j := 0;
			sumWeights := 0;
			WHILE j < numNeighs DO
				label := p.neighs[j] + shift;
				IF label > i THEN values[nnz] :=  - p.weights[j] * tau; INC(nnz) END;
				sumWeights := sumWeights + p.weights[j];
				INC(j)
			END;
			values[diagIndex] := sumWeights * tau;
			(*	the off diagonal blocks in upper triangle, all elements of these blocks	*)
			j := blockI + 1;
			WHILE j < dim DO
				tau := node.tau[tauStart + (blockI * dim + j) * tauStep].Value();
				values[nnz] := sumWeights * tau;
				INC(nnz);
				k := 0;
				WHILE k < numNeighs DO
					values[nnz] :=  - p.weights[k] * tau;
					INC(nnz);
					INC(k)
				END;
				INC(j)
			END;
			INC(i)
		END
	END MatrixElements;

	PROCEDURE (node: Node) MatrixInfo (OUT type, nElements: INTEGER);
		VAR
			dim, i, nElem, numAreas, sizeDiagBlock, sizeNonDiagBlock: INTEGER;
	BEGIN
		type := GraphGMRF.sparse;
		nElem := node.Size();
		dim := node.dim;
		numAreas := nElem DIV dim;
		sizeNonDiagBlock := 0;
		i := 0;
		WHILE i < numAreas DO
			sizeNonDiagBlock := sizeNonDiagBlock + LEN(node.components[i](Node).neighs);
			INC(i)
		END;
		sizeDiagBlock := numAreas + sizeNonDiagBlock DIV 2;
		sizeNonDiagBlock := sizeNonDiagBlock + numAreas;
		nElements := dim * sizeDiagBlock + ((dim * (dim - 1)) DIV 2) * sizeNonDiagBlock
	END MatrixInfo;

	PROCEDURE (node: Node) MatrixMap (OUT rowInd, colPtr: ARRAY OF INTEGER);
		VAR
			blockI, dim, i, j, k, label, nElem, nRegion, nnz, numNeighs, shift: INTEGER;
			p: Node;
	BEGIN
		i := 0;
		nElem := node.Size();
		dim := node.dim;
		nRegion := nElem DIV dim;
		nnz := 0;
		WHILE i < nElem DO
			colPtr[i] := nnz;
			blockI := i DIV nRegion;
			shift := blockI * nRegion;
			p := node.components[i](Node);
			numNeighs := LEN(p.neighs);
			(*	the diagonal block, only keep elements one side of diagonal	*)
			rowInd[nnz] := i; INC(nnz);
			j := 0;
			WHILE j < numNeighs DO
				label := p.neighs[j] + shift;
				ASSERT(label < nElem, 77);
				IF label > i THEN
					rowInd[nnz] := label; INC(nnz)
				END;
				INC(j)
			END;
			(*	the off diagonal blocks in upper triangle, all elements of these blocks	*)
			j := blockI + 1;
			WHILE j < dim DO
				shift := j * nRegion;
				label := (i MOD nRegion) + shift;
				ASSERT(label < nElem, 78);
				rowInd[nnz] := label; INC(nnz);
				k := 0;
				WHILE k < numNeighs DO
					label := p.neighs[k] + shift;
					ASSERT(label < nElem, 70);
					rowInd[nnz] := label; INC(nnz);
					INC(k)
				END;
				INC(j)
			END;
			INC(i)
		END
	END MatrixMap;

	PROCEDURE (node: Node) MVSample (OUT res: SET);
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
			node.components[i].SetValue(x[i]); INC(i)
		END;
	END MVSample;

	PROCEDURE (node: Node) NumberConstraints (): INTEGER;
	BEGIN
		RETURN node.dim
	END NumberConstraints;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			dim, i, tauStart, tauStep: INTEGER;
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		IF (node.index = 0) OR (all & ~(GraphStochastic.nR IN node.props)) THEN
			dim := node.dim;
			tauStart := node.tauStart;
			tauStep := node.tauStep;
			i := 0;
			WHILE i < dim * dim DO
				p := node.tau[tauStart + i * tauStep];
				p.AddParent(list);
				INC(i)
			END
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
		VAR
			area, dim, i, j, neigh, nElem, numAreas, numNeigh, row, tauStart, tauStep: INTEGER;
			mu, prec, x, weight, wSum: REAL;
	BEGIN
		ASSERT(as IN {GraphRules.normal, GraphRules.mVN}, trap);
		IF as = GraphRules.normal THEN
			nElem := node.Size();
			dim := node.dim;
			numAreas := nElem DIV dim;
			area := node.index MOD numAreas;
			row := node.index DIV numAreas;
			ASSERT(node.neighs # NIL, trap);
			numNeigh := LEN(node.neighs);
			i := 0;
			WHILE i < dim DO
				mean[i] := 0.0;
				INC(i)
			END;
			wSum := 0.0;
			j := 0;
			WHILE j < numNeigh DO
				neigh := node.neighs[j];
				weight := node.weights[j];
				wSum := wSum + weight;
				i := 0;
				WHILE i < node.dim DO
					mean[i] := mean[i] + weight * node.components[i * numAreas + neigh].value;
					INC(i)
				END;
				INC(j)
			END;
			i := 0;
			WHILE i < node.dim DO
				mean[i] := mean[i] / wSum;
				INC(i)
			END;
			tauStart := node.tauStart;
			tauStep := node.tauStep;
			p1 := wSum * node.tau[tauStart + (row * dim + row) * tauStep].Value();
			p0 :=  - p1 * mean[row];
			i := 0;
			WHILE i < node.dim DO
				IF i # row THEN
					mu := mean[i];
					x := node.components[i * numAreas + area].value;
					prec := wSum * node.tau[tauStart + (i * dim + row) * tauStep].Value();
					p0 := p0 + (x - mu) * prec
				END;
				INC(i)
			END;
			p0 :=  - p0 / p1
		ELSE
			p0 := 0.0;
			p1 := 0.0
		END
	END PriorForm;

	PROCEDURE (node: Node) Sample (OUT res: SET);
	BEGIN
		HALT(halt)
	END Sample;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
		CONST
			eps = 1.0E-10;
		VAR
			beg, dim, i, index, j, nElem, num,
			numAreas, numNeigh, numIslands, off, start, start0, start1, start2, step: INTEGER;
			x: REAL;
			p: GraphNodes.Node;
			car, rep: Node;
			newCom, oldCom: GraphStochastic.Vector;
	BEGIN
		res := {};
		index := node.index;
		nElem := node.Size();
		oldCom := node.components;
		WITH args: GraphStochastic.Args DO
			ASSERT(args.vectors[0].components # NIL, trap); ASSERT(args.vectors[0].start >= 0, trap);
			ASSERT(args.vectors[0].nElem > 0, trap); ASSERT(args.vectors[0].step = 1, trap);
			ASSERT(args.vectors[1].components # NIL, trap); ASSERT(args.vectors[1].start >= 0, trap);
			ASSERT(args.vectors[1].nElem > 0, trap); ASSERT(args.vectors[1].step = 1, trap);
			ASSERT(args.vectors[2].components # NIL, trap); ASSERT(args.vectors[2].start >= 0, trap);
			ASSERT(args.vectors[2].nElem > 0, trap); ASSERT(args.vectors[2].step > 0, trap);
			ASSERT(args.vectors[3].components # NIL, trap); ASSERT(args.vectors[3].start >= 0, trap);
			ASSERT(args.vectors[3].nElem > 0, trap); ASSERT(args.vectors[3].step = 1, trap);
			IF args.vectors[0].nElem # args.vectors[1].nElem THEN
				res := {GraphNodes.arg2, GraphNodes.length}; RETURN
			END;
			dim := SHORT(ENTIER(Math.Sqrt(args.vectors[3].nElem) + eps));
			node.dim := dim;
			numAreas := args.vectors[2].nElem;
			IF nElem # dim * numAreas THEN
				res := {GraphNodes.length, GraphNodes.arg3};
				RETURN
				(*	dimension mismatch for vector argument	*)
			END;
			start2 := args.vectors[2].start;
			off := (start2 + index) MOD numAreas;
			p := args.vectors[2].components[off];
			IF p = NIL THEN
				res := {GraphNodes.arg3, GraphNodes.nil}; RETURN
			END;
			numNeigh := SHORT(ENTIER(p.Value() + eps));
			IF numNeigh > 0 THEN
				node.tau := args.vectors[3].components;
				node.tauStart := args.vectors[3].start;
				node.tauStep := args.vectors[3].step;
				i := 0;
				start := node.tauStart;
				step := node.tauStep;
				WHILE i < dim * dim DO
					p := node.tau[start + i * step];
					IF p = NIL THEN
						res := {GraphNodes.arg4, GraphNodes.nil}; RETURN
					END;
					INC(i)
				END;
				IF (mean = NIL) OR (LEN(mean) < dim) THEN
					NEW(mean, dim);
					NEW(prec, dim, dim);
					NEW(tau, dim, dim)
				END;
				IF index < numAreas THEN (*	first component of MVCAR	*)
					NEW(node.neighs, numNeigh);
					NEW(node.weights, numNeigh);
					i := 0;
					beg := 0;
					WHILE i < index DO
						car := node.components[i](Node);
						IF car.neighs # NIL THEN INC(beg, LEN(car.neighs)) END;
						INC(i)
					END;
					i := 0;
					start0 := args.vectors[0].start;
					start1 := args.vectors[1].start;
					WHILE i < numNeigh DO
						off := start0 + beg + i;
						p := args.vectors[0].components[off];
						IF p = NIL THEN
							res := {GraphNodes.arg1, GraphNodes.nil}; RETURN
						END;
						IF ~(GraphNodes.data IN p.props) THEN
							res := {GraphNodes.arg1, GraphNodes.notData}; RETURN
						END;
						x := p.Value();
						IF ABS(x - SHORT(ENTIER(x + eps))) > eps THEN
							res := {GraphNodes.arg1, GraphNodes.integer}; RETURN
						END;
						off := SHORT(ENTIER(x + eps)) - 1;
						IF off > numAreas THEN
							res := {GraphNodes.arg1, GraphNodes.length}; RETURN
						END;
						node.neighs[i] := off;
						off := start1 + beg + i;
						p := args.vectors[1].components[off];
						IF p = NIL THEN
							res := {GraphNodes.arg2, GraphNodes.nil}; RETURN
						END;
						IF ~(GraphNodes.data IN p.props) THEN
							res := {GraphNodes.arg2, GraphNodes.notData}; RETURN
						END;
						node.weights[i] := p.Value();
						INC(i)
					END
				ELSE
					rep := node.components[node.index MOD numAreas](Node);
					node.neighs := rep.neighs;
					node.weights := rep.weights
				END
			ELSE
				node.Init;
				node.SetComponent(NIL,  - 1);
				node.SetProps({GraphStochastic.nR, GraphStochastic.initialized});
				node.SetValue(0.0)
			END;
			IF index = nElem - 1 THEN
				i := 0; (*	count number of non singleton regions	*)
				num := 0;
				WHILE i < nElem DO
					car := oldCom[i](Node);
					IF car.neighs # NIL THEN INC(num) END;
					INC(i)
				END;
				NEW(newCom, num); (*	remove singleton regions and relabel list of neighbours	*)
				i := 0;
				num := 0;
				WHILE i < nElem DO
					car := oldCom[i](Node);
					IF car.neighs # NIL THEN
						newCom[num] := car;
						car.SetComponent(newCom, num);
						INC(num)
					END;
					INC(i)
				END;
				i := 0;
				WHILE i < num DO
					car := newCom[i](Node);
					j := 0;
					numNeigh := LEN(car.neighs);
					j := 0;
					WHILE j < numNeigh DO
						car.neighs[j] := oldCom[car.neighs[j]](Node).index;
						INC(j)
					END;
					INC(i)
				END;
				(*	count number of islands always at least 1 the whole map	*)
				car := newCom[0](Node);
				numIslands := NumIslands(car);
				i := 0;
				WHILE i < num DO
					car := newCom[i](Node);
					car.numIslands := numIslands;
					car.SetProps(car.props - {GraphNodes.mark});
					INC(i)
				END
			END
		END
	END Set;

	PROCEDURE (node: Node) Modify (): GraphStochastic.Node;
		VAR
			p: Node;
	BEGIN
		NEW(p);
		p^ := node^;
		RETURN p
	END Modify;

	PROCEDURE (f: Factory) New (): GraphMultivariate.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vvvv"
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "D.J.Lunn"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory;
		CONST
			size = 10;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f;
		mean := NIL;
		prec := NIL
	END Init;

BEGIN
	Init
END SpatialMVCAR.
