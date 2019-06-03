(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

Can only be used as a prior
P(x) =	Sid(x - xi)

*)

MODULE GraphPriorNP;

	

	IMPORT
		Stores := Stores64,
		GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathRandnum;

	TYPE
		Node = POINTER TO RECORD(GraphUnivariate.Node)
			sample: GraphNodes.Vector;
			size, start, step: INTEGER
		END;

		Factory = POINTER TO RECORD(GraphUnivariate.Factory) END;

	VAR
		fact-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) BoundsUnivariate (OUT left, right: REAL);
	BEGIN
		left := - INF;
		right := INF
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
	BEGIN
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
	BEGIN
		HALT(0);
		RETURN 0
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.general
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (x: REAL): REAL;
	BEGIN
		HALT(0);
		RETURN 0
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
	BEGIN
		HALT(0);
		RETURN 0
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
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		v.Init;
		v.components := node.sample;
		v.start := node.start; v.nElem := node.size; v.step := node.step;
		GraphNodes.ExternalizeSubvector(v, wr);
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		GraphNodes.InternalizeSubvector(v, rd);
		node.sample := v.components;
		node.size := v.nElem;
		node.start := v.start;
		node.step := v.step;
	END InternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.sample := NIL;
		node.size := - 1;
		node.start := - 1;
		node.step := - 1;
		node.SetProps(node.props + {GraphStochastic.noMean})
	END InitUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphPriorNP.Install"
	END Install;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
	BEGIN
		RETURN 0
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
	BEGIN
		RETURN NIL
	END ParentsUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
	BEGIN
		RETURN 0
	END LogPrior;

	PROCEDURE (node: Node) Location (): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END Location;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			i: INTEGER;
			value: REAL;
	BEGIN
		i := MathRandnum.DiscreteUniform(1, node.size) - 1;
		value := node.sample[i].Value();
		node.SetValue(value);
		res := {}
	END Sample;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			i, size, start, step: INTEGER;
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.vectors[0].components # NIL, 21);
			node.sample := args.vectors[0].components;
			ASSERT(args.vectors[0].start >= 0, 21);
			size := args.vectors[0].nElem;
			node.size := size;
			start := args.vectors[0].start;
			node.start := start;
			step := args.vectors[0].step;
			node.step := step;
		END;
		i := 0;
		WHILE i < size DO
			IF node.sample[start + i * step] = NIL THEN
				res := {GraphNodes.nil, GraphNodes.arg1};
				RETURN
			END;
			IF ~(GraphNodes.data IN node.sample[start + i * step].props) THEN
				res := {GraphNodes.data, GraphNodes.arg1};
				RETURN
			END;
			INC(i)
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
		signature := "v"
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
END GraphPriorNP.

