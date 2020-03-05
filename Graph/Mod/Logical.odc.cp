(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	   *)

MODULE GraphLogical;


	

	IMPORT
		Stores := Stores64,
		GraphNodes, GraphRules;

	CONST
		(*	node properties	*)
		mark* = 2; 	(*	node is marked	*)
		diff* = 3; 	(*	differentiation mark	*)
		constDiffs* = 4; 	(*	node is  linear in all its arguments	*)
		linear* = 5; 	(*	node is marked as linear in all its arguments	*)
		prediction* = 6; 	(*	node is only used for prediction	*)
		linked* = 7; 	(*	node has been linked into its stochastic parents dependant list	*)
		descreteParent* = 8; 	(*	node has a descrete parent	*)
		noDiffs* = 9;
		undefined* = MAX(INTEGER); 	(*	recursive level of node is undefined	*)

	TYPE
		Node* = POINTER TO ABSTRACT RECORD (GraphNodes.Node)
			work-: POINTER TO ARRAY OF REAL;
			parents-: GraphNodes.Vector;
			level-: INTEGER;
		END;

		Factory* = POINTER TO ABSTRACT RECORD (GraphNodes.Factory) END;

		Vector* = POINTER TO ARRAY OF Node;

		List* = POINTER TO RECORD;
			node-: Node;
			next-: List
		END;

	VAR
		values: POINTER TO ARRAY OF POINTER TO ARRAY OF REAL;
		nodes-: Vector;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE ^Parents* (node: GraphNodes.Node; all: BOOLEAN): List;

		(*	abstract node methods	*)

	PROCEDURE (node: Node) AllocateDiffs- (numDiffs: INTEGER), NEW, EMPTY;

	PROCEDURE (node: Node) ClassFunction- (parent: GraphNodes.Node): INTEGER, NEW, ABSTRACT;

	PROCEDURE (node: Node) Evaluate-, NEW, ABSTRACT;

	PROCEDURE (node: Node) EvaluateDiffs-, NEW, ABSTRACT;

	PROCEDURE (node: Node) ExternalizeLogical- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (node: Node) InitLogical-, NEW, ABSTRACT;

	PROCEDURE (node: Node) InternalizeLogical- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (node: Node) Link-, NEW, EMPTY;

		(*	concrete node methods	*)

	PROCEDURE (node: Node) AddToList* (VAR list: List), NEW;
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

	PROCEDURE (node: Node) CalculateLevel*, NEW;
		VAR
			level: INTEGER;
			p: GraphNodes.Node;
			list: GraphNodes.List;
			all: BOOLEAN;
	BEGIN
		level := 0;
		all := TRUE;
		list := node.Parents(all);
		WHILE (list # NIL) & (level # undefined) DO
			p := list.node;
			WITH p: Node DO
				level := MAX(level, p.level)
			ELSE
			END;
			list := list.next
		END;
		IF level # undefined THEN
			node.level := level + 1
		END
	END CalculateLevel;

	PROCEDURE (node: Node) Diff* (x: GraphNodes.Node): REAL;
		VAR
			i, num: INTEGER;
			diffWRT: GraphNodes.Vector;
	BEGIN
		diffWRT := node.parents;
		num := LEN(diffWRT);
		i := 0; WHILE (i < num) & (diffWRT[i] # x) DO INC(i) END;
		IF i < num THEN RETURN node.work[i] ELSE RETURN 0.0 END;
	END Diff;


	(*	writes internal fields of logical node to store	*)
	PROCEDURE (node: Node) ExternalizeNode- (VAR wr: Stores.Writer);
		VAR
			i, num: INTEGER;
	BEGIN
		wr.WriteInt(node.level);
		IF node.parents # NIL THEN
			num := LEN(node.parents);
			wr.WriteInt(num);
			i := 0; WHILE i < num DO GraphNodes.Externalize(node.parents[i], wr); INC(i) END;
			IF constDiffs IN node.props THEN
				i := 0; WHILE i < num DO wr.WriteReal(node.work[i]); INC(i) END
			END
		ELSE
			wr.WriteInt(0)
		END;
		node.ExternalizeLogical(wr)
	END ExternalizeNode;

	PROCEDURE (node: Node) InitNode-;
	BEGIN
		node.props := {prediction};
		node.level := undefined;
		node.work := NIL;
		node.parents := NIL;
		node.InitLogical
	END InitNode;

	(*	read internal fields of logical node from store	*)
	PROCEDURE (node: Node) InternalizeNode- (VAR rd: Stores.Reader);
		VAR
			i, num: INTEGER;
	BEGIN
		rd.ReadInt(node.level);
		rd.ReadInt(num);
		IF num # 0 THEN
			NEW(node.parents, num);
			i := 0; WHILE i < num DO node.parents[i] := GraphNodes.Internalize(rd); INC(i) END;
			NEW(node.work, num);
			IF constDiffs IN node.props THEN
				i := 0; WHILE i < num DO rd.ReadReal(node.work[i]); INC(i) END
			END;
			node.AllocateDiffs(num)
		ELSE
			node.parents := NIL;
			node.work := NIL
		END;
		node.InternalizeLogical(rd)
	END InternalizeNode;

	PROCEDURE (node: Node) Representative* (): Node, ABSTRACT;

	PROCEDURE (node: Node) SetParents* (parents: GraphNodes.Vector), NEW;
		VAR
			num: INTEGER;
	BEGIN
		node.parents := parents;
		IF parents # NIL THEN
			num := LEN(parents);
			IF (node.work = NIL) OR (num > LEN(node.work)) THEN
				NEW(node.work, num);
				node.AllocateDiffs(num);
			END
		END
	END SetParents;

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
			IF ch = "L" THEN DEC(numParam) END
		END;
		RETURN numParam
	END NumParam;

	PROCEDURE ListToVector* (list: List): Vector;
		VAR
			i, len, level: INTEGER;
			cursor: List;
			vector: Vector;
			node: Node;
	BEGIN
		IF list = NIL THEN
			vector := NIL
		ELSE
			len := 0;
			level := 0;
			cursor := list;
			WHILE cursor # NIL DO
				INC(len);
				node := cursor.node;
				level := MAX(level, node.level);
				cursor := cursor.next
			END;
			NEW(vector, len);
			(*	put list into vector in order by level	*)
			i := len;
			REPEAT
				cursor := list;
				WHILE cursor # NIL DO
					node := cursor.node;
					IF node.level = level THEN DEC(i); vector[i] := cursor.node END;
					cursor := cursor.next
				END;
				DEC(level);
			UNTIL level = 0
		END;
		RETURN vector
	END ListToVector;

	PROCEDURE Ancestors* (nodes: GraphNodes.Vector): Vector;
		VAR
			i, len: INTEGER;
			cursor, list, list1: List;
			logical: Node;
			node: GraphNodes.Node;
			block: Vector;
		CONST
			all = TRUE;
	BEGIN
		len := LEN(nodes);
		list1 := NIL;
		i := 0;
		WHILE i < len DO
			node := nodes[i];
			IF (node # NIL) & (node IS Node) THEN list := Parents(node, all) ELSE list := NIL END;
			cursor := list;
			WHILE cursor # NIL DO
				logical := cursor.node;
				IF ~(mark IN logical.props) THEN
					logical.AddToList(list1); INCL(logical.props, mark)
				END;
				cursor := cursor.next
			END;
			IF (node # NIL) & (node IS Node) THEN
				logical := node(Node);
				IF ~(mark IN logical.props) THEN logical.AddToList(list1); INCL(logical.props, mark) END
			END;
			INC(i)
		END;
		list := list1;
		WHILE list1 # NIL DO logical := list1.node; EXCL(logical.props, mark); list1 := list1.next END;
		block := ListToVector(list);
		RETURN block
	END Ancestors;

	PROCEDURE Classify* (dependents: Vector);
		VAR
			i, j, num, numDep: INTEGER;
			node: Node;
	BEGIN
		IF dependents # NIL THEN
			numDep := LEN(dependents);
			i := 0;
			WHILE i < numDep DO
				node := dependents[i];
				IF node.parents # NIL THEN
					num := LEN(node.parents);
					j := 0;
					WHILE j < num DO
						node.work[j] := node.ClassFunction(node.parents[j]);
						INC(j)
					END
				END;
				INC(i)
			END
		END
	END Classify;

	PROCEDURE ClassifyAll*;
	BEGIN
		Classify(nodes)
	END ClassifyAll;

	PROCEDURE Clear*;
	BEGIN
		nodes := NIL;
		values := NIL;
	END Clear;

	PROCEDURE ClearMarks* (vector: Vector; mark: SET);
		VAR
			i, size: INTEGER;
	BEGIN
		IF vector # NIL THEN
			size := LEN(vector);
			i := 0; WHILE i < size DO vector[i].props := vector[i].props - mark; INC(i) END
		END
	END ClearMarks;

	(*	evaluates a set of dependent nodes	*)
	PROCEDURE Evaluate* (dependents: Vector);
		VAR
			p: Node;
			i, num: INTEGER;
	BEGIN
		IF dependents # NIL THEN
			num := LEN(dependents);
			i := 0;
			WHILE i < num DO
				p := dependents[i];
				p.Evaluate;
				INC(i)
			END
		END
	END Evaluate;

	(*	evaluates all dependent nodes	*)
	PROCEDURE EvaluateAll*;
	BEGIN
		Evaluate(nodes)
	END EvaluateAll;

	(*	evaluates the derivatives of a set of dependent nodes note that procedure leaves marks set	*)
	PROCEDURE EvaluateDiffs* (dependents: Vector);
		VAR
			p: Node;
			i, num: INTEGER;
	BEGIN
		IF dependents # NIL THEN
			num := LEN(dependents);
			i := 0;
			WHILE i < num DO
				p := dependents[i];
				IF ({constDiffs, prediction, noDiffs} * p.props = {}) & (p.parents # NIL) THEN
					p.EvaluateDiffs; INCL(p.props, diff);
				ELSE
					p.Evaluate
				END;
				INC(i)
			END
		END
	END EvaluateDiffs;

	(*	evaluates the derivatives of all dependent nodes	*)
	PROCEDURE EvaluateAllDiffs*;
	BEGIN
		IF nodes # NIL THEN
			EvaluateDiffs(nodes);
			ClearMarks(nodes, {diff})
		END
	END EvaluateAllDiffs;

	PROCEDURE ExternalizeValues* (VAR wr: Stores.Writer);
		VAR
			chain, i, numChains, numDep: INTEGER;
	BEGIN
		IF values # NIL THEN
			numChains := LEN(values);
			numDep := LEN(values[0]);
			chain := 0;
			WHILE chain < numChains DO
				i := 0; WHILE i < numDep DO wr.WriteReal(values[chain, i]); INC(i) END;
				INC(chain)
			END
		END
	END ExternalizeValues;

	(*	writes a vector of pointers to logical nodes to store	*)
	PROCEDURE ExternalizeVector* (v: Vector; mask: SET; VAR wr: Stores.Writer);
		VAR
			node: Node;
			i, num, size: INTEGER;
	BEGIN
		IF v # NIL THEN
			num := LEN(v);
			i := 0;
			size := 0;
			WHILE i < num DO
				node := v[i];
				IF mask * node.props # {} THEN INC(size) END;
				INC(i)
			END;
			wr.WriteInt(size);
			i := 0;
			WHILE i < num DO
				node := v[i];
				IF mask * node.props # {} THEN GraphNodes.Externalize(node, wr) END;
				INC(i)
			END
		ELSE
			wr.WriteInt(0)
		END
	END ExternalizeVector;

	PROCEDURE InternalizeValues* (VAR rd: Stores.Reader);
		VAR
			chain, i, numChains, numDep: INTEGER;
	BEGIN
		IF values # NIL THEN
			numChains := LEN(values);
			numDep := LEN(values[0]);
			chain := 0;
			WHILE chain < numChains DO
				i := 0; WHILE i < numDep DO rd.ReadReal(values[chain, i]); INC(i) END;
				INC(chain)
			END
		END
	END InternalizeValues;

	(*	reads a vector of pointers to logical nodes from store	*)
	PROCEDURE InternalizeVector* (VAR rd: Stores.Reader): Vector;
		VAR
			node: GraphNodes.Node;
			v: Vector;
			i, size: INTEGER;
	BEGIN
		v := NIL;
		rd.ReadInt(size);
		IF size # 0 THEN
			NEW(v, size);
			i := 0;
			WHILE i < size DO
				node := GraphNodes.Internalize(rd);
				v[i] := node(Node);
				INC(i)
			END
		END;
		RETURN v
	END InternalizeVector;

	(*	link a set of dependent nodes	*)
	PROCEDURE Link* (dependents: Vector);
		VAR
			p: Node;
			i, num: INTEGER;
	BEGIN
		IF dependents # NIL THEN
			num := LEN(dependents);
			i := 0; WHILE i < num DO p := dependents[i]; p.Link; INC(i) END
		END
	END Link;

	PROCEDURE LinkAll*;
	BEGIN
		Link(nodes)
	END LinkAll;

	PROCEDURE LoadValues* (chain: INTEGER);
		VAR
			i, num: INTEGER;
	BEGIN
		IF nodes # NIL THEN
			num := LEN(nodes);
			i := 0; WHILE i < num DO nodes[i].value := values[chain, i]; INC(i) END
		END
	END LoadValues;

	PROCEDURE Parents* (node: GraphNodes.Node; all: BOOLEAN): List;
		VAR
			list, list1: List;
			cursor, parents, pList: GraphNodes.List;
			p, q: GraphNodes.Node;
	BEGIN
		parents := node.Parents(all);
		cursor := parents;
		list := NIL;
		WHILE cursor # NIL DO
			p := cursor.node;
			cursor := cursor.next;
			WITH p: Node DO
				p.AddToList(list);
				INCL(p.props, GraphNodes.mark);
				pList := p.Parents(all);
				WHILE pList # NIL DO
					q := pList.node;
					WITH q: Node DO
						q.AddToList(list); q.AddParent(cursor); INCL(q.props, GraphNodes.mark)
					ELSE
					END;
					pList := pList.next
				END
			ELSE
			END
		END;
		list1 := list;
		WHILE list1 # NIL DO p := list1.node; EXCL(p.props, GraphNodes.mark); list1 := list1.next END;
		RETURN list
	END Parents;

	PROCEDURE SetLogicals* (logicals: Vector; numChains: INTEGER);
		VAR
			i, num: INTEGER;
	BEGIN
		values := NIL;
		nodes := logicals;
		IF nodes # NIL THEN
			num := LEN(nodes);
			IF numChains > 0 THEN
				NEW(values, numChains);
				i := 0; WHILE i < numChains DO NEW(values[i], num); INC(i) END
			END
		END
	END SetLogicals;

	PROCEDURE StoreValues* (chain: INTEGER);
		VAR
			i, num: INTEGER;
	BEGIN
		IF nodes # NIL THEN
			num := LEN(nodes);
			i := 0; WHILE i < num DO values[chain, i] := nodes[i].value; INC(i) END
		END
	END StoreValues;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Clear;
		Maintainer
	END Init;

BEGIN
	Init
END GraphLogical.


