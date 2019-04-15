(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)
MODULE BugsParallel;

	IMPORT
		SYSTEM, Stores,
		BugsIndex, BugsNames, BugsPartition,
		GraphConjugateMV, GraphDummy, GraphDummyMV, GraphLogical, 
		GraphMRF, GraphMultivariate, GraphNodes, GraphStochastic, GraphVector,
		UpdaterActions, UpdaterEmpty, UpdaterUpdaters;

	TYPE
		Clearer = POINTER TO RECORD(BugsNames.ElementVisitor) END;

		MarkLogical = POINTER TO RECORD(BugsNames.ElementVisitor) END;

		Writer = POINTER TO RECORD(BugsNames.ElementVisitor)
			wr: Stores.Writer;
			internals, debug: BOOLEAN
		END;

		Counter = POINTER TO RECORD(BugsNames.ElementVisitor)
			num: INTEGER
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		debug*: BOOLEAN;
		globalDeviance: POINTER TO ARRAY OF GraphStochastic.Vector;
		globalStochs: POINTER TO ARRAY OF ARRAY OF GraphStochastic.Node;
		globalUpdaters: POINTER TO ARRAY OF ARRAY OF INTEGER;
		globalId: POINTER TO ARRAY OF INTEGER;

	CONST
		undefined = MAX(INTEGER);
		dummyType = 0;
		dummyMVType = 1;
		thisCore = GraphStochastic.likelihood;
		write = GraphStochastic.mark;
		logical = GraphLogical.mark;

		(*	Checks that markov blanket of stoch can be evaluate on this core and does not depend
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
		p.SetProps(props - {write, thisCore, logical});
		p.Externalize(wr);
		p.SetProps(props)
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
				p.SetProps(p.props + marks);
				INC(i)
			END
		|node: GraphVector.Node DO
			i := 0;
			size := node.Size();
			WHILE i < size DO
				l := node.components[i];
				l.SetProps(l.props + marks);
				INC(i)
			END
		|node: GraphMultivariate.Node DO
			node.SetProps(node.props + marks);
			i := 0;
			size := node.Size();
			WHILE i < size DO
				p := node.components[i];
				IF GraphNodes.data IN p.props THEN 
					p.SetProps(p.props + marks) 
				ELSIF p IS GraphMRF.Node THEN
					p.Install(install);
					IF install = "SpatialCARProper.Install" THEN
						p.SetProps(p.props + marks) 
					END
				END;
				INC(i)
			END
		ELSE
			node.SetProps(node.props + marks)
		END
	END MarkNode;
	
	PROCEDURE (v: Clearer) Do (name: BugsNames.Name);
		VAR
			p: GraphNodes.Node;
	BEGIN
		IF name.passByreference THEN
			p := name.components[v.index];
			IF p # NIL THEN p.SetProps(p.props - {write, thisCore, logical}) END
		END
	END Do;

	PROCEDURE (v: MarkLogical) Do (name: BugsNames.Name);
		VAR
			p: GraphNodes.Node;
	BEGIN
		IF name.passByreference THEN
			p := name.components[v.index];
			IF (p # NIL) & (p IS GraphLogical.Node) THEN
				p.SetProps(p.props + {logical})
			END
		END
	END Do;

	PROCEDURE (v: Writer) Do (name: BugsNames.Name);
		VAR
			p: GraphNodes.Node;
			address: INTEGER;
	BEGIN
		IF name.passByreference THEN
			p := name.components[v.index];
			(*	only write out nodes with the write mark	*)
			IF (p # NIL) & (write IN p.props) THEN
				(*	write out logical nodes and data nodes but not parameter nodes	*)
				IF ~(p IS GraphStochastic.Node) OR (GraphNodes.data IN p.props) THEN
					IF v.internals THEN
						Externalize(p, v.wr)
					ELSIF v.debug THEN
						address := SYSTEM.VAL(INTEGER, p);
						v.wr.WriteInt(address)
					ELSE
						GraphNodes.ExternalizePointer(p, v.wr);
					END
				END
			END
		END
	END Do;

	PROCEDURE (v: Counter) Do (name: BugsNames.Name);
		VAR
			p: GraphNodes.Node;
	BEGIN
		IF name.passByreference THEN
			p := name.components[v.index];
			IF (p # NIL) & (write IN p.props) THEN
				IF ~(p IS GraphStochastic.Node) OR (GraphNodes.data IN p.props) THEN
					INC(v.num)
				END
			END
		END
	END Do;

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

	PROCEDURE CountNamedPointers (): INTEGER;
		VAR
			v: Counter;
	BEGIN
		NEW(v);
		v.num := 0;
		BugsIndex.Accept(v);
		RETURN v.num
	END CountNamedPointers;

	PROCEDURE ExternalizeNamedPointers (VAR wr: Stores.Writer);
		VAR
			v: Writer;
	BEGIN
		NEW(v);
		v.wr := wr;
		v.debug := FALSE;
		v.internals := FALSE;
		BugsIndex.Accept(v)
	END ExternalizeNamedPointers;

	PROCEDURE ExternalizeNamedAddresses (VAR wr: Stores.Writer);
		VAR
			v: Writer;
	BEGIN
		NEW(v);
		v.wr := wr;
		v.debug := TRUE;
		v.internals := FALSE;
		BugsIndex.Accept(v)
	END ExternalizeNamedAddresses;

	PROCEDURE ExternalizeNameInternals (VAR wr: Stores.Writer);
		VAR
			v: Writer;
	BEGIN
		NEW(v);
		v.wr := wr;
		v.debug := FALSE;
		v.internals := TRUE;
		BugsIndex.Accept(v)
	END ExternalizeNameInternals;

	PROCEDURE CountStochasticPointers (IN updaters: ARRAY OF ARRAY OF INTEGER): INTEGER;
		VAR
			i, len, num, size, label: INTEGER;
			u: UpdaterUpdaters.Updater;
	BEGIN
		num := LEN(updaters, 1);
		i := 0;
		len := 0;
		WHILE i < num DO
			label := updaters[0, i];
			IF label # undefined THEN
				u := UpdaterActions.updaters[0, label];
				size := u.Size()
			ELSE
				size := 1
			END;
			INC(len, size);
			INC(i)
		END;
		RETURN len
	END CountStochasticPointers;

	(*	Writes out pointers of stochastic nodes in model, some pointers written multiple times	*)
	PROCEDURE ExternalizeStochasticPointers (IN updaters: ARRAY OF ARRAY OF INTEGER;
	VAR wr: Stores.Writer);
		VAR
			i, j, k, label, num, workersPerChain, size: INTEGER;
			p: GraphStochastic.Node;
			u: UpdaterUpdaters.Updater;
	BEGIN
		workersPerChain := LEN(updaters, 0);
		num := LEN(updaters, 1);
		i := 0;
		WHILE i < workersPerChain DO
			j := 0;
			WHILE j < num DO
				label := updaters[i, j];
				IF label # undefined THEN
					(*	have updater on this core	*)
					u := UpdaterActions.updaters[0, label];
					size := u.Size()
				ELSE
					(*	no updater on this core so find updater on zero core to get info about size	*)
					label := updaters[0, j];
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
					IF u # NIL THEN
						p := u.Node(k)
					ELSE
						(*	no updater so create a dummy node	*)
						p := GraphDummy.fact.New()
					END;
					(*	problem with dStable	*)
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
					INC(k)
				END;
				INC(j)
			END;
			INC(i)
		END
	END ExternalizeStochasticPointers;

	(*
	Writes out internal fields of stochastic nodes in model. For fixed effects data only written once.
	Nodes' likelihoods thinned for fixed effects
	*)
	PROCEDURE ExternalizeStochasticInternals (IN updaters: ARRAY OF ARRAY OF INTEGER;
	rank: INTEGER; OUT this: BOOLEAN; VAR wr: Stores.Writer);
		VAR
			i, j, k, workersPerChain, num, size, label: INTEGER;
			p, dummy: GraphStochastic.Node;
			dummyMV: GraphMultivariate.Node;
			u: UpdaterUpdaters.Updater;
			thinned, temp: GraphStochastic.Vector;
	BEGIN
		(*	create dummy nodes so that their internal fields can be written out if needed	*)
		dummy := GraphDummy.fact.New();
		dummyMV := GraphDummyMV.fact.New();
		workersPerChain := LEN(updaters, 0);
		num := LEN(updaters, 1);
		this := TRUE;
		i := 0;
		WHILE i < workersPerChain DO
			j := 0;
			WHILE j < num DO
				label := updaters[i, j];
				IF label # undefined THEN
					u := UpdaterActions.updaters[0, label];
					size := u.Size();
					p := u.Node(0);
					(*	updater 's likelihood is distributed between cores	*)
					IF GraphStochastic.distributed IN p.props THEN
						(*	only write out fist time	*)
						IF i = 0 THEN
							k := 0;
							WHILE k < size DO
								p := u.Node(k);
								temp := p.children;
								thinned := ThinChildren(temp, rank); 
								p.SetChildren(thinned);
								this := this & ThisCore(p);
								Externalize(p, wr);
								p.SetChildren(temp);
								INC(k)
							END
						END
					ELSE
						k := 0;
						WHILE k < size DO
							p := u.Node(k);
							(*	problem with dStable	*)
							IF (write IN p.props) OR (GraphStochastic.noPDF IN p.props) THEN
								IF i = rank THEN
									(*	updated on this core so need likelihood and prior	*)
									this := this & ThisCore(p);
									Externalize(p, wr)
								ELSE
									(*	not updated on this core so do not need its likelihood but need its prior
									except for multivariate case of mixed data/stochastic	*)
									temp := p.children;
									IF (temp # NIL) & ~(write IN temp[0].props) THEN
										p.SetChildren(NIL)
									END;
									this := this & ThisCore(p);
									Externalize(p, wr);
									p.SetChildren(temp)
								END
							ELSE
								(*	not updated on this core and not used as likelihood, therefore dummy node	*)
								WITH p: GraphMultivariate.Node DO
									dummyMV.SetComponent(p.components, p.index);
									Externalize(dummyMV, wr);
									dummyMV.SetComponent(NIL, - 1)
								ELSE
									Externalize(dummy, wr)
								END
							END;
							INC(k)
						END
					END
				ELSE
					label := updaters[0, j];
					IF label # undefined THEN
						u := UpdaterActions.updaters[0, label];
						size := u.Size();
					ELSE
						size := 1
					END;
					k := 0;
					WHILE k < size DO
						Externalize(dummy, wr);
						INC(k)
					END
				END;
				INC(j)
			END;
			INC(i)
		END
	END ExternalizeStochasticInternals;

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
	END ExternalizeUpdatersData;

	PROCEDURE ClearNodes;
		VAR
			v: Clearer;
			i, j, workersPerChain, num: INTEGER;
			p: GraphStochastic.Node;
			dependents: GraphLogical.List;
			logical: GraphLogical.Node;
	BEGIN
		NEW(v);
		BugsIndex.Accept(v);
		IF globalStochs # NIL THEN
			workersPerChain := LEN(globalStochs);
			num := LEN(globalStochs[0]);
			i := 0;
			WHILE i < workersPerChain DO
				j := 0;
				WHILE j < num DO
					p := globalStochs[i, j];
					p.SetProps(p.props - {write, thisCore});
					(*	deal with any hidden logical dependent nodes	*)
					dependents := p.dependents;
					WHILE dependents # NIL DO
						logical := dependents.node;
						logical.SetProps(logical.props - {write, thisCore});
						dependents := dependents.next
					END;
					INC(j)
				END;
				INC(i)
			END
		END
	END ClearNodes;

	(*	Mark logical nodes that have a name with logical mark	*)
	PROCEDURE MarkLogicals;
		VAR
			v: MarkLogical;
	BEGIN
		NEW(v);
		BugsIndex.Accept(v);
	END MarkLogicals;

	(*	Mark markov blanket of nodes updated on this core. Two marks are used the write mark meaning
	that the internal fields of the marked node are needed and thisCore meaning that the node will reside on
	the core whoose rank is rank	*)
	PROCEDURE MarkMarkovBlanket (IN updaters: ARRAY OF ARRAY OF INTEGER; rank: INTEGER);
		VAR
			i, j, k, num, numCores, size, label, len, sizel: INTEGER;
			p, prior: GraphStochastic.Node;
			u: UpdaterUpdaters.Updater;
			list: GraphLogical.List;
			log, log1: GraphLogical.Node;
			children: GraphStochastic.Vector;
		CONST
			all = TRUE;
	BEGIN
		num := LEN(updaters, 1);
		(*	mark the priors	*)
		i := 0;
		WHILE i < num DO
			label := updaters[rank, i];
			IF label # undefined THEN
				u := UpdaterActions.updaters[0, label];
				size := u.Size();
				k := 0;
				WHILE k < size DO
					p := u.Node(k);
					MarkNode(p, {write, thisCore});
					list := GraphLogical.Parents(p, all);
					WHILE list # NIL DO
						log := list.node;
						IF {logical, GraphLogical.stochParent} * log.props # {} THEN 
							MarkNode(log, {write})
						END;
						list := list.next
					END;
					INC(k)
				END
			END;
			INC(i)
		END;
		(*	mark likelihoods of random effects	*)
		i := 0;
		WHILE i < num DO
			label := updaters[rank, i];
			IF label # undefined THEN
				u := UpdaterActions.updaters[0, label];
				prior := u.Node(0);
				IF ~(GraphStochastic.distributed IN prior.props) THEN
					children := u.Children();
					IF children # NIL THEN len := LEN(children) ELSE len := 0 END;
					j := 0;
					WHILE j < len DO
						p := children[j];
						MarkNode(p, {write});
						list := GraphLogical.Parents(p, all);
						WHILE list # NIL DO
							log := list.node;
							IF {logical, GraphLogical.stochParent} * log.props # {} THEN 
								MarkNode(log, {write})
							END;
							list := list.next
						END;
						INC(j)
					END
				END;
			END;
			INC(i)
		END;
		(*	add write mark to stochastic nodes that have logical memory / vector node dependents	*)
		i := 0;
		numCores := LEN(updaters, 0);
		WHILE i < num DO
			j := 0;
			WHILE j < numCores DO
				label := updaters[j, i];
				IF label # undefined THEN
					u := UpdaterActions.updaters[0, label];
					size := u.Size();
					k := 0;
					WHILE k < size DO
						p := u.Node(k);
						list := p.dependents;
						WHILE (list # NIL) & ~(write IN list.node.props) DO list := list.next END;
						IF list # NIL THEN MarkNode(p, {write}) END; 
						INC(k)
					END;
				END;
				INC(j)
			END;
			INC(i)
		END		
	END MarkMarkovBlanket;

	(*	add observations to nodes to be writen plus their named logical parents and memory nodes	*)
	PROCEDURE MarkObservations (observations: GraphStochastic.Vector);
		VAR
			i, len: INTEGER;
			p: GraphStochastic.Node;
			log: GraphLogical.Node;
			list: GraphLogical.List;
		CONST
			all = TRUE;
	BEGIN
		IF observations # NIL THEN len := LEN(observations) ELSE len := 0 END;
		i := 0;
		WHILE i < len DO
			p := observations[i];
			MarkNode(p, {write, thisCore});
			list := GraphLogical.Parents(p, all);
			WHILE list # NIL DO
				log := list.node;
				IF {logical, GraphLogical.stochParent} * log.props # {} THEN 
					MarkNode(log, {write})
				END;
				list := list.next
			END;
			INC(i)
		END 
	END MarkObservations;

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
		NEW(globalStochs, workersPerChain, len);
		i := 0;
		WHILE i < workersPerChain DO
			j := 0;
			len := 0;
			WHILE j < num DO
				label := globalUpdaters[i, j];
				IF label # undefined THEN
					u := UpdaterActions.updaters[0, label];
					size := u.Size()
				ELSE (*	no updater	add in dummy variables	*)
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
		globalId := NIL
	END Clear;

	PROCEDURE Distribute* (workersPerChain: INTEGER);
		VAR
			i, numUpdaters: INTEGER;
	BEGIN
		IF workersPerChain = 1 THEN
			numUpdaters := UpdaterActions.NumberUpdaters();
			IF numUpdaters > 0 THEN NEW(globalUpdaters, 1, numUpdaters) ELSE globalUpdaters := NIL END;
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

	PROCEDURE MonitoredNodes* (OUT col, row: POINTER TO ARRAY OF INTEGER);
		VAR
			i, j, len, num, workersPerChain: INTEGER;
			p: GraphStochastic.Node;
			q: GraphNodes.Node;
	BEGIN
		workersPerChain := LEN(globalStochs, 0);
		len := LEN(globalStochs, 1);
		i := 0;
		WHILE i < workersPerChain DO
			j := 0;
			WHILE j < len DO (*	any node that has its represenative marked is also marked	*)
				p := globalStochs[i, j];
				q := p.Representative();
				IF GraphStochastic.mark IN q.props THEN
					p.SetProps(p.props + {GraphStochastic.mark})
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
		END
	END MonitoredNodes;

	PROCEDURE NumStochastics* (): INTEGER;
		VAR
			numStochastics: INTEGER;
	BEGIN
		numStochastics := LEN(globalStochs, 0) * LEN(globalStochs, 1);
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
			p.SetValue(values[i]);
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
				globalStochs[i, j].SetValue(values[i * numRow + j]);
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
			chain, workersPerChain, numStochasticPointers, numNamePointers, pos, uEndPos: INTEGER;
			uPos: POINTER TO ARRAY OF INTEGER;
			dummy: GraphStochastic.Node;
			dummyMV: GraphMultivariate.Node;
			filter: SET;
	BEGIN
		ClearNodes;
		filter := GraphStochastic.dependentsFilter;
		GraphStochastic.FilterDependents({(*GraphLogical.stochParent*)write});
		MarkLogicals;
		dummy := GraphDummy.fact.New();
		dummy.Init;
		dummyMV := GraphDummyMV.fact.New();
		dummyMV.Init;
		dummyMV.SetComponent(NIL, - 1);
		MarkMarkovBlanket(globalUpdaters, rank); 
		MarkObservations(globalDeviance[rank]);
		numStochasticPointers := CountStochasticPointers(globalUpdaters);
		numNamePointers := CountNamedPointers();
		workersPerChain := LEN(globalUpdaters, 0);
		wr.WriteBool(debug);
		wr.WriteInt(workersPerChain);
		wr.WriteInt(numChains);
		wr.WriteInt(numStochasticPointers);
		wr.WriteInt(numNamePointers);
		ExternalizeId(globalId, wr);
		(*	Write out a block of pointers to nodes in the model needed on this core. This block might
		contain duplicate nodes but when the worker reads this block it will remove any duplicates	*)
		GraphNodes.BeginExternalize(wr);
		(*	write out a node of type GraphDummy so that this becomes type zero in type index	*)
		GraphNodes.ExternalizePointer(dummy, wr);
		(*	write out a node of type GraphDummyMV so that this becomes type one in type index	*)
		GraphNodes.ExternalizePointer(dummyMV, wr);
		ExternalizeStochasticPointers(globalUpdaters, wr);
		ExternalizeNamedPointers(wr);
		IF debug THEN ExternalizeNamedAddresses(wr) END;
		ExternalizeObservations(globalDeviance[rank], wr);
		(*	Write out internal fields of block of pointers. For duplicate pointers only write internal fields
		for the first time 	*)
		(*	write out data for dummy node so that it can be read back in	*)
		dummy.Externalize(wr);
		(*	write out data for dummy MV node so that it can be read back in	*)
		dummyMV.Externalize(wr);
		ExternalizeStochasticInternals(globalUpdaters, rank, this, wr);
		ExternalizeNameInternals(wr);
		ClearNodes;
		UpdaterUpdaters.BeginExternalize(wr);
		ExternalizeUpdaters(globalUpdaters, rank, wr);
		NEW(uPos, numChains);
		(*	write out token positional info	*)
		pos := wr.Pos();
		chain := 0;
		WHILE chain < numChains DO
			uPos[chain] := - 1;
			wr.WriteInt(uPos[chain]);
			INC(chain)
		END;
		uEndPos := - 1;
		wr.WriteInt(uEndPos);
		chain := 0;
		WHILE chain < numChains DO
			uPos[chain] := wr.Pos();
			ExternalizeUpdatersData(globalUpdaters, rank, chain, wr);
			INC(chain)
		END;
		uEndPos := wr.Pos();
		(*	write out actual positional info	*)
		wr.SetPos(pos);
		chain := 0;
		WHILE chain < numChains DO
			wr.WriteInt(uPos[chain]);
			INC(chain)
		END;
		wr.WriteInt(uEndPos);
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

