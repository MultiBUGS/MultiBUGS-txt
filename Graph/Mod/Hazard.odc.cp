(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	  *)

MODULE GraphHazard;

	IMPORT
		Math, Stores,
		GraphNodes, GraphRules, GraphStochastic;

	TYPE
		Node = POINTER TO RECORD(GraphStochastic.Node) 
			function: GraphNodes.Node;
			t: GraphStochastic.Node;
			event: BOOLEAN
		END;

		Factory = POINTER TO RECORD(GraphStochastic.Factory) END;

	VAR
		fact-: GraphStochastic.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) Bounds (OUT left, upper: REAL);
	BEGIN
		left := 0;
		upper := INF
	END Bounds;

	PROCEDURE (node: Node) CanSample (multiVar: BOOLEAN): BOOLEAN;
	BEGIN
		RETURN FALSE
	END CanSample;

	PROCEDURE (node: Node) Check (): SET;
		VAR
			res: SET;
	BEGIN
		res := {};
		IF ~(GraphStochastic.data IN node.props) THEN res := {GraphNodes.notData} END;
		RETURN res
	END Check;

	PROCEDURE (node: Node) ClassifyLikelihood (parent: GraphStochastic.Node): INTEGER;
	BEGIN
		RETURN GraphRules.general
	END ClassifyLikelihood;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN -1
	END ClassifyPrior;

	PROCEDURE (node: Node) Deviance (): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END Deviance;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
	BEGIN
		RETURN 0.0
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
	BEGIN
		RETURN 0.0
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeStochastic (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.function, wr);
		GraphNodes.Externalize(node.t, wr);
		wr.WriteBool(node.event)
	END ExternalizeStochastic;

	PROCEDURE (node: Node) InternalizeStochastic (VAR rd: Stores.Reader);
		VAR
			p: GraphNodes.Node;
	BEGIN
		node.function := GraphNodes.Internalize(rd);
		p := GraphNodes.Internalize(rd);
		node.t := p(GraphStochastic.Node);
		rd.ReadBool(node.event)	
	END InternalizeStochastic;

	PROCEDURE (node: Node) InitStochastic;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.noMean, GraphStochastic.initialized});
		node.function := NIL;
		node.t := NIL;
		node.event := FALSE
	END InitStochastic;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphHazard.Install"
	END Install;

	PROCEDURE (node: Node) InvMap (y: REAL);
	BEGIN
		node.SetValue(y)
	END InvMap;

	PROCEDURE (node: Node) IsLikelihoodTerm (): BOOLEAN;
	BEGIN
		RETURN TRUE
	END IsLikelihoodTerm;

	PROCEDURE (node: Node) LogDetJacobian (): REAL;
	BEGIN
		RETURN 0.0
	END LogDetJacobian;

	PROCEDURE (node: Node) LogLikelihood (): REAL;
		VAR
			lambda, t1, t2, t3, t4, t5, log: REAL;
			t: GraphStochastic.Node;
	BEGIN
		(*	five point Lobatto quadrature, exact for seventh order polynomials	*)
		t1 := 0;
		t5 := node.value;
		t3 := 0.5 * t5;
		t2 := t3 * (1.0 - Math.Sqrt(3.0 / 7.0));
		t4 := t3 * (1.0 + Math.Sqrt(3.0 / 7.0));
		t.SetValue(t1);
		lambda := node.function.Value();
		log := 9 * lambda;
		t.SetValue(t2);
		lambda := node.function.Value();
		log := log + 49 * lambda;
		t.SetValue(t3);
		lambda := node.function.Value();
		log := log + 64 * lambda;
		t.SetValue(t4);
		lambda := node.function.Value();
		log := log + 49 * lambda;
		t.SetValue(t5);
		lambda := node.function.Value();
		log := log + 9 * lambda;
		log := -t3 * log / 90.0;
		IF node.event THEN log := Math.Ln(lambda) + log END;
		RETURN log
	END LogLikelihood;

	PROCEDURE (node: Node) LogPrior (): REAL;
	BEGIN
		RETURN 0.0
	END LogPrior;

	PROCEDURE (node: Node) Location (): REAL;
	BEGIN
		RETURN 0.0
	END Location;

	PROCEDURE (node: Node) Map (): REAL;
	BEGIN
		RETURN node.value
	END Map;

	PROCEDURE (node: Node) Parents (all: BOOLEAN): GraphNodes.List;
		VAR
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		p := node.function;
		p.AddParent(list);
		p := node.t;
		p.AddParent(list);
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Representative (): GraphStochastic.Node;
	BEGIN
		RETURN node
	END Representative;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.function := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			node.t := args.scalars[1](GraphStochastic.Node);
			ASSERT(args.scalars[2] # NIL, 21);
			node.event := args.scalars[2].Value() > 0.5
		END		
	END Set;

	PROCEDURE (node: Node) Size (): INTEGER;
	BEGIN
		RETURN 1
	END Size;

	PROCEDURE (node: Node) Sample (OUT res: SET);
	BEGIN
		HALT(0)
	END Sample;

	PROCEDURE (f: Factory) New (): GraphStochastic.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "Fs"
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
	END Init;

BEGIN
	Init
END GraphHazard.

