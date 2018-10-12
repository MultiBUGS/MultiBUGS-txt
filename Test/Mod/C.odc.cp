MODULE TestC;

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
		fact: Factory; 	(*	factory object for creating nodes in graphical model	*)
		nodeLabel, typeLabel: INTEGER;
		nodes: Vector;
		nodeList: List;
		installProc: ARRAY maxNumTypes OF String;
		factories: ARRAY maxNumTypes OF Factory;
		startOfGraph: INTEGER;

	(*	checks that internal state of node is consistant	*)
	PROCEDURE (node: Node) Check* (): SET, NEW, ABSTRACT;

	(*	initializes the internal fields of node to non sensible / possible values	*)
	PROCEDURE (node: Node) InitNode-, NEW, ABSTRACT;

		(*	gets name of the Install procedure for this type of node	*)
	PROCEDURE (node: Node) Install* (OUT install: ARRAY OF CHAR), NEW, ABSTRACT;

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
		vector.start := - 1;
		vector.nElem := - 1;
		vector.step := - 1;
		vector.components := NIL;
		RETURN vector
	END NewVector;


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
	END Init;

BEGIN
	Init	
END TestC.
