(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)

MODULE GraphWeibull;


	

	IMPORT
		Math, Stores := Stores64,
		GraphConjugateUV, GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathFunc, MathRandnum;

	TYPE

		Node = POINTER TO RECORD(GraphConjugateUV.Node)
			nu, lambda: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD (GraphUnivariate.Factory) END;

	CONST
		eps = 1.0E-10;

	VAR
		fact-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
		VAR
			density, density0, density1, f0, f1: INTEGER;
	BEGIN
		f0 := GraphStochastic.ClassFunction(node.nu, parent);
		density0 := GraphRules.ClassifyShape(f0);
		f1 := GraphStochastic.ClassFunction(node.lambda, parent);
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
		VAR
			class: INTEGER;
	BEGIN
		IF (GraphNodes.data IN node.nu.props) & (node.nu.Value() >= 1.0) THEN
			class := GraphRules.logCon
		ELSE
			class := GraphRules.general
		END;
		RETURN class
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (x: REAL): REAL;
		VAR
			cumulative, lambda, nu: REAL;
	BEGIN
		lambda := node.lambda.Value();
		nu := node.nu.Value();
		cumulative := 1.0 - Math.Exp( - lambda * Math.Power(x, nu));
		RETURN cumulative
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			lambda, logLambda, logLikelihood, logNu, nu, x: REAL;
	BEGIN
		x := node.value;
		nu := node.nu.Value();
		logNu := MathFunc.Ln(nu);
		lambda := node.lambda.Value();
		logLambda := MathFunc.Ln(lambda);
		logLikelihood := logLambda + logNu + (nu - 1.0) * Math.Ln(x) - lambda * Math.Power(x, nu);
		RETURN - 2.0 * logLikelihood
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			lambda, diffLambda, differential, diffNu, nu, val, pow: REAL;
	BEGIN
		val := node.value;
		IF (GraphStochastic.hint2 IN x.props) OR (GraphNodes.data IN node.lambda.props) THEN
			node.nu.ValDiff(x, nu, diffNu);
			lambda := node.lambda.Value();
			pow := Math.Power(val, nu);
			differential := diffNu * (1 / nu + Math.Ln(val) * (1 - lambda * pow))
		ELSIF (GraphStochastic.hint1 IN x.props) OR (GraphNodes.data IN node.nu.props) THEN
			nu := node.nu.Value();
			node.lambda.ValDiff(x, lambda, diffLambda);
			pow := Math.Power(val, nu);
			differential := diffLambda * (1 / lambda - pow)
		ELSE
			node.nu.ValDiff(x, nu, diffNu);
			node.lambda.ValDiff(x, lambda, diffLambda);
			pow := Math.Power(val, nu);
			differential := diffNu * (1 / nu + Math.Ln(val) * (1 - lambda * pow)) + diffLambda * (1 / lambda - pow)
		END;
		RETURN differential
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			lambda, differential, nu, x: REAL;
	BEGIN
		x := node.value;
		nu := node.nu.Value();
		lambda := node.lambda.Value();
		differential := (nu - 1.0) / x - lambda * nu * Math.Power(x, nu - 1);
		RETURN differential
	END DiffLogPrior;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphWeibull.Install"
	END Install;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			lambda, logLambda, logLikelihood, logNu, nu, x: REAL;
	BEGIN
		x := node.value;
		nu := node.nu.Value();
		logNu := MathFunc.Ln(nu);
		lambda := node.lambda.Value();
		logLambda := MathFunc.Ln(lambda);
		logLikelihood := logLambda + logNu + (nu - 1.0) * Math.Ln(x) - lambda * Math.Power(x, nu);
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
		VAR
			nu: REAL;
	BEGIN
		ASSERT(as = GraphRules.gamma, 21);
		p0 := 1;
		nu := node.nu.Value();
		p1 := Math.Power(node.value, nu);
		x := node.lambda
	END LikelihoodForm;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			lambda, logPrior, nu, x: REAL;
	BEGIN
		x := node.value;
		nu := node.nu.Value();
		lambda := node.lambda.Value();
		logPrior := (nu - 1.0) * Math.Ln(x) - lambda * Math.Power(x, nu);
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			nu, lambda, mean: REAL;
	BEGIN
		nu := node.nu.Value();
		lambda := node.lambda.Value();
		mean := Math.Power(lambda, - 1 / nu) * Math.Exp(MathFunc.LogGammaFunc(1 + 1 / nu));
		RETURN mean
	END Location;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		HALT(0)
	END PriorForm;

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
		IF node.nu.Value() < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg1}
		END;
		IF node.lambda.Value() < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg2}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.nu, wr);
		GraphNodes.Externalize(node.lambda, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.nu := GraphNodes.Internalize(rd);
		node.lambda := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.leftNatural});
		node.lambda := NIL;
		node.nu := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.nu.AddParent(list);
		node.lambda.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.nu := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			node.lambda := args.scalars[1]
		END
	END SetUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			lambda, nu, x, lower, upper: REAL;
			bounds: SET;
	BEGIN
		nu := node.nu.Value();
		lambda := node.lambda.Value();
		bounds := node.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		IF bounds = {} THEN
			x := MathRandnum.Weibull(nu, lambda)
		ELSE
			node.Bounds(lower, upper);
			IF bounds = {GraphStochastic.leftImposed} THEN
				x := MathRandnum.WeibullLB(nu, lambda, lower)
			ELSIF bounds = {GraphStochastic.rightImposed} THEN
				x := MathRandnum.WeibullRB(nu, lambda, upper)
			ELSE
				x := MathRandnum.WeibullIB(nu, lambda, lower, upper)
			END
		END;
		node.SetValue(x);
		res := {}
	END Sample;

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
		maintainer := "A.Thomas"
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
END GraphWeibull.

