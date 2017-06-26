(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



  *)

MODULE UpdaterActions;

	(*	Updating actions	*)

	

	IMPORT
		Stores,
		GraphLogical, GraphMRF, GraphNodes, GraphStochastic,
		MathSort,
		UpdaterAuxillary, UpdaterUpdaters;

	TYPE
		List = POINTER TO RECORD
			updater: UpdaterUpdaters.Updater;
			next: List
		END;

	VAR
		list: List;
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
			i, numUpdaters, numChildren, totalChildren: INTEGER;
	BEGIN
		numUpdaters := NumberUpdaters();
		i := 0;
		totalChildren := 0;
		WHILE i < numUpdaters DO
			updater := updaters[0, i];
			IF updater # NIL THEN
				children := updater.Children();
				IF children # NIL THEN
					numChildren := LEN(children);
					INC(totalChildren, numChildren)
				END
			END;
			INC(i)
		END;
		RETURN totalChildren DIV numUpdaters
	END MeanNumChildren;

	PROCEDURE MedianNumChildren* (): INTEGER;
		VAR
			updater: UpdaterUpdaters.Updater;
			children: GraphStochastic.Vector;
			i, numUpdaters, numChildren: INTEGER;
			x: POINTER TO ARRAY OF REAL;
		CONST
			eps = 0.1;
	BEGIN
		numUpdaters := NumberUpdaters();
		NEW(x, numUpdaters);
		i := 0;
		WHILE i < numUpdaters DO
			updater := updaters[0, i];
			IF updater # NIL THEN
				children := updater.Children();
				IF children # NIL THEN
					numChildren := LEN(children);
				ELSE
					numChildren := 0
				END;
				x[i] := numChildren
			END;
			INC(i)
		END;
		MathSort.HeapSort(x, numUpdaters);
		RETURN SHORT(ENTIER(x[numUpdaters DIV 2] + eps))
	END MedianNumChildren;

	(*	externalize data of nodes associated with updaters	*)
	PROCEDURE AllocateLikelihoods*;
		VAR
			u: UpdaterUpdaters.Updater;
			p: GraphStochastic.Node;
			i, numUpdaters: INTEGER;
	BEGIN
		numUpdaters := NumberUpdaters();
		i := 0;
		WHILE i < numUpdaters DO
			u := updaters[0, i];
			p := u.Node(0);
			p.AllocateLikelihood;
			INC(i)
		END
	END AllocateLikelihoods;

	(*	Clears the updater list etc	*)
	PROCEDURE Clear*;
	BEGIN
		iteration := 0;
		endOfAdapting := MAX(INTEGER);
		initialized := FALSE;
		list := NIL;
		updaters := NIL
	END Clear;

	PROCEDURE Compare (updater0, updater1: UpdaterUpdaters.Updater): INTEGER;
		VAR
			depth0, depth1, len0, len1: INTEGER;
			children: GraphStochastic.Vector;
	BEGIN
		depth0 := updater0.Depth();
		depth1 := updater1.Depth();
		IF depth0 > depth1 THEN
			RETURN 1
		ELSIF depth0 < depth1 THEN
			RETURN - 1
		ELSE
			children := updater0.Children();
			IF children # NIL THEN len0 := LEN(children) ELSE len0 := 0 END;
			children := updater1.Children();
			IF children # NIL THEN len1 := LEN(children) ELSE len1 := 0 END;
			IF len0 > len1 THEN
				RETURN 1
			ELSIF len0 < len1 THEN
				RETURN - 1
			ELSE
				RETURN 0
			END
		END
	END Compare;

	PROCEDURE CreateUpdaters* (numChains: INTEGER);
		VAR
			i, j, numUpdaters: INTEGER;
			cursor: List;
	BEGIN
		numUpdaters := 0;
		cursor := list;
		WHILE cursor # NIL DO
			INC(numUpdaters); cursor := cursor.next
		END;
		IF numUpdaters > 0 THEN
			NEW(updaters, numChains);
			i := 0;
			WHILE i < numChains DO
				NEW(updaters[i], numUpdaters);
				INC(i)
			END
		END;
		cursor := list;
		i := 0;
		WHILE cursor # NIL DO
			updaters[0, i] := cursor.updater;
			j := 1;
			WHILE j < numChains DO
				updaters[j, i] := UpdaterUpdaters.CopyFrom(cursor.updater);
				INC(j)
			END;
			INC(i);
			cursor := cursor.next
		END;
		list := NIL
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
			i, j, numChains, numUpdaters, oldPos, pos: INTEGER;
			updater: UpdaterUpdaters.Updater;
			posArray: POINTER TO ARRAY OF INTEGER;
	BEGIN
		numChains := NumberChains();
		NEW(posArray, numChains);
		numUpdaters := NumberUpdaters();
		oldPos := wr.Pos();
		i := 0;
		WHILE i < numChains DO
			wr.WriteInt( - 1); INC(i)
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
			wr.WriteInt(posArray[i]); INC(i)
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

	PROCEDURE InsertConstraints*;
		VAR
			mrf, oldMRF: GraphMRF.Node;
			cursor, prev, element: List;
			updater: UpdaterUpdaters.Updater;
			prior, prevPrior: GraphStochastic.Node;
			factory: UpdaterUpdaters.Factory;
	BEGIN
		factory := UpdaterUpdaters.InstallFactory("UpdaterMRFConstrain.Install");
		oldMRF := NIL;
		prev := NIL;
		cursor := list;
		WHILE cursor # NIL DO
			updater := cursor.updater;
			prior := updater.Node(0);
			IF prior IS GraphMRF.Node THEN
				mrf := prior(GraphMRF.Node);
				mrf := mrf.components[0](GraphMRF.Node);
				IF mrf # oldMRF THEN
					IF oldMRF # NIL THEN
						IF factory.CanUpdate(prevPrior) THEN
							NEW(element);
							element.updater := factory.New(prevPrior);
							element.next := cursor;
							prev.next := element
						END
					END;
					oldMRF := mrf
				END
			ELSE
				IF oldMRF # NIL THEN
					IF factory.CanUpdate(prevPrior) THEN
						NEW(element);
						element.updater := factory.New(prevPrior);
						element.next := cursor;
						prev.next := element
					END;
					oldMRF := NIL
				END
			END;
			prev := cursor;
			prevPrior := prev.updater.Node(0);
			cursor := cursor.next
		END
	END InsertConstraints;

	(*	Internalize updaters mutable state	*)
	PROCEDURE InternalizeUpdaterData* (chain: INTEGER; VAR rd: Stores.Reader);
		VAR
			i, numChains, numUpdaters, pos: INTEGER;
			updater: UpdaterUpdaters.Updater;
	BEGIN
		numChains := NumberChains();
		numUpdaters := NumberUpdaters();
		i := 0;
		WHILE i <= chain DO
			rd.ReadInt(pos);
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

	(*	Is the chain initialized	*)
	PROCEDURE IsInitialized* (chain: INTEGER): BOOLEAN;
		VAR
			isInitialized: BOOLEAN;
			i, numUpdaters: INTEGER;
			node: GraphStochastic.Node;
			updater: UpdaterUpdaters.Updater;
	BEGIN
		isInitialized := TRUE;
		numUpdaters := NumberUpdaters();
		i := 0;
		WHILE(i < numUpdaters) & isInitialized DO
			updater := updaters[chain, i];
			node := updater.Prior(0);
			isInitialized := updater.IsInitialized() OR (GraphStochastic.hidden IN node.props);
			INC(i)
		END;
		RETURN isInitialized
	END IsInitialized;

	(*	load chain into graph	*)
	PROCEDURE LoadSamples* (chain: INTEGER);
		VAR
			i, numChains, numUpdaters: INTEGER;
			updater: UpdaterUpdaters.Updater;
	BEGIN
		numChains := NumberChains();
		numUpdaters := NumberUpdaters();
		i := 0;
		chain := chain MOD numChains;
		WHILE i < numUpdaters DO
			updater := updaters[chain, i];
			updater.LoadSample;
			INC(i)
		END
	END LoadSamples;

	PROCEDURE CanDistribute (updater: UpdaterUpdaters.Updater; avNum: INTEGER): BOOLEAN;
		VAR
			canDistribute: BOOLEAN;
			i, numChild: INTEGER;
			children: GraphStochastic.Vector;
			prior: GraphStochastic.Node;
			dependents: GraphLogical.List;
	BEGIN
		WITH updater: UpdaterAuxillary.UpdaterUV DO
			canDistribute := FALSE
		ELSE
			children := updater.Children();
			prior := updater.Prior(0);
			IF children # NIL THEN numChild := LEN(children) ELSE numChild := 0 END;
			canDistribute := (numChild > 2 * avNum) OR (GraphNodes.maxStochDepth = 1);
			i := 0;
			WHILE ~canDistribute & (i < numChild) DO
				canDistribute := children[i] IS GraphMRF.Node;
				INC(i)
			END;
			dependents := prior.dependents;
			WHILE canDistribute & (dependents # NIL) DO
				canDistribute := ~(GraphLogical.stochParent IN dependents.node.props);
				dependents := dependents.next
			END
		END;
		RETURN canDistribute
	END CanDistribute;

	PROCEDURE MarkDistributed*;
		VAR
			i, j, numUpdaters, avNumChild, size: INTEGER;
			updater: UpdaterUpdaters.Updater;
			prior: GraphStochastic.Node;
	BEGIN
		avNumChild := MeanNumChildren();
		numUpdaters := NumberUpdaters();
		i := 0;
		WHILE i < numUpdaters DO
			updater := updaters[0, i];
			IF CanDistribute(updater, avNumChild) THEN
				j := 0;
				size := updater.Size();
				WHILE j < size DO
					prior := updater.Prior(j);
					prior.SetProps(prior.props + {GraphStochastic.distributed});
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

	(*	Stores new updater in updaterList	*)
	PROCEDURE RegisterUpdater* (updater: UpdaterUpdaters.Updater);
		VAR
			cursor, element: List;
	BEGIN
		NEW(element);
		element.updater := updater;
		IF (list = NIL) OR (Compare(updater, list.updater) > 0) THEN
			element.next := list;
			list := element
		ELSE
			cursor := list;
			LOOP
				IF cursor.next = NIL THEN
					element.next := NIL;
					cursor.next := element;
					EXIT
				ELSIF Compare(updater, cursor.next.updater) > 0 THEN
					element.next := cursor.next;
					cursor.next := element;
					EXIT
				END;
				cursor := cursor.next
			END
		END
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
					updaters[0, i].LoadSample;
					updaters[0, i] := updater;
					updaters[0, i].StoreSample;
					j := 0;
					WHILE j < numChains DO
						updaters[j, i].LoadSample;
						updaters[j, i] := UpdaterUpdaters.CopyFrom(updater);
						updaters[j, i].StoreSample;
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
	PROCEDURE GenerateInits* (chain: INTEGER; fixFounder: BOOLEAN; OUT res: SET; OUT updater: UpdaterUpdaters.Updater);
		VAR
			i, start, finish, depth, min, max: INTEGER;
	BEGIN
		res := {};
		MinMaxDepth(min, max);
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

	(*	stores chain from graph	*)
	PROCEDURE StoreSamples* (chain: INTEGER);
		VAR
			i, numUpdaters: INTEGER;
			updater: UpdaterUpdaters.Updater;
	BEGIN
		numUpdaters := NumberUpdaters();
		i := 0;
		WHILE i < numUpdaters DO
			updater := updaters[chain, i];
			updater.StoreSample;
			INC(i)
		END
	END StoreSamples;

	PROCEDURE StoreStochastics*;
		VAR
			u: UpdaterUpdaters.Updater;
			i, j, num, numUpdaters, size: INTEGER;
			stochastics: GraphStochastic.Vector;
	BEGIN
		num := 0;
		i := 0;
		numUpdaters := NumberUpdaters();
		WHILE i < numUpdaters DO
			u := updaters[0, i];
			size := u.Size();
			INC(num, size);
			INC(i)
		END;
		IF num > 0 THEN NEW(stochastics, num) END;
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
		GraphStochastic.SetStochastics(stochastics)
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
				prior.SetProps(prior.props - {GraphStochastic.distributed});
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
