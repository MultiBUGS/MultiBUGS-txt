(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"






*)

MODULE GraphNegbin;


	

	IMPORT
		Stores,
		GraphConjugateUV, GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE

		Node = POINTER TO RECORD(GraphConjugateUV.Node)
			p, n: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD(GraphUnivariate.Factory) END;

	CONST
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
			r, n: INTEGER;
			p: REAL;
	BEGIN
		r := SHORT(ENTIER(node.value + eps));
		IF ABS(r - node.value) > eps THEN
			RETURN {GraphNodes.integer, GraphNodes.lhs}
		END;
		IF r < 0 THEN
			RETURN {GraphNodes.invalidInteger, GraphNodes.lhs}
		END;
		p := node.p.Value();
		IF (p < - eps) OR (p > 1.0 + eps) THEN
			RETURN {GraphNodes.proportion, GraphNodes.arg1}
		END;
		n := SHORT(ENTIER(node.n.Value() + eps));
		IF ABS(n - node.n.Value()) > eps THEN
			RETURN {GraphNodes.integer, GraphNodes.arg2}
		END;
		IF n < 1 THEN
			RETURN {GraphNodes.invalidInteger, GraphNodes.arg2}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
		VAR
			density, f0, f1: INTEGER;
	BEGIN
		f0 := GraphStochastic.ClassFunction(node.n, parent);
		IF f0 = GraphRules.const THEN
			f1 := GraphStochastic.ClassFunction(node.p, parent);
			density := GraphRules.ClassifyProportion(f1)
		ELSE
			density := GraphRules.general
		END;
		RETURN density
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.descrete
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (x: REAL): REAL;
		VAR
			p, n: REAL;
	BEGIN
		p := node.p.Value();
		n := node.n.Value();
		RETURN MathCumulative.Negbin(p, n, x)
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			logDensity, logP, logQ, n, p, r: REAL;
	BEGIN
		r := node.value;
		p := node.p.Value();
		logP := MathFunc.Ln(p);
		logQ := MathFunc.Ln(1 - p);
		n := node.n.Value();
		logDensity := n * logP + r * logQ + MathFunc.LogGammaFunc(r + n)
		 - MathFunc.LogGammaFunc(n) - MathFunc.LogGammaFunc(r + 1);
		RETURN - 2.0 * logDensity
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			diff, n, p, r: REAL;
	BEGIN
		r := node.value;
		node.p.ValDiff(x, p, diff);
		n := node.n.Value();
		RETURN diff * (n / p - r / (1 - p))
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.p, wr);
		GraphNodes.Externalize(node.n, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.integer, GraphStochastic.leftNatural});
		node.p := NIL;
		node.n := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphNegbin.Install"
	END Install;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.p := GraphNodes.Internalize(rd);
		node.n := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (offspring: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		ASSERT(as = GraphRules.beta, 20);
		x := offspring.p;
		p0 := offspring.n.Value();
		p1 := offspring.value
	END LikelihoodForm;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			logLikelihood, logP, logQ, n, p, r: REAL;
	BEGIN
		r := node.value;
		p := node.p.Value();
		logP := MathFunc.Ln(p);
		logQ := MathFunc.Ln(1 - p);
		n := node.n.Value();
		logLikelihood := n * logP + r * logQ;
		IF ~(GraphNodes.data IN node.n.props) THEN
			logLikelihood := logLikelihood + MathFunc.LogGammaFunc(r + n)
			 - MathFunc.LogGammaFunc(n) - MathFunc.LogGammaFunc(r + 1)
		END;
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			logP, logQ, logPrior, n, p, r: REAL;
	BEGIN
		r := node.value;
		p := node.p.Value();
		logP := MathFunc.Ln(p);
		logQ := MathFunc.Ln(1 - p);
		n := node.n.Value();
		logPrior := n * logP + r * logQ;
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			n, p: REAL;
	BEGIN
		n := node.n.Value();
		p := node.p.Value();
		RETURN (1 - p) * n / p
	END Location;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.p.AddParent(list);
		node.n.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		HALT(0)
	END PriorForm;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.p := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			node.n := args.scalars[1]
		END
	END SetUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			p, lower, upper: REAL;
			iter, n, l, u, r: INTEGER;
	BEGIN
		res := {};
		node.Bounds(lower, upper);
		l := SHORT(ENTIER(lower + eps));
		u := SHORT(ENTIER(upper + eps));
		p := node.p.Value();
		n := SHORT(ENTIER(node.n.Value() + eps));
		r := MathRandnum.NegativeBinomialIB(p, n, l, u);
		node.SetValue(r)
	END Sample;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "ssCT"
	END Signature;

	PROCEDURE (f: Factory) New (): GraphUnivariate.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

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
END GraphNegbin.

