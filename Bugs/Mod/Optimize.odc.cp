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

	PROCEDURE ^FoldConstants* (tree: BugsParser.Node);

	PROCEDURE Add (tree: BugsParser.Binary);
	BEGIN
		FoldConstants(tree.left);
		IF ~tree.left.evaluated THEN RETURN END;
		FoldConstants(tree.right);
		IF ~tree.right.evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := tree.left.value + tree.right.value
	END Add;

	PROCEDURE Sub (tree: BugsParser.Binary);
		VAR
			refLeft, refRight: GraphNodes.Node;
	BEGIN
		FoldConstants(tree.left);
		IF ~tree.left.evaluated THEN RETURN END;
		FoldConstants(tree.right);
		IF ~tree.right.evaluated THEN RETURN END;
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

	PROCEDURE Mult (tree: BugsParser.Binary);
		CONST
			eps = 1.0E-20;
	BEGIN
		FoldConstants(tree.left);
		IF ~tree.left.evaluated THEN RETURN END;
		FoldConstants(tree.right);
		IF ~tree.right.evaluated THEN RETURN END;
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

	PROCEDURE Div (tree: BugsParser.Binary);
		CONST
			eps = 1.0E-20;
	BEGIN
		FoldConstants(tree.left);
		IF ~tree.left.evaluated THEN RETURN END;
		FoldConstants(tree.right);
		IF ~tree.right.evaluated THEN RETURN END;
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

	PROCEDURE UMinus (tree: BugsParser.Binary);
	BEGIN
		FoldConstants(tree.left);
		IF ~tree.left.evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value :=  - tree.left.value
	END UMinus;

	PROCEDURE Power (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		FoldConstants(tree.parents[1]);
		IF ~tree.parents[1].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := MathFunc.Power(tree.parents[0].value, tree.parents[1].value)
	END Power;

	PROCEDURE Equals (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		FoldConstants(tree.parents[1]);
		IF ~tree.parents[1].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := MathFunc.Equals(tree.parents[0].value, tree.parents[1].value)
	END Equals;

	PROCEDURE Max (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		FoldConstants(tree.parents[1]);
		IF ~tree.parents[1].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := MAX(tree.parents[0].value, tree.parents[1].value)
	END Max;

	PROCEDURE Min (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		FoldConstants(tree.parents[1]);
		IF ~tree.parents[1].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := MIN(tree.parents[0].value, tree.parents[1].value)
	END Min;

	PROCEDURE Abs (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := ABS(tree.parents[0].value)
	END Abs;

	PROCEDURE Round (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := Math.Round(tree.parents[0].value)
	END Round;

	PROCEDURE Trunc (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := Math.Trunc(tree.parents[0].value)
	END Trunc;

	PROCEDURE Tan (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := Math.Tan(tree.parents[0].value)
	END Tan;

	PROCEDURE ArcSin (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := Math.ArcSin(tree.parents[0].value)
	END ArcSin;

	PROCEDURE ArcCos (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := Math.ArcCos(tree.parents[0].value)
	END ArcCos;

	PROCEDURE ArcTan (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := Math.ArcTan(tree.parents[0].value)
	END ArcTan;

	PROCEDURE Sinh (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := Math.Sinh(tree.parents[0].value)
	END Sinh;

	PROCEDURE Cosh (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := Math.Cosh(tree.parents[0].value)
	END Cosh;

	PROCEDURE Tanh (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := Math.Tanh(tree.parents[0].value)
	END Tanh;

	PROCEDURE ArcSinh (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := Math.ArcSinh(tree.parents[0].value)
	END ArcSinh;

	PROCEDURE ArcCosh (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := Math.ArcCosh(tree.parents[0].value)
	END ArcCosh;

	PROCEDURE ArcTanh (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := Math.ArcTanh(tree.parents[0].value)
	END ArcTanh;

	PROCEDURE CLogLog (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := MathFunc.Cloglog(tree.parents[0].value)
	END CLogLog;

	PROCEDURE ICloglog (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := MathFunc.ICloglog(tree.parents[0].value)
	END ICloglog;

	PROCEDURE Exp (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := Math.Exp(tree.parents[0].value)
	END Exp;

	PROCEDURE Cos (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := Math.Cos(tree.parents[0].value)
	END Cos;

	PROCEDURE Sin (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := Math.Sin(tree.parents[0].value)
	END Sin;

	PROCEDURE Phi (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := MathFunc.Phi(tree.parents[0].value)
	END Phi;

	PROCEDURE Log (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := Math.Ln(tree.parents[0].value)
	END Log;

	PROCEDURE Logit (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := MathFunc.Logit(tree.parents[0].value)
	END Logit;

	PROCEDURE ILogit (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		IF tree.parents[0].evaluated THEN
			tree.evaluated := TRUE;
			tree.value := MathFunc.ILogit(tree.parents[0].value)
		ELSE
			tree.evaluated := FALSE
		END
	END ILogit;

	PROCEDURE Sqrt (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := Math.Sqrt(tree.parents[0].value)
	END Sqrt;

	PROCEDURE LogFact (tree: BugsParser.Internal);
		CONST
			eps = 1.0E-20;
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := MathFunc.LogFactorial(SHORT(ENTIER(tree.parents[0].value + eps)))
	END LogFact;

	PROCEDURE LogGamma (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := MathFunc.LogGammaFunc(tree.parents[0].value)
	END LogGamma;

	PROCEDURE Step (tree: BugsParser.Internal);
	BEGIN
		FoldConstants(tree.parents[0]);
		IF ~tree.parents[0].evaluated THEN RETURN END;
		tree.evaluated := TRUE;
		tree.value := MathFunc.Step(tree.parents[0].value)
	END Step;

	PROCEDURE FoldConstants* (tree: BugsParser.Node);

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
				|GraphGrammar.add: Add(t)
				|GraphGrammar.sub: Sub(t)
				|GraphGrammar.mult: Mult(t)
				|GraphGrammar.div: Div(t)
				|GraphGrammar.uminus: UMinus(t)
				END
			|t: BugsParser.Internal DO
				opDescriptor := t.descriptor;
				op := opDescriptor.key;
				CASE op OF
				|GraphGrammar.abs: Abs(t)
				|GraphGrammar.exp: Exp(t)
				|GraphGrammar.log: Log(t)
				|GraphGrammar.sqrt: Sqrt(t)
				|GraphGrammar.max: Max(t)
				|GraphGrammar.min: Min(t)
				|GraphGrammar.cloglog: CLogLog(t)
				|GraphGrammar.cos: Cos(t)
				|GraphGrammar.equals: Equals(t)
				|GraphGrammar.icloglog: ICloglog(t);
				|GraphGrammar.ilogit: ILogit(t);
				|GraphGrammar.logfact: LogFact(t)
				|GraphGrammar.loggam: LogGamma(t)
				|GraphGrammar.logit: Logit(t)
				|GraphGrammar.phi: Phi(t)
				|GraphGrammar.pow: Power(t)
				|GraphGrammar.round: Round(t)
				|GraphGrammar.sin: Sin(t)
				|GraphGrammar.step: Step(t)
				|GraphGrammar.trunc: Trunc(t)
				|GraphGrammar.tan: Tan(t)
				|GraphGrammar.arcsin: ArcSin(t)
				|GraphGrammar.arccos: ArcCos(t)
				|GraphGrammar.arctan: ArcTan(t)
				|GraphGrammar.sinh: Sinh(t)
				|GraphGrammar.cosh: Cosh(t)
				|GraphGrammar.tanh: Tanh(t)
				|GraphGrammar.arcsinh: ArcSinh(t)
				|GraphGrammar.arccosh: ArcCosh(t)
				|GraphGrammar.arctanh: ArcTanh(t)

				END
			|t: BugsParser.Function DO
				funcDescriptor := t.descriptor;
				fact := funcDescriptor.fact;
				IF fact IS GraphVector.Factory THEN RETURN END;
				fact.Signature(sig);
				num := fact.NumParam();
				i := 0;
				t.evaluated := TRUE;
				WHILE i < num DO
					IF (sig[i] = "e") OR (sig[i] = "s") THEN
						FoldConstants(t.parents[i]);
						t.evaluated := t.evaluated & t.parents[i].evaluated
					END;
					INC(i)
				END;
				IF t.evaluated THEN (*	try and evaluate node	*)
					IF sig = "eL" THEN (*	link function	*)
						args.Init;
						args.numScalars := 1;
						args.scalars[0] := GraphConstant.New(t.parents[0].value);
						ref := fact.New();
						ref.Set(args, res);
						t.value := ref.Value()
					ELSE
						args.Init;
						BugsCodegen.WriteFunctionArgs(t, args);
						t.evaluated := args.valid;
						IF t.evaluated THEN
							ref := fact.New();
							t.evaluated := ref # NIL;
							IF t.evaluated THEN
								ref.Set(args, res);
								t.evaluated := (res = {}) & (GraphNodes.data IN ref.props);
								IF t.evaluated THEN
									t.value := ref.Value()
								END
							END
						END
					END
				END
			|t: BugsParser.Variable DO
				ref := BugsEvaluate.RHScalar(t);
				IF ref = NIL THEN
					RETURN
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
	BEGIN
		tree.evaluated := FALSE;
		WITH tree: BugsParser.Binary DO
			UnMarkBinary(tree)
		|tree: BugsParser.Variable DO
			UnMarkVariable(tree)
		|tree: BugsParser.Internal DO
			UnMarkInternal(tree)
		|tree: BugsParser.Function DO
			UnMarkFunction(tree)
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
