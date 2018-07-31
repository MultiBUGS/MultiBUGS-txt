(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsGraph;

	

	IMPORT
		Strings,
		BugsIndex, BugsMsg,
		BugsNames, BugsNodes, BugsParser, BugsRandnum,
		GraphConjugateMV, GraphDeviance, GraphLogical, GraphNodes, GraphRules,
		GraphStochastic, GraphUnivariate, GraphVector,
		UpdaterActions, UpdaterMethods, UpdaterUpdaters;

	TYPE
		BuildConditional = POINTER TO RECORD(BugsNames.ElementVisitor)
			repeat: BOOLEAN
		END;

		AllocateLikelihood = POINTER TO RECORD(BugsNames.ElementVisitor) END;

		ClassifyConditional = POINTER TO RECORD(BugsNames.ElementVisitor) END;

		CalculateDepth = POINTER TO RECORD(BugsNames.ElementVisitor)
			new: BOOLEAN;
			maxDepth, maxStochDepth: INTEGER;
		END;

		ConditionalOfClass = POINTER TO RECORD(BugsNames.ElementVisitor)
			class: SET;
			list: GraphStochastic.List
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
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

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
		BugsMsg.Store(errorMsg)
	END Error;

	PROCEDURE (v: BuildConditional) Do (name: BugsNames.Name);
		VAR
			node: GraphNodes.Node;
	BEGIN
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
	END Do;

	PROCEDURE BuildFullConditionals;
		VAR
			v: BuildConditional;
	BEGIN
		NEW(v);
		REPEAT
			v.repeat := FALSE; ;
			BugsIndex.Accept(v)
		UNTIL v.repeat = FALSE
	END BuildFullConditionals;

	PROCEDURE (v: AllocateLikelihood) Do (name: BugsNames.Name);
		VAR
			node: GraphNodes.Node;
	BEGIN
		node := name.components[v.index];
		IF node # NIL THEN
			WITH node: GraphStochastic.Node DO
				IF ~(GraphStochastic.data IN node.props) THEN
					node.AllocateLikelihood
				END
			ELSE
			END
		END
	END Do;

	PROCEDURE AllocateLikelihoods;
		VAR
			v: AllocateLikelihood;
	BEGIN
		NEW(v);
		BugsIndex.Accept(v)
	END AllocateLikelihoods;

	PROCEDURE (v: ClassifyConditional) Do (name: BugsNames.Name);
		VAR
			node: GraphNodes.Node;
	BEGIN
		node := name.components[v.index];
		IF node # NIL THEN
			WITH node: GraphStochastic.Node DO
				IF {GraphNodes.data, GraphStochastic.hidden} * node.props = {} THEN
					node.ClassifyConditional;
				END
			ELSE
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
									comp[i].SetProps(comp[i].props + {GraphNodes.mark});
									INC(i)
								END
							END
						ELSE
							GraphStochastic.AddToList(stoch, v.list);
							stoch.SetProps(stoch.props + {GraphNodes.mark})
						END;
					END
				END
			ELSE
			END
		END
	END Do;

	PROCEDURE ConditionalsOfClass* (class: SET): GraphStochastic.Vector;
		VAR
			v: ConditionalOfClass;
			vector: GraphStochastic.Vector;
			cursor, list: GraphStochastic.List;
			node: GraphStochastic.Node;
	BEGIN
		NEW(v);
		v.class := class;
		v.list := NIL;
		BugsIndex.Accept(v);
		list := v.list;
		vector := GraphStochastic.ListToVector(list);
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
		node := name.components[v.index];
		IF node # NIL THEN
			IF node IS GraphStochastic.Node THEN
				stochastic := node(GraphStochastic.Node);
				IF noUpdater * stochastic.props = {} THEN
					factory := v.factory;
					IF (UpdaterUpdaters.enabled IN factory.props) & factory.CanUpdate(stochastic) THEN
						updater := factory.New(stochastic);
						UpdaterActions.RegisterUpdater(updater);
						factory.SetProps(factory.props + {UpdaterUpdaters.active})
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
			string: ARRAY 64 OF CHAR;
			updater: UpdaterUpdaters.Updater;
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

	(*	for distributions with two parameters try and determine which parameter is
	active (being sampled from).	*)

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
		node := name.components[v.index];
		IF node # NIL THEN
			IF node IS GraphUnivariate.Node THEN
				stochastic := node(GraphStochastic.Node);
				IF GraphStochastic.update IN node.props THEN
					hints := {};
					children := stochastic.Children();
					IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
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
							stochastic.SetProps(stochastic.props + hints)
						END;
						INC(i)
					END;
					IF (GraphStochastic.hint1 IN stochastic.props)
						 & (GraphStochastic.hint2 IN stochastic.props) THEN
						stochastic.SetProps(stochastic.props - {GraphStochastic.hint1, GraphStochastic.hint2})
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
					(*; ASSERT(node.level # GraphLogical.undefined, 66)*)
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
		IF (dev # NIL) & (GraphDeviance.DevianceTerms(dev) # NIL) THEN
			devianceExists := TRUE;
			name := BugsNames.New("deviance", 0);
			name.AllocateNodes;
			name.components[0] := dev;
			BugsIndex.Store(name)
		ELSE
			devianceExists := FALSE
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
		IF GraphLogical.dependent IN node.props THEN
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
			p := node.Representative();
			p.SetProps(p.props + {GraphLogical.linked});
			IF GraphLogical.dependent IN p.props THEN
				p.SetProps(p.props - {GraphLogical.alwaysEvaluate})
			END
		END
	END LinkNode;

	PROCEDURE (v: BuildDependent) Do (name: BugsNames.Name);
		VAR
			node: GraphNodes.Node;
			list: GraphLogical.List;
			logical: GraphLogical.Node;
			all: BOOLEAN;
	BEGIN
		node := name.components[v.index];
		IF (node # NIL) & (node = node.Representative()) THEN
			all := TRUE;
			list := GraphLogical.Parents(node, all);
			WITH node: GraphLogical.Node DO
				node.AddToList(list)
			ELSE
			END;
			WHILE list # NIL DO
				logical := list.node;
				IF ~(GraphLogical.linked IN logical.props) THEN
					logical := logical.Representative();
					LinkNode(logical)
				END;
				list := list.next
			END
		END
	END Do;

	PROCEDURE BuildDependantLists;
		VAR
			v: BuildDependent;
	BEGIN
		NEW(v);
		BugsIndex.Accept(v)
	END BuildDependantLists;

	PROCEDURE Compile* (numChains: INTEGER; updaterByMethod: BOOLEAN; OUT ok: BOOLEAN);
		VAR
			inits: BOOLEAN;
			chain: INTEGER;
	BEGIN
		BugsRandnum.CreateGenerators(numChains);
		WriteGraph(ok); IF ~ok THEN BugsParser.Clear; UpdaterActions.Clear; RETURN END;
		BuildFullConditionals;
		AllocateLikelihoods;
		ClassifyConditionals;
		IF updaterByMethod THEN
			CreateUpdatersByMethod
		ELSE
			CreateUpdatersByNode
		END;
		MissingUpdaters(ok); IF ~ok THEN BugsParser.Clear; UpdaterActions.Clear; RETURN END;
		UpdaterActions.InsertConstraints;
		UpdaterActions.CreateUpdaters(numChains);
		UpdaterActions.AllocateLikelihoods;
		IF IsAdapting(numChains) THEN
			UpdaterActions.SetAdaption(0, MAX(INTEGER))
		ELSE
			UpdaterActions.SetAdaption(0, 0)
		END;
		BuildDependantLists;
		CalculateHints;
		UpdaterActions.StoreStochastics;
		CreateDeviance;
		(*	if any stochastic nodes are set up with initialized flag propage this info to all chains	*)
		chain := 0;
		WHILE chain < numChains DO
			UpdaterActions.StoreSamples(chain);
			INC(chain)
		END;
		UpdaterActions.LoadSamples(0);
		(*	special case where no stochastic nodes in model	*)
		inits := UpdaterActions.IsInitialized(0);
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

BEGIN
	Maintainer
END BugsGraph.
