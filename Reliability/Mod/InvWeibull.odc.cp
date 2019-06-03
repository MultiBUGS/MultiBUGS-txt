(*			

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)
(*  This module has been set up to implement the Generalized Exponential distribution with shape parameter beta (b >0)  and  scale parameter lambda(l >0).
R. Jiang and D. N. P. Murthy, “Models involving two inverse Weibull distributions,” Reliability Engineering and System Safety, vol. 73, pp. 73–81, 2001.

Usage : dinv.weib(beta, lambda)

*)

MODULE ReliabilityInvWeibull;


	

	IMPORT
		Math, Stores := Stores64,
		GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE

		Node = POINTER TO RECORD(GraphUnivariate.Node)
			lambda, beta: GraphNodes.Node
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
		IF node.beta.Value() < - eps THEN
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
		f0 := GraphStochastic.ClassFunction(node.lambda, parent);
		f1 := GraphStochastic.ClassFunction(node.beta, parent);
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
			lambda, beta: REAL;
	BEGIN
		lambda := node.lambda.Value();
		beta := node.beta.Value();
		RETURN MathCumulative.InvWeibull(beta, lambda, x)
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			beta, logBeta, lambda, logLambda, factor, logLikelihood, x: REAL;
	BEGIN
		x := node.value;
		beta := node.beta.Value();
		logBeta := MathFunc.Ln(beta);
		lambda := node.lambda.Value();
		logLambda := MathFunc.Ln(lambda);
		factor := Math.Power((x / lambda), - beta);
		logLikelihood := logBeta + beta * logLambda - (beta + 1.0) * MathFunc.Ln(x) - factor;
		RETURN - 2.0 * logLikelihood
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			beta, lambda, diff, diffBeta, diffLambda, pow, val: REAL;
	BEGIN
		val := node.value;
		IF (GraphStochastic.hint2 IN x.props) OR (GraphNodes.data IN node.lambda.props) THEN
			node.beta.ValDiff(x, beta, diffBeta);
			lambda := node.lambda.Value();
			pow := Math.Power(lambda / val, beta);
			diff := diffBeta * (1 / beta + Math.Ln(lambda / val) * (1 - pow))
		ELSIF (GraphStochastic.hint1 IN x.props) OR (GraphNodes.data IN node.beta.props) THEN
			beta := node.beta.Value();
			node.lambda.ValDiff(x, lambda, diffLambda);
			pow := Math.Power(lambda / val, beta);
			diff := diffLambda * (beta / lambda) * (1 - pow)
		ELSE
			node.beta.ValDiff(x, beta, diffBeta);
			node.lambda.ValDiff(x, lambda, diffLambda);
			pow := Math.Power(lambda / val, beta);
			diff := diffBeta * (1 / beta + Math.Ln(lambda / val) * (1 - pow)) + 
			diffLambda * (beta / lambda) * (1 - pow)
		END;
		RETURN diff
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			beta, lambda, pow, x: REAL;
	BEGIN
		x := node.value;
		beta := node.beta.Value();
		lambda := node.lambda.Value();
		pow := Math.Power(lambda / x, beta);
		RETURN - (beta + 1) / x + beta * pow / x
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.beta, wr);
		GraphNodes.Externalize(node.lambda, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.leftNatural});
		node.lambda := NIL;
		node.beta := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.beta := GraphNodes.Internalize(rd);
		node.lambda := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "ReliabilityInvWeibull.Install"
	END Install;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			beta, lambda, median: REAL;
		CONST
			p50 = 0.5;
	BEGIN
		beta := node.beta.Value();
		lambda := node.lambda.Value();
		median := lambda * Math.Power( - Math.Ln(p50), - 1.0 / beta);
		RETURN median
	END Location;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			beta, logBeta, lambda, logLambda, factor, logLikelihood, x: REAL;
	BEGIN
		x := node.value;
		beta := node.beta.Value();
		logBeta := MathFunc.Ln(beta);
		lambda := node.lambda.Value();
		logLambda := MathFunc.Ln(lambda);
		factor := Math.Power((x / lambda), - beta);
		logLikelihood := logBeta + beta * logLambda - (beta + 1.0) * MathFunc.Ln(x) - factor;
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			beta, lambda, factor, logPrior, x: REAL;
	BEGIN
		x := node.value;
		beta := node.beta.Value();
		lambda := node.lambda.Value();
		factor := Math.Power((x / lambda), - beta);
		logPrior := - (beta + 1.0) * MathFunc.Ln(x) - factor;
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.beta.AddParent(list);
		node.lambda.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			lambda, beta, x, lower, upper: REAL;
			bounds: SET;
	BEGIN
		beta := node.beta.Value();
		lambda := node.lambda.Value();
		bounds := node.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		IF bounds = {} THEN
			x := MathRandnum.InvWeibull(beta, lambda)
		ELSE
			node.Bounds(lower, upper);
			IF bounds = {GraphStochastic.leftImposed} THEN
				x := MathRandnum.InvWeibullLB(beta, lambda, lower)
			ELSIF bounds = {GraphStochastic.rightImposed} THEN
				x := MathRandnum.InvWeibullRB(beta, lambda, upper)
			ELSE
				x := MathRandnum.InvWeibullIB(beta, lambda, lower, upper)
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
			node.beta := args.scalars[0];
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
END ReliabilityInvWeibull.

