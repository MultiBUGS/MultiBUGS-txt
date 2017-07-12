(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)
MODULE BugsParallel;

	IMPORT
		Stores,
		BugsIndex, BugsNames,
		GraphDeviance, GraphDummy, GraphLogical, GraphMRF, GraphNodes, GraphStochastic,
		UpdaterActions, UpdaterConjugateMV, UpdaterEmpty, UpdaterMultivariate, UpdaterUpdaters;

	TYPE
		List = POINTER TO RECORD
			updater: INTEGER;
			next: List
		END;

		RowId = POINTER TO RECORD
			id: INTEGER;
			next: RowId
		END;

		Marker = POINTER TO RECORD(BugsNames.ElementVisitor)
			mark: BOOLEAN
		END;

		Writer = POINTER TO RECORD(BugsNames.ElementVisitor)
			wr: Stores.Writer;
			data: BOOLEAN
		END;

		Counter = POINTER TO RECORD(BugsNames.ElementVisitor)
			num: INTEGER
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		colPtr, rowInd: POINTER TO ARRAY OF INTEGER;
		globalStochs: POINTER TO ARRAY OF GraphStochastic.Vector;
		globalUpdaters: POINTER TO ARRAY OF ARRAY OF INTEGER;
		globalId: POINTER TO ARRAY OF INTEGER;
		placeHolder: GraphStochastic.Node;

	CONST
		undefined = MAX(INTEGER);
		placeHolderType = 0;

	PROCEDURE (v: Marker) Do (name: BugsNames.Name);
		VAR
			p: GraphNodes.Node;
	BEGIN
		p := name.components[v.index];
		IF v.mark THEN
			p.SetProps(p.props + {GraphStochastic.mark})
		ELSE
			p.SetProps(p.props - {GraphStochastic.mark})
		END
	END Do;

	PROCEDURE (v: Writer) Do (name: BugsNames.Name);
		VAR
			p: GraphNodes.Node;
	BEGIN
		p := name.components[v.index];
		IF (p # NIL) & ~(GraphStochastic.mark IN p.props) THEN
			IF ~(p IS GraphStochastic.Node) OR (GraphNodes.data IN p.props) THEN
				IF ~v.data THEN
					GraphNodes.ExternalizePointer(p, v.wr)
				ELSE
					p.Externalize(v.wr)
				END
			END
		END
	END Do;

	PROCEDURE (v: Counter) Do (name: BugsNames.Name);
		VAR
			p: GraphNodes.Node;
	BEGIN
		p := name.components[v.index];
		IF (p # NIL) & ~(GraphStochastic.mark IN p.props) THEN
			IF ~(p IS GraphStochastic.Node) OR (GraphNodes.data IN p.props) THEN
				INC(v.num)
			END
		END
	END Do;

	PROCEDURE MarkChildren (children: GraphStochastic.Vector; numProc, rank: INTEGER);
		VAR
			node: GraphStochastic.Node;
			j, num: INTEGER;
	BEGIN
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		j := rank;
		WHILE j < num DO
			IF j < num THEN
				node := children[j];
				node.SetProps(node.props + {GraphNodes.mark});
			END;
			INC(j, numProc)
		END
	END MarkChildren;

	PROCEDURE UnmarkChildren (children: GraphStochastic.Vector);
		VAR
			i, num: INTEGER;
			p: GraphStochastic.Node;
	BEGIN
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		WHILE i < num DO
			p := children[i];
			p.SetProps(p.props - {GraphNodes.mark});
			INC(i)
		END
	END UnmarkChildren;

	PROCEDURE ThinChildren (children: GraphStochastic.Vector): GraphStochastic.Vector;
		VAR
			i, num, len, j: INTEGER;
			vector: GraphStochastic.Vector;
	BEGIN
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		len := 0;
		WHILE i < num DO
			IF GraphNodes.mark IN children[i].props THEN INC(len) END;
			INC(i)
		END;
		IF len > 0 THEN NEW(vector, len) ELSE vector := NIL END;
		i := 0;
		j := 0;
		WHILE i < num DO
			IF GraphNodes.mark IN children[i].props THEN
				vector[j] := children[i];
				INC(j)
			END;
			INC(i)
		END;
		RETURN vector
	END ThinChildren;

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

	PROCEDURE UnmarkLike (updater: UpdaterUpdaters.Updater);
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
	END UnmarkLike;

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
			prior := updater.Node(i);
			list := prior.dependents;
			WHILE (list # NIL) & ~isMarked DO
				logical := list.node;
				isMarked := GraphLogical.mark IN logical.props;
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
			prior := updater.Node(i);
			list := prior.dependents;
			WHILE list # NIL DO
				logical := list.node;
				IF GraphLogical.stochParent IN logical.props THEN
					logical.SetProps(logical.props + {GraphLogical.mark})
				END;
				list := list.next
			END;
			INC(i)
		END
	END MarkDependents;

	PROCEDURE UnmarkDependents (updater: UpdaterUpdaters.Updater);
		VAR
			i, size: INTEGER;
			prior: GraphStochastic.Node;
			list: GraphLogical.List;
			logical: GraphLogical.Node;
	BEGIN
		size := updater.Size();
		i := 0;
		WHILE i < size DO
			prior := updater.Node(i);
			list := prior.dependents;
			WHILE list # NIL DO
				logical := list.node;
				logical.SetProps(logical.props - {GraphLogical.mark});
				list := list.next
			END;
			INC(i)
		END
	END UnmarkDependents;

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

	PROCEDURE UnmarkNeighbour (updater: UpdaterUpdaters.Updater);
		VAR
			prior: GraphStochastic.Node;
	BEGIN
		prior := updater.Prior(0);
		WITH prior: GraphMRF.Node DO
			prior.SetProps(prior.props - {GraphStochastic.mark})
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
			IF ({GraphStochastic.distributed, GraphNodes.mark} * prior.props = {}) THEN
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
						prior.SetProps(prior.props + {GraphNodes.mark});
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
		rowId.id := - ABS(rowId.id)
	END RandomEffects;

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
				IF ~(GraphNodes.mark IN prior.props) THEN
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
				IF ~(GraphNodes.mark IN prior.props) & (prior.components[0] = gmrf) THEN
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
				prior.SetProps(prior.props + {GraphNodes.mark});
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

	PROCEDURE DistributeObservations (IN updaters: ARRAY OF ARRAY OF INTEGER):
	POINTER TO ARRAY OF GraphStochastic.Vector;
		VAR
			i, j, k, numChild, numUpdaters, numCores, label: INTEGER;
			children: GraphStochastic.Vector;
			child: GraphStochastic.Node;
			numData: POINTER TO ARRAY OF INTEGER;
			ok, distributed: BOOLEAN;
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
				WHILE j < numCores DO
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

		PROCEDURE DevianceDo (action: INTEGER): BOOLEAN;
		BEGIN
			i := 0;
			WHILE i < numUpdaters DO
				j := 0;
				label := updaters[0, i];
				u := UpdaterActions.updaters[0, label];
				p := u.Node(0);
				distributed := GraphStochastic.distributed IN p.props;
				WHILE j < numCores DO
					label := updaters[j, i];
					IF label # undefined THEN
						u := UpdaterActions.updaters[0, label];
						children := u.Children();
						IF children # NIL THEN numChild := LEN(children) ELSE numChild := 0 END;
						k := 0;
						WHILE k < numChild DO
							IF ~distributed OR (k MOD numCores = j) THEN
								child := children[k];
								IF GraphDeviance.IsObserved(child) THEN
									IF ~GraphDeviance.DevianceExists(child) THEN
										WHILE j < numCores DO numData[j] := 0; INC(j) END;
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
						END
					END;
					INC(j)
				END;
				INC(i)
			END;
			ClearMarks;
			RETURN TRUE
		END DevianceDo;

	BEGIN
		numCores := LEN(updaters, 0);
		numUpdaters := LEN(updaters, 1);
		NEW(observations, numCores);
		NEW(numData, numCores);
		WHILE j < numCores DO numData[j] := 0; INC(j) END;
		ok := DevianceDo(count);
		j := 0;
		WHILE j < numCores DO
			IF numData[j] > 0 THEN
				NEW(observations[j], numData[j]);
				numData[j] := 0
			ELSE
				observations[j] := NIL
			END;
			INC(j)
		END;
		IF ok THEN ok := DevianceDo(add) END;
		RETURN observations
	END DistributeObservations;

	PROCEDURE ExternalizeObservations (observations: GraphStochastic.Vector; VAR wr: Stores.Writer);
		VAR
			i, len: INTEGER;
			p: GraphStochastic.Node;
	BEGIN
		IF observations # NIL THEN len := LEN(observations) ELSE len := 0 END;
		wr.WriteInt(len);
		i := 0;
		WHILE i < len DO
			p := observations[i];
			GraphNodes.ExternalizePointer(p, wr);
			INC(i)
		END
	END ExternalizeObservations;

	PROCEDURE ExternalizeNamePointers (VAR wr: Stores.Writer);
		VAR
			v0: Counter;
			v1: Writer;
	BEGIN
		NEW(v0);
		v0.num := 0;
		BugsIndex.Accept(v0);
		wr.WriteInt(v0.num);
		NEW(v1);
		v1.wr := wr;
		v1.data := FALSE;
		BugsIndex.Accept(v1)
	END ExternalizeNamePointers;

	PROCEDURE ExternalizeNameData (VAR wr: Stores.Writer);
		VAR
			v: Writer;
	BEGIN
		NEW(v);
		v.wr := wr;
		v.data := TRUE;
		BugsIndex.Accept(v)
	END ExternalizeNameData;

	(*	writes out pointers of stochastic nodes in model, some pointers written multiple times	*)
	PROCEDURE ExternalizeStochasticPointers (IN updaters: ARRAY OF ARRAY OF INTEGER;
	rank: INTEGER; VAR wr: Stores.Writer);
		VAR
			i, j, k, numCores, len, num, size, label, pos: INTEGER;
			p: GraphStochastic.Node;
			u: UpdaterUpdaters.Updater;
			args: GraphStochastic.Args;
			res: SET;
	BEGIN
		numCores := LEN(updaters, 0);
		num := LEN(updaters, 1);
		i := 0;
		len := 0;
		WHILE i < num DO
			label := updaters[0, i];
			u := UpdaterActions.updaters[0, label];
			size := u.Size();
			INC(len, size);
			INC(i)
		END;
		wr.WriteInt(numCores);
		wr.WriteInt(len);
		i := 0;
		WHILE i < numCores DO
			j := 0;
			WHILE j < num DO
				label := updaters[i, j];
				IF label # undefined THEN
					u := UpdaterActions.updaters[0, label];
					size := u.Size()
				ELSE (*	no updater	add in dummy variables	*)
					label := updaters[0, j];
					u := UpdaterActions.updaters[0, label];
					size := u.Size();
					u := NIL
				END;
				k := 0;
				WHILE k < size DO
					IF u = NIL THEN
						p := GraphDummy.fact.New();
						args.Init;
						p.Set(args, res);
					ELSE
						p := u.Node(k)
					END;
					GraphNodes.ExternalizePointer(p, wr);
					IF GraphStochastic.mark IN p.props THEN
						pos := wr.Pos();
						wr.SetPos(pos - SIZE(INTEGER));
						wr.WriteInt(placeHolderType)
					END;
					INC(k)
				END;
				INC(j)
			END;
			INC(i)
		END
	END ExternalizeStochasticPointers;

	(*
	Writes out data of stochastic nodes in model. For fixed effects data only written once.
	Nodes likelihoods thinned for fixed effects and set to NIL for random effects that are
	not on core of rank, same for nodes dependent list.
	*)
	PROCEDURE ExternalizeStochasticData (IN updaters: ARRAY OF ARRAY OF INTEGER;
	rank: INTEGER; VAR wr: Stores.Writer);
		VAR
			i, j, k, numCores, num, size, label: INTEGER;
			p: GraphStochastic.Node;
			u: UpdaterUpdaters.Updater;
			children, thinned, temp: GraphStochastic.Vector;
			tempList: GraphLogical.List;
			args: GraphStochastic.Args;
			res: SET;
	BEGIN
		numCores := LEN(updaters, 0);
		num := LEN(updaters, 1);
		i := 0;
		WHILE i < numCores DO
			j := 0;
			WHILE j < num DO
				label := updaters[i, j];
				IF label # undefined THEN
					u := UpdaterActions.updaters[0, label];
					p := u.Node(0);
					(*	updater is distributed between cores	*)
					IF GraphStochastic.distributed IN p.props THEN
						(*	only write out fist time	*)
						IF i = 0 THEN
							children := u.Children();
							MarkChildren(children, numCores, rank);
							IF u IS UpdaterMultivariate.Updater THEN
								size := u.Size();
								k := 0;
								WHILE k < size DO
									p := u.Node(k);
									temp := p.Children();
									thinned := ThinChildren(temp);
									p.SetChildren(thinned);
									p.Externalize(wr);
									p.SetChildren(temp);
									INC(k)
								END
							ELSE
								thinned := ThinChildren(children);
								p.SetChildren(thinned);
								p.Externalize(wr);
								p.SetChildren(children)
							END;
							UnmarkChildren(children)
						END
					ELSE
						size := u.Size();
						k := 0;
						WHILE k < size DO
							p := u.Node(k);
							IF GraphStochastic.mark IN p.props THEN
								(*	not updated on this core & place holder	*)
								placeHolder.Externalize(wr)
							ELSIF i # rank THEN
								(*	not updated on this core	*)
								temp := p.Children();
								tempList := p.dependents;
								p.SetChildren(NIL);
								p.SetDependents(NIL);
								p.Externalize(wr);
								p.SetChildren(temp);
								p.SetDependents(tempList)
							ELSE
								p.Externalize(wr)
							END;
							INC(k)
						END
					END
				ELSE
					label := updaters[0, j];
					u := UpdaterActions.updaters[0, label];
					size := u.Size();
					k := 0;
					WHILE k < size DO
						p := GraphDummy.fact.New();
						args.Init;
						p.Set(args, res);
						p.Externalize(wr);
						INC(k)
					END
				END;
				INC(j)
			END;
			INC(i)
		END
	END ExternalizeStochasticData;

	PROCEDURE ExternalizeUpdaters (IN updaters: ARRAY OF ARRAY OF INTEGER; rank: INTEGER;
	VAR wr: Stores.Writer);
		VAR
			i, label, len, size: INTEGER;
			u: UpdaterUpdaters.Updater;
	BEGIN
		i := 0;
		len := LEN(updaters, 1);
		wr.WriteInt(len);
		WHILE i < len DO
			label := updaters[rank, i];
			IF label # undefined THEN
				u := UpdaterActions.updaters[0, label];
			ELSE
				label := updaters[0, i];
				u := UpdaterActions.updaters[0, label];
				size := u.Size();
				u := UpdaterEmpty.New(size)
			END;
			UpdaterUpdaters.ExternalizePointer(u, wr);
			INC(i)
		END
	END ExternalizeUpdaters;

	PROCEDURE ExternalizeUpdatersData (IN updaters: ARRAY OF ARRAY OF INTEGER; rank, chain: INTEGER;
	VAR wr: Stores.Writer);
		VAR
			i, label, len, size: INTEGER;
			u: UpdaterUpdaters.Updater;
	BEGIN
		i := 0;
		len := LEN(updaters, 1);
		WHILE i < len DO
			label := updaters[rank, i];
			IF label # undefined THEN
				u := UpdaterActions.updaters[chain, label];
			ELSE
				label := updaters[0, i];
				u := UpdaterActions.updaters[0, label];
				size := u.Size();
				u := UpdaterEmpty.New(size)
			END;
			UpdaterUpdaters.Externalize(u, wr);
			INC(i)
		END
	END ExternalizeUpdatersData;

	PROCEDURE MarkNodes;
		VAR
			v: Marker;
			i, j, numCores, num: INTEGER;
			p: GraphStochastic.Node;
	BEGIN
		NEW(v);
		v.mark := TRUE;
		BugsIndex.Accept(v);
		numCores := LEN(globalStochs);
		num := LEN(globalStochs[0]);
		i := 0;
		WHILE i < numCores DO
			j := 0;
			WHILE j < num DO
				p := globalStochs[i, j];
				p.SetProps(p.props + {GraphStochastic.mark});
				INC(j)
			END;
			INC(i)
		END
	END MarkNodes;

	PROCEDURE UnmarkNodes;
		VAR
			v: Marker;
			i, j, numCores, num: INTEGER;
			p: GraphStochastic.Node;
	BEGIN
		NEW(v);
		v.mark := FALSE;
		BugsIndex.Accept(v);
		numCores := LEN(globalStochs);
		num := LEN(globalStochs[0]);
		i := 0;
		WHILE i < numCores DO
			j := 0;
			WHILE j < num DO
				p := globalStochs[i, j];
				p.SetProps(p.props - {GraphStochastic.mark});
				INC(j)
			END;
			INC(i)
		END
	END UnmarkNodes;

	PROCEDURE AddParents (IN updaters: ARRAY OF ARRAY OF INTEGER; rank: INTEGER);
		VAR
			i, k, numCores, num, size, label: INTEGER;
			p: GraphStochastic.Node;
			u: UpdaterUpdaters.Updater;
			list: GraphLogical.List;
			logical: GraphLogical.Node;
		CONST
			all = TRUE;
	BEGIN
		numCores := LEN(updaters, 0);
		num := LEN(updaters, 1);
		i := 0;
		WHILE i < num DO
			label := updaters[rank, i];
			IF label # undefined THEN
				u := UpdaterActions.updaters[0, label];
				size := u.Size();
				k := 0;
				WHILE k < size DO
					p := u.Node(k);
					p.SetProps(p.props - {GraphStochastic.mark});
					list := GraphLogical.Parents(p, all);
					WHILE list # NIL DO
						logical := list.node;
						logical.SetProps(logical.props - {GraphStochastic.mark});
						list := list.next
					END;
					INC(k)
				END
			END;
			INC(i)
		END
	END AddParents;

	(*	add in children of nodes being updated and also their logical parents	*)
	PROCEDURE AddChildrenCoparents (IN updaters: ARRAY OF ARRAY OF INTEGER; rank: INTEGER);
		VAR
			u: UpdaterUpdaters.Updater;
			children: GraphStochastic.Vector;
			p: GraphStochastic.Node;
			i, j, len, num, step, label, thin: INTEGER;
			list: GraphLogical.List;
			logical: GraphLogical.Node;
		CONST
			all = TRUE;
	BEGIN
		i := 0;
		thin := LEN(updaters, 0);
		num := LEN(updaters, 1);
		WHILE i < num DO
			label := updaters[rank, i];
			IF label # undefined THEN
				u := UpdaterActions.updaters[0, label];
				children := u.Children();
				p := u.Node(0);
				IF children # NIL THEN len := LEN(children) ELSE len := 0 END;
				IF GraphStochastic.distributed IN p.props THEN j := rank; step := thin ELSE j := 0; step := 1 END;
				WHILE j < len DO
					p := children[j];
					p.SetProps(p.props - {GraphStochastic.mark});
					list := GraphLogical.Parents(p, all);
					WHILE list # NIL DO
						logical := list.node;
						logical.SetProps(logical.props - {GraphStochastic.mark});
						list := list.next
					END;
					INC(j, step)
				END
			END;
			INC(i)
		END
	END AddChildrenCoparents;

	(*	add observations to nodes nodes to be writen	*)
	PROCEDURE AddObservations (observations: GraphStochastic.Vector);
		VAR
			i, len: INTEGER;
			p: GraphStochastic.Node;
			q: GraphLogical.Node;
			list: GraphLogical.List;
		CONST
			all = TRUE;
	BEGIN
		IF observations # NIL THEN len := LEN(observations) ELSE len := 0 END;
		i := 0;
		WHILE i < len DO
			p := observations[i];
			p.SetProps(p.props - {GraphStochastic.mark});
			list := GraphLogical.Parents(p, all);
			WHILE list # NIL DO
				q := list.node;
				q.SetProps(q.props - {GraphStochastic.mark});
				list := list.next
			END;
			INC(i)
		END
	END AddObservations;

	PROCEDURE ExternalizeId (IN id: ARRAY OF INTEGER; VAR wr: Stores.Writer);
		VAR
			i, len: INTEGER;
	BEGIN
		i := 0;
		len := LEN(id);
		wr.WriteInt(len);
		WHILE i < len DO
			wr.WriteInt(id[i]);
			INC(i)
		END
	END ExternalizeId;

	PROCEDURE MaxSizeParams (updaters: UpdaterUpdaters.Vector): INTEGER;
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
			prior := u.Node(0);
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

	PROCEDURE StoreStochastic;
		VAR
			i, j, k, numCores, len, num, size, label: INTEGER;
			p: GraphStochastic.Node;
			u: UpdaterUpdaters.Updater;
	BEGIN
		numCores := LEN(globalUpdaters, 0);
		num := LEN(globalUpdaters, 1);
		j := 0;
		len := 0;
		WHILE j < num DO
			label := globalUpdaters[0, j];
			u := UpdaterActions.updaters[0, label];
			size := u.Size();
			INC(len, size);
			INC(j)
		END;
		NEW(globalStochs, numCores);
		i := 0;
		WHILE i < numCores DO
			NEW(globalStochs[i], len);
			INC(i)
		END;
		i := 0;
		WHILE i < numCores DO
			j := 0;
			len := 0;
			WHILE j < num DO
				label := globalUpdaters[i, j];
				IF label # undefined THEN
					u := UpdaterActions.updaters[0, label];
					size := u.Size()
				ELSE (*	no updater	add in dummy variables	*)
					label := globalUpdaters[0, j];
					u := UpdaterActions.updaters[0, label];
					size := u.Size();
					u := NIL
				END;
				k := 0;
				WHILE k < size DO
					IF u = NIL THEN
						p := GraphDummy.fact.New()
					ELSE
						p := u.Node(k)
					END;
					globalStochs[i, len] := p;
					INC(len);
					INC(k)
				END;
				INC(j)
			END;
			INC(i)
		END
	END StoreStochastic;

	PROCEDURE Clear*;
	BEGIN
		globalStochs := NIL;
		globalUpdaters := NIL;
		globalId := NIL
	END Clear;

	PROCEDURE DistributeUpdaters* (numCores: INTEGER;
	OUT updaters: POINTER TO ARRAY OF ARRAY OF INTEGER;
	OUT id: POINTER TO ARRAY OF INTEGER);
		VAR
			i, j, depth, maxDepth, minDepth, numUpdaters: INTEGER;
			cursor, element, list, mrfList: List;
			lists: POINTER TO ARRAY OF List;
			rowId: RowId;
			prior, q: GraphStochastic.Node;
			constraint: INTEGER;
			lastColour, new: BOOLEAN;
			u: UpdaterUpdaters.Updater;
	BEGIN
		rowId := NIL;
		NEW(lists, numCores);
		UpdaterActions.MinMaxDepth(minDepth, maxDepth);
		i := 0;
		WHILE i < numCores DO
			lists[i] := NIL;
			INC(i)
		END;
		UpdaterActions.MarkDistributed;
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
		WHILE i < numCores DO
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
		NEW(updaters, numCores, numUpdaters);
		NEW(id, numUpdaters);
		j := 0;
		WHILE j < numUpdaters DO
			i := 0;
			WHILE i < numCores DO
				updaters[i, j] := lists[i].updater;
				IF updaters[i, j] # undefined THEN
					u := UpdaterActions.updaters[0, updaters[i, j]];
					prior := u.Node(0);
					prior.SetProps(prior.props - {GraphNodes.mark})
				END;
				INC(i)
			END;
			i := 0;
			WHILE i < numCores DO
				lists[i] := lists[i].next;
				INC(i)
			END;
			INC(j);
			id[numUpdaters - j] := rowId.id;
			rowId := rowId.next
		END;
		(*	if only one updater in parallel block distribute updater if possible	*)
		IF numCores > 1 THEN
			j := 0;
			WHILE j < numUpdaters DO
				IF id[j] = - 1 THEN
					u := UpdaterActions.updaters[0, updaters[0, j]];
					q := u.Node(0);
					IF (updaters[1, j] = undefined) & ~(q IS GraphMRF.Node) THEN
						id[j] := 0;
						q.SetProps(q.props + {GraphStochastic.distributed});
						i := 1;
						WHILE i < numCores DO
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

	PROCEDURE Distribute* (numCores: INTEGER);
	BEGIN
		DistributeUpdaters(numCores, globalUpdaters, globalId);
		StoreStochastic
	END Distribute;

	PROCEDURE MonitoredNodes* (OUT nodes: ARRAY OF INTEGER; OUT num: INTEGER);
		VAR
			i, j, len, numCores: INTEGER;
			p: GraphStochastic.Node;
			q: GraphNodes.Node;
	BEGIN
		numCores := LEN(globalStochs);
		len := LEN(globalStochs[0]);
		i := 0;
		WHILE i < numCores DO
			j := 0;
			WHILE j < len DO (*	any node that has its represenative marked is also marked	*)
				p := globalStochs[i, j];
				q := p.Representative();
				IF GraphStochastic.mark IN q.props THEN
					p.SetProps(p.props + {GraphStochastic.mark})
				END;
				INC(i)
			END;
			INC(i)
		END;
		num := 0;
		i := 0;
		WHILE i < numCores DO
			j := 0;
			WHILE j < len DO
				p := globalStochs[i, j];
				IF GraphStochastic.mark IN p.props THEN
					IF (i = 0) OR (globalStochs[0, j] # globalStochs[i, j]) THEN
						nodes[num] := i * len + j;
						INC(num)
					END
				END;
				INC(j)
			END;
			INC(i)
		END
	END MonitoredNodes;

	PROCEDURE RecvMonitored* (IN values: ARRAY OF REAL; IN monitored: ARRAY OF INTEGER;
	numMonitored: INTEGER);
		VAR
			i, j, k, index, len: INTEGER;
			p: GraphStochastic.Node;
	BEGIN
		len := LEN(globalStochs[0]);
		i := 0;
		WHILE i < numMonitored DO
			index := monitored[i];
			j := index DIV len;
			k := index MOD len;
			p := globalStochs[j, k];
			p.SetValue(values[i]);
			INC(i)
		END
	END RecvMonitored;

	PROCEDURE RecvSample* (IN values: ARRAY OF REAL);
		VAR
			i, j, numCores, num: INTEGER;
	BEGIN
		numCores := LEN(globalStochs);
		num := LEN(globalStochs[0]);
		i := 0;
		WHILE i < numCores DO
			j := 0;
			WHILE j < num DO
				globalStochs[i, j].SetValue(values[i * num + j]);
				INC(j)
			END;
			INC(i)
		END
	END RecvSample;

	PROCEDURE Write* (rank, numCores, numChains: INTEGER; VAR wr: Stores.Writer);
		VAR
			deviance: POINTER TO ARRAY OF GraphStochastic.Vector;
			chain, maxSizeParams: INTEGER;
	BEGIN
		deviance := DistributeObservations(globalUpdaters);
		maxSizeParams := MaxSizeParams(UpdaterActions.updaters[0]);
		wr.WriteInt(maxSizeParams);
		MarkNodes;
		AddParents(globalUpdaters, rank);
		AddChildrenCoparents(globalUpdaters, rank);
		AddObservations(deviance[rank]);
		GraphNodes.BeginExternalize(wr);
		ExternalizeId(globalId, wr);
		ExternalizeStochasticPointers(globalUpdaters, rank, wr);
		ExternalizeNamePointers(wr);
		ExternalizeObservations(deviance[rank], wr);
		ExternalizeStochasticData(globalUpdaters, rank, wr);
		ExternalizeNameData(wr);
		UnmarkNodes;
		UpdaterUpdaters.BeginExternalize(wr);
		ExternalizeUpdaters(globalUpdaters, rank, wr);
		chain := 0;
		WHILE chain < numChains DO
			ExternalizeUpdatersData(globalUpdaters, rank, chain, wr);
			INC(chain)
		END;
		UpdaterUpdaters.EndExternalize(wr);
		GraphNodes.EndExternalize(wr)
	END Write;

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
		placeHolder := GraphDummy.fact.New();
		placeHolder.Init
	END Init;

BEGIN
	Init
END BugsParallel.

