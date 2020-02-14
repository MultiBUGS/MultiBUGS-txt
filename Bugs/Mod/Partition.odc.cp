(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)
MODULE BugsPartition;

	IMPORT
		GraphConjugateMV, GraphDeviance, GraphLogical, GraphMRF, GraphNodes, GraphStochastic,
		UpdaterActions, UpdaterUpdaters;

	TYPE
		List = POINTER TO RECORD
			updater: INTEGER;
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

	CONST
		undefined = MAX(INTEGER);

	PROCEDURE IsMarkedLike (updater: UpdaterUpdaters.Updater): BOOLEAN;
		VAR
			isMarked: BOOLEAN;
			children: GraphStochastic.Vector;
			i, num: INTEGER;
	BEGIN
		isMarked := FALSE;
		children := updater.Children();
		IF children # NIL THEN
			num := LEN(children);
			i := 0;
			WHILE ~isMarked & (i < num) DO
				isMarked := GraphStochastic.mark IN children[i].props;
				INC(i)
			END
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
		IF children # NIL THEN
			num := LEN(children);
			i := 0;
			WHILE i < num DO
				node := children[i];
				INCL(node.props, GraphStochastic.mark);
				INC(i)
			END
		END
	END MarkLike;

	PROCEDURE UnmarkLike (updater: UpdaterUpdaters.Updater);
		VAR
			children: GraphStochastic.Vector;
			node: GraphStochastic.Node;
			i, num: INTEGER;
	BEGIN
		children := updater.Children();
		IF children # NIL THEN
			num := LEN(children);
			i := 0;
			WHILE i < num DO
				node := children[i];
				EXCL(node.props, GraphStochastic.mark);
				INC(i)
			END
		END
	END UnmarkLike;

	PROCEDURE IsMarkedDependents (updater: UpdaterUpdaters.Updater): BOOLEAN;
		VAR
			i, j, num, size: INTEGER;
			prior: GraphStochastic.Node;
			dependents: GraphLogical.Vector;
			logical: GraphLogical.Node;
			isMarked: BOOLEAN;
	BEGIN
		isMarked := FALSE;
		size := updater.Size();
		i := 0;
		WHILE (i < size) & ~isMarked DO
			prior := updater.Node(i);
			dependents := prior.dependents;
			IF dependents # NIL THEN
				num := LEN(dependents);
				j := 0;
				WHILE (j < num) & ~isMarked DO
					logical := dependents[j];
					isMarked := GraphLogical.mark IN logical.props;
					INC(j)
				END
			END;
			INC(i)
		END;
		RETURN isMarked
	END IsMarkedDependents;

	PROCEDURE MarkDependents (updater: UpdaterUpdaters.Updater);
		VAR
			i, j, num, size: INTEGER;
			prior: GraphStochastic.Node;
			dependents: GraphLogical.Vector;
			logical: GraphLogical.Node;
	BEGIN
		size := updater.Size();
		i := 0;
		WHILE i < size DO
			prior := updater.Node(i);
			dependents := prior.dependents;
			IF dependents # NIL THEN
				num := LEN(dependents);
				j := 0;
				WHILE j < num DO
					logical := dependents[j];
					IF ~(GraphLogical.prediction IN logical.props) THEN
						INCL(logical.props, GraphLogical.mark)
					END;
					INC(j)
				END
			END;
			INC(i)
		END
	END MarkDependents;

	PROCEDURE UnmarkDependents (updater: UpdaterUpdaters.Updater);
		VAR
			i, j, num, size: INTEGER;
			prior: GraphStochastic.Node;
			dependents: GraphLogical.Vector;
			logical: GraphLogical.Node;
	BEGIN
		size := updater.Size();
		i := 0;
		WHILE i < size DO
			prior := updater.Node(i);
			dependents := prior.dependents;
			IF dependents # NIL THEN
				num := LEN(dependents);
				j := 0;
				WHILE j < num DO
					logical := dependents[j];
					EXCL(logical.props, GraphLogical.mark);
					INC(j)
				END
			END;
			INC(i)
		END
	END UnmarkDependents;

	PROCEDURE IsMarkedNeighbour (updater: UpdaterUpdaters.Updater): BOOLEAN;
		VAR
			i, index, j, size: INTEGER;
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
		|prior: GraphConjugateMV.Node DO
			i := 0;
			size := prior.Size();
			WHILE ~isMarked & (i < size) DO
				isMarked := GraphStochastic.mark IN prior.components[i].props;
				INC(i)
			END
		ELSE
		END;
		RETURN isMarked
	END IsMarkedNeighbour;

	PROCEDURE MarkNeighbour (updater: UpdaterUpdaters.Updater);
		VAR
			prior, p: GraphStochastic.Node;
			i, size: INTEGER;
	BEGIN
		prior := updater.Prior(0);
		WITH prior: GraphMRF.Node DO
			INCL(prior.props, GraphStochastic.mark)
		|prior: GraphConjugateMV.Node DO
			i := 0;
			size := prior.Size();
			WHILE i < size DO
				p := prior.components[i];
				INCL(p.props, GraphStochastic.mark);
				INC(i)
			END
		ELSE
		END
	END MarkNeighbour;

	PROCEDURE UnmarkNeighbour (updater: UpdaterUpdaters.Updater);
		VAR
			prior, p: GraphStochastic.Node;
			i, size: INTEGER;
	BEGIN
		prior := updater.Prior(0);
		WITH prior: GraphMRF.Node DO
			EXCL(prior.props, GraphStochastic.mark)
		|prior: GraphConjugateMV.Node DO
			i := 0;
			size := prior.Size();
			WHILE i < size DO
				p := prior.components[i];
				EXCL(p.props, GraphStochastic.mark);
				INC(i)
			END
		ELSE
		END
	END UnmarkNeighbour;

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

	PROCEDURE Unmark (u: UpdaterUpdaters.Updater);
	BEGIN
		UnmarkLike(u); UnmarkDependents(u); UnmarkNeighbour(u)
	END Unmark;

	(*	distribute fixed effects	*)
	PROCEDURE FixedEffects (depth: INTEGER; VAR lists: ARRAY OF List; VAR rowId: RowId);
		VAR
			finish, i, j, numCores, start: INTEGER;
			updater: UpdaterUpdaters.Updater;
			prior: GraphStochastic.Node;
			element: RowId;
			link: List;
	BEGIN
		UpdaterActions.StartFinish(depth, start, finish);
		numCores := LEN(lists);
		i := start;
		WHILE i # finish DO
			updater := UpdaterActions.updaters[0, i];
			prior := updater.Prior(0);
			(*	distributed updater 	*)
			IF GraphStochastic.distributed IN prior.props THEN
				j := 0;
				WHILE j < numCores DO
					NEW(link); link.next := lists[j]; lists[j] := link; INC(j)
				END;
				NEW(element); element.id := 0; element.next := rowId; rowId := element;
				(*	add new row of updaters	*)
				j := 0;
				WHILE j < numCores DO
					lists[j].updater := i;
					INC(j)
				END
			END;
			INC(i)
		END
	END FixedEffects;

	(*	distribute un-structured random effects	*)
	PROCEDURE RandomEffects (depth: INTEGER;
	VAR lists: ARRAY OF List; VAR rowId: RowId; OUT new: BOOLEAN);
		VAR
			i, j, finish, rank, numCores, numRows, size, start: INTEGER;
			u: UpdaterUpdaters.Updater;
			prior: GraphStochastic.Node;
			link: List;
			element: RowId;
	BEGIN
		new := FALSE;
		UpdaterActions.StartFinish(depth, start, finish);
		numCores := LEN(lists);
		rank := 0;
		numRows := 1;
		i := start;
		WHILE i # finish DO
			u := UpdaterActions.updaters[0, i];
			prior := u.Node(0);
			IF ({GraphStochastic.distributed, GraphNodes.mark,
				GraphStochastic.censored} * prior.props = {}) THEN
				(*	potential parallel updater	*)
				IF ~(prior IS GraphMRF.Node) & ~IsMarked(u) THEN
					(*	likelihoods disjoint etc	*)
					IF rank = 0 THEN	(*	add new row of updaters	*)
						j := 0;
						WHILE j < numCores DO
							NEW(link); link.updater := undefined; link.next := lists[j]; lists[j] := link; INC(j)
						END;
						NEW(element); element.id := numRows; element.next := rowId; rowId := element;
						INC(numRows);
						size := u.Size()
					END;
					IF size = u.Size() THEN	(*	check size of potential new updater	*)
						INCL(prior.props, GraphNodes.mark);
						new := TRUE;
						Mark(u);
						lists[rank].updater := i;
						INC(rank);
						rank := rank MOD numCores
					END
				END
			END;
			INC(i)
		END;
		i := start;
		WHILE i # finish DO
			u := UpdaterActions.updaters[0, i];
			Unmark(u);
			INC(i)
		END;
		IF rowId # NIL THEN rowId.id := - ABS(rowId.id) END
	END RandomEffects;

	(*	finds markov random field node in graph provided they are not markes as distributed	*)
	PROCEDURE FindMRF (depth: INTEGER; OUT mrfList: List; OUT constraint: INTEGER);
		VAR
			gmrf: GraphMRF.Node;
			list, element, cursor: List;
			i, start, finish, size, type, nElements, numNeigh: INTEGER;
			prior: GraphStochastic.Node;
			u: UpdaterUpdaters.Updater;
	BEGIN
		list := NIL;
		gmrf := NIL;
		constraint := undefined;
		numNeigh := 0;
		UpdaterActions.StartFinish(depth, start, finish);
		i := start;
		WHILE (i # finish) & (gmrf = NIL) DO
			u := UpdaterActions.updaters[0, i];
			prior := u.Node(0);
			WITH prior: GraphMRF.Node DO
				IF ~(GraphNodes.mark IN prior.props) & ~(GraphStochastic.distributed IN prior.props) THEN
					numNeigh := prior.NumberNeighbours();
					gmrf := prior.components[0](GraphMRF.Node);
					gmrf.MatrixInfo(type, nElements);
					size := prior.Size();
					NEW(rowInd, nElements);
					NEW(colPtr, size + 1);
					gmrf.MatrixMap(rowInd, colPtr);
					NEW(element);
					element.updater := i;
					element.next := list;
					list := element
				END
			ELSE
			END;
			INC(i)
		END;
		(*	add new gmrf nodes to list	*)
		WHILE i # finish DO
			u := UpdaterActions.updaters[0, i];
			prior := u.Node(0);
			WITH prior: GraphMRF.Node DO
				IF ~(GraphNodes.mark IN prior.props) & ~(GraphStochastic.distributed IN prior.props)
					 & (prior.components[0] = gmrf) THEN
					IF u.Size() # 0 THEN
						numNeigh := MAX(numNeigh, prior.NumberNeighbours());
						NEW(element);
						element.updater := i;
						element.next := list;
						list := element
					ELSE
						constraint := i
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
				u := UpdaterActions.updaters[0, cursor.updater];
				prior := u.Node(0);
				gmrf := prior(GraphMRF.Node);
				IF gmrf.NumberNeighbours() = i THEN
					NEW(element);
					element.updater := cursor.updater;
					element.next := mrfList;
					mrfList := element
				END;
				cursor := cursor.next
			END;
			INC(i)
		END
	END FindMRF;

	(*	distribute structured random effects	*)
	PROCEDURE RandomEffectsMRF (mrfList: List;
	VAR lists: ARRAY OF List; VAR rowId: RowId; OUT lastColour: BOOLEAN);
		VAR
			j, rank, numCores, numRows: INTEGER;
			u: UpdaterUpdaters.Updater;
			prior: GraphStochastic.Node;
			link, cursor: List;
			element: RowId;
	BEGIN
		numCores := LEN(lists);
		rank := 0;
		numRows := 1;
		cursor := mrfList;
		WHILE cursor # NIL DO
			u := UpdaterActions.updaters[0, cursor.updater];
			prior := u.Node(0);
			IF ~(GraphNodes.mark IN prior.props) & ~IsMarked(u) THEN
				IF rank = 0 THEN	(*	add new row of updaters	*)
					j := 0;
					WHILE j < numCores DO
						NEW(link); link.updater := undefined; link.next := lists[j]; lists[j] := link; INC(j)
					END;
					NEW(element); element.id := numRows; element.next := rowId; rowId := element;
					INC(numRows);
				END;
				INCL(prior.props, GraphNodes.mark);
				Mark(u);
				lists[rank].updater := cursor.updater;
				INC(rank);
				rank := rank MOD numCores
			END;
			cursor := cursor.next
		END;
		cursor := mrfList;
		lastColour := TRUE;
		WHILE cursor # NIL DO
			u := UpdaterActions.updaters[0, cursor.updater];
			Unmark(u);
			prior := u.Node(0);
			IF ~(GraphNodes.mark IN prior.props) THEN lastColour := FALSE END;
			cursor := cursor.next
		END;
		rowId.id := - ABS(rowId.id)
	END RandomEffectsMRF;

	PROCEDURE AddConstraint (constraint: INTEGER; VAR lists: ARRAY OF List; VAR rowId: RowId);
		VAR
			j, numCores: INTEGER;
			element: RowId;
			link: List;
	BEGIN
		numCores := LEN(lists);
		j := 0;
		WHILE j < numCores DO
			NEW(link); link.next := lists[j]; lists[j] := link; INC(j)
		END;
		NEW(element); element.id := 0; element.next := rowId; rowId := element;
		(*	add new row of updaters	*)
		j := 0;
		WHILE j < numCores DO
			lists[j].updater := constraint;
			INC(j)
		END
	END AddConstraint;

	PROCEDURE DistributeObservations* (IN updaters: ARRAY OF ARRAY OF INTEGER):
	POINTER TO ARRAY OF GraphStochastic.Vector;
		VAR
			i, j, k, numChild, numUpdaters, workersPerChain, label, dataForFE, div, mod: INTEGER;
			children: GraphStochastic.Vector;
			child: GraphStochastic.Node;
			numData: POINTER TO ARRAY OF INTEGER;
			distributed: BOOLEAN;
			u: UpdaterUpdaters.Updater;
			p: GraphStochastic.Node;
			observations: POINTER TO ARRAY OF GraphStochastic.Vector;

		CONST
			count = 0;
			add = 1;

		PROCEDURE ClearMarks;
		BEGIN
			i := 0;
			WHILE i < numUpdaters DO
				j := 0;
				WHILE j < workersPerChain DO
					label := updaters[j, i];
					IF label # undefined THEN
						u := UpdaterActions.updaters[0, label];
						children := u.Children();
						GraphStochastic.ClearMarks(children, {GraphNodes.mark})
					END;
					INC(j)
				END;
				INC(i)
			END
		END ClearMarks;

		(*	do in two stages first non distributed then second distributed nodes	*)
		PROCEDURE DevianceDo (action: INTEGER);
		BEGIN
			i := 0;
			WHILE i < numUpdaters DO
				j := 0;
				label := updaters[0, i];
				u := UpdaterActions.updaters[0, label];
				p := u.Node(0);
				distributed := GraphStochastic.distributed IN p.props;
				IF ~distributed THEN
					WHILE j < workersPerChain DO
						label := updaters[j, i];
						IF label # undefined THEN
							u := UpdaterActions.updaters[0, label];
							children := u.Children();
							IF children # NIL THEN
								numChild := LEN(children);
								k := 0;
								WHILE k < numChild DO
									child := children[k];
									IF GraphDeviance.IsObserved(child) THEN
										IF ~(GraphNodes.mark IN child.props) THEN
											INCL(child.props, GraphNodes.mark);
											IF action = add THEN observations[j, numData[j]] := child END;
											INC(numData[j])
										END
									END;
									INC(k)
								END
							END
						END;
						INC(j)
					END
				END;
				INC(i)
			END;
			i := 0;
			WHILE i < numUpdaters DO
				j := 0;
				label := updaters[0, i];
				u := UpdaterActions.updaters[0, label];
				p := u.Node(0);
				distributed := GraphStochastic.distributed IN p.props;
				IF distributed THEN
					children := u.Children();
					numChild := 0;
					IF children # NIL THEN
						numChild := LEN(children);
						k := 0;
						dataForFE := 0;
						WHILE k < numChild DO
							child := children[k];
							IF GraphDeviance.IsObserved(child) THEN
								IF ~(GraphNodes.mark IN child.props) THEN
									INC(dataForFE)
								END;
							END;
							INC(k)
						END
					END;
					mod := dataForFE MOD workersPerChain;
					j := 0;
					WHILE j < workersPerChain DO
						div := dataForFE DIV workersPerChain;
						IF mod > 0 THEN INC(div) END;
						DEC(mod);
						k := 0;
						WHILE (k < numChild) & (div > 0) DO
							child := children[k];
							IF GraphDeviance.IsObserved(child) THEN
								IF ~(GraphNodes.mark IN child.props) THEN
									INCL(child.props, GraphNodes.mark);
									IF action = add THEN observations[j, numData[j]] := child END;
									INC(numData[j]);
									DEC(div)
								END
							END;
							INC(k);
						END;
						INC(j)
					END;
				END;
				INC(i)
			END;
		END DevianceDo;

	BEGIN
		workersPerChain := LEN(updaters, 0);
		numUpdaters := LEN(updaters, 1);
		NEW(observations, workersPerChain);
		NEW(numData, workersPerChain);
		j := 0;
		WHILE j < workersPerChain DO numData[j] := 0; INC(j) END;
		DevianceDo(count); ClearMarks;
		j := 0;
		WHILE j < workersPerChain DO
			IF numData[j] > 0 THEN
				NEW(observations[j], numData[j]);
				numData[j] := 0
			ELSE
				observations[j] := NIL
			END;
			INC(j)
		END;
		DevianceDo(add); ClearMarks;
		RETURN observations
	END DistributeObservations;

	PROCEDURE DistributeUpdaters* (workersPerChain: INTEGER;
	OUT updaters: POINTER TO ARRAY OF ARRAY OF INTEGER;
	OUT id: POINTER TO ARRAY OF INTEGER);
		VAR
			i, j, depth, maxDepth, minDepth, numUpdaters: INTEGER;
			cursor, element, list, mrfList: List;
			lists: POINTER TO ARRAY OF List;
			rowId: RowId;
			prior: GraphStochastic.Node;
			constraint: INTEGER;
			lastColour, new: BOOLEAN;
			u: UpdaterUpdaters.Updater;
	BEGIN
		rowId := NIL;
		NEW(lists, workersPerChain);
		UpdaterActions.MinMaxDepth(minDepth, maxDepth);
		i := 0;
		WHILE i < workersPerChain DO
			lists[i] := NIL;
			INC(i)
		END;
		UpdaterActions.MarkDistributed(workersPerChain);
		depth := maxDepth;
		WHILE depth >= minDepth DO
			FixedEffects(depth, lists, rowId);
			(*	do structured random effects	*)
			FindMRF(depth, mrfList, constraint);
			WHILE mrfList # NIL DO
				REPEAT
					RandomEffectsMRF(mrfList, lists, rowId, lastColour);
				UNTIL lastColour;
				IF constraint # undefined THEN
					AddConstraint(constraint, lists, rowId)
				END;
				FindMRF(depth, mrfList, constraint)
			END;
			(*	do un-structured random effects	*)
			REPEAT
				RandomEffects(depth, lists, rowId, new);
			UNTIL ~new;
			DEC(depth)
		END;
		i := 0;
		WHILE i < workersPerChain DO
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
		cursor := lists[0];
		numUpdaters := 0;
		WHILE cursor # NIL DO
			INC(numUpdaters);
			cursor := cursor.next
		END;
		NEW(updaters, workersPerChain, numUpdaters);
		NEW(id, numUpdaters);
		j := 0;
		WHILE j < numUpdaters DO
			i := 0;
			WHILE i < workersPerChain DO
				updaters[i, j] := lists[i].updater;
				IF updaters[i, j] # undefined THEN
					u := UpdaterActions.updaters[0, updaters[i, j]];
					prior := u.Node(0);
					EXCL(prior.props, GraphNodes.mark)
				END;
				INC(i)
			END;
			i := 0;
			WHILE i < workersPerChain DO
				lists[i] := lists[i].next;
				INC(i)
			END;
			INC(j);
			id[numUpdaters - j] := rowId.id;
			rowId := rowId.next
		END;
		colPtr := NIL;
		rowInd := NIL
	END DistributeUpdaters;

	(*	the updater for a censored observation is placed on the same worker as the censored observation	*)
	PROCEDURE DistributeCensored* (observations: POINTER TO ARRAY OF GraphStochastic.Vector;
	VAR updaters: POINTER TO ARRAY OF ARRAY OF INTEGER;
	VAR id: POINTER TO ARRAY OF INTEGER);
		VAR
			i, j, k, max, num, numObs, workersPerChain, numUpdaters, oldSize: INTEGER;
			updater: UpdaterUpdaters.Updater;
			prior: GraphStochastic.Node;
			newUpdaters: POINTER TO ARRAY OF ARRAY OF INTEGER;
			newId: POINTER TO ARRAY OF INTEGER;
	BEGIN
		(*	count number of censored observations on each worker	*)
		workersPerChain := LEN(observations);
		max := 0;
		i := 0;
		WHILE i < workersPerChain DO
			j := 0;
			num := 0;
			IF observations[i] # NIL THEN numObs := LEN(observations[i]) ELSE numObs := 0 END;
			WHILE j < numObs DO
				IF GraphStochastic.censored IN observations[i, j].props THEN INC(num) END;
				INC(j)
			END;
			max := MAX(max, num);
			INC(i)
		END;
		(*	place updater for censored observation after other updaters	*)
		IF max > 0 THEN
			oldSize := LEN(updaters, 1);
			NEW(newUpdaters, workersPerChain, oldSize + max);
			NEW(newId, oldSize + max);
			j := 0; WHILE j < oldSize DO newId[j] := id[j]; INC(j) END;
			j := 0; WHILE j < max DO newId[oldSize + j] := j + 1; INC(j) END;
			newId[oldSize + max - 1] := - ABS(newId[oldSize + max - 1]);
			i := 0;
			WHILE i < workersPerChain DO
				j := 0;
				WHILE j < oldSize DO
					newUpdaters[i, j] := updaters[i, j];
					INC(j)
				END;
				j := 0;
				WHILE j < max DO
					newUpdaters[i, oldSize + j] := undefined;
					INC(j)
				END;
				INC(i)
			END;
			numUpdaters := UpdaterActions.NumberUpdaters();
			i := 0;
			WHILE i < workersPerChain DO
				(*	mark censored observations on each worker	*)
				j := 0;
				IF observations[i] # NIL THEN numObs := LEN(observations[i]) ELSE numObs := 0 END;
				WHILE j < numObs DO
					IF GraphStochastic.censored IN observations[i, j].props THEN
						INCL(observations[i, j].props, GraphStochastic.mark)
					END;
					INC(j)
				END;
				j := 0;
				k := oldSize;
				(*	find updater for marked node (which is a censored observation)	*)
				WHILE j < numUpdaters DO
					updater := UpdaterActions.updaters[0, j];
					prior := updater.Prior(0);
					IF GraphStochastic.mark IN prior.props THEN
						newUpdaters[i, k] := j;
						INC(k);
						EXCL(prior.props, GraphStochastic.mark)
					END;
					INC(j)
				END;
				INC(i)
			END;
			updaters := newUpdaters;
			id := newId
		END
	END DistributeCensored;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		colPtr := NIL;
		rowInd := NIL;
	END Init;

BEGIN
	Init
END BugsPartition.

