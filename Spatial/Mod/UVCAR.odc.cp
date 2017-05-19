(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE SpatialUVCAR;


	

	IMPORT
		Stores,
		GraphMRF, GraphNodes, GraphParamtrans, GraphRules, GraphStochastic;

	TYPE
		Node* = POINTER TO ABSTRACT RECORD(GraphMRF.Node)
			neighs-: POINTER TO ARRAY OF INTEGER;
			weights-: POINTER TO ARRAY OF REAL;
			tau-: GraphNodes.Node;
			numIslands-: INTEGER
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE MarkNeighs (node: Node);
		VAR
			i, numNeigh: INTEGER;
			car: Node;
	BEGIN
		IF GraphNodes.mark IN node.props THEN RETURN END;
		node.SetProps(node.props + {GraphNodes.mark});
		i := 0;
		IF node.neighs # NIL THEN numNeigh := LEN(node.neighs) ELSE numNeigh := 0 END;
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
			new: BOOLEAN;
			i, numIslands, nElem: INTEGER;
			car: Node;
	BEGIN
		numIslands := 0;
		nElem := node.Size();
		i := 0;
		REPEAT
			new := FALSE;
			WHILE (i < nElem) & (GraphNodes.mark IN node.components[i].props) DO
				INC(i)
			END;
			IF i < nElem THEN
				new := TRUE;
				INC(numIslands);
				car := node.components[i](Node);
				MarkNeighs(car)
			END;
			INC(i)
		UNTIL ~new;
		i := 0;
		WHILE i < nElem DO
			car := node.components[i](Node);
			car.SetProps(car.props - {GraphNodes.mark});
			INC(i)
		END;
		RETURN numIslands
	END NumIslands;

	PROCEDURE (node: Node) Bounds* (OUT lower, upper: REAL);
	BEGIN
		lower :=  - INF;
		upper := INF
	END Bounds;

	PROCEDURE (node: Node) Check* (): SET;
		CONST
			eps = 1.0E-3;
		VAR
			i, j, len0, len1, n, nElem, thisArea: INTEGER;
			res: SET;
			constraint, tau, weight: REAL;
			neigh: Node;
			constraints: POINTER TO ARRAY OF ARRAY OF REAL;
	BEGIN
		res := {};
		IF node.index #  - 1 THEN (*	not special case of singleton islands	*)
			thisArea := node.index;
			nElem := node.Size();
			IF node.neighs # NIL THEN len0 := LEN(node.neighs) ELSE len0 := 0 END;
			i := 0;
			WHILE i < len0 DO
				weight := node.weights[i];
				n := node.neighs[i];
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
			tau := node.tau.Value();
			IF tau < eps THEN
				RETURN {GraphNodes.arg4, GraphNodes.invalidPosative}
			END;
			IF node = node.components[0] THEN
				NEW(constraints, 1, nElem);
				node.Constraints(constraints);
				j := 0;
				constraint := 0.0;
				WHILE j < nElem DO
					constraint := constraint + node.components[j].value * constraints[0, j];
					INC(j)
				END;
				constraint := constraint / nElem;
				IF ABS(constraint) > eps THEN
					RETURN {GraphNodes.lhs, GraphNodes.invalidValue}
				END
			END
		END;
		RETURN res
	END Check;

	PROCEDURE (node: Node) ClassifyLikelihood* (parent: GraphStochastic.Node): INTEGER;
		VAR
			density, f: INTEGER;
	BEGIN
		f := GraphStochastic.ClassFunction(node.tau, parent);
		density := GraphRules.ClassifyPrecision(f);
		RETURN density
	END ClassifyLikelihood;

	PROCEDURE (node: Node) Constraints* (OUT constraints: ARRAY OF ARRAY OF REAL);
		VAR
			i, nElem: INTEGER;
	BEGIN
		nElem := node.Size();
		i := 0;
		WHILE i < nElem DO constraints[0, i] := 1.0; INC(i) END;
	END Constraints;

	PROCEDURE (node: Node) Deviance* (): REAL;
	BEGIN
		RETURN 0.0
	END Deviance;

	PROCEDURE (node: Node) ExternalizeChain- (VAR wr: Stores.Writer);
		VAR
			j, numNeigh: INTEGER;
	BEGIN
		IF node.index = 0 THEN
			GraphNodes.Externalize(node.tau, wr);
			wr.WriteInt(node.numIslands);
		END;
		IF node.index #  - 1 THEN
			numNeigh := LEN(node.neighs);
			wr.WriteInt(numNeigh);
			j := 0;
			WHILE j < numNeigh DO
				wr.WriteInt(node.neighs[j]);
				wr.WriteReal(node.weights[j]);
				INC(j)
			END
		END
	END ExternalizeChain;

	PROCEDURE (node: Node) InitStochastic-;
	BEGIN
		node.tau := NIL;
		node.neighs := NIL;
		node.weights := NIL;
		node.numIslands := 0;
		node.SetProps(node.props + {GraphStochastic.noMean})
	END InitStochastic;

	PROCEDURE (node: Node) InternalizeChain- (VAR rd: Stores.Reader);
		VAR
			j, numNeigh: INTEGER;
			p: Node;
	BEGIN
		IF node.index = 0 THEN
			node.tau := GraphNodes.Internalize(rd);
			rd.ReadInt(node.numIslands);
		END;
		IF node.index =  - 1 THEN
			node.Init;
			node.SetComponent(NIL,  - 1);
			node.SetProps({GraphStochastic.nR, GraphStochastic.initialized});
			node.tau := NIL;
			node.SetValue(0.0)
		ELSE
			p := node.components[0](Node);
			node.tau := p.tau;
			node.numIslands := p.numIslands;
			rd.ReadInt(numNeigh);
			NEW(node.neighs, numNeigh);
			NEW(node.weights, numNeigh);
			j := 0;
			WHILE j < numNeigh DO
				rd.ReadInt(node.neighs[j]);
				rd.ReadReal(node.weights[j]);
				INC(j)
			END;
		END
	END InternalizeChain;

	PROCEDURE (node: Node) Location* (): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END Location;

	PROCEDURE (node: Node) MatrixInfo* (OUT type, nElements: INTEGER);
		VAR
			i, numNeighs, nElem: INTEGER;
			p: Node;
	BEGIN
		type := GraphMRF.sparse;
		nElem := node.Size();
		i := 0;
		nElements := 0;
		WHILE i < nElem DO
			p := node.components[i](Node);
			numNeighs := LEN(p.neighs);
			INC(nElements, numNeighs);
			INC(i)
		END;
		ASSERT(~ODD(nElements), 66);
		nElements := nElem + nElements DIV 2
	END MatrixInfo;

	PROCEDURE (node: Node) MatrixMap* (OUT rowInd, colPtr: ARRAY OF INTEGER);
		VAR
			i, index, j, nnz, numNeighs, nElem: INTEGER;
			p, q: Node;
	BEGIN
		i := 0;
		nnz := 0;
		nElem := node.Size();
		WHILE i < nElem DO
			colPtr[i] := nnz;
			p := node.components[i](Node);
			numNeighs := LEN(p.neighs);
			index := p.index;
			rowInd[nnz] := index; INC(nnz);
			j := 0;
			WHILE j < numNeighs DO
				q := node.components[p.neighs[j]](Node);
				IF q.index > index THEN rowInd[nnz] := q.index; INC(nnz) END;
				INC(j)
			END;
			INC(i)
		END
	END MatrixMap;

	PROCEDURE (node: Node) NumberConstraints* (): INTEGER;
	BEGIN
		RETURN 1
	END NumberConstraints;

	PROCEDURE (node: Node) NumberNeighbours* (): INTEGER;
		VAR
			num: INTEGER;
	BEGIN
		IF node.neighs # NIL THEN num := LEN(node.neighs) ELSE num := 0 END;
		RETURN num
	END NumberNeighbours;

	PROCEDURE (node: Node) Parents* (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		IF (node.index = 0) OR (all & ~(GraphStochastic.nR IN node.props))THEN
			node.tau.AddParent(list)
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Set* (IN args: GraphNodes.Args; OUT res: SET);
		CONST
			eps = 1.0E-20;
		VAR
			beg, i, index, j, num, numIslands, numNeigh, nElem, off, start0, start1, start2: INTEGER;
			x: REAL;
			p: GraphNodes.Node;
			car: Node;
			newCom, oldCom: GraphStochastic.Vector;
	BEGIN
		res := {};
		index := node.index;
		(*	if Set method called a second time then cut things short	*)
		IF index =  - 1 THEN RETURN END;
		IF node.tau # NIL THEN
			WITH args: GraphStochastic.Args DO
				node.tau := args.scalars[0]
			END;
			RETURN
		END;
		nElem := node.Size();
		oldCom := node.components;
		WITH args: GraphStochastic.Args DO
			ASSERT(args.vectors[0].components # NIL, 21); ASSERT(args.vectors[0].start >= 0, 21);
			ASSERT(args.vectors[0].nElem > 0, 21);
			ASSERT(args.vectors[1].components # NIL, 21); ASSERT(args.vectors[1].start >= 0, 21);
			ASSERT(args.vectors[1].nElem > 0, 21);
			ASSERT(args.vectors[2].components # NIL, 21); ASSERT(args.vectors[2].start >= 0, 21);
			ASSERT(args.vectors[2].nElem > 0, 21);
			ASSERT(args.scalars[0] # NIL, 21);
			IF args.vectors[0].nElem # args.vectors[1].nElem THEN
				res := {GraphNodes.arg2, GraphNodes.length}; RETURN
			END;
			start2 := args.vectors[2].start;
			numNeigh := SHORT(ENTIER(args.vectors[2].components[start2 + index].Value() + eps));
			IF numNeigh > 0 THEN
				node.tau := args.scalars[0];
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
					x := p.Value();
					IF ABS(x - SHORT(ENTIER(x + eps))) > eps THEN
						res := {GraphNodes.arg1, GraphNodes.integer}; RETURN
					END;
					off := SHORT(ENTIER(x + eps)) - 1;
					IF off > nElem THEN
						res := {GraphNodes.arg1, GraphNodes.length}; RETURN
					END;
					node.neighs[i] := node.components[off](Node).index;
					p := args.vectors[1].components[start1 + beg + i];
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
				node.Init;
				node.SetComponent(NIL,  - 1);
				node.SetProps({GraphStochastic.nR, GraphStochastic.initialized});
				node.tau := NIL;
				node.SetValue(0.0)
			END
		END;
		IF index = nElem - 1 THEN (*	this is the last region	*)
			(*	count number of non singleton regions	*)
			i := 0;
			num := 0;
			WHILE i < nElem DO
				car := oldCom[i](Node);
				IF car.neighs # NIL THEN INC(num) END;
				INC(i)
			END;
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
	END Set;

	PROCEDURE (node: Node) TransformParams*, NEW;
	BEGIN
	 node.tau := GraphParamtrans.LogTransform(node.tau)
	END TransformParams;
	
	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END SpatialUVCAR.

