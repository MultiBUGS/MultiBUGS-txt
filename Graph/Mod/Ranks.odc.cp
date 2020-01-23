(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphRanks;


	

	IMPORT
		Stores := Stores64,
		GraphLogical, GraphNodes, GraphRules, GraphScalar, GraphStochastic, GraphVector,
		MathSort;

	TYPE

		RankedNode = POINTER TO RECORD(GraphScalar.Node)
			start, step, nElem: INTEGER;
			rank: GraphNodes.Node;
			vector: GraphNodes.Vector;
			constant: POINTER TO ARRAY OF SHORTREAL
		END;

		RankNode = POINTER TO RECORD(GraphScalar.Node)
			start, step, nElem: INTEGER;
			index: GraphNodes.Node;
			vector: GraphNodes.Vector;
			constant: POINTER TO ARRAY OF SHORTREAL
		END;

		SortNode = POINTER TO RECORD(GraphVector.Node)
			start, step: INTEGER;
			vector: GraphNodes.Vector;
			constant: POINTER TO ARRAY OF SHORTREAL
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
			stochastic: GraphStochastic.Node;
	BEGIN
		stochastic := parent(GraphStochastic.Node);
		form := GraphRules.const;
		i := 0;
		nElem := node.nElem;
		start := node.start;
		step := node.step;
		WHILE (i < nElem) & (form = GraphRules.const) DO
			IF node.vector # NIL THEN
				off := start + i * step;
				p := node.vector[off];
				form := GraphStochastic.ClassFunction(p, stochastic)
			END;
			INC(i)
		END;
		IF form = GraphRules.const THEN
			form := GraphStochastic.ClassFunction(node.rank, stochastic)
		END;
		IF form # GraphRules.const THEN
			form := GraphRules.other
		END;
		RETURN form
	END ClassFunction;

	PROCEDURE (node: RankedNode) Evaluate;
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
			IF node.vector # NIL THEN
				p := node.vector[off];
				workValues[i] := p.value
			ELSE
				workValues[i] := node.constant[off]
			END;
			INC(i)
		END;
		rank := SHORT(ENTIER(node.rank.value + eps));
		node.value := MathSort.Ranked(workValues, rank, nElem)
	END Evaluate;

	PROCEDURE (node: RankedNode) EvaluateDiffs ;
	BEGIN
		HALT(126)
	END EvaluateDiffs;

	PROCEDURE (node: RankedNode) ExternalizeScalar (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		v.Init;
		v.components := node.vector; v.values := node.constant;
		v.start := node.start; v.nElem := node.nElem; v.step := node.step;
		GraphNodes.ExternalizeSubvector(v, wr);
		GraphNodes.Externalize(node.rank, wr)
	END ExternalizeScalar;

	PROCEDURE (node: RankedNode) InitLogical;
	BEGIN
		node.vector := NIL;
		node.constant := NIL;
		node.start := - 1;
		node.nElem := - 1;
		node.step := 0;
		node.rank := NIL
	END InitLogical;

	PROCEDURE (node: RankedNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphRanks.RankedInstall"
	END Install;

	PROCEDURE (node: RankedNode) InternalizeScalar (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		GraphNodes.InternalizeSubvector(v, rd);
		node.start := v.start; node.nElem := v.nElem; node.step := v.step;
		node.vector := v.components; node.constant := v.values;
		node.rank := GraphNodes.Internalize(rd)
	END InternalizeScalar;

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
			IF node.vector # NIL THEN
				off := start + i * step;
				p := node.vector[off];
				p.AddParent(list)
			END;
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
			ASSERT(args.scalars[0] # NIL, 21);
			node.rank := args.scalars[0];
			isData := GraphNodes.data IN node.rank.props;
			i := 0;
			WHILE i < nElem DO
				off := start + i * step;
				IF node.vector # NIL THEN
					p := node.vector[off];
					IF p = NIL THEN
						res := {GraphNodes.nil, GraphNodes.arg1}; RETURN
					ELSE
						isData := isData & (GraphNodes.data IN p.props)
					END
				ELSE
					IF node.constant[off] = INF THEN
						res := {GraphNodes.nil, GraphNodes.arg1}; RETURN
					END
				END;
				INC(i)
			END;
			IF nElem >= LEN(workValues) THEN
				NEW(workValues, nElem + 1)
			END;
			IF isData THEN INCL(node.props, GraphNodes.data) END
		END
	END Set;

	PROCEDURE (node: RankNode) Check (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: RankNode) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			i, form, off, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
			stochastic: GraphStochastic.Node;
	BEGIN
		stochastic := parent(GraphStochastic.Node);
		form := GraphRules.const;
		i := 0;
		nElem := node.nElem;
		start := node.start;
		step := node.step;
		IF node.vector # NIL THEN
			WHILE (i < nElem) & (form = GraphRules.const) DO
				off := start + i * step;
				p := node.vector[off];
				form := GraphStochastic.ClassFunction(p, stochastic);
				INC(i)
			END
		END;
		IF form = GraphRules.const THEN
			form := GraphStochastic.ClassFunction(node.index, stochastic)
		END;
		IF form # GraphRules.const THEN
			form := GraphRules.other
		END;
		RETURN form
	END ClassFunction;

	PROCEDURE (node: RankNode) Evaluate;
		VAR
			i, iValue, off, index, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
			val, value: REAL;
	BEGIN
		nElem := node.nElem;
		start := node.start;
		step := node.step;
		index := SHORT(ENTIER(node.index.value - 1 + eps));
		off := start + index * step;
		IF node.vector # NIL THEN
			p := node.vector[off];
			value := p.value
		ELSE
			value := node.constant[off]
		END;
		i := 0;
		iValue := 1;
		WHILE i < index DO
			off := start + i * step;
			IF node.vector # NIL THEN
				p := node.vector[off];
				val := p.value
			ELSE
				val := node.constant[off]
			END;
			IF val <= value THEN
				INC(iValue)
			END;
			INC(i)
		END;
		i := index + 1;
		WHILE i < nElem DO
			off := start + i * step;
			IF node.vector # NIL THEN
				p := node.vector[off];
				val := p.value
			ELSE
				val := node.constant[off]
			END;
			IF val <= value THEN
				INC(iValue)
			END;
			INC(i)
		END;
		node.value := iValue
	END Evaluate;

	PROCEDURE (node: RankNode) EvaluateDiffs ;
	BEGIN
		HALT(126)
	END EvaluateDiffs;

	PROCEDURE (node: RankNode) ExternalizeScalar (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		v.Init;
		v.components := node.vector; v.values := node.constant;
		v.start := node.start; v.nElem := node.nElem; v.step := node.step;
		GraphNodes.ExternalizeSubvector(v, wr);
		GraphNodes.Externalize(node.index, wr)
	END ExternalizeScalar;

	PROCEDURE (node: RankNode) InitLogical;
	BEGIN
		node.vector := NIL;
		node.constant := NIL;
		node.start := - 1;
		node.nElem := - 1;
		node.step := 0;
		node.index := NIL
	END InitLogical;

	PROCEDURE (node: RankNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphRanks.RankInstall"
	END Install;

	PROCEDURE (node: RankNode) InternalizeScalar (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		GraphNodes.InternalizeSubvector(v, rd);
		node.vector := v.components; node.constant := v.values;
		node.start := v.start; node.nElem := v.nElem; node.step := v.step;
		node.index := GraphNodes.Internalize(rd)
	END InternalizeScalar;

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
			IF node.vector # NIL THEN
				off := start + i * step;
				p := node.vector[off];
				p.AddParent(list)
			END;
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
			ASSERT(args.scalars[0] # NIL, 21);
			node.index := args.scalars[0];
			isData := GraphNodes.data IN node.index.props;
			i := 0;
			WHILE i < nElem DO
				off := start + i * step;
				IF node.vector # NIL THEN
					p := node.vector[off];
					IF p = NIL THEN
						res := {GraphNodes.nil, GraphNodes.arg1}; RETURN
					ELSE
						isData := isData & (GraphNodes.data IN p.props)
					END
				ELSE
					IF node.constant[off] = INF THEN
						res := {GraphNodes.nil, GraphNodes.arg1}; RETURN
					END
				END;
				INC(i)
			END;
			IF isData THEN INCL(node.props, GraphNodes.data) END
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
			stochastic: GraphStochastic.Node;
	BEGIN
		IF node.constant # NIL THEN RETURN GraphRules.const END;
		stochastic := parent(GraphStochastic.Node);
		form := GraphRules.const;
		i := 0;
		nElem := node.Size();
		start := node.start;
		step := node.step;
		WHILE i < nElem DO
			off := start + i * step;
			p := node.vector[off];
			form := GraphStochastic.ClassFunction(p, stochastic);
			INC(i)
		END;
		IF form # GraphRules.const THEN
			form := GraphRules.other
		END;
		RETURN form
	END ClassFunction;

	PROCEDURE (node: SortNode) Evaluate;
		VAR
			i, off, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
			values: POINTER TO ARRAY OF REAL;
	BEGIN
		i := 0;
		nElem := node.Size();
		start := node.start;
		step := node.step;
		NEW(values, nElem); (*	make global	*)
		WHILE i < nElem DO
			off := start + i * step;
			IF node.vector # NIL THEN
				p := node.vector[off];
				values[i] := p.value
			ELSE
				values[i] := node.constant[off]
			END;
			INC(i)
		END;
		MathSort.HeapSort(values, nElem);
		i := 0;
		WHILE i < nElem DO
			node.components[i].value := values[i]; INC(i)
		END
	END Evaluate;

	PROCEDURE (node: SortNode) EvaluateDiffs ;
	BEGIN
		HALT(126)
	END EvaluateDiffs;

	PROCEDURE (node: SortNode) ExternalizeVector (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		IF node.index = 0 THEN
			v.Init;
			v.components := node.vector; v.values := node.constant;
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
			node.vector := v.components; node.constant := v.values;
			node.start := v.start; node.step := v.step;
			i := 1;
			nElem := node.Size();
			WHILE i < nElem DO
				p := node.components[0](SortNode);
				p.vector := node.vector;
				p.constant := node.constant;
				p.start := node.start;
				p.step := node.step;
				INC(i)
			END
		END
	END InternalizeVector;

	PROCEDURE (node: SortNode) InitLogical;
	BEGIN
		node.vector := NIL;
		node.constant := NIL;
		node.start := - 1;
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
		IF node.constant # NIL THEN RETURN NIL END;
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
			ASSERT((args.vectors[0].components # NIL) OR (args.vectors[0].values # NIL), 21);
			node.vector := args.vectors[0].components;
			node.constant := args.vectors[0].values;
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
				IF node.vector # NIL THEN
					p := node.vector[off];
					IF p = NIL THEN
						res := {GraphNodes.nil, GraphNodes.arg1}; RETURN
					ELSE
						isData := isData & (GraphNodes.data IN p.props)
					END
				ELSE
					IF node.constant[off] = INF THEN
						res := {GraphNodes.nil, GraphNodes.arg1}; RETURN
					END
				END;
				INC(i)
			END;
			IF isData THEN INCL(node.props, GraphNodes.data) END
		END
	END Set;

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
