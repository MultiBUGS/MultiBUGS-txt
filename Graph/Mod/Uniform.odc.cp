(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)

MODULE GraphUniform;


	

	IMPORT
		Math, Stores,
		GraphConjugateUV, GraphConstant, GraphNodes, GraphRules, GraphStochastic,
		GraphUnivariate,
		MathFunc, MathRandnum;

	TYPE
		Node = POINTER TO RECORD(GraphConjugateUV.Node)
			a, b: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD(GraphUnivariate.Factory) END;

	CONST
		eps = 1.0E-10;

	VAR
		fact-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) BoundsUnivariate (OUT left, right: REAL);
	BEGIN
		left := node.a.Value();
		right := node.b.Value()
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
		VAR
			a, b, x: REAL;
	BEGIN
		x := node.value;
		a := node.a.Value();
		b := node.b.Value();
		IF x < a - eps THEN
			RETURN {GraphNodes.leftBound, GraphNodes.lhs}
		END;
		IF x > b + eps THEN
			RETURN {GraphNodes.rightBound, GraphNodes.lhs}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
		CONST
			eps = 1.0E-20;
		VAR
			density, f0, f1: INTEGER;
	BEGIN
		f0 := GraphStochastic.ClassFunction(node.a, parent);
		f1 := GraphStochastic.ClassFunction(node.b, parent);
		IF f0 = GraphRules.const THEN
			IF f1 = GraphRules.const THEN
				density := GraphRules.unif
			ELSIF (f1 = GraphRules.ident) & (GraphNodes.data IN node.a.props)
				 & (ABS(node.a.Value()) < eps) THEN
				density := GraphRules.pareto
			ELSE
				density := GraphRules.general
			END
		ELSE density := GraphRules.general
		END;
		RETURN density
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.unif
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (x: REAL): REAL;
		VAR
			a, b: REAL;
	BEGIN
		a := node.a.Value();
		b := node.b.Value();
		RETURN (b - x) / (b - a)
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			a, b, logDensity: REAL;
	BEGIN
		a := node.a.Value();
		b := node.b.Value();
		logDensity := - Math.Ln(b - a);
		RETURN - 2.0 * logDensity
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			a, b, diffA, diffB, differential: REAL;
	BEGIN
		IF (GraphStochastic.hint2 IN x.props) OR (GraphNodes.data IN node.b.props) THEN
			node.a.ValDiff(x, a, diffA);
			b := node.b.Value();
			differential := diffA / ((b - a) * (b - a))
		ELSIF (GraphStochastic.hint1 IN x.props) OR (GraphNodes.data IN node.a.props) THEN
			a := node.a.Value();
			node.b.ValDiff(x, b, diffB);
			differential := - diffB / ((b - a) * (b - a))
		ELSE
			node.a.ValDiff(x, a, diffA);
			node.b.ValDiff(x, b, diffB);
			differential := (diffA - diffB) / ((b - a) * (b - a))
		END;
		RETURN differential
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
	BEGIN
		RETURN 0.0
	END DiffLogPrior;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.a, wr);
		GraphNodes.Externalize(node.b, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.a := GraphNodes.Internalize(rd);
		node.b := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.leftNatural, GraphStochastic.rightNatural});
		node.a := NIL;
		node.b := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphUniform.Install"
	END Install;

	PROCEDURE (likelihood: Node) LikelihoodForm (as: INTEGER; VAR x: GraphNodes.Node;
	OUT p0, p1: REAL);
	BEGIN
		ASSERT(as = GraphRules.pareto, 21);
		p0 := 1.0;
		p1 := likelihood.value;
		x := likelihood.b
	END LikelihoodForm;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			a, b, logLikelihood: REAL;
	BEGIN
		a := node.a.Value();
		b := node.b.Value();
		IF (node.value >= a) & (node.value <= b) THEN
			logLikelihood := - Math.Ln(b - a)
		ELSE
			logLikelihood := MathFunc.logOfZero
		END;
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			a, b: REAL;
	BEGIN
		a := node.a.Value();
		b := node.b.Value();
		RETURN 0.5 * (a + b)
	END Location;

	PROCEDURE (node: Node) LogPrior (): REAL;
	BEGIN
		RETURN 0
	END LogPrior;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.a.AddParent(list);
		node.b.AddParent(list);
		GraphNodes.ClearList(list);
		RETURN list
	END ParentsUnivariate;
	
	PROCEDURE (prior: Node) PriorForm (as: INTEGER; OUT p0, p1: REAL);
	BEGIN
		ASSERT(as IN {GraphRules.beta, GraphRules.gamma, GraphRules.normal}, 21);
		IF as = GraphRules.beta THEN
			p0 := 1.0;
			p1 := 1.0
		ELSIF as = GraphRules.gamma THEN
			p0 := 1.0;
			p1 := 0.0
		ELSIF as = GraphRules.normal THEN
			p0 := 0.0;
			p1 := 0.0
		END
	END PriorForm;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			a, b, value: REAL;
	BEGIN
		a := node.a.Value();
		b := node.b.Value();
		value := MathRandnum.Uniform(a, b);
		node.SetValue(value);
		res := {}
	END Sample;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.a := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			node.b := args.scalars[1]
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

	PROCEDURE Vector* (size: INTEGER; lower, upper: GraphNodes.Node): GraphStochastic.Vector;
		VAR
			i: INTEGER;
			args: GraphStochastic.Args;
			uniforms: GraphStochastic.Vector;
			props, res: SET;
	BEGIN
		NEW(uniforms, size);
		args.scalars[0] := lower;
		args.scalars[1] := upper;
		i := 0;
		WHILE i < size DO
			uniforms[i] := fact.New();
			uniforms[i].Set(args, res);
			props := uniforms[i].props;
			uniforms[i].SetProps(props + {GraphStochastic.hidden, GraphStochastic.initialized});
			INC(i)
		END;
		RETURN uniforms
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
END GraphUniform.

