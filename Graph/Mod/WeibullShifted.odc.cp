(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"




*)

MODULE GraphWeibullShifted;


	

	IMPORT
		Math, Stores,
		GraphConjugateUV, GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE

		Node = POINTER TO RECORD(GraphConjugateUV.Node)
			nu, lambda, x0: GraphNodes.Node
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
			density, density0, density1, density2, f0, f1, f2: INTEGER;
	BEGIN
		f0 := GraphStochastic.ClassFunction(node.nu, parent);
		density0 := GraphRules.ClassifyShape(f0);
		f1 := GraphStochastic.ClassFunction(node.lambda, parent);
		density1 := GraphRules.ClassifyPrecision(f1);
		f2 := GraphStochastic.ClassFunction(node.x0, parent);
		IF f2 = GraphRules.const THEN
			density2 := GraphRules.unif
		ELSIF f2 # GraphRules.other THEN
			density2 := GraphRules.genDiff
		ELSE
			density2 := GraphRules.general
		END;
		IF (density0 = GraphRules.unif) & (density1 = GraphRules.unif) THEN
			density := density2
		ELSIF (density1 = GraphRules.unif) & (density2 = GraphRules.unif) THEN
			density := density0
		ELSIF (density2 = GraphRules.unif) & (density0 = GraphRules.unif) THEN
			density := density1
		ELSIF (density0 # GraphRules.general) & 
			(density1 # GraphRules.general) & (density2 # GraphRules.general) THEN
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
			cumulative, lambda, nu, x0: REAL;
	BEGIN
		lambda := node.lambda.Value();
		nu := node.nu.Value();
		x0 := node.x0.Value();
		cumulative := MathCumulative.WeibullShifted(nu, lambda, x0, x);
		RETURN cumulative
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			logLambda, logLikelihood, logNu, lambda, nu, x0, x: REAL;
	BEGIN
		x := node.value;
		x0 := node.x0.Value();
		x := x - x0;
		nu := node.nu.Value();
		logNu := MathFunc.Ln(nu);
		lambda := node.lambda.Value();
		logLambda := MathFunc.Ln(lambda);
		logLikelihood := logLambda + logNu + (nu - 1.0) * Math.Ln(x) - lambda * Math.Power(x, nu);
		RETURN - 2.0 * logLikelihood
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			lambda, diffLambda, differential, diffNu, nu, val, x0, pow: REAL;
	BEGIN
		val := node.value;
		x0 := node.x0.Value();
		val := val - x0;
		node.nu.ValDiff(x, nu, diffNu);
		node.lambda.ValDiff(x, lambda, diffLambda);
		pow := Math.Power(val, nu);
		differential := diffNu * (1 / nu + Math.Ln(val) * (1 - lambda * pow)) + diffLambda * (1 / lambda - pow);
		RETURN differential
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			differential, lambda, nu, x0, x: REAL;
	BEGIN
		x := node.value;
		x0 := node.x0.Value();
		x := x - x0;
		nu := node.nu.Value();
		lambda := node.lambda.Value();
		differential := (nu - 1.0) / x - lambda * nu * Math.Power(x, nu - 1);
		RETURN differential
	END DiffLogPrior;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphWeibullShifted.Install"
	END Install;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
		VAR
			nu, x0: REAL;
	BEGIN
		ASSERT(as = GraphRules.gamma, 21);
		p0 := 1;
		nu := node.nu.Value();
		x0 := node.x0.Value();
		p1 := Math.Power(node.value - x0, nu);
		x := node.lambda
	END LikelihoodForm;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			nu, lambda, mean, x0: REAL;
	BEGIN
		nu := node.nu.Value();
		lambda := node.lambda.Value();
		x0 := node.x0.Value();
		mean := Math.Power(lambda, - 1 / nu) * Math.Exp(MathFunc.LogGammaFunc(1 + 1 / nu));
		RETURN mean + x0
	END Location;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		HALT(0)
	END PriorForm;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			lambda, logLambda, logLikelihood, logNu, nu, x0, x: REAL;
	BEGIN
		x := node.value;
		x0 := node.x0.Value();
		x := x - x0;
		IF x < 0 THEN
			logLikelihood := MathFunc.logOfZero
		ELSE
			nu := node.nu.Value();
			logNu := MathFunc.Ln(nu);
			lambda := node.lambda.Value();
			logLambda := MathFunc.Ln(lambda);
			logLikelihood := logLambda + logNu + (nu - 1.0) * Math.Ln(x) - lambda * Math.Power(x, nu)
		END;
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			lambda, logPrior, nu, x0, x: REAL;
	BEGIN
		x := node.value;
		x0 := node.x0.Value();
		x := x - x0;
		IF x < 0 THEN
			logPrior := MathFunc.logOfZero
		ELSE
			nu := node.nu.Value();
			lambda := node.lambda.Value();
			logPrior := (nu - 1.0) * Math.Ln(x) - lambda * Math.Power(x, nu)
		END;
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) BoundsUnivariate (OUT left, right: REAL);
	BEGIN
		left := node.x0.Value();
		right := INF
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
	BEGIN
		IF node.nu.Value() < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg1}
		END;
		IF node.lambda.Value() < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg2}
		END;
		IF node.x0.Value() < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg3}
		END;
		IF node.value < node.x0.Value() THEN
			RETURN {GraphNodes.invalidPosative, GraphNodes.lhs}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.nu, wr);
		GraphNodes.Externalize(node.lambda, wr);
		GraphNodes.Externalize(node.x0, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.nu := GraphNodes.Internalize(rd);
		node.lambda := GraphNodes.Internalize(rd);
		node.x0 := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.leftNatural});
		node.nu := NIL;
		node.lambda := NIL;
		node.x0 := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.nu.AddParent(list);
		node.lambda.AddParent(list);
		node.x0.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.nu := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			node.lambda := args.scalars[1];
			ASSERT(args.scalars[2] # NIL, 21);
			node.x0 := args.scalars[2]
		END
	END SetUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			lambda, nu, x, x0, lower, upper: REAL;
			bounds: SET;
	BEGIN
		nu := node.nu.Value();
		lambda := node.lambda.Value();
		x0 := node.x0.Value();
		bounds := node.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		IF bounds = {} THEN
			x := x0 + MathRandnum.Weibull(nu, lambda)
		ELSE
			node.Bounds(lower, upper);
			IF bounds = {GraphStochastic.leftImposed} THEN
				lower := MAX(0, lower - x0);
				x := x0 + MathRandnum.WeibullLB(nu, lambda, lower);
			ELSIF bounds = {GraphStochastic.rightImposed} THEN
				upper := MAX(0, upper - x0);
				x := x0 + MathRandnum.WeibullRB(nu, lambda, upper);
			ELSE
				lower := MAX(0, lower - x0);
				upper := MAX(0, upper - x0);
				x := x0 + MathRandnum.WeibullIB(nu, lambda, lower, upper);
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
		signature := "sssCT"
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
END GraphWeibullShifted.

