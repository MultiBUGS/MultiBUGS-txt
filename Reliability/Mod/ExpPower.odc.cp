(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"






*)
(*  This module has been set up to implement the Exponential Power distribution with shape parameter alpha (a >0)  and  scale parameter lambda(l >0).
R. M. Smith and L. J. Bain,(1975). An exponential power life-testing distribution, Communications Statistics, vol. 4, pp. 469–481.
Usage : dexp.power(alpha, lambda)

*)

MODULE ReliabilityExpPower;


	

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
		maxIts = 100000;
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
		IF node.alpha.value< - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg1}
		END;
		IF node.lambda.value< - eps THEN
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
		RETURN MathCumulative.ExpPower(alpha, lambda, x)
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			alpha, logAlpha, lambda, logLambda, factor, logLikelihood, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.value;
		logAlpha := MathFunc.Ln(alpha);
		lambda := node.lambda.value;
		logLambda := MathFunc.Ln(lambda);
		factor := Math.Power(lambda * x, alpha);
		logLikelihood := logAlpha + alpha * logLambda + (alpha - 1.0) * MathFunc.Ln(x) + factor
		 + 1.0 - Math.Exp(factor);
		RETURN - 2.0 * logLikelihood
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			alpha, lambda, diff, diffAlpha, diffLambda, exp, pow, val: REAL;
	BEGIN
		val := node.value;
		alpha := node.alpha.value;
		lambda := node.lambda.value;
		IF (GraphStochastic.hint2 IN x.props) OR (GraphNodes.data IN node.lambda.props) THEN
			diffAlpha := node.alpha.Diff(x);
			pow := Math.Power(lambda * val, alpha);
			exp := Math.Exp(pow);
			diff := diffAlpha * (1 / alpha + Math.Ln(lambda * val) * (1 + pow * (1 - exp)))
		ELSIF (GraphStochastic.hint1 IN x.props) OR (GraphNodes.data IN node.alpha.props) THEN
			diffLambda := node.lambda.Diff(x);
			pow := Math.Power(lambda * val, alpha);
			exp := Math.Exp(pow);
			diff := diffLambda * (alpha / lambda + alpha * pow * (1 - exp) / lambda)
		ELSE
			diffAlpha := node.alpha.Diff(x);
			diffLambda := node.lambda.Diff(x);
			pow := Math.Power(lambda * val, alpha);
			exp := Math.Exp(pow);
			diff := diffAlpha * (1 / alpha + Math.Ln(lambda * val) * (1 + pow * (1 - exp))) + 
			diffLambda * (alpha / lambda + alpha * pow * (1 - exp) / lambda)
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
		pow := Math.Power(lambda * x, alpha);
		exp := Math.Exp(pow);
		RETURN (alpha - 1) / x + alpha * pow * (1 - exp) / x
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
		install := "ReliabilityExpPower.Install"
	END Install;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			alpha, lambda, median: REAL;
		CONST
			p50 = 0.5;
	BEGIN
		alpha := node.alpha.value;
		lambda := node.lambda.value;
		median := Math.Power((1.0 / lambda) * Math.Ln(1.0 - Math.Ln(1.0 - p50)), 1.0 / alpha);
		RETURN median
	END Location;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			alpha, logAlpha, lambda, logLambda, factor, logLikelihood, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.value;
		logAlpha := MathFunc.Ln(alpha);
		lambda := node.lambda.value;
		logLambda := MathFunc.Ln(lambda);
		factor := Math.Power(lambda * x, alpha);
		logLikelihood := logAlpha + alpha * logLambda + (alpha - 1.0) * MathFunc.Ln(x) + factor
		 + 1.0 - Math.Exp(factor);
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			alpha, lambda, factor, logPrior, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.value;
		lambda := node.lambda.value;
		factor := Math.Power(lambda * x, alpha);
		logPrior := (alpha - 1.0) * MathFunc.Ln(x) + factor - Math.Exp(factor);
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
		IF bounds = {} THEN
			x := MathRandnum.ExpPower(alpha, lambda)
		ELSE
			node.Bounds(lower, upper);
			IF bounds = {GraphStochastic.leftImposed} THEN
				x := MathRandnum.ExpPowerLB(alpha, lambda, lower)
			ELSIF bounds = {GraphStochastic.rightImposed} THEN
				x := MathRandnum.ExpPowerRB(alpha, lambda, upper)
			ELSE
				x := MathRandnum.ExpPowerIB(alpha, lambda, lower, upper)
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
END ReliabilityExpPower.

