(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	   *)

MODULE GraphLogical;


	

	IMPORT
		Stores := Stores64,
		GraphNodes;

	CONST
		(*	node properties	*)
		mark* = 2; 	(*	node is marked	*)
		alwaysEvaluate* = 5; 	(*	always recalculate value of node	*)
		dependent* = 6; 	(*	fine control of evaluation of nodes with memory	*)
		linked* = 7; 	(*	node has been linked into its stochastic parents dependant list	*)
		dirty* = 8; 	(*	node value needs to be recalculated	*)
		stochParent* = 9; 	(*	logical node is parent of stochastic node	*)

		undefined* = MAX(INTEGER); 	(*	recursive level of node is undefined	*)

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

	PROCEDURE ^Parents* (node: GraphNodes.Node; all: BOOLEAN): List;

	(*	abstract node methods	*)

	PROCEDURE (node: Node) ClassFunction* (parent: GraphNodes.Node): INTEGER, NEW, ABSTRACT;

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

	(*	writes internal fields of logical node to store	*)
	PROCEDURE (node: Node) ExternalizeNode- (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteInt(node.level);
		node.ExternalizeLogical(wr)
	END ExternalizeNode;

	PROCEDURE (node: Node) InitNode-;
	BEGIN
		node.SetProps({alwaysEvaluate, dirty});
		node.level := undefined;
		node.InitLogical
	END InitNode;

		(*	read internal fields of logical node from store	*)
	PROCEDURE (node: Node) InternalizeNode- (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadInt(node.level);
		node.InternalizeLogical(rd)
	END InternalizeNode;

	PROCEDURE (node: Node) Representative* (): Node, ABSTRACT;

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

	(*	writes a list of pointers to logical nodes to store	*)
	PROCEDURE ExternalizeList* (list: List; mask: SET; VAR wr: Stores.Writer);
		VAR
			node: Node;
	BEGIN
		WHILE list # NIL DO
			node := list.node;
			IF mask * node.props # {} THEN
				GraphNodes.Externalize(node, wr);
			END;
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
				p.SetProps(p.props + {GraphNodes.mark});
				pList := p.Parents(all);
				WHILE pList # NIL DO
					q := pList.node;
					WITH q: Node DO
						q.AddToList(list);
						q.AddParent(cursor);
						q.SetProps(q.props + {GraphNodes.mark})
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
			p.SetProps(p.props - {GraphNodes.mark});
			list1 := list1.next
		END;
		RETURN list
	END Parents;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END GraphLogical.


