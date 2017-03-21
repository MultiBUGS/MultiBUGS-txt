(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	  *)

MODULE GraphStochastic;


	

	IMPORT
		Stores,
		GraphLogical, GraphNodes, GraphRules,
		MathFunc;

	CONST
		(*	node properties	*)
		mark* = GraphNodes.mark;
		data* = GraphNodes.data;

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
		devParent* = 14; 	(*	node is stochastic parent of deviance	*)
		blockMark* = 15; 	(*	node is member of a block	*)
		coParent* = 16; 	(*	node is coparent	*)

		noCDF* = 17; 	(*	cdf can not be expressed in closed form	*)
		noPDF* = 18; 	(*	pdf can not be expressed in closed form	*)
		noMean* = 19; (*	node has mean	*)

		distributed* = 20; (*	distribute sampling over multiple processors	*)

		nR* = 28; 	(*	node is not relevant and should not be used in normal way	*)
		temp* = 29; 	(*	node is tempory	*)
		hint1* = 30;
		hint2* = 31;

		bounds* = {leftNatural, leftImposed, rightNatural, rightImposed};

		undefined* = 0; (*	topological depth of node in graph is undefined/not yet calculated	*)

		(* censoring and truncation of distribution *)
		std* = 0;
		left* = 1;
		right* = 2;
		interval* = 3;

	TYPE

		List* = POINTER TO LIMITED RECORD
			node-: Node;
			next-: List
		END;

		Node* = POINTER TO ABSTRACT RECORD(GraphNodes.Node)
			depth-: INTEGER;
			classConditional-: INTEGER;
			value-: REAL;
			likelihood-: Likelihood;
			dependents: GraphLogical.List;
		END;

		Likelihood* = POINTER TO LIMITED RECORD
			children: Vector;
		END;

		ListLikelihood = POINTER TO LIMITED RECORD(Likelihood)
			list: List
		END;

		Vector* = POINTER TO ARRAY OF Node;

		Factory* = POINTER TO ABSTRACT RECORD (GraphNodes.Factory) END;

		Args* = RECORD(GraphNodes.Args)
			numScalars*, numVectors*: INTEGER;
			scalars*: ARRAY 10 OF GraphNodes.Node;
			leftCen*, rightCen*, leftTrunc*, rightTrunc*: GraphNodes.Node;
			vectors*: ARRAY 10 OF GraphNodes.SubVector;
			leftVectorCen*, rightVectorCen*,
			leftVectorTrunc*, rightVectorTrunc*: GraphNodes.SubVector
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
		stochastics-: Vector;
		numStochastics-: INTEGER;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE AddToList* (node: Node; VAR list: List);
		VAR
			cursor: List;
	BEGIN
		IF ~(mark IN node.props) THEN
			NEW(cursor);
			cursor.node := node;
			cursor.next := list;
			list := cursor
		END
	END AddToList;

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
			WITH p: Node DO
				AddToList(p, list);
				p.SetProps(p.props + {mark})
			ELSE
			END;
			nList := nList.next
		END;
		lList := GraphLogical.Ancestors(node, all);
		WHILE lList # NIL DO
			p := lList.node;
			nList := p.Parents(all);
			WHILE nList # NIL DO
				q := nList.node;
				WITH q: Node DO
					AddToList(q, list);
					q.SetProps(q.props + {mark})
				ELSE
				END;
				nList := nList.next
			END;
			lList := lList.next
		END;
		cursor := list;
		WHILE cursor # NIL DO
			stochastic := cursor.node;
			stochastic.SetProps(stochastic.props - {mark});
			cursor := cursor.next
		END;
		RETURN list
	END Parents;

	PROCEDURE ClassFunction* (node, parent: GraphNodes.Node): INTEGER;
		CONST
			maxLevel = 7;
		VAR
			class: INTEGER;
			p, q: GraphNodes.Node;
			parents: List;
			all: BOOLEAN;
	BEGIN
		ASSERT(parent IS Node, 21);
		WITH node: GraphLogical.Node DO
			all := FALSE;
			IF node.level < maxLevel THEN
				class := node.ClassFunction(parent)
			ELSE
				parents := Parents(node, all);
				p := parent.Representative();
				WHILE (parents # NIL) & (parents.node.Representative() # p) DO
					parents := parents.next
				END;
				IF parents = NIL THEN
					class := GraphRules.const
				ELSE
					class := GraphRules.other
				END
			END
		|node: Node DO
			IF node = parent THEN
				class := GraphRules.ident
			ELSE
				p := node.Representative();
				q := parent.Representative();
				IF p = q THEN
					class := GraphRules.linear
				ELSIF blockMark IN p.props THEN
					class := GraphRules.linear
				ELSE
					class := GraphRules.const
				END
			END
		ELSE
			class := GraphRules.const
		END;
		RETURN class
	END ClassFunction;

	(*	abstract node methods	*)

	PROCEDURE (node: Node) AddLikelihoodTerm- (offspring: Node), NEW, ABSTRACT;

	PROCEDURE (node: Node) AllocateLikelihood*, NEW, ABSTRACT;

	PROCEDURE (node: Node) Bounds* (OUT lower, upper: REAL), NEW, ABSTRACT;

	PROCEDURE (node: Node) CanEvaluate* (): BOOLEAN, NEW, ABSTRACT;

	PROCEDURE (node: Node) ClassifyLikelihood* (parent: Node): INTEGER, NEW, ABSTRACT;

	PROCEDURE (node: Node) ClassifyPrior* (): INTEGER, NEW, ABSTRACT;

	PROCEDURE (node: Node) Deviance* (): REAL, NEW, ABSTRACT;

	PROCEDURE (node: Node) DiffLogConditionalMap* (): REAL, NEW, ABSTRACT;

	PROCEDURE (node: Node) DiffLogLikelihood* (x: Node): REAL, NEW, ABSTRACT;

	PROCEDURE (node: Node) DiffLogPrior* (): REAL, NEW, ABSTRACT;

	PROCEDURE (node: Node) ExternalizeStochastic- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (node: Node) InitStochastic-, NEW, ABSTRACT;

	PROCEDURE (node: Node) InternalizeStochastic- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (node: Node) InvMap* (y: REAL), NEW, ABSTRACT;

	PROCEDURE (node: Node) IsLikelihoodTerm- (): BOOLEAN, NEW, ABSTRACT;

	PROCEDURE (node: Node) Location* (): REAL, NEW, ABSTRACT;

	PROCEDURE (node: Node) LogLikelihood* (): REAL, NEW, ABSTRACT;

	PROCEDURE (node: Node) LogJacobian* (): REAL, NEW, ABSTRACT;

	PROCEDURE (node: Node) LogPrior* (): REAL, NEW, ABSTRACT;

	PROCEDURE (node: Node) Map* (): REAL, NEW, ABSTRACT;

	PROCEDURE (node: Node) Sample* (OUT res: SET), NEW, ABSTRACT;

	PROCEDURE (node: Node) Modify* (): Node, NEW, ABSTRACT;

		(*	concrete node methods	*)

	PROCEDURE (node: Node) AddDependent* (dependent: GraphLogical.Node), NEW;
	BEGIN
		ASSERT(dependent # NIL, 21);
		IF GraphLogical.dependent IN dependent.props THEN
			dependent.AddToList(node.dependents)
		END
	END AddDependent;

	PROCEDURE (node: Node) AddToLikelihood* (VAR likelihood: Likelihood), NEW;
		VAR
			element: List;
			listLike: ListLikelihood;
	BEGIN
		IF ~(mark IN node.props) THEN
			IF likelihood = NIL THEN
				NEW(listLike);
				listLike.children := NIL;
				listLike.list := NIL
			ELSE
				listLike := likelihood(ListLikelihood)
			END;
			NEW(element);
			element.node := node;
			element.next := listLike.list;
			listLike.list := element;
			likelihood := listLike
		END
	END AddToLikelihood;

	PROCEDURE (node: Node) Children* (): Vector, NEW;
		VAR
			children: Vector;
	BEGIN
		children := NIL;
		IF node.likelihood # NIL THEN children := node.likelihood.children END;
		RETURN children
	END Children;

	PROCEDURE (node: Node) DiffLogConditional* (): REAL, NEW;
		VAR
			diff: REAL;
			children: Vector;
			i, num: INTEGER;
	BEGIN
		diff := node.DiffLogPrior();
		children := node.likelihood.children;
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		WHILE i < num DO
			diff := diff + children[i].DiffLogLikelihood(node);
			INC(i)
		END;
		RETURN diff
	END DiffLogConditional;

	PROCEDURE (node: Node) BuildLikelihood*, NEW;
		VAR
			list, pList: List;
			p: Node;
			all: BOOLEAN;
	BEGIN
		ASSERT(~(likelihood IN node.props), 21);
		IF node.IsLikelihoodTerm() THEN
			all := FALSE;
			node.SetProps(node.props + {likelihood});
			list := Parents(node, all);
			WHILE list # NIL DO
				p := list.node;
				IF data IN node.props THEN
					p.AddLikelihoodTerm(node)
				ELSE
					pList := Parents(p, all);
					WHILE (pList # NIL) & (pList.node # node) DO
						pList := pList.next
					END;
					IF pList = NIL THEN
						p.AddLikelihoodTerm(node)
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
		ASSERT({data, nR} * node.props = {}, 21);
		num := 0;
		IF node.likelihood # NIL THEN
			children := node.likelihood.children;
			IF children # NIL THEN num := LEN(children) END
		END;
		i := 0;
		classConditional := node.ClassifyPrior();
		WHILE (i < num) & (classConditional # GraphRules.invalid) DO
			classLikelihood := children[i].ClassifyLikelihood(node);
			classConditional := GraphRules.product[classConditional, classLikelihood];
			INC(i)
		END;
		node.classConditional := classConditional
	END ClassifyConditional;

	PROCEDURE (node: Node) CoParents* (): List, NEW;
		VAR
			children: Vector;
			list, coParents: List;
			p, q: Node;
			i, num: INTEGER;
			all: BOOLEAN;
	BEGIN
		coParents := NIL;
		all := FALSE;
		node.SetProps(node.props + {coParent});
		children := node.likelihood.children;
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		WHILE i < num DO
			p := children[i];
			list := Parents(p, all);
			WHILE list # NIL DO
				q := list.node;
				IF ~(coParent IN q.props) THEN
					q.SetProps(q.props + {coParent});
					AddToList(q, coParents)
				END;
				list := list.next
			END;
			INC(i)
		END;
		list := coParents;
		WHILE list # NIL DO
			q := list.node;
			q.SetProps(q.props - {coParent});
			list := list.next
		END;
		node.SetProps(node.props - {coParent});
		RETURN coParents
	END CoParents;

	(*	writes internal base fields of stochastic node to store	*)
	PROCEDURE (node: Node) Externalize* (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteSet(node.props - {update});
		wr.WriteInt(node.depth);
		wr.WriteInt(node.classConditional);
		IF data IN node.props THEN wr.WriteReal(node.value) END;
		GraphLogical.ExternalizeList(node.dependents, wr);
		node.ExternalizeStochastic(wr);
	END Externalize;

	PROCEDURE (node: Node) InitNode-;
	BEGIN
		node.SetProps({});
		node.value := 0.0;
		node.depth := undefined;
		node.likelihood := NIL;
		node.dependents := NIL;
		node.classConditional := GraphRules.invalid;
		node.InitStochastic
	END InitNode;

	(*	read internal base fields of stochastic node from store	*)
	PROCEDURE (node: Node) Internalize* (VAR rd: Stores.Reader);
		VAR
			props: SET;
	BEGIN
		rd.ReadSet(props);
		node.SetProps(props);
		rd.ReadInt(node.depth);
		rd.ReadInt(node.classConditional);
		IF data IN node.props THEN rd.ReadReal(node.value) END;
		node.dependents := GraphLogical.InternalizeList(rd);
		node.InternalizeStochastic(rd)
	END Internalize;

	PROCEDURE (node: Node) LogConditional* (): REAL, NEW;
		VAR
			log: REAL;
			children: Vector;
			i, num: INTEGER;
	BEGIN
		log := node.LogPrior();
		children := node.likelihood.children;
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		WHILE (i < num) & (log > 0.50 * MathFunc.logOfZero) DO
			log := log + children[i].LogLikelihood();
			INC(i)
		END;
		RETURN log
	END LogConditional;

	PROCEDURE (node: Node) ParentsInitialized* (): BOOLEAN, NEW;
		VAR
			all, initP: BOOLEAN;
			list: List;
			p: Node;
	BEGIN
		initP := TRUE;
		all := TRUE;
		list := Parents(node, all);
		WHILE initP & (list # NIL) DO
			p := list.node;
			initP := initialized IN p.props;
			list := list.next
		END;
		RETURN initP
	END ParentsInitialized;

	PROCEDURE (node: Node) PropagateMsg* (msg: INTEGER), NEW;
		VAR
			p: GraphLogical.Node;
			cursor: GraphLogical.List;
	BEGIN
		cursor := node.dependents;
		WHILE cursor # NIL DO
			p := cursor.node;
			p.HandleMsg(msg);
			cursor := cursor.next
		END
	END PropagateMsg;

	PROCEDURE (node: Node) Representative* (): Node, ABSTRACT;

	PROCEDURE (node: Node) SetChildren* (children: Vector), NEW;
	BEGIN
		IF node.likelihood = NIL THEN NEW(node.likelihood) END;
		node.likelihood.children := children
	END SetChildren;

	PROCEDURE (node: Node) SetLikelihood* (likelihood: Likelihood), NEW;
	BEGIN
		node.likelihood := likelihood
	END SetLikelihood;

	PROCEDURE (node: Node) SetValue* (value: REAL), NEW;
		VAR
			cursor: GraphLogical.List;
			p: GraphLogical.Node;
	BEGIN
		node.value := value;
		cursor := node.dependents;
		WHILE cursor # NIL DO
			p := cursor.node;
			p.SetProps(p.props + {GraphLogical.dirty});
			cursor := cursor.next
		END
	END SetValue;

	PROCEDURE (node: Node) ValDiff* (x: GraphNodes.Node; OUT val, diff: REAL);
	BEGIN
		val := node.value;
		IF node = x THEN diff := 1.0 ELSE diff := 0.0 END
	END ValDiff;

	PROCEDURE (node: Node) Value* (): REAL;
	BEGIN
		RETURN node.value
	END Value;

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

	(*	writes an array of pointers to stochastic nodes to store	*)
	PROCEDURE ExternalizeLikelihood* (likelihood: Likelihood; VAR wr: Stores.Writer);
		VAR
			i, num: INTEGER;
			children: Vector;
	BEGIN
		num := 0;
		IF likelihood # NIL THEN
			children := likelihood.children;
			IF children # NIL THEN num := LEN(children) END
		END;
		i := 0;
		wr.WriteInt(num);
		WHILE i < num DO
			GraphNodes.Externalize(children[i], wr);
			INC(i)
		END;
	END ExternalizeLikelihood;

	(*	reads an array of pointers to stochastic nodes from store	*)
	PROCEDURE InternalizeLikelihood* (VAR rd: Stores.Reader): Likelihood;
		VAR
			children: Vector;
			node: GraphNodes.Node;
			i, num: INTEGER;
			likelihood: Likelihood;
	BEGIN
		likelihood := NIL;
		rd.ReadInt(num); 
		IF num > 0 THEN
			NEW(likelihood);
			NEW(children, num);
			i := 0;
			WHILE i < num DO
				node := GraphNodes.Internalize(rd);
				children[i] := node(Node);
				INC(i)
			END;
			likelihood.children := children
		END;
		RETURN likelihood
	END InternalizeLikelihood;

	PROCEDURE (VAR args: Args) Init*;
	BEGIN
		args.valid := TRUE;
		args.numScalars := 0;
		args.numVectors := 0;
		args.leftCen := NIL;
		args.rightCen := NIL;
		args.leftTrunc := NIL;
		args.rightTrunc := NIL;
		args.leftVectorCen := NIL;
		args.rightVectorCen := NIL;
		args.leftVectorTrunc := NIL;
		args.rightVectorTrunc := NIL;
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
	END Init;

	PROCEDURE AddMark* (vector: Vector; mark: SET);
		VAR
			i, size: INTEGER;
	BEGIN
		size := LEN(vector);
		i := 0;
		WHILE i < size DO
			vector[i].SetProps(vector[i].props + mark);
			INC(i)
		END
	END AddMark;

	PROCEDURE ClearMark* (vector: Vector; mark: SET);
		VAR
			i, size: INTEGER;
	BEGIN
		IF vector # NIL THEN size := LEN(vector) ELSE size := 0 END;
		i := 0;
		WHILE i < size DO
			vector[i].SetProps(vector[i].props - mark);
			INC(i)
		END
	END ClearMark;

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

	PROCEDURE AllocateLikelihood* (likelihood: Likelihood): Likelihood;
		VAR
			list: List;
			like: Likelihood;
	BEGIN
		IF (likelihood # NIL) & (likelihood IS ListLikelihood) THEN
			list := likelihood(ListLikelihood).list;
			NEW(like);
			like.children := ListToVector(list);
			likelihood := like
		END;
		RETURN likelihood
	END AllocateLikelihood;

	PROCEDURE ReadSample* (IN values: ARRAY OF REAL);
		VAR
			i, len: INTEGER;
	BEGIN
		i := 0;
		len := LEN(stochastics);
		WHILE i < len DO
			stochastics[i].SetValue(values[i]);
			INC(i)
		END
	END ReadSample;

	PROCEDURE SetStochastics* (nodes: Vector);
	BEGIN
		stochastics := nodes;
		IF nodes # NIL THEN
			numStochastics := LEN(nodes)
		ELSE
			numStochastics := 0
		END
	END SetStochastics;

	PROCEDURE WriteSample* (OUT values: ARRAY OF REAL);
		VAR
			i, len: INTEGER;
	BEGIN
		i := 0;
		len := LEN(stochastics);
		WHILE i < len DO
			values[i] := stochastics[i].value;
			INC(i)
		END
	END WriteSample;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		stochastics := NIL;
		numStochastics := 0
	END Init;

BEGIN
	Init
END GraphStochastic.


