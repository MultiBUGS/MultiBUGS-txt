(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	  *)

MODULE GraphStochastic;


	

	IMPORT
		SYSTEM,
		Stores := Stores64,
		GraphLogical, GraphNodes, GraphRules;

	CONST
		(*	node properties	*)
		data* = GraphNodes.data;

		mark* = 3; 	(*	node is marked	*)
		censored* = 4; 	(*	node is censored	*)
		truncated* = 5; 	(*	node is truncated	*)
		leftNatural* = 6; 	(*	node's support has left / lower bound	*)
		rightNatural* = 7; 	(*	node's support has right / upper bound	*)
		leftImposed* = 8; 	(* node is either left censored or left truncated	*)
		rightImposed* = 9; 	(* node is either right censored or right truncated	*)
		integer* = 10; 	(*	node has supprt on the integers	*)
		update* = 11; 	(*	node needs updating by sampling algorithm	*)
		initialized* = 12; 	(*	node has been given an initial value	*)
		likelihood* = 13; 	(*	nodes has been added to likelihood list of its parents	*)
		hasLikelihood* = 14; (*	node has likelihood	*)
		devParent* = 15; 	(*	node is stochastic parent of deviance	*)
		noCDF* = 16; 	(*	cdf can not be expressed in closed form	*)
		noPDF* = 17; 	(*	pdf can not be expressed in closed form	*)
		noMean* = 18; (*	node does not have mean	*)
		logical* = 19; 	(*	marks node as originaly being a logical node	*)
		optimizeDiffs* = 20; 	(*	skip calculating diffs	*)
		hidden* = 21; 	(*	node is not relevant and should not be used in normal way	*)
		hint1* = 22; 	(*	hints to optimize differentiation	*)
		hint2* = 23;

		distributed* = 31; (*	distribute sampling over multiple processors	*)



		undefined* = 0; (*	topological depth of node in graph is undefined / not yet calculated	*)

		bounds* = {leftNatural, leftImposed, rightNatural, rightImposed};

	TYPE

		List* = POINTER TO LIMITED RECORD
			node-: Node;
			next-: List
		END;

		Node* = POINTER TO ABSTRACT RECORD(GraphNodes.Node)
			depth-: INTEGER;
			classConditional-: INTEGER;
			children-: Vector;
			dependents-: GraphLogical.Vector;
		END;

		Vector* = POINTER TO ARRAY OF Node;

		Factory* = POINTER TO ABSTRACT RECORD (GraphNodes.Factory) END;

		Args* = RECORD(GraphNodes.Args)
			numScalars*, numVectors*: INTEGER;
			scalars*: ARRAY 10 OF GraphNodes.Node;
			leftCen*, rightCen*, leftTrunc*, rightTrunc*: GraphNodes.Node;
			vectors*: ARRAY 10 OF GraphNodes.SubVector;
		END;

		ArgsLogical* = RECORD (GraphNodes.Args)
			numConsts*, numLogicals*, numStochs*, numScalars*, numOps*, numVectors*: INTEGER;
			consts*: ARRAY 50 OF REAL;
			logicals*: ARRAY 50 OF GraphLogical.Node;
			stochastics*: ARRAY 50 OF Node;
			scalars*: ARRAY 50 OF GraphNodes.Node;
			ops*: ARRAY 100 OF INTEGER;
			vectors*: ARRAY 10 OF GraphNodes.SubVector
		END;

	VAR
		cacheLikelihood: POINTER TO ARRAY OF List;
		cacheDependents: POINTER TO ARRAY OF GraphLogical.List;
		values-: POINTER TO ARRAY OF POINTER TO ARRAY OF REAL;
		dependentsFilter-: SET;
		nodes-: Vector;
		auxillary-: List;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE ^AddToList* (node: Node; VAR list: List);

	PROCEDURE ^ListToVector* (list: List): Vector;

	PROCEDURE ^Parents* (node: GraphNodes.Node; all: BOOLEAN): List;

		(*	abstract node methods	*)

	PROCEDURE (node: Node) Bounds* (OUT lower, upper: REAL), NEW, ABSTRACT;

	PROCEDURE (node: Node) CanSample* (multiVar: BOOLEAN): BOOLEAN, NEW, ABSTRACT;

	PROCEDURE (node: Node) ClassifyLikelihood* (parent: Node): INTEGER, NEW, ABSTRACT;

	PROCEDURE (node: Node) ClassifyPrior* (): INTEGER, NEW, ABSTRACT;

	PROCEDURE (node: Node) Deviance* (): REAL, NEW, ABSTRACT;

	PROCEDURE (node: Node) DiffLogConditional* (): REAL, NEW, ABSTRACT;

	PROCEDURE (node: Node) DiffLogLikelihood* (x: Node): REAL, NEW, ABSTRACT;

	PROCEDURE (node: Node) DiffLogPrior* (): REAL, NEW, ABSTRACT;

	PROCEDURE (node: Node) ExternalizeStochastic- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (node: Node) InitStochastic-, NEW, ABSTRACT;

	PROCEDURE (node: Node) InternalizeStochastic- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (node: Node) InvMap* (y: REAL), NEW, ABSTRACT;

	PROCEDURE (node: Node) IsLikelihoodTerm- (): BOOLEAN, NEW, ABSTRACT;

	PROCEDURE (node: Node) Location* (): REAL, NEW, ABSTRACT;

	PROCEDURE (node: Node) LogDetJacobian* (): REAL, NEW, ABSTRACT;

	PROCEDURE (node: Node) LogLikelihood* (): REAL, NEW, ABSTRACT;

	PROCEDURE (node: Node) LogPrior* (): REAL, NEW, ABSTRACT;

	PROCEDURE (node: Node) Map* (): REAL, NEW, ABSTRACT;

	PROCEDURE (node: Node) Sample* (OUT res: SET), NEW, ABSTRACT;

		(*	concrete node methods	*)

	PROCEDURE (node: Node) AddDependent* (dependent: GraphLogical.Node), NEW;
		VAR
			element: GraphLogical.List;
			label: INTEGER;
	BEGIN
		ASSERT(dependent # NIL, 21);
		label := node.label;
		IF label > 0 THEN
			element := cacheDependents[label - 1];
			dependent.AddToList(element);
			cacheDependents[label - 1] := element
		END
	END AddDependent;

	PROCEDURE (node: Node) BuildLikelihood*, NEW;
		VAR
			list, pList: List;
			p: Node;
			all: BOOLEAN;

		PROCEDURE AddLikelihood (likelihoodTerm, prior: Node);
			VAR
				element: List;
				label: INTEGER;
		BEGIN
			IF ~(GraphNodes.mark IN likelihoodTerm.props) THEN
				label := prior.label;
				IF label > 0 THEN
					NEW(element);
					element.node := likelihoodTerm;
					element.next := cacheLikelihood[label - 1];
					cacheLikelihood[label - 1] := element
				END
			END
		END AddLikelihood;

	BEGIN
		ASSERT(~(likelihood IN node.props), 21);
		IF node.IsLikelihoodTerm() THEN
			all := FALSE;
			INCL(node.props, likelihood);
			list := Parents(node, all);
			WHILE list # NIL DO
				p := list.node;
				IF data IN node.props THEN
					AddLikelihood(node, p);
					INCL(p.props, hasLikelihood)
				ELSE
					pList := Parents(p, all);
					WHILE (pList # NIL) & (pList.node # node) DO
						pList := pList.next
					END;
					IF pList = NIL THEN
						AddLikelihood(node, p);
						INCL(p.props, hasLikelihood)
					END
				END;
				list := list.next
			END
		END
	END BuildLikelihood;

	(*	tries to calculate nodes topological depth in graph	*)
	PROCEDURE (node: Node) CalculateDepth*, NEW;
		VAR
			maxDepth, minDepth: INTEGER;
			p: Node;
			list, pList: List;
			all: BOOLEAN;
	BEGIN
		ASSERT(node.depth = undefined, 21);
		all := TRUE;
		list := Parents(node, all);
		IF list = NIL THEN
			node.depth := 1
		ELSE
			maxDepth := undefined;
			minDepth := 1;
			WHILE list # NIL DO
				p := list.node;
				pList := Parents(p, all);
				WHILE (pList # NIL) & (pList.node # node) DO (*	undirected link?	*)
					pList := pList.next
				END;
				IF pList = NIL THEN
					maxDepth := MAX(maxDepth, p.depth);
					minDepth := MIN(minDepth, p.depth)
				END;
				list := list.next
			END;
			IF minDepth # undefined THEN node.depth := maxDepth + 1 END
		END
	END CalculateDepth;

	PROCEDURE (node: Node) ClassifyConditional*, NEW;
		VAR
			classConditional, classLikelihood, i, num: INTEGER;
			children: Vector;
	BEGIN
		ASSERT({data, hidden} * node.props = {}, 21);
		classConditional := node.ClassifyPrior();
		children := node.children;
		IF children # NIL THEN
			num := LEN(children);
			i := 0;
			WHILE (i < num) & (classConditional # GraphRules.invalid) DO
				classLikelihood := children[i].ClassifyLikelihood(node);
				classConditional := GraphRules.product[classConditional, classLikelihood];
				INC(i)
			END
		END;
		node.classConditional := classConditional
	END ClassifyConditional;

	PROCEDURE (node: Node) ClearDiffs*, NEW;
		VAR
			i, num: INTEGER;
			dependents: GraphLogical.Vector;
	BEGIN
		dependents := node.dependents;
		IF dependents # NIL THEN
			num := LEN(dependents);
			i := 0; WHILE i < num DO EXCL(dependents[i].props, GraphNodes.mark); INC(i) END
		END
	END ClearDiffs;

	PROCEDURE (node: Node) Diff* (x: GraphNodes.Node): REAL;
	BEGIN
		IF node = x THEN RETURN 1.0 ELSE RETURN 0.0 END
	END Diff;

	PROCEDURE (node: Node) Evaluate*, NEW;
	BEGIN
		GraphLogical.Evaluate(node.dependents)
	END Evaluate;

	PROCEDURE (node: Node) EvaluateDiffs*, NEW;
		VAR
			dependents: GraphLogical.Vector;
	BEGIN
		dependents := node.dependents;
		GraphLogical.EvaluateDiffs(dependents)
	END EvaluateDiffs;

	(*	writes internal base fields of stochastic node to store	*)
	PROCEDURE (node: Node) ExternalizeNode- (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteInt(node.depth);
		wr.WriteInt(node.classConditional);
		IF data IN node.props THEN wr.WriteReal(node.value) END;
		GraphLogical.ExternalizeVector(node.dependents, dependentsFilter, wr);
		node.ExternalizeStochastic(wr);
	END ExternalizeNode;

	(*	initialize base fields of stochastic node	*)
	PROCEDURE (node: Node) InitNode-;
	BEGIN
		node.value := 0.0;
		node.depth := undefined;
		node.children := NIL;
		node.dependents := NIL;
		node.classConditional := GraphRules.invalid;
		node.InitStochastic
	END InitNode;

	(*	read internal base fields of stochastic node from store	*)
	PROCEDURE (node: Node) InternalizeNode- (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadInt(node.depth);
		rd.ReadInt(node.classConditional);
		IF data IN node.props THEN rd.ReadReal(node.value) END;
		node.dependents := GraphLogical.InternalizeVector(rd);
		node.InternalizeStochastic(rd)
	END InternalizeNode;

	PROCEDURE (node: Node) ParentsInitialized* (): BOOLEAN, NEW;
		VAR
			all, parentsInitialized: BOOLEAN;
			list: List;
			logicalList: GraphLogical.List;
			dependents: GraphLogical.Vector;
			p: Node;
	BEGIN
		parentsInitialized := TRUE;
		all := TRUE;
		list := Parents(node, all);
		WHILE parentsInitialized & (list # NIL) DO
			p := list.node;
			parentsInitialized := initialized IN p.props;
			list := list.next
		END;
		IF parentsInitialized THEN
			logicalList := GraphLogical.Parents(node, all);
			dependents := GraphLogical.ListToVector(logicalList);
			GraphLogical.Evaluate(dependents)
		END;
		RETURN parentsInitialized
	END ParentsInitialized;

	PROCEDURE (node: Node) Representative* (): Node, ABSTRACT;

	PROCEDURE (node: Node) SetChildren* (children: Vector), NEW;
	BEGIN
		node.children := children
	END SetChildren;

	PROCEDURE (node: Node) SetDependents* (dependents: GraphLogical.Vector), NEW;
		VAR
			p: GraphNodes.Node;
			label: INTEGER;
	BEGIN
		node.dependents := dependents
	END SetDependents;

	PROCEDURE (node: Node) SetLikelihood*, NEW;
		VAR
			p: GraphNodes.Node;
			label: INTEGER;
			children: Vector;
			dependents: GraphLogical.Vector;
	BEGIN
		p := node.Representative();
		IF p = node THEN
			label := GraphNodes.Label(node);
			label := ABS(label);
			children := ListToVector(cacheLikelihood[label - 1]);
			dependents := GraphLogical.ListToVector(cacheDependents[label - 1]);
			IF dependents # NIL THEN
				ASSERT(node.dependents = NIL, 33);
				node.dependents := dependents
			END;
			IF children # NIL THEN
				ASSERT(node.children = NIL, 33);
				node.children := children
			END
		ELSE
			dependents := p(Node).dependents;
			node.dependents := dependents;
			children := p(Node).children;
			node.children := children
		END
	END SetLikelihood;

	(*	Factory methods	*)
	PROCEDURE (f: Factory) New* (): Node, ABSTRACT;

	PROCEDURE (f: Factory) NumParam* (): INTEGER;
		VAR
			signature: ARRAY 64 OF CHAR;
			numParam: INTEGER;
			ch: CHAR;
	BEGIN
		f.Signature(signature);
		numParam := LEN(signature$);
		IF numParam > 0 THEN
			ch := signature[numParam - 1];
			IF (ch = "C") OR (ch = "T") THEN DEC(numParam) END;
			IF numParam > 0 THEN
				ch := signature[numParam - 1];
				IF (ch = "C") OR (ch = "T") THEN DEC(numParam) END
			END
		END;
		RETURN numParam
	END NumParam;

	(*	Args methods	*)
	PROCEDURE (VAR args: Args) Init*;
		VAR
			i: INTEGER;
	BEGIN
		args.valid := TRUE;
		args.numScalars := 0;
		args.numVectors := 0;
		args.leftCen := NIL;
		args.rightCen := NIL;
		args.leftTrunc := NIL;
		args.rightTrunc := NIL;
		i := 0; WHILE i < LEN(args.vectors) DO args.vectors[i].Init; INC(i) END
	END Init;

	PROCEDURE (VAR args: ArgsLogical) Init*;
		VAR
			i: INTEGER;
	BEGIN
		args.valid := TRUE;
		args.numStochs := 0;
		args.numLogicals := 0;
		args.numConsts := 0;
		args.numScalars := 0;
		args.numOps := 0;
		args.numVectors := 0;
		i := 0;
		WHILE i < LEN(args.logicals) DO
			args.logicals[i] := NIL;
			args.stochastics[i] := NIL;
			args.scalars[i] := NIL;
			INC(i)
		END;
		i := 0; WHILE i < LEN(args.vectors) DO args.vectors[i].Init; INC(i) END
	END Init;

	PROCEDURE HeapSort (VAR nodes: GraphLogical.Vector);
		VAR
			i, j, k, len: INTEGER;

		PROCEDURE Less (l, m: INTEGER): BOOLEAN;
			VAR
				level0, level1: INTEGER;
		BEGIN
			level0 := nodes[l - 1].level;
			level1 := nodes[m - 1].level;
			IF level0 = level1 THEN
				RETURN nodes[l - 1].value > nodes[m - 1].value
			ELSE
				RETURN level0 < level1
			END
		END Less;

		PROCEDURE Swap (l, m: INTEGER);
			VAR
				temp: GraphLogical.Node;
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

	PROCEDURE AddMarks* (vector: Vector; mark: SET);
		VAR
			i, size: INTEGER;
	BEGIN
		IF vector # NIL THEN size := LEN(vector) ELSE size := 0 END;
		i := 0;
		WHILE i < size DO
			vector[i].props := vector[i].props + mark;
			INC(i)
		END
	END AddMarks;

	PROCEDURE AddToList* (node: Node; VAR list: List);
		VAR
			cursor: List;
	BEGIN
		IF ~(GraphNodes.mark IN node.props) THEN
			NEW(cursor);
			cursor.node := node;
			cursor.next := list;
			list := cursor
		END
	END AddToList;

	PROCEDURE ClassFunction* (node: GraphNodes.Node; parent: Node): INTEGER;
		VAR
			parents: GraphNodes.Vector;
			all: BOOLEAN;
			class, i, num: INTEGER;
		CONST
			eps = 1.0E-6;
	BEGIN
		ASSERT(parent IS Node, 21);
		WITH node: GraphLogical.Node DO
			parents := node.parents;
			IF parents # NIL THEN num := LEN(parents) ELSE num := 0 END;
			i := 0; WHILE (i < num) & (parents[i] # parent) DO INC(i) END;
			IF i < num THEN 
				class := SHORT(ENTIER(node.work[i] + eps))
			ELSE 
				 class := GraphRules.const
			END;
		|node: Node DO
			IF node = parent THEN
				class := GraphRules.ident
			ELSIF mark IN node.props THEN
				class := GraphRules.linear
			ELSE
				class := GraphRules.const
			END
		ELSE
			class := GraphRules.const
		END;
		RETURN class
	END ClassFunction;

	PROCEDURE Clear*;
	BEGIN
		nodes := NIL;
		values := NIL;
		auxillary := NIL;
		cacheLikelihood := NIL;
		cacheDependents := NIL;
		dependentsFilter := {MIN(SET)..MAX(SET)};
	END Clear;

	(*	clear likelihood and dependent caches	*)
	PROCEDURE ClearCache*;
	BEGIN
		cacheLikelihood := NIL;
		cacheDependents := NIL;
		GraphNodes.UnlabelNodes
	END ClearCache;

	PROCEDURE ClearMarks* (vector: Vector; mark: SET);
		VAR
			i, size: INTEGER;
	BEGIN
		IF vector # NIL THEN size := LEN(vector) ELSE size := 0 END;
		i := 0;
		WHILE i < size DO
			vector[i].props := vector[i].props - mark;
			INC(i)
		END
	END ClearMarks;

	PROCEDURE Dependents* (stochastics: Vector): GraphLogical.Vector;
		VAR
			i, j, numDep, numLogical, numStoch: INTEGER;
			dependents, logicals: GraphLogical.Vector;
			logical: GraphLogical.Node;
	BEGIN
		logicals := NIL;
		IF stochastics # NIL THEN
			numStoch := LEN(stochastics);
			numLogical := 0;
			i := 0;
			WHILE i < numStoch DO
				dependents := stochastics[i].dependents;
				IF dependents # NIL THEN
					numDep := LEN(dependents);
					j := 0;
					WHILE j < numDep DO
						logical := dependents[j];
						IF ~(GraphNodes.mark IN logical.props) THEN
							logical.value := numLogical;
							INC(numLogical);
							INCL(logical.props, GraphNodes.mark);
						END;
						INC(j)
					END
				END;
				INC(i)
			END;
			IF numLogical > 0 THEN NEW(logicals, numLogical) END;
			numLogical := 0;
			i := 0;
			WHILE i < numStoch DO
				dependents := stochastics[i].dependents;
				IF dependents # NIL THEN
					numDep := LEN(dependents);
					j := 0;
					WHILE j < numDep DO
						logical := dependents[j];
						IF GraphNodes.mark IN logical.props THEN
							logicals[numLogical] := logical;
							INC(numLogical);
							EXCL(logical.props, GraphNodes.mark);
						END;
						INC(j)
					END
				END;
				INC(i)
			END;
		END;
		IF logicals # NIL THEN HeapSort(logicals) END;
		RETURN logicals
	END Dependents;

	(*	writes an array of pointers to stochastic nodes to store	*)
	PROCEDURE ExternalizeVector* (vector: Vector; VAR wr: Stores.Writer);
		VAR
			i, num: INTEGER;
	BEGIN
		IF vector # NIL THEN num := LEN(vector) ELSE num := 0 END;
		i := 0;
		wr.WriteInt(num);
		WHILE i < num DO
			GraphNodes.Externalize(vector[i], wr);
			INC(i)
		END;
	END ExternalizeVector;

	PROCEDURE ExternalizeValues* (VAR wr: Stores.Writer);
		VAR
			chain, i, numChains, numStoch: INTEGER;
	BEGIN
		IF values # NIL THEN
			numChains := LEN(values);
			numStoch := LEN(values[0]);
			chain := 0;
			WHILE chain < numChains DO
				i := 0; WHILE i < numStoch DO wr.WriteReal(values[chain, i]); INC(i) END;
				INC(chain)
			END
		END
	END ExternalizeValues;

	PROCEDURE FilterDependents* (filter: SET);
	BEGIN
		dependentsFilter := filter
	END FilterDependents;

	(*	initialize likelihood dependent cache	*)
	PROCEDURE InitCache* (numStoch: INTEGER);
		VAR
			i: INTEGER;
	BEGIN
		IF numStoch > 0 THEN
			NEW(cacheLikelihood, numStoch);
			NEW(cacheDependents, numStoch)
		END;
		i := 0;
		WHILE i < numStoch DO
			cacheLikelihood[i] := NIL;
			cacheDependents[i] := NIL;
			INC(i)
		END
	END InitCache;

	PROCEDURE InternalizeValues* (VAR rd: Stores.Reader);
		VAR
			chain, i, numChains, numStoch: INTEGER;
	BEGIN
		IF values # NIL THEN
			numChains := LEN(values);
			numStoch := LEN(values[0]);
			chain := 0;
			WHILE chain < numChains DO
				i := 0; WHILE i < numStoch DO rd.ReadReal(values[chain, i]); INC(i) END;
				INC(chain)
			END
		END
	END InternalizeValues;

	(*	reads an array of pointers to stochastic nodes from store	*)
	PROCEDURE InternalizeVector* (VAR rd: Stores.Reader): Vector;
		VAR
			node: GraphNodes.Node;
			i, num: INTEGER;
			vector: Vector;
	BEGIN
		vector := NIL;
		rd.ReadInt(num);
		IF num > 0 THEN
			NEW(vector, num);
			i := 0;
			WHILE i < num DO
				node := GraphNodes.Internalize(rd);
				vector[i] := node(Node);
				INC(i)
			END;
		END;
		RETURN vector
	END InternalizeVector;

	PROCEDURE IsBounded* (vector: Vector): BOOLEAN;
		VAR
			i, size: INTEGER;
			bounded: BOOLEAN;
	BEGIN
		size := LEN(vector);
		i := 0;
		bounded := FALSE;
		WHILE (i < size) & ~bounded DO
			bounded := bounds * vector[i].props # {};
			INC(i)
		END;
		RETURN bounded
	END IsBounded;

	PROCEDURE ListToVector* (list: List): Vector;
		VAR
			i, len: INTEGER;
			cursor: List;
			vector: Vector;
	BEGIN
		IF list = NIL THEN
			vector := NIL
		ELSE
			len := 0;
			cursor := list;
			WHILE cursor # NIL DO
				INC(len);
				cursor := cursor.next
			END;
			NEW(vector, len);
			(*	put list into vector in reverse order	*)
			i := len;
			cursor := list;
			WHILE cursor # NIL DO
				DEC(i);
				vector[i] := cursor.node;
				cursor := cursor.next
			END
		END;
		RETURN vector
	END ListToVector;

	PROCEDURE LoadValues* (chain: INTEGER);
		VAR
			i, numStoch: INTEGER;
	BEGIN
		IF nodes # NIL THEN
			numStoch := LEN(nodes);
			i := 0; WHILE i < numStoch DO nodes[i].value := values[chain, i]; INC(i) END
		END
	END LoadValues;

	PROCEDURE Parents* (node: GraphNodes.Node; all: BOOLEAN): List;
		VAR
			cursor, list: List;
			lList: GraphLogical.List;
			nList: GraphNodes.List;
			stochastic: Node;
			p, q: GraphNodes.Node;
	BEGIN
		list := NIL;
		nList := node.Parents(all);
		WHILE nList # NIL DO
			p := nList.node;
			p := p.Representative();
			WITH p: Node DO
				AddToList(p, list);
				INCL(p.props, GraphNodes.mark)
			ELSE
			END;
			nList := nList.next
		END;
		lList := GraphLogical.Parents(node, all);
		WHILE lList # NIL DO
			p := lList.node;
			IF node IS Node THEN
				EXCL(p.props, GraphLogical.prediction)
			END;
			nList := p.Parents(all);
			WHILE nList # NIL DO
				q := nList.node;
				q := q.Representative();
				WITH q: Node DO
					AddToList(q, list);
					INCL(q.props, GraphNodes.mark)
				ELSE
				END;
				nList := nList.next
			END;
			lList := lList.next
		END;
		cursor := list;
		WHILE cursor # NIL DO
			stochastic := cursor.node;
			EXCL(stochastic.props, GraphNodes.mark);
			cursor := cursor.next
		END;
		RETURN list
	END Parents;

	PROCEDURE RegisterAuxillary* (node: Node);
		VAR
			list: List;
	BEGIN
		NEW(list);
		list.node := node;
		list.next := auxillary;
		auxillary := list
	END RegisterAuxillary;

	PROCEDURE SetStochastics* (stochastics: Vector; numChains: INTEGER);
		VAR
			j, num: INTEGER;
	BEGIN
		nodes := stochastics;
		IF nodes # NIL THEN
			num := LEN(nodes);
			IF numChains > 0 THEN
				NEW(values, numChains);
				j := 0; WHILE j < numChains DO NEW(values[j], num); INC(j) END
			END
		END;
	END SetStochastics;

	PROCEDURE StoreValues* (chain: INTEGER);
		VAR
			i, numStoch: INTEGER;
	BEGIN
		IF nodes # NIL THEN
			numStoch := LEN(nodes);
			i := 0; WHILE i < numStoch DO values[chain, i] := nodes[i].value; INC(i) END
		END
	END StoreValues;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		Clear
	END Init;

BEGIN
	Init
END GraphStochastic.


