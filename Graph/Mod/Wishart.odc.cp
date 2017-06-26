(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)

MODULE GraphWishart;

	

	IMPORT
		Math, Stores,
		GraphConjugateMV, GraphMultivariate, GraphNodes, GraphRules, GraphStochastic,
		MathMatrix, MathRandnum;

		(*	The Wishart distribution can only be used as a prior in statistical models

		Its parameters must have fixed values, that is they can not be estimated.

		*)


	TYPE
		Node = POINTER TO RECORD(GraphConjugateMV.Node)
			r: GraphNodes.Vector;
			k: GraphNodes.Node;
			dim, start, step: INTEGER
		END;

		Factory = POINTER TO RECORD(GraphMultivariate.Factory) END;

	CONST
		eps = 1.0E-10;

	VAR
		fact-: GraphMultivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		value, s: POINTER TO ARRAY OF ARRAY OF REAL;

	PROCEDURE (node: Node) BoundsConjugateMV (OUT lower, upper: REAL);
	BEGIN
		lower := - INF;
		upper := INF
	END BoundsConjugateMV;

	PROCEDURE (node: Node) CheckConjugateMV (): SET;
	BEGIN
		RETURN {}
	END CheckConjugateMV;

	PROCEDURE (node: Node) ClassifyLikelihood (parent: GraphStochastic.Node): INTEGER;
	BEGIN
		HALT(126);
		RETURN 0
	END ClassifyLikelihood;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.wishart
	END ClassifyPrior;

	PROCEDURE (node: Node) Deviance (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END Deviance;

	PROCEDURE (node: Node) DiffLogConditionalMap (): REAL;
	BEGIN
		HALT(126);
		RETURN 0
	END DiffLogConditionalMap;

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

	PROCEDURE (node: Node) ExternalizeConjugateMV (VAR wr: Stores.Writer);
		VAR
			v: GraphNodes.SubVector;
			size: INTEGER;
	BEGIN
		IF node.index = 0 THEN
			size := node.Size();
			wr.WriteInt(node.dim);
			v := GraphNodes.NewVector();
			v.components := node.r;
			v.start := node.start; v.nElem := size; v.step := node.step;
			GraphNodes.ExternalizeSubvector(v, wr);
			GraphNodes.Externalize(node.k, wr)
		END
	END ExternalizeConjugateMV;

	PROCEDURE (node: Node) InternalizeConjugateMV (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
			dim, i, size: INTEGER;
			p: Node;
	BEGIN
		IF node. index = 0 THEN
			size := node.Size();
			rd.ReadInt(dim);
			IF dim > LEN(value, 0) THEN
				NEW(value, dim, dim);
				NEW(s, dim, dim);
			END;
			GraphNodes.InternalizeSubvector(v, rd);
			node.r := v.components;
			node.start := v.start;
			node.step := v.step;
			node.dim := dim;
			node.k := GraphNodes.Internalize(rd);
			i := 1;
			WHILE i < size DO
				p := node.components[0](Node);
				p.start := node.start;
				p.step := node.step;
				p.r := node.r;
				p.dim := node.dim;
				p.k := node.k;
				INC(i)
			END
		END
	END InternalizeConjugateMV;

	PROCEDURE (node: Node) InitConjugateMV;
	BEGIN
		node.r := NIL;
		node.k := NIL;
		node.dim := 0;
		node.start := - 1;
		node.step := 0
	END InitConjugateMV;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphWishart.Install"
	END Install;

	PROCEDURE (node: Node) InvMap (y: REAL);
	BEGIN
		HALT(126);
	END InvMap;

	PROCEDURE (node: Node) LogJacobian (): REAL;
	BEGIN
		HALT(126);
		RETURN 0
	END LogJacobian;

	PROCEDURE (node: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		HALT(126)
	END LikelihoodForm;

	PROCEDURE (node: Node) Map (): REAL;
	BEGIN
		HALT(126);
		RETURN node.value
	END Map;

	PROCEDURE (node: Node) MVLikelihoodForm (as: INTEGER; OUT x: GraphNodes.Vector;
	OUT start, step: INTEGER; OUT p0: ARRAY OF REAL; OUT p1: ARRAY OF ARRAY OF REAL);
	BEGIN
		HALT(126)
	END MVLikelihoodForm;

	PROCEDURE (node: Node) LogMVPrior (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END LogMVPrior;

	PROCEDURE (prior: Node) MVPriorForm (as: INTEGER; OUT p0: ARRAY OF REAL;
	OUT p1: ARRAY OF ARRAY OF REAL);
		VAR
			dim, i, j, start, step: INTEGER;
			node: GraphNodes.Node;
	BEGIN
		ASSERT(as = GraphRules.wishart, 21);
		i := 0;
		dim := prior.dim;
		start := prior.start;
		step := prior.step;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				node := prior.r[start + (i * dim + j) * step];
				p1[i, j] := node.Value();
				INC(j)
			END;
			INC(i)
		END;
		p0[0] := prior.k.Value()
	END MVPriorForm;

	PROCEDURE (node: Node) MVSample (OUT res: SET);
	BEGIN
		node.Sample(res)
	END MVSample;

	PROCEDURE (node: Node) LogLikelihood (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END LogLikelihood;

	PROCEDURE (node: Node) LogPrior (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END LogPrior;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL); BEGIN
		HALT(126)
	END PriorForm;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			dim, i, j, start, step, index: INTEGER;
			k: REAL;
	BEGIN
		dim := node.dim;
		start := node.start;
		step := node.step;
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				s[i, j] := node.r[start + (i + dim * j) * step].Value();
				INC(j)
			END;
			INC(i)
		END;
		k := node.k.Value();
		MathMatrix.Invert(s, dim);
		index := node.index;
		i := index DIV dim;
		j := index MOD dim;
		RETURN k * s[i, j]
	END Location;

	PROCEDURE (node: Node) ParentsConjugateMV (all: BOOLEAN): GraphNodes.List;
	BEGIN
		RETURN NIL
	END ParentsConjugateMV;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			dim, i, j, start, step: INTEGER;
			k: REAL;
	BEGIN
		dim := node.dim;
		start := node.start;
		step := node.step;
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				s[i, j] := node.r[start + (i + dim * j) * step].Value();
				INC(j)
			END;
			INC(i)
		END;
		k := node.k.Value();
		MathMatrix.Invert(s, dim);
		MathMatrix.Cholesky(s, dim);
		MathRandnum.Wishart(s, k, dim, value);
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO node.components[i * dim + j].SetValue(value[i, j]);
				INC(j)
			END;
			INC(i)
		END;
		res := {}
	END Sample;

	PROCEDURE (node: Node) SetConjugateMV (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			dim, i, nElem, start, step: INTEGER;
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			nElem := node.Size();
			dim := SHORT(ENTIER(Math.Sqrt(nElem) + eps));
			IF dim * dim # nElem THEN
				res := {GraphNodes.length, GraphNodes.lhs};
				RETURN
			END;
			node.dim := dim;
			IF dim > LEN(s, 0) THEN
				NEW(s, dim, dim);
				NEW(value, dim, dim)
			END;
			ASSERT(args.vectors[0].components # NIL, 21);
			IF args.vectors[0].nElem # nElem THEN
				res := {GraphNodes.length, GraphNodes.arg1};
				RETURN
			END;
			node.r := args.vectors[0].components;
			ASSERT(args.vectors[0].start >= 0, 21);
			node.start := args.vectors[0].start;
			node.step := args.vectors[0].step;
			ASSERT(args.scalars[0] # NIL, 21);
			node.k := args.scalars[0];
			IF ~(GraphNodes.data IN node.k.props) THEN
				res := {GraphNodes.data, GraphNodes.arg2};
				RETURN
			END;
			IF node.k.Value() < node.dim - eps THEN
				res := {GraphNodes.invalidPosative, GraphNodes.arg2};
				RETURN
			END;
			i := 0;
			start := node.start;
			step := node.step;
			WHILE i < nElem DO
				IF GraphNodes.data IN node.components[i].props THEN
					res := {GraphNodes.data, GraphNodes.lhs};
					RETURN
				END;
				IF node.r[start + i] = NIL THEN
					res := {GraphNodes.nil, GraphNodes.arg1};
					RETURN
				END;
				IF ~(GraphNodes.data IN node.r[start + i * step].props) THEN
					res := {GraphNodes.data, GraphNodes.arg1};
					RETURN
				END;
				INC(i)
			END
		END
	END SetConjugateMV;

	PROCEDURE (f: Factory) New (): GraphMultivariate.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vs"
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
		CONST
			nElem = 2;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		NEW(s, nElem, nElem);
		NEW(value, nElem, nElem);
		fact := f
	END Init;

BEGIN
	Init
END GraphWishart.

