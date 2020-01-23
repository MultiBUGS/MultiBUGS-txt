(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		 *)

MODULE SpatialBound;


	

	IMPORT

		Math, Stores := Stores64, 
		GraphNodes, GraphRules, GraphScalar, GraphStochastic, 
		MathMatrix;

	TYPE

		Node = POINTER TO ABSTRACT RECORD (GraphScalar.Node)
			adj, cols: POINTER TO ARRAY OF INTEGER;
			c, m: GraphNodes.Vector
		END;

		MaxNode = POINTER TO RECORD (Node) END;

		MinNode = POINTER TO RECORD (Node) END;

		MaxFactory = POINTER TO RECORD (GraphScalar.Factory) END;

		MinFactory = POINTER TO RECORD (GraphScalar.Factory) END;

	VAR
		factMax-, factMin-: GraphScalar.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		
	PROCEDURE Value (IN c, m: ARRAY OF REAL; IN adj, num: ARRAY OF INTEGER; index: INTEGER): REAL;
		VAR
			a: POINTER TO ARRAY OF ARRAY OF REAL; 
			eigenValues: POINTER TO ARRAY OF REAL;
			row, j, k, len, n, col: INTEGER; 
			mRow, mCol: REAL;
	BEGIN
		len := LEN(c); 
		n := LEN(num);
		NEW(a, n, n);
		NEW(eigenValues, n);
		row := 0;
		WHILE row < n DO
			col := 0;
			WHILE col < n DO
				a[row, col] := 0.0;
				INC(col)
			END;
			INC(row)
		END;
		row := 0; 
		j := 0;
		WHILE row < n DO
			mRow := m[row];
			k := j + num[row];
			WHILE j < k DO
				col := adj[j];
				mCol := m[col];
				a[col, row] := c[j] * Math.Sqrt(mCol / mRow);
				a[row, col] := a[col, row];
				INC(j)
			END;
			INC(row)
		END;
		MathMatrix.Jacobi(a, eigenValues, n);
		RETURN 1.0 / eigenValues[index]
	END Value;

	PROCEDURE (node: Node) Check (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			form, i, len: INTEGER;
			stochastic: GraphStochastic.Node;
	BEGIN
		stochastic := parent(GraphStochastic.Node);
		i := 0; len := LEN(node.c); form := GraphRules.const;
		WHILE (i < len) & (form = GraphRules.const) DO
			form := GraphStochastic.ClassFunction(node.c[i], stochastic); INC(i)
		END;
		IF form # GraphRules.const THEN form := GraphRules.other END;
		RETURN form
	END ClassFunction;

	PROCEDURE (node: Node) EvaluateDiffs ;
	BEGIN
		HALT(126)
	END EvaluateDiffs;

	PROCEDURE (node: Node) ExternalizeScalar (VAR wr: Stores.Writer);
		VAR
			i, len: INTEGER;
	BEGIN
		len := LEN(node.adj);
		wr.WriteInt(len);
		i := 0; WHILE i < len DO wr.WriteInt(node.adj[i]); INC(i) END;
		len := LEN(node.cols);
		wr.WriteInt(len);
		i := 0; WHILE i < len DO wr.WriteInt(node.cols[i]); INC(i) END;
		len := LEN(node.c);
		wr.WriteInt(len);
		i := 0; WHILE i < len DO GraphNodes.Externalize(node.c[i], wr); INC(i) END;
		len := LEN(node.m);
		wr.WriteInt(len);
		i := 0; WHILE i < len DO GraphNodes.Externalize(node.m[i], wr); INC(i) END;
	END ExternalizeScalar;

	PROCEDURE (node: Node) InternalizeScalar (VAR rd: Stores.Reader);
		VAR
			i, len: INTEGER;
	BEGIN
		rd.ReadInt(len);
		NEW(node.adj, len);
		i := 0; WHILE i < len DO rd.ReadInt(node.adj[i]); INC(i) END;
		rd.ReadInt(len);
		NEW(node.cols, len);
		i := 0; WHILE i < len DO rd.ReadInt(node.cols[i]); INC(i) END;
		rd.ReadInt(len);
		NEW(node.c, len);
		i := 0; WHILE i < len DO node.c[i] := GraphNodes.Internalize(rd); INC(i) END;
		len := LEN(node.m);
		rd.ReadInt(len);
		i := 0; WHILE i < len DO node.m[i] := GraphNodes.Internalize(rd); INC(i) END;
	END InternalizeScalar;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.adj := NIL;
		node.cols := NIL;
		node.c := NIL
	END InitLogical;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			i, len: INTEGER;
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		i := 0;
		len := LEN(node.c);
		WHILE i < len DO
			p := node.c[i];
			p.AddParent(list);
			INC(i)
		END;
		i := 0;
		len := LEN(node.m);
		WHILE i < len DO
			p := node.m[i];
			p.AddParent(list);
			INC(i)
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
		CONST
			eps = 1.0E-10;
		VAR
			i, nElem, start: INTEGER;
			isData: BOOLEAN;
	BEGIN
		res := {};
		isData := TRUE;
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.vectors[0].components # NIL, 21); ASSERT(args.vectors[0].start >= 0, 21);
			ASSERT(args.vectors[0].nElem > 0, 21);
			ASSERT(args.vectors[1].components # NIL, 21); ASSERT(args.vectors[1].start >= 0, 21);
			ASSERT(args.vectors[1].nElem > 0, 21);
			ASSERT(args.vectors[2].components # NIL, 21); ASSERT(args.vectors[2].start >= 0, 21);
			ASSERT(args.vectors[2].nElem > 0, 21);
			ASSERT(args.vectors[3].components # NIL, 21); ASSERT(args.vectors[3].start >= 0, 21);
			ASSERT(args.vectors[3].nElem > 0, 21);
			nElem := args.vectors[0].nElem; start := args.vectors[0].start;
			IF start + nElem > LEN(args.vectors[0].components) THEN res := {1}; RETURN END;
			NEW(node.c, nElem);
			i := 0;
			WHILE i < nElem DO
				node.c[i] := args.vectors[0].components[start + i];
				IF node.c[i] = NIL THEN res := {GraphNodes.arg1, GraphNodes.nil}; RETURN END;
				IF ~(GraphNodes.data IN node.c[i].props) THEN isData := FALSE END;
				INC(i)
			END;
			nElem := args.vectors[1].nElem; start := args.vectors[1].start;
			IF start + nElem > LEN(args.vectors[1].components) THEN
				res := {GraphNodes.arg2, GraphNodes.length}; RETURN
			END;
			i := 0;
			WHILE i < nElem DO
				IF args.vectors[1].components[start + i] = NIL THEN
					res := {GraphNodes.arg2, GraphNodes.nil}; RETURN
				END;
				IF ~(GraphNodes.data IN args.vectors[1].components[start + i].props) THEN
					res := {GraphNodes.arg2, GraphNodes.notData}; RETURN
				END;
				INC(i)
			END;
			IF args.vectors[0].nElem # args.vectors[1].nElem THEN
				res := {GraphNodes.arg2, GraphNodes.length}; RETURN
			END;
			NEW(node.adj, nElem);
			i := 0;
			WHILE i < nElem DO
				node.adj[i] := SHORT(ENTIER(args.vectors[1].components[start + i].value + eps)) - 1; 
				INC(i)
			END;
			nElem := args.vectors[2].nElem; start := args.vectors[2].start;
			IF start + nElem > LEN(args.vectors[2].components) THEN
				res := {GraphNodes.arg3, GraphNodes.length}; RETURN
			END;
			i := 0;
			WHILE i < nElem DO
				IF args.vectors[2].components[start + i] = NIL THEN
					res := {GraphNodes.arg3, GraphNodes.nil}; RETURN
				END;
				IF ~(GraphNodes.data IN args.vectors[2].components[start + i].props) THEN
					res := {GraphNodes.arg3, GraphNodes.notData}; RETURN
				END;
				INC(i)
			END;
			NEW(node.cols, nElem);
			i := 0;
			WHILE i < nElem DO
				node.cols[i] := SHORT(ENTIER(args.vectors[2].components[start + i].value + eps)); 
				INC(i)
			END;
			nElem := args.vectors[3].nElem; start := args.vectors[3].start;
			IF start + nElem > LEN(args.vectors[3].components) THEN
				res := {GraphNodes.arg4, GraphNodes.length}; RETURN
			END;
			NEW(node.m, nElem);
			i := 0;
			WHILE i < nElem DO
				node.m[i] := args.vectors[3].components[start + i];
				IF node.m[i] = NIL THEN
					res := {GraphNodes.arg4, GraphNodes.nil}; RETURN
				END;
				IF ~(GraphNodes.data IN node.m[i].props) THEN isData := FALSE END;
				INC(i)
			END;
			IF args.vectors[2].nElem # args.vectors[3]. nElem THEN
				res := {GraphNodes.arg4, GraphNodes.length}; RETURN
			END
		END;
		IF isData THEN INCL(node.props, GraphNodes.data) END
	END Set;

	PROCEDURE (node: MaxNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "SpatialBound.MaxInstall"
	END Install;

	PROCEDURE (node: MaxNode) Evaluate;
		VAR
			i, len, nElem: INTEGER;
			c, m: POINTER TO ARRAY OF REAL;
	BEGIN
		i := 0; len := LEN(node.c); NEW(c, len);
		WHILE i < len DO c[i] := node.c[i].value; INC(i) END;
		i := 0; nElem := LEN(node.m); NEW(m, nElem);
		WHILE i < nElem DO m[i] := node.m[i].value; INC(i) END;
		node.value := Value(c, m, node.adj, node.cols, nElem - 1);
	END Evaluate;

	PROCEDURE (node: MinNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "SpatialBound.MinInstall"
	END Install;

	PROCEDURE (node: MinNode) Evaluate;
		VAR
			i, len, nElem: INTEGER;
			value: REAL;
			c, m: POINTER TO ARRAY OF REAL;
	BEGIN
		i := 0; len := LEN(node.c); NEW(c, len);
		WHILE i < len DO c[i] := node.c[i].value; INC(i) END;
		i := 0; nElem := LEN(node.m); NEW(m, nElem);
		WHILE i < nElem DO m[i] := node.m[i].value; INC(i) END;
		node.value := Value(c, m, node.adj, node.cols, 0);
	END Evaluate;

	PROCEDURE (f: MaxFactory) New (): GraphScalar.Node;
		VAR
			node: MaxNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: MaxFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vvvv"
	END Signature;

	PROCEDURE (f: MinFactory) New (): GraphScalar.Node;
		VAR
			node: MinNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: MinFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vvvv"
	END Signature;

	PROCEDURE MaxInstall*;
	BEGIN
		GraphNodes.SetFactory(factMax)
	END MaxInstall;

	PROCEDURE MinInstall*;
	BEGIN
		GraphNodes.SetFactory(factMin)
	END MinInstall;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			fMax: MaxFactory;
			fMin: MinFactory;
	BEGIN
		Maintainer;
		NEW(fMax);
		factMax := fMax;
		NEW(fMin);
		factMin := fMin
	END Init;

BEGIN
	Init
END SpatialBound.

