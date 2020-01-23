(*			(* PKBugs Version 2.0 *)



license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)


MODULE PharmacoSum;


	IMPORT
		Math, Meta, Stores := Stores64, Strings,
		BugsMsg,
		GraphConstant, GraphNodes,
		GraphRules, GraphScalar, GraphStochastic, PharmacoInputs;

	CONST
		trap = 77;

	TYPE
		Node = POINTER TO ABSTRACT RECORD (GraphScalar.Node)
			model, bio: GraphNodes.Vector;
			baseIV: BOOLEAN
		END;

		MemNode = POINTER TO ABSTRACT RECORD(Node) END;

		SumNode = POINTER TO RECORD (Node) END;
		LogNode = POINTER TO RECORD (Node) END;

		SumFactory = POINTER TO RECORD (GraphScalar.Factory) END;
		LogFactory = POINTER TO RECORD (GraphScalar.Factory) END;

	VAR
		factS-, factL-: GraphScalar.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) ExternalizeScalar- (VAR wr: Stores.Writer);
		VAR
			i, len: INTEGER;
	BEGIN
		IF node.model # NIL THEN len := LEN(node.model) ELSE len := 0 END;
		wr.WriteInt(len);
		i := 0;
		WHILE i < len DO
			GraphNodes.Externalize(node.model[i], wr); INC(i)
		END;
		IF node.bio # NIL THEN len := LEN(node.bio) ELSE len := 0 END;
		wr.WriteInt(len);
		i := 0;
		WHILE i < len DO
			GraphNodes.Externalize(node.bio[i], wr); INC(i)
		END;
		wr.WriteBool(node.baseIV)
	END ExternalizeScalar;

	PROCEDURE (node: Node) InternalizeScalar- (VAR rd: Stores.Reader);
		VAR
			i, len: INTEGER;
	BEGIN
		rd.ReadInt(len);
		IF len # 0 THEN NEW(node.model, len) ELSE node.model := NIL END;
		i := 0;
		WHILE i < len DO
			node.model[i] := GraphNodes.Internalize(rd); INC(i)
		END;
		rd.ReadInt(len);
		IF len # 0 THEN NEW(node.bio, len) ELSE node.bio := NIL END;
		i := 0;
		WHILE i < len DO
			node.bio[i] := GraphNodes.Internalize(rd); INC(i)
		END;
		rd.ReadBool(node.baseIV)
	END InternalizeScalar;

	PROCEDURE (node: Node) Check (): SET;
	BEGIN
		RETURN {}
	END Check;

	PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			f, form, i, len: INTEGER; p: GraphNodes.Node;
			stochastic: GraphStochastic.Node;
	BEGIN
		stochastic := parent(GraphStochastic.Node);
		form := GraphRules.const;
		len := LEN(node.model);
		i := 0;
		WHILE i < len DO
			p := node.model[i]; f := GraphStochastic.ClassFunction(p, stochastic);
			IF f # GraphRules.const THEN form := GraphRules.other END;
			IF node.bio[i] # NIL THEN
				p := node.bio[i]; f := GraphStochastic.ClassFunction(p, stochastic);
				IF f # GraphRules.const THEN form := GraphRules.other END
			END;
			INC(i)
		END;
		RETURN form
	END ClassFunction;

	PROCEDURE (node: Node) InitLogical;
	BEGIN
		node.model := NIL; node.bio := NIL;
		node.baseIV := TRUE
	END InitLogical;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			len, i: INTEGER; p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		len := LEN(node.model);
		list := NIL;
		i := 0;
		WHILE i < len DO
			p := node.model[i]; p.AddParent(list);
			IF node.bio[i] # NIL THEN
				p := node.bio[i]; p.AddParent(list)
			END;
			INC(i)
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;


	PROCEDURE Set (node: Node; IN args: GraphNodes.Args; OUT res: SET);
		CONST
			prefix = "PharmacoPK";
		VAR
			first, last, nComp, nLev, nRec, nDoses, pos, i, j,
			startLoop, off, resInt, inpInt, levInt, ssInt, len, nDisp: INTEGER;
			time, t: REAL;
			install0, install1, nString: ARRAY 80 OF CHAR;
			p: GraphNodes.Node;
			hist, theta: GraphNodes.SubVector;
			summary: PharmacoInputs.Summary;
			resInfo: PharmacoInputs.Reset;
			inpVar: PharmacoInputs.Input;
			argsL0, argsL1: GraphStochastic.ArgsLogical;
			fact: GraphNodes.Factory;

		PROCEDURE CheckPos;
			VAR
				let, net: REAL;
				ri, index: INTEGER;
		BEGIN
			PharmacoInputs.GetStart(resInfo, pos, startLoop, res); IF res # {} THEN RETURN END;
			ASSERT(pos >= startLoop, trap);
			PharmacoInputs.SetErrorOff(hist.start + pos * PharmacoInputs.nCol + PharmacoInputs.time);
			let := hist.components[hist.start + pos * PharmacoInputs.nCol + PharmacoInputs.time].value;
			IF time < let THEN (* pos and time arguments inconsistent *)
				res := {GraphNodes.arg4, GraphNodes.invalidValue}; RETURN
			END;
			IF pos < nRec - 1 THEN
				index := hist.start + (pos + 1) * PharmacoInputs.nCol;
				ASSERT(PharmacoInputs.IsInteger(hist.components[index + PharmacoInputs.reset]), trap);
				ri := PharmacoInputs.RealToInt(hist.components[index + PharmacoInputs.reset]);
				ASSERT((ri = 0) OR (ri = 1), trap);
				PharmacoInputs.SetErrorOff(index + PharmacoInputs.time);
				net := hist.components[index + PharmacoInputs.time].value;
				IF (ri = 0) & (time > net) THEN (* pos and time arguments inconsistent *)
					res := {GraphNodes.arg4, GraphNodes.invalidValue}; RETURN
				END
			END
		END CheckPos;

		PROCEDURE AddOmega;
			VAR
				q: GraphNodes.Node;
		BEGIN
			IF ssInt = 1 THEN
				PharmacoInputs.SetErrorOff(off + PharmacoInputs.omega);
				q := hist.components[off + PharmacoInputs.omega];
				(* dosing interval fields in dosing matrix must be constants when ss = 1 *)
				IF ~PharmacoInputs.IsConstant(q) THEN
					res := {GraphNodes.arg4, GraphNodes.notData}; RETURN
				END;
				IF q.value <= 0 THEN (* dosing interval values in dosing matrix must be > 0 *)
					res := {GraphNodes.arg4, GraphNodes.invalidValue}; RETURN
				END;
				argsL0.scalars[argsL0.numScalars] := q;
				INC(argsL0.numScalars); install0 := install0 + "ss"
			END
		END AddOmega;

		PROCEDURE AddConstants;
			VAR
				k: INTEGER; q: GraphNodes.Node;
		BEGIN
			k := 0;
			WHILE k < inpVar.nCon DO
				PharmacoInputs.SetErrorOff(off + PharmacoInputs.nKnown + k);
				q := hist.components[off + PharmacoInputs.nKnown + k];
				(* constant(s) missing for specified input(s) in dosing matrix *)
				IF ~PharmacoInputs.IsConstant(q) THEN
					res := {GraphNodes.arg4, GraphNodes.notData}; RETURN
				END;
				argsL0.scalars[argsL0.numScalars] := q; argsL1.scalars[argsL1.numScalars] := q;
				INC(argsL0.numScalars); INC(argsL1.numScalars);
				INC(k)
			END
		END AddConstants;

		PROCEDURE AddDisp;
			VAR
				k, comp, index: INTEGER;
		BEGIN
			WITH args: GraphStochastic.ArgsLogical DO
				k := 0;
				WHILE k < nDisp DO
					comp := nLev - 1 + k; ASSERT(comp < args.vectors[0].nElem, trap);
					index := args.vectors[0].start + comp * args.vectors[0].step; ASSERT(len > index, trap);
					theta.components[k] := args.vectors[0].components[index];
					INC(k)
				END
			END
		END AddDisp;

		PROCEDURE AddAbs;
			VAR
				k, comp, index: INTEGER;
		BEGIN
			WITH args: GraphStochastic.ArgsLogical DO
				k := 0;
				WHILE k < inpVar.nPar DO
					comp := nLev - 1 + nDisp + inpVar.cum + k; ASSERT(comp < args.vectors[0].nElem, trap);
					index := args.vectors[0].start + comp * args.vectors[0].step; ASSERT(len > index, trap);
					theta.components[nDisp + k] := args.vectors[0].components[index];
					INC(k)
				END
			END
		END AddAbs;

		PROCEDURE AddDose (IN install: ARRAY OF CHAR; IN argsL: GraphStochastic.ArgsLogical);
			VAR
				index: INTEGER;
				item: Meta.Item;
		BEGIN
			res := {};
			Meta.LookupPath(install, item);
			fact := GraphNodes.InstallFactory(install);
			node.model[j] := fact.New();
			node.model[j].Set(argsL, res); ASSERT(res = {}, trap); 	(* needs proper error handler *)
			IF levInt = first THEN
				node.bio[j] := NIL
			ELSE
				WITH args: GraphStochastic.ArgsLogical DO
					index := args.vectors[0].start + (levInt - first - 1) * args.vectors[0].step;
					ASSERT(len > index, trap);
					node.bio[j] := args.vectors[0].components[index]
				END
			END;
			INC(j)
		END AddDose;

		PROCEDURE AddImplied;
			VAR
				q: GraphNodes.Node;
				s, interval: REAL;
				addlInt, k: INTEGER;
		BEGIN
			q := hist.components[off + PharmacoInputs.addl];
			IF PharmacoInputs.IsConstant(q) THEN
				ASSERT(PharmacoInputs.IsInteger(q), trap);
				addlInt := PharmacoInputs.RealToInt(q);
				ASSERT(addlInt > 0, trap);
				q := hist.components[off + PharmacoInputs.omega];
				ASSERT(PharmacoInputs.IsConstant(q), trap);
				interval := q.value; ASSERT(interval > 0, trap);
				k := 0; s := t + interval;
				WHILE (k < addlInt) & (s <= time) DO
					argsL1.scalars[0] := GraphConstant.New(time - s);
					AddDose(install1, argsL1); IF res # {} THEN RETURN END;
					INC(k); s := s + interval
				END
			END
		END AddImplied;

	BEGIN
		res := {};
		WITH args: GraphStochastic.ArgsLogical DO
			hist := args.vectors[1];
			p := args.scalars[0];
			IF ~PharmacoInputs.IsConstant(p) THEN (* no. of compartments must be a constant *)
				res := {GraphNodes.arg1, GraphNodes.notData}; RETURN
			END;
			IF ~PharmacoInputs.IsInteger(p) THEN (* no. of compartments must be an integer *)
				res := {GraphNodes.arg1, GraphNodes.integer}; RETURN
			END;
			nComp := PharmacoInputs.RealToInt(p);
			Strings.IntToString(nComp, nString); nDisp := 2 * nComp;
			p := args.scalars[1];
			IF ~PharmacoInputs.IsConstant(p) THEN (* time argument must be a constant *)
				res := {GraphNodes.arg3, GraphNodes.notData}; RETURN
			END;
			time := p.value;
			p := args.scalars[2];
			IF ~PharmacoInputs.IsConstant(p) THEN (* pos argument must be a constant *)
				res := {GraphNodes.arg5, GraphNodes.notData}; RETURN
			END;
			IF ~PharmacoInputs.IsInteger(p) THEN (* pos argument must be an integer *)
				res := {GraphNodes.arg5, GraphNodes.integer}; RETURN
			END;
			nRec := hist.nElem DIV PharmacoInputs.nCol;
			pos := PharmacoInputs.RealToInt(p);
			(* pos argument must equal a row field from relevant section of dosing matrix *)
			IF (pos < 0) OR (pos >= nRec) THEN
				res := {GraphNodes.arg5, GraphNodes.invalidValue}; RETURN
			END;
			PharmacoInputs.GetSummary(hist, summary, resInfo, first, last, res);
			IF res # {} THEN RETURN END;
			IF ~PharmacoInputs.CheckTheta(args.vectors[0], summary, nComp) THEN
				res := {GraphNodes.arg2, GraphNodes.length}; RETURN
			END;
			len := LEN(args.vectors[0].components);
			node.baseIV := first = 0;
			nLev := LEN(summary);
			CheckPos; IF res # {} THEN RETURN END;
			PharmacoInputs.CountDoses(hist, startLoop, pos, time, nDoses, res);
			IF res # {} THEN RETURN END;
			NEW(node.model, nDoses); NEW(node.bio, nDoses);
			i := startLoop; j := 0;
			WHILE i <= pos DO
				argsL0.Init; argsL1.Init; argsL0.numVectors := 1; argsL1.numVectors := 1;
				off := hist.start + i * PharmacoInputs.nCol;
				ASSERT(PharmacoInputs.IsInteger(hist.components[off + PharmacoInputs.reset]), trap);
				resInt := PharmacoInputs.RealToInt(hist.components[off + PharmacoInputs.reset]);
				ASSERT(resInt = 0, trap);
				ASSERT(PharmacoInputs.IsInteger(hist.components[off + PharmacoInputs.input]), trap);
				inpInt := PharmacoInputs.RealToInt(hist.components[off + PharmacoInputs.input]);
				ASSERT(PharmacoInputs.IsInteger(hist.components[off + PharmacoInputs.level]), trap);
				levInt := PharmacoInputs.RealToInt(hist.components[off + PharmacoInputs.level]);
				t := hist.components[off + PharmacoInputs.time].value; ASSERT(time >= t, trap);
				inpVar := PharmacoInputs.IdToInput(inpInt, summary[levInt - first]); ASSERT(inpVar # NIL, trap);
				install0 := prefix + inpVar.name + nString; install1 := install0 + ".Install";
				argsL0.scalars[0] := GraphConstant.New(time - t);
				PharmacoInputs.SetErrorOff(off + PharmacoInputs.amount);
				p := hist.components[off + PharmacoInputs.amount];
				(* amount fields in dosing matrix must be constants when reset = 0 *)
				IF ~PharmacoInputs.IsConstant(p) THEN
					res := {GraphNodes.arg4, GraphNodes.notData}; RETURN
				END;
				IF p.value <= 0 THEN (* amount values in dosing matrix must be > 0 *)
					res := {GraphNodes.arg4, GraphNodes.invalidValue}; RETURN
				END;
				argsL0.scalars[1] := p; argsL1.scalars[1] := p;
				argsL0.numScalars := 2; argsL1.numScalars := 2;
				PharmacoInputs.SetErrorOff(off + PharmacoInputs.ss);
				p := hist.components[off + PharmacoInputs.ss];
				(* ss fields in dosing matrix must be constants when reset = 0 *)
				IF ~PharmacoInputs.IsConstant(p) THEN
					res := {GraphNodes.arg4, GraphNodes.notData}; RETURN
				END;
				(* ss fields in dosing matrix must be integers when reset = 0 *)
				IF ~PharmacoInputs.IsInteger(p) THEN
					res := {GraphNodes.arg4, GraphNodes.integer}; RETURN
				END;
				ssInt := PharmacoInputs.RealToInt(p);
				IF (ssInt # 0) & (ssInt # 1) THEN (* ss values in dosing matrix must be 0 or 1 *)
					res := {GraphNodes.arg4, GraphNodes.invalidValue}; RETURN
				END;
				AddOmega; IF res # {} THEN RETURN END;
				install0 := install0 + ".Install";
				AddConstants; IF res # {} THEN RETURN END;
				theta.Init;
				theta.start := 0; theta.nElem := nDisp + inpVar.nPar; theta.step := 1;
				NEW(theta.components, theta.nElem);
				AddDisp; AddAbs;
				argsL0.vectors[0] := theta; argsL1.vectors[0] := theta;
				AddDose(install0, argsL0); IF res # {} THEN RETURN END;
				AddImplied; IF res # {} THEN RETURN END;
				INC(i)
			END;
			GraphNodes.SetFactory(fact)
		END
	END Set;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			off, col, row: INTEGER;
			mes, intString: ARRAY 80 OF CHAR;
	BEGIN
		Set(node, args, res);
		IF GraphNodes.arg4 IN res THEN
			off := PharmacoInputs.errorOff;
			col := (off MOD PharmacoInputs.nCol) + 1;
			row := (off DIV PharmacoInputs.nCol) + 1;
			Strings.IntToString(col, intString);
			mes := " column " + intString + " row ";
			Strings.IntToString(row, intString);
			mes := mes + intString + " ";
			BugsMsg.StoreError(mes)
		END
	END Set;

	PROCEDURE (node: Node) EvaluateDiffs ;
	BEGIN
		HALT(126)
	END EvaluateDiffs;

	PROCEDURE (node: SumNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "PharmacoSum.SumInstall"
	END Install;

	PROCEDURE (node: SumNode) Evaluate;
		VAR
			len, i: INTEGER; sum, contrib, exp, F: REAL;
	BEGIN
		len := LEN(node.model);
		i := 0; sum := 0;
		WHILE i < len DO
			contrib := node.model[i].value;
			IF node.bio[i] # NIL THEN
				exp := Math.Exp(node.bio[i].value);
				IF node.baseIV THEN F := exp / (1 + exp) ELSE F := exp END;
				contrib := contrib * F
			END;
			sum := sum + contrib;
			INC(i)
		END;
		node.value := sum
	END Evaluate;

	PROCEDURE (node: LogNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "PharmacoSum.LogInstall"
	END Install;

	PROCEDURE (node: LogNode) Evaluate;
		CONST
			eps = 1.0E-40; logZero = -1.0E+10;
		VAR
			len, i: INTEGER; sum, contrib, exp, F: REAL;
	BEGIN
		len := LEN(node.model);
		i := 0; sum := 0;
		WHILE i < len DO
			contrib := node.model[i].value;
			IF node.bio[i] # NIL THEN
				exp := Math.Exp(node.bio[i].value);
				IF node.baseIV THEN F := exp / (1 + exp) ELSE F := exp END;
				contrib := contrib * F
			END;
			sum := sum + contrib;
			INC(i)
		END;
		IF sum > eps THEN node.value := Math.Ln(sum) ELSE node.value := logZero END
	END Evaluate;

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
		signature := "svsvs"
	END Signature;

	PROCEDURE (f: LogFactory) New (): GraphScalar.Node;
		VAR
			node: LogNode;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: LogFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "svsvs"
	END Signature;

	PROCEDURE SumInstall*;
	BEGIN
		GraphNodes.SetFactory(factS)
	END SumInstall;

	PROCEDURE LogInstall*;
	BEGIN
		GraphNodes.SetFactory(factL)
	END LogInstall;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "DLunn"
	END Maintainer;

	PROCEDURE Init;
		VAR
			fS: SumFactory;
			fL: LogFactory;
	BEGIN
		Maintainer;
		NEW(fS); factS := fS;
		NEW(fL); factL := fL
	END Init;

BEGIN
	Init
END PharmacoSum.
