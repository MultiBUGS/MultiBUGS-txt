(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		  *)

MODULE GraphTable;


	

	IMPORT
		Stores, 
		GraphLogical, GraphMemory, GraphNodes, GraphRules, GraphScalar, GraphStochastic;

	TYPE

		Node = POINTER TO RECORD(GraphMemory.Node)
			nElem, xStart, xStep, yStart, yStep: INTEGER;
			x0: GraphNodes.Node;
			x, y: GraphNodes.Vector;
			constantX, constantY: POINTER TO ARRAY OF SHORTREAL
		END;

		Factory = POINTER TO RECORD(GraphMemory.Factory) END;

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
			IF node.x # NIL THEN
				f := GraphStochastic.ClassFunction(node.x[xStart + i * xStep], parent)
			ELSE
				f := GraphRules.const
			END;
			IF f # GraphRules.const THEN
				form := GraphRules.other
			END;
			IF node.y # NIL THEN
				f := GraphStochastic.ClassFunction(node.y[yStart + i * yStep], parent)
			ELSE
				f := GraphRules.const
			END;
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
			delta, slope, x, x0, x1, y, y1: REAL;
			offX, offY: INTEGER;
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
			offX := xStart + i * xStep;
			IF node.x # NIL THEN x := node.x[offX].Value() ELSE x := node.constantX[offX] END;
			IF x < x0 THEN
				lower := i
			ELSE
				upper := i
			END
		END;
		i := lower;
		offX := xStart + i * xStep;
		IF node.x # NIL THEN 
			x := node.x[offX].Value();
			x1 := node.x[offX + xStep].Value()
		ELSE 
			x := node.constantX[offX];
			x1 := node.constantX[offX + xStep] 
		END;
		delta := x0 - x;
		offY:= yStart + i * yStep;
		IF node.y # NIL THEN 
			y := node.y[offY].Value();
			y1 := node.y[offY + yStep].Value()
		ELSE 
			y := node.constantY[offY];
			y1 := node.constantY[offY + yStep] 
		END;
		slope := (y1 - y) / (x1 - x);
		value := y + delta * slope
	END Evaluate;

	PROCEDURE (node: Node) EvaluateVD (z: GraphNodes.Node; OUT val, diff: REAL);
		VAR
			i, lower, upper, xStart, xStep, yStart, yStep: INTEGER;
			delta, slope, x, x0, x1, y, y1: REAL;
			offX, offY: INTEGER;
	BEGIN
		xStart := node.xStart;
		xStep := node.xStep;
		yStart := node.yStart;
		yStep := node.yStep;
		node.x0.ValDiff(z, x0, diff);
		lower := 0;
		upper := node.nElem;
		WHILE upper - lower > 1 DO
			i := (lower + upper) DIV 2;
			offX := xStart + i * xStep;
			IF node.x # NIL THEN x := node.x[offX].Value() ELSE x := node.constantX[offX] END;
			IF x < x0 THEN
				lower := i
			ELSE
				upper := i
			END
		END;
		i := lower;
		offX := xStart + i * xStep;
		IF node.x # NIL THEN 
			x := node.x[offX].Value();
			x1 := node.x[offX + xStep].Value()
		ELSE 
			x := node.constantX[offX];
			x1 := node.constantX[offX + xStep] 
		END;
		delta := x0 - x;
		offY:= yStart + i * yStep;
		IF node.y # NIL THEN 
			y := node.y[offY].Value();
			y1 := node.y[offY + yStep].Value()
		ELSE 
			y := node.constantY[offY];
			y1 := node.constantY[offY + yStep] 
		END;
		slope := (y1 - y) / (x1 - x);
		val := y + delta * slope;
		diff := diff * slope
	END EvaluateVD;
	
	PROCEDURE (node: Node) ExternalizeMemory (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		v := GraphNodes.NewVector();
		v.components := node.x; v.values := node.constantX;
		v.start := node.xStart; v.nElem := node.nElem; v.step := node.xStep;
		GraphNodes.ExternalizeSubvector(v, wr);
		v.components := node.y; v.values := node.constantY;
		v.start := node.yStart; v.nElem := node.nElem; v.step := node.yStep;
		GraphNodes.ExternalizeSubvector(v, wr);
		GraphNodes.Externalize(node.x0, wr)
	END ExternalizeMemory;

	PROCEDURE (node: Node) InternalizeMemory (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		GraphNodes.InternalizeSubvector(v, rd);
		node.x := v.components; node.constantX := v.values;
		node.xStart := v.start; node.nElem := v.nElem; node.xStep := v.step;
		GraphNodes.InternalizeSubvector(v, rd);
		node.y := v.components; node.constantY := v.values;
		node.yStart := v.start; node.nElem := v.nElem; node.yStep := v.step;
		node.x0 := GraphNodes.Internalize(rd)
	END InternalizeMemory;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.x0 := NIL;
		node.x := NIL;
		node.y := NIL;
		node.constantX := NIL;
		node.constantY := NIL;
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
			IF node.x # NIL THEN
				p := node.x[xStart + i * xStep];
				p.AddParent(list)
			END;
			IF node.y # NIL THEN
				p := node.y[yStart + i * yStep];
				p.AddParent(list)
			END;
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
			ASSERT((args.vectors[0].components # NIL) OR (args.vectors[0].values # NIL), 21);
			node.x := args.vectors[0].components;
			node.constantX := args.vectors[0].values;
			ASSERT(args.vectors[0].start >= 0, 21);
			node.xStart := args.vectors[0].start;
			node.xStep := args.vectors[0].step;
			ASSERT(args.vectors[0].nElem > 0, 21);
			node.nElem := args.vectors[0].nElem;
			ASSERT((args.vectors[1].components # NIL) OR (args.vectors[1].values # NIL), 21);
			node.y := args.vectors[1].components;
			node.constantY := args.vectors[1].values;
			ASSERT(args.vectors[1].start >= 0, 21);
			node.yStart := args.vectors[1].start;
			node.yStep := args.vectors[1].step;
			ASSERT(args.vectors[1].nElem > 0, 21);
			nElem := args.vectors[1].nElem;
			IF nElem # node.nElem THEN
				res := {GraphNodes.length, 1};
				RETURN
			END;
			IF node.x # NIL THEN
				i := 0;
				WHILE i < node.nElem DO
					off := node.xStart + i * node.xStep;
					p := node.x[off];
					IF p = NIL THEN
						res := {GraphNodes.nil, GraphNodes.arg2}; RETURN
					ELSE
						isData := isData & (GraphNodes.data IN p.props)
					END;
					INC(i)
				END
			END;
			IF node.y # NIL THEN
				i := 0; 
				WHILE i < node.nElem DO
					off := node.yStart + i * node.yStep;
					p := node.y[off];
					IF p = NIL THEN
						res := {GraphNodes.nil, GraphNodes.arg3}; RETURN
					ELSE
						isData := isData & (GraphNodes.data IN p.props)
					END;
					INC(i)
				END
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



