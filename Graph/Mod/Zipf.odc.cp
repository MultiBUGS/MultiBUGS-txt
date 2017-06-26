(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphZipf;


	

	IMPORT
		Math, Stores,
		GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathFunc, MathRandnum;

	TYPE
		Node = POINTER TO RECORD(GraphUnivariate.Node)
			alpha: GraphNodes.Node
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
		left := 1;
		right := MAX(INTEGER)
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
		VAR
			r: INTEGER;
			alpha: REAL;
	BEGIN
		r := SHORT(ENTIER(node.value + eps));
		alpha := node.alpha.Value();
		IF (GraphStochastic.update IN node.props) & (ABS(r - node.value) > eps) THEN
			RETURN {GraphNodes.integer, GraphNodes.lhs}
		END;
		IF r < 1 THEN
			RETURN {GraphNodes.invalidInteger, GraphNodes.lhs}
		END;
		IF alpha < 1.0 THEN
			RETURN {GraphNodes.posative, GraphNodes.arg1}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.poisson
	END ClassifyPrior;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
		VAR
			class, f: INTEGER;
	BEGIN
		f := GraphStochastic.ClassFunction(node.alpha, parent);
		IF f # GraphRules.const THEN class := GraphRules.general ELSE class := GraphRules.unif END;
		RETURN class
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (node: Node) Cumulative (r: REAL): REAL;
	BEGIN
		HALT(0);
		RETURN 0
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			alpha, logDensity, x: REAL;
	BEGIN
		alpha := node.alpha.Value();
		x := node.value;
		logDensity := - Math.Ln(MathFunc.Zeta(alpha)) - alpha * Math.Ln(x);
		RETURN - 2.0 * logDensity
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.alpha, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + 
		{GraphStochastic.integer, GraphStochastic.leftNatural, GraphStochastic.noCDF});
		node.alpha := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.alpha := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphZipf.Install"
	END Install;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			alpha, logDensity, x: REAL;
	BEGIN
		alpha := node.alpha.Value();
		x := node.value;
		; logDensity := - Math.Ln(MathFunc.Zeta(alpha)) - alpha * Math.Ln(x);
		RETURN logDensity
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			alpha, logPrior, x: REAL;
	BEGIN
		alpha := node.alpha.Value();
		x := node.value;
		logPrior := - alpha * Math.Ln(x);
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			alpha: REAL;
	BEGIN
		alpha := node.alpha.Value();
		RETURN MathFunc.Zeta(alpha - 1) / MathFunc.Zeta(alpha)
	END Location;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.alpha.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.alpha := args.scalars[0]
		END
	END SetUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			alpha, r, left, right: REAL;
			bounds: SET;
			lower, upper: INTEGER;
	BEGIN
		alpha := node.alpha.Value();
		bounds := node.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		IF bounds = {} THEN
			r := MathRandnum.Zipf(alpha)
		ELSE
			node.Bounds(left, right);
			IF bounds = {GraphStochastic.leftImposed} THEN
				lower := SHORT(ENTIER(left + eps));
				r := MathRandnum.ZipfLB(alpha, lower);
			ELSIF bounds = {GraphStochastic.rightImposed} THEN
				upper := SHORT(ENTIER(right + eps));
				r := MathRandnum.ZipfRB(alpha, upper);
			ELSE
				lower := SHORT(ENTIER(left + eps));
				upper := SHORT(ENTIER(right + eps));
				r := MathRandnum.ZipfIB(alpha, lower, upper);
			END
		END;
		node.SetValue(r);
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
		signature := "sC"
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
END GraphZipf.

