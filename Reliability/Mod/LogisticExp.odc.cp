(*			    

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)
(*  This module has been set up to implement the Logistic-Exponential distribution with  shape parameter alpha (a >0)  and scale parameter lambda(l >0).
Lan, Y. and Leemis, L. M. (2008). : The Log-Exponential Distribution. Naval Research Logistics, Vol 55, pp. 252-264.

Usage : dlogistic.exp(alpha, lambda)

*)

MODULE ReliabilityLogisticExp;


	

	IMPORT
		Math, Stores := Stores64,
		GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE

		Node = POINTER TO RECORD(GraphUnivariate.Node)
			lambda, alpha: GraphNodes.Node
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
			lambda, alpha: REAL;
	BEGIN
		lambda := node.lambda.value;
		alpha := node.alpha.value;
		RETURN MathCumulative.LogisticExp(alpha, lambda, x)
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			alpha, logAlpha, lambda, logLambda, factor, logFactor, logLikelihood, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.value;
		logAlpha := MathFunc.Ln(alpha);
		lambda := node.lambda.value;
		logLambda := MathFunc.Ln(lambda);
		factor := Math.Exp(lambda * x) - 1.0;
		logFactor := MathFunc.Ln(factor);
		logLikelihood := logAlpha + logLambda + lambda * x + (alpha - 1.0) * logFactor - 
		2.0 * MathFunc.Ln(1.0 + Math.Power(factor, alpha));
		RETURN - 2.0 * logLikelihood
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			alpha, lambda, diff, diffAlpha, diffLambda, exp, log, pow, val: REAL;
	BEGIN
		val := node.value;
		alpha := node.alpha.value;
		lambda := node.lambda.value;
		IF (GraphStochastic.hint2 IN x.props) OR (GraphNodes.data IN node.lambda.props) THEN
			diffAlpha := node.alpha.Diff(x);
			exp := Math.Exp(lambda * val);
			pow := Math.Power(exp - 1, alpha);
			log := Math.Ln(exp - 1);
			diff := diffAlpha * (1 / alpha + log - 2 * pow * log / (1 + pow))
		ELSIF (GraphStochastic.hint1 IN x.props) OR (GraphNodes.data IN node.alpha.props) THEN
			diffLambda := node.lambda.Diff(x);
			exp := Math.Exp(lambda * val);
			pow := Math.Power(exp - 1, alpha);
			diff := diffLambda * (1 / lambda + val + (alpha - 1) * val * exp / (exp - 1)
			 - 2 * alpha * val * exp * pow / ((exp - 1) * (1 + pow)))
		ELSE
			diffAlpha := node.alpha.Diff(x);
			diffLambda := node.lambda.Diff(x);
			exp := Math.Exp(lambda * val);
			pow := Math.Power(exp - 1, alpha);
			log := Math.Ln(exp - 1);
			diff := diffAlpha * (1 / alpha + log - 2 * pow * log / (1 + pow)) + 
			diffLambda * (1 / lambda + val + (alpha - 1) * val * exp / (exp - 1)
			 - 2 * alpha * val * exp * pow / ((exp - 1) * (1 + pow)))
		END;
		RETURN diff
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			alpha, lambda, exp, pow, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.value;
		lambda := node.lambda.value;
		exp := Math.Exp(lambda * x);
		pow := Math.Power(exp - 1, alpha);
		RETURN lambda + (alpha - 1) * lambda * exp / (exp - 1) - 
		2 * alpha * lambda * exp * pow / ((exp - 1) * (1 + pow))
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.alpha, wr);
		GraphNodes.Externalize(node.lambda, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		INCL(node.props, GraphStochastic.leftNatural);
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
		install := "ReliabilityLogisticExp.Install"
	END Install;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			alpha, lambda, median: REAL;
		CONST
			p50 = 0.5;
	BEGIN
		alpha := node.alpha.value;
		lambda := node.lambda.value;
		median := (1.0 / lambda) * Math.Ln(1 + Math.Power(p50 / (1.0 - p50), 1.0 / alpha));
		RETURN median
	END Location;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			alpha, logAlpha, lambda, logLambda, factor, logFactor, logLikelihood, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.value;
		logAlpha := MathFunc.Ln(alpha);
		lambda := node.lambda.value;
		logLambda := MathFunc.Ln(lambda);
		factor := Math.Exp(lambda * x) - 1.0;
		logFactor := MathFunc.Ln(factor);
		logLikelihood := logAlpha + logLambda + lambda * x + (alpha - 1.0) * logFactor
		 - 2.0 * MathFunc.Ln(1.0 + Math.Power(factor, alpha));
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			alpha, lambda, factor, logFactor, logPrior, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.value;
		lambda := node.lambda.value;
		factor := Math.Exp(lambda * x) - 1.0;
		logFactor := MathFunc.Ln(factor);
		logPrior := lambda * x + (alpha - 1.0) * logFactor - 2.0 * MathFunc.Ln(1.0 + Math.Power(factor, alpha));
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
			lambda, alpha, x, lower, upper: REAL;
			bounds: SET;
	BEGIN
		alpha := node.alpha.value;
		lambda := node.lambda.value;
		bounds := node.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		IF bounds = {} THEN
			x := MathRandnum.LogisticExponential(alpha, lambda)
		ELSE
			node.Bounds(lower, upper);
			IF bounds = {GraphStochastic.leftImposed} THEN
				x := MathRandnum.LogisticExponentialLB(alpha, lambda, lower)
			ELSIF bounds = {GraphStochastic.rightImposed} THEN
				x := MathRandnum.LogisticExponentialRB(alpha, lambda, upper)
			ELSE
				x := MathRandnum.LogisticExponentialIB(alpha, lambda, lower, upper)
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
END ReliabilityLogisticExp.

