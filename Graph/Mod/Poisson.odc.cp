(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"






*)

MODULE GraphPoisson;


	

	IMPORT
		Stores := Stores64,
		GraphConjugateUV, GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE
		Node = POINTER TO RECORD(GraphConjugateUV.Node)
			lambda: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD(GraphUnivariate.Factory) END;

	CONST
		maxIts = 100000;
		eps = 1.0E-5;

	VAR
		fact-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) BoundsUnivariate (OUT left, right: REAL);
	BEGIN
		left := 0;
		right := MAX(INTEGER)
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
		VAR
			r: INTEGER;
			lambda: REAL;
	BEGIN
		r := SHORT(ENTIER(node.value + eps));
		lambda := node.lambda.value;
		IF (GraphStochastic.update IN node.props) & (ABS(r - node.value) > eps) THEN
			RETURN {GraphNodes.integer, GraphNodes.lhs}
		END;
		IF r < 0 THEN
			RETURN {GraphNodes.invalidInteger, GraphNodes.lhs}
		END;
		IF lambda < - eps
			THEN
			RETURN {GraphNodes.posative, GraphNodes.arg1}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
		VAR
			class, f0: INTEGER;
	BEGIN
		f0 := GraphStochastic.ClassFunction(node.lambda, parent);
		class := GraphRules.ClassifyPrecision(f0);
		RETURN class
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		IF GraphStochastic.rightImposed IN node.props THEN
			RETURN GraphRules.catagorical
		ELSE
			RETURN GraphRules.descrete
		END
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (r: REAL): REAL;
		VAR
			cumulative, lambda: REAL;
	BEGIN
		lambda := node.lambda.value;
		cumulative := MathCumulative.Poisson(lambda, r);
		RETURN cumulative
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			lambda, logDensity, logLambda, r: REAL;
	BEGIN
		r := SHORT(ENTIER(node.value + eps));
		IF r > 0 THEN
			lambda := node.lambda.value;
			logLambda := MathFunc.Ln(lambda);
			logDensity := r * logLambda - lambda - MathFunc.LogGammaFunc(r + 1)
		ELSE
			lambda := node.lambda .value;
			logDensity := - lambda - MathFunc.LogGammaFunc(r + 1)
		END;
		RETURN - 2.0 * logDensity
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			diff, lambda, r: REAL;
	BEGIN
		r := SHORT(ENTIER(node.value + eps));
		lambda := node.lambda.value;
		diff := node.lambda.Diff(x);
		IF r > 0 THEN
			RETURN diff * (r / lambda - 1)
		ELSE
			RETURN - diff
		END
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.lambda, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.props := node.props + {GraphStochastic.integer, GraphStochastic.leftNatural};
		node.lambda := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.lambda := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphPoisson.Install"
	END Install;

	PROCEDURE (likelihood: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		ASSERT(as = GraphRules.gamma, 21);
		p0 := likelihood.value;
		p1 := 1.0;
		x := likelihood.lambda
	END LikelihoodForm;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			r: INTEGER;
			lambda, logLambda, logLikelihood: REAL;
	BEGIN
		r := SHORT(ENTIER(node.value + eps));
		IF r > 0 THEN
			lambda := node.lambda.value;
			logLambda := MathFunc.Ln(lambda);
			logLikelihood := r * logLambda - lambda
		ELSE
			lambda := node.lambda.value;
			logLikelihood := - lambda
		END;
		IF ~(GraphNodes.data IN node.props) THEN
			logLikelihood := logLikelihood - MathFunc.LogGammaFunc(r + 1)
		END;
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			r: INTEGER;
			lambda, logLambda, logPrior: REAL;
	BEGIN
		r := SHORT(ENTIER(node.value + eps));
		logPrior := - MathFunc.LogGammaFunc(r + 1);
		IF r > 0 THEN
			lambda := node.lambda.value;
			logLambda := MathFunc.Ln(lambda);
			logPrior := logPrior + r * logLambda
		END;
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			lambda: REAL;
	BEGIN
		lambda := node.lambda.value;
		RETURN lambda
	END Location;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.lambda.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		ASSERT(as = GraphRules.poisson, 21);
		p0 := node.lambda.value
	END PriorForm;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			lambda, lower, upper: REAL;
			iter, l, u, r: INTEGER;
	BEGIN
		res := {};
		iter := maxIts;
		node.Bounds(lower, upper);
		l := SHORT(ENTIER(lower + eps));
		u := SHORT(ENTIER(upper + eps));
		lambda := node.lambda.value;
		REPEAT
			r := MathRandnum.Poisson(lambda);
			DEC(iter)
		UNTIL ((r >= l) & (r <= u)) OR (iter = 0);
		IF iter # 0 THEN
			node.value := r
		ELSE
			res := {GraphNodes.lhs}
		END
	END Sample;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.lambda := args.scalars[0]
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
		signature := "sCT"
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
END GraphPoisson.

