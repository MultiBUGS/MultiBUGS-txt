(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



		  *)

MODULE GraphInprod;

	

	IMPORT
		Stores := Stores64,
		GraphLogical, GraphNodes, GraphRules, GraphScalar, GraphStochastic;

	TYPE

		Node = POINTER TO RECORD(GraphScalar.Node)
			start0, start1, step0, step1, nElem: INTEGER;
			vector0, vector1: GraphNodes.Vector;
			constant0, constant1: POINTER TO ARRAY OF SHORTREAL;
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
			i, f0, f1, form, off, nElem: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		i := 0; nElem := node.nElem;
		WHILE i < nElem DO
			off := node.start0 + i * node.step0;
			IF node.vector0 # NIL THEN
				p := node.vector0[off];
				f0 := GraphStochastic.ClassFunction(p, parent)
			ELSE
				f0 := GraphRules.const
			END;
			IF node.vector1 # NIL THEN
				off := node.start1 + i * node.step1;
				p := node.vector1[off];
				f1 := GraphStochastic.ClassFunction(p, parent)
			ELSE
				f1 := GraphRules.const
			END;
			f1 := GraphRules.multF[f0, f1];
			IF i = 0 THEN
				form := f1
			ELSE
				form := GraphRules.addF[form, f1]
			END;
			INC(i)
		END;
		RETURN form
	END ClassFunction;

	PROCEDURE (node: Node) Evaluate;
		VAR
			value, value0, value1: REAL;
			i, off0, off1, nElem: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		value := 0.0;
		i := 0;
		nElem := node.nElem;
		IF node.constant0 # NIL THEN
			IF node.constant1 # NIL THEN
				WHILE i < nElem DO
					off0 := node.start0 + i * node.step0;
					off1 := node.start1 + i * node.step1;
					value := value + node.constant0[off0] * node.constant1[off1]; INC(i)
				END
			ELSE
				WHILE i < nElem DO
					off0 := node.start0 + i * node.step0;
					off1 := node.start1 + i * node.step1;
					p := node.vector1[off1];
					value := value + node.constant0[off0] * p.value; INC(i)
				END
			END
		ELSE
			IF node.constant1 # NIL THEN
				WHILE i < nElem DO
					off0 := node.start0 + i * node.step0;
					off1 := node.start1 + i * node.step1;
					p := node.vector0[off0];
					value := value + p.value * node.constant1[off1]; INC(i)
				END
			ELSE
				WHILE i < nElem DO
					off0 := node.start0 + i * node.step0;
					p := node.vector0[off0];
					value0 := p.value;
					off1 := node.start1 + i * node.step1;
					p := node.vector1[off1];
					value1 := p.value;
					value := value + value0 * value1; INC(i)
				END
			END
		END;
		node.value := value
	END Evaluate;

	PROCEDURE (node: Node) EvaluateDiffs;
		VAR
			value0, value1, diff0, diff1, N: REAL;
			i, j, off0, off1, nElem: INTEGER;
			p, p0, p1: GraphNodes.Node;
			x: GraphNodes.Vector;
	BEGIN
		x := node.diffWRT;
		N := LEN(x);
		j := 0;
		nElem := node.nElem;
		IF node.constant0 # NIL THEN
			IF node.constant1 = NIL THEN
				i := 0;
				WHILE i < N DO
					node.diffs[i] := 0.0;
					WHILE j < nElem DO
						off0 := node.start0 + j * node.step0;
						off1 := node.start1 + j * node.step1;
						p := node.vector1[off1];
						node.diffs[i] := node.diffs[i] + node.constant0[off0] * p.Diff(x[i]);
						INC(j)
					END;
					INC(i)
				END
			END
		ELSE
			IF node.constant1 # NIL THEN
				i := 0;
				WHILE i < N DO
					node.diffs[i] := 0.0;
					WHILE j < nElem DO
						off0 := node.start0 + j * node.step0;
						off1 := node.start1 + j * node.step1;
						p := node.vector0[off0];
						node.diffs[i] := node.diffs[i] + p.Diff(x[i]) * node.constant1[off1];
						INC(j)
					END;
					INC(i)
				END
			ELSE
				i := 0;
				WHILE i < N DO
					node.diffs[i] := 0.0;
					WHILE j < nElem DO
						off0 := node.start0 + j * node.step0;
						off1 := node.start1 + j * node.step1;
						p0 := node.vector0[off0];
						value0 := p0.value;
						p1 := node.vector1[off1];
						value1 := p1.value;
						node.diffs[i] := node.diffs[i] + value0 * p1.Diff(x[i]) + value1 * p0.Diff(x[i]);
						INC(j)
					END;
					INC(i)
				END
			END
		END;
	END EvaluateDiffs;

	PROCEDURE (node: Node) ExternalizeScalar (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
			i, len: INTEGER;
	BEGIN
		v.Init;
		v.components := node.vector0; v.values := node.constant0;
		v.start := node.start0; v.step := node.step0; v.nElem := node.nElem;
		GraphNodes.ExternalizeSubvector(v, wr);
		v.components := node.vector1; v.values := node.constant1;
		v.start := node.start1; v.step := node.step1; v.nElem := node.nElem;
		GraphNodes.ExternalizeSubvector(v, wr)
	END ExternalizeScalar;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.vector0 := NIL;
		node.vector1 := NIL;
		node.constant0 := NIL;
		node.constant1 := NIL;
		node.start0 := - 1;
		node.step0 := - 1;
		node.start1 := - 1;
		node.step1 := - 1;
		node.nElem := - 1
	END InitLogical;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphInprod.Install"
	END Install;

	PROCEDURE (node: Node) InternalizeScalar (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
			p: GraphNodes.Node;
			i, len: INTEGER;
	BEGIN
		GraphNodes.InternalizeSubvector(v, rd);
		node.vector0 := v.components; node.constant0 := v.values;
		node.start0 := v.start; node.step0 := v.step;
		node.nElem := v.nElem;
		GraphNodes.InternalizeSubvector(v, rd);
		node.vector1 := v.components; node.constant1 := v.values;
		node.start1 := v.start; node.step1 := v.step;
	END InternalizeScalar;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			i, off, nElem: INTEGER;
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		i := 0;
		nElem := node.nElem;
		list := NIL;
		WHILE i < nElem DO
			IF node.vector0 # NIL THEN
				off := node.start0 + i * node.step0;
				p := node.vector0[off];
				p.AddParent(list)
			END;
			IF node.vector1 # NIL THEN
				off := node.start1 + i * node.step1;
				p := node.vector1[off];
				p.AddParent(list)
			END;
			INC(i)
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			i, off, nElem, nStoch0, nStoch1, nCon0, nCon1: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		res := {};
		node.vector0 := NIL;
		node.vector1 := NIL;
		node.constant0 := NIL;
		node.constant1 := NIL;
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT((args.vectors[0].components # NIL) OR (args.vectors[0].values # NIL), 21);
			node.vector0 := args.vectors[0].components;
			node.constant0 := args.vectors[0].values;
			ASSERT((args.vectors[1].components # NIL) OR (args.vectors[1].values # NIL), 21);
			node.vector1 := args.vectors[1].components;
			node.constant1 := args.vectors[1].values;
			ASSERT(args.vectors[0].start >= 0, 21);
			node.start0 := args.vectors[0].start;
			node.step0 := args.vectors[0].step;
			ASSERT(args.vectors[1].start >= 0, 21);
			node.start1 := args.vectors[1].start;
			node.step1 := args.vectors[1].step;
			ASSERT(args.vectors[0].nElem > 0, 21);
			node.nElem := args.vectors[0].nElem;
			ASSERT(args.vectors[1].nElem > 0, 21);
			IF node.nElem # args.vectors[1].nElem THEN
				res := {GraphNodes.length, GraphNodes.arg2};
				RETURN
			END
		END;
		i := 0;
		nElem := node.nElem;
		nCon0 := 0;
		nCon1 := 0;
		WHILE i < nElem DO
			off := node.start0 + i * node.step0;
			IF node.vector0 # NIL THEN
				p := node.vector0[off];
				IF p = NIL THEN res := {GraphNodes.nil, GraphNodes.arg1}; RETURN END;
				IF GraphNodes.data IN p.props THEN INC(nCon0) END;
			ELSE
				IF node.constant0[off] = INF THEN res := {GraphNodes.nil, GraphNodes.arg1}; RETURN END;
				INC(nCon0)
			END;
			off := node.start1 + i * node.step1;
			IF node.vector1 # NIL THEN
				p := node.vector1[off];
				IF p = NIL THEN res := {GraphNodes.nil, GraphNodes.arg2}; RETURN END;
				IF GraphNodes.data IN p.props THEN INC(nCon1) END
			ELSE
				IF node.constant1[off] = INF THEN res := {GraphNodes.nil, GraphNodes.arg2}; RETURN END;
				INC(nCon1)
			END;
			INC(i)
		END;
		IF (nCon0 = nElem) & (nCon1 = nElem) THEN
			INCL(node.props, GraphNodes.data)
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
		signature := "vv"
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
END GraphInprod.
