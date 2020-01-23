(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	  *)

MODULE GraphWeibullHazard;

	IMPORT
		Math, Stores := Stores64, 
		GraphJacobi, GraphLogical, GraphNodes, GraphRules, GraphStochastic;

	TYPE
		Node = POINTER TO RECORD(GraphStochastic.Node) 
			nu: GraphNodes.Node;
			rule: GraphJacobi.Node;
			function: GraphNodes.Node;
			t: GraphStochastic.Node;
			event: BOOLEAN
		END;

		Factory = POINTER TO RECORD(GraphStochastic.Factory) END;
		
		RuleList = POINTER TO RECORD 
								rule: GraphJacobi.Node;
								next: RuleList
							END;
	CONST
		order = 8;
		
	VAR
		ruleCache: RuleList;
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
		IF ~(GraphStochastic.data IN node.props) THEN RETURN {GraphNodes.notData} END;
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
		RETURN -2.0 * node.LogLikelihood()
	END Deviance;
	
	PROCEDURE (node: Node) DiffLogConditional (): REAL;
	BEGIN
		RETURN 0.0
	END DiffLogConditional;

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
		GraphNodes.Externalize(node.nu, wr);
		GraphNodes.Externalize(node.function, wr);
		GraphNodes.Externalize(node.t, wr);
		GraphNodes.Externalize(node.rule, wr);
		wr.WriteBool(node.event)
	END ExternalizeStochastic;

	PROCEDURE (node: Node) InternalizeStochastic (VAR rd: Stores.Reader);
		VAR
			p: GraphNodes.Node;
	BEGIN
		node.nu := GraphNodes.Internalize(rd);
		node.function := GraphNodes.Internalize(rd);
		p := GraphNodes.Internalize(rd);
		node.t := p(GraphStochastic.Node);
		p := GraphNodes.Internalize(rd);
		node.rule := p(GraphJacobi.Node);
		rd.ReadBool(node.event)	
	END InternalizeStochastic;

	PROCEDURE (node: Node) InitStochastic;
	BEGIN
		node.props := node.props + {GraphStochastic.noMean, GraphStochastic.noCDF};
		node.nu := NIL;
		node.function := NIL;
		node.t := NIL;
		node.event := FALSE
	END InitStochastic;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphWeibullHazard.Install";
	END Install;

	PROCEDURE (node: Node) InvMap (y: REAL);
	BEGIN
		node.value := y
	END InvMap;

	PROCEDURE (node: Node) IsLikelihoodTerm (): BOOLEAN;
	BEGIN
		RETURN TRUE
	END IsLikelihoodTerm;

	PROCEDURE (node: Node) LogDetJacobian (): REAL;
	BEGIN
		RETURN 0.0
	END LogDetJacobian;

	(*	hazard = F(s) * nu * s ^ (nu - 1) 	*)
	PROCEDURE (node: Node) LogLikelihood (): REAL;
		VAR
			dummy, integral, log, s, time, y, nu, scale: REAL;
			t: GraphStochastic.Node;
			i, order: INTEGER;
			rule: GraphJacobi.Node;
	BEGIN
		(*	this causes Gauss-Jacobi rule to be set up if needs be	*)
		dummy := node.rule.value;
		nu := node.nu.value;
		time := node.value;
		scale := Math.Power(0.5 * time, nu);
		rule := node.rule;
		order := LEN(rule.absisca);
		t := node.t;
		i := 0;
		integral := 0.0; 
		WHILE i < order DO
			s := 0.5 * time * (rule.absisca[i] + 1.0);
			t.value := s;
			y := node.function.value;
			integral := integral + rule.weights[i] * y;
			INC(i)
		END;
		log := -integral * nu * scale;
		IF node.event THEN 
			t.value := time;
			y := node.function.value;
			log := Math.Ln(nu) + (nu - 1.0) * Math.Ln(time) + Math.Ln(y) + log 
		END;
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
		p := node.nu;
		p.AddParent(list);
		p := node.function;
		p.AddParent(list);
		p := node.t;
		p.AddParent(list);
		p := node.rule;
		p.AddParent(list);
		GraphNodes.ClearList(list);
		RETURN list
	END Parents;

	PROCEDURE (node: Node) Representative (): GraphStochastic.Node;
	BEGIN
		RETURN node
	END Representative;

	PROCEDURE (node: Node) Set (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			logicalArgs: GraphStochastic.ArgsLogical;
			p: GraphNodes.Node;
			list: RuleList;
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.nu := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			node.function := args.scalars[1];
			ASSERT(args.scalars[2] # NIL, 21);
			node.t := args.scalars[2](GraphStochastic.Node);
			ASSERT(args.scalars[3] # NIL, 21);
			node.event := args.scalars[3].value > 0.5;
			list := ruleCache;
			WHILE (list # NIL) & (list.rule.beta # node.nu) DO list := list.next END;
			IF list = NIL THEN
				NEW(list);
				list.next := ruleCache;
				ruleCache := list;
				logicalArgs.Init;
				logicalArgs.ops[0] := order;
				logicalArgs.scalars[0] := NIL;
				logicalArgs.scalars[1] := node.nu;
				p := GraphJacobi.fact.New();
				ruleCache.rule :=  p(GraphJacobi.Node);
				ruleCache.rule.Set(logicalArgs, res);
				node.rule := ruleCache.rule
			ELSE
				node.rule := list.rule
			END
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

	PROCEDURE (f: Factory) Clear;
	BEGIN
		ruleCache := NIL
	END Clear;
	
	PROCEDURE (f: Factory) New (): GraphStochastic.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "sHs"
	END Signature;

	PROCEDURE Install*;
		VAR
			fact: Factory;
	BEGIN
		NEW(fact);
		ruleCache := NIL;
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		ruleCache := NIL
	END Init;
	
BEGIN
	Init
END GraphWeibullHazard.

