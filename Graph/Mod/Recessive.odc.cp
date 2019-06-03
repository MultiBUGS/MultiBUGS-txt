(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphRecessive;


	

	IMPORT
		Stores := Stores64,
		GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathFunc;

	TYPE
		Node = POINTER TO RECORD(GraphUnivariate.Node)
			genotype: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD(GraphUnivariate.Factory) END;

	CONST
		eps = 1.0E-6;

	VAR
		fact-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) BoundsUnivariate (OUT left, right: REAL);
	BEGIN
		left := 1;
		right := 2
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
		VAR
			r: INTEGER;
	BEGIN
		r := SHORT(ENTIER(node.value + eps));
		IF ABS(r - node.value) > eps THEN
			RETURN {GraphNodes.integer, GraphNodes.lhs}
		END;
		IF ~(r IN {1, 2}) THEN
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
			genotype, r: INTEGER;
			logDensity: REAL;
	BEGIN
		genotype := SHORT(ENTIER(node.genotype.Value() + eps));
		r := SHORT(ENTIER(node.value + eps));
		IF genotype < 3 THEN
			IF r = 1 THEN
				logDensity := 0
			ELSE
				logDensity := MathFunc.logOfZero
			END
		ELSE
			IF r = 1 THEN
				logDensity := MathFunc.logOfZero
			ELSE
				logDensity := 0
			END
		END;
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
		GraphNodes.Externalize(node.genotype, wr);
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + 
		{GraphStochastic.integer, GraphStochastic.leftNatural,
		GraphStochastic.rightNatural, GraphStochastic.noMean});
		node.genotype := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.genotype := GraphNodes.Internalize(rd);
	END InternalizeUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphRecessive.Install"
	END Install;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			genotype, r: INTEGER;
			logLikelihood: REAL;
	BEGIN
		genotype := SHORT(ENTIER(node.genotype.Value() + eps));
		r := SHORT(ENTIER(node.value + eps));
		IF genotype < 3 THEN
			IF r = 1 THEN
				logLikelihood := 0
			ELSE
				logLikelihood := MathFunc.logOfZero
			END
		ELSE
			IF r = 1 THEN
				logLikelihood := MathFunc.logOfZero
			ELSE
				logLikelihood := 0
			END
		END;
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			genotype, r: INTEGER;
			logPrior: REAL;
	BEGIN
		genotype := SHORT(ENTIER(node.genotype.Value() + eps));
		r := SHORT(ENTIER(node.value + eps));
		IF genotype < 3 THEN
			IF r = 1 THEN
				logPrior := 0
			ELSE
				logPrior := MathFunc.logOfZero
			END
		ELSE
			IF r = 1 THEN
				logPrior := MathFunc.logOfZero
			ELSE
				logPrior := 0
			END
		END;
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
		node.genotype.AddParent(list);
		GraphNodes.ClearList(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		CONST
			eps = 1.0E-20;
		VAR
			genotype, r: INTEGER;
	BEGIN
		genotype := SHORT(ENTIER(node.genotype.Value() + eps));
		IF genotype < 3 THEN
			r := 1
		ELSE
			r := 2
		END;
		node.SetValue(r);
		res := {}
	END Sample;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.genotype := args.scalars[0]
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
		signature := "s"
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.thomas"
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
END GraphRecessive.

