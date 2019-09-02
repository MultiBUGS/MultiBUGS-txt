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
		linear* = 4; 	(*	node is linear in all its arguments	*)
		prediction* = 5; 	(*	node is only used for prediction	*)
		linked* = 6; 	(*	node has been linked into its stochastic parents dependant list	*)
		(*stochParent* = 9; *)	(*	logical node is parent of stochastic node	*)

		undefined* = MAX(INTEGER); 	(*	recursive level of node is undefined	*)

	TYPE
		Node* = POINTER TO ABSTRACT RECORD (GraphNodes.Node)
			diffs-: POINTER TO ARRAY OF REAL;
			diffWRT-: GraphNodes.Vector;
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

	PROCEDURE (node: Node) ClassFunction* (parent: GraphNodes.Node): INTEGER, NEW, ABSTRACT;

	PROCEDURE (node: Node) Evaluate-, NEW, ABSTRACT;

	PROCEDURE (node: Node) EvaluateDiffs-, NEW, ABSTRACT;

	PROCEDURE (node: Node) ExternalizeLogical- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (node: Node) InitLogical-, NEW, ABSTRACT;

	PROCEDURE (node: Node) InternalizeLogical- (VAR rd: Stores.Reader), NEW, ABSTRACT;

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

	PROCEDURE (node: Node) ClearLevel*, NEW;
		VAR
			list: List;
			p: Node;
			all: BOOLEAN;
	BEGIN
		IF node.level # undefined THEN
			all := TRUE;
			node.level := undefined;
			list := Parents(node, all);
			WHILE list # NIL DO
				p := list.node;
				p.level := undefined;
				list := list.next
			END
		END
	END ClearLevel;

	PROCEDURE (node: Node) Diff* (x: GraphNodes.Node): REAL;
		VAR
			i, num: INTEGER;
			diffWRT: GraphNodes.Vector;
	BEGIN
		diffWRT := node.diffWRT;
		num := LEN(diffWRT);
		i := 0; WHILE (i < num) & (diffWRT[i] # x) DO INC(i) END;
		IF i < num THEN RETURN node.diffs[i] ELSE RETURN 0.0 END;
	END Diff;

	(*	writes internal fields of logical node to store	*)
	PROCEDURE (node: Node) ExternalizeNode- (VAR wr: Stores.Writer);
		VAR
			i, num: INTEGER;
	BEGIN
		wr.WriteInt(node.level);
		IF node.diffWRT # NIL THEN
			num := LEN(node.diffWRT);
			wr.WriteInt(num);
			i := 0; WHILE i < num DO GraphNodes.Externalize(node.diffWRT[i], wr); INC(i) END;
			IF linear IN node.props THEN
				i := 0; WHILE i < num DO wr.WriteReal(node.diffs[i]); INC(i) END
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
		node.diffs := NIL;
		node.diffWRT := NIL;
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
			NEW(node.diffWRT, num);
			i := 0; WHILE i < num DO node.diffWRT[i] := GraphNodes.Internalize(rd); INC(i) END;
			NEW(node.diffs, num);
			IF linear IN node.props THEN
				i := 0; WHILE i < num DO rd.ReadReal(node.diffs[i]); INC(i) END
			END;
			node.AllocateDiffs(num)
		ELSE
			node.diffWRT := NIL;
			node.diffs := NIL
		END;
		node.InternalizeLogical(rd)
	END InternalizeNode;

	PROCEDURE (node: Node) Representative* (): Node, ABSTRACT;

	PROCEDURE (node: Node) SetDiffWRT* (diffWRT: GraphNodes.Vector), NEW;
		VAR
			numDiffs: INTEGER;
	BEGIN
		node.diffWRT := diffWRT;
		IF diffWRT # NIL THEN
			numDiffs := LEN(diffWRT);
			IF (node.diffs = NIL) OR (numDiffs > LEN(node.diffs)) THEN
				NEW(node.diffs, numDiffs);
				node.AllocateDiffs(numDiffs);
			END
		END
	END SetDiffWRT;

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

	PROCEDURE Ancestors* (nodes: GraphNodes.Vector): List;
		VAR
			i, len: INTEGER;
			list, list1: List;
			logical: Node;
			node: GraphNodes.Node;
		CONST
			all = TRUE;
	BEGIN
		len := LEN(nodes);
		list1 := NIL;
		i := 0;
		WHILE i < len DO
			node := nodes[i];
			IF node IS Node THEN list := Parents(nodes[i], all) ELSE list := NIL END;
			WHILE list # NIL DO
				logical := list.node;
				IF ~(mark IN logical.props) THEN
					logical.AddToList(list1); INCL(logical.props, mark)
				END;
				list := list.next
			END;
			IF node IS Node THEN
				logical := node(Node);
				IF ~(mark IN logical.props) THEN logical.AddToList(list1); INCL(logical.props, mark) END
			END;
			INC(i)
		END;
		list := list1;
		WHILE list1 # NIL DO logical := list1.node; EXCL(logical.props, mark); list1 := list1.next END;
		RETURN list
	END Ancestors;

	PROCEDURE Clear*;
	BEGIN
		nodes := NIL;
		values := NIL;
	END Clear;

	(*	clears set marks	*)
	PROCEDURE ClearDiffs* (dependents: Vector);
		VAR
			p: Node;
			i, num: INTEGER;
	BEGIN
		IF dependents # NIL THEN
			num := LEN(dependents);
			i := 0;
			WHILE i < num DO
				p := dependents[i];
				EXCL(p.props, GraphNodes.mark);
				INC(i)
			END
		END
	END ClearDiffs;

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

	(*	evaluates all sdependent nodes	*)
	PROCEDURE EvaluateAll*;
		VAR
			p: Node;
			i, num: INTEGER;
	BEGIN
		IF nodes # NIL THEN
			num := LEN(nodes);
			i := 0;
			WHILE i < num DO
				p := nodes[i];
				p.Evaluate;
				INC(i)
			END
		END
	END EvaluateAll;

	(*	evaluates all dependent nodes	*)
	PROCEDURE EvaluateAllDiffs*;
		VAR
			p: Node;
			i, num: INTEGER;
	BEGIN
		IF nodes # NIL THEN
			num := LEN(nodes);
			i := 0;
			WHILE i < num DO
				p := nodes[i];
				IF ({linear, prediction} * p.props = {}) & (p.diffWRT # NIL) THEN
					p.EvaluateDiffs
				ELSE
					p.Evaluate
				END;
				INCL(p.props, GraphNodes.mark);
				INC(i)
			END;
			i := 0; WHILE i < num DO p := nodes[i]; EXCL(p.props, GraphNodes.mark); INC(i) END
		END
	END EvaluateAllDiffs;

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
				IF ({linear, prediction} * p.props = {}) & (p.diffWRT # NIL) THEN
					p.EvaluateDiffs;
				ELSE
					p.Evaluate
				END;
				INCL(p.props, GraphNodes.mark);
				INC(i)
			END
		END
	END EvaluateDiffs;

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
						q.AddToList(list);
						q.AddParent(cursor);
						INCL(q.props, GraphNodes.mark)
					ELSE
					END;
					pList := pList.next
				END
			ELSE
			END
		END;
		list1 := list;
		WHILE list1 # NIL DO
			p := list1.node;
			EXCL(p.props, GraphNodes.mark);
			list1 := list1.next
		END;
		RETURN list
	END Parents;

	PROCEDURE SetLogicals* (logicals: Vector; numChains: INTEGER);
		VAR
			i, num: INTEGER;
	BEGIN
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


