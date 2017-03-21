(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)
(*  This module has been set up to implement the Generalized Power Weibull distribution with shape parameters alpha (a >0)  and  theta(q >0).

Usage : dgp.weib(alpha, theta)

*)

MODULE ReliabilityGPWeibull;


	

	IMPORT
		Math, Stores,
		GraphNodes, GraphParamtrans, GraphRules, GraphStochastic, GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE

		Node = POINTER TO RECORD(GraphUnivariate.Node)
			alpha, theta: GraphNodes.Node
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
		IF node.theta.Value() < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg2}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
		VAR
			f0, f1: INTEGER;
	BEGIN
		f0 := GraphStochastic.ClassFunction(node.alpha, parent);
		f1 := GraphStochastic.ClassFunction(node.theta, parent);
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
			alpha, theta: REAL;
	BEGIN
		alpha := node.alpha.Value();
		theta := node.theta.Value();
		RETURN MathCumulative.GPWeibull(alpha, theta, x)
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			alpha, logAlpha, theta, logTheta, factor, logFactor, logLikelihood, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		logAlpha := MathFunc.Ln(alpha);
		theta := node.theta.Value();
		logTheta := MathFunc.Ln(theta);
		factor := 1.0 + Math.Power(x, alpha);
		logFactor := MathFunc.Ln(factor);
		logLikelihood := logAlpha + logTheta + (alpha - 1.0) * MathFunc.Ln(x) + (theta - 1.0) * logFactor
		 + 1.0 - Math.Power(factor, theta);
		RETURN - 2.0 * logLikelihood
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			alpha, theta, diff, diffAlpha, diffTheta, log, pow, val: REAL;
	BEGIN
		val := node.value;
		IF (GraphStochastic.hint2 IN x.props) OR (GraphNodes.data IN node.theta.props) THEN
			node.alpha.ValDiff(x, alpha, diffAlpha);
			theta := node.theta.Value();
			pow := Math.Power(val, alpha);
			log := Math.Ln(val);
			diff := diffAlpha * (1 / alpha + log + (theta - 1) * pow * log / (1 + pow)
			 - theta * Math.Power(1 + pow, theta - 1) * pow * log)
		ELSIF (GraphStochastic.hint1 IN x.props) OR (GraphNodes.data IN node.alpha.props) THEN
			alpha := node.alpha.Value();
			node.theta.ValDiff(x, theta, diffTheta);
			pow := Math.Power(val, alpha);
			diff := diffTheta * (1 / theta + Math.Ln(1 + pow) * (1 - Math.Power(1 + pow, theta)))
		ELSE
			node.alpha.ValDiff(x, alpha, diffAlpha);
			node.theta.ValDiff(x, theta, diffTheta);
			pow := Math.Power(val, alpha);
			log := Math.Ln(val);
			diff := diffAlpha * (1 / alpha + log + (theta - 1) * pow * log / (1 + pow)
			 - theta * Math.Power(1 + pow, theta - 1) * pow * log)
			 + diffTheta * (1 / theta + Math.Ln(1 + pow) * (1 - Math.Power(1 + pow, theta)))
		END;
		RETURN diff
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			alpha, theta, pow, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		theta := node.theta.Value();
		pow := Math.Power(x, alpha);
		RETURN (alpha - 1) / x + (theta - 1) * alpha * pow / (x * (1 + pow)) - 
		theta * Math.Power(1 + pow, theta - 1) * alpha * pow / x
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.alpha, wr);
		GraphNodes.Externalize(node.theta, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.leftNatural});
		node.theta := NIL;
		node.alpha := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.alpha := GraphNodes.Internalize(rd);
		node.theta := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "ReliabilityGPWeibull.Install"
	END Install;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			alpha, theta, median, u: REAL;
		CONST
			p50 = 0.5;
	BEGIN
		alpha := node.alpha.Value();
		theta := node.theta.Value();
		u := Math.Ln(1.0 - p50);
		median := Math.Power(Math.Power((1.0 - u), 1.0 / theta) - 1.0, 1.0 / alpha);
		RETURN median
	END Location;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			alpha, logAlpha, theta, logTheta, factor, logFactor, logLikelihood, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		logAlpha := MathFunc.Ln(alpha);
		theta := node.theta.Value();
		logTheta := MathFunc.Ln(theta);
		factor := 1.0 + Math.Power(x, alpha);
		logFactor := MathFunc.Ln(factor);
		logLikelihood := logAlpha + logTheta + (alpha - 1.0) * MathFunc.Ln(x) + (theta - 1.0) * logFactor
		 + 1.0 - Math.Power(factor, theta);
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			alpha, logAlpha, theta, logTheta, factor, logFactor, logPrior, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		logAlpha := MathFunc.Ln(alpha);
		theta := node.theta.Value();
		logTheta := MathFunc.Ln(theta);
		factor := 1.0 + Math.Power(x, alpha);
		logFactor := MathFunc.Ln(factor);
		logPrior := (alpha - 1.0) * MathFunc.Ln(x) + (theta - 1.0) * logFactor - Math.Power(factor, theta);
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.alpha.AddParent(list);
		node.theta.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			alpha, theta, x, lower, upper: REAL;
			bounds: SET;
	BEGIN
		alpha := node.alpha.Value();
		theta := node.theta.Value();
		bounds := node.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		IF bounds = {} THEN
			x := MathRandnum.GPWeibull(alpha, theta)
		ELSE
			node.Bounds(lower, upper);
			IF bounds = {GraphStochastic.leftImposed} THEN
				x := MathRandnum.GPWeibullLB(alpha, theta, lower)
			ELSIF bounds = {GraphStochastic.rightImposed} THEN
				x := MathRandnum.GPWeibullRB(alpha, theta, upper)
			ELSE
				x := MathRandnum.GPWeibullIB(alpha, theta, lower, upper)
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
			node.theta := args.scalars[1]
		END
	END SetUnivariate;

	PROCEDURE (node: Node) ModifyUnivariate (): GraphUnivariate.Node;
		VAR
			p: Node;
	BEGIN
		NEW(p);
		p^ := node^;
		p.alpha := GraphParamtrans.LogTransform(p.alpha);
		p.theta := GraphParamtrans.LogTransform(p.theta);
		RETURN p
	END ModifyUnivariate;

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
END ReliabilityGPWeibull.

