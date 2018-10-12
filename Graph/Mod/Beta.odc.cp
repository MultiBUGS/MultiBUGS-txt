(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)

MODULE GraphBeta;


	

	IMPORT
		Math, Stores,
		GraphConjugateUV, GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE
		Node = POINTER TO RECORD(GraphConjugateUV.Node)
			a, b: GraphNodes.Node;
		END;

		Factory = POINTER TO RECORD(GraphUnivariate.Factory) END;

	VAR
		fact-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
		VAR
			density, density0, density1, f0, f1: INTEGER;
	BEGIN
		f0 := GraphStochastic.ClassFunction(node.a, parent);
		f1 := GraphStochastic.ClassFunction(node.b, parent);
		density0 := GraphRules.ClassifyShape(f0);
		density1 := GraphRules.ClassifyShape(f1);
		IF density0 = GraphRules.unif THEN
			density := density1
		ELSIF density1 = GraphRules.unif THEN
			density := density0
		ELSIF (density0 = GraphRules.logCon) & (density1 = GraphRules.logCon) THEN
			density := GraphRules.logCon
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
		IF (GraphNodes.data IN node.a.props) & (node.a.Value() > 1.0)
			 & (GraphNodes.data IN node.b.props) & (node.b.Value() > 1.0) THEN
			class := GraphRules.beta1
		ELSE
			class := GraphRules.beta
		END;
		RETURN class
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (p: REAL): REAL;
		VAR
			a, b, cumulative: REAL;
	BEGIN
		a := node.a.Value();
		b := node.b.Value();
		cumulative := MathCumulative.Beta(a, b, p);
		RETURN cumulative
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			a, b, logDensity, p: REAL;
	BEGIN
		p := node.value;
		a := node.a.Value();
		b := node.b.Value();
		logDensity := (a - 1) * Math.Ln(p) + (b - 1) * Math.Ln(1 - p) + MathFunc.LogGammaFunc(a + b)
		 - MathFunc.LogGammaFunc(a) - MathFunc.LogGammaFunc(b);
		RETURN - 2.0 * logDensity
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			a, b, diffA, diffB, differential, p: REAL;
	BEGIN
		p := node.value;
		node.a.ValDiff(x, a, diffA);
		node.b.ValDiff(x, b, diffB);
		differential := diffA * Math.Ln(p) + diffB * Math.Ln(1 - p)
		 + (diffA + diffB) * MathFunc.Digamma(a + b)
		 - diffA * MathFunc.Digamma(a) - diffB * MathFunc.Digamma(b);
		RETURN differential
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			a, b, p: REAL;
	BEGIN
		a := node.a.Value();
		b := node.b.Value();
		p := node.value;
		RETURN ((a - 1) / p) - ((b - 1) / (1 - p))
	END DiffLogPrior;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphBeta.Install"
	END Install;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		HALT(0)
	END LikelihoodForm;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			a, b, logLikelihood, p: REAL;
	BEGIN
		p := node.value;
		a := node.a.Value();
		b := node.b.Value();
		logLikelihood := (a - 1) * Math.Ln(p) + (b - 1) * Math.Ln(1 - p);
		IF ~(GraphNodes.data IN node.a.props) OR ~(GraphNodes.data IN node.b.props) THEN
			logLikelihood := logLikelihood + MathFunc.LogGammaFunc(a + b)
			 - MathFunc.LogGammaFunc(a) - MathFunc.LogGammaFunc(b)
		END;
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			a, b, logPrior, p: REAL;
	BEGIN
		p := node.value;
		a := node.a.Value();
		b := node.b.Value();
		logPrior := (a - 1) * Math.Ln(p) + (b - 1) * Math.Ln(1 - p);
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			a, b: REAL;
	BEGIN
		a := node.a.Value();
		b := node.b.Value();
		RETURN a / (a + b)
	END Location;

	PROCEDURE (prior: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		ASSERT(as = GraphRules.beta, 21);
		p0 := prior.a.Value();
		p1 := prior.b.Value()
	END PriorForm;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.a := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			node.b := args.scalars[1];
		END
	END SetUnivariate;

	PROCEDURE (node: Node) BoundsUnivariate (OUT left, right: REAL);
	BEGIN
		left := 0.0;
		right := 1.0
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
		VAR
			p: REAL;
		CONST
			eps = 1.0E-20;
	BEGIN
		p := node.value;
		IF (p < - eps) OR (p > 1.0 + eps) THEN
			RETURN {GraphNodes.proportion, GraphNodes.lhs}
		END;
		IF node.a.Value() < eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg1}
		END;
		IF node.b.Value() < eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg2}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.a, wr);
		GraphNodes.Externalize(node.b, wr);
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.a := GraphNodes.Internalize(rd);
		node.b := GraphNodes.Internalize(rd);
	END InternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.leftNatural, GraphStochastic.rightNatural});
		node.a := NIL;
		node.b := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.a.AddParent(list);
		node.b.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		CONST
			minParam = 0.005;
		VAR
			a, b, p, lower, upper: REAL;
			bounds: SET;
	BEGIN
		a := node.a.Value();
		b := node.b.Value();
		IF a < minParam THEN
			res := {GraphNodes.arg1, GraphNodes.invalidPosative};
			RETURN
		END;
		IF b < minParam THEN
			res := {GraphNodes.arg2, GraphNodes.invalidPosative};
			RETURN
		END;
		bounds := node.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		IF bounds = {} THEN
			p := MathRandnum.Beta(a, b)
		ELSE
			node.Bounds(lower, upper);
			IF bounds = {GraphStochastic.leftImposed} THEN
				p := MathRandnum.BetaLB(a, b, lower)
			ELSIF bounds = {GraphStochastic.rightImposed} THEN
				p := MathRandnum.BetaRB(a, b, upper)
			ELSE
				p := MathRandnum.BetaIB(a, b, lower, upper)
			END
		END;
		node.SetValue(p);
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
END GraphBeta.

