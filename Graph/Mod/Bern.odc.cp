(*	 	

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)

MODULE GraphBern;


	

	IMPORT
		Stores,
		GraphConjugateUV, GraphConstant, GraphNodes, GraphRules,
		GraphStochastic, GraphUnivariate,
		MathFunc, MathRandnum;

	TYPE
		Node = POINTER TO RECORD(GraphConjugateUV.Node)
			p: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD(GraphUnivariate.Factory) END;

	VAR
		fact-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) BoundsUnivariate (OUT lower, upper: REAL);
	BEGIN
		lower := 0;
		upper := 1
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
		CONST
			eps = 1.0E-5;
		VAR
			r: INTEGER;
			p: REAL;
	BEGIN
		r := SHORT(ENTIER(node.value + eps));
		IF ABS(r - node.value) > eps THEN
			RETURN {GraphNodes.integer, GraphNodes.lhs}
		END;
		IF (r # 0) & (r # 1) THEN
			RETURN {GraphNodes.invalidInteger, GraphNodes.lhs}
		END;
		p := node.p.Value();
		IF (p <= - eps) OR (p >= 1.0 + eps) THEN
			RETURN {GraphNodes.proportion, GraphNodes.arg1}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
		VAR
			form: INTEGER;
	BEGIN
		form := GraphStochastic.ClassFunction(node.p, parent);
		RETURN GraphRules.ClassifyProportion(form)
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.catagorical
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (r: REAL): REAL;
		VAR
			cumulative, p: REAL;
	BEGIN
		p := node.p.Value();
		IF r < 0.50 THEN
			cumulative := 1.0 - p
		ELSE
			cumulative := 1.0
		END;
		RETURN cumulative
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			p, r: REAL;
	BEGIN
		r := node.value;
		p := node.p.Value();
		IF r < 0.5 THEN
			RETURN - 2 * MathFunc.Ln(1 - p)
		ELSE
			RETURN - 2 * MathFunc.Ln(p)
		END
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			diff, p, r: REAL;
	BEGIN
		r := node.value;
		node.p.ValDiff(x, p, diff);
		IF r < 0.5 THEN
			RETURN - diff / (1 - p)
		ELSE
			RETURN diff / p
		END
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.p, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.integer});
		node.p := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.p := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphBern.Install"
	END Install;

	PROCEDURE (offspring: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		ASSERT(as = GraphRules.beta, 21);
		x := offspring.p;
		p0 := offspring.value;
		p1 := 1.0 - offspring.value
	END LikelihoodForm;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			p, r: REAL;
	BEGIN
		r := node.value;
		p := node.p.Value();
		IF r < 0.5 THEN
			RETURN MathFunc.Ln(1 - p)
		ELSE
			RETURN MathFunc.Ln(p)
		END
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			p, r: REAL;
	BEGIN
		r := node.value;
		p := node.p.Value();
		IF r < 0.5 THEN
			RETURN MathFunc.Ln(1 - p)
		ELSE
			RETURN MathFunc.Ln(p)
		END
	END LogPrior;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			p: REAL;
	BEGIN
		p := node.p.Value();
		RETURN p
	END Location;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		p := node.p;
		p.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		HALT(0)
	END PriorForm;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			p, r: REAL;
	BEGIN
		res := {};
		p := node.p.Value();
		r := MathRandnum.Bernoulli(p);
		node.SetValue(r)
	END Sample;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.p := args.scalars[0]
		END
	END SetUnivariate;

	PROCEDURE (f: Factory) New (): GraphUnivariate.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		node.SetProps(node.props + 
		{GraphStochastic.integer, GraphStochastic.leftNatural, GraphStochastic.rightNatural});
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

	PROCEDURE Vector* (size: INTEGER; p: REAL): GraphStochastic.Vector;
		VAR
			i: INTEGER;
			args: GraphStochastic.Args;
			bernoullis: GraphStochastic.Vector;
			props, res: SET;
	BEGIN
		NEW(bernoullis, size);
		args.Init;
		args.numScalars := 1;
		args.scalars[0] := GraphConstant.New(p);
		i := 0;
		WHILE i < size DO
			bernoullis[i] := fact.New();
			bernoullis[i].Set(args, res);
			props := bernoullis[i].props;
			bernoullis[i].SetProps(props + {GraphStochastic.hidden, GraphStochastic.initialized});
			INC(i)
		END;
		RETURN bernoullis
	END Vector;

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
END GraphBern.

