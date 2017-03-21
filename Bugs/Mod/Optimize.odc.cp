(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsOptimize;


	

	IMPORT
		Math,
		BugsCodegen, BugsEvaluate, BugsParser,
		GraphConstant, GraphGrammar, GraphNodes, GraphStochastic, GraphVector,
		MathFunc;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE ^FoldConstants* (tree: BugsParser.Node; OUT ok: BOOLEAN);

	PROCEDURE Add (tree: BugsParser.Binary; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.left, ok);
		IF ~ok THEN RETURN END;
		FoldConstants(tree.right, ok);
		IF ~ok THEN RETURN END;
		tree.evaluated := tree.left.evaluated & tree.right.evaluated;
		IF tree.evaluated THEN
			tree.value := tree.left.value + tree.right.value
		END
	END Add;

	PROCEDURE Sub (tree: BugsParser.Binary; OUT ok: BOOLEAN);
		VAR
			refLeft, refRight: GraphNodes.Node;
	BEGIN
		ok := TRUE;
		FoldConstants(tree.left, ok);
		IF ~ok THEN RETURN END;
		FoldConstants(tree.right, ok);
		IF ~ok THEN RETURN END;
		tree.evaluated := tree.left.evaluated & tree.right.evaluated;
		IF tree.evaluated THEN
			tree.value := tree.left.value - tree.right.value
		ELSIF tree.left IS BugsParser.Variable THEN
			refLeft := BugsEvaluate.RHScalar(tree.left);
			IF refLeft # NIL THEN
				IF tree.right IS BugsParser.Variable THEN
					refRight := BugsEvaluate.RHScalar(tree.right);
					IF refLeft = refRight THEN
						tree.evaluated := TRUE;
						tree.value := 0.0
					END
				END
			END
		END
	END Sub;

	PROCEDURE Mult (tree: BugsParser.Binary; OUT ok: BOOLEAN);
		CONST
			eps = 1.0E-20;
	BEGIN
		ok := TRUE;
		FoldConstants(tree.left, ok);
		IF ~ok THEN RETURN END;
		FoldConstants(tree.right, ok);
		IF ~ok THEN RETURN END;
		IF tree.left.evaluated & (ABS(tree.left.value) < eps) THEN
			tree.evaluated := TRUE;
			tree.value := 0.0
		ELSIF tree.right.evaluated & (ABS(tree.right.value) < eps) THEN
			tree.evaluated := TRUE;
			tree.value := 0.0
		ELSIF tree.left.evaluated & tree.right.evaluated THEN
			tree.evaluated := TRUE;
			tree.value := tree.left.value * tree.right.value
		ELSE
			tree.evaluated := FALSE
		END
	END Mult;

	PROCEDURE Div (tree: BugsParser.Binary; OUT ok: BOOLEAN);
		CONST
			eps = 1.0E-20;
	BEGIN
		ok := TRUE;
		FoldConstants(tree.left, ok);
		IF ~ok THEN RETURN END;
		FoldConstants(tree.right, ok);
		IF ~ok THEN RETURN END;
		IF tree.left.evaluated & (ABS(tree.left.value) < eps) THEN
			tree.evaluated := TRUE;
			tree.value := 0.0
		ELSIF tree.left.evaluated & tree.right.evaluated THEN
			tree.evaluated := TRUE;
			tree.value := tree.left.value / tree.right.value
		ELSE
			tree.evaluated := FALSE
		END
	END Div;

	PROCEDURE UMinus (tree: BugsParser.Binary; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.left, ok);
		IF ~ok THEN RETURN END;
		IF tree.left.evaluated THEN
			tree.evaluated := TRUE;
			tree.value :=  - tree.left.value
		ELSE
			tree.evaluated := FALSE
		END
	END UMinus;

	PROCEDURE Power (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		FoldConstants(tree.parents[1], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated & tree.parents[1].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := MathFunc.Power(tree.parents[0].value, tree.parents[1].value)
		ELSE
			tree.evaluated := FALSE
		END
	END Power;

	PROCEDURE Equals (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		FoldConstants(tree.parents[1], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated & tree.parents[1].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := MathFunc.Equals(tree.parents[0].value, tree.parents[1].value)
		ELSE
			tree.evaluated := FALSE
		END
	END Equals;

	PROCEDURE Max (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		FoldConstants(tree.parents[1], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated & tree.parents[1].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := MAX(tree.parents[0].value, tree.parents[1].value)
		ELSE
			tree.evaluated := FALSE
		END
	END Max;

	PROCEDURE Min (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		FoldConstants(tree.parents[1], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated & tree.parents[1].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := MIN(tree.parents[0].value, tree.parents[1].value)
		ELSE
			tree.evaluated := FALSE
		END
	END Min;

	PROCEDURE Abs (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := ABS(tree.parents[0].value)
		ELSE
			tree.evaluated := FALSE
		END
	END Abs;

	PROCEDURE Round (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := Math.Round(tree.parents[0].value)
		ELSE
			tree.evaluated := FALSE
		END
	END Round;

	PROCEDURE Trunc (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := Math.Trunc(tree.parents[0].value)
		ELSE
			tree.evaluated := FALSE
		END
	END Trunc;

	PROCEDURE Tan (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := Math.Tan(tree.parents[0].value)
		ELSE
			tree.evaluated := FALSE
		END
	END Tan;

	PROCEDURE ArcSin (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := Math.ArcSin(tree.parents[0].value)
		ELSE
			tree.evaluated := FALSE
		END
	END ArcSin;

	PROCEDURE ArcCos (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := Math.ArcCos(tree.parents[0].value)
		ELSE
			tree.evaluated := FALSE
		END
	END ArcCos;

	PROCEDURE ArcTan (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := Math.ArcTan(tree.parents[0].value)
		ELSE
			tree.evaluated := FALSE
		END
	END ArcTan;

	PROCEDURE Sinh (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := Math.Sinh(tree.parents[0].value)
		ELSE
			tree.evaluated := FALSE
		END
	END Sinh;

	PROCEDURE Cosh (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := Math.Cosh(tree.parents[0].value)
		ELSE
			tree.evaluated := FALSE
		END
	END Cosh;

	PROCEDURE Tanh (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := Math.Tanh(tree.parents[0].value)
		ELSE
			tree.evaluated := FALSE
		END
	END Tanh;

	PROCEDURE ArcSinh (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := Math.ArcSinh(tree.parents[0].value)
		ELSE
			tree.evaluated := FALSE
		END
	END ArcSinh;

	PROCEDURE ArcCosh (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := Math.ArcCosh(tree.parents[0].value)
		ELSE
			tree.evaluated := FALSE
		END
	END ArcCosh;

	PROCEDURE ArcTanh (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := Math.ArcTanh(tree.parents[0].value)
		ELSE
			tree.evaluated := FALSE
		END
	END ArcTanh;

	PROCEDURE CLogLog (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := MathFunc.Cloglog(tree.parents[0].value)
		ELSE
			tree.evaluated := FALSE
		END
	END CLogLog;

	PROCEDURE ICloglog (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := MathFunc.ICloglog(tree.parents[0].value)
		ELSE
			tree.evaluated := FALSE
		END
	END ICloglog;

	PROCEDURE Exp (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := Math.Exp(tree.parents[0].value)
		ELSE
			tree.evaluated := FALSE
		END
	END Exp;

	PROCEDURE Cos (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := Math.Cos(tree.parents[0].value)
		ELSE
			tree.evaluated := FALSE
		END
	END Cos;

	PROCEDURE Sin (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := Math.Sin(tree.parents[0].value)
		ELSE
			tree.evaluated := FALSE
		END
	END Sin;

	PROCEDURE Phi (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := MathFunc.Phi(tree.parents[0].value)
		ELSE
			tree.evaluated := FALSE
		END
	END Phi;

	PROCEDURE Log (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := Math.Ln(tree.parents[0].value)
		ELSE
			tree.evaluated := FALSE
		END
	END Log;

	PROCEDURE Logit (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := MathFunc.Logit(tree.parents[0].value)
		ELSE
			tree.evaluated := FALSE
		END
	END Logit;

	PROCEDURE ILogit (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := MathFunc.ILogit(tree.parents[0].value)
		ELSE
			tree.evaluated := FALSE
		END
	END ILogit;

	PROCEDURE Sqrt (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := Math.Sqrt(tree.parents[0].value)
		ELSE
			tree.evaluated := FALSE
		END
	END Sqrt;

	PROCEDURE LogFact (tree: BugsParser.Internal; OUT ok: BOOLEAN);
		CONST
			eps = 1.0E-20;
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := MathFunc.LogFactorial(SHORT(ENTIER(tree.parents[0].value + eps)))
		ELSE
			tree.evaluated := FALSE
		END
	END LogFact;

	PROCEDURE LogGamma (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := MathFunc.LogGammaFunc(tree.parents[0].value)
		ELSE
			tree.evaluated := FALSE
		END
	END LogGamma;

	PROCEDURE Step (tree: BugsParser.Internal; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE;
		FoldConstants(tree.parents[0], ok);
		IF ~ok THEN RETURN END;
		IF tree.parents[0].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := MathFunc.Step(tree.parents[0].value)
		ELSE
			tree.evaluated := FALSE
		END
	END Step;

	PROCEDURE FoldConstants* (tree: BugsParser.Node; OUT ok: BOOLEAN);

		PROCEDURE Evaluate (t: BugsParser.Node);
			VAR
				ref: GraphNodes.Node;
				i, num, op: INTEGER;
				opDescriptor: GraphGrammar.Internal;
				funcDescriptor: GraphGrammar.External;
				fact: GraphNodes.Factory;
				sig: ARRAY 64 OF CHAR;
				args: GraphStochastic.ArgsLogical;
				res: SET;
		BEGIN
			WITH t: BugsParser.Binary DO
				op := t.op;
				CASE op OF
				|GraphGrammar.add: Add(t, ok)
				|GraphGrammar.sub: Sub(t, ok)
				|GraphGrammar.mult: Mult(t, ok)
				|GraphGrammar.div: Div(t, ok)
				|GraphGrammar.uminus: UMinus(t, ok)
				END
			|t: BugsParser.Internal DO
				opDescriptor := t.descriptor;
				op := opDescriptor.key;
				CASE op OF
				|GraphGrammar.abs: Abs(t, ok)
				|GraphGrammar.exp: Exp(t, ok)
				|GraphGrammar.log: Log(t, ok)
				|GraphGrammar.sqrt: Sqrt(t, ok)
				|GraphGrammar.max: Max(t, ok)
				|GraphGrammar.min: Min(t, ok)
				|GraphGrammar.cloglog: CLogLog(t, ok)
				|GraphGrammar.cos: Cos(t, ok)
				|GraphGrammar.equals: Equals(t, ok)
				|GraphGrammar.icloglog: ICloglog(t, ok);
				|GraphGrammar.ilogit: ILogit(t, ok);
				|GraphGrammar.logfact: LogFact(t, ok)
				|GraphGrammar.loggam: LogGamma(t, ok)
				|GraphGrammar.logit: Logit(t, ok)
				|GraphGrammar.phi: Phi(t, ok)
				|GraphGrammar.pow: Power(t, ok)
				|GraphGrammar.round: Round(t, ok)
				|GraphGrammar.sin: Sin(t, ok)
				|GraphGrammar.step: Step(t, ok)
				|GraphGrammar.trunc: Trunc(t, ok)
				|GraphGrammar.tan: Tan(t, ok)
				|GraphGrammar.arcsin: ArcSin(t, ok)
				|GraphGrammar.arccos: ArcCos(t, ok)
				|GraphGrammar.arctan: ArcTan(t, ok)
				|GraphGrammar.sinh: Sinh(t, ok)
				|GraphGrammar.cosh: Cosh(t, ok)
				|GraphGrammar.tanh: Tanh(t, ok)
				|GraphGrammar.arcsinh: ArcSinh(t, ok)
				|GraphGrammar.arccosh: ArcCosh(t, ok)
				|GraphGrammar.arctanh: ArcTanh(t, ok)

				END
			|t: BugsParser.Function DO
				funcDescriptor := t.descriptor;
				fact := funcDescriptor.fact;
				IF fact IS GraphVector.Factory THEN RETURN END;
				fact.Signature(sig);
				num := fact.NumParam();
				i := 0;
				ok := TRUE;
				WHILE ok & (i < num) DO FoldConstants(t.parents[i], ok); INC(i) END;
				IF ok THEN
					IF sig = "eL" THEN (*	link function	*)
						IF t.parents[0].evaluated THEN
							args.Init;
							args.numScalars := 1;
							args.scalars[0] := GraphConstant.New(t.parents[0].value);
							ref := fact.New();
							ref.Set(args, res);
							t.evaluated := TRUE;
							t.value := ref.Value()
						END
					ELSE
						args.Init;
						BugsCodegen.WriteFunctionArgs(t, args);
						ok := args.valid;
						IF ok THEN
							ref := fact.New();
							ok := ref # NIL;
							IF ok THEN
								ref.Set(args, res);
								ok := res = {};
								IF ok & (GraphNodes.data IN ref.props) THEN
									t.evaluated := TRUE;
									t.value := ref.Value()
								END
							END
						END
					END
				END
			|t: BugsParser.Variable DO
				ref := BugsEvaluate.RHScalar(t);
				IF ref = NIL THEN
					ok := FALSE; t.evaluated := FALSE; RETURN
				END;
				t.evaluated := GraphNodes.data IN ref.props;
				IF t.evaluated THEN t.value := ref.Value() END
			|t: BugsParser.Integer DO
				t.evaluated := TRUE;
				t.value := t.integer
			|t: BugsParser.Real DO
				t.evaluated := TRUE;
				t.value := t.real
			|t: BugsParser.Index DO
				t.evaluated := TRUE;
				t.value := t.intVal
			END
		END Evaluate;
	BEGIN
		ok := TRUE;
		Evaluate(tree)
	END FoldConstants;

	PROCEDURE ^UnMark* (tree: BugsParser.Node);

	PROCEDURE UnMarkBinary (tree: BugsParser.Binary);
	BEGIN
		UnMark(tree.left);
		IF tree.right # NIL THEN UnMark(tree.right) END
	END UnMarkBinary;

	PROCEDURE UnMarkVariable (tree: BugsParser.Variable);
		VAR
			i: INTEGER;
	BEGIN
		IF tree.lower # NIL THEN
			i := 0;
			WHILE i < LEN(tree.lower) DO
				IF tree.lower[i] # NIL THEN UnMark(tree.lower[i]) END;
				INC(i)
			END
		END;
		IF tree.upper # NIL THEN
			i := 0;
			WHILE i < LEN(tree.upper) DO
				IF tree.upper[i] # NIL THEN UnMark(tree.upper[i]) END;
				INC(i)
			END
		END
	END UnMarkVariable;

	PROCEDURE UnMarkInternal (tree: BugsParser.Internal);
		VAR
			i: INTEGER;
			descriptor: GraphGrammar.Internal;
	BEGIN
		i := 0;
		descriptor := tree.descriptor;
		WHILE i < descriptor.numPar DO
			UnMark(tree.parents[i]);
			INC(i)
		END
	END UnMarkInternal;

	PROCEDURE UnMarkFunction (tree: BugsParser.Function);
		VAR
			i, numPar: INTEGER;
			descriptor: GraphGrammar.External;
			fact: GraphNodes.Factory;
			signature: ARRAY 64 OF CHAR;
	BEGIN
		i := 0;
		descriptor := tree.descriptor;
		fact := descriptor.fact;
		fact.Signature(signature);
		numPar := fact.NumParam();
		WHILE i < numPar DO
			IF signature[i] = "e" THEN UnMark(tree.parents[i]) END;
			INC(i)
		END
	END UnMarkFunction;

	PROCEDURE UnMark* (tree: BugsParser.Node);
		VAR
			binary: BugsParser.Binary;
			func: BugsParser.Function;
			op: BugsParser.Internal;
			var: BugsParser.Variable;
	BEGIN
		tree.evaluated := FALSE;
		WITH tree: BugsParser.Binary DO
			binary := tree(BugsParser.Binary);
			UnMarkBinary(binary)
		|tree: BugsParser.Variable DO
			var := tree(BugsParser.Variable);
			UnMarkVariable(var)
		|tree: BugsParser.Internal DO
			op := tree(BugsParser.Internal);
			UnMarkInternal(op)
		|tree: BugsParser.Function DO
			func := tree(BugsParser.Function);
			UnMarkFunction(func)
		|tree: BugsParser.Index DO
		|tree: BugsParser.Integer DO
		|tree: BugsParser.Real DO
		END
	END UnMark;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END BugsOptimize.
