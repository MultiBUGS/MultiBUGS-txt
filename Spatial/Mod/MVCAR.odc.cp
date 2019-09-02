(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE SpatialMVCAR;

	

	IMPORT
		Stores := Stores64,
		GraphMRF, GraphMVGMRF, GraphMultivariate, GraphNodes, GraphRules, GraphStochastic;

	TYPE
		Node = POINTER TO RECORD(GraphMVGMRF.Node)
			neighs: POINTER TO ARRAY OF INTEGER;
			weights: POINTER TO ARRAY OF REAL;
			numIslands: INTEGER
		END;

		Factory = POINTER TO RECORD (GraphMultivariate.Factory) END;

	VAR
		fact-: GraphMultivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		mean, value: POINTER TO ARRAY OF REAL;
		tau: POINTER TO ARRAY OF ARRAY OF REAL;

	PROCEDURE CalculateTau (node: Node);
		VAR
			i, j, dim: INTEGER;
	BEGIN
		dim := node.dim;
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				tau[i, j] := node.tau[i * dim + j].value;
				INC(j)
			END;
			INC(i)
		END
	END CalculateTau;

	PROCEDURE MarkNeighs (node: Node);
		VAR
			dim, i, numNeigh: INTEGER;
			car: Node;
	BEGIN
		IF GraphNodes.mark IN node.props THEN RETURN END;
		dim := node.dim;
		INCL(node.props, GraphNodes.mark);
		numNeigh := LEN(node.neighs);
		i := 0;
		WHILE i < numNeigh DO
			car := node.components[node.neighs[i] * dim](Node);
			IF ~(GraphNodes.mark IN car.props) THEN
				MarkNeighs(car)
			END;
			INC(i)
		END
	END MarkNeighs;

	PROCEDURE NumIslands (node: Node): INTEGER;
		VAR
			dim, i, nElem, numIslands: INTEGER;
			new: BOOLEAN;
			car: Node;
	BEGIN
		numIslands := 0;
		nElem := node.Size();
		dim := node.dim;
		i := 0;
		REPEAT
			new := FALSE;
			WHILE (i < nElem) & (GraphNodes.mark IN node.components[i].props) DO
				INC(i, dim)
			END;
			IF i < nElem THEN
				new := TRUE;
				INC(numIslands);
				car := node.components[i](Node);
				MarkNeighs(car)
			END;
			INC(i, dim)
		UNTIL ~new;
		i := 0;
		WHILE i < nElem DO
			car := node.components[i](Node);
			EXCL(car.props, GraphNodes.mark);
			INC(i, dim)
		END;
		RETURN numIslands
	END NumIslands;

	PROCEDURE (node: Node) MVLikelihoodForm (as: INTEGER; OUT x: GraphNodes.Vector;
	OUT start, step: INTEGER; OUT p0: ARRAY OF REAL; OUT p1: ARRAY OF ARRAY OF REAL);
		VAR
			dim, first, i, j, k, nElem, numAreas, numNeigh: INTEGER;
			mu, sumWeight: REAL;
	BEGIN
		ASSERT(as = GraphRules.wishart, 21);
		nElem := node.Size();
		dim := node.dim;
		numAreas := nElem DIV dim;
		p0[0] := (numAreas - node.numIslands) / numAreas;
		x := node.tau;
		start := node.tauStart;
		step := node.tauStep;
		first := (node.index DIV dim) * dim;
		numNeigh := LEN(node.neighs);
		sumWeight := 0.0;
		i := 0; WHILE i < numNeigh DO sumWeight := sumWeight + node.weights[i]; INC(i) END;
		i := 0;
		WHILE i < dim DO
			mu := 0.0;
			value[i] := node.components[first + i].value;
			j := 0;
			WHILE j < numNeigh DO
				k := node.components[node.neighs[j] * dim + i](Node).index;
				mu := mu + node.components[k].value * node.weights[j];
				INC(j)
			END;
			mean[i] := mu / sumWeight;
			INC(i);
		END;
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				p1[i, j] := value[i] * (value[j] - mean[j]);
				INC(j)
			END;
			INC(i)
		END
	END MVLikelihoodForm;

	PROCEDURE (node: Node) MVPriorForm (OUT p0: ARRAY OF REAL;
	OUT p1: ARRAY OF ARRAY OF REAL);
		VAR
			dim, i, j, k, numNeigh: INTEGER;
			mu, sumWeight: REAL;
	BEGIN
		dim := node.dim;
		numNeigh := LEN(node.neighs);
		sumWeight := 0.0;
		i := 0; WHILE i < numNeigh DO sumWeight := sumWeight + node.weights[i]; INC(i) END;
		i := 0;
		WHILE i < dim DO
			mu := 0.0;
			j := 0;
			WHILE j < numNeigh DO
				k := node.components[node.neighs[j] * dim + i](Node).index;
				mu := mu + node.components[k].value * node.weights[j];
				INC(j)
			END;
			p0[i] := mu / sumWeight;
			INC(i);
		END;
		CalculateTau(node);
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				p1[i, j] := tau[i, j] / sumWeight;
				INC(j)
			END;
			INC(i)
		END
	END MVPriorForm;

	PROCEDURE (node: Node) CheckMVGMRF (): SET;
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
		IF node.index # - 1 THEN (*	not special case of singleton islands	*)
			nElem := node.Size();
			numAreas := nElem DIV node.dim;
			thisArea := node.index MOD numAreas;
			IF node.neighs # NIL THEN len0 := LEN(node.neighs) ELSE len0 := 0 END;
			i := 0;
			WHILE i < len0 DO
				weight := node.weights[i];
				n := node.neighs[i]; ASSERT(n < numAreas, 45);
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
	END CheckMVGMRF;

	PROCEDURE (node: Node) ExternalizeMVGMRF (VAR wr: Stores.Writer);
		VAR
			dim, j, numNeigh: INTEGER;
	BEGIN
		dim := node.dim;
		IF node.index = 0 THEN
			wr.WriteInt(node.numIslands);
		END;
		IF (node.index # - 1) & (node.index MOD dim = 0) THEN
			numNeigh := LEN(node.neighs);
			wr.WriteInt(numNeigh);
			j := 0;
			WHILE j < numNeigh DO
				wr.WriteInt(node.neighs[j]);
				wr.WriteReal(node.weights[j]);
				INC(j)
			END
		END
	END ExternalizeMVGMRF;

	PROCEDURE (node: Node) InitMVGMRF;
	BEGIN
		node.numIslands := 0;
		node.neighs := NIL;
		node.weights := NIL;
	END InitMVGMRF;

	PROCEDURE (node: Node) InternalizeMVGMRF (VAR rd: Stores.Reader);
		VAR
			dim, i, j, index, numNeigh, nElem: INTEGER;
			p: Node;
	BEGIN
		index := node.index;
		IF index = 0 THEN
			rd.ReadInt(node.numIslands);
			nElem := node.Size();
			i := 0;
			WHILE i < nElem DO
				p := node.components[i](Node);
				p.numIslands := node.numIslands;
				INC(i)
			END
		END;
		IF index # - 1 THEN
			dim := node.dim;
			IF index MOD dim = 0 THEN
				rd.ReadInt(numNeigh);
				NEW(node.neighs, numNeigh);
				NEW(node.weights, numNeigh);
				j := 0;
				WHILE j < numNeigh DO
					rd.ReadInt(node.neighs[j]);
					rd.ReadReal(node.weights[j]);
					INC(j)
				END;
				rd.ReadInt(node.numIslands)
			ELSE
				p := node.components[(index DIV dim) * dim](Node);
				node.neighs := p.neighs;
				node.weights := p.weights;
				node.numIslands := p.numIslands
			END
		ELSE
			node.Init;
			node.SetComponent(NIL, - 1);
			node.props := {GraphStochastic.hidden};
			node.value := 0.0
		END
	END InternalizeMVGMRF;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "SpatialMVCAR.Install"
	END Install;

	PROCEDURE (node: Node) MatrixElements (OUT values: ARRAY OF REAL);
		VAR
			diagIndex, i, j, k, l, dim, region, region1, n, nnz, numNeighs: INTEGER;
			sumWeights: REAL;
			p, q: Node;
	BEGIN
		CalculateTau(node);
		n := node.Size();
		dim := node.dim;
		i := 0;
		nnz := 0;
		WHILE i < n DO
			p := node.components[i](Node);
			numNeighs := LEN(p.neighs);
			region := p.index DIV dim;
			j := 0;
			WHILE j < dim DO
				diagIndex := nnz; INC(nnz, dim - j); (*	leave space for diagonal entires	*)
				sumWeights := 0;
				k := 0;
				WHILE k < numNeighs DO
					q := node.components[p.neighs[j] * dim](Node);
					region1 := q.index DIV dim;
					IF region1 > region THEN
						l := 0;
						WHILE l < dim DO
							values[nnz] := - p.weights[k] * tau[j, l]; INC(nnz);
							INC(l)
						END;
					END;
					sumWeights := sumWeights + p.weights[k];
					INC(k)
				END;
				(*	now write out row of the upper half of diagonal block	*)
				k := j;
				WHILE k < dim DO
					values[diagIndex] := sumWeights * tau[j, k]; INC(diagIndex);
					INC(k)
				END;
				INC(j)
			END;
			INC(i, dim)
		END
	END MatrixElements;

	PROCEDURE (node: Node) MatrixInfo (OUT type, nElements: INTEGER);
		VAR
			dim, i, nElem, numNeighs, numDiagBlock, numNonDiagBlock: INTEGER;
			p: Node;
	BEGIN
		type := GraphMRF.sparse;
		dim := node.dim;
		nElem := node.Size();
		numDiagBlock := nElem DIV dim;
		i := 0;
		numNonDiagBlock := 0;
		WHILE i < numDiagBlock DO
			p := node.components[i * dim](Node);
			numNeighs := p.NumberNeighbours();
			INC(numNonDiagBlock, numNeighs);
			INC(i)
		END;
		ASSERT(~ODD(numNonDiagBlock), 66);
		numNonDiagBlock := numNonDiagBlock DIV 2;
		nElements := (numDiagBlock * dim * (dim + 1)) DIV 2 + numNonDiagBlock * dim * dim
	END MatrixInfo;

	PROCEDURE (node: Node) MatrixMap (OUT rowInd, colPtr: ARRAY OF INTEGER);
		VAR
			col, diagIndex, i, j, k, l, dim, region, region1, nElem, nnz, numNeighs: INTEGER;
			p, q: Node;
	BEGIN
		nElem := node.Size();
		dim := node.dim;
		i := 0;
		nnz := 0;
		col := 0;
		WHILE i < nElem DO
			p := node.components[i](Node);
			numNeighs := LEN(p.neighs);
			region := p.index DIV dim;
			j := 0;
			WHILE j < dim DO
				diagIndex := nnz; colPtr[col] := diagIndex; INC(col); INC(nnz, dim - j);
				k := 0;
				WHILE k < numNeighs DO
					q := node.components[p.neighs[j] * dim](Node);
					region1 := q.index DIV dim;
					IF region1 > region THEN
						l := 0;
						WHILE l < dim DO
							rowInd[nnz] := region1 * dim + l; INC(nnz);
							INC(l)
						END;
					END;
					INC(k)
				END;
				k := j;
				WHILE k < dim DO
					rowInd[diagIndex] := region * dim + k; INC(diagIndex);
					INC(k)
				END;
				INC(j)
			END;
			INC(i, dim)
		END
	END MatrixMap;

	PROCEDURE (node: Node) NumberConstraints (): INTEGER;
	BEGIN
		RETURN node.dim
	END NumberConstraints;

	PROCEDURE (node: Node) NumberNeighbours (): INTEGER;
		VAR
			num: INTEGER;
	BEGIN
		IF node.neighs # NIL THEN num := LEN(node.neighs) ELSE num := 0 END;
		RETURN num
	END NumberNeighbours;

	PROCEDURE (node: Node) RemoveSingletons, NEW;
		VAR
			dim, i, j, nElem, num, numNeigh: INTEGER;
			newCom, oldCom: GraphStochastic.Vector;
			car: Node;
	BEGIN
		(*	count number of non singleton regions	*)
		i := 0;
		num := 0;
		nElem := node.Size();
		oldCom := node.components;
		WHILE i < nElem DO
			car := oldCom[i](Node);
			IF car.neighs # NIL THEN INC(num) END;
			INC(i)
		END;
		IF num = nElem THEN RETURN END;
		(*	remove singleton regions	*)
		NEW(newCom, num);
		i := 0;
		num := 0;
		WHILE i < nElem DO
			car := oldCom[i](Node);
			IF car.neighs # NIL THEN
				newCom[num] := car;
				car.SetComponent(newCom, num);
				INC(num)
			ELSE
				car.Init;
				car.SetComponent(NIL, - 1);
				car.props := {GraphStochastic.hidden, GraphStochastic.initialized, GraphStochastic.data};
				car.value := 0.0
			END;
			INC(i)
		END;
		(*	renumber the neighbours	*)
		dim := node.dim;
		i := 0;
		WHILE i < num DO
			car := newCom[i](Node);
			j := 0;
			numNeigh := LEN(car.neighs);
			j := 0;
			WHILE j < numNeigh DO
				car.neighs[j] := oldCom[car.neighs[j] * dim](Node).index DIV dim;
				INC(j)
			END;
			INC(i, dim)
		END
	END RemoveSingletons;

	PROCEDURE (node: Node) SetMVGMRF (IN args: GraphNodes.Args; OUT res: SET);
		CONST
			eps = 1.0E-20;
		VAR
			dim, beg, i, index, numNeigh, nElem, off, start0, start1, start2, numIslands: INTEGER;
			x: REAL;
			p: GraphNodes.Node;
			car: Node;
	BEGIN
		res := {};
		index := node.index;
		dim := node.dim;
		nElem := node.Size();
		IF index MOD dim = 0 THEN
			WITH args: GraphStochastic.Args DO
				ASSERT(args.vectors[0].components # NIL, 21); ASSERT(args.vectors[0].start >= 0, 21);
				ASSERT(args.vectors[0].nElem > 0, 21);
				ASSERT(args.vectors[1].components # NIL, 21); ASSERT(args.vectors[1].start >= 0, 21);
				ASSERT(args.vectors[1].nElem > 0, 21);
				ASSERT(args.vectors[2].components # NIL, 21); ASSERT(args.vectors[2].start >= 0, 21);
				ASSERT(args.vectors[2].nElem > 0, 21);
				IF args.vectors[0].nElem # args.vectors[1].nElem THEN
					res := {GraphNodes.arg2, GraphNodes.length}; RETURN
				END;
				start2 := args.vectors[2].start;
				numNeigh := SHORT(ENTIER(args.vectors[2].components[start2 + index].value + eps));
				IF numNeigh > 0 THEN
					NEW(node.neighs, numNeigh);
					NEW(node.weights, numNeigh);
					i := 0;
					beg := 0;
					WHILE i < index DO
						car := node.components[i](Node);
						IF car.neighs # NIL THEN
							INC(beg, LEN(car.neighs))
						END;
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
						x := p.value;
						IF ABS(x - SHORT(ENTIER(x + eps))) > eps THEN
							res := {GraphNodes.arg1, GraphNodes.integer}; RETURN
						END;
						off := SHORT(ENTIER(x + eps)) - 1;
						IF off > nElem THEN
							res := {GraphNodes.arg1, GraphNodes.length}; RETURN
						END;
						node.neighs[i] := off;
						p := args.vectors[1].components[start1 + beg + i];
						IF p = NIL THEN
							res := {GraphNodes.arg2, GraphNodes.nil}; RETURN
						END;
						IF ~(GraphNodes.data IN p.props) THEN
							res := {GraphNodes.arg2, GraphNodes.notData}; RETURN
						END;
						node.weights[i] := p.value;
						INC(i)
					END
				END
			END
		ELSE
			index := (index DIV dim) * dim;
			car := node.components[index](Node);
			node.neighs := car.neighs;
			node.weights := car.weights
		END;
		(*	last element	*)
		IF node.index = nElem - 1 THEN
			node.RemoveSingletons;
			numIslands := NumIslands(node);
			i := 0;
			WHILE i < nElem DO
				car := node.components[i](Node);
				car.numIslands := numIslands;
				INC(i)
			END
		END
	END SetMVGMRF;

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
		maintainer := "A.Thomas"
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
		NEW(mean, size);
		NEW(value, size);
		NEW(tau, size, size)
	END Init;

BEGIN
	Init
END SpatialMVCAR.
