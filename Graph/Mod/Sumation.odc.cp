(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphSumation;


	

	IMPORT
		Math, Stores := Stores64,
		GraphLogical, GraphNodes, GraphRules, GraphScalar, GraphStochastic;

	TYPE

		Node = POINTER TO ABSTRACT RECORD(GraphScalar.Node)
			start, step, nElem: INTEGER;
			vector: GraphNodes.Vector;
			values: POINTER TO ARRAY OF SHORTREAL;
		END;

		SumNode = POINTER TO RECORD(Node) END;

		MeanNode = POINTER TO RECORD(Node) END;

		SdNode = POINTER TO RECORD(Node) END;

		SumFactory = POINTER TO RECORD(GraphScalar.Factory) END;

		MeanFactory = POINTER TO RECORD(GraphScalar.Factory) END;

		SdFactory = POINTER TO RECORD(GraphScalar.Factory) END;

	VAR
		sumFact-, meanFact-, sdFact-: GraphScalar.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) Check (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE ClassFunction (node: Node; parent: GraphNodes.Node): INTEGER;
		VAR
			i, f, form, off, nElem, step, start: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		IF node.vector = NIL THEN RETURN GraphRules.const END;
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
				form := GraphRules.addF[form, f]
			END;
			INC(i)
		END;
		RETURN form
	END ClassFunction;

	PROCEDURE (node: Node) ExternalizeScalar (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		v.Init;
		v.components := node.vector;
		v.values := node.values;
		v.start := node.start; v.nElem := node.nElem; v.step := node.step;
		GraphNodes.ExternalizeSubvector(v, wr)
	END ExternalizeScalar;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.vector := NIL;
		node.values := NIL;
		node.start := - 1;
		node.nElem := - 1;
		node.step := - 1
	END InitLogical;

	PROCEDURE (node: Node) InternalizeScalar (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		GraphNodes.InternalizeSubvector(v, rd);
		node.vector := v.components;
		node.values := v.values;
		node.start := v.start; node.nElem := v.nElem; node.step := v.step
	END InternalizeScalar;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			i, off, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
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
			node.values := args.vectors[0].values;
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
						res := {GraphNodes.nil, GraphNodes.arg1};
						RETURN
					ELSE
						isData := isData & (GraphNodes.data IN p.props)
					END;
					INC(i)
				END
			END;
			IF isData THEN INCL(node.props, GraphNodes.data) END
		END
	END Set;

	PROCEDURE (node: SumNode) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			form: INTEGER;
	BEGIN
		form := ClassFunction(node, parent);
		RETURN form
	END ClassFunction;

	PROCEDURE (node: SumNode) Evaluate;
		VAR
			i, off, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
			value: REAL;
	BEGIN
		value := 0.0;
		i := 0;
		nElem := node.nElem;
		start := node.start;
		step := node.step;
		WHILE i < nElem DO
			off := start + i * step;
			IF node.vector # NIL THEN
				p := node.vector[off];
				value := value + p.value
			ELSE
				value := value + node.values[off]
			END;
			INC(i)
		END;
		node.value := value
	END Evaluate;

	PROCEDURE (node: SumNode) EvaluateDiffs;
		VAR
			i, j, off, nElem, start, step, N: INTEGER;
			p: GraphNodes.Node;
			value: REAL;
			x: GraphNodes.Vector;
	BEGIN
		x := node.diffWRT;
		N := LEN(x);
		value := 0.0;
		i := 0; WHILE i < N DO node.diffs[i] := 0.0; INC(i) END;
		IF node.vector = NIL THEN RETURN END;
		nElem := node.nElem;
		start := node.start;
		step := node.step;
		j := 0;
		WHILE j < nElem DO
			off := start + j * step;
			p := node.vector[off];
			i := 0;
			WHILE i < N DO
				IF p = x[i] THEN
					node.diffs[i] := node.diffs[i] + 1.0;
				ELSIF ~(GraphNodes.data IN p.props) THEN
					node.diffs[i] := node.diffs[i] + p.Diff(x[i])
				END;
				INC(i)
			END;
			IF node.vector # NIL THEN
				value := value + p.value
			ELSE
				value := value + node.values[off]
			END;
			INC(j)
		END;
		node.value := value
	END EvaluateDiffs;

	PROCEDURE (node: SumNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphSumation.SumInstall"
	END Install;

	PROCEDURE (node: MeanNode) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			form: INTEGER;
	BEGIN
		form := ClassFunction(node, parent);
		RETURN form
	END ClassFunction;

	PROCEDURE (node: MeanNode) Evaluate;
		VAR
			i, off, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
			value: REAL;
	BEGIN
		value := 0.0;
		i := 0;
		nElem := node.nElem;
		start := node.start;
		step := node.step;
		WHILE i < nElem DO
			off := start + i * step;
			IF node.vector # NIL THEN
				p := node.vector[off];
				value := value + p.value
			ELSE
				value := value + node.values[off]
			END;
			INC(i)
		END;
		node.value := value / node.nElem
	END Evaluate;

	PROCEDURE (node: MeanNode) EvaluateDiffs;
		VAR
			i, j, off, nElem, start, step, N: INTEGER;
			p: GraphNodes.Node;
			x: GraphNodes.Vector;
	BEGIN
		x := node.diffWRT;
		N := LEN(x);
		i := 0;
		WHILE i < N DO node.diffs[i] := 0.0; INC(i) END;
		j := 0;
		nElem := node.nElem;
		start := node.start;
		step := node.step;
		WHILE i < nElem DO
			off := start + j * step;
			p := node.vector[off];
			i := 0;
			WHILE i < N DO
				IF p = x[i] THEN
					node.diffs[i] := node.diffs[i] + 1.0;
				ELSIF ~(GraphNodes.data IN p.props) THEN
					node.diffs[i] := node.diffs[i] + p.Diff(x[i])
				END;
				INC(i)
			END;
			INC(j)
		END;
		i := 0; WHILE i < N DO node.diffs[i] := node.diffs[i] / node.nElem; INC(i) END
	END EvaluateDiffs;

	PROCEDURE (node: MeanNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphSumation.MeanInstall"
	END Install;

	PROCEDURE (node: SdNode) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			form, i, off, nElem, step, start: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		form := GraphRules.const;
		IF node.vector = NIL THEN RETURN form END;
		i := 0;
		nElem := node.nElem;
		start := node.start;
		step := node.step;
		WHILE (i < nElem) & (form = GraphRules.const) DO
			off := start + i * step;
			p := node.vector[off];
			form := GraphStochastic.ClassFunction(p, parent);
			INC(i)
		END;
		IF form # GraphRules.const THEN
			form := GraphRules.other
		END;
		RETURN form
	END ClassFunction;


	PROCEDURE (node: SdNode) Evaluate;
		VAR
			i, off, nElem, start, step: INTEGER;
			sum, sum2, value: REAL;
			p: GraphNodes.Node;
	BEGIN
		sum := 0.0;
		sum2 := 0.0;
		i := 0;
		nElem := node.nElem;
		start := node.start;
		step := node.step;
		WHILE i < nElem DO
			off := start + i * step;
			IF node.vector # NIL THEN
				p := node.vector[off];
				value := p.value
			ELSE
				value := node.values[off]
			END;
			sum := sum + value;
			sum2 := sum2 + value * value;
			INC(i)
		END;
		sum := sum / nElem;
		sum2 := sum2 / nElem;
		node.value := Math.Sqrt(nElem * (sum2 - sum * sum) / (nElem - 1))
	END Evaluate;

	PROCEDURE (node: SdNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphSumation.SdInstall"
	END Install;

	PROCEDURE (node: SdNode) EvaluateDiffs;
		VAR
			i, j, off, nElem, start, step, N: INTEGER;
			sum, sum2, differ1, differ2, d, differ, value: REAL;
			p: GraphNodes.Node;
			x: GraphNodes.Vector;
	BEGIN
		x := node.diffWRT;
		N := LEN(x);
		i := 0; WHILE i < N DO node.diffs[i] := 0.0; INC(i) END;
		IF node.vector = NIL THEN RETURN END;
		sum := 0.0;
		sum2 := 0.0;
		j := 0;
		nElem := node.nElem;
		start := node.start;
		step := node.step;
		WHILE j < nElem DO
			off := start + j * step;
			p := node.vector[off];
			value := p.value;
			sum := sum + value;
			sum2 := sum2 + value * value;
			INC(j)
		END;
		sum := sum / nElem;
		sum2 := sum2 / nElem;
		sum2 := Math.Sqrt(nElem * (sum2 - sum * sum) / (nElem - 1));
		i := 0;
		WHILE i < N DO
			differ1 := 0.0;
			differ2 := 0.0;
			j := 0;
			WHILE j < nElem DO
				off := start + j * step;
				p := node.vector[off];
				value := p.value;
				IF p = x[i] THEN
					differ1 := differ1 + 1.0;
					differ2 := differ2 + 2.0 * value
				ELSE
					d := p.Diff(x[i]);
					differ1 := differ1 + d;
					differ2 := differ2 + 2.0 * value * d
				END;
			END;
			node.diffs[i] := (differ2 - 2.0 * sum * differ1) * nElem / ((nElem - 1) * 2.0 * sum2);
			INC(i)
		END
	END EvaluateDiffs;

	PROCEDURE (f: SumFactory) New (): GraphScalar.Node;
		VAR
			node: SumNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: SumFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "v"
	END Signature;

	PROCEDURE SumInstall*;
	BEGIN
		GraphNodes.SetFactory(sumFact)
	END SumInstall;

	PROCEDURE (f: MeanFactory) New (): GraphScalar.Node;
		VAR
			node: MeanNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: MeanFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "v"
	END Signature;

	PROCEDURE MeanInstall*;
	BEGIN
		GraphNodes.SetFactory(meanFact)
	END MeanInstall;

	PROCEDURE (f: SdFactory) New (): GraphScalar.Node;
		VAR
			node: SdNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: SdFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "v"
	END Signature;

	PROCEDURE SdInstall*;
	BEGIN
		GraphNodes.SetFactory(sdFact)
	END SdInstall;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			fSum: SumFactory;
			fMean: MeanFactory;
			fSd: SdFactory;
	BEGIN
		Maintainer;
		NEW(fSum);
		sumFact := fSum;
		NEW(fMean);
		meanFact := fMean;
		NEW(fSd);
		sdFact := fSd
	END Init;

BEGIN
	Init
END GraphSumation.
