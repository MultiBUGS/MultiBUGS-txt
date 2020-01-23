(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		 *)

MODULE GraphPiecewise;

	

	IMPORT
		Stores := Stores64,
		GraphDummy, GraphLogical, GraphNodes, GraphRules, GraphScalar, GraphStochastic;

	CONST
		trap = 55;

	TYPE
		Node = POINTER TO RECORD (GraphScalar.Node)
			components: POINTER TO ARRAY OF GraphNodes.Node;
			index, dummy: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD (GraphScalar.Factory) END;

	VAR
		fact-: GraphScalar.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 20 OF CHAR;

	PROCEDURE SetPiecewise* (node, index: GraphNodes.Node; num: INTEGER);
	BEGIN
		WITH node: Node DO
			ASSERT(index # NIL, trap); node.index := index;
			ASSERT(LEN(node.components) = num, trap)
		ELSE
		END
	END SetPiecewise;

	PROCEDURE (node: Node) Check (): SET;
		VAR a: SET;
	BEGIN
		a := {};
		RETURN a;
	END Check;

	PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			numComp, i, form: INTEGER;
			p: GraphNodes.Node;
			stochastic: GraphStochastic.Node;
	BEGIN
		stochastic := parent(GraphStochastic.Node);
		numComp := LEN(node.components);
		form := GraphRules.const;
		i := 0;
		WHILE (i < numComp) & (form = GraphRules.const) DO
			p := node.components[i]; form := GraphStochastic.ClassFunction(p, stochastic);
			IF form # GraphRules.const THEN form := GraphRules.other END;
			INC(i)
		END;
		RETURN form
	END ClassFunction;

	PROCEDURE (node: Node) ExternalizeScalar (VAR wr: Stores.Writer);
		VAR
			len, i: INTEGER;
	BEGIN
		GraphNodes.Externalize(node.index, wr);
		GraphNodes.Externalize(node.dummy, wr);
		len := LEN(node.components);
		wr.WriteInt(len);
		i := 0;
		WHILE i < len DO
			GraphNodes.Externalize(node.components[i], wr);
			INC(i);
		END;
	END ExternalizeScalar;

	PROCEDURE (node: Node) InternalizeScalar (VAR rd: Stores.Reader);
		VAR
			len, i: INTEGER;
	BEGIN
		node.index := GraphNodes.Internalize(rd);
		node.dummy := GraphNodes.Internalize(rd);
		rd.ReadInt(len);
		IF len > 0 THEN
			NEW(node.components, len);
		ELSE
			node.components := NIL;
		END;
		i := 0;
		WHILE i < len DO
			node.components[i] := GraphNodes.Internalize(rd);
			INC(i);
		END;
	END InternalizeScalar;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.dummy := NIL;
		node.index := NIL;
		node.components := NIL;
	END InitLogical;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphPiecewise.Install"
	END Install;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			numComp, i: INTEGER;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.dummy.AddParent(list);
		numComp := LEN(node.components);
		i := 0;
		WHILE i < numComp DO
			node.components[i].AddParent(list);
			INC(i);
		END;
		GraphNodes.ClearList(list);
		RETURN list;
	END Parents;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			len, numComp, i, off: INTEGER;
			argsS: GraphStochastic.Args;
	BEGIN
		res := {};
		node.dummy := GraphDummy.fact.New();
		(*	need to set node.dummy	*)
		argsS.Init;
		node.dummy.Set(argsS, res);
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.vectors[0].components # NIL, trap);
			len := LEN(args.vectors[0].components);
			numComp := args.vectors[0].nElem;
			ASSERT(numComp > 0, trap); 
			ASSERT(args.vectors[0].start >= 0, trap);
			ASSERT(args.vectors[0].step > 0, trap);
			NEW(node.components, numComp);
			i := 0;
			WHILE i < numComp DO
				off := args.vectors[0].start + i * args.vectors[0].step;
				ASSERT(len > off, trap);
				ASSERT(args.vectors[0].components[off] # NIL, trap);
				node.components[i] := args.vectors[0].components[off];
				INC(i)
			END
		END
	END Set;

	PROCEDURE (node: Node) Evaluate;
		CONST
			eps = 1.0E-6;
		VAR
			index: INTEGER;
	BEGIN
		IF node.index # NIL THEN
			index := SHORT(ENTIER(node.index.value + eps))
		ELSE
			index := 0
		END;
		node.value :=  node.components[index].value
	END Evaluate;

	PROCEDURE (node: Node) EvaluateDiffs;
	BEGIN
	END EvaluateDiffs;

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
		signature := "v";
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "---";
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
END GraphPiecewise.
