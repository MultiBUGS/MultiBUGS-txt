(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphStick;


	

	IMPORT
		Stores,
		GraphLogical, GraphNodes, GraphRules, GraphStochastic, GraphVector;

	TYPE
		Node = POINTER TO RECORD (GraphVector.Node)
			prop: GraphNodes.Vector;
			constant: POINTER TO ARRAY OF SHORTREAL;
			dim, start, step: INTEGER
		END;

		Factory = POINTER TO RECORD (GraphVector.Factory) END;

	VAR
		fact-: GraphVector.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) Check (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			dim, form, i, index, off, start, step: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		IF node.constant # NIL THEN RETURN GraphRules.const END;
		dim := node.dim ;
		start := node.start;
		step := node.step;
		index := node.index;
		i := 0;
		LOOP
			off := start + i * step;
			p := node.prop[off];
			IF p = parent THEN 
				IF i = index THEN
					form := GraphRules.ident
				ELSIF i > index THEN
					form := GraphRules.const
				ELSE
					form := GraphRules.linear
				END;
				EXIT 
			END;
			INC(i);
			IF i = dim THEN form := GraphRules.const; EXIT END
		END;
		RETURN form
	END ClassFunction;

	PROCEDURE (node: Node) Evaluate (OUT values: ARRAY OF REAL);
		VAR
			dim, i, off, start, step: INTEGER;
			p: GraphNodes.Node;
			pVal, len: REAL;
	BEGIN
		dim := node.dim;
		start := node.start;
		step := node.step;
		p := node.prop[start];
		len := 1.0;
		i := 0;
		WHILE i < dim - 1 DO
			off := start + i * step;
			IF node.prop # NIL THEN
				p := node.prop[off];
				pVal := p.Value()
			ELSE
				pVal := node.constant[off]
			END;
			values[i] := len * pVal;
			len := len - values[i];
			INC(i)
		END;
		values[dim - 1] := len
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
			v.components := node.prop; v.values := node.constant;
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
			node.dim := dim;
			GraphNodes.InternalizeSubvector(v, rd);
			node.prop := v.components;
			node.constant := v.values;
			node.start := v.start;
			node.step := v.step;
			i := 1;
			size := node.Size();
			WHILE i < size DO
				p := node.components[i](Node);
				p.start := node.start;
				p.step := node.step;
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
		install := "GraphStick.Install"
	END Install;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			dim, i, off, start, step: INTEGER;
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
			off := start + i * step;
			p := node.prop[off];
			p.AddParent(list);
			INC(i)
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			dim, i, off, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
			isData: BOOLEAN;
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			nElem := node.Size();
			dim := nElem - 1;
			node.dim := dim;
			ASSERT((args.vectors[0].components # NIL) OR (args.vectors[0].values # NIL), 21);
			node.prop := args.vectors[0].components;
			node.constant := args.vectors[0].values;
			ASSERT(args.vectors[0].start >= 0, 21);
			node.start := args.vectors[0].start;
			node.step := args.vectors[0].step;
			ASSERT(args.vectors[0].step = 1, 21);
			ASSERT(args.vectors[0].nElem > 0, 21);
			IF dim # args.vectors[0].nElem THEN
				res := {GraphNodes.length, 0};
				RETURN
			END;
			isData := TRUE;
			i := 0;
			start := node.start;
			step := node.step;
			WHILE i < dim DO
				off := start + i * step;
				IF node.prop # NIL THEN
					p := node.prop[off];
					IF p = NIL THEN
						res := {GraphNodes.invalidParameters, GraphNodes.arg1}; RETURN
					ELSE
						isData := isData & (GraphNodes.data IN p.props);
						IF ~(p IS GraphStochastic.Node) THEN
							res := {GraphNodes.nil, GraphNodes.arg1}; RETURN
						END
					END
				ELSE
					IF node.constant[off] = INF THEN 
						res := {GraphNodes.nil, GraphNodes.arg1}; RETURN 
					END
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
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f;
	END Init;

BEGIN
	Init
END GraphStick.
