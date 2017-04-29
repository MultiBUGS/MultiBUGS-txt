(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)

MODULE UpdaterParallel;

	IMPORT 
		Stores,
		GraphFlat, GraphLogical, GraphMultivariate, GraphNodes, GraphStochastic,
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

	PROCEDURE MarkChildren (children: GraphStochastic.Vector; numProc, rank: INTEGER);
		VAR
			node: GraphStochastic.Node;
			i, num: INTEGER;
	BEGIN
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := rank;
		WHILE i < num DO
			node := children[i];
			node.SetProps(node.props + {GraphStochastic.blockMark});
			INC(i, numProc);
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
			p.SetProps(p.props - {GraphStochastic.blockMark});
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
			IF GraphStochastic.blockMark IN children[i].props THEN INC(len) END;
			INC(i)
		END;
		IF len > 0 THEN NEW(vector, len) ELSE vector := NIL END;
		i := 0;
		j := 0;
		WHILE i < num DO
			IF GraphStochastic.blockMark IN children[i].props THEN
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
			isMarked := GraphNodes.mark IN children[i].props;
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
			node.SetProps(node.props + {GraphNodes.mark});
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
			node.SetProps(node.props - {GraphNodes.mark});
			INC(i)
		END
	END UnMarkLike;

	PROCEDURE IsMarkedDependent (updater: UpdaterUpdaters.Updater): BOOLEAN;
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
				isMarked :=  GraphNodes.mark IN logical.props;
				list := list.next
			END;
			INC(i)
		END;
		RETURN isMarked
	END IsMarkedDependent;
	
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
				logical.SetProps(logical.props + {GraphStochastic.blockMark});
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
				logical.SetProps(logical.props + {GraphStochastic.blockMark});
				list := list.next
			END;
			INC(i)
		END
	END UnMarkDependents;

	(*	distribute fixed effects	*)
	PROCEDURE DistByDepth0 (depth, chain: INTEGER; VAR lists: ARRAY OF List; VAR rowId: RowId);
		VAR
			new: BOOLEAN;
			finish, i, j, commSize, numRows, start: INTEGER;
			updater: UpdaterUpdaters.Updater;
			prior: GraphStochastic.Node;
			element: RowId;
			link: List;
	BEGIN
		UpdaterActions.StartFinish(depth, start, finish);
		commSize := LEN(lists);
		REPEAT	(*	repeat until all updaters at depth have been distributed	*)
			new := FALSE;
			i := start;
			WHILE i # finish DO
				updater := UpdaterActions.GetUpdater(chain, i);
				prior := updater.Prior(0);
				IF GraphStochastic.distributed IN prior.props THEN	(*	distributed updater	*)
					IF ~(GraphNodes.mark IN prior.props) THEN	(*	not distributed this updater yet	*)
						prior.SetProps(prior.props + {GraphNodes.mark});
						j := 0;
						WHILE j < commSize DO
							NEW(link); link.next := lists[j]; lists[j] := link; INC(j)
						END;
						new := TRUE;
						numRows := 0;
						NEW(element); element.id := numRows; element.next := rowId; rowId := element;
						(*	add new row of updaters	*)
						j := 0;
						WHILE j < commSize DO
							lists[j].updater := updater;
							INC(j)
						END
					END
				END;
				INC(i);
			END;
		UNTIL ~new;
		i := start;
		WHILE i # finish DO
			updater := UpdaterActions.GetUpdater(chain, i);
			prior := updater.Prior(0);
			prior.SetProps(prior.props - {GraphNodes.mark});
			INC(i)
		END
	END DistByDepth0;

	(*	distribute random effects	*)
	PROCEDURE DistByDepth1 (depth, chain: INTEGER; VAR lists: ARRAY OF List; VAR rowId: RowId);
		VAR
			new: BOOLEAN;
			i, j, finish, rank, commSize, numRows, size, start: INTEGER;
			updater: UpdaterUpdaters.Updater;
			prior: GraphStochastic.Node;
			link: List;
			element: RowId;
	BEGIN
		UpdaterActions.StartFinish(depth, start, finish);
		commSize := LEN(lists);
		REPEAT	(*	repeat until all updaters at depth have been distributed	*)
			new := FALSE;
			rank := 0;
			numRows := 1;
			i := start;
			WHILE i # finish DO
				updater := UpdaterActions.GetUpdater(chain, i);
				prior := updater.Prior(0);
				IF ~(GraphStochastic.distributed IN prior.props) THEN	(*	not distributed updater	*)
					IF ~(GraphNodes.mark IN prior.props) THEN	(*	not distributed this updater yet	*)
						(*	potential parallel updater	*)
						IF ~IsMarkedLike(updater) & ~IsMarkedDependent(updater) THEN	(*	likelihoods disjoint	*)
							IF rank = 0 THEN	(*	add new row of updaters	*)
								j := 0;
								WHILE j < commSize DO
									NEW(link); link.next := lists[j]; lists[j] := link; INC(j)
								END;
								new := TRUE;
								NEW(element); element.id := numRows; element.next := rowId; rowId := element;
								INC(numRows);
								size := updater.Size()
							END;
							IF size = updater.Size() THEN	(*	check size of potential new updater	*)
								prior.SetProps(prior.props + {GraphNodes.mark});
								MarkLike(updater);
								MarkDependents(updater);
								lists[rank].updater := updater;
								INC(rank);
								rank := rank MOD commSize
							END
						END
					END;
				END;
				INC(i)
			END;
			i := start;
			WHILE i # finish DO
				updater := UpdaterActions.GetUpdater(chain, i);
				UnMarkLike(updater);
				UnMarkDependents(updater);
				INC(i)
			END;
			IF new THEN rowId.id :=  - ABS(rowId.id) END
		UNTIL ~new;
		i := start;
		WHILE i # finish DO
			updater := UpdaterActions.GetUpdater(chain, i);
			prior := updater.Prior(0);
			prior.SetProps(prior.props - {GraphNodes.mark});
			INC(i)
		END
	END DistByDepth1;

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

		PROCEDURE IsObserved (stochastic: GraphStochastic.Node): BOOLEAN;
			CONST
				observed = {GraphNodes.data, GraphStochastic.censored};
			VAR
				i, size: INTEGER;
				isObserved: BOOLEAN;
				multi: GraphMultivariate.Node;
		BEGIN
			isObserved := observed * stochastic.props # {};
			IF ~isObserved & (stochastic IS GraphMultivariate.Node) THEN
				multi := stochastic(GraphMultivariate.Node);
				i := 0;
				size := stochastic.Size();
				WHILE ~isObserved & (i < size) DO
					IF multi.components # NIL THEN
						isObserved := observed * multi.components[i].props # {}
					END;
					INC(i)
				END
			END;
			RETURN isObserved
		END IsObserved;

		PROCEDURE ClearMarks;
		BEGIN
			i := 0;
			WHILE i < numUpdaters DO
				j := 0;
				WHILE j < commSize DO
					children := updaters[j, i].Children();
					GraphStochastic.ClearMark(children, {GraphNodes.mark});
					INC(j)
				END;
				INC(i)
			END
		END ClearMarks;

		PROCEDURE Deviance (add: BOOLEAN);
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
							IF IsObserved(child) THEN
								IF ~(GraphNodes.mark IN child.props) THEN
									child.SetProps(child.props + {GraphNodes.mark});
									IF add THEN observations[j, numData[j]] := child END;
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
		END Deviance;

	BEGIN
		numUpdaters := LEN(updaters[0]);
		commSize := LEN(updaters);
		NEW(observations, commSize);
		NEW(numData, commSize);
		WHILE j < commSize DO numData[j] := 0; INC(j) END;
		Deviance(FALSE);
		j := 0;
		WHILE j < commSize DO
			IF numData[j] > 0 THEN NEW(observations[j], numData[j]) END;
			numData[j] := 0;
			INC(j)
		END;
		ClearMarks;
		Deviance(TRUE);
		ClearMarks
	END DistributeObservations;

	PROCEDURE DistributeUpdaters* (commSize, chain: INTEGER;
	OUT updaters: POINTER TO ARRAY OF UpdaterUpdaters.Vector;
	OUT id: POINTER TO ARRAY OF INTEGER);
		VAR
			i, j, maxDepth, minDepth, numUpdaters: INTEGER;
			cursor, element, list: List;
			lists: POINTER TO ARRAY OF List;
			rowId: RowId;
			prior: GraphStochastic.Node;
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
		i := maxDepth;
		WHILE i >= minDepth DO
			DistByDepth0(i, chain, lists, rowId);
			DistByDepth1(i, chain, lists, rowId);
			DEC(i)
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
		rowId.id :=  - ABS(rowId.id);
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
		(*	if only one updater in parallel block distribute updater	*)
		IF commSize > 1 THEN
			j := 0;
			WHILE j < numUpdaters DO
				IF id[j] =  - 1 THEN
					id[j] := 0;
					prior := updaters[1, j].Prior(0);
					IF prior = NIL THEN
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
		END
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
					block[offset] := u.Prior(k);
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
				prior := u.Prior(i);
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
				prior := u.Prior(i);
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
	END Init;

BEGIN
	Init
END UpdaterParallel.

