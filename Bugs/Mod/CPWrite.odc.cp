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
		GraphGrammar, GraphStochastic;


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
			op := args.ops[i] MOD GraphGrammar.modifier;
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

		PROCEDURE Out (string: ARRAY OF CHAR); BEGIN f.WriteString(string) END Out;

		PROCEDURE Ln; BEGIN f.WriteLn END Ln;

		PROCEDURE Int (int: INTEGER); BEGIN f.WriteInt(int) END Int;

		PROCEDURE Class; BEGIN Out("class"); Int(top) END Class;

		PROCEDURE Class1; BEGIN Out("class"); Int(top + 1) END Class1;

		PROCEDURE WriteRefStoch (j: INTEGER);
		BEGIN
			INC(top);
			Out("p := node.s"); Int(j); Out(";"); Ln;
			Out("class"); Int(top);
			Out(" := GraphStochastic.ClassFunction(p, parent);")
		END WriteRefStoch;

		PROCEDURE WriteRefLogical (j: INTEGER);
		BEGIN
			Out("p := node.l"); Int(j); Out(";"); Ln;
			Out("l"); Int(j);
			Out(" := GraphStochastic.ClassFunction(p, parent);"); Ln;
			Out("CASE l"); Int(j); Out(" OF"); Ln;
			Out("GraphRules.logLink, GraphRules.logitLink,");
			Out("GraphRules.probitLink, GraphRules.cloglogLink:");
			Out("l"); Int(j); Out(" := GraphRules.linkFun"); Ln;
			Out("ELSE"); Ln;
			Out("END;")
		END WriteRefLogical;

		PROCEDURE WriteRef (j: INTEGER);
		BEGIN
			INC(top); Class; Out(" := l"); Int(j); Out(";")
		END WriteRef;

		PROCEDURE WriteConst;
		BEGIN
			INC(top); Class; Out(" := GraphRules.const;")
		END WriteConst;

		PROCEDURE WriteAdd;
		BEGIN
			DEC(top); Class; Out(" := GraphRules.addF["); Class; Out(" , "); Class1; Out("];")
		END WriteAdd;

		PROCEDURE WriteSub;
		BEGIN
			DEC(top); Class; Out(" := GraphRules.subF["); Class; Out(" , "); Class1; Out("];")
		END WriteSub;

		PROCEDURE WriteDiv;
		BEGIN
			DEC(top); Class; Out(" := GraphRules.divF["); Class; Out(" , "); Class1; Out("];")
		END WriteDiv;

		PROCEDURE WriteMult;
		BEGIN
			DEC(top); Class; Out(" := GraphRules.multF["); Class; Out(" , "); Class1; Out("];")
		END WriteMult;

		PROCEDURE WritePow;
		BEGIN
			DEC(top); Class; Out(" := GraphRules.powF["); Class; Out(" , "); Class1; Out("];")
		END WritePow;

		PROCEDURE WriteNonDiff2;
		BEGIN
			DEC(top); Class; Out(" := GraphRules.nonDiff2F["); Class; Out(" , "); Class1; Out("];")
		END WriteNonDiff2;

		PROCEDURE WriteUminus;
		BEGIN
			Class; Out(" := GraphRules.uminusF["); Class; Out("];")
		END WriteUminus;

		PROCEDURE WriteExp;
		BEGIN
			Class; Out(" := GraphRules.expF["); Class; Out("];")
		END WriteExp;

		PROCEDURE WriteLink;
		BEGIN
			Class; Out(" := GraphRules.linkF["); Class; Out("];")
		END WriteLink;

		PROCEDURE WriteNonDiff1;
		BEGIN
			Class; Out(" := GraphRules.nonDiff1F["); Class; Out("];")
		END WriteNonDiff1;

		PROCEDURE WriteOther;
		BEGIN
			Class; Out(" := GraphRules.uopF["); Class; Out("];")
		END WriteOther;

	BEGIN
		stackSize := StackSize(args);
		Out("PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;");
		Ln;
		Out("VAR"); Ln;
		Out("class0");
		i := 1;
		WHILE i < stackSize DO
			Out(", class"); Int(i); INC(i)
		END;
		Out(": INTEGER;"); Ln;
		IF args.numLogicals > 0 THEN
			Out("l0");
			i := 1;
			WHILE i < args.numLogicals DO
				Out(", l"); Int(i); INC(i)
			END;
			Out(": INTEGER;"); Ln;
		END;
		Out("p: GraphNodes.Node;"); Ln;
		Out("BEGIN"); Ln;
		IF args.numLogicals > 0 THEN
			WriteRefLogical(0); Ln;
			i := 1;
			WHILE i < args.numLogicals DO
				WriteRefLogical(i); Ln;
				INC(i)
			END;
		END;
		i := 0;
		top := - 1;
		WHILE i < args.numOps DO
			op := args.ops[i] MOD GraphGrammar.modifier;
			CASE op OF
			|GraphGrammar.refStoch: INC(i); WriteRefStoch(args.ops[i])
			|GraphGrammar.ref: INC(i); WriteRef(args.ops[i])
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
			Ln;
			INC(i);
		END;
		Out("RETURN class0"); Ln;
		Out("END ClassFunction;"); Ln;
		Ln
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

		PROCEDURE Out (IN s: ARRAY OF CHAR); BEGIN f.WriteString(s) END Out;
		
		PROCEDURE Int (int: INTEGER); BEGIN f.WriteInt(int) END Int;

		(*	write CP code from tree	*)
		PROCEDURE Write (t: Tree);
			VAR
				op: INTEGER;
		BEGIN
			op := t.op MOD GraphGrammar.modifier;
			CASE op OF
			|GraphGrammar.refStoch: Out("s"); Int(t.label)
			|GraphGrammar.ref: Out("l"); Int(t.label)
			|GraphGrammar.const: Out("node.c"); Int(t.label)
			|GraphGrammar.abs: Out("ABS("); Write(t.left); Out(")")
			|GraphGrammar.add: Out("("); Write(t.left); Out(" + "); Write(t.right); Out(")")
			|GraphGrammar.sub: Out("("); Write(t.left); Out(" - "); Write(t.right); Out(")")
			|GraphGrammar.mult: Out("("); Write(t.left); Out(" * "); Write(t.right); Out(")")
			|GraphGrammar.div: Out("("); Write(t.left); Out(" / "); Write(t.right); Out(")")
			|GraphGrammar.uminus: Out("(-("); Write(t.left); Out("))")
			|GraphGrammar.cloglog: Out("Math.Ln(-Math.Ln(1.0 - "); Write(t.left); Out("));")
			|GraphGrammar.icloglog: Out("(1.0 - Math.Exp( - Math.Exp("); Write(t.left); Out(")))");
			|GraphGrammar.cos: Out("Math.Cos("); Write(t.left); Out(")")
			|GraphGrammar.equals:
				Out("MathFunc.Equals("); Write(t.left); Out(", "); Write(t.right); Out(")")
			|GraphGrammar.exp: Out("Math.Exp("); Write(t.left); Out(")")
			|GraphGrammar.log: Out("Math.Ln("); Write(t.left); Out(")")
			|GraphGrammar.logfact:
				Out("MathFunc.LogFactorial(SHORT(ENTIER("); Write(t.left); Out(" + eps)))")
			|GraphGrammar.loggam: Out("MathFunc.LogGammaFunc("); Write(t.left); Out(")")
			|GraphGrammar.logit: Out("MathFunc.Logit("); Write(t.left); Out(")")
			|GraphGrammar.ilogit: Out("MathFunc.ILogit("); Write(t.left); Out(")")
			|GraphGrammar.max: Out("MAX("); Write(t.left); Out(", "); Write(t.right); Out(")")
			|GraphGrammar.min: Out("MIN("); Write(t.left); Out(", "); Write(t.right); Out(")")
			|GraphGrammar.phi: Out("MathFunc.Phi("); Write(t.left); Out(")")
			|GraphGrammar.pow: Out("MathFunc.Power("); Write(t.left); Out(", "); Write(t.right); Out(")")
			|GraphGrammar.round: Out("Math.Round("); Write(t.left); Out(")")
			|GraphGrammar.sin: Out("Math.Sin("); Write(t.left); Out(")")
			|GraphGrammar.sqrt: Out("Math.Sqrt("); Write(t.left); Out(")")
			|GraphGrammar.step: Out("MathFunc.Step("); Write(t.left); Out(")")
			|GraphGrammar.trunc: Out("Math.Trunc("); Write(t.left); Out(")")
			|GraphGrammar.tan: Out("Math.Tan("); Write(t.left); Out(")")
			|GraphGrammar.arcsin: Out("Math.ArcSin("); Write(t.left); Out(")")
			|GraphGrammar.arccos: Out("Math.ArcCos("); Write(t.left); Out(")")
			|GraphGrammar.arctan: Out("Math.ArcTan("); Write(t.left); Out(")")
			|GraphGrammar.sinh: Out("Math.Sinh("); Write(t.left); Out(")")
			|GraphGrammar.cosh: Out("Math.Cosh("); Write(t.left); Out(")")
			|GraphGrammar.tanh: Out("Math.Tanh("); Write(t.left); Out(")")
			|GraphGrammar.arcsinh: Out("Math.ArcSinh("); Write(t.left); Out(")")
			|GraphGrammar.arccosh: Out("Math.ArcCosh("); Write(t.left); Out(")")
			|GraphGrammar.arctanh: Out("Math.ArcTanh("); Write(t.left); Out(")")
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
			t.op := args.ops[i] MOD GraphGrammar.modifier;
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

		PROCEDURE Out (IN x: ARRAY OF CHAR); BEGIN f.WriteString(x) END Out;

		PROCEDURE Comment (IN x: ARRAY OF CHAR); BEGIN
			Out(" (* " + x + " *)"); f.WriteLn
		END Comment;

		PROCEDURE Int (i: INTEGER); BEGIN f.WriteInt(i) END Int;

		PROCEDURE Else; BEGIN Out(" ELSE ") END Else;

		PROCEDURE End; BEGIN Out(" END; ") END End;

		PROCEDURE Val; BEGIN Out("val"); f.WriteInt(top) END Val;

		PROCEDURE Val1; BEGIN Out("val"); f.WriteInt(top + 1) END Val1;

		PROCEDURE Diff; BEGIN Out("diff"); f.WriteInt(top) END Diff;

		PROCEDURE Diff1; BEGIN Out("diff"); f.WriteInt(top + 1) END Diff1;

		PROCEDURE Active; BEGIN Out("active"); f.WriteInt(top); END Active;

		PROCEDURE Active1; BEGIN Out("active"); f.WriteInt(top + 1); END Active1;

		PROCEDURE IfActive; BEGIN Out("IF active"); f.WriteInt(top); Out(" THEN ") END IfActive;

		PROCEDURE IfActive1; BEGIN Out("IF active"); f.WriteInt(top + 1); Out(" THEN ") END IfActive1;

		PROCEDURE Ln; BEGIN f.WriteLn END Ln;

		PROCEDURE Assert;
		BEGIN
			Out("ASSERT(~"); Active; Out(", 0);"); Ln;
		END Assert;

		PROCEDURE Assert1; BEGIN
			Out("ASSERT(~"); Active1; Out(", 0);"); Ln;
		END Assert1;

		PROCEDURE WriteRefStoch (j: INTEGER);
		BEGIN
			Comment("stochastic");
			INC(top);
			Active; Out(" := node.s"); Int(j); Out(" = x;"); Ln;
			Val; Out(" := node.s"); Int(j); Out(".value;"); Ln;
			IfActive; Diff; Out(" := 1.0 "); Else; Diff; Out(" := 0.0 "); End; Ln
		END WriteRefStoch;

		PROCEDURE WriteRefLogical (j: INTEGER);
		BEGIN
			Comment("logical intermediate");
			Out("node.l"); Int(j); Out(".ValDiff(x, l"); Int(j); Out(", d"); Int(j); Out(");"); Ln
		END WriteRefLogical;

		PROCEDURE WriteRef (j: INTEGER);
		BEGIN
			Comment("logical");
			INC(top);
			Active; Out(" := TRUE;"); Ln;
			Val; Out(" := l"); Int(j); Out(";"); Ln;
			Diff; Out(" := d"); Int(j); Out(";"); Ln;
		END WriteRef;

		PROCEDURE WriteConst;
		BEGIN
			Comment("constant");
			INC(top);
			Active; Out(" := FALSE;"); Ln;
			Val; Out(" := node.c"); Int(numC); Out(";"); Ln;
			Diff; Out(" := 0.0;"); Ln;
			INC(numC)
		END WriteConst;

		PROCEDURE WriteAdd (op: INTEGER);
		BEGIN
			DEC(top);
			IF op = GraphGrammar.add + GraphGrammar.rightConst THEN
				Comment("addR");
			ELSIF op = GraphGrammar.add + GraphGrammar.leftConst THEN
				Comment("addL");
				Diff; Out(" := "); Diff1; Out(";"); Active; Out(" := TRUE;"); Ln;
			ELSE
				Comment("add");
				IfActive; Ln;
				IfActive1; Diff; Out(" := "); Diff; Out(" + "); Diff1; End; Ln;
				Else; Ln;
				IfActive1; Diff; Out(" := "); Diff1; Out(" ; "); Active; Out(" := TRUE;"); End; Ln;
				End; Ln
			END;
			Val; Out(" :="); Val; Out(" + "); Val1; Out(";"); Ln;
		END WriteAdd;

		PROCEDURE WriteSub (op: INTEGER);
		BEGIN
			DEC(top);
			IF op = GraphGrammar.sub + GraphGrammar.rightConst THEN
				Comment("subtractR");
			ELSIF op = GraphGrammar.sub + GraphGrammar.leftConst THEN
				Comment("subtractL"); 
				Diff; Out(" := -"); Diff1; Out(";"); Active; Out(" := TRUE;"); Ln;
			ELSE
				Comment("subtract");
				IfActive; Ln;
				IfActive1; Diff; Out(" := "); Diff; Out(" - "); Diff1; Else; Diff; Out(" := "); Diff; End; Ln;
				Else; Ln;
				IfActive1; Diff; Out(" := "); Out(" - "); Diff1; Out(" ; "); Active; Out(" := TRUE;"); End; Ln;
				End; Ln
			END;
			Val; Out(" := "); Val; Out(" - "); Val1; Out(";"); Ln
		END WriteSub;

		PROCEDURE WriteMult (op: INTEGER);
		BEGIN
			DEC(top);
			IF op = GraphGrammar.mult + GraphGrammar.rightConst THEN
				Comment("multiplyR");
				IfActive; Diff; Out(" := "); Diff; Out(" * "); Val1; End; Ln
			ELSIF op = GraphGrammar.mult + GraphGrammar.leftConst THEN
				Comment("multiplyL");
				IfActive1; Diff; Out(" := "); Diff1; Out(" * "); Val; Out(" ; "); Active; Out(" := TRUE;"); End; Ln
			ELSE
				Comment("multiply");
				IfActive; Ln;
				IfActive1; Diff; Out(" := "); Diff1; Out(" * "); Val; Out(" + "); Diff; Out(" * "); Val1;
				Else;
				Diff; Out(" := "); Diff; Out(" * "); Val1;
				End; Ln;
				Else; Ln;
				IfActive1; Diff; Out(" := "); Diff1; Out(" * "); Val; Out(" ; "); Active; Out(" := TRUE;"); End; Ln;
				End; Ln;
			END;
			Val; Out(" := "); Val; Out(" * "); Val1; Out(";"); Ln;
		END WriteMult;

		PROCEDURE WriteDiv (op: INTEGER);
		BEGIN
			DEC(top);
			IF op = GraphGrammar.div + GraphGrammar.rightConst THEN
				Comment("divideR");
				IfActive; Diff; Out(" := "); Diff; Out(" / "); Val1; Ln; End; Ln
			ELSIF op = GraphGrammar.div + GraphGrammar.leftConst THEN
				Comment("divideL");
				IfActive1; Diff; Out(" := "); Out(" - "); Diff1; Out(" * "); Val; Out("/ ("); Val1; Out(" * "); Val1; Out(");");
				Active; Out(" := TRUE;");
				End; Ln
			ELSE
				Comment("divide");
				IfActive; Ln;
				IfActive1; Diff; Out(" := "); Diff; Out(" / "); Val1; Out(" - "); Diff1; Out(" * "); Val;
				Out("/ ("); Val1; Out(" * "); Val1; Out(");"); Ln;
				Else;
				Diff; Out(" := "); Diff; Out(" / "); Val1; End; Ln;
				Else; Ln;
				IfActive1;
				Diff; Out(" := "); Out(" - "); Diff1; Out(" * "); Val; Out("/ ("); Val1; Out(" * "); Val1; Out("); ");
				Active; Out(" := TRUE;");
				End; Ln;
				End; Ln
			END;
			Val; Out(" := "); Val; Out(" / "); Val1; Out(";"); Ln;
		END WriteDiv;

		PROCEDURE WriteEquals;
		BEGIN
			(*	no differential	*)
			DEC(top);
			Assert; Assert1;
			Val; Out(" := MathFunc.Equals("); Val; Out(", "); Val1; Out(");"); Ln
		END WriteEquals;

		PROCEDURE WriteMax;
		BEGIN
			(*	no differential	*)
			DEC(top);
			Assert; Assert1;
			Val; Out(" := MAX("); Val; Out(" , "); Val1; Out(");"); Ln
		END WriteMax;

		PROCEDURE WriteMin;
		BEGIN
			(*	no differential	*)
			DEC(top);
			Assert; Assert1;
			Val; Out(" := MIN("); Val; Out(" , "); Val1; Out(");"); Ln
		END WriteMin;

		PROCEDURE WritePow (op: INTEGER);
		BEGIN
			DEC(top);
			IF op = GraphGrammar.pow + GraphGrammar.rightConst THEN
				Comment("powerR");
				IfActive; Ln;
				Diff; Out(" := ("); Diff;
				Out(" * "); Val1; Out(" / "); Val; Out(");"); Ln;
				Val; Out(" := MathFunc.Power("); Val; Out(", "); Val1; Out(");"); Ln;
				Diff; Out(" := "); Diff; Out(" * "); Val; Out(";"); Ln;
				Else;
				Val; Out(" := MathFunc.Power("); Val; Out(", "); Val1; Out(");"); Ln;	
				End; Ln;
			ELSIF op = GraphGrammar.pow + GraphGrammar.leftConst THEN
				Comment("powerL");
				IfActive1; Ln;
				Diff; Out(" :=  "); Diff1; Out(" * Math.Ln("); Val; Out(");"); Ln;
				Val; Out(" := MathFunc.Power("); Val; Out(", "); Val1; Out(");"); Ln;
				Diff; Out(" := "); Diff; Out(" * "); Val; Out(";"); Active; Out(" := TRUE;");
				Else;
				Val; Out(" := MathFunc.Power("); Val; Out(", "); Val1; Out(");"); Ln;	
				End; Ln;
			ELSE
				Comment("power");
				IfActive; Ln;
				IfActive1; Ln;
				Diff; Out(" := ("); Diff;
				Out(" * "); Val1; Out(" / "); Val; Out(") + ");
				Diff1; Out(" * Math.Ln("); Val; Out(");"); Ln;
				Val; Out(" := MathFunc.Power("); Val; Out(", "); Val1; Out(");"); Ln;
				Diff; Out(" := "); Diff; Out(" * "); Val; Out(";"); Ln;
				Else; Ln;
				Diff; Out(" := ("); Diff;
				Out(" * "); Val1; Out(" / "); Val; Out(");"); Ln;
				Val; Out(" := MathFunc.Power("); Val; Out(", "); Val1; Out(");"); Ln;
				Diff; Out(" := "); Diff; Out(" * "); Val; Out(";"); Ln;
				End; Ln;
				Else; Ln;
				IfActive1;
				Diff; Out(" :=  ");
				Diff1; Out(" * Math.Ln("); Val; Out(");"); Ln;
				Val; Out(" := MathFunc.Power("); Val; Out(", "); Val1; Out(");"); Ln;
				Diff; Out(" := "); Diff; Out(" * "); Val; Out(";"); Active; Out(" := TRUE;"); Ln;
				Else; Ln;
				Val; Out(" := MathFunc.Power("); Val; Out(", "); Val1; Out(");"); Ln;
				End; Ln;
				End; Ln;
			END
		END WritePow;

		PROCEDURE WriteUminus;
		BEGIN
			Comment("unary minus");
			Val; Out(" := -"); Val; Out(";");
			IfActive; Diff; Out(" := -"); Diff; Out(";"); End; Ln
		END WriteUminus;

		PROCEDURE WriteAbs;
		BEGIN
			(*	no differential	*)
			Assert;
			Val; Out(" := ABS("); Val; Out(");"); Ln
		END WriteAbs;

		PROCEDURE WriteCloglog;
		BEGIN
			Comment("cloglog");
			IfActive; Diff; Out(" := "); Diff; Out(" * Math.Ln(1.0 - "); Val; Out(") / (1.0 - "); Val; Out(");");
			End; Ln;
			Val; Out(" := MathFunc.Cloglog("); Val; Out(");"); Ln
		END WriteCloglog;

		PROCEDURE WriteICloglog;
		BEGIN
			Comment("inverse cloglog");
			Val; Out(" := Math.Exp("); Val; Out(");");
			IfActive; Diff; Out(" := "); Diff; Out(" * "); Val; Out(";"); End; Ln;
			Val; Out(" := Math.Exp(-"); Val; Out(");"); Ln;
			IfActive; Diff; Out(" := "); Diff; Out(" * "); Val; Out(";"); End; Ln;
			Val; Out(" := "); Out("1.0 - "); Val; Out(";"); Ln
		END WriteICloglog;

		PROCEDURE WriteCos;
		BEGIN
			Comment("cosine");
			IfActive; Diff; Out(" := -"); Diff; Out(" * Math.Sin("); Val; Out(");"); End; Ln;
			Val; Out(" := Math.Cos("); Val; Out(");"); Ln
		END WriteCos;

		PROCEDURE WriteExp;
		BEGIN
			Comment("exp");
			Val; Out(" := Math.Exp("); Val; Out(");");
			IfActive; Diff; Out(" := "); Diff; Out(" * "); Val; Out(";"); End; Ln
		END WriteExp;

		PROCEDURE WriteLog;
		BEGIN
			Comment("log");
			IfActive; Diff; Out(" := "); Diff; Out(" / "); Val; Out(";"); End; Ln;
			Val; Out(" := Math.Ln("); Val; Out(");"); Ln
		END WriteLog;

		PROCEDURE WriteLogFact;
		BEGIN
			(*	no differential	*)
			Assert;
			Val; Out(" := MathFunc.LogFactorial(SHORT(ENTIER("); Val; Out(" + eps)));"); Ln
		END WriteLogFact;

		PROCEDURE WriteLogGamma;
		BEGIN
			Comment("log gamma");
			IfActive; Diff; Out(" := "); Diff; Out(" * MathFunc.Digamma("); Val; Out(");"); End; Ln;
			Val; Out(" := MathFunc.LogGammaFunc("); Val; Out(");"); Ln
		END WriteLogGamma;

		PROCEDURE WriteLogit;
		BEGIN
			Comment("logit");
			IfActive;
			Diff; Out(" := "); Diff; Out(" * ((1.0 / "); Val; Out(") + 1.0 / (1.0 - "); Val; Out("));");
			End; Ln;
			Val; Out(" := MathFunc.Logit("); Val; Out(");"); Ln
		END WriteLogit;

		PROCEDURE WriteILogit;
		BEGIN
			Comment("inverse logit");
			Val; Out(" := MathFunc.Ilogit("); Val; Out(");");
			IfActive; Diff; Out(" := "); Diff; Out(" * "); Val; Out(" * (1.0 -  "); Val; Out(");");
			End; Ln;
		END WriteILogit;

		PROCEDURE WritePhi;
		BEGIN
			Comment("phi");
			IfActive; Diff; Out(" := "); Diff; Out(" * Math.Exp(-0.5 * "); Val; Out(" * "); Val;
			Out(") / Math.Sqrt(2 * Math.Pi());"); Ln;
			End; Ln;
			Val; Out(" := MathFunc.Phi("); Val; Out(");"); Ln
		END WritePhi;

		PROCEDURE WriteRound;
		BEGIN
			(*	no differential	*)
			Assert;
			Val; Out(" := Math.Round("); Val; Out(");"); Ln
		END WriteRound;

		PROCEDURE WriteSin;
		BEGIN
			Comment("sin");
			IfActive; Diff; Out(" := "); Diff; Out(" * Math.Cos("); Val; Out(");"); Ln; End; Ln;
			Val; Out(" := Math.Sin("); Val; Out(");"); Ln
		END WriteSin;

		PROCEDURE WriteSqrt;
		BEGIN
			Comment("sqrt");
			Val; Out(" := Math.Sqrt("); Val; Out(");"); Ln;
			IfActive; Diff; Out(" := 0.5 * "); Diff; Out("/ "); Val; Out(";"); End; Ln;
		END WriteSqrt;

		PROCEDURE WriteStep;
		BEGIN
			(* no differential	*)
			Assert;
			Val; Out(" := MathFunc.Step("); Val; Out(");"); Ln;
		END WriteStep;

		PROCEDURE WriteTrunc;
		BEGIN
			(* no differential	*)
			Assert;
			Val; Out(" := Math.Trunc("); Val; Out(");"); Ln
		END WriteTrunc;

		PROCEDURE WriteTan;
		BEGIN
			Comment("tan");
			Val; Out(" := Math.Tan("); Val; Out(");"); Ln;
			IfActive; Diff; Out(" := "); Diff; Out(" * (1.0 + "); Val; Out(" * "); Val; Out(");");
			End; Ln
		END WriteTan;

		PROCEDURE WriteArcSin;
		BEGIN
			Comment("arc sin");
			IfActive; Diff; Out(" := "); Diff; Out(" / Math.Sqrt(1.0 - "); Val; Out(" * "); Val; Out(");");
			End; Ln;
			Val; Out(" := Math.ArcSin("); Val; Out(");"); Ln
		END WriteArcSin;

		PROCEDURE WriteArcCos;
		BEGIN
			Comment("arc cos");
			IfActive; Diff; Out(" := -"); Diff; Out(" / Math.Sqrt(1.0 - "); Val; Out(" * "); Val; Out(");");
			End; Ln;
			Val; Out(" := Math.ArcCos("); Val; Out(");"); Ln
		END WriteArcCos;

		PROCEDURE WriteArcTan;
		BEGIN
			Comment("arc tan");
			IfActive; Diff; Out(" := "); Diff; Out(" / (1.0 + "); Val; Out(" * "); Val; Out(");");
			End; Ln;
			Val; Out(" := Math.ArcTan("); Val; Out(");"); Ln
		END WriteArcTan;

		PROCEDURE WriteSinh;
		BEGIN
			Comment("sinh");
			IfActive; Diff; Out(" := "); Diff; Out(" * Math.Cosh("); Val; Out(");"); End; Ln;
			Val; Out(" := Math.Sinh("); Val; Out(");"); Ln
		END WriteSinh;

		PROCEDURE WriteCosh;
		BEGIN
			Comment("cosh");
			IfActive; Diff; Out(" := "); Diff; Out(" * Math.Sinh("); Val; Out(");"); End; Ln;
			Val; Out(" := Math.Cosh("); Val; Out(");"); Ln
		END WriteCosh;

		PROCEDURE WriteTanh;
		BEGIN
			Comment("tanh"); Ln;
			IfActive; Diff; Out(" := "); Diff; Out(" * Math.IntPower(Math.Cosh("); Val; Out(") , 2);");
			End; Ln;
			Val; Out(" := Math.Tanh("); Val; Out(");"); Ln
		END WriteTanh;

		PROCEDURE WriteArcSinh;
		BEGIN
			Comment("arc sinh");
			IfActive; Diff; Out(" := "); Diff; Out(" / Math.Sqrt("); Val; Out(" * "); Val; Out(" + 1.0);");
			End; Ln;
			Val; Out(" := Math.ArcSinh("); Val; Out(");"); Ln
		END WriteArcSinh;

		PROCEDURE WriteArcCosh;
		BEGIN
			Comment("arc cosh");
			IfActive; Diff; Out(" := "); Diff; Out(" / Math.Sqrt("); Val; Out(" * "); Val; Out(" - 1.0);");
			End; Ln;
			Val; Out(" := Math.ArcCosh("); Val; Out(");"); Ln
		END WriteArcCosh;

		PROCEDURE WriteArcTanh;
		BEGIN
			Comment("arc tanh");
			IfActive; Diff; Out(" := "); Diff; Out(" / ( 1.0 - "); Val; Out(" * "); Val; Out(");");
			End; Ln;
			Val; Out(" := Math.ArcTanh("); Val; Out(");"); Ln
		END WriteArcTanh;

	BEGIN
		stackSize := StackSize(args);
		Out("PROCEDURE (node: Node) ValDiff (x: GraphNodes.Node; OUT val0, diff0: REAL);"); Ln;
		IF (stackSize > 0) OR (args.numLogicals > 0)THEN
			Out("VAR"); Ln
		END;
		IF stackSize > 1 THEN
			Out("val1, diff1");
			i := 2;
			WHILE i < stackSize DO
				Out(", val"); f.WriteInt(i);
				Out(", diff"); f.WriteInt(i);
				INC(i)
			END;
			Out(": REAL;"); Ln;
		END;
		IF stackSize > 0 THEN
			Out("active0");
			i := 1;
			WHILE i < stackSize DO
				Out(", active"); f.WriteInt(i); INC(i)
			END;
			Out(": BOOLEAN;"); Ln
		END;
		IF args.numLogicals > 0 THEN
			Out("l0, d0");
			i := 1;
			WHILE i < args.numLogicals DO
				Out(", l"); f.WriteInt(i);
				Out(", d"); f.WriteInt(i);
				INC(i)
			END;
			Out(": REAL;"); Ln;
		END;
		Out("BEGIN"); Ln;
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
			op := args.ops[i] MOD GraphGrammar.modifier;
			CASE op OF
			|GraphGrammar.refStoch: INC(i); WriteRefStoch(args.ops[i]);
			|GraphGrammar.ref: INC(i); WriteRef(args.ops[i]);
			|GraphGrammar.const: WriteConst
			|GraphGrammar.abs: WriteAbs
			|GraphGrammar.add: WriteAdd(args.ops[i])
			|GraphGrammar.sub: WriteSub(args.ops[i])
			|GraphGrammar.mult: WriteMult(args.ops[i])
			|GraphGrammar.div: WriteDiv(args.ops[i])
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
			|GraphGrammar.pow: WritePow(args.ops[i])
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
			INC(i);
		END;
		Out(" END ValDiff;"); Ln;
		Ln;
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
			fileName: ARRAY 128 OF CHAR;
	BEGIN
		Strings.IntToString(index, fileName);
		fileName := "Node" + fileName;
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
