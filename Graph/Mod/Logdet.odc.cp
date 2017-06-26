(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphLogdet;


	

	IMPORT
		Math, Stores,
		GraphLogical, GraphMemory, GraphNodes, GraphRules, GraphStochastic,
		MathMatrix;

	TYPE
		Node = POINTER TO RECORD(GraphMemory.Node)
			dim, start, step: INTEGER;
			matrix: GraphNodes.Vector
		END;

		Factory = POINTER TO RECORD(GraphMemory.Factory) END;

	CONST
		eps = 1.0E-10;

	VAR
		fact-: GraphNodes.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		matrix: POINTER TO ARRAY OF ARRAY OF REAL;

	PROCEDURE (node: Node) Check (): SET;
	BEGIN
		(*	should check that symetric	*)
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			dim, form, i, j, off, start, step: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		i := 0;
		dim := node.dim;
		start := node.start;
		step := node.step;
		form := GraphRules.const;
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

	PROCEDURE (node: Node) Evaluate (OUT value: REAL);
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
			WHILE j < dim DO
				IF j >= i THEN
					off := start + (i * dim + j) * step;
					p := node.matrix[off];
					matrix[i, j] := p.Value()
				ELSE
					matrix[i, j] := matrix[j, i]
				END;
				INC(j)
			END;
			INC(i)
		END;
		value := MathMatrix.LogDet(matrix, dim)
	END Evaluate;

	PROCEDURE (node: Node) EvaluateVD (x: GraphNodes.Node; OUT val, diff: REAL);
	BEGIN
		HALT(126)
	END EvaluateVD;

	PROCEDURE (node: Node) ExternalizeMemory (VAR wr: Stores.Writer);
		VAR
			dim: INTEGER;
			v: GraphNodes.SubVector;
	BEGIN
		dim := node.dim;
		wr.WriteInt(dim);
		v := GraphNodes.NewVector();
		v.components := node.matrix;
		v.start := node.start; v.step := node.step; v.nElem := dim * dim;
		GraphNodes.ExternalizeSubvector(v, wr)
	END ExternalizeMemory;

	PROCEDURE (node: Node) InternalizeMemory (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		rd.ReadInt(node.dim);
		GraphNodes.InternalizeSubvector(v, rd);
		node.matrix := v.components;
		node.start := v.start; node.step := v.step;
	END InternalizeMemory;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.SetProps(node.props + {GraphLogical.dependent});
		node.matrix := NIL;
		node.start := - 1;
		node.step := 0;
		node.dim := - 1
	END InitLogical;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphLogdet.Install"
	END Install;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			dim, i, j, off, start, step: INTEGER;
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		i := 0;
		dim := node.dim;
		start := node.start;
		step := node.step;
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
			dim, i, j, off, start, step: INTEGER;
			p: GraphNodes.Node;
			isData: BOOLEAN;
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.vectors[0].components # NIL, 21);
			node.matrix := args.vectors[0].components;
			node.start := args.vectors[0].start;
			node.step := args.vectors[0].step;
			ASSERT(args.vectors[0].start >= 0, 21);
			start := node.start;
			step := node.step;
			ASSERT(args.vectors[0].step > 0, 21);
			ASSERT(args.vectors[0].nElem > 0, 21);
			dim := SHORT(ENTIER(Math.Sqrt(args.vectors[0].nElem) + eps));
			node.dim := dim;
			IF dim * dim # args.vectors[0].nElem THEN
				res := {GraphNodes.length, 0};
				RETURN
			END;
			IF dim > LEN(matrix, 0) THEN
				NEW(matrix, dim, dim)
			END;
			isData := TRUE;
			i := 0;
			WHILE i < dim DO
				j := 0;
				WHILE j < dim DO
					IF j >= i THEN
						off := start + (i * dim + j) * step;
						p := node.matrix[off];
						IF p = NIL THEN
							res := {GraphNodes.nil, GraphNodes.arg1}; RETURN
						ELSE
							isData := isData & (GraphNodes.data IN p.props)
						END
					END;
					INC(j)
				END;
				INC(i)
			END;
			IF isData THEN node.SetProps(node.props + {GraphNodes.data}) END
		END
	END Set;

	PROCEDURE (f: Factory) New (): GraphMemory.Node;
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
			dim = 20;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(matrix, dim, dim);
		NEW(f);
		fact := f
	END Init;

BEGIN
	Init
END GraphLogdet.
