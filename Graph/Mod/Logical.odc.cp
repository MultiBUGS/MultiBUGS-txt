(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	   *)

MODULE GraphLogical;


	

	IMPORT
		Stores,
		GraphNodes;

	CONST
		(* marks *)
		mark = GraphNodes.mark;

		alwaysEvaluate* = 5; 	(*	always recalculate value of node	*)
		dependent* = 6; 	(*	fine control of evaluation of nodes with memory	*)
		linked* = 7; 	(*	node has been linked into its stochastic parents dependant list	*)
		dirty* = 8; 	(*	node needs to be recalculated	*)
		saved* = 9; 	(*	value is stored in node	*)
		logical* = 21; 	(*	marks node as originaly being a logical node	*)

		undefined* = MAX(INTEGER); 	(*	recursive level of node is undefined	*)

		(* messages *)
		metBegin* = 31;
		metReject* = 30;
		metEnd* = 29;

	TYPE
		Node* = POINTER TO ABSTRACT RECORD (GraphNodes.Node)
			level-: INTEGER;
		END;

		Factory* = POINTER TO ABSTRACT RECORD (GraphNodes.Factory) END;

		Vector* = POINTER TO ARRAY OF Node;

		List* = POINTER TO RECORD;
			node-: Node;
			next-: List
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) ClassFunction* (parent: GraphNodes.Node): INTEGER, NEW, ABSTRACT;

	PROCEDURE (node: Node) HandleMsg* (msg: INTEGER), NEW, EMPTY;

	PROCEDURE (node: Node) InitLogical-, NEW, ABSTRACT;

	PROCEDURE (node: Node) AddToList* (VAR list: List), NEW;
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

	PROCEDURE Clear (list: List);
		VAR
			cursor: List;
			p: Node;
	BEGIN
		cursor := list;
		WHILE cursor # NIL DO
			p := cursor.node;
			p.SetProps(p.props - {mark});
			cursor := cursor.next
		END
	END Clear;

	PROCEDURE Parents* (node: GraphNodes.Node; all: BOOLEAN): List;
		VAR
			list: List;
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
				p.SetProps(p.props + {mark});
				pList := p.Parents(all);
				WHILE pList # NIL DO
					q := pList.node;
					WITH q: Node DO
						q.AddToList(list);
						q.AddParent(cursor);
						q.SetProps(q.props + {mark})
					ELSE
					END;
					pList := pList.next
				END
			ELSE
			END
		END;
		Clear(list);
		RETURN list
	END Parents;

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

	PROCEDURE (node: Node) InitNode-;
	BEGIN
		node.SetProps({alwaysEvaluate, dirty});
		node.level := undefined;
		node.InitLogical
	END InitNode;

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

	(*	writes a list of pointers to logical nodes to store	*)
	PROCEDURE ExternalizeList* (list: List; VAR wr: Stores.Writer);
	BEGIN
		WHILE list # NIL DO
			GraphNodes.Externalize(list.node, wr);
			list := list.next
		END;
		GraphNodes.Externalize(NIL, wr)
	END ExternalizeList;

	(*	reads a list of pointers to logical nodes from store	*)
	PROCEDURE InternalizeList* (VAR rd: Stores.Reader): List;
		VAR
			element, list, logicalList, temp: List;
			node: GraphNodes.Node;
	BEGIN
		list := NIL;
		logicalList := NIL;
		node := GraphNodes.Internalize(rd);
		WHILE node # NIL DO
			NEW(element);
			element.node := node(Node);
			element.next := list;
			list := element;
			node := GraphNodes.Internalize(rd)
		END;
		WHILE list # NIL DO	(* reverse list *)
			temp := list; list := temp.next; temp.next := logicalList; logicalList := temp
		END;
		RETURN logicalList
	END InternalizeList;

	PROCEDURE (node: Node) ExternalizeLogical- (VAR wr: Stores.Writer), NEW, ABSTRACT;

		(*	writes internal base fields of logical node to store	*)
	PROCEDURE (node: Node) Externalize* (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteSet(node.props);
		wr.WriteInt(node.level);
		node.ExternalizeLogical(wr)
	END Externalize;

	PROCEDURE (node: Node) InternalizeLogical- (VAR rd: Stores.Reader), NEW, ABSTRACT;

		(*	read internal base fields of logical node from store	*)
	PROCEDURE (node: Node) Internalize* (VAR rd: Stores.Reader);
		VAR
			props: SET;
	BEGIN
		rd.ReadSet(props);
		node.SetProps(props);
		rd.ReadInt(node.level);
		node.InternalizeLogical(rd)
	END Internalize;

	PROCEDURE (node: Node) Representative* (): Node, ABSTRACT;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END GraphLogical.


