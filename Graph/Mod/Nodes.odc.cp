(*	 	

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	  *)

MODULE GraphNodes;

	(*	building blocks for graphical models	*)

	

	IMPORT 
		Meta, Stores, Strings;

	CONST
		(*	node properties	*)
		data* = 0; 	(*	the node is data (has a fixed value)	*)
		mark* = 1; 	(*	the node has been marked	*)

		(*	asscociate error with this argument of function / density	*)
		arg1* = 1; 	(*	1st argument	*)
		arg2* = 2; 	(*	2nd argument	*)
		arg3* = 3; 	(*	3rd argument	*)
		arg4* = 4; 	(*	4th argument	*)
		arg5* = 5; 	(*	5th argument	*)
		arg6* = 6; 	(*	6th argument	*)
		arg7* = 7; 	(*	7th argument	*)
		arg8* = 8; 	(*	8th argument	*)
		arg9* = 9; 	(*	9th argument*)
		arg10* = 10; 	(*	10th argument	*)
		lhs* = 11; (*	left hand argument	*)

		(*	type of error	*)
		invalidCensoring* = 12; 	(*	censored node can not be a prior	*)
		invalidTruncation* = 13; 	(*	multivariate truncated node can not be used as likelihood	*)
		notStochastic* = 14; (*	node / argument is not stochastic	*)
		notMultivariate* = 15; 	(*	node / argument is not multivariate	*)
		invalidParameters* = 16; 	(*	parameter is invalid	*)
		invalidValue* = 17; 	(*	parameter has invalid value	*)
		notSymmetric* = 18; 	(*	matrix is not symmetric	*)
		notData* = 19; 	(*	parameters is not data	*)
		tooManyIts* = 20; 	(*	too many iterations	*)
		leftBound* = 21; 	(*	left bounds breached	*)
		rightBound* = 22; 	(*	right bound breached	*)
		proportion* = 23; 	(*	parameter is not a proportion	*)
		invalidProportions* = 24; 	(*	parameter is not a proportion	*)
		posative* = 25; 	(*	parameters is not posative	*)
		invalidPosative* = 26; 	(*	parameters is not posative	*)
		integer* = 27; 	(*	parameter is not integer	*)
		invalidInteger* = 28; (*	parameter is not integer	*)
		mixedData* = 29; (*	mixture of data and unobserved	*)
		length* = 30; 	(*	vector of wrong length	*)
		nil* = 31; 	(*	vector contains undefined components	*)

		maxNumTypes = 500;

	TYPE
		(*	abstract base type from which all nodes in graphical model are derived	*)
		Node* = POINTER TO ABSTRACT RECORD
			props-: SET; 	(*	properties of node	*)
			label: INTEGER
		END;

		(*	list of nodes	*)
		List* = POINTER TO RECORD
			node-: Node; (*	node in list	*)
			next-: List	(*	next element in list	*)
		END;

		(*	vector of nodes	*)
		Vector* = POINTER TO ARRAY OF Node;

		(*	set of equally spaced elements from a vector	*)
		SubVector* = POINTER TO LIMITED RECORD
			start*: INTEGER; 	(*	first element in sub vector	*)
			nElem*: INTEGER; 	(*	number of elements in sub vector	*)
			step*: INTEGER; 	(*	spacing of elements in sub vector	*)
			components*: Vector (*	vector containing sub vectors elements	*)
		END;

		(*	arguments to be passed to node	*)
		Args* = ABSTRACT RECORD
			valid*: BOOLEAN (*	are arguments valid	*)
		END;

		(*	abstract factory class for creating nodes in graphical model	*)
		Factory* = POINTER TO ABSTRACT RECORD END;

		String = ARRAY 128 OF CHAR;

	VAR
		maxDepth-, maxStochDepth-: INTEGER;
		version-: INTEGER; (*	version number	*)
		maintainer-: ARRAY 40 OF CHAR; (*	person maintaining module	*)
		timeStamp-: LONGINT;
		fact: Factory; 	(*	factory object for creating nodes in graphical model	*)
		nodeLabel, typeLabel: INTEGER;
		nodes: Vector;
		nodeList: List;
		installProc: ARRAY maxNumTypes OF String;
		factories: ARRAY maxNumTypes OF Factory;
		startOfGraph: INTEGER;

	PROCEDURE SetTimeStamp* (t: LONGINT);
	BEGIN
		timeStamp := t
	END SetTimeStamp;

	PROCEDURE InstallFactory* (IN install: ARRAY OF CHAR): Factory;
		VAR
			ok: BOOLEAN;
			i, pos, pos1: INTEGER;
			item: Meta.Item;
			string, install1: ARRAY 128 OF CHAR;
			stringCmd: RECORD(Meta.Value) Do: PROCEDURE (s: ARRAY OF CHAR) END;
			f: Factory;
	BEGIN
		fact := NIL;
		Strings.Find(install, "(", 0, pos);
		Strings.Find(install, ")", 0, pos1);
		IF pos =  - 1 THEN
			Meta.LookupPath(install, item);
			ASSERT(item.obj = Meta.procObj, 20);
			item.Call(ok)
		ELSE
			Strings.Extract(install, pos + 1, pos1 - pos - 1, string);
			i := 0;
			WHILE i < pos DO
				install1[i] := install[i];
				INC(i)
			END;
			install1[i] := 0X;
			Meta.LookupPath(install1, item);
			ASSERT(item.obj = Meta.procObj, 21);
			item.GetVal(stringCmd, ok);
			stringCmd.Do(string)
		END;
		f := fact;
		fact := NIL;
		RETURN f
	END InstallFactory;

	(*	checks that internal state of node is consistant	*)
	PROCEDURE (node: Node) Check* (): SET, NEW, ABSTRACT;
	
	(*	externalize interrnal fields of node	*)
	PROCEDURE (node: Node) ExternalizeNode- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (node: Node) Externalize* (VAR wr: Stores.Writer), NEW;
	BEGIN
		wr.WriteSet(node.props);
		node.ExternalizeNode(wr);
	END Externalize;
	
	(*	initializes the internal fields of node to non sensible / possible values	*)
	PROCEDURE (node: Node) InitNode-, NEW, ABSTRACT;

	(*	gets name of the Install procedure for this type of node	*)
	PROCEDURE (node: Node) Install* (OUT install: ARRAY OF CHAR), NEW, ABSTRACT;
	
	(*	internalize internal fields of node	*)
	PROCEDURE (node: Node) InternalizeNode- (VAR rd: Stores.Reader), NEW, ABSTRACT;
	
	PROCEDURE (node: Node) Internalize* (VAR rd: Stores.Reader), NEW;
	BEGIN
		rd.ReadSet(node.props);
		node.InternalizeNode(rd)
	END Internalize;

	(*	returns parents of node, 'all' controls if parents not relevant to likelihood are returned	*)
	PROCEDURE (node: Node) Parents* (all: BOOLEAN): List, NEW, ABSTRACT;

	(*	returns a node that is representetive of node	*)
	PROCEDURE (node: Node) Representative* (): Node, NEW, ABSTRACT;

	(*	sets internal fields of node	*)
	PROCEDURE (node: Node) Set* (IN args: Args; OUT res: SET), NEW, ABSTRACT;

	(*	size of node	*)
	PROCEDURE (node: Node) Size* (): INTEGER, NEW, ABSTRACT;

	(*	value of node	*)
	PROCEDURE (node: Node) Value* (): REAL, NEW, ABSTRACT;

	(*	value and value of differential of node wrt x	*)
	PROCEDURE (node: Node) ValDiff* (x: Node; OUT val, diff: REAL), NEW, ABSTRACT;

	(*	add parent node to list, if node is marked then it is not added to list	*)
	PROCEDURE (node: Node) AddParent* (VAR list: List), NEW;
		VAR
			cursor: List;
	BEGIN
		node := node.Representative();
		IF {data, mark} * node.props = {} THEN
			NEW(cursor);
			cursor.node := node;
			cursor.next := list;
			list := cursor;
			node.props := node.props + {mark}
		END
	END AddParent;

	(*	initialize node	*)
	PROCEDURE (node: Node) Init*, NEW;
	BEGIN
		node.props := {};
		node.label := 0;
		node.InitNode
	END Init;

	(*	set properties of node	*)
	PROCEDURE (node: Node) SetProps* (props: SET), NEW;
	BEGIN
		node.props := props
	END SetProps;

	(*	initialize args data type	*)
	PROCEDURE (VAR args: Args) Init*, NEW, ABSTRACT;

	(*	create a new node in graphical model	*)
	PROCEDURE (f: Factory) New* (): Node, NEW, ABSTRACT;

	(*	number of parameters that node created by factory has	*)
	PROCEDURE (f: Factory) NumParam* (): INTEGER, NEW, ABSTRACT;

	(*	signature of parameters of node created by factory	*)
	PROCEDURE (f: Factory) Signature* (OUT signature: ARRAY OF CHAR), NEW, ABSTRACT;

	(*	clears mark from nodes in list	*)
	PROCEDURE ClearList* (list: List);
		VAR
			cursor: List;
			p: Node;
	BEGIN
		cursor := list;
		WHILE cursor # NIL DO
			p := cursor.node;
			p.props := p.props - {mark};
			cursor := cursor.next
		END
	END ClearList;

	(*	create new sub vector	*)
	PROCEDURE NewVector* (): SubVector;
		VAR
			vector: SubVector;
	BEGIN
		NEW(vector);
		vector.start :=  - 1;
		vector.nElem :=  - 1;
		vector.step :=  - 1;
		vector.components := NIL;
		RETURN vector
	END NewVector;

	(*	Writes out a pointer to a node to store, if it is the first time this pointer has been
	written type information is also written so that the object can be recreated when it is
	read back from store. If it is the first time an object of a given type has been seen full type
	information is writen otherwise and index into a type table is writen. If 'deep' is true then
	the internal fields of the node are also written.	*)

	PROCEDURE Externalize0 (node: Node; deep: BOOLEAN; VAR wr: Stores.Writer);
		VAR
			install: String;
			i: INTEGER;
			element: List;
	BEGIN
		IF node = NIL THEN
			wr.WriteInt(0); 
		ELSIF node.label > 0 THEN
			wr.WriteInt(node.label); 
		ELSE
			(*	first time seen node	*)
			NEW(element);
			element.node := node;
			element.next := nodeList;
			nodeList := element;
			INC(nodeLabel);
			node.label := nodeLabel;
			wr.WriteInt( - nodeLabel);
			node.Install(install);
			i := 0;
			WHILE (i < typeLabel) & (installProc[i] # install) DO INC(i) END;
			IF i < typeLabel THEN
				wr.WriteInt(i);
			ELSE
				installProc[typeLabel] := install;
				wr.WriteInt(typeLabel);
				INC(typeLabel)
			END;
			IF deep THEN node.Externalize(wr) END
		END
	END Externalize0;

	(*	externalizes a pointer to node in graphical model	*)
	PROCEDURE ExternalizePointer* (node: Node; VAR wr: Stores.Writer);
		CONST
			deep = FALSE;
	BEGIN
		Externalize0(node, deep, wr)
	END ExternalizePointer;

	(*	externalizes a pointer to node in graphical model plus its data	*)
	PROCEDURE Externalize* (node: Node; VAR wr: Stores.Writer);
		CONST
			deep = TRUE;
	BEGIN
		Externalize0(node, deep, wr)
	END Externalize;

	(*	Reads in a pointer to a node from store. If it is the first time the pointer has been read a new
	object of the correct type is created and placed in the global array 'nodes'. Otherwise the pointer
	is looked in the the array of 'nodes'. If 'deep' is true the internal field of the node are also read	*)
	PROCEDURE Internalize0 (deep: BOOLEAN; VAR rd: Stores.Reader): Node;
		VAR
			node: Node;
			label, typeLabel: INTEGER;
	BEGIN
		rd.ReadInt(label);
		IF label = 0 THEN
			node := NIL
		ELSIF label > 0 THEN
			ASSERT(label < LEN(nodes), 100);
			node := nodes[label]
		ELSE
			label :=  - label;
			nodeLabel := MAX(nodeLabel, label);
			rd.ReadInt(typeLabel);
			IF factories[typeLabel] # NIL THEN
				node := factories[typeLabel].New();
				node.label := 0
			ELSE
				node := NIL
			END;
			nodes[label] := node;
			node.Init;
			IF deep THEN node.Internalize(rd) END
		END;
		RETURN node
	END Internalize0;

	(*	internalizes pointer to node in graphical model	*)
	PROCEDURE InternalizePointer* (VAR rd: Stores.Reader): Node;
		CONST
			deep = FALSE;
		VAR
			node: Node;
	BEGIN
		node := Internalize0(deep, rd);
		RETURN node
	END InternalizePointer;

	(*	internalizes pointer to node in graphical model plus its data	*)
	PROCEDURE Internalize* (VAR rd: Stores.Reader): Node;
		CONST
			deep = TRUE;
		VAR
			node: Node;
	BEGIN
		node := Internalize0(deep, rd);
		RETURN node
	END Internalize;

	(*	internalize a block of pointers to nodes	*)
	PROCEDURE InternalizePointers* (num: INTEGER; VAR rd: Stores.Reader);
		VAR
			i: INTEGER;
			p: Node;
	BEGIN
		(*	the p get stored in a global array in GraphNodes	*)
		i := 0;
		WHILE i < num DO
			p := InternalizePointer(rd);
			INC(i)
		END;
	END InternalizePointers;

	(*	externalizes a sub-vector	*)
	PROCEDURE ExternalizeSubvector* (v: SubVector; VAR wr: Stores.Writer);
		VAR
			i, start, nElem, step: INTEGER;
			components: Vector;
	BEGIN
		start := v.start; nElem := v.nElem; step := v.step;
		components := v.components;
		wr.WriteInt(nElem);
		i := 0;
		WHILE i < nElem DO
			Externalize(components[start + i * step], wr); INC(i)
		END
	END ExternalizeSubvector;

	(*	internalizes a sub-vector	*)
	PROCEDURE InternalizeSubvector* (OUT v: SubVector; VAR rd: Stores.Reader);
		VAR
			i, nElem: INTEGER;
			components: Vector;
	BEGIN
		NEW(v);
		rd.ReadInt(nElem);
		v.start := 0; v.nElem := nElem; v.step := 1;
		NEW(components, nElem);
		v.components := components;
		i := 0;
		WHILE i < nElem DO
			components[i] := Internalize(rd); INC(i)
		END
	END InternalizeSubvector;

	(*	writes a list of pointers to nodes to store	*)
	PROCEDURE ExternalizeList* (list: List; VAR wr: Stores.Writer);
	BEGIN
		WHILE list # NIL DO
			Externalize(list.node, wr);
			list := list.next
		END;
		Externalize(NIL, wr)
	END ExternalizeList;

	(*	reads a list of pointers to nodes from store	*)
	PROCEDURE InternalizeList* (VAR rd: Stores.Reader): List;
		VAR
			element, list, temp: List;
			node: Node;
	BEGIN
		list := NIL;
		nodeList := NIL;
		node := Internalize(rd);
		WHILE node # NIL DO
			NEW(element);
			element.node := node(Node);
			element.next := list;
			list := element;
			node := Internalize(rd)
		END;
		WHILE list # NIL DO	(* reverse list *)
			temp := list; list := temp.next; temp.next := nodeList; nodeList := temp
		END;
		RETURN nodeList
	END InternalizeList;

	(*	Reads in the internal fields of nodes whose pointers have already been read and stored
	in the 'nodes' array.	*)
	PROCEDURE InternalizeNodeData* (VAR rd: Stores.Reader);
		VAR
			i, numPtr: INTEGER;
	BEGIN
		i := 1;
		numPtr := nodeLabel;
		WHILE i <= numPtr DO
			ASSERT(nodes[i] # NIL, 21);
			nodes[i].Internalize(rd);
			INC(i)
		END
	END InternalizeNodeData;

	(*	initializes the externalization of graph	*)
	PROCEDURE BeginExternalize* (VAR wr: Stores.Writer);
		VAR
			i, len: INTEGER;
	BEGIN
		wr.WriteLong(timeStamp);
		startOfGraph := wr.Pos();
		wr.WriteInt(MIN(INTEGER));
		wr.WriteInt(MIN(INTEGER));
		nodes := NIL;
		nodeList := NIL;
		nodeLabel := 0;
		i := 0;
		len := LEN(installProc);
		WHILE i < len DO
			installProc[i] := "";
			factories[i] := NIL;
			INC(i)
		END;
		typeLabel := 0
	END BeginExternalize;

	(*	finalizes the externalization of graph	*)
	PROCEDURE EndExternalize* (VAR wr: Stores.Writer);
		VAR
			endPos, numNodes, i, numTypes: INTEGER;
	BEGIN
		endPos := wr.Pos();
		wr.SetPos(startOfGraph);
		numNodes := nodeLabel;
		wr.WriteInt(numNodes);
		wr.WriteInt(endPos);
		(*	clear label field	*)
		WHILE nodeList # NIL DO
			nodeList.node.label := 0;
			nodeList := nodeList.next
		END;
		wr.SetPos(endPos);
		numTypes := typeLabel;
		wr.WriteInt(numTypes);
		i := 0;
		WHILE i < numTypes DO
			wr.WriteString(installProc[i]);
			INC(i)
		END
	END EndExternalize;

	(*	initialize the internalization of graph	*)
	PROCEDURE BeginInternalize* (VAR rd: Stores.Reader);
		VAR
			numNodes, endPos, pos, numTypes, i: INTEGER;
			string: ARRAY 128 OF CHAR;
	BEGIN
		rd.ReadLong(timeStamp);
		rd.ReadInt(numNodes);
		NEW(nodes, numNodes + 1);
		nodes[0] := NIL;
		nodeLabel := 0;
		rd.ReadInt(endPos);
		pos := rd.Pos();
		rd.SetPos(endPos);
		rd.ReadInt(numTypes);
		i := 0;
		Strings.IntToString(timeStamp, string);
		WHILE i < numTypes DO
			rd.ReadString(installProc[i]);
			factories[i] := InstallFactory(installProc[i]);
			ASSERT(factories[i] # NIL, 99);
			INC(i)
		END;
		rd.SetPos(pos)
	END BeginInternalize;

	(*	finalize the internalization of graph	*)
	PROCEDURE EndInternalize* (VAR rd: Stores.Reader);
		VAR
			i, len, numTypes: INTEGER;
			string: String;
	BEGIN
		nodes := NIL;
		nodeLabel := 0;
		i := 0;
		len := LEN(installProc);
		WHILE i < len DO
			factories[i] := NIL;
			INC(i)
		END;
		typeLabel := 0;
		(*	read to end of type info	*)
		rd.ReadInt(numTypes);
		i := 0;
		WHILE i < numTypes DO
			rd.ReadString(string);
			INC(i)
		END
	END EndInternalize;

	PROCEDURE GetInstallProc* (index: INTEGER; OUT name: ARRAY OF CHAR);
	BEGIN
		IF index < maxNumTypes THEN
			name := installProc[index]$
		ELSE
			name := ""
		END
	END GetInstallProc;

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
			i := 0;
			cursor := list;
			WHILE cursor # NIL DO
				vector[i] := cursor.node;
				INC(i);
				cursor := cursor.next
			END
		END;
		RETURN vector
	END ListToVector;

	(*	sets depth information	*)
	PROCEDURE SetDepth* (maxD, maxSD: INTEGER);
	BEGIN
		maxDepth := maxD;
		maxStochDepth := maxSD
	END SetDepth;

	(*	sets global factory object, fact, for creating nodes in graphical model	*)
	PROCEDURE SetFactory* (f: Factory);
	BEGIN
		fact := f
	END SetFactory;

	(*	clears global parameters	*)
	PROCEDURE Clear*;
	BEGIN
		maxDepth := 0;
		maxStochDepth := 0;
		timeStamp := - 1
	END Clear;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		fact := NIL;
		nodes := NIL;
		nodeLabel := 0;
		installProc[0] := "";
		typeLabel := 1;
		timeStamp := - 1
	END Init;

BEGIN
	Init
END GraphNodes.


