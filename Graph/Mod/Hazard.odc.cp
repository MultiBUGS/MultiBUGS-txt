(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	  *)

MODULE GraphHazard;

	IMPORT
		Math, Stores := Stores64,
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
		wg, wk, x : ARRAY 8 OF REAL; (*	weights and absiscae for 7/15 guassian kondrad quadrature	*)

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
		node.props := node.props + {GraphStochastic.noMean, GraphStochastic.noCDF};
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

	PROCEDURE (node: Node) LogLikelihood (): REAL;
		VAR
			absisca, integralK, integralG, log, scale, y: REAL;
			t: GraphStochastic.Node;
			i, len: INTEGER;
	BEGIN
		t := node.t;
		scale := 0.5 * node.value;
		t.value:= scale;
		y := node.function.value;
		integralK := wk[0] * y;
		integralG := wg[0] * y;
		i := 1;
		len := LEN(x);
		WHILE i < len DO
			absisca := scale * (1.0 + x[i]);
			y := node.function.value;
			t.value := absisca;
			integralK := integralK + wk[i] * y;
			IF ~ODD(i) THEN integralG := integralG + wg[i DIV 2] * y END;
			absisca := scale * (1.0 - x[i]);
			y := node.function.value;
			t.value := absisca;
			integralK := integralK + wk[i] * y;
			IF ~ODD(i) THEN integralG := integralG + wg[i DIV 2] * y END;
			INC(i)
		END;
		ASSERT(ABS(integralK - integralG) < 1.0E-3 * (integralK + integralG), 77); 
		log := -integralK * scale;
		IF node.event THEN 
			t.value := node.value;
			log := Math.Ln(node.function.value) + log 
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
			node.event := args.scalars[2].value > 0.5
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
		signature := "Hs"
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
		(*	Guass weights	*)
		wg[0] := 4.179591836734693877551020408163265E-01;
		wg[1] := 3.818300505051189449503697754889751E-01;
		wg[2] := 2.797053914892766679014677714237796E-01;
		wg[3] := 1.294849661688696932706114326790820E-01;

		(*	Guass-Kondrod points	*)
		x[0] := 0.000000000000000000000000000000000E+00;	
		x[1] := 2.077849550078984676006894037732449E-01;	
		x[2] := 4.058451513773971669066064120769615E-01;	
		x[3] := 5.860872354676911302941448382587296E-01;	
		x[4] := 7.415311855993944398638647732807884E-01;	
		x[5] := 8.648644233597690727897127886409262E-01;	
		x[6] := 9.491079123427585245261896840478513E-01;	
		x[7] := 9.914553711208126392068546975263285E-01;	

		(*	Kondrod weights	*)
		wk[0] := 2.094821410847278280129991748917143E-01;
		wk[1] := 2.044329400752988924141619992346491E-01;
		wk[2] := 1.903505780647854099132564024210137E-01;
		wk[3] := 1.690047266392679028265834265985503E-01;
		wk[4] := 1.406532597155259187451895905102379E-01;
		wk[5] := 1.047900103222501838398763225415180E-01;
		wk[6] := 6.309209262997855329070066318920429E-02;
		wk[7] := 2.293532201052922496373200805896959E-02
	END Init;

BEGIN
	Init
END GraphHazard.

