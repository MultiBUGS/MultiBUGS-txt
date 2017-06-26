(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)

MODULE GraphExp;


	

	IMPORT
		Math, Stores,
		GraphConjugateUV, GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE
		Node = POINTER TO RECORD(GraphConjugateUV.Node)
			lambda: GraphNodes.Node
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
			f0: INTEGER;
	BEGIN
		f0 := GraphStochastic.ClassFunction(node.lambda, parent);
		RETURN GraphRules.ClassifyPrecision(f0)
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
		VAR
			class: INTEGER;
	BEGIN
		class := GraphRules.gamma1;
		RETURN class
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (x: REAL): REAL;
		VAR
			cumulative, lambda: REAL;
	BEGIN
		lambda := node.lambda.Value();
		cumulative := MathCumulative.Exp(lambda, x);
		RETURN cumulative
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			lambda, logDensity, logLambda, x: REAL;
	BEGIN
		x := node.value;
		lambda := node.lambda.Value();
		logLambda := MathFunc.Ln(lambda);
		logDensity := logLambda - x * lambda;
		RETURN - 2.0 * logDensity
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			lambda, differential, lambdaDiff, val: REAL;
	BEGIN
		val := node.value;
		node.lambda.ValDiff(x, lambda, lambdaDiff);
		differential := lambdaDiff / lambda - val * lambdaDiff;
		RETURN differential
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			lambda: REAL;
	BEGIN
		lambda := node.lambda.Value();
		RETURN - lambda
	END DiffLogPrior;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphExp.Install"
	END Install;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			lambda, logLambda, logLikelihood, x: REAL;
	BEGIN
		x := node.value;
		lambda := node.lambda.Value();
		logLambda := MathFunc.Ln(lambda);
		logLikelihood := logLambda - x * lambda;
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (offspring: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		ASSERT(as = GraphRules.gamma, 21);
		p0 := 1.0;
		p1 := offspring.value;
		x := offspring.lambda
	END LikelihoodForm;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			lambda, logPrior, x: REAL;
	BEGIN
		x := node.value;
		lambda := node.lambda.Value();
		logPrior := - x * lambda;
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			lambda: REAL;
	BEGIN
		lambda := node.lambda.Value();
		RETURN 1 / lambda
	END Location;

	PROCEDURE (prior: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		ASSERT(as IN {GraphRules.gamma, GraphRules.gamma1}, 21);
		p0 := 1.0;
		p1 := prior.lambda.Value()
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
		IF node.lambda.Value() < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg1}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.lambda, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.lambda := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.leftNatural});
		node.lambda := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.lambda.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.lambda := args.scalars[0]
		END
	END SetUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			lambda, x, lower, upper: REAL;
			bounds: SET;
	BEGIN
		lambda := node.lambda.Value();
		bounds := node.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		IF bounds = {} THEN
			x := MathRandnum.Exponential(lambda)
		ELSE
			node.Bounds(lower, upper);
			IF bounds = {GraphStochastic.leftImposed} THEN
				x := MathRandnum.ExponentialLB(lambda, lower)
			ELSIF bounds = {GraphStochastic.rightImposed} THEN
				x := MathRandnum.ExponentialRB(lambda, upper)
			ELSE
				x := MathRandnum.ExponentialIB(lambda, lower, upper)
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
END GraphExp.

