(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)

MODULE GraphPareto;


	

	IMPORT
		Math, Stores,
		GraphConjugateUV, GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathFunc, MathRandnum;

	TYPE
		Node = POINTER TO RECORD(GraphConjugateUV.Node)
			alpha, c: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD(GraphUnivariate.Factory) END;

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
		f0 := GraphStochastic.ClassFunction(node.c, parent);
		f1 := GraphStochastic.ClassFunction(node.alpha, parent);
		density0 := GraphRules.ClassifyShape(f0);
		density1 := GraphRules.ClassifyShape(f1);
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

	PROCEDURE (prior: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.pareto
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (x: REAL): REAL;
		VAR
			cumulative, alpha, c: REAL;
	BEGIN
		alpha := node.alpha.Value();
		c := node.c.Value();
		cumulative := 1.0 - Math.Power(c / x, alpha);
		RETURN cumulative
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			logC, logDensity, logAlpha, alpha, x, c: REAL;
	BEGIN
		x := node.value;
		c := node.c.Value();
		logC := MathFunc.Ln(c);
		alpha := node.alpha.Value();
		logAlpha := MathFunc.Ln(alpha);
		logDensity := logAlpha + alpha * logC - (alpha + 1.0) * Math.Ln(x);
		RETURN - 2.0 * logDensity
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			logX0, differential, alpha, val, c, diffC, diffAlpha: REAL;
	BEGIN
		val := node.value;
		IF (GraphStochastic.hint2 IN x.props) OR (GraphNodes.data IN node.alpha.props) THEN
			node.c.ValDiff(x, c, diffC);
			alpha := node.alpha.Value();
			differential := diffC * alpha / c;
		ELSIF (GraphStochastic.hint1 IN x.props) OR (GraphNodes.data IN node.c.props) THEN
			node.alpha.ValDiff(x, alpha, diffAlpha);
			c := node.c.Value();
			differential := diffAlpha * (1 / alpha + Math.Ln(c) - Math.Ln(val))
		ELSE
			node.c.ValDiff(x, c, diffC);
			node.alpha.ValDiff(x, alpha, diffAlpha);
			differential := diffAlpha * (1 / alpha + Math.Ln(c) - Math.Ln(val)) + diffC * alpha / c
		END;
		RETURN differential
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			differential, alpha, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		differential := - (alpha + 1.0) / x;
		RETURN differential
	END DiffLogPrior;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphPareto.Install"
	END Install;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			x, c, logX0, alpha, logTheta, logLikelihood: REAL;
	BEGIN
		x := node.value;
		c := node.c.Value();
		logX0 := MathFunc.Ln(c);
		alpha := node.alpha.Value();
		logTheta := MathFunc.Ln(alpha);
		logLikelihood := logTheta + alpha * logX0 - (alpha + 1.0) * Math.Ln(x);
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (offspring: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		HALT(126)
	END LikelihoodForm;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			alpha, logPrior, x: REAL;
	BEGIN
		x := node.value;
		alpha := node.alpha.Value();
		logPrior := - (alpha + 1.0) * Math.Ln(x);
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			alpha, c: REAL;
	BEGIN
		alpha := node.alpha.Value();
		c := node.c.Value();
		RETURN c * alpha / (alpha - 1)
	END Location;

	PROCEDURE (prior: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		ASSERT(as IN {GraphRules.gamma, GraphRules.gamma1, GraphRules.pareto}, 21);
		IF as = GraphRules.pareto THEN
			p0 := prior.alpha.Value();
			p1 := prior.c.Value()
		ELSE
			p0 := - prior.alpha.Value();
			p1 := 0.0
		END
	END PriorForm;

	PROCEDURE (node: Node) BoundsUnivariate (OUT left, right: REAL);
	BEGIN
		left := node.c.Value();
		right := INF
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
		VAR
			alpha, c: REAL;
	BEGIN
		c := node.c.Value();
		alpha := node.alpha.Value();
		IF c < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg2}
		END;
		IF node.value < c - eps THEN
			RETURN {GraphNodes.invalidPosative, GraphNodes.lhs}
		END;
		IF alpha < eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg1}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.alpha, wr);
		GraphNodes.Externalize(node.c, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.alpha := GraphNodes.Internalize(rd);
		node.c := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.leftNatural});
		node.c := NIL;
		node.alpha := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.c.AddParent(list);
		node.alpha.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 20);
			node.alpha := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 20);
			node.c := args.scalars[1]
		END
	END SetUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			alpha, c, x, lower, upper: REAL;
			bounds: SET;
	BEGIN
		c := node.c.Value();
		alpha := node.alpha.Value();
		bounds := node.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		IF bounds = {} THEN
			x := MathRandnum.Pareto(alpha, c)
		ELSE
			node.Bounds(lower, upper);
			IF bounds = {GraphStochastic.leftImposed} THEN
				x := MathRandnum.ParetoLB(alpha, c, lower)
			ELSIF bounds = {GraphStochastic.rightImposed} THEN
				x := MathRandnum.ParetoRB(alpha, c, upper)
			ELSE
				x := MathRandnum.ParetoIB(alpha, c, lower, upper);
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
END GraphPareto.

