(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		  *)

MODULE GraphProduct;


	

	IMPORT
		Stores, 
		GraphLogical, GraphNodes, GraphRules, GraphScalar, GraphStochastic;

	TYPE

		Node = POINTER TO RECORD(GraphScalar.Node)
			start, step, nElem: INTEGER;
			vector: GraphNodes.Vector;
			constant: POINTER TO ARRAY OF SHORTREAL;
		END;

		Factory = POINTER TO RECORD(GraphScalar.Factory) END;

	VAR
		fact-: GraphScalar.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) Check (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			i, f, form, off, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		IF node.constant # NIL THEN RETURN GraphRules.const END;
		i := 0;
		nElem := node.nElem;
		start := node.start;
		step := node.step;
		WHILE i < nElem DO
			off := start + i * step;
			p := node.vector[off];
			f := GraphStochastic.ClassFunction(p, parent);
			IF i = 0 THEN
				form := f
			ELSE
				form := GraphRules.multF[form, f]
			END;
			INC(i)
		END;
		RETURN form
	END ClassFunction;

	PROCEDURE (node: Node) ExternalizeLogical (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		v := GraphNodes.NewVector();
		v.components := node.vector; v.values := node.constant;
		v.start := node.start; v.nElem := node.nElem; v.step := node.step; 
		GraphNodes.ExternalizeSubvector(v, wr)
	END ExternalizeLogical;

	PROCEDURE (node: Node) InternalizeLogical (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		GraphNodes.InternalizeSubvector(v, rd);
		node.vector := v.components; node.constant := v.values;
		node.start := v.start; node.nElem := v.nElem; node.step := v.step
	END InternalizeLogical;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.vector := NIL;
		node.constant := NIL;
		node.start := - 1;
		node.nElem := - 1;
		node.step := - 1
	END InitLogical;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphProduct.Install"
	END Install;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			i, off, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		IF node.constant # NIL THEN RETURN NIL END;
		list := NIL;
		i := 0;
		nElem := node.nElem;
		start := node.start;
		step := node.step;
		WHILE i < nElem DO
			off := start + i * step;
			p := node.vector[off];
			p.AddParent(list);
			INC(i)
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			i, off, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
			isData: BOOLEAN;
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT((args.vectors[0].components # NIL) OR (args.vectors[0].values # NIL), 21);
			node.vector := args.vectors[0].components;
			node.constant := args.vectors[0].values;
			ASSERT(args.vectors[0].start >= 0, 21);
			start := args.vectors[0].start;
			node.start := start;
			ASSERT(args.vectors[0].step > 0, 21);
			step := args.vectors[0].step;
			node.step := step;
			ASSERT(args.vectors[0].nElem > 0, 21);
			nElem := args.vectors[0].nElem;
			node.nElem := nElem;
			i := 0;
			isData := TRUE;
			IF node.vector # NIL THEN
				WHILE i < nElem DO
					off := start + i * step;
					p := node.vector[off];
					IF p = NIL THEN
						res := {GraphNodes.nil, GraphNodes.arg1}; RETURN
					ELSE
						isData := isData & (GraphNodes.data IN p.props)
					END;
					INC(i)
				END;
				IF isData THEN node.SetProps(node.props + {GraphNodes.data}) END
			END
		END
	END Set;

	PROCEDURE (node: Node) Value (): REAL;
		VAR
			i, off, nElem, start, step: INTEGER;
			value: REAL;
			p: GraphNodes.Node;
	BEGIN
		value := 1.0;
		i := 0;
		nElem := node.nElem;
		start := node.start;
		step := node.step;
		WHILE i < nElem DO
			off := start + i * step;
			IF node.vector # NIL THEN
				p := node.vector[off];
				value := value * p.Value()
			ELSE
				value := value * node.constant[off]
			END;
			INC(i)
		END;
		RETURN value
	END Value;

	PROCEDURE (node: Node) ValDiff (x: GraphNodes.Node; OUT val, diff: REAL);
		VAR
			i, off, nElem, start, step: INTEGER;
			differ, value: REAL;
			p: GraphNodes.Node;
	BEGIN
		val := 1.0;
		diff := 0.0;
		i := 0;
		nElem := node.nElem;
		start := node.start;
		step := node.step;
		WHILE i < nElem DO
			off := start + i * step;
			IF node.vector # NIL THEN
				p := node.vector[off];
				p.ValDiff(x, value, differ);
				diff := diff + differ / value;
				val := val * p.Value()
			ELSE
				val := val * node.constant[off]
			END;
			INC(i)
		END;
		diff := val * diff
	END ValDiff;

	PROCEDURE (f: Factory) New (): GraphScalar.Node;
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
		fact := f
	END Init;

BEGIN
	Init
END GraphProduct.
