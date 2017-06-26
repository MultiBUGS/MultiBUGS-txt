(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphMendelian;


	

	IMPORT
		Stores,
		GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathFunc, MathRandnum;

	TYPE
		Node = POINTER TO RECORD(GraphUnivariate.Node)
			mother, farther: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD(GraphUnivariate.Factory) END;

	CONST
		eps = 1.0E-5;

	VAR
		fact-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		prob: ARRAY 3, 3, 3 OF REAL;

	PROCEDURE (node: Node) BoundsUnivariate (OUT left, right: REAL);
	BEGIN
		left := 1;
		right := 3
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
		VAR
			r: INTEGER;
	BEGIN
		r := SHORT(ENTIER(node.value + eps));
		IF ABS(r - node.value) > eps THEN
			RETURN {GraphNodes.integer, GraphNodes.lhs}
		END;
		IF ~(r IN {1, 2, 3}) THEN
			RETURN {GraphNodes.invalidInteger, GraphNodes.lhs}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
	BEGIN
		RETURN GraphRules.general
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
			farther, mother, r: INTEGER;
			density, logDensity: REAL;
	BEGIN
		mother := SHORT(ENTIER(node.mother.Value() + eps));
		farther := SHORT(ENTIER(node.farther.Value() + eps));
		r := SHORT(ENTIER(node.value + eps));
		density := prob[mother - 1, farther - 1, r - 1];
		logDensity := MathFunc.Ln(density);
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
		GraphNodes.Externalize(node.mother, wr);
		GraphNodes.Externalize(node.farther, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + 
		{GraphStochastic.integer, GraphStochastic.leftNatural,
		GraphStochastic.rightNatural, GraphStochastic.noMean});
		node.mother := NIL;
		node.farther := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.mother := GraphNodes.Internalize(rd);
		node.farther := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphMendelian.Install"
	END Install;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			farther, mother, r: INTEGER;
			density: REAL;
	BEGIN
		mother := SHORT(ENTIER(node.mother.Value() + eps));
		farther := SHORT(ENTIER(node.farther.Value() + eps));
		r := SHORT(ENTIER(node.value + eps));
		density := prob[mother - 1, farther - 1, r - 1];
		RETURN MathFunc.Ln(density)
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			farther, mother, r: INTEGER;
			density: REAL;
	BEGIN
		mother := SHORT(ENTIER(node.mother.Value() + eps));
		farther := SHORT(ENTIER(node.farther.Value() + eps));
		r := SHORT(ENTIER(node.value + eps));
		density := prob[mother - 1, farther - 1, r - 1];
		RETURN MathFunc.Ln(density)
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
		node.mother.AddParent(list);
		node.farther.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			mother, farther: INTEGER;
			rand: REAL;
			p: ARRAY 2 OF REAL;
	BEGIN
		mother := SHORT(ENTIER(node.mother.Value() + eps));
		farther := SHORT(ENTIER(node.farther.Value() + eps));
		p[0] := prob[mother - 1, farther - 1, 0];
		p[1] := p[0] + prob[mother - 1, farther - 1, 1];
		rand := MathRandnum.Rand();
		IF rand < p[0] THEN
			node.SetValue(1)
		ELSIF rand < p[1] THEN
			node.SetValue(2)
		ELSE
			node.SetValue(3)
		END;
		res := {}
	END Sample;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.mother := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			node.farther := args.scalars[1]
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
		signature := "ss"
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
		fact := f;
		prob[0, 0, 0] := 1.0; prob[0, 0, 1] := 0.0; prob[0, 0, 2] := 0.0;
		prob[0, 1, 0] := 0.5; prob[0, 1, 1] := 0.5; prob[0, 1, 2] := 0.0;
		prob[0, 2, 0] := 0.0; prob[0, 2, 1] := 1.0; prob[0, 2, 2] := 0.0;
		prob[1, 0, 0] := 0.5; prob[1, 0, 1] := 0.5; prob[1, 0, 2] := 0.0;
		prob[1, 1, 0] := 0.25; prob[1, 1, 1] := 0.5; prob[1, 1, 2] := 0.25;
		prob[1, 2, 0] := 0.0; prob[1, 2, 1] := 0.5; prob[1, 2, 2] := 0.5;
		prob[2, 0, 0] := 0.0; prob[2, 0, 1] := 1.0; prob[2, 0, 2] := 0.0;
		prob[2, 1, 0] := 0.0; prob[2, 1, 1] := 0.5; prob[2, 1, 2] := 0.5;
		prob[2, 2, 0] := 0.0; prob[2, 2, 1] := 0.0; prob[2, 2, 2] := 1.0
	END Init;

BEGIN
	Init
END GraphMendelian.

