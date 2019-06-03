(*	

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"







This template module has been set up to implement the  Log-logistic distribution with beta(shape) and theta(scale) parameters, respectively dlog.logis(beta, theta).

Usage : dlog.logis(beta, theta)

*)

MODULE ReliabilityLogLogistic;


	

	IMPORT
		Math, Stores := Stores64,
		GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE

		Node = POINTER TO RECORD(GraphUnivariate.Node)
			beta, theta: GraphNodes.Node
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
		IF node.beta.Value() < - eps THEN
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
		f0 := GraphStochastic.ClassFunction(node.theta, parent);
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
			beta, theta: REAL;
	BEGIN
		beta := node.beta.Value();
		theta := node.theta.Value();
		RETURN MathCumulative.LogLogistic(beta, theta, x)
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			logDensity, theta, beta, logBeta, factor, x: REAL;
	BEGIN
		x := node.value;
		beta := node.beta.Value();
		theta := node.theta.Value();
		logBeta := MathFunc.Ln(beta);
		factor := Math.Power((x / theta), beta);
		logDensity := logBeta + MathFunc.Ln(factor) - MathFunc.Ln(x) - 2 * MathFunc.Ln(1 + factor);
		RETURN - 2.0 * logDensity
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			beta, theta, diff, diffBeta, diffTheta, log, pow, val: REAL;
	BEGIN
		val := node.value;
		IF (GraphStochastic.hint2 IN x.props) OR (GraphNodes.data IN node.theta.props) THEN
			node.beta.ValDiff(x, beta, diffBeta);
			theta := node.theta.Value();
			pow := Math.Power(val / theta, beta);
			log := Math.Ln(val / theta);
			diff := diffBeta * (1 / beta + log - 2 * pow * log / (1 + pow))
		ELSIF (GraphStochastic.hint1 IN x.props) OR (GraphNodes.data IN node.beta.props) THEN
			beta := node.beta.Value();
			node.theta.ValDiff(x, theta, diffTheta);
			pow := Math.Power(val / theta, beta);
			diff := diffTheta * (beta / theta) * (2 * pow / (1 + pow) - 1)
		ELSE
			node.beta.ValDiff(x, beta, diffBeta);
			node.theta.ValDiff(x, theta, diffTheta);
			pow := Math.Power(val / theta, beta);
			log := Math.Ln(val / theta);
			diff := diffBeta * (1 / beta + log - 2 * pow * log / (1 + pow))
			 + diffTheta * (beta / theta) * (2 * pow / (1 + pow) - 1)
		END;
		RETURN diff
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			beta, theta, pow, x: REAL;
	BEGIN
		x := node.value;
		beta := node.beta.Value();
		theta := node.theta.Value();
		pow := Math.Power(x / theta, beta);
		RETURN (beta - 1) / x - 2 * pow / (x * (1 + pow))
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.beta, wr);
		GraphNodes.Externalize(node.theta, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.leftNatural});
		node.theta := NIL;
	END InitUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.beta := GraphNodes.Internalize(rd);
		node.theta := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "ReliabilityLogLogistic.Install"
	END Install;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			theta, beta, median: REAL;
		CONST
			p50 = 0.5;
	BEGIN
		beta := node.beta.Value();
		theta := node.theta.Value();
		median := theta * Math.Power(p50 / (1 - p50), (1.0 / beta));
		RETURN median
	END Location;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			theta, beta, logBeta, factor, x: REAL;
	BEGIN
		x := node.value;
		beta := node.beta.Value();
		theta := node.theta.Value();
		logBeta := MathFunc.Ln(beta);
		factor := Math.Power((x / theta), beta);
		RETURN logBeta + MathFunc.Ln(factor) - MathFunc.Ln(x) - 2 * MathFunc.Ln(1 + factor)
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			theta, beta, logBeta, factor, x: REAL;
	BEGIN
		x := node.value;
		beta := node.beta.Value();
		theta := node.theta.Value();
		logBeta := MathFunc.Ln(beta);
		factor := Math.Power((x / theta), beta);
		RETURN MathFunc.Ln(factor) - MathFunc.Ln(x) - 2 * MathFunc.Ln(1 + factor)
	END LogPrior;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.beta.AddParent(list);
		node.theta.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			theta, beta, x, lower, upper: REAL;
			bounds: SET;
	BEGIN
		beta := node.beta.Value();
		theta := node.theta.Value();
		bounds := node.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		IF bounds = {} THEN
			x := MathRandnum.Loglogistic(beta, theta)
		ELSE
			node.Bounds(lower, upper);
			IF bounds = {GraphStochastic.leftImposed} THEN
				x := MathRandnum.LoglogisticLB(beta, theta, lower)
			ELSIF bounds = {GraphStochastic.rightImposed} THEN
				x := MathRandnum.LoglogisticRB(beta, theta, upper)
			ELSE
				x := MathRandnum.LoglogisticIB(beta, theta, lower, upper)
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
END ReliabilityLogLogistic.
