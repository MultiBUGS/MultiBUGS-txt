(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	  *)

MODULE GraphWeibullHazard;

	IMPORT
		Math, Stores,
		MathJacobi,
		GraphNodes, GraphRules, GraphStochastic;

	TYPE
		Node = POINTER TO RECORD(GraphStochastic.Node) 
			nu: GraphNodes.Node;
			function: GraphNodes.Node;
			t: GraphStochastic.Node;
			event: BOOLEAN
		END;

		Factory = POINTER TO RECORD(GraphStochastic.Factory) END;

	CONST
		ruleOrder = 8;
		grid = 100;
		delta = 1.0 / grid;
		eps = 1.0E-10;
		maxNu = 5;
		numNu = grid * maxNu;
		
	VAR
		fact-: GraphStochastic.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		(*	table of Gauss-Jacobi quadrature rules	*)
		gaussJacobiW, gaussJacobiX: ARRAY numNu OF ARRAY ruleOrder OF REAL;	

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
			nu: REAL;
	BEGIN
		res := {};
		IF ~(GraphStochastic.data IN node.props) THEN RETURN {GraphNodes.notData} END;
		nu := node.nu.Value();
		IF nu < - eps THEN RETURN {GraphNodes.posative, GraphNodes.arg1} END;
		IF nu > maxNu - eps THEN RETURN {GraphNodes.invalidPosative, GraphNodes.arg1} END;
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
		rd.ReadBool(node.event)	
	END InternalizeStochastic;

	PROCEDURE (node: Node) InitStochastic;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.noMean, GraphStochastic.noCDF});
		node.nu := NIL;
		node.function := NIL;
		node.t := NIL;
		node.event := FALSE
	END InitStochastic;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphWeibullHazard.Install"
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

	(*	hazard = F(s) * nu * s ^ (nu - 1) 	*)
	PROCEDURE (node: Node) LogLikelihood (): REAL;
		VAR
			integral, log, s, time, y, nu, deltaNu, scale: REAL;
			t: GraphStochastic.Node;
			i, index: INTEGER;
	BEGIN
		nu := node.nu.Value();
		(*	use the tabulated Gauss-Jacobi quadrature rule that has a beta just below the 
			   actual beta and correct the integrand with a power of s	*)
		index := SHORT(ENTIER(nu / delta)) - 1;
		ASSERT(index >= 0, 66);
		deltaNu := nu - index * delta;
		time := node.value;
		(*	
			  evaluate integral of hazard using pre-computed Gauss-Jacobi quadrature rule
			  change of variable s = 1/2 t (x + 1);  ds = 1/2 t dx; 
			  s = 0 => x = -1; s = t => x = 1
		*)
		scale := Math.Power(0.5 * time, index * delta);
		t := node.t;
		i := 0;
		integral := 0.0;
		WHILE i < ruleOrder DO
			s := 0.5 * time * (gaussJacobiX[index, i] + 1.0);
			t.SetValue(s);
			y := node.function.Value() * nu * Math.Power(s, deltaNu);
			integral := integral + gaussJacobiW[index, i] * y;
			INC(i)
		END;
		log := -integral * scale;
		IF node.event THEN 
			t.SetValue(time);
			y := node.function.Value();
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
			node.nu := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			node.function := args.scalars[1];
			ASSERT(args.scalars[2] # NIL, 21);
			node.t := args.scalars[2](GraphStochastic.Node);
			ASSERT(args.scalars[3] # NIL, 21);
			node.event := args.scalars[3].Value() > 0.5
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
		signature := "sFs"
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
			i: INTEGER;
			alpha, beta: REAL;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f;
		(*	construct a grid of Gauss-Jacobi quadrature rules for alpha = 0 and different values of beta	*)
		i := 0;
		alpha := 0.0;
		WHILE i < numNu DO
			beta := (i  + 1) * delta - 1.0;
			MathJacobi.QuadratureRule(gaussJacobiX[i], gaussJacobiW[i], alpha, beta, ruleOrder);
			INC(i);
		END;
	END Init;

BEGIN
	Init
END GraphWeibullHazard.

