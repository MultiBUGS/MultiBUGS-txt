(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



  *)

MODULE UpdaterActions;

	(*	Updating actions	*)

	

	IMPORT
		SYSTEM,
		Stores := Stores64,
		GraphLogical, GraphMRF, GraphNodes, GraphStochastic,
		MathSort,
		UpdaterAuxillary, UpdaterUpdaters;

	TYPE
		List = POINTER TO RECORD
			updater: UpdaterUpdaters.Updater;
			label: INTEGER;
			next: List
		END;

	VAR
		updaterList: List;
		updaters-: POINTER TO ARRAY OF UpdaterUpdaters.Vector;
		endOfAdapting-, iteration-: INTEGER;
		initialized-: BOOLEAN;
		version-: INTEGER; 	(*	version number	*)
		maintainer-: ARRAY 40 OF CHAR; 	(*	person maintaining module	*)

	PROCEDURE NumberChains* (): INTEGER;
		VAR
			numChains: INTEGER;
	BEGIN
		numChains := 1;
		IF updaters # NIL THEN numChains := LEN(updaters, 0) END;
		RETURN numChains
	END NumberChains;

	PROCEDURE NumberUpdaters* (): INTEGER;
		VAR
			numUpdater: INTEGER;
	BEGIN
		numUpdater := 0;
		IF updaters # NIL THEN numUpdater := LEN(updaters[0], 0) END;
		RETURN numUpdater
	END NumberUpdaters;

	PROCEDURE MeanNumChildren* (): INTEGER;
		VAR
			updater: UpdaterUpdaters.Updater;
			children: GraphStochastic.Vector;
			i, numUpdaters, numChildren, totalChildren, num: INTEGER;
	BEGIN
		(*	remove forward sampling nodes from mean calculation	*)
		i := 0;
		numUpdaters := NumberUpdaters();
		totalChildren := 0;
		num := 0;
		WHILE i < numUpdaters DO
			updater := updaters[0, i];
			IF updater # NIL THEN
				children := updater.Children();
				IF children # NIL THEN
					numChildren := LEN(children);
					INC(totalChildren, numChildren);
					INC(num)
				END
			END;
			INC(i)
		END;
		IF totalChildren = 0 THEN
			RETURN 0
		ELSE
			RETURN totalChildren DIV num
		END
	END MeanNumChildren;

	PROCEDURE MedianNumChildren* (): INTEGER;
		VAR
			updater: UpdaterUpdaters.Updater;
			children: GraphStochastic.Vector;
			i, j, numUpdaters, numChildren, numForward: INTEGER;
			x: POINTER TO ARRAY OF REAL;
	BEGIN
		numForward := 0;
		numUpdaters := NumberUpdaters();
		i := 0;
		WHILE i < numUpdaters DO
			updater := updaters[0, i];
			IF updater # NIL THEN
				children := updater.Children();
				IF children = NIL THEN INC(numForward) END
			END;
			INC(i)
		END;
		NEW(x, numUpdaters - numForward);
		i := 0;
		j := 0;
		WHILE i < numUpdaters DO
			updater := updaters[0, i];
			IF updater # NIL THEN
				children := updater.Children();
				IF children # NIL THEN
					numChildren := LEN(children);
					x[j] := numChildren;
					INC(j)
				END
			END;
			INC(i)
		END;
		MathSort.HeapSort(x, j);
		RETURN SHORT(ENTIER(x[j DIV 2] + 0.5))
	END MedianNumChildren;

	(*	Clears the updater list etc	*)
	PROCEDURE Clear*;
	BEGIN
		iteration := 0;
		endOfAdapting := MAX(INTEGER);
		initialized := FALSE;
		updaterList := NIL;
		updaters := NIL
	END Clear;

	(*	counts number of constraints in MRF models if using univariate sampling	*)
	PROCEDURE CountUpdaters (OUT numUpdaters, numConstraints: INTEGER);
		VAR
			mrf: GraphMRF.Node;
			cursor: List;
			updater: UpdaterUpdaters.Updater;
			prior: GraphStochastic.Node;
	BEGIN
		numUpdaters := 0;
		numConstraints := 0;
		cursor := updaterList;
		WHILE cursor # NIL DO
			INC(numUpdaters);
			updater := cursor.updater;
			prior := updater.Node(0);
			IF (prior IS GraphMRF.Node) & (updater.Size() = 1) THEN
				mrf := prior(GraphMRF.Node);
				mrf := mrf.components[0](GraphMRF.Node);
				IF ~(GraphNodes.mark IN mrf.props) THEN
					INCL(mrf.props, GraphNodes.mark);
					IF mrf.NumberConstraints() # 0 THEN
						INC(numConstraints)
					END
				END
			END;
			cursor := cursor.next
		END;
		cursor := updaterList;
		WHILE cursor # NIL DO
			updater := cursor.updater;
			prior := updater.Node(0);
			IF (prior IS GraphMRF.Node) & (updater.Size() = 1) THEN
				mrf := prior(GraphMRF.Node);
				mrf := mrf.components[0](GraphMRF.Node);
				EXCL(mrf.props, GraphNodes.mark);
			END;
			cursor := cursor.next
		END
	END CountUpdaters;

	PROCEDURE HeapSort (VAR updaters: ARRAY OF List; len: INTEGER);
		VAR
			i, j, k: INTEGER;

		PROCEDURE Less (l, m: INTEGER): BOOLEAN;
			VAR
				depth0, depth1, len0, len1, a0, a1: INTEGER;
				children: GraphStochastic.Vector;
				prior0, prior1: GraphStochastic.Node;
		BEGIN
			depth0 := updaters[l - 1].updater.Depth();
			depth1 := updaters[m - 1].updater.Depth();
			IF depth0 > depth1 THEN
				RETURN TRUE
			ELSIF depth0 < depth1 THEN
				RETURN FALSE
			ELSE
				children := updaters[l - 1].updater.Children();
				IF children # NIL THEN len0 := LEN(children) ELSE len0 := 0 END;
				children := updaters[m - 1].updater.Children();
				IF children # NIL THEN len1 := LEN(children) ELSE len1 := 0 END;
				IF len0 > len1 THEN
					RETURN TRUE
				ELSIF len0 < len1 THEN
					RETURN FALSE
				ELSE
					RETURN updaters[l - 1].label < updaters[m - 1].label
				END
			END
		END Less;

		PROCEDURE Swap (l, m: INTEGER);
			VAR
				temp: List;
		BEGIN
			temp := updaters[l - 1];
			updaters[l - 1] := updaters[m - 1];
			updaters[m - 1] := temp
		END Swap;

	BEGIN
		ASSERT(LEN(updaters) >= len, 20);
		IF len > 1 THEN
			i := len DIV 2;
			REPEAT
				j := i;
				LOOP
					k := j * 2;
					IF k > len THEN EXIT END;
					IF (k < len) & Less(k, k + 1) THEN INC(k) END;
					IF Less(j, k) THEN Swap(j, k) ELSE EXIT END;
					j := k
				END;
				DEC(i)
			UNTIL i = 0;
			i := len;
			REPEAT
				j := 1; Swap(j, i); DEC(i);
				LOOP
					k := j * 2; IF k > i THEN EXIT END;
					IF (k < i) & Less(k, k + 1) THEN INC(k) END;
					Swap(j, k);
					j := k
				END;
				LOOP
					k := j DIV 2;
					IF (k > 0) & Less(k, j) THEN Swap(j, k); j := k ELSE EXIT END
				END
			UNTIL i = 0
		END
	END HeapSort;

	PROCEDURE CreateUpdaters* (numChains: INTEGER);
		VAR
			i, j, k, numConstraints, numUpdaters, size: INTEGER;
			mrf: GraphMRF.Node;
			cursor: List;
			updater: UpdaterUpdaters.Updater;
			prior: GraphStochastic.Node;
			factory: UpdaterUpdaters.Factory;
			lists: POINTER TO ARRAY OF List;
	BEGIN
		CountUpdaters(numUpdaters, numConstraints);
		IF numConstraints # 0 THEN	(*	create constraint factory if needed	*)
			factory := UpdaterUpdaters.InstallFactory("UpdaterMRFConstrain.Install")
		END;
		IF numUpdaters > 0 THEN
			NEW(lists, numUpdaters);
			NEW(updaters, numChains);
			i := 0;
			WHILE i < numChains DO
				NEW(updaters[i], numUpdaters + numConstraints);
				INC(i)
			END
		END;
		cursor := updaterList;
		j := 0;
		WHILE cursor # NIL DO
			lists[j] := cursor;
			cursor := cursor.next;
			INC(j)
		END;
		HeapSort(lists, numUpdaters);
		j := 0;
		k := 0;
		WHILE j < numUpdaters DO
			updater := lists[j].updater;
			updaters[0, k] := updater;
			prior := updater.Prior(0);
			(*	insert constraints	*)
			IF (prior IS GraphMRF.Node) & (updater.Size() = 1) THEN
				mrf := prior(GraphMRF.Node);
				mrf := mrf.components[0](GraphMRF.Node);
				IF mrf.NumberConstraints() # 0 THEN
					IF ~(GraphNodes.mark IN mrf.props) THEN
						INCL(mrf.props, GraphNodes.mark);
						size := mrf.Size() - 1
					ELSE
						DEC(size);
						IF size = 0 THEN
							EXCL(mrf.props, GraphNodes.mark);
							INC(k);
							updaters[0, k] := factory.New(mrf)
						END
					END
				END
			END;
			INC(k);
			INC(j)
		END;
		i := 1;
		WHILE i < numChains DO
			j := 0;
			WHILE j < numUpdaters + numConstraints DO
				updaters[i, j] := UpdaterUpdaters.CopyFrom(updaters[0, j]);
				INC(j)
			END;
			INC(i)
		END;
		updaterList := NIL
	END CreateUpdaters;

	(*	externalize data of nodes associated with updaters	*)
	PROCEDURE ExternalizeParamData* (VAR wr: Stores.Writer);
		VAR
			updater: UpdaterUpdaters.Updater;
			p: GraphStochastic.Node;
			i, j, numUpdaters, size: INTEGER;
	BEGIN
		numUpdaters := NumberUpdaters();
		i := 0;
		WHILE i < numUpdaters DO
			updater := updaters[0, i];
			size := updater.Size();
			j := 0;
			WHILE j < size DO
				p := updater.Node(j);
				p.Externalize(wr);
				INC(j)
			END;
			INC(i)
		END
	END ExternalizeParamData;

	(*	externalize pointers to nodes associated with updaters	*)
	PROCEDURE ExternalizeParamPointers* (VAR wr: Stores.Writer);
		VAR
			u: UpdaterUpdaters.Updater;
			p: GraphStochastic.Node;
			i, j, numUpdaters, num, size: INTEGER;
	BEGIN
		numUpdaters := NumberUpdaters();
		num := 0;
		i := 0;
		WHILE i < numUpdaters DO
			u := updaters[0, i];
			size := u.Size();
			INC(num, size);
			INC(i)
		END;
		wr.WriteInt(num);
		i := 0;
		WHILE i < numUpdaters DO
			u := updaters[0, i];
			size := u.Size();
			j := 0;
			WHILE j < size DO
				p := u.Node(j);
				GraphNodes.ExternalizePointer(p, wr);
				INC(j)
			END;
			INC(i)
		END
	END ExternalizeParamPointers;

	(*	Externalize updaters mutable state	*)
	PROCEDURE ExternalizeUpdaterData* (VAR wr: Stores.Writer);
		VAR
			i, j, numChains, numUpdaters: INTEGER;
			oldPos, pos: LONGINT;
			updater: UpdaterUpdaters.Updater;
			posArray: POINTER TO ARRAY OF LONGINT;
	BEGIN
		numChains := NumberChains();
		NEW(posArray, numChains);
		numUpdaters := NumberUpdaters();
		oldPos := wr.Pos();
		i := 0;
		WHILE i < numChains DO
			wr.WriteLong( - 1); INC(i)
		END;
		i := 0;
		WHILE i < numChains DO
			posArray[i] := wr.Pos();
			j := 0;
			WHILE j < numUpdaters DO
				updater := updaters[i, j];
				UpdaterUpdaters.Externalize(updater, wr);
				INC(j)
			END;
			INC(i)
		END;
		pos := wr.Pos();
		wr.SetPos(oldPos);
		i := 0;
		WHILE i < numChains DO
			wr.WriteLong(posArray[i]); INC(i)
		END;
		wr.SetPos(pos)
	END ExternalizeUpdaterData;

	(*	Externalize updaters	*)
	PROCEDURE ExternalizeUpdaterPointers* (VAR wr: Stores.Writer);
		VAR
			updater: UpdaterUpdaters.Updater;
			i, numChains, numUpdaters: INTEGER;
	BEGIN
		UpdaterUpdaters.BeginExternalize(wr);
		numChains := NumberChains();
		numUpdaters := NumberUpdaters();
		wr.WriteBool(initialized);
		wr.WriteInt(numChains);
		wr.WriteInt(numUpdaters);
		i := 0;
		WHILE i < numUpdaters DO
			updater := updaters[0, i];
			UpdaterUpdaters.ExternalizePointer(updater, wr);
			INC(i)
		END;
		UpdaterUpdaters.EndExternalize(wr)
	END ExternalizeUpdaterPointers;

	(*	Finds multivariate sampler for node p	*)
	PROCEDURE FindMVSampler* (p: GraphStochastic.Node): UpdaterUpdaters.Updater;
		VAR
			updater: UpdaterUpdaters.Updater;
			i, j, numUpdaters, size: INTEGER;
	BEGIN
		numUpdaters := LEN(updaters[0], 0);
		i := 0;
		LOOP
			updater := updaters[0, i];
			size := updater.Size();
			j := 0;
			WHILE (j < size) & (p # updater.Prior(j)) DO INC(j) END;
			IF j < size THEN EXIT END;
			INC(i);
			IF i = numUpdaters THEN EXIT END
		END;
		IF i = numUpdaters THEN updater := NIL END;
		RETURN updater
	END FindMVSampler;

	(*	Finds sampler for node p	*)
	PROCEDURE FindSampler* (chain: INTEGER; p: GraphStochastic.Node): UpdaterUpdaters.Updater;
		VAR
			i, numUpdaters: INTEGER;
			updater: UpdaterUpdaters.Updater;
	BEGIN
		numUpdaters := LEN(updaters[0], 0);
		i := 0;
		LOOP
			updater := updaters[chain, i];
			IF (updater # NIL) & (p = updater.Node(0)) THEN EXIT END;
			INC(i);
			IF i = numUpdaters THEN EXIT END
		END;
		IF i = numUpdaters THEN updater := NIL END;
		RETURN updater
	END FindSampler;

	PROCEDURE FindUpdater* (updater: UpdaterUpdaters.Updater; OUT chain, index: INTEGER);
		VAR
			found: BOOLEAN;
			i, j, numChains, numUpdaters: INTEGER;
	BEGIN
		numChains := NumberChains();
		numUpdaters := NumberUpdaters();
		found := FALSE;
		i := 0;
		WHILE (i < numChains) & ~found DO
			j := 0;
			WHILE (j < numUpdaters) & ~found DO
				found := updaters[i, j] = updater;
				INC(j)
			END;
			INC(i)
		END;
		IF found THEN
			chain := i - 1; index := j - 1
		ELSE
			chain := - 1; index := - 1
		END
	END FindUpdater;

	(*	Internalize updaters mutable state	*)
	PROCEDURE InternalizeUpdaterData* (chain: INTEGER; VAR rd: Stores.Reader);
		VAR
			i, numChains, numUpdaters: INTEGER;
			updater: UpdaterUpdaters.Updater;
			pos: LONGINT;
	BEGIN
		numChains := NumberChains();
		numUpdaters := NumberUpdaters();
		i := 0;
		WHILE i <= chain DO
			rd.ReadLong(pos);
			INC(i)
		END;
		rd.SetPos(pos);
		chain := chain MOD numChains;
		i := 0;
		WHILE i < numUpdaters DO
			updater := updaters[chain, i];
			UpdaterUpdaters.Internalize(updater, rd);
			INC(i)
		END
	END InternalizeUpdaterData;

	(*	Internalize updaters	*)
	PROCEDURE InternalizeUpdaterPointers* (nChains: INTEGER; VAR rd: Stores.Reader);
		VAR
			updater: UpdaterUpdaters.Updater;
			i, j, numChains, numUpdaters: INTEGER;
	BEGIN
		UpdaterUpdaters.BeginInternalize(rd);
		rd.ReadBool(initialized);
		rd.ReadInt(numChains);
		numChains := MIN(numChains, nChains);
		rd.ReadInt(numUpdaters);
		NEW(updaters, numChains);
		i := 0;
		WHILE i < numChains DO
			NEW(updaters[i], numUpdaters);
			INC(i)
		END;
		i := 0;
		WHILE i < numUpdaters DO
			updater := UpdaterUpdaters.InternalizePointer(rd);
			updaters[0, i] := updater;
			j := 1;
			WHILE j < numChains DO
				updaters[j, i] := UpdaterUpdaters.CopyFrom(updater);
				INC(j)
			END;
			INC(i)
		END;
		UpdaterUpdaters.EndInternalize(rd)
	END InternalizeUpdaterPointers;

	(*	Is the chain in adaptive phase	*)
	PROCEDURE IsAdapting* (chain: INTEGER): BOOLEAN;
		VAR
			isAdapting: BOOLEAN;
			i, numUpdaters: INTEGER;
			updater: UpdaterUpdaters.Updater;
	BEGIN
		isAdapting := FALSE;
		numUpdaters := NumberUpdaters();
		i := 0;
		WHILE (i < numUpdaters) & ~isAdapting DO
			updater := updaters[chain, i];
			isAdapting := updater.IsAdapting();
			INC(i)
		END;
		RETURN isAdapting
	END IsAdapting;

	PROCEDURE CanDistribute (updater: UpdaterUpdaters.Updater; avNum, workersPerChain: INTEGER): BOOLEAN;
		VAR
			canDistribute: BOOLEAN;
			i, numChild, num: INTEGER;
			children: GraphStochastic.Vector;
			prior: GraphStochastic.Node;
			dependents: GraphLogical.Vector;
			logical: GraphLogical.Node;
	BEGIN
		WITH updater: UpdaterAuxillary.UpdaterUV DO
			canDistribute := FALSE
		ELSE
			children := updater.Children();
			prior := updater.Prior(0);
			IF children # NIL THEN numChild := LEN(children) ELSE numChild := 0 END;
			canDistribute := (numChild > 2 * avNum) (*OR numChild > 50 * workersPerChain*) OR
			((GraphNodes.maxStochDepth = 1) & (numChild > 2 * workersPerChain))
			OR ((prior.depth = 1) & (numChild > 10 * workersPerChain));
			i := 0;
			WHILE ~canDistribute & (i < numChild) DO
				canDistribute := children[i] IS GraphMRF.Node;
				INC(i)
			END;
			(*	do not distribute if prior has logical dependent nodes	*)
			dependents := prior.dependents;
			IF dependents # NIL THEN
				num := LEN(dependents);
				i := 0;
				WHILE canDistribute & (i < num) DO
					logical := dependents[i];
					canDistribute := ~(GraphLogical.prediction IN logical.props) OR (logical.Size() = 1);
					INC(i)
				END
			END
		END;
		RETURN canDistribute
	END CanDistribute;

	PROCEDURE MarkDistributed* (workersPerChain: INTEGER);
		VAR
			i, j, numUpdaters, avNumChild, size: INTEGER;
			updater: UpdaterUpdaters.Updater;
			prior, p: GraphStochastic.Node;
			list: GraphStochastic.List;
		CONST
			all = TRUE;
	BEGIN
		avNumChild := MeanNumChildren();
		numUpdaters := NumberUpdaters();
		i := 0;
		WHILE i < numUpdaters DO
			updater := updaters[0, i];
			IF CanDistribute(updater, avNumChild, workersPerChain) THEN
				j := 0;
				size := updater.Size();
				WHILE j < size DO
					prior := updater.Prior(j);
					INCL(prior.props, GraphStochastic.distributed);
					list := GraphStochastic.Parents(prior, all);
					WHILE updaterList # NIL DO
						p := list.node;
						INCL(p.props, GraphStochastic.distributed);
						list := list.next
					END;
					INC(j)
				END
			END;
			INC(i)
		END
	END MarkDistributed;

	PROCEDURE MinMaxDepth* (OUT min, max: INTEGER);
		VAR
			numUpdaters: INTEGER;
	BEGIN
		numUpdaters := NumberUpdaters();
		max := updaters[0, 0].Depth();
		min := updaters[0, numUpdaters - 1].Depth()
	END MinMaxDepth;

	PROCEDURE NumParameters* (): INTEGER;
		VAR
			updater: UpdaterUpdaters.Updater;
			i, numParameter, numUpdaters, size: INTEGER;
	BEGIN
		numUpdaters := NumberUpdaters();
		i := 0;
		numParameter := 0;
		WHILE i < numUpdaters DO
			updater := updaters[0, i];
			IF updater # NIL THEN
				size := updater.Size();
				INC(numParameter, size)
			END;
			INC(i);
		END;
		RETURN numParameter
	END NumParameters;

	PROCEDURE OptimizeUpdaters*;
		VAR
			i, chain, numChains, numUpdaters: INTEGER;
	BEGIN
		IF updaters # NIL THEN
			numChains := LEN(updaters);
			IF updaters[0] # NIL THEN
				numUpdaters := LEN(updaters[0]);
				chain := 0;
				WHILE chain < numChains DO
					i := 0; WHILE i < numUpdaters DO updaters[chain, i].Optimize; INC(i) END;
					INC(chain)
				END
			END
		END
	END OptimizeUpdaters;

	(*	Stores new updater in updaterList insertion sort	*)
	PROCEDURE RegisterUpdater* (updater: UpdaterUpdaters.Updater);
		VAR
			element: List;
	BEGIN
		NEW(element);
		element.updater := updater;
		IF updaterList = NIL THEN
			element.label := 0
		ELSE
			element.label := updaterList.label + 1
		END;
		element.next := updaterList;
		updaterList := element
	END RegisterUpdater;

	(*	Replaces old registered sampler with a new one	*)
	PROCEDURE ReplaceUpdater* (updater: UpdaterUpdaters.Updater);
		VAR
			sameNode: BOOLEAN;
			i, j, numChains, numUpdaters, size: INTEGER;
			p: GraphStochastic.Node;
			u: UpdaterUpdaters.Updater;
	BEGIN
		p := updater.Prior(0);
		numChains := NumberChains();
		numUpdaters := NumberUpdaters();
		i := 0;
		LOOP
			u := updaters[0, i];
			IF p = u.Prior(0) THEN EXIT END;
			INC(i);
			IF i = numUpdaters THEN EXIT END
		END;
		IF i < numUpdaters THEN
			size := updater.Size();
			IF size = u.Size() THEN
				j := 0;
				sameNode := TRUE;
				WHILE (j < size) & sameNode DO
					sameNode := updater.Prior(j) = u.Prior(j);
					INC(j)
				END;
				IF sameNode THEN
					updaters[0, i] := updater;
					j := 1;
					WHILE j < numChains DO
						updaters[j, i] := UpdaterUpdaters.CopyFrom(updater);
						INC(j)
					END
				END
			END
		END
	END ReplaceUpdater;

	(*	Carry out one sampling update of all registered updaters	*)
	PROCEDURE Sample* (overRelax: BOOLEAN; chain: INTEGER; OUT res: SET;
	OUT updater: UpdaterUpdaters.Updater);
		VAR
			i, numUpdaters: INTEGER;
	BEGIN
		res := {};
		numUpdaters := NumberUpdaters();
		i := 0;
		WHILE (i < numUpdaters) & (res = {}) DO
			updater := updaters[chain, i];
			ASSERT(updater # NIL, 33);
			updater.Sample(overRelax, res);
			INC(i)
		END
	END Sample;

	PROCEDURE SetAdaption* (it, end: INTEGER);
	BEGIN
		iteration := it;
		endOfAdapting := end
	END SetAdaption;

	PROCEDURE SetInitialized*;
	BEGIN
		initialized := TRUE
	END SetInitialized;

	PROCEDURE StartFinish* (depth: INTEGER; OUT start, finish: INTEGER);
		VAR
			i, numUpdaters: INTEGER;
	BEGIN
		numUpdaters := NumberUpdaters();
		i := 0;
		WHILE (i < numUpdaters) & (updaters[0, i].Depth() # depth) DO
			INC(i)
		END;
		start := i;
		WHILE (i < numUpdaters) & (updaters[0, i].Depth() = depth) DO
			INC(i)
		END;
		finish := i
	END StartFinish;

	(*	Generates initial values	*)
	PROCEDURE GenerateInits* (chain: INTEGER; fixFounder: BOOLEAN; OUT res: SET;
	OUT updater: UpdaterUpdaters.Updater);
		VAR
			i, start, finish, depth, min, max: INTEGER;
	BEGIN
		MinMaxDepth(min, max);
		(*	try to generate inits	*)
		res := {};
		depth := 1;
		WHILE depth <= max DO
			StartFinish(depth, start, finish);
			i := start;
			WHILE i < finish DO
				updater := updaters[chain, i];
				updater.GenerateInit(fixFounder, res);
				INC(i)
			END;
			INC(depth)
		END;
		(*	now generate inits again for any case that failed first time	*)
		res := {};
		depth := 1;
		WHILE (depth <= max) & (res = {}) DO
			StartFinish(depth, start, finish);
			i := start;
			WHILE (i < finish) & (res = {}) DO
				updater := updaters[chain, i];
				updater.GenerateInit(fixFounder, res);
				INC(i)
			END;
			INC(depth)
		END;
		(*	forward sampling for prediction nodes	*)
		depth := - 1;
		WHILE (depth >= min) & (res = {}) DO
			StartFinish(depth, start, finish);
			i := start;
			WHILE (i < finish) & (res = {}) DO
				updater := updaters[chain, i];
				updater.GenerateInit(fixFounder, res);
				INC(i)
			END;
			DEC(depth)
		END
	END GenerateInits;

	PROCEDURE StoreStochastics*;
		VAR
			u: UpdaterUpdaters.Updater;
			i, j, num, numChains, numUpdaters, size: INTEGER;
			stochastics: GraphStochastic.Vector;
	BEGIN
		num := 0;
		i := 0;
		numUpdaters := NumberUpdaters();
		numChains := NumberChains();
		WHILE i < numUpdaters DO
			u := updaters[0, i];
			size := u.Size();
			INC(num, size);
			INC(i)
		END;
		IF num > 0 THEN NEW(stochastics, num) ELSE stochastics := NIL END;
		i := 0;
		num := 0;
		WHILE i < numUpdaters DO
			u := updaters[0, i];
			size := u.Size();
			j := 0;
			WHILE j < size DO
				stochastics[num] := u.Node(j);
				INC(num);
				INC(j)
			END;
			INC(i)
		END;
		GraphStochastic.SetStochastics(stochastics, numChains)
	END StoreStochastics;

	PROCEDURE UnMarkDistributed*;
		VAR
			i, j, numUpdaters, size: INTEGER;
			updater: UpdaterUpdaters.Updater;
			prior: GraphStochastic.Node;
	BEGIN
		numUpdaters := NumberUpdaters();
		i := 0;
		WHILE i < numUpdaters DO
			updater := updaters[0, i];
			j := 0;
			size := updater.Size();
			WHILE j < size DO
				prior := updater.Prior(j);
				EXCL(prior.props, GraphStochastic.distributed);
				INC(j)
			END;
			INC(i)
		END
	END UnMarkDistributed;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		Clear;
	END Init;

BEGIN
	Init
END UpdaterActions.
