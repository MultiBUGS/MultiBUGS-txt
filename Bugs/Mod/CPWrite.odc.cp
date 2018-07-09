(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

(*	
Each logical node is uniquely defined by the array of operators 'ops' in the data type GraphStochastic.ArgsLogical. This module writes out Component Pascal code to represent this unique stack of operators. The CP code consists of a module defining a new type of GraphScalar.Node plus a factory object to create nodes of this new type. Only one new module is written for each unique stack of operators.	*)

MODULE BugsCPWrite;


	

	IMPORT
		Strings,
		TextMappers,
		GraphGrammar, GraphNodes,
		GraphStochastic;


	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE StackSize (args: GraphStochastic.ArgsLogical): INTEGER;
		VAR
			i, maxTop, op, top: INTEGER;
	BEGIN
		i := 0;
		maxTop := 0;
		top := - 1;
		WHILE i < args.numOps DO
			op := args.ops[i];
			CASE op OF
			|GraphGrammar.ref, GraphGrammar.refStoch:
				INC(top); INC(i)
			|GraphGrammar.const:
				INC(top)
			|GraphGrammar.add, GraphGrammar.sub, GraphGrammar.mult, GraphGrammar.div,
				GraphGrammar.equals, GraphGrammar.max, GraphGrammar.min, GraphGrammar.pow:
				DEC(top)
			ELSE
			END;
			maxTop := MAX(top, maxTop);
			INC(i);
		END;
		RETURN maxTop + 1
	END StackSize;

	PROCEDURE WriteModuleHeader (module: ARRAY OF CHAR; VAR f: TextMappers.Formatter);
	BEGIN
		f.WriteString("MODULE "); f.WriteString(module); f.WriteChar(";"); f.WriteLn;
		f.WriteString("IMPORT"); f.WriteLn;
		f.WriteString("GraphLogical, GraphNodes, GraphRules, GraphScalar, GraphStochastic,  ");
		f.WriteLn;
		f.WriteString("Math, MathFunc, Stores;"); f.WriteLn;
		f.WriteLn
	END WriteModuleHeader;

	PROCEDURE WriteTypes (IN args: GraphStochastic.ArgsLogical; VAR f: TextMappers.Formatter);
		VAR
			i: INTEGER;
	BEGIN
		f.WriteString(" TYPE "); f.WriteLn;
		(*	node type	*)
		f.WriteString("Node = POINTER TO RECORD(GraphScalar.Node)"); f.WriteLn;
		IF args.numConsts > 0 THEN
			f.WriteString("c0");
			i := 1;
			WHILE i < args.numConsts DO
				f.WriteString(", c"); f.WriteInt(i); INC(i)
			END;
			f.WriteString(": REAL;"); f.WriteLn;
		END;
		IF args.numStochs > 0 THEN
			f.WriteString("s0");
			i := 1;
			WHILE i < args.numStochs DO
				f.WriteString(", s"); f.WriteInt(i); INC(i)
			END;
			f.WriteString(": GraphStochastic.Node;"); f.WriteLn;
		END;
		IF args.numLogicals > 0 THEN
			f.WriteString("l0");
			i := 1;
			WHILE i < args.numLogicals DO
				f.WriteString(", l"); f.WriteInt(i); INC(i)
			END;
			f.WriteString(": GraphLogical.Node;"); f.WriteLn;
		END;
		f.WriteString(" END;"); f.WriteLn;
		f.WriteLn;
		(*	write Factory stuff	*)
		f.WriteString("Factory = POINTER TO RECORD(GraphNodes.Factory) END;"); f.WriteLn;
		f.WriteLn;
	END WriteTypes;

	PROCEDURE WriteCheckMethod (VAR f: TextMappers.Formatter);
	BEGIN
		f.WriteString("PROCEDURE (node: Node) Check (): SET;"); f.WriteLn;
		f.WriteString("BEGIN"); f.WriteLn;
		f.WriteString("RETURN {}"); f.WriteLn;
		f.WriteString("END Check;"); f.WriteLn;
		f.WriteLn;
	END WriteCheckMethod;

	PROCEDURE WriteClassFunctionMethod (IN args: GraphStochastic.ArgsLogical;
	VAR f: TextMappers.Formatter);
		VAR
			i, op, stackSize, top: INTEGER;

		PROCEDURE WriteRefStoch (j: INTEGER);
		BEGIN
			INC(top);
			f.WriteString("p := node.s"); f.WriteInt(j); f. WriteString(";"); f.WriteLn;
			f.WriteString("class"); f.WriteInt(top);
			f.WriteString(" := GraphStochastic.ClassFunction(p, parent);"); f.WriteLn;
		END WriteRefStoch;

		PROCEDURE WriteRefLogical (j: INTEGER);
		BEGIN
			f.WriteString("p := node.l"); f.WriteInt(j); f.WriteString(";"); f.WriteLn;
			f.WriteString("l"); f.WriteInt(j);
			f.WriteString(" := GraphStochastic.ClassFunction(p, parent);"); f.WriteLn;
			f.WriteString("CASE l"); f.WriteInt(j); f.WriteString(" OF"); f.WriteLn;
			f.WriteString("GraphRules.logLink, GraphRules.logitLink,");
			f.WriteString("GraphRules.probitLink, GraphRules.cloglogLink:");
			f.WriteString("l"); f.WriteInt(j); f.WriteString(" := GraphRules.linkFun"); f.WriteLn;
			f.WriteString("ELSE"); f.WriteLn;
			f.WriteString("END;"); f.WriteLn
		END WriteRefLogical;

		PROCEDURE WriteRef (j: INTEGER);
		BEGIN
			INC(top);
			f.WriteString("class"); f.WriteInt(top);
			f.WriteString(" := l"); f.WriteInt(j); f.WriteString(";"); f.WriteLn;
		END WriteRef;

		PROCEDURE WriteConst;
		BEGIN
			INC(top); f.WriteString("class"); f.WriteInt(top); f.WriteString(" := GraphRules.const;"); f.WriteLn;
		END WriteConst;

		PROCEDURE WriteAdd;
		BEGIN
			DEC(top);
			f.WriteString("class"); f.WriteInt(top);
			f.WriteString(" := GraphRules.addF[class"); f.WriteInt(top); f.WriteString(" , class");
			f.WriteInt(top + 1); f.WriteString("];"); f.WriteLn;
		END WriteAdd;

		PROCEDURE WriteSub;
		BEGIN
			DEC(top);
			f.WriteString("class"); f.WriteInt(top);
			f.WriteString(" := GraphRules.subF[class"); f.WriteInt(top); f.WriteString(" , class");
			f.WriteInt(top + 1); f.WriteString("];"); f.WriteLn;
		END WriteSub;

		PROCEDURE WriteDiv;
		BEGIN
			DEC(top);
			f.WriteString("class"); f.WriteInt(top);
			f.WriteString(" := GraphRules.divF[class"); f.WriteInt(top); f.WriteString(" , class");
			f.WriteInt(top + 1); f.WriteString("];"); f.WriteLn;
		END WriteDiv;

		PROCEDURE WriteMult;
		BEGIN
			DEC(top);
			f.WriteString("class"); f.WriteInt(top);
			f.WriteString(" := GraphRules.multF[class"); f.WriteInt(top); f.WriteString(" , class");
			f.WriteInt(top + 1); f.WriteString("];"); f.WriteLn;
		END WriteMult;

		PROCEDURE WritePow;
		BEGIN
			DEC(top);
			f.WriteString("class"); f.WriteInt(top);
			f.WriteString(" := GraphRules.powF[class"); f.WriteInt(top); f.WriteString(" , class");
			f.WriteInt(top + 1); f.WriteString("];"); f.WriteLn;
		END WritePow;

		PROCEDURE WriteNonDiff2;
		BEGIN
			DEC(top);
			f.WriteString("class"); f.WriteInt(top);
			f.WriteString(" := GraphRules.nonDiff2F[class"); f.WriteInt(top); f.WriteString(" , class");
			f.WriteInt(top + 1); f.WriteString("];"); f.WriteLn;
		END WriteNonDiff2;

		PROCEDURE WriteUminus;
		BEGIN
			f.WriteString("class"); f.WriteInt(top); f.WriteString(" := GraphRules.uminusF[class");
			f.WriteInt(top); f.WriteString("];"); f.WriteLn
		END WriteUminus;

		PROCEDURE WriteExp;
		BEGIN
			f.WriteString("class"); f.WriteInt(top); f.WriteString(" := GraphRules.expF[class");
			f.WriteInt(top); f.WriteString("];"); f.WriteLn
		END WriteExp;

		PROCEDURE WriteLink;
		BEGIN
			f.WriteString("class"); f.WriteInt(top); f.WriteString(" := GraphRules.linkF[class");
			f.WriteInt(top); f.WriteString("];"); f.WriteLn
		END WriteLink;

		PROCEDURE WriteNonDiff1;
		BEGIN
			f.WriteString("class"); f.WriteInt(top); f.WriteString(" := GraphRules.nonDiff1F[class");
			f.WriteInt(top); f.WriteString("];"); f.WriteLn
		END WriteNonDiff1;

		PROCEDURE WriteOther;
		BEGIN
			f.WriteString("class"); f.WriteInt(top); f.WriteString(" := GraphRules.uopF[class");
			f.WriteInt(top); f.WriteString("];"); f.WriteLn
		END WriteOther;

	BEGIN
		stackSize := StackSize(args);
		f.WriteString("PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;");
		f.WriteLn;
		f.WriteString("VAR"); f.WriteLn;
		f.WriteString("class0");
		i := 1;
		WHILE i < stackSize DO
			f.WriteString(", class"); f.WriteInt(i); INC(i)
		END;
		f.WriteString(": INTEGER;"); f.WriteLn;
		IF args.numLogicals > 0 THEN
			f.WriteString("l0");
			i := 1;
			WHILE i < args.numLogicals DO
				f.WriteString(", l"); f.WriteInt(i); INC(i)
			END;
			f.WriteString(": INTEGER;"); f.WriteLn;
		END;
		f.WriteString("p: GraphNodes.Node;"); f.WriteLn;
		f.WriteString("BEGIN"); f.WriteLn;
		IF args.numLogicals > 0 THEN
			WriteRefLogical(0);
			i := 1;
			WHILE i < args.numLogicals DO
				WriteRefLogical(i);
				INC(i)
			END;
		END;
		i := 0;
		top := - 1;
		WHILE i < args.numOps DO
			op := args.ops[i];
			CASE op OF
			|GraphGrammar.refStoch: INC(i); WriteRefStoch(args.ops[i]);
			|GraphGrammar.ref: INC(i); WriteRef(args.ops[i]);
			|GraphGrammar.const: WriteConst
			|GraphGrammar.add: WriteAdd
			|GraphGrammar.sub: WriteSub
			|GraphGrammar.div: WriteDiv
			|GraphGrammar.mult: WriteMult
			|GraphGrammar.pow: WritePow
			|GraphGrammar.max, GraphGrammar.min, GraphGrammar.equals: WriteNonDiff2
			|GraphGrammar.uminus: WriteUminus
			|GraphGrammar.exp: WriteExp
			|GraphGrammar.icloglog, GraphGrammar.ilogit, GraphGrammar.phi: WriteLink
			|GraphGrammar.abs, GraphGrammar.step: WriteNonDiff1
			ELSE WriteOther
			END;
			INC(i);
		END;
		f.WriteString("RETURN class0"); f.WriteLn;
		f.WriteString("END ClassFunction;"); f.WriteLn;
		f.WriteLn;
	END WriteClassFunctionMethod;

	PROCEDURE WriteExternalizeLogicalMethod (IN args: GraphStochastic.ArgsLogical; VAR f: TextMappers.Formatter);
		VAR
			i: INTEGER;
	BEGIN
		f.WriteString("PROCEDURE (node: Node) ExternalizeLogical (VAR wr: Stores.Writer);"); f.WriteLn;
		f.WriteString("BEGIN"); f.WriteLn;
		i := 0;
		WHILE i < args.numConsts DO
			f.WriteString("wr.WriteReal(node.c"); f.WriteInt(i); f.WriteString(");"); f.WriteLn;
			INC(i)
		END;
		i := 0;
		WHILE i < args.numLogicals DO
			f.WriteString("GraphNodes.Externalize(node.l"); f.WriteInt(i); f.WriteString(", wr);"); f.WriteLn;
			INC(i)
		END;
		i := 0;
		WHILE i < args.numStochs DO
			f.WriteString("GraphNodes.Externalize(node.s"); f.WriteInt(i); f.WriteString(", wr);"); f.WriteLn;
			INC(i)
		END;
		f.WriteString("END ExternalizeLogical;"); f.WriteLn;
		f.WriteLn;
	END WriteExternalizeLogicalMethod;

	PROCEDURE WriteInitLogicalMethod (VAR f: TextMappers.Formatter);
	BEGIN
		f.WriteString("PROCEDURE (node: Node) InitLogical;"); f.WriteLn;
		f.WriteString("BEGIN"); f.WriteLn;
		f.WriteString("END InitLogical;"); f.WriteLn;
		f.WriteLn;
	END WriteInitLogicalMethod;

	PROCEDURE WriteInstallMethod (module: ARRAY OF CHAR; VAR f: TextMappers.Formatter);
	BEGIN
		f.WriteString("PROCEDURE (node: Node) Install(OUT install: ARRAY OF CHAR);"); f.WriteLn;
		f.WriteString("BEGIN"); f.WriteLn;
		f.WriteString("install := '" + module + ".Install'"); f.WriteLn;
		f.WriteString("END Install;"); f.WriteLn;
		f.WriteLn;
	END WriteInstallMethod;

	PROCEDURE WriteInternalizeLogicalMethod (IN args: GraphStochastic.ArgsLogical;
	VAR f: TextMappers.Formatter);
		VAR
			i: INTEGER;
	BEGIN
		f.WriteString("PROCEDURE (node: Node) InternalizeLogical (VAR rd: Stores.Reader);"); f.WriteLn;
		f.WriteString("VAR"); f.WriteLn;
		f.WriteString("p: GraphNodes.Node;"); f.WriteLn;
		f.WriteString("BEGIN"); f.WriteLn;
		i := 0;
		WHILE i < args.numConsts DO
			f.WriteString("rd.ReadReal(node.c"); f.WriteInt(i); f.WriteString(");"); f.WriteLn;
			INC(i)
		END;
		i := 0;
		WHILE i < args.numLogicals DO
			f.WriteString("p := GraphNodes.Internalize(rd);"); f.WriteLn;
			f.WriteString("node.l"); f.WriteInt(i); f.WriteString(" := p(GraphLogical.Node);"); f.WriteLn;
			INC(i)
		END;
		i := 0;
		WHILE i < args.numStochs DO
			f.WriteString("p := GraphNodes.Internalize(rd);"); f.WriteLn;
			f.WriteString("node.s"); f.WriteInt(i); f.WriteString(" := p(GraphStochastic.Node);"); f.WriteLn;
			INC(i)
		END;
		f.WriteString("END InternalizeLogical;"); f.WriteLn;
		f.WriteLn;
	END WriteInternalizeLogicalMethod;

	PROCEDURE WriteParentsMethod (IN args: GraphStochastic.ArgsLogical;
	VAR f: TextMappers.Formatter);
		VAR
			i: INTEGER;
	BEGIN
		f.WriteString("PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;"); f.WriteLn;
		f.WriteString("VAR"); f.WriteLn;
		f.WriteString("list: GraphNodes.List;"); f.WriteLn;
		f.WriteString("BEGIN"); f.WriteLn;
		f.WriteString("list := NIL;"); f.WriteLn;
		IF args.numLogicals > 0 THEN
			f.WriteString("node.l0.AddParent(list);"); f.WriteLn;
			i := 1;
			WHILE i < args.numLogicals DO
				f.WriteString("node.l"); f.WriteInt(i); f.WriteString(".AddParent(list);"); f.WriteLn;
				INC(i)
			END
		END;
		IF args.numStochs > 0 THEN
			f.WriteString("node.s0.AddParent(list);"); f.WriteLn;
			i := 1;
			WHILE i < args.numStochs DO
				f.WriteString("node.s"); f.WriteInt(i); f.WriteString(".AddParent(list);"); f.WriteLn;
				INC(i)
			END
		END;
		f.WriteString("GraphNodes.ClearList(list);"); f.WriteLn;
		f.WriteString("   RETURN list"); f.WriteLn;
		f.WriteString("END Parents;"); f.WriteLn;
		f.WriteLn;
	END WriteParentsMethod;

	PROCEDURE WriteSetMethod (IN args: GraphStochastic.ArgsLogical; VAR f: TextMappers.Formatter);
		VAR
			i: INTEGER;
	BEGIN
		f.WriteString("PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);"); f.WriteLn;
		f.WriteString("VAR"); f.WriteLn;
		f.WriteString("i: INTEGER;"); f.WriteLn;
		f.WriteString("logicalArgs: GraphStochastic.ArgsLogical;"); f.WriteLn;
		f.WriteString("BEGIN"); f.WriteLn;
		f.WriteString("res := {};"); f.WriteLn;
		f.WriteString("logicalArgs := args(GraphStochastic.ArgsLogical);"); f.WriteLn;
		f.WriteString("ASSERT(logicalArgs.numOps > 0, 20);"); f.WriteLn;
		f.WriteString("ASSERT(logicalArgs.numScalars = 0, 21);"); f.WriteLn;
		IF args.numStochs > 0 THEN
			f.WriteString("i := 0;"); f.WriteLn;
			f.WriteString("WHILE i < logicalArgs.numStochs DO"); f.WriteLn;
			f.WriteString("IF logicalArgs.stochastics[i] = NIL THEN"); f.WriteLn;
			f.WriteString("res := {GraphNodes.nil, GraphNodes.lhs}; RETURN"); f.WriteLn;
			f.WriteString("END;"); f.WriteLn;
			f.WriteString("INC(i)"); f.WriteLn;
			f.WriteString("END;"); f.WriteLn;
		END;
		IF args.numLogicals > 0 THEN
			f.WriteString("i := 0;"); f.WriteLn;
			f.WriteString("WHILE i < logicalArgs.numLogicals DO"); f.WriteLn;
			f.WriteString("IF logicalArgs.logicals[i] = NIL THEN"); f.WriteLn;
			f.WriteString("res := {GraphNodes.nil, GraphNodes.lhs}; RETURN"); f.WriteLn;
			f.WriteString("END;"); f.WriteLn;
			f.WriteString("INC(i)"); f.WriteLn;
			f.WriteString("END;"); f.WriteLn;
		END;
		i := 0; ;
		WHILE i < args.numConsts DO
			f.WriteString("node.c"); f.WriteInt(i); f.WriteString(":= logicalArgs.consts[");
			f.WriteInt(i); f.WriteString("];"); f.WriteLn;
			INC(i)
		END;
		i := 0;
		WHILE i < args.numLogicals DO
			f.WriteString("node.l"); f.WriteInt(i); f.WriteString(" := logicalArgs.logicals[");
			f.WriteInt(i); f.WriteString("];"); f.WriteLn;
			INC(i)
		END;
		i := 0;
		WHILE i < args.numStochs DO
			f.WriteString("node.s"); f.WriteInt(i); f.WriteString(" := logicalArgs.stochastics[");
			f.WriteInt(i); f.WriteString("];"); f.WriteLn;
			INC(i)
		END;
		f.WriteString("END Set;"); f.WriteLn;
		f.WriteLn;
	END WriteSetMethod;

	PROCEDURE WriteFormula (IN args: GraphStochastic.ArgsLogical; VAR f: TextMappers.Formatter);

		TYPE
			Tree = POINTER TO RECORD
				op, label: INTEGER;
				left, right: Tree
			END;

		VAR
			i, numC, stackSize, top: INTEGER;
			trees: POINTER TO ARRAY OF Tree;
			t: Tree;

		PROCEDURE String (IN s: ARRAY OF CHAR);
		BEGIN
			f.WriteString(s)
		END String;

		(*	write CP code from tree	*)
		PROCEDURE Write (t: Tree);
		BEGIN
			CASE t.op OF
			|GraphGrammar.refStoch: String("s"); f.WriteInt(t.label)
			|GraphGrammar.ref: String("l"); f.WriteInt(t.label)
			|GraphGrammar.const: String("node.c"); f.WriteInt(t.label)
			|GraphGrammar.abs: String("ABS("); Write(t.left); String(")")
			|GraphGrammar.add: String("("); Write(t.left); String(" + "); Write(t.right); String(")")
			|GraphGrammar.sub: String("("); Write(t.left); String(" - "); Write(t.right); String(")")
			|GraphGrammar.mult: String("("); Write(t.left); String(" * "); Write(t.right); String(")")
			|GraphGrammar.div: String("("); Write(t.left); String(" / "); Write(t.right); String(")")
			|GraphGrammar.uminus: String("(-("); Write(t.left); String("))")
			|GraphGrammar.cloglog: String("Math.Ln(-Math.Ln(1.0 - "); Write(t.left); String("));")
			|GraphGrammar.icloglog: String("(1.0 - Math.Exp( - Math.Exp(value[top])))");
			|GraphGrammar.cos: String("Math.Cos("); Write(t.left); String(")")
			|GraphGrammar.equals:
				String("MathFunc.Equals("); Write(t.left); String(", "); Write(t.right); String(")")
			|GraphGrammar.exp: String("Math.Exp("); Write(t.left); String(")")
			|GraphGrammar.log: String("Math.Ln("); Write(t.left); String(")")
			|GraphGrammar.logfact:
				String("MathFunc.LogFactorial(SHORT(ENTIER("); Write(t.left); String(" + eps)))")
			|GraphGrammar.loggam: String("MathFunc.LogGammaFunc("); Write(t.left); String(")")
			|GraphGrammar.logit: String("MathFunc.Logit("); Write(t.left); String(")")
			|GraphGrammar.ilogit: String("MathFunc.ILogit("); Write(t.left); String(")")
			|GraphGrammar.max: String("MAX("); Write(t.left); String(", "); Write(t.right); String(")")
			|GraphGrammar.min: String("MIN("); Write(t.left); String(", "); Write(t.right); String(")")
			|GraphGrammar.phi: String("MathFunc.Phi("); Write(t.left); String(")")
			|GraphGrammar.pow: String("MathFunc.Power("); Write(t.left); String(", "); Write(t.right); String(")")
			|GraphGrammar.round: String("Math.Round("); Write(t.left); String(")")
			|GraphGrammar.sin: String("Math.Sin("); Write(t.left); String(")")
			|GraphGrammar.sqrt: String("Math.Sqrt("); Write(t.left); String(")")
			|GraphGrammar.step: String("MathFunc.Step("); Write(t.left); String(")")
			|GraphGrammar.trunc: String("Math.Trunc("); Write(t.left); String(")")
			|GraphGrammar.tan: String("Math.Tan("); Write(t.left); String(")")
			|GraphGrammar.arcsin: String("Math.ArcSin("); Write(t.left); String(")")
			|GraphGrammar.arccos: String("Math.ArcCos("); Write(t.left); String(")")
			|GraphGrammar.arctan: String("Math.ArcTan("); Write(t.left); String(")")
			|GraphGrammar.sinh: String("Math.Sinh("); Write(t.left); String(")")
			|GraphGrammar.cosh: String("Math.Cosh("); Write(t.left); String(")")
			|GraphGrammar.tanh: String("Math.Tanh("); Write(t.left); String(")")
			|GraphGrammar.arcsinh: String("Math.ArcSinh("); Write(t.left); String(")")
			|GraphGrammar.arccosh: String("Math.ArcCosh("); Write(t.left); String(")")
			|GraphGrammar.arctanh: String("Math.ArcTanh("); Write(t.left); String(")")
			END;
		END Write;

	BEGIN
		(*	build tree representation of stack	*)
		numC := 0;
		top := - 1;
		i := 0;
		stackSize := StackSize(args);
		NEW(trees, stackSize);
		WHILE i < args.numOps DO
			NEW(t);
			t.left := NIL; t.right := NIL;
			t.op := args.ops[i];
			CASE t.op OF
			|GraphGrammar.refStoch, GraphGrammar.ref:
				INC(i); t.label := args.ops[i]; INC(top); trees[top] := t
			|GraphGrammar.const:
				t.label := numC; INC(numC); INC(top); trees[top] := t
			|GraphGrammar.add, GraphGrammar.sub, GraphGrammar.mult, GraphGrammar.div,
				GraphGrammar.equals, GraphGrammar.max, GraphGrammar.min, GraphGrammar.pow:
				DEC(top); t.left := trees[top]; t.right := trees[top + 1]; trees[top] := t;
			ELSE
				t.left := trees[top]; trees[top] := t
			END;
			INC(i)
		END;
		(*	write CP code for tree	*)
		Write(trees[0])
	END WriteFormula;

	PROCEDURE WriteValueMethod (IN args: GraphStochastic.ArgsLogical; VAR f: TextMappers.Formatter);
		VAR
			i: INTEGER;
	BEGIN
		f.WriteString("PROCEDURE (node: Node) Value (): REAL;"); f.WriteLn;
		f.WriteString("VAR "); f.WriteLn;
		f.WriteString("value: REAL; "); f.WriteLn;
		IF args.numLogicals > 0 THEN
			f.WriteString("l0");
			i := 1;
			WHILE i < args.numLogicals DO
				f.WriteString(", l"); f.WriteInt(i); INC(i)
			END;
			f.WriteString(": REAL;"); f.WriteLn;
		END;
		IF args.numStochs > 0 THEN
			f.WriteString("s0");
			i := 1;
			WHILE i < args.numStochs DO
				f.WriteString(", s"); f.WriteInt(i); INC(i)
			END;
			f.WriteString(": REAL;"); f.WriteLn;
		END;
		f.WriteString(" BEGIN "); f.WriteLn;
		IF args.numLogicals > 0 THEN
			f.WriteString("l0 := node.l0.Value();"); f.WriteLn;
			i := 1;
			WHILE i < args.numLogicals DO
				f.WriteString("l"); f.WriteInt(i); f.WriteString(" := node.l"); f.WriteInt(i);
				f.WriteString(".Value();"); f.WriteLn;
				INC(i)
			END;
		END;
		IF args.numStochs > 0 THEN
			f.WriteString("s0 := node.s0.value;"); f.WriteLn;
			i := 1;
			WHILE i < args.numStochs DO
				f.WriteString("s"); f.WriteInt(i); f.WriteString(" := node.s"); f.WriteInt(i);
				f.WriteString(".value;"); f.WriteLn;
				INC(i)
			END;
		END;
		f.WriteString("value := ");
		WriteFormula(args, f);
		f.WriteChar(";"); f.WriteLn;
		f.WriteString("   RETURN value"); f.WriteLn;
		f.WriteString(" END Value;"); f. WriteLn;
		f. WriteLn;
	END WriteValueMethod;

	PROCEDURE WriteValDiffMethod (IN args: GraphStochastic.ArgsLogical;
	VAR f: TextMappers.Formatter);

		VAR
			i, numC, op, stackSize, top: INTEGER;

		PROCEDURE String (IN x: ARRAY OF CHAR);
		BEGIN
			f.WriteString(x)
		END String;

		PROCEDURE Comment (IN x: ARRAY OF CHAR);
		BEGIN
			String(" (* " + x + " *)")
		END Comment;

		PROCEDURE If;
		BEGIN
			String("IF ")
		END If;

		PROCEDURE Then;
		BEGIN
			String(" THEN ")
		END Then;

		PROCEDURE Else;
		BEGIN
			String(" ELSE ")
		END Else;

		PROCEDURE End;
		BEGIN
			String(" END; ")
		END End;

		PROCEDURE Val;
		BEGIN
			String("val"); f.WriteInt(top)
		END Val;

		PROCEDURE Val1;
		BEGIN
			String("val"); f.WriteInt(top + 1)
		END Val1;

		PROCEDURE Diff;
		BEGIN
			String("diff"); f.WriteInt(top)
		END Diff;

		PROCEDURE Diff1;
		BEGIN
			String("diff"); f.WriteInt(top + 1)
		END Diff1;

		PROCEDURE Active;
		BEGIN
			String("active"); f.WriteInt(top)
		END Active;

		PROCEDURE Active1;
		BEGIN
			String("active"); f.WriteInt(top + 1)
		END Active1;

		PROCEDURE Ln;
		BEGIN
			f.WriteLn
		END Ln;

		PROCEDURE Assert;
		BEGIN
			String("ASSERT(~"); Active; String(", 0);"); Ln;
		END Assert;

		PROCEDURE Assert1;
		BEGIN
			String("ASSERT(~"); Active1; String(", 0);"); Ln;
		END Assert1;

		PROCEDURE WriteRefStoch (j: INTEGER);
		BEGIN
			INC(top);
			Val; String(" := node.s"); f.WriteInt(j); String(".value;"); Comment("stochastic"); Ln;
			Active; String(" := node.s"); f.WriteInt(j); String(" = x;"); Ln;
			If; Active; Then;
			Diff; String(" := 1.0 ");
			Else;
			Diff; String(" := 0.0 ");
			End; Ln
		END WriteRefStoch;

		PROCEDURE WriteRefLogical (j: INTEGER);
		BEGIN
			String("node.l"); f.WriteInt(j); String(".ValDiff(x, l"); f.WriteInt(j); String(", d"); f.WriteInt(j); String(");");
			Comment("logical intermediate"); Ln
		END WriteRefLogical;

		PROCEDURE WriteRef (j: INTEGER);
		BEGIN
			INC(top);
			Active; String(" := TRUE;"); Comment("logical"); Ln;
			Val; String(" := l"); f.WriteInt(j); String(";"); Ln;
			Diff; String(" := d"); f.WriteInt(j); String(";"); Ln;
		END WriteRef;

		PROCEDURE WriteConst;
		BEGIN
			INC(top);
			Val; String(" := node.c"); f.WriteInt(numC); String(";"); Comment("constant"); Ln;
			Diff; String(" := 0.0;"); Ln;
			Active; String(" := FALSE;"); Ln;
			INC(numC)
		END WriteConst;

		PROCEDURE WriteAdd;
		BEGIN
			DEC(top);
			Val; String(" :="); Val; String(" + "); Val1; String(";"); Comment("add"); Ln;
			If; Active; Then; Ln;
			If; Active1; Then;
			Diff; String(" := "); Diff; String(" + "); Diff1; End; Ln;
			Else; Ln;
			If; Active1; Then;
			Diff; String(" := "); Diff1; Else; Diff; String(" := "); String("0");
			End; Ln;
			End; Ln;
		END WriteAdd;

		PROCEDURE WriteSub;
		BEGIN
			DEC(top);
			Val; String(" := "); Val; String(" - "); Val1; String(";"); Comment("subtract"); Ln;
			If; Active; Then; Ln;
			If; Active1; Then;
			Diff; String(" := "); Diff; String(" - "); Diff1; Else; Diff; String(" := "); Diff; End; Ln;
			Else; Ln;
			If; Active1; Then;
			Diff; String(" := "); String(" - "); Diff1; Else; Diff; String(" := "); String("0");
			End; Ln;
			End; Ln;
		END WriteSub;

		PROCEDURE WriteMult;
		BEGIN
			DEC(top);
			If; Active; Then; Comment("multiply"); Ln;
			If; Active1; Then;
			Diff; String(" := "); Diff1; String(" * "); Val; String(" + "); Diff; String(" * "); Val1;
			Else;
			Diff; String(" := "); Diff; String(" * "); Val1;
			End; Ln;
			Else; Ln;
			If; Active1; Then;
			Diff; String(" := "); Diff1; String(" * "); Val;
			Else;
			Diff; String(" := "); String("0");
			End; Ln;
			End; Ln;
			Val; String(" := "); Val; String(" * "); Val1; String(";"); Ln;
		END WriteMult;

		PROCEDURE WriteDiv;
		BEGIN
			DEC(top);
			If; Active; Then; Comment("divide"); Ln;
			If; Active1; Then;
			Diff; String(" := "); Diff; String(" / "); Val1;
			String(" - "); Diff1; String(" * "); Val;
			String("/ ("); Val1; String(" * "); Val1; String(");"); Ln;
			Else;
			Diff; String(" := "); Diff; String(" / "); Val1; Ln;
			End; Ln;
			Else; Ln;
			If; Active1; Then;
			Diff; String(" := ");
			String(" - "); Diff1; String(" * "); Val;
			String("/ ("); Val1; String(" * "); Val1; String(");"); Ln;
			Else;
			Diff; String(" := "); String("0"); Ln;
			End; Ln;
			End; Ln;
			Val; String(" := "); Val; String(" / "); Val1; String(";"); Ln;
		END WriteDiv;

		PROCEDURE WriteEquals;
		BEGIN
			(*	no differential	*)
			DEC(top);
			Assert; Assert1;
			Val; String(" := MathFunc.Equals("); Val; String(", "); Val1; String(");"); Ln
		END WriteEquals;

		PROCEDURE WriteMax;
		BEGIN
			(*	no differential	*)
			DEC(top);
			Assert; Assert1;
			Val; String(" := MAX("); Val; String(" , "); Val1; String(");"); Ln
		END WriteMax;

		PROCEDURE WriteMin;
		BEGIN
			(*	no differential	*)
			DEC(top);
			Assert; Assert1;
			Val; String(" := MIN("); Val; String(" , "); Val1; String(");"); Ln
		END WriteMin;

		PROCEDURE WritePow;
		BEGIN
			DEC(top);
			If; Active; Then; Comment("power"); Ln;
			If; Active1; Then; Ln;
			Diff; String(" := ("); Diff;
			String(" * "); Val1; String(" / "); Val; String(") + ");
			Diff1; String(" * Math.Ln("); Val; String(");"); Ln;
			Val; String(" := MathFunc.Power("); Val; String(", "); Val1; String(");"); Ln;
			Diff; String(" := "); Diff; String(" * "); Val; String(";"); Ln;
			Else; Ln;
			Diff; String(" := ("); Diff;
			String(" * "); Val1; String(" / "); Val; String(");"); Ln;
			Val; String(" := MathFunc.Power("); Val; String(", "); Val1; String(");"); Ln;
			Diff; String(" := "); Diff; String(" * "); Val; String(";"); Ln;
			End; Ln;
			Else; Ln;
			If; Active1; Then; Ln;
			Diff; String(" :=  ");
			Diff1; String(" * Math.Ln("); Val; String(");"); Ln;
			Val; String(" := MathFunc.Power("); Val; String(", "); Val1; String(");"); Ln;
			Diff; String(" := "); Diff; String(" * "); Val; String(";"); Ln;
			Else; Ln;
			Diff; String(" := 0;"); Ln;
			Val; String(" := MathFunc.Power("); Val; String(", "); Val1; String(");"); Ln;
			End; Ln;
			End; Ln;
		END WritePow;

		PROCEDURE WriteUminus;
		BEGIN
			Val; String(" := -"); Val; String(";"); Comment("unary minus"); Ln;
			If; Active; Then; Ln;
			Diff; String(" := -"); Diff; String(";"); Ln;
			End; Ln;
		END WriteUminus;

		PROCEDURE WriteAbs;
		BEGIN
			(*	no differential	*)
			Assert;
			Val; String(" := ABS("); Val; String(");"); Ln
		END WriteAbs;

		PROCEDURE WriteCloglog;
		BEGIN
			If; Active; Then; Comment("cloglog"); Ln;
			Diff; String(" := "); Diff; String(" * Math.Ln(1.0 - "); Val; String(") / (1.0 - "); Val; String(");"); End;
			End; Ln;
			Val; String(" := MathFunc.Cloglog("); Val; String(");"); Ln
		END WriteCloglog;

		PROCEDURE WriteICloglog;
		BEGIN
			Val; String(" := Math.Exp("); Val; String(");"); Comment("inverse cloglog"); Ln;
			If; Active; Then; Diff; String(" := "); Diff; String(" * "); Val; String(";"); End; Ln;
			Val; String(" := Math.Exp(-"); Val; String(");"); Ln;
			If; Active; Then; Diff; String(" := "); Diff; String(" * "); Val; String(";"); End; Ln;
			Val; String(" := "); String("1.0 - "); Val; String(";"); Ln
		END WriteICloglog;

		PROCEDURE WriteCos;
		BEGIN
			If; Active; Then; Comment("cosine"); Ln;
			Diff; String(" := -"); Diff; String(" * Math.Sin("); Val; String(");"); Ln;
			End; Ln;
			Val; String(" := Math.Cos("); Val; String(");"); Ln
		END WriteCos;

		PROCEDURE WriteExp;
		BEGIN
			Val; String(" := Math.Exp("); Val; String(");"); Comment("exp"); Ln;
			If; Active; Then; Ln;
			Diff; String(" := "); Diff; String(" * "); Val; String(";");
			End; Ln
		END WriteExp;

		PROCEDURE WriteLog;
		BEGIN
			If; Active; Then; Comment("log"); Ln;
			Diff; String(" := "); Diff; String(" / "); Val; String(";"); Ln;
			End; Ln;
			Val; String(" := Math.Ln("); Val; String(");"); Ln
		END WriteLog;

		PROCEDURE WriteLogFact;
		BEGIN
			(*	no differential	*)
			Assert;
			Val; String(" := MathFunc.LogFactorial(SHORT(ENTIER("); Val; String(" + eps)));"); Ln
		END WriteLogFact;

		PROCEDURE WriteLogGamma;
		BEGIN
			If; Active; Then; Comment("log gamma"); Ln;
			Diff; String(" := "); Diff; String(" * MathFunc.Digamma("); Val; String(");"); Ln;
			End; Ln;
			Val; String(" := MathFunc.LogGammaFunc("); Val; String(");"); Ln
		END WriteLogGamma;

		PROCEDURE WriteLogit;
		BEGIN
			If; Active; Then; Comment("logit"); Ln;
			Diff; String(" := ");
			Diff; String(" * ((1.0 / "); Val; String(") + 1.0 / (1.0 - "); Val; String("));"); Ln;
			End; Ln;
			Val; String(" := MathFunc.Logit("); Val; String(");"); Ln
		END WriteLogit;

		PROCEDURE WriteILogit;
		BEGIN
			Val; String(" := MathFunc.Ilogit("); Val; String(");"); Comment("inverse logit"); Ln;
			If; Active; Then; Ln;
			Diff; String(" := "); Diff; String(" * "); Val; String(" * (1.0 -  "); Val; String(");"); Ln;
			End; Ln;
		END WriteILogit;

		PROCEDURE WritePhi;
		BEGIN
			If; Active; Then; Comment("phi"); Ln;
			Diff; String(" := "); Diff; String(" * Math.Exp(-0.5 * "); Val; String(" * "); Val;
			String(") / Math.Sqrt(2 * Math.Pi());"); Ln;
			End; Ln;
			Val; String(" := MathFunc.Phi("); Val; String(");"); Ln
		END WritePhi;

		PROCEDURE WriteRound;
		BEGIN
			(*	no differential	*)
			Assert;
			Val; String(" := Math.Round("); Val; String(");"); Ln
		END WriteRound;

		PROCEDURE WriteSin;
		BEGIN
			If; Active; Then; Comment("sin"); Ln;
			Diff; String(" := "); Diff; String(" * Math.Cos("); Val; String(");"); Ln;
			End; Ln;
			Val; String(" := Math.Sin("); Val; String(");"); Ln
		END WriteSin;

		PROCEDURE WriteSqrt;
		BEGIN
			Val; String(" := Math.Sqrt("); Val; String(");"); Comment("sqrt"); Ln;
			If; Active; Then; Ln;
			Diff; String(" := 0.5 * "); Diff; String("/ "); Val; String(";"); Ln;
			End; Ln;
		END WriteSqrt;

		PROCEDURE WriteStep;
		BEGIN
			(* no differential	*)
			Assert;
			Val; String(" := MathFunc.Step("); Val; String(");"); Ln;
		END WriteStep;

		PROCEDURE WriteTrunc;
		BEGIN
			(* no differential	*)
			Assert;
			Val; String(" := Math.Trunc("); Val; String(");"); Ln
		END WriteTrunc;

		PROCEDURE WriteTan;
		BEGIN
			Val; String(" := Math.Tan("); Val; String(");"); Comment("tan"); Ln;
			If; Active; Then; Ln;
			Diff; String(" := "); Diff; String(" * (1.0 + "); Val; String(" * "); Val; String(");"); Ln;
			End; Ln
		END WriteTan;

		PROCEDURE WriteArcSin;
		BEGIN
			If; Active; Then; Comment("arc sin"); Ln;
			Diff; String(" := "); Diff; String(" / Math.Sqrt(1.0 - "); Val; String(" * "); Val; String(");"); Ln;
			End; Ln;
			Val; String(" := Math.ArcSin("); Val; String(");"); Ln
		END WriteArcSin;

		PROCEDURE WriteArcCos;
		BEGIN
			If; Active; Then; Comment("arc cos"); Ln;
			Diff; String(" := -"); Diff; String(" / Math.Sqrt(1.0 - "); Val; String(" * "); Val; String(");"); Ln;
			End; Ln;
			Val; String(" := Math.ArcCos("); Val; String(");"); Ln
		END WriteArcCos;

		PROCEDURE WriteArcTan;
		BEGIN
			If; Active; Then; Comment("arc tan"); Ln;
			Diff; String(" := "); Diff; String(" / (1.0 + "); Val; String(" * "); Val; String(");"); Ln;
			End; Ln;
			Val; String(" := Math.ArcTan("); Val; String(");"); Ln
		END WriteArcTan;

		PROCEDURE WriteSinh;
		BEGIN
			If; Active; Then; Comment("sinh"); Ln;
			Diff; String(" := "); Diff; String(" * Math.Cosh("); Val; String(");"); Ln;
			End; Ln;
			Val; String(" := Math.Sinh("); Val; String(");"); Ln
		END WriteSinh;

		PROCEDURE WriteCosh;
		BEGIN
			If; Active; Then; Comment("cosh"); Ln;
			Diff; String(" := "); Diff; String(" * Math.Sinh("); Val; String(");"); Ln;
			End; Ln;
			Val; String(" := Math.Cosh("); Val; String(");"); Ln
		END WriteCosh;

		PROCEDURE WriteTanh;
		BEGIN
			If; Active; Then; Comment("tanh"); Ln;
			Diff; String(" := "); Diff; String(" * Math.IntPower(Math.Cosh("); Val; String(") , 2);"); Ln;
			End; Ln;
			Val; String(" := Math.Tanh("); Val; String(");"); Ln
		END WriteTanh;

		PROCEDURE WriteArcSinh;
		BEGIN
			If; Active; Then; Comment("arc sinh"); Ln;
			Diff; String(" := "); Diff; String(" / Math.Sqrt("); Val; String(" * "); Val; String(" + 1.0);"); Ln;
			End; Ln;
			Val; String(" := Math.ArcSinh("); Val; String(");"); Ln
		END WriteArcSinh;

		PROCEDURE WriteArcCosh;
		BEGIN
			If; Active; Then; Comment("arc cosh"); Ln;
			Diff; String(" := "); Diff; String(" / Math.Sqrt("); Val; String(" * "); Val; String(" - 1.0);");
			End; Ln;
			Val; String(" := Math.ArcCosh("); Val; String(");"); Ln
		END WriteArcCosh;

		PROCEDURE WriteArcTanh;
		BEGIN
			If; Active; Then; Comment("arc tanh"); Ln;
			Diff; String(" := "); Diff; String(" / ( 1.0 - "); Val; String(" * "); Val; String(");"); Ln;
			End; Ln;
			Val; String(" := Math.ArcTanh("); Val; String(");"); Ln
		END WriteArcTanh;

	BEGIN
		stackSize := StackSize(args);
		String("PROCEDURE (node: Node) ValDiff (x: GraphNodes.Node; OUT val0, diff0: REAL);"); Ln;
		IF stackSize > 0 THEN
			String("VAR"); Ln
		END;
		IF stackSize > 1 THEN
			String("val1, diff1");
			i := 2;
			WHILE i < stackSize DO
				String(", val"); f.WriteInt(i);
				String(", diff"); f.WriteInt(i);
				INC(i)
			END;
			String(": REAL;"); Ln;
		END;
		IF stackSize > 0 THEN
			String("active0");
			i := 1;
			WHILE i < stackSize DO
				String(", active"); f.WriteInt(i); INC(i)
			END;
			String(": BOOLEAN;"); Ln
		END;
		IF args.numLogicals > 0 THEN
			IF stackSize < 2 THEN String("VAR"); Ln END;
			String("l0, d0");
			i := 1;
			WHILE i < args.numLogicals DO
				String(", l"); f.WriteInt(i);
				String(", d"); f.WriteInt(i);
				INC(i)
			END;
			String(": REAL;"); Ln;
		END;
		String("BEGIN"); Ln;
		IF args.numLogicals > 0 THEN
			WriteRefLogical(0);
			i := 1;
			WHILE i < args.numLogicals DO
				WriteRefLogical(i);
				INC(i)
			END;
		END;
		i := 0;
		top := - 1;
		numC := 0;
		WHILE i < args.numOps DO
			op := args.ops[i];
			CASE op OF
			|GraphGrammar.refStoch: INC(i); WriteRefStoch(args.ops[i]);
			|GraphGrammar.ref: INC(i); WriteRef(args.ops[i]);
			|GraphGrammar.const: WriteConst
			|GraphGrammar.abs: WriteAbs
			|GraphGrammar.add: WriteAdd
			|GraphGrammar.sub: WriteSub
			|GraphGrammar.mult: WriteMult
			|GraphGrammar.div: WriteDiv
			|GraphGrammar.uminus: WriteUminus
			|GraphGrammar.cloglog: WriteCloglog
			|GraphGrammar.icloglog: WriteICloglog
			|GraphGrammar.cos: WriteCos
			|GraphGrammar.equals: WriteEquals
			|GraphGrammar.exp: WriteExp
			|GraphGrammar.log: WriteLog
			|GraphGrammar.logfact: WriteLogFact
			|GraphGrammar.loggam: WriteLogGamma
			|GraphGrammar.logit: WriteLogit
			|GraphGrammar.ilogit: WriteILogit
			|GraphGrammar.max: WriteMax
			|GraphGrammar.min: WriteMin
			|GraphGrammar.phi: WritePhi
			|GraphGrammar.pow: WritePow
			|GraphGrammar.round: WriteRound
			|GraphGrammar.sin: WriteSin
			|GraphGrammar.sqrt: WriteSqrt
			|GraphGrammar.step: WriteStep
			|GraphGrammar.trunc: WriteTrunc
			|GraphGrammar.tan: WriteTan
			|GraphGrammar.arcsin: WriteArcSin
			|GraphGrammar.arccos: WriteArcCos
			|GraphGrammar.arctan: WriteArcTan
			|GraphGrammar.sinh: WriteSinh
			|GraphGrammar.cosh: WriteCosh
			|GraphGrammar.tanh: WriteTanh
			|GraphGrammar.arcsinh: WriteArcSinh
			|GraphGrammar.arccosh: WriteArcCosh
			|GraphGrammar.arctanh: WriteArcTanh
			END;
			IF i < args.numOps - 1 THEN
				CASE op OF
				|GraphGrammar.add, GraphGrammar.sub, GraphGrammar.mult, GraphGrammar.div,
					GraphGrammar.pow:
					Active; String(" := "); Active; String(" OR "); Active1; String(";"); Ln;
				ELSE
				END
			END;
			INC(i);
		END;
		String(" END ValDiff;"); Ln;
		f. WriteLn;
	END WriteValDiffMethod;

	PROCEDURE WriteFactoryMethods (VAR f: TextMappers.Formatter);
	BEGIN
		f.WriteLn;
		f.WriteString(" PROCEDURE (f: Factory) NumParam (): INTEGER;"); f.WriteLn;
		f.WriteString("BEGIN"); f.WriteLn;
		f.WriteString("RETURN 0"); f.WriteLn;
		f.WriteString("END NumParam;"); f.WriteLn;
		f.WriteLn;

		f.WriteString(" PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);"); f.WriteLn;
		f.WriteString("BEGIN"); f.WriteLn;
		f.WriteString("signature := '' "); f.WriteLn;
		f.WriteString("END Signature;"); f.WriteLn;
		f.WriteLn;
		f.WriteString(" PROCEDURE (f: Factory) New (): GraphNodes.Node;"); f.WriteLn;
		f.WriteString(" VAR node: Node;"); f.WriteLn;
		f.WriteString(" BEGIN "); f. WriteLn;
		f.WriteString(" NEW(node); node.Init; "); f.WriteLn;
		f.WriteString(" RETURN node"); f.WriteLn;
		f.WriteString(" END New;"); f.WriteLn;
		f.WriteLn;
	END WriteFactoryMethods;

	PROCEDURE WriteInstallProcedure (VAR f: TextMappers.Formatter);
	BEGIN
		f.WriteString(" PROCEDURE Install*;"); f.WriteLn;
		f.WriteString(" BEGIN "); f.WriteLn;
		f.WriteString("GraphNodes.SetFactory(fact)"); f.WriteLn;
		f.WriteString(" END Install;"); f.WriteLn; f.WriteLn
	END WriteInstallProcedure;

	PROCEDURE WriteInitProcedure (VAR f: TextMappers.Formatter);
	BEGIN
		f.WriteString(" PROCEDURE Init;"); f.WriteLn;
		f.WriteString(" VAR factory: Factory;"); f.WriteLn;
		f.WriteString(" BEGIN "); f.WriteLn;
		f.WriteString("version := 500;"); f.WriteLn;
		f.WriteString("maintainer := 'OpenBUGS';"); f.WriteLn;
		f.WriteString(" NEW(factory);"); f.WriteLn;
		f.WriteString("fact := factory;"); f.WriteLn;
		f.WriteString(" END Init;"); f.WriteLn; f.WriteLn;
	END WriteInitProcedure;

	PROCEDURE WriteModule* (IN args: GraphStochastic.ArgsLogical; index: INTEGER;
	VAR f: TextMappers.Formatter);
		VAR
			fileName, timeStamp: ARRAY 128 OF CHAR;
	BEGIN
		Strings.IntToString(index, fileName);
		Strings.IntToString(GraphNodes.timeStamp, timeStamp);
		fileName := "Node" + fileName + "_" + timeStamp; ;
		f.SetPos(0);
		(*	write MODULE stuff	*)
		WriteModuleHeader("Dynamic" + fileName, f);
		(*	write types	*)
		WriteTypes(args, f);
		(*	write global variables	*)
		f.WriteString("VAR"); f.WriteLn;
		f.WriteString("version-: INTEGER;"); f.WriteLn;
		f.WriteString("maintainer-: ARRAY 40 OF CHAR;"); f.WriteLn;
		f.WriteString("fact-: GraphNodes.Factory;"); f.WriteLn;
		f.WriteLn;
		(*	write global constants	*)
		f.WriteString("CONST"); f.WriteLn;
		f.WriteString("eps = 1.0E-10;"); f.WriteLn; f.WriteLn;
		(*	write Check method	*)
		WriteCheckMethod(f);
		(*	write ClassFunction method	*)
		WriteClassFunctionMethod(args, f);
		(*	write ExternalizeLogical method	*)
		WriteExternalizeLogicalMethod(args, f);
		(*	write InitLogical method	*)
		WriteInitLogicalMethod(f);
		(*	write InternalizeLogical method	*)
		WriteInternalizeLogicalMethod(args, f);
		(*	write Install method	*)
		WriteInstallMethod("Dynamic" + fileName, f);
		(*	write Parent method	*)
		WriteParentsMethod(args, f);
		(*	write Set method	*)
		WriteSetMethod(args, f);
		(*	write Value method	*)
		WriteValueMethod(args, f);
		(*	write ValDiff method	*)
		WriteValDiffMethod(args, f);
		(*	write factory stuff	*)
		WriteFactoryMethods(f);
		(*	write Install procedure	*)
		WriteInstallProcedure(f);
		(*	write Init procedure	*)
		WriteInitProcedure(f);
		(*	write body of module	*)
		f.WriteString(" BEGIN "); f.WriteLn;
		f.WriteString("Init"); f.WriteLn;
		(*	end of module	*)
		f.WriteString(" END "); f.WriteString("Dynamic"); f.WriteString(fileName); f.WriteChar("."); f.WriteLn;
		f.WriteLn
	END WriteModule;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END BugsCPWrite.
