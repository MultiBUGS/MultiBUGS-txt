(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		  *)

MODULE GraphInprod;

	

	IMPORT
		Stores,
		GraphLogical, GraphMemory, GraphNodes, GraphRules, GraphStochastic;

	TYPE

		Node = POINTER TO RECORD(GraphMemory.Node)
			start0, start1, step0, step1, nElem: INTEGER;
			vector0, vector1: GraphNodes.Vector;
			constant0, constant1: POINTER TO ARRAY OF REAL;
			stoch0, stoch1: GraphStochastic.Vector
		END;

		Factory = POINTER TO RECORD(GraphMemory.Factory) END;

	VAR
		fact-: GraphNodes.Factory;
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
			p := node.vector0[off];
			f0 := GraphStochastic.ClassFunction(p, parent);
			off := node.start1 + i * node.step1;
			p := node.vector1[off];
			f1 := GraphStochastic.ClassFunction(p, parent);
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

	PROCEDURE (node: Node) ExternalizeMemory (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
			i, len: INTEGER;
	BEGIN
		v := GraphNodes.NewVector();
		v.components := node.vector0;
		v.start := node.start0; v.step := node.step0; v.nElem := node.nElem;
		GraphNodes.ExternalizeSubvector(v, wr);
		v.components := node.vector1;
		v.start := node.start1; v.step := node.step1; v.nElem := node.nElem;
		GraphNodes.ExternalizeSubvector(v, wr);
		IF node.constant0 # NIL THEN
			len := LEN(node.constant0);
			wr.WriteInt(len);
			i := 0; WHILE i < len DO wr.WriteReal(node.constant0[i]); INC(i) END
		ELSE
			wr.WriteInt(0)
		END;
		IF node.constant1 # NIL THEN
			len := LEN(node.constant1);
			wr.WriteInt(len);
			i := 0; WHILE i < len DO wr.WriteReal(node.constant1[i]); INC(i) END
		ELSE
			wr.WriteInt(0)
		END;
		IF node.stoch0 # NIL THEN
			len := LEN(node.stoch0);
			wr.WriteInt(len);
			i := 0;
			WHILE i < len DO GraphNodes.Externalize(node.stoch0[i], wr); INC(i) END
		ELSE
			wr.WriteInt(0)
		END;
		IF node.stoch1 # NIL THEN
			len := LEN(node.stoch1);
			wr.WriteInt(len);
			i := 0; WHILE i < len DO GraphNodes.Externalize(node.stoch1[i], wr); INC(i) END
		ELSE
			wr.WriteInt(0)
		END
	END ExternalizeMemory;

	PROCEDURE (node: Node) InternalizeMemory (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
			p: GraphNodes.Node;
			i, len: INTEGER;
	BEGIN
		GraphNodes.InternalizeSubvector(v, rd);
		node.vector0 := v.components;
		node.start0 := v.start; node.step0 := v.step;
		node.nElem := v.nElem;
		GraphNodes.InternalizeSubvector(v, rd);
		node.vector1 := v.components;
		node.start1 := v.start; node.step1 := v.step;
		rd.ReadInt(len);
		IF len > 0 THEN
			NEW(node.constant0, len);
			i := 0; WHILE i < len DO rd.ReadReal(node.constant0[i]); INC(i) END
		ELSE
			node.constant0 := NIL
		END;
		rd.ReadInt(len);
		IF len > 0 THEN
			NEW(node.constant1, len);
			i := 0; WHILE i < len DO rd.ReadReal(node.constant1[i]); INC(i) END
		ELSE
			node.constant1 := NIL
		END;
		rd.ReadInt(len);
		IF len > 0 THEN
			NEW(node.stoch0, len);
			i := 0;
			WHILE i < len DO
				p := GraphNodes.Internalize(rd);
				node.stoch0[i] := p(GraphStochastic.Node);
				INC(i)
			END
		ELSE
			node.stoch0 := NIL
		END;
		rd.ReadInt(len);
		IF len > 0 THEN
			NEW(node.stoch1, len);
			i := 0;
			WHILE i < len DO
				p := GraphNodes.Internalize(rd);
				node.stoch1[i] := p(GraphStochastic.Node);
				INC(i)
			END
		ELSE
			node.stoch1 := NIL
		END
	END InternalizeMemory;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.SetProps(node.props + {GraphLogical.dependent});
		node.vector0 := NIL;
		node.vector1 := NIL;
		node.constant0 := NIL;
		node.constant1 := NIL;
		node.stoch0 := NIL;
		node.stoch1 := NIL;
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
			off := node.start0 + i * node.step0;
			p := node.vector0[off];
			p.AddParent(list);
			off := node.start1 + i * node.step1;
			p := node.vector1[off];
			p.AddParent(list);
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
		node.stoch0 := NIL;
		node.stoch1 := NIL;
		node.constant0 := NIL;
		node.constant1 := NIL;
		WITH args: GraphStochastic.ArgsLogical DO
			ASSERT(args.vectors[0].components # NIL, 21);
			node.vector0 := args.vectors[0].components;
			ASSERT(args.vectors[1].components # NIL, 21);
			node.vector1 := args.vectors[1].components;
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
		nStoch0 := 0;
		nStoch1 := 0;
		nCon0 := 0;
		nCon1 := 0;
		WHILE i < nElem DO
			off := node.start0 + i * node.step0;
			p := node.vector0[off];
			IF p = NIL THEN res := {GraphNodes.nil, GraphNodes.arg1}; RETURN END;
			IF GraphNodes.data IN p.props THEN INC(nCon0) END;
			IF p IS GraphStochastic.Node THEN INC(nStoch0) END;
			off := node.start1 + i * node.step1;
			p := node.vector1[off];
			IF p = NIL THEN res := {GraphNodes.nil, GraphNodes.arg2}; RETURN END;
			IF GraphNodes.data IN p.props THEN INC(nCon1) END;
			IF p IS GraphStochastic.Node THEN INC(nStoch1) END;
			INC(i)
		END;
		IF nCon0 = nElem THEN
			NEW(node.constant0, nElem);
			i := 0;
			WHILE i < nElem DO
				off := node.start0 + i * node.step0;
				p := node.vector0[off];
				node.constant0[i] := p.Value();
				INC(i)
			END
		ELSIF nStoch0 = nElem THEN
			NEW(node.stoch0, nElem);
			i := 0;
			WHILE i < nElem DO
				off := node.start0 + i * node.step0;
				p := node.vector0[off];
				node.stoch0[i] := p(GraphStochastic.Node);
				INC(i)
			END
		END;
		IF nCon1 = nElem THEN
			NEW(node.constant1, nElem);
			i := 0;
			WHILE i < nElem DO
				off := node.start1 + i * node.step1;
				p := node.vector1[off];
				node.constant1[i] := p.Value();
				INC(i)
			END
		ELSIF nStoch1 = nElem THEN
			NEW(node.stoch1, nElem);
			i := 0;
			WHILE i < nElem DO
				off := node.start1 + i * node.step1;
				p := node.vector1[off];
				node.stoch1[i] := p(GraphStochastic.Node);
				INC(i)
			END
		END;
		IF (nCon0 = nElem) & (nCon1 = nElem) THEN
			node.SetProps(node.props + {GraphNodes.data})
		END
	END Set;

	PROCEDURE (node: Node) Evaluate (OUT value: REAL);
		VAR
			value0, value1: REAL;
			i, off, nElem: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		value := 0.0;
		i := 0;
		nElem := node.nElem;
		IF node.constant0 # NIL THEN
			IF node.constant1 # NIL THEN
				WHILE i < nElem DO value := value + node.constant0[i] * node.constant1[i]; INC(i) END
			ELSIF node.stoch1 # NIL THEN
				WHILE i < nElem DO value := value + node.constant0[i] * node.stoch1[i].value; INC(i) END
			ELSE
				WHILE i < nElem DO
					off := node.start1 + i * node.step1;
					p := node.vector1[off];
					value := value + node.constant0[i] * p.Value(); INC(i)
				END
			END
		ELSIF node.stoch0 # NIL THEN
			IF node.constant1 # NIL THEN
				WHILE i < nElem DO value := value + node.stoch0[i].value * node.constant1[i]; INC(i) END
			ELSIF node.stoch1 # NIL THEN
				WHILE i < nElem DO value := value + node.stoch0[i].value * node.stoch1[i].value; INC(i) END
			ELSE
				WHILE i < nElem DO
					off := node.start1 + i * node.step1;
					p := node.vector1[off];
					value := value + node.stoch0[i].value * p.Value(); INC(i)
				END
			END
		ELSE
			IF node.constant1 # NIL THEN
				WHILE i < nElem DO
					off := node.start0 + i * node.step0;
					p := node.vector0[off];
					value := value + p.Value() * node.constant1[i]; INC(i)
				END
			ELSIF node.stoch1 # NIL THEN
				WHILE i < nElem DO
					off := node.start0 + i * node.step0;
					p := node.vector0[off];
					value := value + p.Value() * node.stoch1[i].value; INC(i)
				END
			ELSE
				WHILE i < nElem DO
					off := node.start0 + i * node.step0;
					p := node.vector0[off];
					value0 := p.Value();
					off := node.start1 + i * node.step1;
					p := node.vector1[off];
					value1 := p.Value();
					value := value + value0 * value1; INC(i)
				END
			END
		END;
	END Evaluate;

	PROCEDURE (node: Node) EvaluateVD (x: GraphNodes.Node; OUT value, differ: REAL);
		VAR
			value0, value1, diff0, diff1: REAL;
			i, off, nElem: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		value := 0.0;
		differ := 0.0;
		i := 0;
		nElem := node.nElem;
		IF node.constant0 # NIL THEN
			IF node.constant1 # NIL THEN
				WHILE i < nElem DO value := value + node.constant0[i] * node.constant1[i]; INC(i) END
			ELSIF node.stoch1 # NIL THEN
				WHILE i < nElem DO 
					value := value + node.constant0[i] * node.stoch1[i].value; 
					IF node.stoch1[i] = x THEN differ := differ + node.constant0[i] END;
					INC(i) 
				END
			ELSE
				WHILE i < nElem DO
					off := node.start1 + i * node.step1;
					p := node.vector1[off];
					p.ValDiff(x, value1, diff1);
					value := value + node.constant0[i] * value1;
					differ := differ + node.constant0[i] * diff1;
					INC(i)
				END
			END
		ELSIF node.stoch0 # NIL THEN
			IF node.constant1 # NIL THEN
				WHILE i < nElem DO 
					value := value + node.stoch0[i].value * node.constant1[i]; 
					IF node.stoch0[i] = x THEN differ := differ + node.constant1[i] END;
					INC(i) 
				END
			ELSIF node.stoch1 # NIL THEN
				WHILE i < nElem DO 
					value := value + node.stoch0[i].value * node.stoch1[i].value; 
					IF node.stoch0[i] = x THEN differ := differ + node.stoch1[i].value END;
					IF node.stoch1[i] = x THEN differ := differ + node.stoch0[i].value END;
					INC(i) 
				END
			ELSE
				WHILE i < nElem DO
					off := node.start1 + i * node.step1;
					p := node.vector1[off];
					p.ValDiff(x, value1, diff1);
					value := value + node.stoch0[i].value * value1; 
					differ := differ + node.stoch0[i].value * diff1;
					INC(i)
				END
			END
		ELSE
			IF node.constant1 # NIL THEN
				WHILE i < nElem DO
					off := node.start0 + i * node.step0;
					p := node.vector0[off];
					p.ValDiff(x, value0, diff0);
					value := value + value0 * node.constant1[i]; 
					differ := differ + diff0 * node.constant1[i]; 
					INC(i)
				END
			ELSIF node.stoch1 # NIL THEN
				WHILE i < nElem DO
					off := node.start0 + i * node.step0;
					p := node.vector0[off];
					p.ValDiff(x, value0, diff0);
					value := value + value0 * node.stoch1[i].value;
					differ := differ + diff0 *  node.stoch1[i].value;
					IF node.stoch1[i] = x THEN differ := differ + value0 END;
					INC(i)
				END
			ELSE
				WHILE i < nElem DO
					off := node.start0 + i * node.step0;
					p := node.vector0[off];
					p.ValDiff(x, value0, diff0);
					off := node.start1 + i * node.step1;
					p := node.vector1[off];
					p.ValDiff(x, value1, diff1);
					value := value + value0 * value1; 
					differ := differ + value0 * diff1 + value1 * diff0;
					INC(i)
				END
			END
		END
	END EvaluateVD;

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
