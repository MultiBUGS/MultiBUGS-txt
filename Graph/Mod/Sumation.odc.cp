(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphSumation;


	

	IMPORT
		Math, Stores,
		GraphLogical, GraphNodes, GraphRules, GraphScalar, GraphStochastic;

	TYPE

		Node = POINTER TO ABSTRACT RECORD(GraphScalar.MemNode)
			start, step, nElem: INTEGER;
			vector: GraphNodes.Vector
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
		v := GraphNodes.NewVector();
		v.components := node.vector;
		v.start := node.start; v.nElem := node.nElem; v.step := node.step;
		GraphNodes.ExternalizeSubvector(v, wr)
	END ExternalizeScalar;

	PROCEDURE (node: Node) InternalizeScalar (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		GraphNodes.InternalizeSubvector(v, rd);
		node.vector := v.components;
		node.start := v.start; node.nElem := v.nElem; node.step := v.step
	END InternalizeScalar;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.SetProps(node.props + {GraphLogical.dependent});
		node.vector := NIL;
		node.start := - 1;
		node.nElem := - 1;
		node.step := - 1
	END InitLogical;

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
			ASSERT(args.vectors[0].components # NIL, 21);
			node.vector := args.vectors[0].components;
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
			END;
			IF isData THEN node.SetProps(node.props + {GraphNodes.data}) END
		END
	END Set;

	PROCEDURE (node: SumNode) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			form: INTEGER;
	BEGIN
		form := ClassFunction(node, parent);
		RETURN form
	END ClassFunction;

	PROCEDURE (node: SumNode) Evaluate (OUT value: REAL);
		VAR
			i, off, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		value := 0.0;
		i := 0;
		nElem := node.nElem;
		start := node.start;
		step := node.step;
		WHILE i < nElem DO
			off := start + i * step;
			p := node.vector[off];
			value := value + p.Value();
			INC(i)
		END
	END Evaluate;

	PROCEDURE (node: SumNode) EvaluateVD (x: GraphNodes.Node; OUT value, differ: REAL);
		VAR
			i, off, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		value := 0.0;
		differ := 0.0;
		i := 0;
		nElem := node.nElem;
		start := node.start;
		step := node.step;
		WHILE i < nElem DO
			off := start + i * step;
			p := node.vector[off];
			IF p = x THEN differ := differ + 1.0 END;
			value := value + p.Value();
			INC(i)
		END
	END EvaluateVD;

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

	PROCEDURE (node: MeanNode) Evaluate (OUT value: REAL);
		VAR
			i, off, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		value := 0.0;
		i := 0;
		nElem := node.nElem;
		start := node.start;
		step := node.step;
		WHILE i < nElem DO
			off := start + i * step;
			p := node.vector[off];
			value := value + p.Value();
			INC(i)
		END;
		value := value / node.nElem
	END Evaluate;

	PROCEDURE (node: MeanNode) EvaluateVD (x: GraphNodes.Node; OUT value, differ: REAL);
		VAR
			i, off, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		value := 0.0;
		differ := 0.0;
		i := 0;
		nElem := node.nElem;
		start := node.start;
		step := node.step;
		WHILE i < nElem DO
			off := start + i * step;
			p := node.vector[off];
			IF p = x THEN differ := differ + 1.0 END;
			value := value + p.Value();
			INC(i)
		END;
		value := value / node.nElem;
		differ := differ / node.nElem
	END EvaluateVD;

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

	PROCEDURE (node: SdNode) Evaluate (OUT value: REAL);
		VAR
			i, off, nElem, start, step: INTEGER;
			node1, node2: REAL;
			p: GraphNodes.Node;
	BEGIN
		node1 := 0.0;
		node2 := 0.0;
		i := 0;
		nElem := node.nElem;
		start := node.start;
		step := node.step;
		WHILE i < nElem DO
			off := start + i * step;
			p := node.vector[off];
			value := p.Value();
			node1 := node1 + value;
			node2 := node2 + value * value;
			INC(i)
		END;
		node1 := node1 / nElem;
		node2 := node2 / nElem;
		value := Math.Sqrt(nElem * (node2 - node1 * node1) / (nElem - 1))
	END Evaluate;

	PROCEDURE (node: SdNode) EvaluateVD (x: GraphNodes.Node; OUT value, differ: REAL);
		VAR
			i, off, nElem, start, step: INTEGER;
			node1, node2, differ1, differ2: REAL;
			p: GraphNodes.Node;
	BEGIN
		node1 := 0.0;
		node2 := 0.0;
		differ1 := 0.0;
		differ2 := 0.0;
		i := 0;
		nElem := node.nElem;
		start := node.start;
		step := node.step;
		WHILE i < nElem DO
			off := start + i * step;
			p := node.vector[off];
			value := p.Value();
			node1 := node1 + value;
			node2 := node2 + value * value;
			IF p = x THEN
				differ1 := differ1 + 1.0;
				differ2 := differ2 + 2.0 * value
			END;
			INC(i)
		END;
		node1 := node1 / nElem;
		node2 := node2 / nElem;
		differ1 := differ1 / nElem;
		differ2 := differ2 / nElem;
		value := Math.Sqrt(nElem * (node2 - node1 * node1) / (nElem - 1));
		differ := (differ2 - 2.0 * node1 * differ1) * nElem / ((nElem - 1) * 2.0 * value)
	END EvaluateVD;

	PROCEDURE (node: SdNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphSumation.SdInstall"
	END Install;

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
