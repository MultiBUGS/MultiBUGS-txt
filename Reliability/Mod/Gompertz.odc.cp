(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)
(*  This template module has been set up to implement the  Gompertz distribution with parameters alpha & theta.

Usage : dgpz(alpha, theta)

*)

MODULE ReliabilityGompertz;


	

	IMPORT
		Math, Stores := Stores64,
		GraphConjugateUV, GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE

		Node = POINTER TO RECORD(GraphConjugateUV.Node)
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
		IF node.alpha.value < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg1}
		END;
		IF node.theta.value < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg2}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
		VAR
			density, density0, density1, f0, f1: INTEGER;
	BEGIN
		f0 := GraphStochastic.ClassFunction(node.alpha, parent);
		f1 := GraphStochastic.ClassFunction(node.theta, parent);
		CASE f0 OF
		|GraphRules.const:
			density0 := GraphRules.unif
		|GraphRules.other:
			density0 := GraphRules.general
		ELSE
			density0 := GraphRules.genDiff
		END;
		density1 := GraphRules.ClassifyPrecision(f1);
		IF density0 = GraphRules.unif THEN
			density := density1
		ELSIF density1 = GraphRules.unif THEN
			density := density0
		ELSIF (density0 # GraphRules.general) & (density1 # GraphRules.general) THEN
			density := GraphRules.genDiff
		ELSE
			density := GraphRules.general
		END;
		RETURN density
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.genDiff
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (x: REAL): REAL;
		VAR
			alpha, theta: REAL;
	BEGIN
		alpha := node.alpha.value;
		theta := node.theta.value;
		RETURN MathCumulative.Gompertz(alpha, theta, x)
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			alpha, theta, logTheta, logDensity, factor, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.value;
		theta := node.theta.value;
		logTheta := MathFunc.Ln(theta);
		factor := 1.0 - Math.Exp(x * alpha);
		logDensity := logTheta + alpha * x + (theta / alpha) * factor;
		RETURN - 2.0 * logDensity
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			alpha, theta, diff, diffAlpha, diffTheta, exp, val: REAL;
	BEGIN
		val := node.value;
		alpha := node.alpha.value;
		theta := node.theta.value;
		IF (GraphStochastic.hint2 IN x.props) OR (GraphNodes.data IN node.theta.props) THEN
			diffAlpha := node.alpha.Diff(x);
			exp := Math.Exp(alpha * val);
			diff := diffAlpha * (val - theta * val * exp / alpha - theta * (1 - exp) / (alpha * alpha))
		ELSIF (GraphStochastic.hint1 IN x.props) OR (GraphNodes.data IN node.alpha.props) THEN
			diffTheta := node.theta.Diff(x);
			exp := Math.Exp(alpha * val);
			diff := diffTheta * (1 / theta + (1 - exp) / alpha)
		ELSE
			diffAlpha := node.alpha.Diff(x);
			diffTheta := node.theta.Diff(x);
			exp := Math.Exp(alpha * val);
			diff := diffAlpha * (val - theta * val * exp / alpha - theta * (1 - exp) / (alpha * alpha))
			 + diffTheta * (1 / theta + (1 - exp) / alpha)
		END;
		RETURN diff
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			alpha, theta, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.value;
		theta := node.theta.value;
		RETURN alpha - theta * Math.Exp(alpha * x)
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.alpha, wr);
		GraphNodes.Externalize(node.theta, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		INCL(node.props, GraphStochastic.leftNatural);
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
		install := "ReliabilityGompertz.Install"
	END Install;

	PROCEDURE (likelihood: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
		VAR
			alpha,value: REAL;
	BEGIN
		ASSERT(as = GraphRules.gamma, 21);
		p0 := 1.0;
		alpha := likelihood.alpha.value;
		value := likelihood.value;
		p1 := -(1.0 - Math.Exp(alpha * value)) / alpha;
		x := likelihood.theta
	END LikelihoodForm;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			alpha, theta, median: REAL;
		CONST
			p50 = 0.5;
	BEGIN
		alpha := node.alpha.value;
		theta := node.theta.value;
		median := (1.0 / alpha) * Math.Ln(1.0 - (alpha / theta) * Math.Ln(1.0 - p50));
		RETURN median
	END Location;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			alpha, theta, logTheta, logLikelihood, factor, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.value;
		theta := node.theta.value;
		logTheta := MathFunc.Ln(theta);
		factor := 1.0 - Math.Exp(x * alpha);
		logLikelihood := logTheta + alpha * x + (theta / alpha) * factor;
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			alpha, theta, logPrior, factor, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.value;
		theta := node.theta.value;
		factor := 1.0 - Math.Exp(x * alpha);
		logPrior := alpha * x + (theta / alpha) * factor;
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

	PROCEDURE (prior: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		HALT(0)
	END PriorForm;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			theta, alpha, x, lower, upper: REAL;
			bounds: SET;
	BEGIN
		alpha := node.alpha.value;
		theta := node.theta.value;
		bounds := node.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		IF bounds = {} THEN
			x := MathRandnum.Gompertz(alpha, theta)
		ELSE
			node.Bounds(lower, upper);
			IF bounds = {GraphStochastic.leftImposed} THEN
				x := MathRandnum.GompertzLB(alpha, theta, lower)
			ELSIF bounds = {GraphStochastic.rightImposed} THEN
				x := MathRandnum.GompertzRB(alpha, theta, upper)
			ELSE
				x := MathRandnum.GompertzIB(alpha, theta, lower, upper)
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
END ReliabilityGompertz.

