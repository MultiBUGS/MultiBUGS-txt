(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsGraph;

	

	IMPORT
		SYSTEM, 
		Strings,
		BugsIndex, BugsMsg, BugsNames, BugsNodes, BugsParser, BugsRandnum,
		GraphConjugateMV, GraphDeviance, GraphLogical, GraphNodes, GraphRules,
		GraphScalar, GraphStochastic, GraphUnivariate, GraphVector,
		UpdaterActions, UpdaterMethods, UpdaterUpdaters;

	TYPE
		BuildConditional = POINTER TO RECORD(BugsNames.ElementVisitor)
			repeat: BOOLEAN
		END;

		ClearMarks = POINTER TO RECORD(BugsNames.ElementVisitor)
			marks: SET
		END;

		ReadCache = POINTER TO RECORD(BugsNames.ElementVisitor) END;

		ClassifyConditional = POINTER TO RECORD(BugsNames.ElementVisitor) END;

		CalculateDepth = POINTER TO RECORD(BugsNames.ElementVisitor)
			new: BOOLEAN;
			maxDepth, maxStochDepth: INTEGER;
		END;

		ConditionalOfClass = POINTER TO RECORD(BugsNames.ElementVisitor)
			class: SET;
			list: GraphStochastic.List
		END;

		CountStochastic = POINTER TO RECORD(BugsNames.ElementVisitor)
			num: INTEGER
		END;

		BuildDependent = POINTER TO RECORD(BugsNames.ElementVisitor) END;

		CalculateLevel = POINTER TO RECORD(BugsNames.ElementVisitor)
			new: BOOLEAN
		END;

		CalculateLevel1 = POINTER TO RECORD(BugsNames.ElementVisitor) END;

		CreateUpdaterByMethod = POINTER TO RECORD(BugsNames.ElementVisitor)
			factory: UpdaterUpdaters.Factory
		END;

		CreateUpdaterByNode = POINTER TO RECORD(BugsNames.ElementVisitor) END;

		MissingUpdater = POINTER TO RECORD(BugsNames.ElementVisitor)
			ok: BOOLEAN
		END;

		Hints = POINTER TO RECORD(BugsNames.ElementVisitor) END;

	VAR
		devianceExists-: BOOLEAN;
		initialized: POINTER TO ARRAY OF POINTER TO ARRAY OF BOOLEAN;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE HeapSort (VAR nodes: GraphNodes.Vector);
		VAR
			i, j, k, len: INTEGER;

		PROCEDURE Less (l, m: INTEGER): BOOLEAN;
		BEGIN
			RETURN SYSTEM.VAL(INTEGER, nodes[l - 1]) < SYSTEM.VAL(INTEGER, nodes[m - 1])
		END Less;

		PROCEDURE Swap (l, m: INTEGER);
			VAR
				temp: GraphNodes.Node;
		BEGIN
			temp := nodes[l - 1];
			nodes[l - 1] := nodes[m - 1];
			nodes[m - 1] := temp
		END Swap;

	BEGIN
		len := LEN(nodes);
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

	PROCEDURE InitDiffs;
		VAR
			logicals: GraphLogical.Vector;
			i, j, nodeSize, num, size: INTEGER;
			p: GraphLogical.Node;
			list, cursor: GraphStochastic.List;
			diffWRT: GraphNodes.Vector;
			stoch: GraphStochastic.Node;
			isDifferentiable: BOOLEAN;

		CONST
			all = TRUE;
			hidden = GraphStochastic.hidden;

		PROCEDURE IsDifferentiable (node: GraphStochastic.Node): BOOLEAN;
		BEGIN
			RETURN ~(node.classConditional IN {GraphRules.general, GraphRules.catagorical,
			GraphRules.poisson, GraphRules.descrete}) & ~(hidden IN node.props)
		END IsDifferentiable;

	BEGIN
		logicals := GraphLogical.nodes; 
		IF logicals # NIL THEN
			num := LEN(logicals);
			i := 0;
			WHILE i < num DO
				p := logicals[i];
				list := GraphStochastic.Parents(p, all);
				cursor := list; size := 0;
				WHILE cursor # NIL DO
					stoch := cursor.node;
					isDifferentiable := IsDifferentiable(stoch);
					IF isDifferentiable THEN
						WITH stoch: GraphConjugateMV.Node DO
							nodeSize := stoch.Size(); INC(size, nodeSize)
						ELSE
							INC(size)
						END
					END;
					cursor := cursor.next
				END;
				IF size > 0 THEN NEW(diffWRT, size) ELSE diffWRT := NIL END;
				cursor := list; size := 0;
				WHILE cursor # NIL DO
					stoch := cursor.node;
					isDifferentiable := IsDifferentiable(stoch);
					IF isDifferentiable THEN
						WITH stoch: GraphConjugateMV.Node DO
							nodeSize := stoch.Size();
							j := 0;
							WHILE j < nodeSize DO
								diffWRT[size] := stoch.components[j]; INC(size); INC(j)
							END
						ELSE
							diffWRT[size] := stoch; INC(size)
						END
					END;
					cursor := cursor.next
				END;
				IF size > 0 THEN HeapSort(diffWRT) END;
				p.SetDiffWRT(diffWRT);
				INC(i)
			END
		END
	END InitDiffs;

	PROCEDURE IsInitialized* (chain: INTEGER): BOOLEAN;
		VAR
			i, numStoch: INTEGER;
			isInit: BOOLEAN;
	BEGIN
		isInit := TRUE;
		IF (initialized # NIL) & (initialized[chain] # NIL) THEN
			numStoch := LEN(initialized[chain]);
			i := 0; WHILE isInit & (i < numStoch) DO isInit := initialized[chain, i]; INC(i) END
		END;
		RETURN isInit
	END IsInitialized;

	PROCEDURE LoadInits* (chain: INTEGER);
		VAR
			i, numStoch: INTEGER;
			stochastics: GraphStochastic.Vector;
	BEGIN
		stochastics := GraphStochastic.nodes;
		IF stochastics # NIL THEN
			numStoch := LEN(stochastics);
			i := 0;
			WHILE i < numStoch DO
				IF initialized[chain, i] THEN
					INCL(stochastics[i].props, GraphStochastic.initialized)
				ELSE
					EXCL(stochastics[i].props, GraphStochastic.initialized)
				END;
				INC(i)
			END
		END
	END LoadInits;

	(*	mark linear nodes	*)
	PROCEDURE OptimizeDifferentiation*;
		VAR
			class, i, j, len, size: INTEGER;
			p: GraphLogical.Node;
			diffWRT: GraphNodes.Vector;
			nodes: GraphLogical.Vector;
	BEGIN
		nodes := GraphLogical.nodes;
		IF nodes # NIL THEN
			size := LEN(nodes);
			i := 0;
			WHILE i < size DO
				p := nodes[i];
				diffWRT := p.diffWRT;
				IF diffWRT # NIL THEN
					len := LEN(diffWRT);
					p.SetDiffWRT(diffWRT);
					j := 0; WHILE j < len DO INCL(diffWRT[j].props, GraphStochastic.mark); INC(j) END;
					class := p.ClassFunction(diffWRT[0]);
					j := 0; WHILE j < len DO EXCL(diffWRT[j].props, GraphStochastic.mark); INC(j) END;
					IF class IN {GraphRules.ident, GraphRules.prod, GraphRules.linear} THEN
						INCL(p.props, GraphLogical.linear)
					END;
				END;
				INC(i)
			END
		END
	END OptimizeDifferentiation;

	PROCEDURE SetInitialized*;
		VAR
			i, chain, numStoch, numChains: INTEGER;
			isInit: BOOLEAN;
	BEGIN
		isInit := TRUE;
		IF (initialized # NIL) & (initialized[0] # NIL) THEN
			numStoch := LEN(initialized[0]); numChains := LEN(initialized);
			chain := 0;
			WHILE isInit & (chain < numChains) DO
				i := 0; WHILE isInit & (i < numStoch) DO isInit := initialized[chain, i]; INC(i) END;
				INC(chain)
			END;
		END;
		IF isInit THEN UpdaterActions.SetInitialized; UpdaterActions.OptimizeUpdaters END
	END SetInitialized;

	PROCEDURE StoreInits* (chain: INTEGER);
		VAR
			i, numStoch: INTEGER;
			stochastics: GraphStochastic.Vector;
	BEGIN
		stochastics := GraphStochastic.nodes;
		IF stochastics # NIL THEN
			numStoch := LEN(stochastics);
			i := 0;
			WHILE i < numStoch DO
				initialized[chain, i] := GraphStochastic.initialized IN stochastics[i].props;
				INC(i)
			END
		END
	END StoreInits;

	PROCEDURE IsAdapting* (numChains: INTEGER): BOOLEAN;
		VAR
			isAdaptingChain: BOOLEAN;
			chain: INTEGER;
	BEGIN
		chain := 0;
		isAdaptingChain := FALSE;
		WHILE (chain < numChains) & ~isAdaptingChain DO
			isAdaptingChain := UpdaterActions.IsAdapting(chain);
			INC(chain)
		END;
		RETURN isAdaptingChain
	END IsAdapting;

	PROCEDURE Error (errorNum: INTEGER; name: ARRAY OF CHAR);
		VAR
			errorMsg: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
			numToString: ARRAY 8 OF CHAR;
	BEGIN
		Strings.IntToString(errorNum, numToString);
		p[0] := name$;
		BugsMsg.LookupParam("BugsGraph" + numToString, p, errorMsg);
		BugsMsg.StoreError(errorMsg)
	END Error;

	PROCEDURE (v: BuildConditional) Do (name: BugsNames.Name);
		VAR
			node: GraphNodes.Node;
	BEGIN
		IF name.passByreference THEN
			node := name.components[v.index];
			IF node # NIL THEN
				WITH node: GraphStochastic.Node DO
					IF ~(GraphStochastic.likelihood IN node.props) THEN
						node.BuildLikelihood;
						IF GraphStochastic.likelihood IN node.props THEN
							v.repeat := TRUE
						END
					END
				ELSE
				END
			END
		END
	END Do;

	PROCEDURE (v: ClearMarks) Do (name: BugsNames.Name);
		VAR
			node: GraphNodes.Node;
	BEGIN
		IF name.passByreference THEN
			node := name.components[v.index];
			IF node # NIL THEN
				IF node IS GraphStochastic.Node THEN
					node.props := node.props - v.marks
				END
			END
		END
	END Do;

	PROCEDURE BuildFullConditionals;
		VAR
			v: BuildConditional;
			v1: ClearMarks;
			cursor: GraphStochastic.List;
			node: GraphStochastic.Node;
	BEGIN
		NEW(v);
		NEW(v1);
		REPEAT
			v.repeat := FALSE;
			BugsIndex.Accept(v);
			cursor := GraphStochastic.auxillary;
			WHILE cursor # NIL DO
				node := cursor.node;
				IF ~(GraphStochastic.likelihood IN node.props) THEN
					node.BuildLikelihood;
					IF GraphStochastic.likelihood IN node.props THEN
						v.repeat := TRUE
					END
				END;
				cursor := cursor.next
			END;
		UNTIL v.repeat = FALSE;
		v1.marks := {GraphStochastic.likelihood};
		BugsIndex.Accept(v1);
		cursor := GraphStochastic.auxillary;
		WHILE cursor # NIL DO
			node := cursor.node;
			EXCL(node.props, GraphStochastic.likelihood);
			cursor := cursor.next
		END
	END BuildFullConditionals;

	PROCEDURE (v: ReadCache) Do (name: BugsNames.Name);
		VAR
			node: GraphNodes.Node;
	BEGIN
		IF name.passByreference THEN
			node := name.components[v.index];
			IF node # NIL THEN
				WITH node: GraphStochastic.Node DO
					IF ~(GraphStochastic.data IN node.props) THEN node.SetLikelihood END
				ELSE
				END
			END
		END
	END Do;

	PROCEDURE ReadCaches;
		VAR
			v: ReadCache;
			cursor: GraphStochastic.List;
			node: GraphStochastic.Node;
	BEGIN
		NEW(v);
		BugsIndex.Accept(v); 
		cursor := GraphStochastic.auxillary;
		WHILE cursor # NIL DO
			node := cursor.node;
			node.SetLikelihood;
			cursor := cursor.next
		END;
		GraphStochastic.ClearCache
	END ReadCaches;

	PROCEDURE (v: ClassifyConditional) Do (name: BugsNames.Name);
		VAR
			node: GraphNodes.Node;
	BEGIN
		IF name.passByreference THEN
			node := name.components[v.index];
			IF node # NIL THEN
				WITH node: GraphStochastic.Node DO
					IF {GraphNodes.data, GraphStochastic.hidden} * node.props = {} THEN
						node.ClassifyConditional;
					END
				ELSE
				END
			END
		END
	END Do;

	PROCEDURE ClassifyConditionals;
		VAR
			v: ClassifyConditional;
	BEGIN
		NEW(v);
		BugsIndex.Accept(v)
	END ClassifyConditionals;

	PROCEDURE (v: ConditionalOfClass) Do (name: BugsNames.Name);
		VAR
			node: GraphNodes.Node;
			stoch: GraphStochastic.Node;
			mv: GraphConjugateMV.Node;
			i, size: INTEGER;
			comp: GraphStochastic.Vector;
		CONST
			noUpdater = {GraphNodes.data, GraphStochastic.hidden};
	BEGIN
		IF name.passByreference THEN
			node := name.components[v.index];
			IF node # NIL THEN
				IF node IS GraphStochastic.Node THEN
					stoch := node(GraphStochastic.Node);
					IF noUpdater * stoch.props = {} THEN
						IF stoch.classConditional IN v.class THEN
							IF stoch IS GraphConjugateMV.Node THEN
								mv := stoch(GraphConjugateMV.Node);
								size := mv.Size();
								i := 0;
								comp := mv.components;
								WHILE (i < size) & (noUpdater * comp[i].props = {}) DO INC(i) END;
								IF i = size THEN
									i := 0;
									WHILE i < size DO
										GraphStochastic.AddToList(comp[i], v.list);
										INCL(comp[i].props, GraphNodes.mark);
										INC(i)
									END
								END
							ELSE
								GraphStochastic.AddToList(stoch, v.list);
								INCL(stoch.props, GraphNodes.mark)
							END;
						END
					END
				ELSE
				END
			END
		END
	END Do;

	PROCEDURE ConditionalsOfClass* (class: SET): GraphStochastic.Vector;
		VAR
			v: ConditionalOfClass;
			vector: GraphStochastic.Vector;
			list: GraphStochastic.List;
	BEGIN
		NEW(v);
		v.class := class;
		v.list := NIL;
		BugsIndex.Accept(v);
		list := v.list;
		vector := GraphStochastic.ListToVector(list);
		GraphStochastic.ClearMarks(vector, {GraphNodes.mark});
		RETURN vector
	END ConditionalsOfClass;

	PROCEDURE (v: CreateUpdaterByMethod) Do (name: BugsNames.Name);
		CONST
			noUpdater = {GraphNodes.data, GraphStochastic.update, GraphStochastic.hidden};
		VAR
			updater: UpdaterUpdaters.Updater;
			factory: UpdaterUpdaters.Factory;
			node: GraphNodes.Node;
			stochastic: GraphStochastic.Node;
	BEGIN
		IF name.passByreference THEN
			node := name.components[v.index];
			IF node # NIL THEN
				IF node IS GraphStochastic.Node THEN
					stochastic := node(GraphStochastic.Node);
					IF noUpdater * stochastic.props = {} THEN
						factory := v.factory;
						IF factory.CanUpdate(stochastic) THEN
							updater := factory.New(stochastic);
							UpdaterActions.RegisterUpdater(updater);
							factory.SetProps(factory.props + {UpdaterUpdaters.active})
						END
					END
				END
			END
		END
	END Do;

	PROCEDURE CreateUpdatersByMethod;
		VAR
			i, numFactories: INTEGER;
			v: CreateUpdaterByMethod;
			factory: UpdaterUpdaters.Factory;
	BEGIN
		NEW(v);
		i := 0;
		numFactories := LEN(UpdaterMethods.factories);
		WHILE i < numFactories DO
			factory := UpdaterMethods.factories[i];
			IF UpdaterUpdaters.enabled IN factory.props THEN
				v.factory := factory;
				BugsIndex.Accept(v)
			END;
			INC(i)
		END
	END CreateUpdatersByMethod;

	PROCEDURE (v: CreateUpdaterByNode) Do (name: BugsNames.Name);
		CONST
			noUpdater = {GraphNodes.data, GraphStochastic.update, GraphStochastic.hidden};
		VAR
			updater: UpdaterUpdaters.Updater;
			factory: UpdaterUpdaters.Factory;
			node: GraphNodes.Node;
			stochastic: GraphStochastic.Node;
			i, numMethods: INTEGER;
	BEGIN
		IF name.passByreference THEN
			node := name.components[v.index];
			updater := NIL;
			IF node # NIL THEN
				IF node IS GraphStochastic.Node THEN
					stochastic := node(GraphStochastic.Node);
					IF noUpdater * stochastic.props = {} THEN
						i := 0;
						numMethods := LEN(UpdaterMethods.factories);
						WHILE (i < numMethods) & (updater = NIL) DO
							factory := UpdaterMethods.factories[i];
							IF (UpdaterUpdaters.enabled IN factory.props) & factory.CanUpdate(stochastic) THEN
								updater := factory.New(stochastic);
								UpdaterActions.RegisterUpdater(updater);
								factory.SetProps(factory.props + {UpdaterUpdaters.active})
							END;
							INC(i)
						END
					END
				END
			END
		END
	END Do;

	PROCEDURE CreateUpdatersByNode;
		VAR
			v: CreateUpdaterByNode;
	BEGIN
		NEW(v);
		BugsIndex.Accept(v)
	END CreateUpdatersByNode;

	PROCEDURE (v: MissingUpdater) Do (name: BugsNames.Name);
		CONST
			noUpdater = {GraphNodes.data, GraphStochastic.update, GraphStochastic.hidden};
		VAR
			node: GraphNodes.Node;
			stochastic: GraphStochastic.Node;
			string: ARRAY 1024 OF CHAR;
	BEGIN
		IF name.passByreference THEN
			node := name.components[v.index];
			IF (node # NIL) & v.ok THEN
				IF node IS GraphStochastic.Node THEN
					stochastic := node(GraphStochastic.Node);
					IF noUpdater * stochastic.props = {} THEN
						v.ok := FALSE;
						name.Indices(v.index, string);
						string := name.string + string;
						Error(1, string)
					END
				END
			END
		END
	END Do;

	PROCEDURE MissingUpdaters (OUT ok: BOOLEAN);
		VAR
			v: MissingUpdater;
	BEGIN
		NEW(v);
		v.ok := TRUE;
		BugsIndex.Accept(v);
		ok := v.ok
	END MissingUpdaters;

	(*	For distributions with two scalar parameters try and determine which parameter is
	active (being sampled from). Problem if distribution has three parameters and one is constant. So do not
	use this hint information for distributions with more than two parameters	*)

	PROCEDURE (v: Hints) Do (name: BugsNames.Name);
		VAR
			node, p: GraphNodes.Node;
			child, stochastic: GraphStochastic.Node;
			children: GraphStochastic.Vector;
			parents: GraphNodes.List;
			class, hint, numParam, i, num: INTEGER;
			hints: SET;
		CONST
			all = TRUE;
	BEGIN
		IF name.passByreference THEN
			node := name.components[v.index];
			IF node # NIL THEN
				IF node IS GraphUnivariate.Node THEN
					stochastic := node(GraphUnivariate.Node);
					IF GraphStochastic.update IN stochastic.props THEN
						hints := {};
						children := stochastic.children;
						IF children # NIL THEN
							num := LEN(children);
							i := 0;
							WHILE i < num DO
								child := children[i];
								parents := child.Parents(all);
								numParam := 0;
								hint := 0;
								WHILE parents # NIL DO
									p := parents.node;
									INC(numParam);
									class := GraphStochastic.ClassFunction(p, node);
									IF class # GraphRules.const THEN
										hint := numParam;
										IF hint = 1 THEN
											INCL(hints, GraphStochastic.hint1)
										ELSIF hint = 2 THEN
											INCL(hints, GraphStochastic.hint2)
										END
									END;
									parents := parents.next
								END;
								IF numParam = 2 THEN
									stochastic.props := stochastic.props + hints
								END;
								INC(i)
							END
						END;
						IF (GraphStochastic.hint1 IN stochastic.props)
							 & (GraphStochastic.hint2 IN stochastic.props) THEN
							stochastic.props := stochastic.props - {GraphStochastic.hint1, GraphStochastic.hint2}
						END
					END
				END
			END
		END
	END Do;

	PROCEDURE CalculateHints;
		VAR
			v: Hints;
	BEGIN
		NEW(v);
		BugsIndex.Accept(v);
	END CalculateHints;

	PROCEDURE (v: CalculateLevel) Do (name: BugsNames.Name);
		VAR
			node: GraphNodes.Node;
			list: GraphLogical.List;
			p: GraphLogical.Node;
			all: BOOLEAN;
	BEGIN
		IF ~name.passByreference THEN RETURN END;
		node := name.components[v.index];
		IF node # NIL THEN
			WITH node: GraphLogical.Node DO
				all := TRUE;
				IF node.level = GraphLogical.undefined THEN
					v.new := TRUE;
					list := GraphLogical.Parents(node, all);
					WHILE list # NIL DO
						p := list.node;
						IF p.level = GraphLogical.undefined THEN
							p.CalculateLevel
						END;
						list := list.next
					END;
					node.CalculateLevel
				END
			ELSE
			END
		END
	END Do;

	PROCEDURE CalculateLevels;
		VAR
			v: CalculateLevel;
	BEGIN
		NEW(v);
		REPEAT
			v.new := FALSE;
			BugsIndex.Accept(v)
		UNTIL ~v.new
	END CalculateLevels;

	(*	Calculates level of logical parents of stochastic nodes, needed for mixture models	*)
	PROCEDURE (v: CalculateLevel1) Do (name: BugsNames.Name);
		VAR
			list: GraphLogical.List;
			node: GraphNodes.Node;
			logical: GraphLogical.Node;
			all: BOOLEAN;
	BEGIN
		IF ~name.passByreference THEN RETURN END;
		node := name.components[v.index];
		IF node # NIL THEN
			IF (node IS GraphStochastic.Node) & ~(GraphStochastic.hidden IN node.props) THEN
				all := TRUE;
				list := GraphLogical.Parents(node, all);
				WHILE list # NIL DO
					logical := list.node;
					IF logical.level = GraphLogical.undefined THEN
						logical.CalculateLevel
					END;
					list := list.next
				END
			END
		END
	END Do;

	PROCEDURE CalculateLevels1;
		VAR
			v: CalculateLevel1;
	BEGIN
		NEW(v);
		BugsIndex.Accept(v)
	END CalculateLevels1;

	PROCEDURE (v: CalculateDepth) Do (name: BugsNames.Name);
		VAR
			node: GraphNodes.Node;
			list: GraphStochastic.List;
			p: GraphStochastic.Node;
			all: BOOLEAN;
	BEGIN
		(*	what if chain graph ???	*)
		IF ~name.passByreference THEN RETURN END;
		node := name.components[v.index];
		IF (node # NIL) & ~(GraphStochastic.hidden IN node.props) THEN
			WITH node: GraphStochastic.Node DO
				all := TRUE;
				list := GraphStochastic.Parents(node, all);
				WHILE list # NIL DO
					p := list.node;
					IF p.depth = GraphStochastic.undefined THEN
						p.CalculateDepth;
						IF p.depth # GraphStochastic.undefined THEN
							v.maxDepth := MAX(v.maxDepth, p.depth)
						END
					END;
					list := list.next
				END;
				IF node.depth = GraphStochastic.undefined THEN
					node.CalculateDepth;
					IF node.depth # GraphStochastic.undefined THEN
						v.maxDepth := MAX(v.maxDepth, node.depth);
						v.new := TRUE
					END
				END;
				IF ~(GraphNodes.data IN node.props) & (node.depth > 0) THEN
					v.maxStochDepth := MAX(v.maxStochDepth, node.depth)
				END
			ELSE
			END
		END
	END Do;

	PROCEDURE CalculateDepths;
		VAR
			v: CalculateDepth;
	BEGIN
		NEW(v);
		v.maxDepth := 0;
		v.maxStochDepth := 0;
		REPEAT
			v.new := FALSE;
			BugsIndex.Accept(v)
		UNTIL ~v.new;
		GraphNodes.SetDepth(v.maxDepth, v.maxStochDepth)
	END CalculateDepths;

	(*

	The optimization procedures for logical nodes process the nodes by level (where level one nodes only
	have stochastic parents, level two nodes only level one logical or stochastic parents etc.)

	Once the final form of the logical nodes has been written the stochastic nodes are written. The
	topological depth of each stochastic node in the graphical model is calculated

	*)

	PROCEDURE WriteGraph (OUT ok: BOOLEAN);
		VAR
			model: BugsParser.Statement;
	BEGIN
		model := BugsParser.model;
		UpdaterActions.Clear;
		BugsNodes.Allocate(model, ok); IF ~ok THEN RETURN END;
		BugsNodes.CreateSentinels(model, ok); IF ~ok THEN RETURN END;
		BugsNodes.CreateConstants(model, ok);
		BugsNodes.CreateStochastics(model, ok); IF ~ok THEN RETURN END;
		BugsNodes.CreateLogicals(model, ok); IF ~ok THEN RETURN END;
		BugsNodes.WriteLogicals(model, ok); IF~ok THEN RETURN END;
		BugsNodes.WriteStochastics(model, ok); IF ~ok THEN RETURN END;
		CalculateLevels;
		CalculateLevels1;
		CalculateDepths;
	END WriteGraph;

	PROCEDURE CreateDeviance*;
		VAR
			name: BugsNames.Name;
			dev: GraphNodes.Node;
	BEGIN
		dev := GraphDeviance.fact.New();
		devianceExists := dev # NIL;
		IF devianceExists THEN
			name := BugsNames.New("deviance", 0);
			name.passByreference := TRUE;
			name.AllocateNodes;
			name.components[0] := dev;
			BugsIndex.Store(name)
		END
	END CreateDeviance;

	PROCEDURE LinkNode (node: GraphLogical.Node);
		VAR
			pLen, i: INTEGER;
			p: GraphNodes.Node;
			pList: GraphStochastic.List;
			parent: GraphStochastic.Node;
			components: GraphStochastic.Vector;
			all: BOOLEAN;
	BEGIN
		all := TRUE;
		pList := GraphStochastic.Parents(node, all);
		WHILE pList # NIL DO
			parent := pList.node;
			WITH parent: GraphConjugateMV.Node DO
				pLen := parent.Size();
				components := parent.components;
				i := 0;
				WHILE i < pLen DO
					components[i].AddDependent(node);
					INC(i)
				END
			ELSE
				parent.AddDependent(node)
			END;
			pList := pList.next
		END;
		p := node;
		INCL(p.props, GraphLogical.linked);
	END LinkNode;

	PROCEDURE (v: CountStochastic) Do (name: BugsNames.Name);
		VAR
			node: GraphNodes.Node;
	BEGIN
		IF name.passByreference THEN
			node := name.components[v.index];
			IF (node # NIL) & (node IS GraphStochastic.Node) & ~(GraphNodes.data IN node.props) THEN
				INC(v.num)
			END
		END
	END Do;

	PROCEDURE (v: BuildDependent) Do (name: BugsNames.Name);
		VAR
			node: GraphNodes.Node;
			list: GraphLogical.List;
			logical: GraphLogical.Node;
			all: BOOLEAN;
			i, size: INTEGER;
	BEGIN
		IF name.passByreference THEN
			node := name.components[v.index];
			IF (node # NIL) & (node = node.Representative()) THEN
				all := TRUE;
				list := GraphLogical.Parents(node, all);
				WITH node: GraphScalar.Node DO
					node.AddToList(list);
				|node: GraphVector.Node DO
					size := node.Size();
					i := 0; WHILE i < size DO node.components[i].AddToList(list); INC(i) END
				ELSE
				END;
				WHILE list # NIL DO
					logical := list.node;
					IF ~(GraphLogical.linked IN logical.props) THEN
						LinkNode(logical)
					END;
					list := list.next
				END
			END
		END
	END Do;

	PROCEDURE AllocateCaches;
		VAR
			v: CountStochastic;
			cursor: GraphStochastic.List;
			numStoch: INTEGER;
	BEGIN
		NEW(v);
		v.num := 0;
		BugsIndex.Accept(v);
		numStoch := v.num;
		cursor := GraphStochastic.auxillary;
		WHILE cursor # NIL DO INC(numStoch); cursor := cursor.next END;
		GraphStochastic.InitCache(numStoch);
	END AllocateCaches;

	PROCEDURE BuildDependantLists;
		VAR
			v: BuildDependent;
			cursor: GraphStochastic.List;
			node: GraphStochastic.Node;
			list: GraphLogical.List;
			logical: GraphLogical.Node;
			all: BOOLEAN;
	BEGIN
		NEW(v);
		BugsIndex.Accept(v);
		cursor := GraphStochastic.auxillary;
		WHILE cursor # NIL DO
			node := cursor.node;
			all := TRUE;
			list := GraphLogical.Parents(node, all); 
			WHILE list # NIL DO
				logical := list.node;
				IF ~(GraphLogical.linked IN logical.props) THEN
					LinkNode(logical)
				END;
				list := list.next
			END;
			cursor := cursor.next
		END;
	END BuildDependantLists;

	PROCEDURE Compile* (numChains: INTEGER; updaterByMethod: BOOLEAN; OUT ok: BOOLEAN);
		VAR
			inits: BOOLEAN;
			chain, i, j, numStoch: INTEGER;
			nodes: GraphStochastic.Vector;
			logicals: GraphLogical.Vector;
	BEGIN
		BugsRandnum.CreateGenerators(numChains);
		WriteGraph(ok); IF ~ok THEN BugsParser.Clear; UpdaterActions.Clear; RETURN END;
		AllocateCaches;
		BuildFullConditionals;
		BuildDependantLists;
		ReadCaches;
		ClassifyConditionals;
		IF updaterByMethod THEN
			CreateUpdatersByMethod
		ELSE
			CreateUpdatersByNode
		END;
		MissingUpdaters(ok); IF ~ok THEN BugsParser.Clear; UpdaterActions.Clear; RETURN END;
		UpdaterActions.CreateUpdaters(numChains);
		IF IsAdapting(numChains) THEN
			UpdaterActions.SetAdaption(0, MAX(INTEGER))
		ELSE
			UpdaterActions.SetAdaption(0, 0)
		END;
		CalculateHints;
		UpdaterActions.StoreStochastics;
		nodes := GraphStochastic.nodes;
		IF nodes # NIL THEN
			numStoch := LEN(nodes);
			NEW(initialized, numChains);
			i := 0;
			WHILE i < numChains DO
				NEW(initialized[i], numStoch);
				j := 0; WHILE j < numStoch DO initialized[i, j] := FALSE; INC(j) END;
				INC(i)
			END
		END;
		logicals := GraphStochastic.Dependents(nodes);
		GraphLogical.SetLogicals(logicals, numChains);
		InitDiffs; 
		CreateDeviance;
		(*	stochastic nodes could be set up with initialized flag so propage this info to all chains	*)
		chain := 0;
		WHILE chain < numChains DO
			GraphStochastic.StoreValues(chain);
			StoreInits(chain);
			INC(chain)
		END;
		(*	special case where no stochastic nodes in model	*)
		inits := IsInitialized(0);
		IF inits THEN
			BugsNodes.Checks(ok);
			IF ~ok THEN BugsParser.Clear; UpdaterActions.Clear; RETURN END
		END
	END Compile;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		initialized := NIL;
		Maintainer
	END Init;

BEGIN
	Init
END BugsGraph.
