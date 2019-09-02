(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)

MODULE GraphChisqr;


	

	IMPORT
		Math, Stores := Stores64,
		GraphConjugateUV, GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE
		Node = POINTER TO RECORD(GraphConjugateUV.Node)
			k: GraphNodes.Node
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
			f: INTEGER;
	BEGIN
		f := GraphStochastic.ClassFunction(node.k, parent);
		RETURN GraphRules.ClassifyShape(f)
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
		VAR
			class: INTEGER;
	BEGIN
		IF (GraphNodes.data IN node.k.props) & (node.k.value >= 2.0) THEN
			class := GraphRules.gamma1
		ELSE
			class := GraphRules.gamma
		END;
		RETURN class
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (x: REAL): REAL;
		VAR
			cumulative, k: REAL;
	BEGIN
		k := 0.5 * node.k.value;
		cumulative := MathCumulative.Chisqr(k, x);
		RETURN cumulative
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			x, logDensity, k: REAL;
	BEGIN
		x := node.value;
		k := 0.5 * node.k.value;
		logDensity := - k * Math.Ln(2) + (k - 1.0) * Math.Ln(x) - 0.5 * x - MathFunc.LogGammaFunc(k);
		RETURN - 2.0 * logDensity
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			differential, k, diffK, val: REAL;
	BEGIN
		val := node.value;
		k := node.k.value;
		diffK := node.k.Diff(x);
		k := 0.5 * k;
		diffK := 0.5 * diffK;
		differential := - diffK * Math.Ln(2) + diffK * Math.Ln(val) - diffK * MathFunc.Digamma(k);
		RETURN differential
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			k, x: REAL;
	BEGIN
		k := node.k.value;
		x := node.value;
		RETURN (0.5 * k - 1) / x - 0.5
	END DiffLogPrior;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphChisqr.Install"
	END Install;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			logLikelihood, k, x: REAL;
	BEGIN
		x := node.value;
		k := 0.5 * node.k.value;
		logLikelihood := - k * Math.Ln(2) + (k - 1.0) * Math.Ln(x) - 0.5 * x - MathFunc.LogGammaFunc(k);
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node; OUT p0, p1: REAL);
	BEGIN
		HALT(0)
	END LikelihoodForm;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			logPrior, k, x: REAL;
	BEGIN
		x := node.value;
		k := 0.5 * node.k.value;
		logPrior := (k - 1.0) * Math.Ln(x) - 0.5 * x;
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			k: REAL;
	BEGIN
		k := node.k.value;
		RETURN k
	END Location;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		ASSERT(as = GraphRules.gamma, 21);
		p0 := 0.50 * node.k.value;
		p1 := 0.50
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
		IF node.k.value < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg1}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.k, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.k := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		INCL(node.props,GraphStochastic.leftNatural);
		node.k := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.k.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.k := args.scalars[0]
		END
	END SetUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			k, x, lower, upper: REAL;
			bounds: SET;
	BEGIN
		k := 0.50 * node.k.value;
		bounds := node.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		IF bounds = {} THEN
			x := MathRandnum.Gamma(k, 0.5)
		ELSE
			node.Bounds(lower, upper);
			IF bounds = {GraphStochastic.leftImposed} THEN
				x := MathRandnum.GammaLB(k, 0.5, lower)
			ELSIF bounds = {GraphStochastic.rightImposed} THEN
				x := MathRandnum.GammaRB(k, 0.5, lower)
			ELSE
				x := MathRandnum.GammaIB(k, 0.5, lower, upper)
			END
		END;
		node.value := x;
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
END GraphChisqr.

