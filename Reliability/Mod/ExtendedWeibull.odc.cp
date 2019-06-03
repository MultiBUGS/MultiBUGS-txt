(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)
(*  This module has been set up to implement the extented Weibull distribution with shape parameter alpha (a >0) and  'tilt'  parameter lambda(l >0).
Marshall, A. W., Olkin, I. (1997). A new method for adding a parameter to a family of distributions with application to the exponential and Weibull families. Biometrika 84(3):641–652.
Marshall, A. W., Olkin, I.(2007). Life Distributions: Structure of Nonparametric, Semiparametric, and Parametric Families. Springer, New York.

Usage : dext.weib(alpha, lambda)

*)

MODULE ReliabilityExtendedWeibull;


	

	IMPORT
		Math, Stores := Stores64,
		GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE

		Node = POINTER TO RECORD(GraphUnivariate.Node)
			alpha, lambda: GraphNodes.Node
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
		IF node.alpha.Value() < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg1}
		END;
		IF node.lambda.Value() < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg2}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
		VAR
			f0, f1: INTEGER;
	BEGIN
		f0 := GraphStochastic.ClassFunction(node.alpha, parent);
		f1 := GraphStochastic.ClassFunction(node.lambda, parent);
		IF (f0 = GraphRules.const) & (f1 = GraphRules.const) THEN
			RETURN GraphRules.unif
		ELSIF (f0 = GraphRules.other) OR (f1 = GraphRules.other) THEN
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
			alpha, lambda: REAL;
	BEGIN
		alpha := node.alpha.Value();
		lambda := node.lambda.Value();
		RETURN MathCumulative.ExtendedWeibull(alpha, lambda, x)
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			alpha, logAlpha, lambda, logLambda, factor, logFactor, logLikelihood, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		logAlpha := MathFunc.Ln(alpha);
		lambda := node.lambda.Value();
		logLambda := MathFunc.Ln(lambda);
		factor := Math.Exp( - Math.Power(x, alpha));
		logFactor := - Math.Power(x, alpha);
		logLikelihood := logAlpha + logLambda + (alpha - 1.0) * MathFunc.Ln(x) + logFactor - 
		2 * MathFunc.Ln(1.0 - (1.0 - lambda) * factor);
		RETURN - 2.0 * logLikelihood
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			alpha, lambda, diff, diffAlpha, diffLambda, exp, pow, log, val: REAL;
	BEGIN
		val := node.value;
		IF (GraphStochastic.hint2 IN x.props) OR (GraphNodes.data IN node.lambda.props) THEN
			node.alpha.ValDiff(x, alpha, diffAlpha);
			lambda := node.lambda.Value();
			pow := Math.Power(val, alpha);
			exp := Math.Exp( - pow);
			log := Math.Ln(val);
			diff := diffAlpha * (1 / alpha + log - pow * log * (1 + (1 - lambda) * exp) / (1 - (1 - lambda) * exp))
		ELSIF (GraphStochastic.hint1 IN x.props) OR (GraphNodes.data IN node.alpha.props) THEN
			alpha := node.alpha.Value();
			node.lambda.ValDiff(x, lambda, diffLambda);
			pow := Math.Power(val, alpha);
			exp := Math.Exp( - pow);
			diff := diffLambda * (1 / lambda - 2 / (exp - (1 - lambda)))
		ELSE
			node.alpha.ValDiff(x, alpha, diffAlpha);
			node.lambda.ValDiff(x, lambda, diffLambda);
			pow := Math.Power(val, alpha);
			exp := Math.Exp( - pow);
			diff := diffAlpha * (1 / alpha + log - pow * log * (1 + (1 - lambda) * exp) / (1 - (1 - lambda) * exp)) + 
			diffLambda * (1 / lambda - 2 / (exp - (1 - lambda)))
		END;
		RETURN diff
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			alpha, lambda, exp, pow, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		lambda := node.lambda.Value();
		pow := Math.Power(x, alpha);
		exp := Math.Exp( - pow);
		RETURN (alpha - 1) / x + alpha * pow * (1 + (1 - lambda) * exp) / ((1 - (1 - lambda) * exp) * x)
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.alpha, wr);
		GraphNodes.Externalize(node.lambda, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.leftNatural});
		node.lambda := NIL;
		node.alpha := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.alpha := GraphNodes.Internalize(rd);
		node.lambda := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "ReliabilityExtendedWeibull.Install"
	END Install;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			alpha, lambda, mean: REAL;
		CONST
			p50 = 0.5;
	BEGIN
		alpha := node.alpha.Value();
		lambda := node.lambda.Value();
		mean := Math.Power(Math.Ln(1.0 + (lambda * p50 / (1.0 - p50))), (1.0 / alpha));
		RETURN mean
	END Location;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			alpha, logAlpha, lambda, logLambda, factor, logFactor, logLikelihood, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		logAlpha := MathFunc.Ln(alpha);
		lambda := node.lambda.Value();
		logLambda := MathFunc.Ln(lambda);
		factor := Math.Exp( - Math.Power(x, alpha));
		logFactor := - Math.Power(x, alpha);
		logLikelihood := logAlpha + logLambda + (alpha - 1.0) * MathFunc.Ln(x) + logFactor - 
		2 * MathFunc.Ln(1.0 - (1.0 - lambda) * factor);
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			alpha, lambda, factor, logFactor, logPrior, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		lambda := node.lambda.Value();
		factor := Math.Exp( - Math.Power(x, alpha));
		logFactor := - Math.Power(x, alpha);
		logPrior := (alpha - 1.0) * MathFunc.Ln(x) + logFactor - 2 * MathFunc.Ln(1.0 - (1.0 - lambda) * factor);
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.alpha.AddParent(list);
		node.lambda.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			alpha, lambda, x, lower, upper: REAL;
			bounds: SET;
	BEGIN
		alpha := node.alpha.Value();
		lambda := node.lambda.Value();
		bounds := node.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		IF bounds = {} THEN
			x := MathRandnum.ExtendedWeibull(alpha, lambda)
		ELSE
			node.Bounds(lower, upper);
			IF bounds = {GraphStochastic.leftImposed} THEN
				x := MathRandnum.ExtendedWeibullLB(alpha, lambda, lower)
			ELSIF bounds = {GraphStochastic.rightImposed} THEN
				x := MathRandnum.ExtendedWeibullRB(alpha, lambda, upper)
			ELSE
				x := MathRandnum.ExtendedWeibullIB(alpha, lambda, lower, upper)
			END
		END;
		node.SetValue(x);
		res := {}
	END Sample;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.alpha := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			node.lambda := args.scalars[1]
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
		signature := "ssCT"
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "Vijay Kumar."
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
END ReliabilityExtendedWeibull.

