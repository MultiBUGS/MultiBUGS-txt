(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)
MODULE BugsParallel;

	IMPORT
		SYSTEM, Stores := Stores64,
		BugsPartition,
		GraphConjugateMV, GraphDummy, GraphDummyMV, GraphLogical,
		GraphMRF, GraphMultivariate, GraphNodes, GraphStochastic, GraphVector,
		UpdaterActions, UpdaterEmpty, UpdaterUpdaters;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		debug*: BOOLEAN;
		globalDeviance: POINTER TO ARRAY OF GraphStochastic.Vector;
		globalStochs: POINTER TO ARRAY OF GraphStochastic.Vector;
		globalUpdaters: POINTER TO ARRAY OF ARRAY OF INTEGER;
		globalId: POINTER TO ARRAY OF INTEGER;

	CONST
		undefined = MAX(INTEGER);
		dummyType = 0;
		dummyMVType = 1;
		thisCore = 30;
		write = GraphStochastic.mark;

		(*	Checks that markov blanket of stoch can be evaluated on this core and does not depend
		on values of parameters on other cores	*)
	PROCEDURE ThisCore (stoch: GraphStochastic.Node): BOOLEAN;
		VAR
			bool: BOOLEAN;

		PROCEDURE OnThisCore (p: GraphStochastic.Node): BOOLEAN;
			VAR
				this: BOOLEAN;
				list: GraphStochastic.List;
				q: GraphStochastic.Node;
			CONST
				all = TRUE;
		BEGIN
			this := {GraphNodes.data, thisCore} * p.props # {};
			IF this THEN
				list := GraphStochastic.Parents(p, all);
				WHILE this & (list # NIL) DO
					q := list.node;
					this := {thisCore, GraphStochastic.hidden} * q.props # {};
					list := list.next
				END
			END;
			RETURN this
		END OnThisCore;

		PROCEDURE ChildrenThisCore (children: GraphStochastic.Vector): BOOLEAN;
			VAR
				i, num: INTEGER;
				child: GraphStochastic.Node;
				this: BOOLEAN;
		BEGIN
			this := TRUE;
			IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
			i := 0;
			WHILE this & (i < num) DO
				child := children[i];
				this := OnThisCore(child);
				INC(i)
			END;
			RETURN this
		END ChildrenThisCore;

	BEGIN
		bool := OnThisCore(stoch) & ChildrenThisCore(stoch.children);
		RETURN bool
	END ThisCore;

	PROCEDURE Externalize (p: GraphNodes.Node; VAR wr: Stores.Writer);
		VAR
			props: SET;
	BEGIN
		(*	do not write out marks to file	*)
		props := p.props;
		p.props := props - {write, thisCore};
		p.Externalize(wr);
		p.props := props
	END Externalize;

	PROCEDURE MarkNode (node: GraphNodes.Node; marks: SET);
		VAR
			p: GraphStochastic.Node;
			l: GraphLogical.Node;
			i, size: INTEGER;
			install: ARRAY 128 OF CHAR;
	BEGIN
		WITH node: GraphConjugateMV.Node DO
			i := 0;
			size := node.Size();
			WHILE i < size DO
				p := node.components[i];
				p.props := p.props + marks;
				INC(i)
			END
		|node: GraphMultivariate.Node DO
			node.props := node.props + marks;
			i := 0;
			size := node.Size();
			WHILE i < size DO
				p := node.components[i];
				IF GraphNodes.data IN p.props THEN
					p.props := p.props + marks
				ELSIF p IS GraphMRF.Node THEN
					p.Install(install);
					IF install = "SpatialCARProper.Install" THEN
						p.props := p.props + marks
					END
				END;
				INC(i)
			END
		|node: GraphVector.Node DO
			i := 0;
			size := node.Size();
			WHILE i < size DO
				l := node.components[i];
				l.props := l.props + marks;
				INC(i)
			END
		ELSE
			node.props := node.props + marks
		END
	END MarkNode;

	PROCEDURE UnmarkNode (node: GraphNodes.Node; marks: SET);
		VAR
			p: GraphStochastic.Node;
			l: GraphLogical.Node;
			i, size: INTEGER;
			install: ARRAY 128 OF CHAR;
	BEGIN
		WITH node: GraphConjugateMV.Node DO
			i := 0;
			size := node.Size();
			WHILE i < size DO
				p := node.components[i];
				p.props := p.props - marks;
				INC(i)
			END
		|node: GraphMultivariate.Node DO
			node.props := node.props + marks;
			i := 0;
			size := node.Size();
			WHILE i < size DO
				p := node.components[i];
				IF GraphNodes.data IN p.props THEN
					p.props := p.props - marks
				ELSIF p IS GraphMRF.Node THEN
					p.Install(install);
					IF install = "SpatialCARProper.Install" THEN
						p.props := p.props - marks
					END
				END;
				INC(i)
			END
		|node: GraphVector.Node DO
			i := 0;
			size := node.Size();
			WHILE i < size DO
				l := node.components[i];
				l.props := l.props - marks;
				INC(i)
			END
		ELSE
			node.props := node.props - marks
		END
	END UnmarkNode;

	(*	Only children marked as thisCore survive thinning, distributed nodes are marked on all cores so only
	include once on the zero worker	*)
	PROCEDURE ThinChildren (children: GraphStochastic.Vector; worker: INTEGER): GraphStochastic.Vector;
		VAR
			i, num, len, j: INTEGER;
			vector: GraphStochastic.Vector;
			child: GraphStochastic.Node;
	BEGIN
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		len := 0;
		WHILE i < num DO
			child := children[i];
			IF thisCore IN child.props THEN
				IF ~(GraphStochastic.distributed IN child.props) OR (worker = 0) THEN
					INC(len)
				END
			END;
			INC(i)
		END;
		IF len > 0 THEN NEW(vector, len) ELSE RETURN NIL END;
		i := 0;
		j := 0;
		WHILE i < num DO
			child := children[i];
			IF thisCore IN child.props THEN
				IF ~(GraphStochastic.distributed IN child.props) OR (worker = 0) THEN
					vector[j] := child;
					INC(j)
				END
			END;
			INC(i)
		END;
		RETURN vector
	END ThinChildren;

	PROCEDURE ExternalizeDeviance (rank: INTEGER; VAR wr: Stores.Writer);
		VAR
			i, len: INTEGER;
			p: GraphStochastic.Node;
			observations: GraphStochastic.Vector;
	BEGIN
		IF globalDeviance # NIL THEN
			observations := globalDeviance[rank];
			IF observations # NIL THEN
				len := LEN(observations);
				wr.WriteInt(len);
				i := 0;
				WHILE i < len DO
					p := observations[i];
					GraphNodes.ExternalizePointer(p, wr);
					INC(i)
				END
			END
		END
	END ExternalizeDeviance;

	PROCEDURE ExternalizeLogicalAddresses (VAR wr: Stores.Writer);
		VAR
			add, i, num: INTEGER;
			nodes: GraphLogical.Vector;
			p: GraphLogical.Node;
	BEGIN
		nodes := GraphLogical.nodes;
		IF nodes # NIL THEN
			num := LEN(nodes);
			i := 0;
			WHILE i < num DO
				p := nodes[i];
				IF write IN p.props THEN add := SYSTEM.VAL(INTEGER, p); wr.WriteInt(add) END;
				INC(i)
			END
		END
	END ExternalizeLogicalAddresses;

	PROCEDURE CountDataPointers (): INTEGER;
		VAR
			count, i, j, numStoch, workersPerChain: INTEGER;
			p: GraphStochastic.Node;
	BEGIN
		count := 0;
		IF globalDeviance # NIL THEN
			workersPerChain := LEN(globalDeviance);
			i := 0;
			WHILE i < workersPerChain DO
				IF globalDeviance[i] # NIL THEN
					numStoch := LEN(globalDeviance[i]);
					j := 0;
					WHILE j < numStoch DO
						p := globalDeviance[i, j];
						IF write IN p.props THEN INC(count) END;
						INC(j)
					END;
					INC(i)
				END
			END
		END;
		RETURN count
	END CountDataPointers;

	PROCEDURE ExternalizeDataAddresses (VAR wr: Stores.Writer);
		VAR
			address, i, j, numStoch, workersPerChain: INTEGER;
			p: GraphStochastic.Node;
	BEGIN
		IF globalDeviance # NIL THEN
			workersPerChain := LEN(globalDeviance);
			i := 0;
			WHILE i < workersPerChain DO
				IF globalDeviance[i] # NIL THEN
					numStoch := LEN(globalDeviance[i]);
					j := 0;
					WHILE j < numStoch DO
						p := globalDeviance[i, j];
						IF write IN p.props THEN address := SYSTEM.VAL(INTEGER, p); wr.WriteInt(address) END;
						INC(j)
					END;
					INC(i)
				END
			END
		END
	END ExternalizeDataAddresses;

	PROCEDURE ExternalizeDataPointers (VAR wr: Stores.Writer);
		VAR
			i, j, numStoch, workersPerChain: INTEGER;
			p: GraphStochastic.Node;
	BEGIN
		IF globalDeviance # NIL THEN
			workersPerChain := LEN(globalDeviance);
			i := 0;
			WHILE i < workersPerChain DO
				IF globalDeviance[i] # NIL THEN
					numStoch := LEN(globalDeviance[i]);
					j := 0;
					WHILE j < numStoch DO
						p := globalDeviance[i, j];
						IF write IN p.props THEN GraphNodes.ExternalizePointer(p, wr) END;
						INC(j)
					END;
					INC(i)
				END
			END
		END
	END ExternalizeDataPointers;

	PROCEDURE ExternalizeDataInternals (VAR wr: Stores.Writer);
		VAR
			i, j, numStoch, workersPerChain: INTEGER;
			p: GraphStochastic.Node;
	BEGIN
		IF globalDeviance # NIL THEN
			workersPerChain := LEN(globalDeviance);
			i := 0;
			WHILE i < workersPerChain DO
				IF globalDeviance[i] # NIL THEN
					numStoch := LEN(globalDeviance[i]);
					j := 0;
					WHILE j < numStoch DO
						p := globalDeviance[i, j];
						IF (write IN p.props) & (GraphNodes.data IN p.props) THEN p.Externalize(wr) END;
						INC(j)
					END;
					INC(i)
				END
			END
		END
	END ExternalizeDataInternals;

	PROCEDURE CountStochasticPointers (): INTEGER;
		VAR
			len: INTEGER;
	BEGIN
		len := 0;
		IF globalStochs # NIL THEN
			len := LEN(globalStochs[0])
		END;
		RETURN len
	END CountStochasticPointers;

	PROCEDURE CountLogicalPointers (): INTEGER;
		VAR
			i, num, count: INTEGER;
			nodes: GraphLogical.Vector;
	BEGIN
		count := 0;
		nodes := GraphLogical.nodes;
		IF nodes # NIL THEN
			num := LEN(nodes);
			i := 0; WHILE i < num DO IF write IN nodes[i].props THEN INC(count) END; INC(i) END
		END;
		RETURN count
	END CountLogicalPointers;

	PROCEDURE ExternalizeLogicalPointers (VAR wr: Stores.Writer);
		VAR
			i, num: INTEGER;
			nodes: GraphLogical.Vector;
			p: GraphLogical.Node;
	BEGIN
		nodes := GraphLogical.nodes;
		IF nodes # NIL THEN
			num := LEN(nodes);
			i := 0;
			WHILE i < num DO
				p := nodes[i];
				IF write IN p.props THEN GraphNodes.ExternalizePointer(p, wr) END; INC(i)
			END
		END
	END ExternalizeLogicalPointers;

	(*	Writes out pointers of stochastic nodes in model, fixed effect pointers writen multiple times	*)
	PROCEDURE ExternalizeStochasticPointers (VAR wr: Stores.Writer);
		VAR
			i, j, label, num, workersPerChain: INTEGER;
			p: GraphStochastic.Node;
	BEGIN
		IF globalStochs # NIL THEN
			workersPerChain := LEN(globalStochs);
			num := LEN(globalStochs[0]);
			i := 0;
			WHILE i < workersPerChain DO
				j := 0;
				WHILE j < num DO
					p := globalStochs[i, j];
					(*	problem with dStable so always write out	*)
					IF (write IN p.props) OR (GraphStochastic.noPDF IN p.props) THEN
						GraphNodes.ExternalizePointer(p, wr)
					ELSE
						(*	write out the dummy node but change its type if multivariate	*)
						label := GraphNodes.Label(p);
						wr.WriteInt(label);
						IF p IS GraphMultivariate.Node THEN
							wr.WriteInt(dummyMVType)
						ELSE
							wr.WriteInt(dummyType)
						END
					END;
					INC(j)
				END;
				INC(i)
			END
		END
	END ExternalizeStochasticPointers;

	(*
	Writes out internal fields of stochastic nodes in model. For fixed effects data only written once.
	Nodes' likelihoods thinned for fixed effects
	*)
	PROCEDURE ExternalizeStochasticInternals (rank: INTEGER; OUT this: BOOLEAN;
	VAR wr: Stores.Writer);
		VAR
			i, j, label, num, workersPerChain: INTEGER;
			p, dummy: GraphStochastic.Node;
			dummyMV: GraphMultivariate.Node;
			thinned, children: GraphStochastic.Vector;
			dependents: GraphLogical.Vector;
	BEGIN
		(*	create dummy nodes so that their internal fields can be written out if needed	*)
		dummy := GraphDummy.fact.New();
		dummyMV := GraphDummyMV.fact.New();
		this := TRUE;
		IF globalStochs # NIL THEN
			workersPerChain := LEN(globalStochs);
			num := LEN(globalStochs[0]);
			i := 0;
			WHILE i < workersPerChain DO
				j := 0;
				WHILE j < num DO
					p := globalStochs[i, j];
					IF GraphStochastic.distributed IN p.props THEN
						(*	only write out first time	*)
						IF i = 0 THEN
							children := p.children;
							thinned := ThinChildren(children, rank);
							p.SetChildren(thinned);
							this := this & ThisCore(p);
							Externalize(p, wr);
							p.SetChildren(children);
						END
					ELSE
						(*	problem with dStable so always write out	*)
						IF (write IN p.props) OR (GraphStochastic.noPDF IN p.props) THEN
							IF i = rank THEN
								(*	updated on this core so need likelihood and prior	*)
								this := this & ThisCore(p);
								Externalize(p, wr)
							ELSE
								(*	not updated on this core so do not need its likelihood but need its prior
								except for multivariate case of mixed data/stochastic?	*)
								children := p.children;
								dependents := p.dependents;
								IF (children # NIL) & ~(write IN children[0].props) THEN
									p.SetChildren(NIL);
									p.SetDependents(NIL);
								END;
								this := this & ThisCore(p);
								Externalize(p, wr);
								p.SetChildren(children);
								p.SetDependents(dependents)
							END
						ELSE
							(*	not updated on this core and not used as likelihood, therefore write dummy node	*)
							WITH p: GraphMultivariate.Node DO
								dummyMV.SetComponent(p.components, p.index);
								Externalize(dummyMV, wr);
								dummyMV.SetComponent(NIL, - 1)
							ELSE
								Externalize(dummy, wr)
							END
						END
					END;
					INC(j)
				END;
				INC(i)
			END
		END
	END ExternalizeStochasticInternals;

	PROCEDURE ExternalizeLogicalInternals (VAR wr: Stores.Writer);
		VAR
			i, num: INTEGER;
			p: GraphLogical.Node;
			nodes: GraphLogical.Vector;
	BEGIN
		nodes := GraphLogical.nodes;
		IF nodes # NIL THEN
			num := LEN(nodes);
			i := 0;
			WHILE i < num DO
				p := nodes[i];
				IF write IN p.props THEN p.Externalize(wr) END; INC(i)
			END
		END
	END ExternalizeLogicalInternals;

	PROCEDURE ExternalizeUpdaters (rank: INTEGER; VAR wr: Stores.Writer);
		VAR
			i, label, len, size: INTEGER;
			u: UpdaterUpdaters.Updater;
	BEGIN
		i := 0;
		len := LEN(globalUpdaters, 1);
		wr.WriteInt(len);
		WHILE i < len DO
			label := globalUpdaters[rank, i];
			IF label # undefined THEN
				u := UpdaterActions.updaters[0, label];
			ELSE
				label := globalUpdaters[0, i];
				IF label # undefined THEN
					u := UpdaterActions.updaters[0, label];
					size := u.Size();
				ELSE
					size := 1
				END;
				u := UpdaterEmpty.New(size)
			END;
			UpdaterUpdaters.ExternalizePointer(u, wr);
			INC(i)
		END
	END ExternalizeUpdaters;

	PROCEDURE ExternalizeUpdatersInternals (rank, chain: INTEGER; VAR wr: Stores.Writer);
		VAR
			i, label, len, size: INTEGER;
			u: UpdaterUpdaters.Updater;
	BEGIN
		i := 0;
		len := LEN(globalUpdaters, 1);
		WHILE i < len DO
			label := globalUpdaters[rank, i];
			IF label # undefined THEN
				u := UpdaterActions.updaters[chain, label];
			ELSE
				label := globalUpdaters[0, i];
				IF label # undefined THEN
					u := UpdaterActions.updaters[0, label];
					size := u.Size()
				ELSE
					size := 1
				END;
				u := UpdaterEmpty.New(size)
			END;
			UpdaterUpdaters.Externalize(u, wr);
			INC(i)
		END
	END ExternalizeUpdatersInternals;

	PROCEDURE ClearMarks;
		VAR
			i, j, num, numStoch, workersPerChain: INTEGER;
			p: GraphStochastic.Node;
			dependents: GraphLogical.Vector;
			logical: GraphLogical.Node;
	BEGIN
		IF globalStochs # NIL THEN
			workersPerChain := LEN(globalStochs);
			numStoch := LEN(globalStochs[0]);
			i := 0;
			WHILE i < workersPerChain DO
				j := 0;
				WHILE j < numStoch DO
					p := globalStochs[i, j];
					UnmarkNode(p, {write, thisCore});
					INC(j)
				END;
				INC(i)
			END
		END;
		dependents := GraphLogical.nodes;
		IF dependents # NIL THEN
			num := LEN(dependents);
			i := 0;
			WHILE i < num DO
				logical := dependents[i];
				UnmarkNode(logical, {write, thisCore});
				INC(i)
			END
		END;
		IF globalDeviance # NIL THEN
			workersPerChain := LEN(globalDeviance);
			i := 0;
			WHILE i < workersPerChain DO
				IF globalDeviance[i] # NIL THEN
					numStoch := LEN(globalDeviance[i]);
					j := 0;
					WHILE j < numStoch DO
						p := globalDeviance[i, j];
						UnmarkNode(p, {write, thisCore});
						INC(j)
					END;
					INC(i)
				END
			END
		END
	END ClearMarks;

	(*	Mark markov blanket of nodes updated on this core. The write mark meanis
	that the internal fields of the marked node are needed and thisCore means that the node will
	be updated on core rank	*)
	PROCEDURE MarkMarkovBlanket (rank: INTEGER);
		VAR
			i, j, num, label, len, size: INTEGER;
			child, p, q: GraphStochastic.Node;
			list: GraphLogical.List;
			log: GraphLogical.Node;
			children: GraphStochastic.Vector;
			u: UpdaterUpdaters.Updater;
		CONST
			all = TRUE;
	BEGIN
		(*	mark observations	*)
		IF globalDeviance # NIL THEN
			IF globalDeviance[rank] # NIL THEN
				num := LEN(globalDeviance[rank]);
				i := 0;
				WHILE i < num DO
					p := globalDeviance[rank, i];
					MarkNode(p, {write, thisCore});
					list := GraphLogical.Parents(p, all);
					WHILE list # NIL DO
						log := list.node;
						MarkNode(log, {write});
						list := list.next
					END;
					INC(i)
				END
			END
		END;
		(*	mark prior and do something special for auxillary updaters for which p # q	*)
		i := 0;
		len := LEN(globalUpdaters, 1);
		WHILE i < len DO
			label := globalUpdaters[rank, i];
			IF label # undefined THEN
				u := UpdaterActions.updaters[0, label];
				size := u.Size();
				j := 0;
				WHILE j < size DO
					p := u.Node(j);
					q := u.Prior(j);
					MarkNode(p, {write, thisCore});
					list := GraphLogical.Parents(p, all);
					WHILE list # NIL DO
						log := list.node;
						MarkNode(log, {write});
						list := list.next
					END;
					IF p # q THEN
						MarkNode(q, {write});
						list := GraphLogical.Parents(q, all);
						WHILE list # NIL DO
							log := list.node;
							MarkNode(log, {write});
							list := list.next
						END
					END;
					INC(j)
				END
			END;
			INC(i)
		END;
		(*	mark likelihoods of random effects	*)
		IF globalStochs # NIL THEN
			IF globalStochs[rank] # NIL THEN
				num := LEN(globalStochs[rank]);
				i := 0;
				WHILE i < num DO
					p := globalStochs[rank, i];
					IF ~(GraphStochastic.distributed IN p.props) THEN
						children := p.children;
						IF children # NIL THEN
							len := LEN(children);
							j := 0;
							WHILE j < len DO
								child := children[j];
								MarkNode(child, {write});
								list := GraphLogical.Parents(child, all);
								WHILE list # NIL DO
									log := list.node;
									MarkNode(log, {write});
									list := list.next
								END;
								INC(j)
							END
						END
					END;
					INC(i)
				END
			END
		END
	END MarkMarkovBlanket;

	PROCEDURE ExternalizeId (id: POINTER TO ARRAY OF INTEGER; VAR wr: Stores.Writer);
		VAR
			i, len: INTEGER;
	BEGIN
		i := 0;
		IF id # NIL THEN len := LEN(id) ELSE len := 0 END;
		wr.WriteInt(len);
		WHILE i < len DO
			wr.WriteInt(id[i]);
			INC(i)
		END
	END ExternalizeId;

	PROCEDURE StoreStochastic;
		VAR
			i, j, k, workersPerChain, len, num, size, label: INTEGER;
			p: GraphStochastic.Node;
			u: UpdaterUpdaters.Updater;
	BEGIN
		workersPerChain := LEN(globalUpdaters, 0);
		num := LEN(globalUpdaters, 1);
		j := 0;
		len := 0;
		WHILE j < num DO
			label := globalUpdaters[0, j];
			IF label # undefined THEN
				u := UpdaterActions.updaters[0, label];
				size := u.Size();
			ELSE
				size := 1
			END;
			INC(len, size);
			INC(j)
		END;
		NEW(globalStochs, workersPerChain);
		i := 0; WHILE i < workersPerChain DO NEW(globalStochs[i], len); INC(i) END;
		i := 0;
		WHILE i < workersPerChain DO
			j := 0;
			len := 0;
			WHILE j < num DO
				label := globalUpdaters[i, j];
				IF label # undefined THEN
					u := UpdaterActions.updaters[0, label];
					size := u.Size()
				ELSE (*	no updater add in dummy variables	*)
					label := globalUpdaters[0, j];
					IF label # undefined THEN
						u := UpdaterActions.updaters[0, label];
						size := u.Size()
					ELSE
						size := 1
					END;
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
		globalId := NIL;
		globalDeviance := NIL
	END Clear;

	PROCEDURE Distribute* (workersPerChain: INTEGER);
		VAR
			i, numUpdaters: INTEGER;
	BEGIN
		IF workersPerChain = 1 THEN
			numUpdaters := UpdaterActions.NumberUpdaters();
			IF numUpdaters > 0 THEN
				NEW(globalUpdaters, 1, numUpdaters)
			ELSE
				globalUpdaters := NIL
			END;
			i := 0;
			WHILE i < numUpdaters DO globalUpdaters[0, i] := i; INC(i) END;
			globalDeviance := BugsPartition.DistributeObservations(globalUpdaters);
			globalId := NIL
		ELSE
			BugsPartition.DistributeUpdaters(workersPerChain, globalUpdaters, globalId);
			globalDeviance := BugsPartition.DistributeObservations(globalUpdaters);
			BugsPartition.DistributeCensored(globalDeviance, globalUpdaters, globalId)
		END;
		StoreStochastic
	END Distribute;

	PROCEDURE MonitoredNodes* (OUT col, row: POINTER TO ARRAY OF INTEGER;
	OUT logicals: GraphLogical.Vector);
		VAR
			i, j, len, num, workersPerChain: INTEGER;
			p: GraphStochastic.Node;
			q: GraphNodes.Node;
			nodes: GraphLogical.Vector;
	BEGIN
		workersPerChain := LEN(globalStochs);
		len := LEN(globalStochs[0]);
		i := 0;
		WHILE i < workersPerChain DO
			j := 0;
			WHILE j < len DO (*	any node that has its represenative marked is also marked	*)
				p := globalStochs[i, j];
				q := p.Representative();
				IF GraphStochastic.mark IN q.props THEN
					INCL(p.props, GraphStochastic.mark)
				END;
				INC(j)
			END;
			INC(i)
		END;
		num := 0;
		i := 0;
		WHILE i < workersPerChain DO
			j := 0;
			WHILE j < len DO
				p := globalStochs[i, j];
				IF GraphStochastic.mark IN p.props THEN
					IF (i = 0) OR (globalStochs[0, j] # globalStochs[i, j]) THEN
						INC(num)
					END
				END;
				INC(j)
			END;
			INC(i)
		END;
		IF num > 0 THEN
			NEW(col, num); NEW(row, num)
		ELSE
			col := NIL; row := NIL
		END;
		num := 0;
		i := 0;
		WHILE i < workersPerChain DO
			j := 0;
			WHILE j < len DO
				p := globalStochs[i, j];
				IF GraphStochastic.mark IN p.props THEN
					IF (i = 0) OR (globalStochs[0, j] # globalStochs[i, j]) THEN
						col[num] := i;
						row[num] := j;
						INC(num)
					END
				END;
				INC(j)
			END;
			INC(i)
		END;
		logicals := NIL;
		nodes := GraphLogical.nodes;
		IF nodes # NIL THEN
			num := LEN(nodes);
			i := 0; j := 0;
			WHILE i < num DO IF GraphStochastic.mark IN nodes[i].props THEN INC(j) END; INC(i) END;
			IF j > 0 THEN
				NEW(logicals, j);
				i := 0; j := 0;
				WHILE i < num DO
					IF GraphStochastic.mark IN nodes[i].props THEN logicals[j] := nodes[i]; INC(j) END;
					INC(i)
				END;
			END
		END
	END MonitoredNodes;

	PROCEDURE NumStochastics* (): INTEGER;
		VAR
			numStochastics: INTEGER;
	BEGIN
		numStochastics := LEN(globalStochs) * LEN(globalStochs[0]);
		RETURN numStochastics
	END NumStochastics;

	PROCEDURE RecvMonitored* (IN values: ARRAY OF REAL; IN col, row: ARRAY OF INTEGER);
		VAR
			i, c, r, numMonitored: INTEGER;
			p: GraphStochastic.Node;
	BEGIN
		i := 0;
		numMonitored := LEN(col);
		WHILE i < numMonitored DO
			c := col[i];
			r := row[i];
			p := globalStochs[c, r];
			p.value := values[i];
			INC(i)
		END
	END RecvMonitored;

	PROCEDURE RecvSample* (IN values: ARRAY OF REAL);
		VAR
			i, j, numCol, numRow: INTEGER;
	BEGIN
		numCol := LEN(globalStochs);
		numRow := LEN(globalStochs[0]);
		i := 0;
		WHILE i < numCol DO
			j := 0;
			WHILE j < numRow DO
				globalStochs[i, j].value := values[i * numRow + j];
				INC(j)
			END;
			INC(i)
		END
	END RecvSample;

	PROCEDURE Stochastic* (col, row: INTEGER): GraphStochastic.Node;
	BEGIN
		RETURN globalStochs[row, col]
	END Stochastic;

	PROCEDURE Write* (rank, numChains: INTEGER; OUT this: BOOLEAN; VAR wr: Stores.Writer);
		VAR
			i, chain, workersPerChain, numLogicalPointers, numStochasticPointers, numDataPointers: INTEGER;
			pos, uEndPos: LONGINT;
			uPos: POINTER TO ARRAY OF LONGINT;
			dummy: GraphStochastic.Node;
			dummyMV: GraphMultivariate.Node;
			filter: SET;
	BEGIN
		ClearMarks;
		filter := GraphStochastic.dependentsFilter;
		GraphStochastic.FilterDependents({write});
		dummy := GraphDummy.fact.New();
		dummy.Init;
		dummyMV := GraphDummyMV.fact.New();
		dummyMV.Init;
		dummyMV.SetComponent(NIL, - 1);
		MarkMarkovBlanket(rank);
		numStochasticPointers := CountStochasticPointers();
		numLogicalPointers := CountLogicalPointers();
		numDataPointers := CountDataPointers();
		workersPerChain := LEN(globalUpdaters, 0);
		wr.WriteBool(debug);
		wr.WriteInt(workersPerChain);
		wr.WriteInt(numChains);
		wr.WriteInt(numStochasticPointers);
		wr.WriteInt(numLogicalPointers);
		wr.WriteInt(numDataPointers);
		ExternalizeId(globalId, wr);
		(*	Write out a block of pointers to nodes in the model needed on this core. This block might
		contain duplicate nodes but when the worker reads this block it will remove any duplicates	*)
		GraphNodes.BeginExternalize(wr);
		(*	write out a node of type GraphDummy so that this becomes type zero in type index	*)
		GraphNodes.ExternalizePointer(dummy, wr);
		(*	write out a node of type GraphDummyMV so that this becomes type one in type index	*)
		GraphNodes.ExternalizePointer(dummyMV, wr);
		(*	write out parameters	*)
		ExternalizeStochasticPointers(wr);
		(*	write out logical nodes	*)
		ExternalizeLogicalPointers(wr);
		(*	write out data pointers	*)
		ExternalizeDataPointers(wr);
		IF debug THEN
			ExternalizeLogicalAddresses(wr);
			ExternalizeDataAddresses(wr)
		END;
		ExternalizeDeviance(rank, wr);
		(*	Write out internal fields of block of pointers. For duplicate pointers only write internal fields
		for the first time 	*)
		(*	write out data for dummy node so that it can be read back in	*)
		dummy.Externalize(wr);
		(*	write out data for dummy MV node so that it can be read back in	*)
		dummyMV.Externalize(wr);
		ExternalizeStochasticInternals(rank, this, wr);
		ExternalizeLogicalInternals(wr);
		ExternalizeDataInternals(wr);
		ClearMarks;
		UpdaterUpdaters.BeginExternalize(wr);
		ExternalizeUpdaters(rank, wr);
		NEW(uPos, numChains);
		(*	write out token positional info	*)
		pos := wr.Pos();
		chain := 0;
		WHILE chain < numChains DO
			uPos[chain] := - 1;
			wr.WriteLong(uPos[chain]);
			INC(chain)
		END;
		uEndPos := - 1;
		wr.WriteLong(uEndPos);
		chain := 0;
		WHILE chain < numChains DO
			uPos[chain] := wr.Pos();
			ExternalizeUpdatersInternals(rank, chain, wr);
			GraphStochastic.LoadValues(chain);
			i := 0; WHILE i < numStochasticPointers DO wr.WriteReal(globalStochs[rank, i].value); INC(i) END;
			INC(chain)
		END;
		GraphStochastic.LoadValues(0);
		uEndPos := wr.Pos();
		(*	write out actual positional info	*)
		wr.SetPos(pos);
		chain := 0;
		WHILE chain < numChains DO
			wr.WriteLong(uPos[chain]);
			INC(chain)
		END;
		wr.WriteLong(uEndPos);
		wr.SetPos(uEndPos);
		UpdaterUpdaters.EndExternalize(wr);
		GraphNodes.EndExternalize(wr);
		GraphStochastic.FilterDependents(filter)
	END Write;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		debug := FALSE
	END Init;

BEGIN
	Init
END BugsParallel.

