(*	

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"






This module has been set up to implement the modified Weibull distribution(MWD) with parameters alpha (a >0) beta(b >0) and lambda((l >0).
C. D. Lai, M. Xie, and D. N. P. Murthy, “Modified Weibull model,” IEEE Trans. Reliability, vol. 52, pp. 33–37, 2003.

Usage : dweib.modified(alpha, beta, lambda)
*)

MODULE ReliabilityModifiedWeibull;


	

	IMPORT
		Math, Stores := Stores64,
		GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE
		Node = POINTER TO RECORD(GraphUnivariate.Node)
			alpha, beta, lambda: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD (GraphUnivariate.Factory) END;

	CONST
		eps = 1.0E-10;

	VAR
		fact-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) BoundsUnivariate (OUT left, right: REAL);
	BEGIN
		left := 0.0;
		right := INF
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
	BEGIN
		IF node.value < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.lhs}
		END;
		IF node.alpha.value < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg1}
		END;
		IF node.lambda.value < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg2}
		END;
		IF node.beta.value < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg3}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
		VAR
			f0, f1, f2: INTEGER;
	BEGIN
		f0 := GraphStochastic.ClassFunction(node.alpha, parent);
		f1 := GraphStochastic.ClassFunction(node.beta, parent);
		f2 := GraphStochastic.ClassFunction(node.lambda, parent);
		IF (f0 # GraphRules.const) & (f1 # GraphRules.const) & (f2 # GraphRules.const) THEN
			RETURN GraphRules.unif
		ELSIF (f0 = GraphRules.other) OR (f1 = GraphRules.other) OR (f2 = GraphRules.other) THEN
			RETURN GraphRules.general
		ELSE
			RETURN GraphRules.genDiff
		END;
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.genDiff
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (x: REAL): REAL;
		VAR
			alpha, beta, lambda: REAL;
	BEGIN
		beta := node.beta.value;
		lambda := node.lambda.value;
		alpha := node.alpha.value;
		RETURN MathCumulative.ModifiedWeibull(alpha, beta, lambda, x)
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			alpha, logAlpha, logLikelihood, beta, lambda, z, x: REAL;
		CONST
			eps = 1.0E-20;
	BEGIN
		x := node.value;
		alpha := node.alpha.value;
		logAlpha := Math.Ln(alpha);
		beta := node.beta.value;
		lambda := node.lambda.value;
		z := lambda * x;
		logLikelihood := logAlpha + Math.Ln(beta + z) + (beta - 1) * Math.Ln(x) + z - alpha * Math.Power(x, beta) * Math.Exp(z);
		RETURN - 2 * logLikelihood
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			alpha, beta, lambda, diff, diffAlpha, diffBeta, diffLambda, exp, pow, val: REAL;
	BEGIN
		val := node.value;
		alpha := node.alpha.value;
		beta := node.beta.value;
		lambda := node.lambda.value;
		exp := Math.Exp(lambda * val);
		pow := Math.Power(val, beta);
		diffAlpha := node.alpha.Diff(x);
		diffBeta := node.beta.Diff(x);
		diffLambda := node.lambda.Diff(x);
		pow := Math.Power(val, beta);
		diff := diffAlpha * (1 / alpha - pow * exp) + 
		diffBeta * (1 / (beta + lambda * val) + Math.Ln(val) * (1 - alpha * pow * exp)) + 
		diffLambda * (val / (beta + lambda * val) + val - alpha * val * pow * exp);
		RETURN diff
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			alpha, beta, lambda, exp, pow, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.value;
		beta := node.beta.value;
		exp := Math.Exp(lambda * x);
		pow := Math.Power(x, beta);
		RETURN lambda / (beta + lambda * x) + lambda + (beta - 1 - alpha * pow * exp * (beta + lambda * x)) / x;
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.alpha, wr);
		GraphNodes.Externalize(node.beta, wr);
		GraphNodes.Externalize(node.lambda, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		INCL(node.props, GraphStochastic.leftNatural);
		node.alpha := NIL;
		node.beta := NIL;
		node.lambda := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.alpha := GraphNodes.Internalize(rd);
		node.beta := GraphNodes.Internalize(rd);
		node.lambda := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "ReliabilityModifiedWeibull.Install"
	END Install;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			alpha, beta, lambda: REAL;
		CONST
			eps = 1.0E-20;
	BEGIN
		alpha := node.alpha.value;
		beta := node.beta.value;
		lambda := node.lambda.value;
		HALT(0);
		RETURN 0.0
	END Location;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			alpha, logAlpha, logLikelihood, beta, lambda, z, x: REAL;
		CONST
			eps = 1.0E-20;
	BEGIN
		x := node.value;
		alpha := node.alpha.value;
		logAlpha := Math.Ln(alpha);
		beta := node.beta.value;
		lambda := node.lambda.value;
		z := lambda * x;
		logLikelihood := logAlpha + Math.Ln(beta + z) + (beta - 1) * Math.Ln(x) + z - alpha * Math.Power(x, beta) * Math.Exp(z);
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
	BEGIN
		RETURN node.LogLikelihood()
	END LogPrior;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.alpha.AddParent(list);
		node.beta.AddParent(list);
		node.lambda.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			alpha, beta, lambda, x, lower, upper: REAL;
			bounds: SET;
	BEGIN
		beta := node.beta.value;
		lambda := node.lambda.value;
		alpha := node.alpha.value;
		bounds := node.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		IF bounds = {} THEN
			x := MathRandnum.ModifiedWeibull(alpha, beta, lambda)
		ELSE
			node.Bounds(lower, upper);
			IF bounds = {GraphStochastic.leftImposed} THEN
				x := MathRandnum.ModifiedWeibullLB(alpha, beta, lambda, lower)
			ELSIF bounds = {GraphStochastic.rightImposed} THEN
				x := MathRandnum.ModifiedWeibullRB(alpha, beta, lambda, upper)
			ELSE
				x := MathRandnum.ModifiedWeibullIB(alpha, beta, lambda, lower, upper)
			END
		END;
		node.value := x;
		res := {}
	END Sample;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.alpha := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			node.beta := args.scalars[1];
			ASSERT(args.scalars[2] # NIL, 21);
			node.lambda := args.scalars[2];
		END
	END SetUnivariate;

	PROCEDURE (f: Factory) New (): GraphUnivariate.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "sssCT"
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "Vijay Kumar"
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
END ReliabilityModifiedWeibull.

