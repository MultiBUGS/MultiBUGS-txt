(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	  *)

MODULE GraphMixture;


	

	IMPORT
		Stores := Stores64,
		GraphLogical, GraphNodes, GraphRules, GraphScalar, GraphStochastic;

	CONST
		maxSlots = 4;

	TYPE

		Node = POINTER TO RECORD(GraphScalar.Node)
			start, slots: INTEGER;
			nElem, step: ARRAY maxSlots OF INTEGER;
			index, vector: GraphNodes.Vector
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
			class0, class, n, off, slots: INTEGER;
			i: ARRAY maxSlots OF INTEGER;
			p: GraphNodes.Node;
			stochastic: GraphStochastic.Node;
	BEGIN
		stochastic := parent(GraphStochastic.Node);
		class := GraphRules.const;
		n := 0;
		slots := node.slots;
		WHILE (n < slots) & (class # GraphRules.other) DO
			class := GraphStochastic.ClassFunction(node.index[n], stochastic);
			IF class # GraphRules.const THEN
				class := GraphRules.other
			END;
			INC(n)
		END;
		i[0] := 0;
		WHILE (i[0] < node.nElem[0]) & (class # GraphRules.other) DO
			i[1] := 0;
			WHILE (i[1] < node.nElem[1]) & (class # GraphRules.other) DO
				i[2] := 0;
				WHILE (i[2] < node.nElem[2]) & (class # GraphRules.other) DO
					i[3] := 0;
					WHILE (i[3] < node.nElem[3]) & (class # GraphRules.other) DO
						n := 0;
						off := node.start;
						WHILE n < slots DO
							off := off + i[n] * node.step[n];
							INC(n)
						END;
						p := node.vector[off];
						class0 := GraphStochastic.ClassFunction(p, stochastic);
						IF (i[0] = 0) & (i[1] = 0) THEN
							class := class0
						ELSE
							class := GraphRules.orF[class0, class]
						END;
						INC(i[3])
					END;
					INC(i[2])
				END;
				INC(i[1])
			END;
			INC(i[0])
		END;
		IF (class = GraphRules.logLink) OR (class = GraphRules.logitLink) THEN
			class := GraphRules.linkFun
		END;
		RETURN class
	END ClassFunction;

	PROCEDURE (node: Node) Evaluate;
		CONST
			eps = 1.0E-20;
		VAR
			i, index, off, slots: INTEGER;
			value: REAL;
			p, q: GraphNodes.Node;
	BEGIN
		i := 0;
		slots := node.slots;
		off := node.start;
		WHILE i < slots DO
			p := node.index[i];
			index := SHORT(ENTIER(p.value - 1 + eps));
			off := off + index * node.step[i];
			INC(i)
		END;
		q := node.vector[off];
		node.value := q.value;
	END Evaluate;

	PROCEDURE (node: Node) EvaluateDiffs ;
		CONST
			eps = 1.0E-20;
		VAR
			i,  index, off, slots, N: INTEGER;
			p, q: GraphNodes.Node;
			x: GraphNodes.Vector;
	BEGIN
		x := node.parents;
		N := LEN(x);
		i := 0;
		slots := node.slots;
		off := node.start;
		WHILE i < slots DO
			p := node.index[i];
			index := SHORT(ENTIER(p.value - 1 + eps));
			off := off + index * node.step[i];
			INC(i)
		END;
		q := node.vector[off];
		i := 0; WHILE i < N DO node.work[i] := q.Diff(x[i]); INC(i) END
	END EvaluateDiffs;

	PROCEDURE (node: Node) ExternalizeScalar (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
			i: INTEGER;
	BEGIN
		wr.WriteInt(node.slots);
		wr.WriteInt(node.start);
		v.Init;
		v.components := node.vector;
		v.start := 0; v.step := 1; v.nElem := LEN(node.vector);
		GraphNodes.ExternalizeSubvector(v, wr);
		i := 0;
		WHILE i < node.slots DO
			wr.WriteInt(node.nElem[i]);
			wr.WriteInt(node.step[i]);
			GraphNodes.Externalize(node.index[i], wr);
			INC(i)
		END
	END ExternalizeScalar;

	PROCEDURE (node: Node) InternalizeScalar (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
			i, slots: INTEGER;
	BEGIN
		NEW(node.index, maxSlots);
		i := 0;
		WHILE i < maxSlots DO
			node.step[i] := 0;
			node.nElem[i] := 1;
			node.index[i] := NIL;
			INC(i)
		END;
		rd.ReadInt(slots);
		node.slots := slots;
		rd.ReadInt(node.start);
		GraphNodes.InternalizeSubvector(v, rd);
		node.vector := v.components;
		i := 0;
		WHILE i < slots DO
			rd.ReadInt(node.nElem[i]);
			rd.ReadInt(node.step[i]);
			node.index[i] := GraphNodes.Internalize(rd);
			INC(i)
		END
	END InternalizeScalar;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.slots := 0;
		node.start := - 1;
		node.index := NIL;
		node.vector := NIL
	END InitLogical;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphMixture.Install"
	END Install;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			n, off, slots: INTEGER;
			i: ARRAY maxSlots OF INTEGER;
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		i[0] := 0;
		slots := node.slots;
		WHILE i[0] < node.nElem[0] DO
			i[1] := 0;
			WHILE i[1] < node.nElem[1] DO
				i[2] := 0;
				WHILE i[2] < node.nElem[2] DO
					i[3] := 0;
					WHILE i[3] < node.nElem[3] DO
						n := 0;
						off := node.start;
						WHILE n < slots DO
							off := off + i[n] * node.step[n];
							INC(n)
						END;
						p := node.vector[off];
						p.AddParent(list);
						INC(i[3])
					END;
					INC(i[2])
				END;
				INC(i[1])
			END;
			INC(i[0])
		END;
		n := 0;
		WHILE n < slots DO
			p := node.index[n];
			p.AddParent(list);
			INC(n)
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			i, slots: INTEGER;
	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			node.slots := args.numScalars;
			ASSERT(args.vectors[0].components # NIL, 21);
			node.vector := args.vectors[0].components;
			node.start := args.vectors[0].start;
			i := 0;
			slots := node.slots;
			WHILE i < slots DO
				ASSERT(args.ops[2 * i] > 0, 21);
				node.nElem[i] := args.ops[2 * i];
				ASSERT(args.ops[2 * i + 1] > 0, 21);
				node.step[i] := args.ops[2 * i + 1];
				ASSERT(args.scalars[i] # NIL, 21);
				node.index[i] := args.scalars[i];
				INC(i)
			END
		END
	END Set;

	PROCEDURE (f: Factory) New* (): GraphScalar.Node;
		VAR
			i: INTEGER;
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		NEW(node.index, maxSlots);
		i := 0;
		WHILE i < maxSlots DO
			node.step[i] := 0;
			node.nElem[i] := 1;
			node.index[i] := NIL;
			INC(i)
		END;
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := ""
	END Signature;

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

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

BEGIN
	Init
END GraphMixture.
