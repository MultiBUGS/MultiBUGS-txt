(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)



MODULE GraphHypergeometric;


	

	IMPORT
		Math, Stores,
		GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE
		Node = POINTER TO RECORD(GraphUnivariate.Node)
			n1, m1, n, psi: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD(GraphUnivariate.Factory) END;

	CONST
		eps = 1.0E-5;
		maxIts = 100000;
		hgSize = 100;

		(* Approximation for saving computing time by chopping off the tail values that have
		negligible probability. See Liao and Rosen (Am.Stat 55(4), 2001)
		Could this be in some Rsrc file so the user can change it?  Use delta=0 for exact. *)

		delta = 1.0E-14;

	VAR
		fact-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		hgProbs: POINTER TO ARRAY OF REAL;

		(*
		* HYPERGEOMETRIC-SPECIFIC PROCEDURES. Algorithms from Liao and Rosen
		(Am. Stat 55(4), 2001)
		*)

		(* Recursive relation for calculating the probabilities *)

	PROCEDURE Rfunc (n1, m1, n: INTEGER; psi: REAL; i: INTEGER): REAL;
	BEGIN
		RETURN (n1 - i + 1) * (m1 - i + 1) * psi / (i * (n - m1 - n1 + i))
	END Rfunc;

	(* Mode of the distribution. If this cannot be computed, start calculations at left limit *)

	PROCEDURE Mode (n1, m1, n: INTEGER; psi: REAL): INTEGER;
		VAR
			left, right, mm: INTEGER;
			a, b, c, q: REAL;
	BEGIN
		left := MAX(0, m1 - n + n1);
		right := MIN(n1, m1);
		a := psi - 1;
		b :=  - ((m1 + n1 + 2) * psi + n - n1 - m1);
		c := psi * (n1 + 1) * (m1 + 1);
		q :=  - (b + Math.Sign(b) * Math.Sqrt(b * b - 4 * a * c)) / 2;
		mm := SHORT(ENTIER(c / q));
		IF ((right >= mm) & (mm >= left)) THEN
			RETURN mm
		ELSE
			RETURN left
		END
	END Mode;

	(* Calculate non-normalised probabilities as far as the probability for a single desired x, and return that
	probability. Used for the prior density of x *)

	PROCEDURE HGprob (n1, m1, n: INTEGER; psi: REAL; x: INTEGER): REAL;
		VAR
			i, left, mode, right: INTEGER;
			p, r: REAL;
	BEGIN
		left := MAX(0, m1 - n + n1);
		right := MIN(n1, m1);
		ASSERT(x >= left, 60);
		ASSERT(x <= right, 61);
		mode := Mode(n1, m1, n, psi);
		p := 1;
		IF x = mode THEN
			RETURN p
		END;
		IF (mode < right) & (x > mode) THEN
			i := 0;
			REPEAT
				r := Rfunc(n1, m1, n, psi, mode + 1 + i);
				p := p * r;
				INC(i);
				IF ((p < delta / 10) & (r < 5 / 6)) THEN
					RETURN 0.0
				END
			UNTIL i = x - mode;
			RETURN p
		END;
		IF (mode > left) & (x < mode) THEN
			i := 0;
			REPEAT
				r := Rfunc(n1, m1, n, psi, mode - i);
				p := p / r;
				INC(i);
				IF ((p < delta / 10) & (r > 6 / 5)) THEN
					RETURN 0.0
				END
			UNTIL i = mode - x;
			RETURN p
		END
	END HGprob;

	(*
	* GENERIC PROCEDURES
	*)

	PROCEDURE Bounds (node: Node; OUT left, right: INTEGER);
		VAR
			l, u: REAL;
	BEGIN
		node.Bounds(l, u);
		left := SHORT(ENTIER(l + eps));
		right := SHORT(ENTIER(u + eps))
	END Bounds;

	PROCEDURE (node: Node) BoundsUnivariate (OUT left, right: REAL);
		VAR
			m1, n, n1: REAL;
	BEGIN
		n1 := node.n1.Value();
		m1 := node.m1.Value();
		n := node.n.Value();
		left := MAX(0, m1 - n + n1);
		right := MIN(n1, m1)
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
		VAR
			left, m1, n, n1, r, right: INTEGER;
			psi: REAL;
	BEGIN
		r := SHORT(ENTIER(node.value + eps));
		IF ABS(r - node.value) > eps THEN
			RETURN {GraphNodes.integer, GraphNodes.lhs}
		END;
		n1 := SHORT(ENTIER(node.n1.Value() + eps));
		m1 := SHORT(ENTIER(node.m1.Value() + eps));
		n := SHORT(ENTIER(node.n.Value() + eps));
		psi := node.psi.Value();
		Bounds(node, left, right);
		IF ABS(node.n1.Value() - n1) > eps THEN
			RETURN {GraphNodes.integer, 0}
		END;
		IF ABS(node.m1.Value() - m1) > eps THEN
			RETURN {GraphNodes.integer, 1}
		END;
		IF ABS(node.n.Value() - n) > eps THEN
			RETURN {GraphNodes.integer, 2}
		END;
		IF (n1 < 0) OR (n1 > n) THEN
			RETURN {GraphNodes.invalidInteger, 0}
		END;
		IF (m1 < 0) OR (m1 > n) THEN
			RETURN {GraphNodes.invalidInteger, 1}
		END;
		IF n < 1 THEN
			RETURN {GraphNodes.invalidInteger, 2}
		END;
		IF r < left THEN
			RETURN {GraphNodes.leftBound, GraphNodes.lhs}
		END;
		IF r > right THEN
			RETURN {GraphNodes.rightBound, GraphNodes.lhs}
		END;
		IF psi < eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg3}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
	BEGIN
		RETURN GraphRules.catagorical
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.catagorical
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (x: REAL): REAL;
		VAR
			m1, n, n1: INTEGER;
			psi: REAL;
	BEGIN
		n1 := SHORT(ENTIER(node.n1.Value() + eps));
		m1 := SHORT(ENTIER(node.m1.Value() + eps));
		n := SHORT(ENTIER(node.n.Value() + eps));
		psi := node.psi.Value();
		RETURN MathCumulative.Hypergeometric(n1, m1, n, psi, x)
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			ok: BOOLEAN;
			left, n1, m1, n, r, nprobs, shift, right: INTEGER;
			logDensity, psi: REAL;
	BEGIN
		r := SHORT(ENTIER(node.value));
		n1 := SHORT(ENTIER(node.n1.Value()));
		m1 := SHORT(ENTIER(node.m1.Value()));
		n := SHORT(ENTIER(node.n.Value()));
		psi := node.psi.Value();
		Bounds(node, left, right);
		shift := 1 - left;
		nprobs := right - left + 1;
		IF nprobs > LEN(hgProbs) THEN NEW(hgProbs, nprobs) END;
		MathFunc.HGprobs(n1, m1, n, psi, hgProbs, ok);
		logDensity := Math.Ln(hgProbs[r + shift - 1]);
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
		GraphNodes.Externalize(node.n1, wr);
		GraphNodes.Externalize(node.m1, wr);
		GraphNodes.Externalize(node.n, wr);
		GraphNodes.Externalize(node.psi, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + 
		{GraphStochastic.integer, GraphStochastic.leftNatural, GraphStochastic.rightNatural});
		node.n1 := NIL;
		node.m1 := NIL;
		node.n := NIL;
		node.psi := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.n1 := GraphNodes.Internalize(rd);
		node.m1 := GraphNodes.Internalize(rd);
		node.n := GraphNodes.Internalize(rd);
		node.psi := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			ok: BOOLEAN;
			left, m1, n, n1, r, nprobs, shift, right: INTEGER;
			logLikelihood, psi: REAL;
	BEGIN
		r := SHORT(ENTIER(node.value + eps));
		n1 := SHORT(ENTIER(node.n1.Value() + eps));
		m1 := SHORT(ENTIER(node.m1.Value() + eps));
		n := SHORT(ENTIER(node.n.Value() + eps));
		psi := node.psi.Value();
		Bounds(node, left, right);
		(* zero likelihood for out of range values *)
		IF (r < left) OR(r > right) THEN
			RETURN MathFunc.logOfZero
		END;
		shift := 1 - left;
		nprobs := right - left + 1;
		IF nprobs > LEN(hgProbs) THEN NEW(hgProbs, nprobs) END;
		MathFunc.HGprobs(n1, m1, n, psi, hgProbs, ok);
		logLikelihood := Math.Ln(hgProbs[r + shift - 1]);
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			m1, n, n1, r: INTEGER;
			logPrior, psi: REAL;
	BEGIN
		r := SHORT(ENTIER(node.value));
		n1 := SHORT(ENTIER(node.n1.Value()));
		m1 := SHORT(ENTIER(node.m1.Value()));
		n := SHORT(ENTIER(node.n.Value()));
		psi := node.psi.Value();
		logPrior := Math.Ln(HGprob(n1, m1, n, psi, r));
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			m1: REAL;
	BEGIN
		m1 := node.m1.Value();
		RETURN m1
	END Location;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphHypergeometric.Install"
	END Install;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		p := node.n1;
		p.AddParent(list);
		p := node.m1;
		p.AddParent(list);
		p := node.n;
		p.AddParent(list);
		p := node.psi;
		p.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.n1 := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			node.m1 := args.scalars[1];
			ASSERT(args.scalars[2] # NIL, 21);
			node.n := args.scalars[2];
			ASSERT(args.scalars[3] # NIL, 21);
			node.psi := args.scalars[3]
		END
	END SetUnivariate;

	PROCEDURE (node: Node) ModifyUnivariate (): GraphUnivariate.Node;
		VAR
			p: Node;
	BEGIN
		NEW(p);
		p^ := node^;
		RETURN p
	END ModifyUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			i, left, m1, n, n1, r, right: INTEGER;
			lower, upper, psi: REAL;
	BEGIN
		n1 := SHORT(ENTIER(node.n1.Value() + eps));
		m1 := SHORT(ENTIER(node.m1.Value() + eps));
		n := SHORT(ENTIER(node.n.Value() + eps));
		psi := node.psi.Value();
		node.Bounds(lower, upper);
		left := SHORT(ENTIER(lower + eps));
		right := SHORT(ENTIER(upper + eps));
		i := maxIts;
		REPEAT
			r := MathRandnum.Hypergeometric(n1, m1, n, psi);
			DEC(i)
		UNTIL ((r > left) & (r < right)) OR (i = 0) OR (r =  - 1);
		IF (i = 0) OR (r =  - 1) THEN
			res := {GraphNodes.lhs}
		ELSE
			node.SetValue(r);
			res := {}
		END
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
		signature := "ssssCT"
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "C.Jackson"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f;
		NEW(hgProbs, hgSize)
	END Init;

BEGIN
	Init
END GraphHypergeometric.

