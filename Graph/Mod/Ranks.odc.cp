(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphRanks;


	

	IMPORT
		Stores,
		GraphLogical, GraphNodes, GraphRules, GraphScalar, GraphStochastic, GraphVector,
		MathSort;

	TYPE

		RankedNode = POINTER TO RECORD(GraphScalar.MemNode)
			start, step, nElem: INTEGER;
			rank: GraphNodes.Node;
			vector: GraphNodes.Vector
		END;

		RankNode = POINTER TO RECORD(GraphScalar.MemNode)
			start, step, nElem: INTEGER;
			index: GraphNodes.Node;
			vector: GraphNodes.Vector
		END;

		SortNode = POINTER TO RECORD(GraphVector.Node)
			start, step: INTEGER;
			vector: GraphNodes.Vector
		END;

		RankedFactory = POINTER TO RECORD(GraphScalar.Factory) END;

		RankFactory = POINTER TO RECORD(GraphScalar.Factory) END;

		SortFactory = POINTER TO RECORD(GraphVector.Factory) END;

	CONST
		eps = 1.0E-20;

	VAR
		factRanked-, factRank-: GraphScalar.Factory;
		factSort-: GraphVector.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		workValues: POINTER TO ARRAY OF REAL;

	PROCEDURE (node: RankedNode) Check (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: RankedNode) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			i, form, off, nElem, start, step: INTEGER;
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
		IF form = GraphRules.const THEN
			form := GraphStochastic.ClassFunction(node.rank, parent)
		END;
		IF form # GraphRules.const THEN
			form := GraphRules.other
		END;
		RETURN form
	END ClassFunction;

	PROCEDURE (node: RankedNode) Evaluate (OUT value: REAL);
		VAR
			i, off, rank, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		nElem := node.nElem;
		start := node.start;
		step := node.step;
		i := 0;
		WHILE i < nElem DO
			off := start + i * step;
			p := node.vector[off];
			workValues[i] := p.Value();
			INC(i)
		END;
		rank := SHORT(ENTIER(node.rank.Value() + eps));
		value := MathSort.Ranked(workValues, rank, nElem)
	END Evaluate;

	PROCEDURE (node: RankedNode) EvaluateVD (x: GraphNodes.Node; OUT val, diff: REAL);
	BEGIN
		HALT(126)
	END EvaluateVD;

	PROCEDURE (node: RankedNode) ExternalizeScalar (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		v := GraphNodes.NewVector();
		v.components := node.vector;
		v.start := node.start; v.nElem := node.nElem; v.step := node.step;
		GraphNodes.ExternalizeSubvector(v, wr);
		GraphNodes.Externalize(node.rank, wr)
	END ExternalizeScalar;

	PROCEDURE (node: RankedNode) InternalizeScalar (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		GraphNodes.InternalizeSubvector(v, rd);
		node.start := v.start; node.nElem := v.nElem; node.step := v.step;
		node.vector := v.components;
		node.rank := GraphNodes.Internalize(rd)
	END InternalizeScalar;

	PROCEDURE (node: RankedNode) InitLogical;
	BEGIN
		node.SetProps(node.props + {GraphLogical.dependent});
		node.vector := NIL;
		node.start :=  - 1;
		node.nElem :=  - 1;
		node.step := 0;
		node.rank := NIL
	END InitLogical;

	PROCEDURE (node: RankedNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphRanks.RankedInstall"
	END Install;

	PROCEDURE (node: RankedNode) Parents (all: BOOLEAN): GraphNodes.List;
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
		p := node.rank;
		p.AddParent(list);
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: RankedNode) Set (IN args: GraphNodes.Args; OUT res: SET);
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
			ASSERT(args.scalars[0] # NIL, 21);
			node.rank := args.scalars[0];
			isData := GraphNodes.data IN node.rank.props;
			i := 0;
			WHILE i < nElem DO
				off := start + i * step; p := node.vector[off];
				IF p = NIL THEN
					res := {GraphNodes.nil, GraphNodes.arg1}; RETURN
				ELSE
					isData := isData & (GraphNodes.data IN p.props)
				END;
				INC(i)
			END;
			IF nElem >= LEN(workValues) THEN
				NEW(workValues, nElem + 1)
			END;
			IF isData THEN node.SetProps(node.props + {GraphNodes.data}) END
		END
	END Set;

	PROCEDURE (node: RankNode) Check (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: RankNode) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			i, f, form, off, nElem, start, step: INTEGER;
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
			f := GraphStochastic.ClassFunction(p, parent);
			INC(i)
		END;
		IF form = GraphRules.const THEN
			f := GraphStochastic.ClassFunction(node.index, parent)
		END;
		IF f # GraphRules.const THEN
			form := GraphRules.other
		END;
		RETURN form
	END ClassFunction;

	PROCEDURE (node: RankNode) Evaluate (OUT value: REAL);
		VAR
			i, iValue, off, index, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		nElem := node.nElem;
		start := node.start;
		step := node.step;
		index := SHORT(ENTIER(node.index.Value() - 1 + eps));
		off := start + index * step;
		p := node.vector[off];
		value := p.Value();
		i := 0;
		iValue := 1;
		WHILE i < index DO
			off := start + i * step;
			p := node.vector[off];
			IF p.Value() <= value THEN
				INC(iValue)
			END;
			INC(i)
		END;
		i := index + 1;
		WHILE i < nElem DO
			off := start + i * step;
			p := node.vector[off];
			IF p.Value() <= value THEN
				INC(iValue)
			END;
			INC(i)
		END;
		value := iValue
	END Evaluate;

	PROCEDURE (node: RankNode) EvaluateVD (x: GraphNodes.Node; OUT val, diff: REAL);
	BEGIN
		HALT(126)
	END EvaluateVD;

	PROCEDURE (node: RankNode) ExternalizeScalar (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		v := GraphNodes.NewVector();
		v.components := node.vector;
		v.start := node.start; v.nElem := node.nElem; v.step := node.step;
		GraphNodes.ExternalizeSubvector(v, wr);
		GraphNodes.Externalize(node.index, wr)
	END ExternalizeScalar;

	PROCEDURE (node: RankNode) InternalizeScalar (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		GraphNodes.InternalizeSubvector(v, rd);
		node.start := v.start; node.nElem := v.nElem; node.step := v.step;
		node.index := GraphNodes.Internalize(rd)
	END InternalizeScalar;

	PROCEDURE (node: RankNode) InitLogical;
	BEGIN
		node.SetProps(node.props + {GraphLogical.dependent});
		node.vector := NIL;
		node.start :=  - 1;
		node.nElem :=  - 1;
		node.step := 0;
		node.index := NIL
	END InitLogical;

	PROCEDURE (node: RankNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphRanks.RankInstall"
	END Install;

	PROCEDURE (node: RankNode) Parents (all: BOOLEAN): GraphNodes.List;
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
		p := node.index;
		p.AddParent(list);
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: RankNode) Set (IN args: GraphNodes.Args; OUT res: SET);
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
			ASSERT(args.scalars[0] # NIL, 21);
			node.index := args.scalars[0];
			isData := GraphNodes.data IN node.index.props;
			i := 0;
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
	END Set;

	PROCEDURE (node: SortNode) Check (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: SortNode) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			form, i, off, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		form := GraphRules.const;
		i := 0;
		nElem := node.Size();
		start := node.start;
		step := node.step;
		WHILE i < nElem DO
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

	PROCEDURE (node: SortNode) Evaluate (OUT values: ARRAY OF REAL);
		VAR
			i, off, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		i := 0;
		nElem := node.Size();
		start := node.start;
		step := node.step;
		WHILE i < nElem DO
			off := start + i * step;
			p := node.vector[off];
			values[i] := p.Value();
			INC(i)
		END;
		MathSort.HeapSort(values, nElem);
	END Evaluate;

	PROCEDURE (node: SortNode) ExternalizeVector (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		IF node.index = 0 THEN
			v := GraphNodes.NewVector();
			v.components := node.vector;
			v.start := node.start; v.nElem := node.Size(); v.step := node.step;
			GraphNodes.ExternalizeSubvector(v, wr)
		END
	END ExternalizeVector;

	PROCEDURE (node: SortNode) InternalizeVector (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
			i, nElem: INTEGER;
			p: SortNode;
	BEGIN
		IF node.index = 0 THEN
			GraphNodes.InternalizeSubvector(v, rd);
			node.vector := v.components;
			node.start := v.start; node.step := v.step;
		ELSE
			p := node.components[0](SortNode);
			node.vector := p.vector;
			node.start := p.start;
			node.step := p.step
		END
	END InternalizeVector;

	PROCEDURE (node: SortNode) InitLogical;
	BEGIN
		node.SetProps(node.props + {GraphLogical.dependent});
		node.vector := NIL;
		node.start :=  - 1;
		node.step := 0
	END InitLogical;

	PROCEDURE (node: SortNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphRanks.SortInstall"
	END Install;

	PROCEDURE (node: SortNode) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			i, off, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		i := 0;
		nElem := node.Size();
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

	PROCEDURE (node: SortNode) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			i, off, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
			isData: BOOLEAN;
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			nElem := node.Size();
			ASSERT(args.vectors[0].components # NIL, 21);
			node.vector := args.vectors[0].components;
			ASSERT(args.vectors[0].start >= 0, 21);
			node.start := args.vectors[0].start;
			node.step := args.vectors[0].step;
			ASSERT(args.vectors[0].nElem > 0, 21);
			IF nElem # args.vectors[0].nElem THEN
				res := {GraphNodes.length, GraphNodes.arg1};
				RETURN
			END;
			IF nElem > LEN(workValues) THEN
				NEW(workValues, nElem)
			END;
			isData := TRUE;
			i := 0;
			start := node.start;
			step := node.step;
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
	END Set;

	PROCEDURE (node: SortNode) ValDiff (x: GraphNodes.Node; OUT val, diff: REAL);
	BEGIN
		HALT(126)
	END ValDiff;

	PROCEDURE (f: RankedFactory) New (): GraphScalar.Node;
		VAR
			node: RankedNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: RankedFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vs"
	END Signature;

	PROCEDURE (f: RankFactory) New (): GraphScalar.Node;
		VAR
			node: RankNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: RankFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vs"
	END Signature;

	PROCEDURE (f: SortFactory) New (): GraphVector.Node;
		VAR
			node: SortNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: SortFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "v"
	END Signature;

	PROCEDURE RankedInstall*;
	BEGIN
		GraphNodes.SetFactory(factRanked)
	END RankedInstall;

	PROCEDURE RankInstall*;
	BEGIN
		GraphNodes.SetFactory(factRank)
	END RankInstall;

	PROCEDURE SortInstall*;
	BEGIN
		GraphNodes.SetFactory(factSort)
	END SortInstall;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		CONST
			len = 100;
		VAR
			fRanked: RankedFactory;
			fRank: RankFactory;
			fSort: SortFactory;
	BEGIN
		Maintainer;
		NEW(workValues, len);
		NEW(fRanked);
		factRanked := fRanked;
		NEW(fRank);
		factRank := fRank;
		NEW(fSort);
		factSort := fSort
	END Init;

BEGIN
	Init
END GraphRanks.
