(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		  *)

MODULE GraphTable;


	

	IMPORT
		Stores, 
		GraphLogical, GraphNodes, GraphRules, GraphScalar, GraphStochastic;

	TYPE

		Node = POINTER TO RECORD(GraphScalar.MemNode)
			nElem, xStart, xStep, yStart, yStep: INTEGER;
			x0: GraphNodes.Node;
			x, y: GraphNodes.Vector
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
			f, form, i, nElem, xStart, xStep, yStart, yStep: INTEGER;
	BEGIN
		form := GraphStochastic.ClassFunction(node.x0, parent);
		IF form # GraphRules.const THEN
			form := GraphRules.other
		END;
		i := 0;
		xStart := node.xStart;
		xStep := node.xStep;
		yStart := node.yStart;
		yStep := node.yStep;
		nElem := node.nElem;
		WHILE (i < nElem) & (form # GraphRules.other) DO
			f := GraphStochastic.ClassFunction(node.x[xStart + i * xStep], parent);
			IF f # GraphRules.const THEN
				form := GraphRules.other
			END;
			f := GraphStochastic.ClassFunction(node.y[yStart + i * yStep], parent);
			IF f # GraphRules.const THEN
				form := GraphRules.other
			END;
			INC(i)
		END;
		RETURN form
	END ClassFunction;

	PROCEDURE (node: Node) Evaluate (OUT value: REAL);
		VAR
			i, lower, upper, xStart, xStep, yStart, yStep: INTEGER;
			delta, slope, x0: REAL;
	BEGIN
		xStart := node.xStart;
		xStep := node.xStep;
		yStart := node.yStart;
		yStep := node.yStep;
		x0 := node.x0.Value();
		lower := 0;
		upper := node.nElem;
		WHILE upper - lower > 1 DO
			i := (lower + upper) DIV 2;
			IF node.x[xStart + i * xStep].Value() < x0 THEN
				lower := i
			ELSE
				upper := i
			END
		END;
		i := lower;
		delta := x0 - node.x[xStart + i * xStep].Value();
		slope := (node.y[yStart + (i + 1) * yStep].Value() - node.y[yStart + i * yStep].Value()) / 
		(node.x[xStart + (i + 1) * xStep].Value() - node.x[xStart + i * xStep].Value());
		value := node.y[yStart + i * yStep].Value() + delta * slope
	END Evaluate;

	PROCEDURE (node: Node) EvaluateVD (x: GraphNodes.Node; OUT val, diff: REAL);
		VAR
			i, lower, upper, xStart, xStep, yStart, yStep: INTEGER;
			delta, slope, x0: REAL;
	BEGIN
		xStart := node.xStart;
		xStep := node.xStep;
		yStart := node.yStart;
		yStep := node.yStep;
		node.x0.ValDiff(x, x0, diff);
		lower := 0;
		upper := node.nElem;
		WHILE upper - lower > 1 DO
			i := (lower + upper) DIV 2;
			IF node.x[xStart + i * xStep].Value() < x0 THEN
				lower := i
			ELSE
				upper := i
			END
		END;
		i := lower;
		delta := x0 - node.x[xStart + i * xStep].Value();
		slope := (node.y[yStart + (i + 1) * yStep].Value() - node.y[yStart + i * yStep].Value()) / 
		(node.x[xStart + (i + 1) * xStep].Value() - node.x[xStart + i * xStep].Value());
		val := node.y[yStart + i * yStep].Value() + delta * slope;
		diff := diff * slope
	END EvaluateVD;
	
	PROCEDURE (node: Node) ExternalizeScalar (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		v := GraphNodes.NewVector();
		v.components := node.x;
		v.start := node.xStart; v.nElem := node.nElem; v.step := node.xStep;
		GraphNodes.ExternalizeSubvector(v, wr);
		v.components := node.y;
		v.start := node.yStart; v.nElem := node.nElem; v.step := node.yStep;
		GraphNodes.ExternalizeSubvector(v, wr);
		GraphNodes.Externalize(node.x0, wr)
	END ExternalizeScalar;

	PROCEDURE (node: Node) InternalizeScalar (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		GraphNodes.InternalizeSubvector(v, rd);
		node.x := v.components;
		node.xStart := v.start; node.nElem := v.nElem; node.xStep := v.step;
		GraphNodes.InternalizeSubvector(v, rd);
		node.y := v.components;
		node.yStart := v.start; node.nElem := v.nElem; node.yStep := v.step;
		node.x0 := GraphNodes.Internalize(rd)
	END InternalizeScalar;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.x0 := NIL;
		node.x := NIL;
		node.y := NIL;
		node.xStart := - 1;
		node.xStep := 0;
		node.yStart := - 1;
		node.yStep := 0;
		node.nElem := 0
	END InitLogical;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphTable.Install"
	END Install;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			i, nElem, xStart, xStep, yStart, yStep: INTEGER;
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		p := node.x0;
		p.AddParent(list);
		i := 0;
		xStart := node.xStart;
		xStep := node.xStep;
		yStart := node.yStart;
		yStep := node.yStep;
		nElem := node.nElem;
		WHILE i < nElem DO
			p := node.x[xStart + i * xStep];
			p.AddParent(list);
			p := node.y[yStart + i * yStep];
			p.AddParent(list);
			INC(i)
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			i, off, nElem: INTEGER;
			p: GraphNodes.Node;
			isData: BOOLEAN;
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.x0 := args.scalars[0];
			isData := GraphNodes.data IN node.x0.props;
			ASSERT(args.vectors[0].components # NIL, 21);
			node.x := args.vectors[0].components;
			ASSERT(args.vectors[0].start >= 0, 21);
			node.xStart := args.vectors[0].start;
			node.xStep := args.vectors[0].step;
			ASSERT(args.vectors[0].nElem > 0, 21);
			node.nElem := args.vectors[0].nElem;
			ASSERT(args.vectors[1].components # NIL, 21);
			node.y := args.vectors[1].components;
			ASSERT(args.vectors[1].start >= 0, 21);
			node.yStart := args.vectors[1].start;
			node.yStep := args.vectors[1].step;
			ASSERT(args.vectors[1].nElem > 0, 21);
			nElem := args.vectors[1].nElem;
			IF nElem # node.nElem THEN
				res := {GraphNodes.length, 1};
				RETURN
			END;
			i := 0;
			WHILE i < node.nElem DO
				off := node.xStart + i * node.xStep;
				p := node.x[off];
				IF p = NIL THEN
					res := {GraphNodes.nil, GraphNodes.arg2}; RETURN
				ELSE
					isData := isData & (GraphNodes.data IN p.props)
				END;
				off := node.yStart + i * node.yStep;
				p := node.y[off];
				IF p = NIL THEN
					res := {GraphNodes.nil, GraphNodes.arg3}; RETURN
				ELSE
					isData := isData & (GraphNodes.data IN p.props)
				END;
				INC(i)
			END;
			IF isData THEN node.SetProps(node.props + {GraphNodes.data}) END
		END
	END Set;

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
		signature := "svv"
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
END GraphTable.



