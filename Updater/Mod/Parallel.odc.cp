(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)

MODULE UpdaterParallel;

	IMPORT 
		Stores, StdLog,
		GraphDeviance, GraphFlat, GraphMRF, GraphLogical, GraphNodes, GraphStochastic,
		UpdaterActions, UpdaterConjugateMV, UpdaterEmpty,
		UpdaterMultivariate, UpdaterUpdaters;

	TYPE
		List = POINTER TO RECORD
			updater: UpdaterUpdaters.Updater;
			next: List
		END;

		RowId = POINTER TO RECORD
			id: INTEGER;
			next: RowId
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		colPtr, rowInd: POINTER TO ARRAY OF INTEGER;
		
	PROCEDURE MarkChildren* (children: GraphStochastic.Vector; numProc, rank: INTEGER);
		VAR
			node: GraphStochastic.Node;
			i,  j, num: INTEGER;
	BEGIN
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		j := 0;
		i := 0;
		WHILE (i < rank) & (j < num) DO
			node := children[j];
			WITH node: GraphMRF.Node DO
				node.SetProps(node.props + {GraphStochastic.mark});
				node.ThinLikelihood(rank, numProc)
			ELSE
				INC(i)
			END;
			INC(j)
		END;
		IF i = rank THEN
			node := children[j];
			node.SetProps(node.props + {GraphStochastic.mark})
		END;
		WHILE j < num DO
			i := 0;
			WHILE (i < numProc) & (j < num) DO
				node := children[j];
				WITH node: GraphMRF.Node DO
					node.SetProps(node.props + {GraphStochastic.mark});
					node.ThinLikelihood(rank, numProc)
				ELSE
					INC(i)
				END;
				INC(j)
			END;
			IF (i = numProc) & (j < num) THEN
				node := children[j];
				node.SetProps(node.props + {GraphStochastic.mark});
			END
		END
	END MarkChildren;

	PROCEDURE UnMarkChildren (children: GraphStochastic.Vector);
		VAR
			i, num: INTEGER;
			p: GraphStochastic.Node;
	BEGIN
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		WHILE i < num DO
			p := children[i];
			p.SetProps(p.props - {GraphStochastic.mark});
			INC(i)
		END
	END UnMarkChildren;
	
	PROCEDURE ThinChildren (children: GraphStochastic.Vector): GraphStochastic.Vector;
		VAR
			i, num, len, j: INTEGER;
			vector: GraphStochastic.Vector;
	BEGIN
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		len := 0;
		WHILE i < num DO
			IF GraphStochastic.mark IN children[i].props THEN INC(len) END;
			INC(i)
		END;
		IF len > 0 THEN NEW(vector, len) ELSE vector := NIL END;
		i := 0;
		j := 0;
		WHILE i < num DO
			IF GraphStochastic.mark IN children[i].props THEN
				vector[j] := children[i];
				INC(j)
			END;
			INC(i)
		END;
		RETURN vector
	END ThinChildren;

	PROCEDURE ThinUpdaters (updaters: UpdaterUpdaters.Vector; numProc, rank: INTEGER);
		VAR
			u: UpdaterUpdaters.Updater;
			children, thinned: GraphStochastic.Vector;
			p: GraphStochastic.Node;
			i, j, size, numUpdaters: INTEGER;
	BEGIN
		numUpdaters := LEN(updaters);
		i := 0;
		WHILE i < numUpdaters DO
			u := updaters[i];
			p := u.Prior(0);
			IF p # NIL THEN
				IF GraphStochastic.distributed IN p.props THEN
					children := u.Children();
					MarkChildren(children, numProc, rank);
					thinned := ThinChildren(children);
					IF u IS UpdaterMultivariate.Updater THEN
						u.SetChildren(thinned);
						j := 0;
						size := u.Size();
						WHILE j < size DO
							p := u.Prior(j);
							thinned := p.Children();
							thinned := ThinChildren(thinned);
							p.SetChildren(thinned);
							INC(j)
						END
					ELSE
						p := u.Prior(0);
						p.SetChildren(thinned)
					END;
					UnMarkChildren(children)
				END
			END;
			INC(i)
		END
	END ThinUpdaters;

	PROCEDURE IsMarkedLike (updater: UpdaterUpdaters.Updater): BOOLEAN;
		VAR
			isMarked: BOOLEAN;
			children: GraphStochastic.Vector;
			i, num: INTEGER;
	BEGIN
		children := updater.Children();
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		isMarked := FALSE;
		WHILE ~isMarked & (i < num) DO
			isMarked := GraphStochastic.mark IN children[i].props;
			INC(i)
		END;
		RETURN isMarked
	END IsMarkedLike;

	PROCEDURE MarkLike (updater: UpdaterUpdaters.Updater);
		VAR
			children: GraphStochastic.Vector;
			node: GraphStochastic.Node;
			i, num: INTEGER;
	BEGIN
		children := updater.Children();
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		WHILE i < num DO
			node := children[i];
			node.SetProps(node.props + {GraphStochastic.mark});
			INC(i)
		END
	END MarkLike;

	PROCEDURE UnMarkLike (updater: UpdaterUpdaters.Updater);
		VAR
			children: GraphStochastic.Vector;
			node: GraphStochastic.Node;
			i, num: INTEGER;
	BEGIN
		children := updater.Children();
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		WHILE i < num DO
			node := children[i];
			node.SetProps(node.props - {GraphStochastic.mark});
			INC(i)
		END
	END UnMarkLike;

	PROCEDURE IsMarkedDependents (updater: UpdaterUpdaters.Updater): BOOLEAN;
		VAR
			i, size: INTEGER;
			prior: GraphStochastic.Node;
			list: GraphLogical.List;
			logical: GraphLogical.Node;
			isMarked: BOOLEAN;
	BEGIN
		isMarked := FALSE;
		size := updater.Size();
		i := 0;
		WHILE (i < size) & ~isMarked DO
			prior := updater.Prior(i);
			list := prior.dependents;
			WHILE (list # NIL) & ~isMarked DO
				logical := list.node;
				isMarked :=  GraphStochastic.mark IN logical.props;
				list := list.next
			END;
			INC(i)
		END;
		RETURN isMarked
	END IsMarkedDependents;
	
	PROCEDURE MarkDependents (updater: UpdaterUpdaters.Updater);
		VAR
			i, size: INTEGER;
			prior: GraphStochastic.Node;
			list: GraphLogical.List;
			logical: GraphLogical.Node;
	BEGIN
		size := updater.Size();
		i := 0;
		WHILE i < size DO
			prior := updater.Prior(i);
			list := prior.dependents;
			WHILE list # NIL DO
				logical := list.node;
				IF GraphStochastic.stochParent IN logical.props THEN
					logical.SetProps(logical.props + {GraphStochastic.mark})
				END;
				list := list.next
			END;
			INC(i)
		END
	END MarkDependents;

	PROCEDURE UnMarkDependents (updater: UpdaterUpdaters.Updater);
		VAR
			i, size: INTEGER;
			prior: GraphStochastic.Node;
			list: GraphLogical.List;
			logical: GraphLogical.Node;
	BEGIN
		size := updater.Size();
		i := 0;
		WHILE i < size DO
			prior := updater.Prior(i);
			list := prior.dependents;
			WHILE list # NIL DO
				logical := list.node;
				logical.SetProps(logical.props - {GraphStochastic.mark});
				list := list.next
			END;
			INC(i)
		END
	END UnMarkDependents;

	PROCEDURE IsMarkedNeighbour (updater: UpdaterUpdaters.Updater): BOOLEAN;
		VAR
			i, index, j: INTEGER;
			prior, neigh: GraphStochastic.Node;
			isMarked: BOOLEAN;
	BEGIN
		isMarked := FALSE;
		prior := updater.Prior(0);
		WITH prior: GraphMRF.Node DO
			index := prior.index;
			i := colPtr[index] + 1;
			WHILE ~isMarked & (i < colPtr[index + 1]) DO
				neigh := prior.components[rowInd[i]];
				isMarked := GraphStochastic.mark IN neigh.props;
				INC(i)
			END;
			j := 0;
			WHILE ~isMarked & (j < index) DO
				i := colPtr[j] + 1;
				WHILE ~isMarked & (i < colPtr[j + 1]) DO
					IF rowInd[i] = index THEN
						neigh := prior.components[j];
						isMarked := GraphStochastic.mark IN neigh.props
					END;
					INC(i)
				END;
				INC(j)
			END
		ELSE
		END;
		RETURN isMarked
	END IsMarkedNeighbour;
	
	PROCEDURE MarkNeighbour (updater: UpdaterUpdaters.Updater);
		VAR
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.Prior(0);
		WITH prior: GraphMRF.Node DO
			prior.SetProps(prior.props + {GraphStochastic.mark})
		ELSE
		END
	END MarkNeighbour;

	PROCEDURE UnMarkNeighbour (updater: UpdaterUpdaters.Updater);
		VAR
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.Prior(0);
		WITH prior: GraphMRF.Node DO
			prior.SetProps(prior.props - {GraphStochastic.mark})
		ELSE
		END
	END UnMarkNeighbour;

	PROCEDURE IsMarked (u: UpdaterUpdaters.Updater): BOOLEAN;
		VAR
			isMarked: BOOLEAN;
	BEGIN
		isMarked := IsMarkedNeighbour(u) OR IsMarkedLike(u) OR IsMarkedDependents(u);
		RETURN isMarked
	END IsMarked;
	
	PROCEDURE Mark (u: UpdaterUpdaters.Updater);
	BEGIN
		MarkLike(u); MarkDependents(u); MarkNeighbour(u)
	END Mark;
	
	PROCEDURE UnMark (u: UpdaterUpdaters.Updater);
	BEGIN
		UnMarkLike(u); UnMarkDependents(u); UnMarkNeighbour(u)
	END UnMark;
	
	(*	distribute fixed effects	*)
	PROCEDURE FixedEffects (depth, chain: INTEGER; VAR lists: ARRAY OF List; VAR rowId: RowId);
		VAR
			finish, i, j, commSize, start: INTEGER;
			updater: UpdaterUpdaters.Updater;
			prior: GraphStochastic.Node;
			element: RowId;
			link: List;
	BEGIN
		UpdaterActions.StartFinish(depth, start, finish);
		commSize := LEN(lists);
		i := start;
		WHILE i # finish DO
			updater := UpdaterActions.GetUpdater(chain, i);
			prior := updater.Prior(0);
			(*	distributed updater 	*)
			IF GraphStochastic.distributed IN prior.props THEN	
				j := 0;
				WHILE j < commSize DO
					NEW(link); link.next := lists[j]; lists[j] := link; INC(j)
				END;
				NEW(element); element.id := 0; element.next := rowId; rowId := element;
				(*	add new row of updaters	*)
				j := 0;
				WHILE j < commSize DO
					lists[j].updater := updater;
					INC(j)
				END
			END;
			INC(i)
		END
	END FixedEffects;

	(*	distribute un-structured random effects	*)
	PROCEDURE RandomEffects (depth, chain: INTEGER; 
						VAR lists: ARRAY OF List; VAR rowId: RowId; OUT new: BOOLEAN);
		VAR
			i, j, finish, rank, commSize, numRows, size, start: INTEGER;
			u: UpdaterUpdaters.Updater;
			prior: GraphStochastic.Node;
			link: List;
			element: RowId;
	BEGIN
		new := FALSE;
		UpdaterActions.StartFinish(depth, start, finish);
		commSize := LEN(lists);
		rank := 0;
		numRows := 1;
		i := start;
		WHILE i # finish DO
			u := UpdaterActions.GetUpdater(chain, i);
			prior := u.UpdatedBy(0);
			IF ({GraphStochastic.distributed, GraphNodes.mark} * prior.props = {}) THEN	
				(*	potential parallel updater	*)
				IF ~(prior IS GraphMRF.Node) & ~IsMarked(u) THEN	
				(*	likelihoods disjoint etc	*)
					IF rank = 0 THEN	(*	add new row of updaters	*)
						j := 0;
						WHILE j < commSize DO
							NEW(link); link.next := lists[j]; lists[j] := link; INC(j)
						END;
						NEW(element); element.id := numRows; element.next := rowId; rowId := element;
						INC(numRows);
						size := u.Size()
					END;
					IF size = u.Size() THEN	(*	check size of potential new updater	*)
						prior.SetProps(prior.props + {GraphNodes.mark});
						new := TRUE;
						Mark(u);
						lists[rank].updater := u;
						INC(rank);
						rank := rank MOD commSize
					END
				END
			END;
			INC(i)
		END;
		i := start;
		WHILE i # finish DO
			u := UpdaterActions.GetUpdater(chain, i);
			UnMark(u);
			INC(i)
		END;
		rowId.id := -ABS(rowId.id)
	END RandomEffects;

	PROCEDURE FindMRF (depth, chain: INTEGER; 
							OUT mrfList: List; OUT constraint: UpdaterUpdaters.Updater);
		VAR
			gmrf: GraphMRF.Node;
			list, element, cursor: List;
			i, start, finish, size, type, nElements, numNeigh: INTEGER;
			prior: GraphStochastic.Node;
			u: UpdaterUpdaters.Updater;
	BEGIN
		list := NIL;
		gmrf := NIL;
		constraint := NIL;
		UpdaterActions.StartFinish(depth, start, finish);
		i := start;
		WHILE (i # finish) & (gmrf = NIL) DO
			u := UpdaterActions.GetUpdater(chain, i);
			prior := u.UpdatedBy(0);
			WITH prior: GraphMRF.Node DO
				IF ~(GraphNodes.mark IN prior.props) THEN
					numNeigh := prior.NumberNeighbours();
					gmrf := prior.components[0](GraphMRF.Node);
					gmrf.MatrixInfo(type, nElements);
					size := prior.Size();
					NEW(rowInd, nElements);
					NEW(colPtr, size + 1);
					gmrf.MatrixMap(rowInd, colPtr);
					NEW(element);
					element.updater := u;
					element.next := list;
					list := element
				END
			ELSE
			END;
			INC(i)
		END; 
		(*	add new gmrf nodes to list	*)
		WHILE i # finish DO
			u := UpdaterActions.GetUpdater(chain, i);
			prior := u.UpdatedBy(0);
			WITH prior: GraphMRF.Node DO
				IF ~(GraphNodes.mark IN prior.props) & (prior.components[0] = gmrf) THEN
					IF u.Size() # 0 THEN
						numNeigh := MAX(numNeigh, prior.NumberNeighbours());
						NEW(element);
						element.updater := u;
						element.next := list;
						list := element
					ELSE
						constraint := u
					END
				END
			ELSE
			END;
			INC(i)
		END;
		(*	sort list by number of neighbours	*)
		i := 0;
		mrfList := NIL;
		WHILE i <= numNeigh DO
			cursor := list;
			WHILE cursor # NIL DO
				u := cursor.updater;
				prior := u.UpdatedBy(0);
				gmrf := prior(GraphMRF.Node);
				IF gmrf.NumberNeighbours() = i THEN
					NEW(element);
					element.updater := u;
					element.next := mrfList;
					mrfList := element
				END;
				cursor := cursor.next
			END;
			INC(i)
		END
	END FindMRF;

	(*	distribute structured random effects	*)
	PROCEDURE RandomEffectsMRF (depth, chain: INTEGER; mrfList: List;
						VAR lists: ARRAY OF List; VAR rowId: RowId; OUT lastColour: BOOLEAN);
		VAR
			j, rank, commSize, numRows: INTEGER;
			u: UpdaterUpdaters.Updater;
			prior: GraphStochastic.Node;
			link, cursor: List;
			element: RowId;
	BEGIN
		commSize := LEN(lists);
		rank := 0;
		numRows := 1;
		cursor := mrfList;
		WHILE cursor # NIL DO
			u := cursor.updater;
			prior := u.UpdatedBy(0);
			IF ~(GraphNodes.mark IN  prior.props) & ~IsMarked(u) THEN	
				IF rank = 0 THEN	(*	add new row of updaters	*)
					j := 0;
					WHILE j < commSize DO
						NEW(link); link.next := lists[j]; lists[j] := link; INC(j)
					END;
					NEW(element); element.id := numRows; element.next := rowId; rowId := element;
					INC(numRows);
				END;
				prior.SetProps(prior.props + {GraphNodes.mark});
				Mark(u);
				lists[rank].updater := u;
				INC(rank);
				rank := rank MOD commSize
			END;
			cursor := cursor.next
		END;
		cursor := mrfList;
		lastColour := TRUE;
		WHILE cursor # NIL DO
			u := cursor.updater;
			UnMark(u);
			prior := u.UpdatedBy(0);
			IF ~(GraphNodes.mark IN  prior.props) THEN lastColour := FALSE END;
			cursor := cursor.next
		END;
		rowId.id := -ABS(rowId.id)
	END RandomEffectsMRF;
	
	PROCEDURE AddConstraint (constraint: UpdaterUpdaters.Updater;VAR  lists: ARRAY OF List;
	VAR rowId: RowId);
		VAR
			j, commSize: INTEGER;
			element: RowId;
			link: List;
	BEGIN
		commSize := LEN(lists);
		j := 0;
		WHILE j < commSize DO
			NEW(link); link.next := lists[j]; lists[j] := link; INC(j)
		END;
		NEW(element); element.id := 0; element.next := rowId; rowId := element;
		(*	add new row of updaters	*)
		j := 0;
		WHILE j < commSize DO
			lists[j].updater := constraint;
			INC(j)
		END
	END AddConstraint;
	
	PROCEDURE CompleteRows (lists: ARRAY OF List);
		VAR
			i, num, size: INTEGER;
			updater: UpdaterUpdaters.Updater;
	BEGIN
		num := LEN(lists);
		WHILE lists[0] # NIL DO
			updater := lists[0].updater;
			size := updater.Size();
			i := 1;
			WHILE i < num DO
				IF lists[i].updater = NIL THEN
					updater := UpdaterEmpty.New(size);
					lists[i].updater := updater
				END;
				INC(i)
			END;
			i := 0;
			WHILE i < num DO
				lists[i] := lists[i].next;
				INC(i)
			END
		END
	END CompleteRows;

	PROCEDURE DistributeObservations* (updaters: POINTER TO ARRAY OF UpdaterUpdaters.Vector;
	id: POINTER TO ARRAY OF INTEGER;
	OUT observations: POINTER TO ARRAY OF GraphStochastic.Vector);
		VAR
			i, j, k, numChild, numUpdaters, commSize: INTEGER;
			children: GraphStochastic.Vector;
			child: GraphStochastic.Node;
			numData: POINTER TO ARRAY OF INTEGER;
			ok: BOOLEAN;

		CONST
			count = 0;
			add = 1;
			
		PROCEDURE ClearMarks;
		BEGIN
			i := 0;
			WHILE i < numUpdaters DO
				j := 0;
				WHILE j < commSize DO
					children := updaters[j, i].Children();
					GraphStochastic.ClearMarks(children, {GraphNodes.mark});
					INC(j)
				END;
				INC(i)
			END
		END ClearMarks;

		PROCEDURE DevianceDo (action: INTEGER): BOOLEAN;
		BEGIN
			i := 0;
			WHILE i < numUpdaters DO
				j := 0;
				WHILE j < commSize DO
					children := updaters[j, i].Children();
					IF children # NIL THEN numChild := LEN(children) ELSE numChild := 0 END;
					k := 0;
					WHILE k < numChild DO
						IF (id[i] # 0) OR (k MOD commSize = j) THEN
							child := children[k];
							IF GraphDeviance.IsObserved(child) THEN
								IF ~GraphDeviance.DevianceExists(child) THEN
									WHILE j < commSize DO numData[j] := 0; INC(j) END;
									ClearMarks;
									RETURN FALSE
								END;
								IF ~(GraphNodes.mark IN child.props) THEN
									child.SetProps(child.props + {GraphNodes.mark});
									IF action = add THEN observations[j, numData[j]] := child END;
									INC(numData[j])
								END
							END
						END;
						INC(k)
					END;
					INC(j)
				END;
				INC(i)
			END;
			ClearMarks;
			RETURN TRUE
		END DevianceDo;

	BEGIN
		numUpdaters := LEN(updaters[0]);
		commSize := LEN(updaters);
		NEW(observations, commSize);
		NEW(numData, commSize);
		WHILE j < commSize DO numData[j] := 0; INC(j) END;
		ok := DevianceDo(count);
		j := 0;
		WHILE j < commSize DO
			IF numData[j] > 0 THEN 
				NEW(observations[j], numData[j]);
				numData[j] := 0
			ELSE
				observations[j] := NIL
			END;
			INC(j)
		END;
		IF ok THEN ok := DevianceDo(add) END
	END DistributeObservations;

	PROCEDURE DistributeUpdaters* (commSize, chain: INTEGER;
	OUT updaters: POINTER TO ARRAY OF UpdaterUpdaters.Vector;
	OUT id: POINTER TO ARRAY OF INTEGER);
		VAR
			i, j, depth, maxDepth, minDepth, numUpdaters: INTEGER;
			cursor, element, list, mrfList: List;
			lists: POINTER TO ARRAY OF List;
			rowId: RowId;
			prior, q: GraphStochastic.Node;
			constraint: UpdaterUpdaters.Updater;
			lastColour, new: BOOLEAN;
	BEGIN
		rowId := NIL;
		NEW(lists, commSize);
		UpdaterActions.MinMaxDepth(minDepth, maxDepth);
		i := 0;
		WHILE i < commSize DO
			lists[i] := NIL;
			INC(i)
		END;
		UpdaterActions.MarkDistributed;
		depth := maxDepth;
		WHILE depth >= minDepth DO
			FixedEffects(depth, chain, lists, rowId); 
			(*	do structured random effects	*)
			FindMRF(depth, chain, mrfList, constraint);
			WHILE mrfList # NIL DO
				REPEAT
					RandomEffectsMRF(depth, chain, mrfList, lists, rowId, lastColour); 
				UNTIL lastColour;
				IF constraint # NIL THEN 
					AddConstraint(constraint, lists, rowId)
				END; 
				FindMRF(depth, chain, mrfList, constraint)
			END;
			(*	do un-structured random effects	*)
			REPEAT
				RandomEffects(depth, chain, lists, rowId, new); 
			UNTIL ~new;
			DEC(depth)
		END;
		i := 0;
		WHILE i < commSize DO
			(*	reverse lists*)
			list := NIL;
			cursor := lists[i];
			WHILE cursor # NIL DO
				NEW(element);
				element.updater := cursor.updater;
				element.next := list;
				list := element;
				cursor := cursor.next
			END;
			lists[i] := list;
			INC(i)
		END;
		CompleteRows(lists);
		cursor := lists[0];
		numUpdaters := 0;
		WHILE cursor # NIL DO
			INC(numUpdaters);
			cursor := cursor.next
		END;
		NEW(updaters, commSize);
		i := 0;
		WHILE i < commSize DO
			NEW(updaters[i], numUpdaters);
			INC(i)
		END;
		NEW(id, numUpdaters);
		j := 0;
		WHILE j < numUpdaters DO
			i := 0;
			WHILE i < commSize DO
				updaters[i, j] := lists[i].updater;
				prior := lists[i].updater.UpdatedBy(0);
				IF prior # NIL THEN
					prior.SetProps(prior.props - {GraphNodes.mark})
				END;
				INC(i)
			END;
			i := 0;
			WHILE i < commSize DO
				lists[i] := lists[i].next;
				INC(i)
			END;
			INC(j);
			id[numUpdaters - j] := rowId.id;
			rowId := rowId.next
		END;
		(*	if only one updater in parallel block distribute updater if possible	*)
		IF commSize > 1 THEN
			j := 0;
			WHILE j < numUpdaters DO
				IF id[j] =  - 1 THEN
					q := updaters[0, j].Prior(0);
					prior := updaters[1, j].Prior(0);
					IF (prior = NIL) & ~(q IS GraphMRF.Node) THEN
						id[j] := 0;
						prior := updaters[0, j].Prior(0);
						prior.SetProps(prior.props + {GraphStochastic.distributed});
						i := 1;
						WHILE i < commSize DO
							updaters[i, j] := updaters[0, j];
							INC(i)
						END
					END
				END;
				INC(j)
			END
		END;
		colPtr := NIL;
		rowInd := NIL
	END DistributeUpdaters;

	PROCEDURE CountRows (id: POINTER TO ARRAY OF INTEGER; blockNum: INTEGER): INTEGER;
		VAR
			i, b: INTEGER;
	BEGIN
		b := 0;
		i := 0;
		WHILE b < blockNum DO
			IF id[i] < 0 THEN INC(b) END;
			INC(i)
		END;
		RETURN i - 1
	END CountRows;

	PROCEDURE ParallelBlock* (IN updaters: ARRAY OF UpdaterUpdaters.Vector;
	id: POINTER TO ARRAY OF INTEGER; blockNum: INTEGER): GraphStochastic.Vector;
		VAR
			block: GraphStochastic.Vector;
			numRow, row, size, i, j, k, numProc, offset, start: INTEGER;
			u: UpdaterUpdaters.Updater;
	BEGIN
		numProc := LEN(updaters);
		row := CountRows(id, blockNum);
		numRow := ABS(id[row]);
		start := row + 1 - numRow;
		size := updaters[0, start].Size();
		NEW(block, numProc * size * numRow);
		i := 0;
		offset := 0;
		WHILE i < numProc DO
			j := start;
			WHILE j < start + numRow DO
				u := updaters[i, j];
				k := 0;
				WHILE k < size DO
					block[offset] := u.UpdatedBy(k);
					IF block[offset] = NIL THEN (*	handle dummy updater case	*)
						block[offset] := GraphFlat.fact.New();
						block[offset].SetValue(0.0)
					END;
					INC(offset);
					INC(k)
				END;
				INC(j)
			END;
			INC(i)
		END;
		RETURN block
	END ParallelBlock;

	PROCEDURE MarkPriors (updaters: UpdaterUpdaters.Vector; mark: SET);
		VAR
			u: UpdaterUpdaters.Updater;
			prior: GraphStochastic.Node;
			i, j, size, numUpdaters: INTEGER;
	BEGIN
		numUpdaters := LEN(updaters);
		j := 0;
		WHILE j < numUpdaters DO
			u := updaters[j];
			size := u.Size();
			i := 0;
			WHILE i < size DO
				prior := u.UpdatedBy(i);
				IF prior # NIL THEN
					prior.SetProps(prior.props + mark)
				END;
				INC(i)
			END;
			INC(j)
		END
	END MarkPriors;

	PROCEDURE UnMarkPriors (updaters: UpdaterUpdaters.Vector; mark: SET);
		VAR
			u: UpdaterUpdaters.Updater;
			prior: GraphStochastic.Node;
			i, j, size, numUpdaters: INTEGER;
	BEGIN
		numUpdaters := LEN(updaters);
		j := 0;
		WHILE j < numUpdaters DO
			u := updaters[j];
			size := u.Size();
			i := 0;
			WHILE i < size DO
				prior := u.UpdatedBy(i);
				IF prior # NIL THEN
					prior.SetProps(prior.props - mark)
				END;
				INC(i)
			END;
			INC(j)
		END
	END UnMarkPriors;

	PROCEDURE ReadGraph* (chain: INTEGER; VAR rd: Stores.Reader);
		VAR
			numNamed, numStoch, maxDepth, maxStochDepth: INTEGER;
		CONST
			nChains = 1;
	BEGIN
		GraphNodes.BeginInternalize(rd);
		rd.ReadInt(numStoch);
		GraphNodes.InternalizePointers(numStoch, rd);
		rd.ReadInt(numNamed);
		GraphNodes.InternalizePointers(numNamed, rd);
		GraphNodes.InternalizeNodeData(rd);
		UpdaterActions.InternalizeUpdaterPointers(nChains, rd);
		GraphNodes.EndInternalize(rd);
		rd.ReadInt(maxDepth);
		rd.ReadInt(maxStochDepth);
		GraphNodes.SetDepth(maxDepth, maxStochDepth);
		UpdaterActions.InternalizeUpdaterData(chain, rd)
	END ReadGraph;

	PROCEDURE MaxSizeParams* (updaters: UpdaterUpdaters.Vector): INTEGER;
		CONST
			eps = 1.0E-3;
		VAR
			i, size, size0, numUpdaters: INTEGER;
			left, right: REAL;
			u: UpdaterUpdaters.Updater;
			prior: GraphStochastic.Node;
	BEGIN
		size := 2; (*	univariate conjugate distributions have two parameters	*)
		numUpdaters := LEN(updaters);
		i := 0;
		WHILE i < numUpdaters DO
			u := updaters[i];
			prior := u.Prior(0);
			IF u IS UpdaterConjugateMV.Updater THEN
				size0 := u(UpdaterConjugateMV.Updater).ParamsSize();
				size := MAX(size, size0)
			ELSIF u IS UpdaterMultivariate.Updater THEN
				size0 := u.Size();
				size := MAX(size, 1 + size0 + size0 * size0)
			ELSIF (prior # NIL) & (GraphStochastic.integer IN prior.props) THEN
				prior.Bounds(left, right);
				size0 := SHORT(ENTIER(right + eps)) + 1;
				size := MAX(size, size0)
			END;
			INC(i)
		END;
		RETURN size
	END MaxSizeParams;

	PROCEDURE MaxBlockSize* (updaters: UpdaterUpdaters.Vector;
	id: POINTER TO ARRAY OF INTEGER): INTEGER;
		VAR
			blockSize, i, numUpdaters, size: INTEGER;
	BEGIN
		blockSize := 0;
		numUpdaters := LEN(updaters);
		i := 0;
		WHILE i < numUpdaters DO
			size := updaters[i].Size();
			blockSize := MAX(blockSize, ABS(id[i]) * size);
			INC(i)
		END;
		RETURN blockSize
	END MaxBlockSize;

	PROCEDURE ModifyUpdaters* (commSize, rank, chain: INTEGER;
	OUT updaters: POINTER TO ARRAY OF UpdaterUpdaters.Vector;
	OUT id: POINTER TO ARRAY OF INTEGER;
	OUT observations: POINTER TO ARRAY OF GraphStochastic.Vector);
		VAR
			p: GraphStochastic.Node;
			i, num: INTEGER;
	BEGIN
		UpdaterActions.StoreStochastics;
		DistributeUpdaters(commSize, chain, updaters, id);
		DistributeObservations(updaters, id, observations);
		ThinUpdaters(updaters[rank], commSize, rank);
		(*	only keep likelihoods for nodes being updated on this core	*)
		MarkPriors(updaters[rank], {GraphNodes.mark});
		i := 0;
		num := LEN(GraphStochastic.stochastics);
		WHILE i < num DO
			p := GraphStochastic.stochastics[i];
			IF ~(GraphNodes.mark IN p.props) THEN
				p.SetLikelihood(NIL)
			END;
			INC(i)
		END;
		UnMarkPriors(updaters[rank], {GraphNodes.mark})
	END ModifyUpdaters;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		colPtr := NIL;
		rowInd := NIL
	END Init;

BEGIN
	Init
END UpdaterParallel.

