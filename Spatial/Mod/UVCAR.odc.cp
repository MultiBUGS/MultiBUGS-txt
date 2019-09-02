(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE SpatialUVCAR;


	

	IMPORT
		Stores := Stores64, 
		GraphMRF, GraphUVMRF, GraphNodes, GraphStochastic;

	TYPE
		Node* = POINTER TO ABSTRACT RECORD(GraphUVMRF.Node)
			neighs-: POINTER TO ARRAY OF INTEGER;
			weights-: POINTER TO ARRAY OF REAL;
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
		INCL(node.props, GraphNodes.mark);
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
			EXCL(car.props, GraphNodes.mark);
			INC(i)
		END;
		RETURN numIslands
	END NumIslands;

	PROCEDURE (node: Node) CheckUVCAR- (): SET, NEW, ABSTRACT;

	PROCEDURE (node: Node) CheckUVMRF- (): SET;
		CONST
			eps = 1.0E-3;
		VAR
			i, j, numNeigh, numNeigh1, nElem, index: INTEGER;
			constraint: REAL;
			p: Node;
			constraints: POINTER TO ARRAY OF ARRAY OF REAL;
	BEGIN
		index := node.index;
		IF index #  - 1 THEN (*	not special case of singleton islands	*)
			nElem := node.Size();
			IF node.neighs # NIL THEN numNeigh := LEN(node.neighs) ELSE numNeigh := 0 END;
			i := 0;
			WHILE i < numNeigh DO
				p := node.components[node.neighs[i]](Node);
				IF p.neighs # NIL THEN numNeigh1 := LEN(p.neighs) ELSE numNeigh1 := 0 END;
				j := 0; WHILE (j < numNeigh1) & (p.neighs[j] # index) DO INC(j) END;
				IF j = numNeigh1 THEN
					RETURN {GraphNodes.arg1, GraphNodes.notSymmetric}
				END;
				INC(i)
			END;
			IF (node = node.components[0]) & (node.NumberConstraints() = 1) THEN
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
		RETURN node.CheckUVCAR()
	END CheckUVMRF;
		
	PROCEDURE (node: Node) CountIslands*, NEW;
		VAR
			nElem, i, index, numIslands: INTEGER;
			car: Node;
	BEGIN
		index := node.index;
		nElem := node.Size();
		IF index = nElem - 1 THEN (*	this is the last region	*)
			(*	count number of islands always at least 1 the whole map	*)
			car := node.components[0](Node);
			numIslands := NumIslands(car);
			i := 0;
			WHILE i < nElem DO
				car := node.components[i](Node);
				car.numIslands := numIslands;
				EXCL(car.props, GraphNodes.mark);
				INC(i)
			END
		END
	END CountIslands;
	
	PROCEDURE (node: Node) ExternalizeUVCAR- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (node: Node) ExternalizeUVMRF- (VAR wr: Stores.Writer);
		VAR
			j, numNeigh: INTEGER;
	BEGIN
		wr.WriteInt(node.numIslands);
		IF node.index #  - 1 THEN
			IF node.neighs # NIL THEN numNeigh := LEN(node.neighs) ELSE numNeigh := 0 END;
			wr.WriteInt(numNeigh);
			j := 0;
			WHILE j < numNeigh DO
				wr.WriteInt(node.neighs[j]);
				wr.WriteReal(node.weights[j]);
				INC(j)
			END
		END;
		node.ExternalizeUVCAR(wr)
	END ExternalizeUVMRF;
	
	PROCEDURE (node: Node) InitUVCAR-, NEW, ABSTRACT;

	PROCEDURE (node: Node) InitUVMRF-;
	BEGIN
		node.neighs := NIL;
		node.weights := NIL;
		node.numIslands := 0;
		INCL(node.props, GraphStochastic.noMean);
		node.InitUVCAR
	END InitUVMRF;
	
	PROCEDURE (node: Node) InternalizeUVCAR- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (node: Node) InternalizeUVMRF- (VAR rd: Stores.Reader);
		VAR
			i, j, index, numNeigh, size: INTEGER;
			p: GraphStochastic.Node;
	BEGIN
		index := node.index;
		rd.ReadInt(node.numIslands);
		IF index =  - 1 THEN
			node.Init;
			node.SetComponent(NIL,  - 1);
			node.props := {GraphStochastic.hidden, GraphStochastic.initialized};
			node.value := 0.0
		ELSE
			rd.ReadInt(numNeigh);
			IF numNeigh # 0 THEN
				NEW(node.neighs, numNeigh);
				NEW(node.weights, numNeigh)
			END;
			j := 0;
			WHILE j < numNeigh DO
				rd.ReadInt(node.neighs[j]);
				rd.ReadReal(node.weights[j]);
				INC(j)
			END;
		END;
		node.InternalizeUVCAR(rd)
	END InternalizeUVMRF;

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
			numNeighs := p.NumberNeighbours();
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
			numNeighs := p.NumberNeighbours();
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

	PROCEDURE (node: Node) NumberNeighbours* (): INTEGER;
		VAR
			num: INTEGER;
	BEGIN
		IF node.neighs # NIL THEN num := LEN(node.neighs) ELSE num := 0 END;
		RETURN num
	END NumberNeighbours;
	
	PROCEDURE (node: Node) RemoveSingletons*, NEW;
		VAR
			i, j, nElem, num, numNeigh: INTEGER;
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
				car.SetComponent(NIL,  - 1);
				car.props := {GraphStochastic.hidden, GraphStochastic.initialized, GraphStochastic.data};
				car.value := 0.0
			END;
			INC(i)
		END;
		(*	renumber the neighbours	*)
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
		END
	END RemoveSingletons;

	PROCEDURE (node: Node) SetUVCAR- (IN args: GraphNodes.Args; OUT res: SET), NEW, ABSTRACT;

	PROCEDURE (node: Node) SetUVMRF- (IN args: GraphNodes.Args; OUT res: SET);
		CONST
			eps = 1.0E-20;
		VAR
			beg, i, index,numNeigh, nElem, off, start0, start1, start2: INTEGER;
			x: REAL;
			p: GraphNodes.Node;
			car: Node;
	BEGIN
		res := {};
		index := node.index;
		nElem := node.Size();
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
				END;
			END
		END;
		node.SetUVCAR(args, res)
	END SetUVMRF;
	
	PROCEDURE Maintainer;
	BEGIN
		version := 500; 
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END SpatialUVCAR.

