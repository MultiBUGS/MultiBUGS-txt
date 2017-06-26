(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)
(*  This template module has been set up to implement the  Exponentiated Weibull distribution with two shape parameters alpha & theta.

Usage : dexp.weib(alpha, theta)

*)

MODULE ReliabilityExpoWeibull;



	

	IMPORT
		Math, Stores,
		GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE

		Node = POINTER TO RECORD(GraphUnivariate.Node)
			alpha, theta: GraphNodes.Node
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
		RETURN MathCumulative.ExpoWeibull(alpha, theta, x)
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			logFactor, logDensity, alpha, theta, logAlpha, logTheta, factor, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		theta := node.theta.Value();
		logAlpha := MathFunc.Ln(alpha);
		logTheta := MathFunc.Ln(theta);
		factor := Math.Exp( - Math.Power(x, alpha));
		logFactor := MathFunc.Ln(1 - factor);
		logDensity := logAlpha + logTheta + (alpha - 1) * MathFunc.Ln(x) - Math.Power(x, alpha)
		 + (theta - 1) * logFactor;
		RETURN - 2.0 * logDensity
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			alpha, theta, diff, diffAlpha, diffTheta, exp, pow, val: REAL;
	BEGIN
		val := node.value;
		IF (GraphStochastic.hint2 IN x.props) OR (GraphNodes.data IN node.theta.props) THEN
			node.alpha.ValDiff(x, alpha, diffAlpha);
			theta := node.theta.Value();
			pow := Math.Power(val, alpha);
			exp := Math.Exp( - pow);
			diff := diffAlpha * (1 / alpha + Math.Ln(val) - pow * Math.Ln(val)
			 + (theta - 1) * exp * pow * Math.Ln(val) / (1 - exp))
		ELSIF (GraphStochastic.hint1 IN x.props) OR (GraphNodes.data IN node.alpha.props) THEN
			alpha := node.alpha.Value();
			node.theta.ValDiff(x, theta, diffTheta);
			pow := Math.Power(val, alpha);
			exp := Math.Exp( - pow);
			diff := diffTheta * (1 / theta + Math.Ln(1 - exp))
		ELSE
			node.alpha.ValDiff(x, alpha, diffAlpha);
			node.theta.ValDiff(x, theta, diffTheta);
			pow := Math.Power(val, alpha);
			exp := Math.Exp( - pow);
			diff := diffAlpha * (1 / alpha + Math.Ln(val) - pow * Math.Ln(val)
			 + (theta - 1) * exp * pow * Math.Ln(val) / (1 - exp))
			 + diffTheta * (1 / theta + Math.Ln(1 - exp))
		END;
		RETURN diff
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			alpha, theta, exp, pow, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		theta := node.theta.Value();
		pow := Math.Power(x, alpha);
		exp := Math.Exp( - pow);
		RETURN (alpha - 1) / x - alpha * pow / x + (theta - 1) * alpha * exp * pow / (x * (1 - exp))
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
		install := "ReliabilityExpoWeibull.Install"
	END Install;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			alpha, theta, mean: REAL;
		CONST
			p50 = 0.5;
	BEGIN
		alpha := node.alpha.Value();
		theta := node.theta.Value();
		mean := Math.Power( - Math.Ln(1 - Math.Power(p50, 1.0 / theta)), 1.0 / alpha);
		RETURN mean
	END Location;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			logFactor, alpha, theta, logAlpha, logTheta, factor, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		theta := node.theta.Value();
		logAlpha := MathFunc.Ln(alpha);
		logTheta := MathFunc.Ln(theta);
		factor := Math.Exp( - Math.Power(x, alpha));
		logFactor := MathFunc.Ln(1 - factor);
		RETURN logAlpha + logTheta + (alpha - 1) * MathFunc.Ln(x) - Math.Power(x, alpha)
		 + (theta - 1) * logFactor
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			logFactor, alpha, theta, factor, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		theta := node.theta.Value();
		factor := Math.Exp( - Math.Power(x, alpha));
		logFactor := MathFunc.Ln(1 - factor);
		RETURN (alpha - 1) * MathFunc.Ln(x) - Math.Power(x, alpha) + (theta - 1) * logFactor
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
			theta, alpha, x, lower, upper: REAL;
			bounds: SET;
	BEGIN
		alpha := node.alpha.Value();
		theta := node.theta.Value();
		bounds := node.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		IF bounds = {} THEN
			x := MathRandnum.ExpoWeibull(alpha, theta)
		ELSE
			node.Bounds(lower, upper);
			IF bounds = {GraphStochastic.leftImposed} THEN
				x := MathRandnum.ExpoWeibullLB(alpha, theta, lower)
			ELSIF bounds = {GraphStochastic.rightImposed} THEN
				x := MathRandnum.ExpoWeibullRB(alpha, theta, upper)
			ELSE
				x := MathRandnum.ExpoWeibullIB(alpha, theta, lower, upper);
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
END ReliabilityExpoWeibull.

