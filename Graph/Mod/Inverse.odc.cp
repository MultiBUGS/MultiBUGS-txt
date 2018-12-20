(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphInverse;


	

	IMPORT
		Math, Stores,
		GraphLogical, GraphNodes, GraphRules, GraphStochastic, GraphVector,
		MathMatrix;

	TYPE
		Node = POINTER TO RECORD (GraphVector.Node)
			matrix: GraphNodes.Vector;
			constant: POINTER TO ARRAY OF SHORTREAL;
			dim, start, step: INTEGER
		END;

		Factory = POINTER TO RECORD (GraphVector.Factory) END;

	CONST
		eps = 1.0E-10;

	VAR
		tau: POINTER TO ARRAY OF ARRAY OF REAL;
		fact-: GraphVector.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) Check (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			dim, form, i, j, off, start, step: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		IF node.constant # NIL THEN RETURN GraphRules.const END;
		dim := node.dim;
		start := node.start;
		step := node.step;
		form := GraphRules.const;
		i := 0;
		WHILE (i < dim) & (form = GraphRules.const) DO
			j := 0;
			WHILE (j < dim) & (form = GraphRules.const) DO
				off := start + (i * dim + j) * step;
				p := node.matrix[off];
				form := GraphStochastic.ClassFunction(p, parent);
				INC(j)
			END;
			INC(i)
		END;
		IF form # GraphRules.const THEN
			form := GraphRules.other
		END;
		RETURN form
	END ClassFunction;

	PROCEDURE (node: Node) Evaluate (OUT values: ARRAY OF REAL);
		VAR
			dim, i, j, off, start, step: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		dim := node.dim;
		start := node.start;
		step := node.step;
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j <= i DO
				off := start + (i * dim + j) * step;
				IF node.matrix # NIL THEN
					p := node.matrix[off];
					tau[i, j] := p.Value()
				ELSE
					tau[i, j] := node.constant[off]
				END;
				tau[j, i] := tau[i, j];
				INC(j)
			END;
			INC(i)
		END;
		MathMatrix.Invert(tau, dim);
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				values[i * dim + j] := tau[i, j];
				INC(j)
			END;
			INC(i)
		END
	END Evaluate;

	PROCEDURE (node: Node) ExternalizeVector (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
			dim: INTEGER;
	BEGIN
		IF node.index = 0 THEN
			dim := node.dim;
			wr.WriteInt(dim);
			v := GraphNodes.NewVector();
			v.components := node.matrix; v.values := node.constant;
			v.start := node.start; v.nElem := dim * dim; v.step := node.step;
			GraphNodes.ExternalizeSubvector(v, wr)
		END
	END ExternalizeVector;

	PROCEDURE (node: Node) InternalizeVector (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
			dim, i, size: INTEGER;
			p: Node;
	BEGIN
		IF node.index = 0 THEN
			rd.ReadInt(dim);
			IF dim > LEN(tau, 0) THEN
				NEW(tau, dim, dim);
			END;
			node.dim := dim;
			GraphNodes.InternalizeSubvector(v, rd);
			node.matrix := v.components;
			node.constant := v.values;
			node.start := v.start;
			node.step := v.step;
			i := 1;
			size := node.Size();
			WHILE i < size DO
				p := node.components[i](Node);
				p.start := node.start;
				p.step := node.step;
				p.matrix := node.matrix;
				p.constant := node.constant;
				p.dim := node.dim;
				INC(i)
			END
		END
	END InternalizeVector;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.SetProps(node.props + {GraphLogical.dependent})
	END InitLogical;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphInverse.Install"
	END Install;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			dim, i, j, off, start, step: INTEGER;
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		IF node.constant # NIL THEN RETURN NIL END;
		list := NIL;
		dim := node.dim;
		start := node.start;
		step := node.step;
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				off := start + (i * dim + j) * step;
				p := node.matrix[off];
				p.AddParent(list);
				INC(j)
			END;
			INC(i)
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			dim, i, j, off, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
			isData: BOOLEAN;
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			nElem := node.Size();
			dim := SHORT(ENTIER(Math.Sqrt(nElem) + eps));
			IF ABS(dim * dim - nElem) > eps THEN
				res := {GraphNodes.length, GraphNodes.lhs};
				RETURN
			END;
			node.dim := dim;
			ASSERT((args.vectors[0].components # NIL) OR (args.vectors[0].values # NIL), 21);
			node.matrix := args.vectors[0].components;
			node.constant := args.vectors[0].values;
			ASSERT(args.vectors[0].start >= 0, 21);
			node.start := args.vectors[0].start;
			node.step := args.vectors[0].step;
			ASSERT(args.vectors[0].step = 1, 21);
			ASSERT(args.vectors[0].nElem > 0, 21);
			IF nElem # args.vectors[0].nElem THEN
				res := {GraphNodes.length, 0};
				RETURN
			END;
			IF nElem > LEN(tau, 0) THEN
				NEW(tau, dim, dim);
			END;
			isData := TRUE;
			i := 0;
			start := node.start;
			step := node.step;
			WHILE i < dim DO
				j := 0;
				WHILE j < dim DO
					IF j >= i THEN
						off := start + (i * dim + j) * step;
						IF node.matrix # NIL THEN
							p := node.matrix[off];
							IF p = NIL THEN
								res := {GraphNodes.nil, GraphNodes.arg1}; RETURN
							ELSE
								isData := isData & (GraphNodes.data IN p.props)
							END
						ELSE
							IF node.constant[off] = INF THEN 
								res := {GraphNodes.nil, GraphNodes.arg1}; RETURN 
							END
						END
					END;
					INC(j)
				END;
				INC(i)
			END;
			IF isData THEN node.SetProps(node.props + {GraphNodes.data}) END
		END
	END Set;

	PROCEDURE (node: Node) ValDiff (x: GraphNodes.Node; OUT val, diff: REAL);
	BEGIN
		HALT(0)
	END ValDiff;

	PROCEDURE (f: Factory) New (): GraphVector.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "v"
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		CONST
			len = 20;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f;
		NEW(tau, len, len);
	END Init;

BEGIN
	Init
END GraphInverse.
