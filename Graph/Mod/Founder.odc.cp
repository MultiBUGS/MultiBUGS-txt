(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphFounder;


	

	IMPORT
		Math, Stores := Stores64,
		GraphConjugateUV, GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathFunc, MathRandnum;

	TYPE
		Node = POINTER TO RECORD(GraphConjugateUV.Node)
			p: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD(GraphUnivariate.Factory) END;

	VAR
		fact-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) BoundsUnivariate (OUT left, right: REAL);
	BEGIN
		left := 1;
		right := 3
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
		CONST
			eps = 1.0E-6;
		VAR
			r: INTEGER;
			p: REAL;
	BEGIN
		r := SHORT(ENTIER(node.value + eps));
		IF ABS(r - node.value) > eps THEN
			RETURN {GraphNodes.integer, GraphNodes.lhs}
		END;
		IF ~(r IN {1, 2, 3}) THEN
			RETURN {GraphNodes.invalidInteger, GraphNodes.lhs}
		END;
		p := node.p.value;
		IF (p <= - eps) OR (p >= 1 + eps) THEN
			RETURN {GraphNodes.proportion, GraphNodes.arg1}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
		VAR
			f0: INTEGER;
	BEGIN
		f0 := GraphStochastic.ClassFunction(node.p, parent);
		RETURN GraphRules.ClassifyProportion(f0)
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.catagorical
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (x: REAL): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			logDensity, logP, logQ, p, r: REAL;
	BEGIN
		r := node.value;
		p := node.p.value;
		logP := MathFunc.Ln(p);
		logQ := MathFunc.Ln(1 - p);
		logDensity := (r - 1) * logP + (3 - r) * logQ;
		IF (r > 0.5) & (r < 1.5) THEN
			logDensity := logDensity + Math.Ln(2)
		END;
		RETURN - 2.0 * logDensity
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			diff, p, r: REAL;
	BEGIN
		r := node.value;
		p := node.p.value;
		diff := node.p.Diff(x);
		RETURN diff * ((r - 1) / p - (3 - r) / (1 - p));
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.p, wr);
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.p := GraphNodes.Internalize(rd);
	END InternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.props := node.props + 
		{GraphStochastic.integer, GraphStochastic.leftNatural,
		GraphStochastic.rightNatural, GraphStochastic.noMean};
		node.p := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphFounder.Install"
	END Install;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
		CONST
			eps = 1.0E-20;
		VAR
			r: INTEGER;
	BEGIN
		ASSERT(as = GraphRules.beta, 21);
		r := SHORT(ENTIER(node.value + eps));
		IF r = 1 THEN
			p0 := 2;
			p1 := 0
		ELSIF r = 2 THEN
			p0 := 1;
			p1 := 1
		ELSE
			p0 := 0;
			p1 := 2
		END;
		x := node.p
	END LikelihoodForm;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			logLikelihood, logP, logQ, p, r: REAL;
	BEGIN
		r := node.value;
		p := node.p.value;
		logP := MathFunc.Ln(p);
		logQ := MathFunc.Ln(1 - p);
		logLikelihood := (r - 1) * logP + (3 - r) * logQ;
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			logP, logPrior, logQ, p, r: REAL;
	BEGIN
		r := node.value;
		p := node.p.value;
		logP := MathFunc.Ln(p);
		logQ := MathFunc.Ln(1 - p);
		logPrior := (r - 1) * logP + (3 - r) * logQ;
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) Location (): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END Location;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.p.AddParent(list);
		GraphNodes.ClearList(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		HALT(0)
	END PriorForm;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			p, r: REAL;
	BEGIN
		p := node.p.value;
		r := MathRandnum.Binomial(p, 2) + 1;
		node.value := r;
		res := {}
	END Sample;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.p := args.scalars[0]
		END
	END SetUnivariate;

	PROCEDURE (ft: Factory) New (): GraphUnivariate.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "s"
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
END GraphFounder.
