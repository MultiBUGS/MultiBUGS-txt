(*	

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphTrapezium;


	

	IMPORT
		Math, Stores := Stores64,
		GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathFunc, MathRandnum;

	TYPE
		Node = POINTER TO RECORD(GraphUnivariate.Node)
			a, b, c, d: GraphNodes.Node
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
		right := node.d.Value()
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
		VAR
			a, d, x: REAL;
	BEGIN
		x := node.value;
		a := node.a.Value();
		d := node.d.Value();
		IF x < a - eps THEN
			RETURN {GraphNodes.leftBound, GraphNodes.lhs}
		END;
		IF x > d + eps THEN
			RETURN {GraphNodes.rightBound, GraphNodes.lhs}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
		VAR
			density, f: INTEGER;
	BEGIN
		density := GraphRules.unif;
		f := GraphStochastic.ClassFunction(node.a, parent);
		IF f # GraphRules.const THEN
			density := GraphRules.general
		END;
		f := GraphStochastic.ClassFunction(node.b, parent);
		IF f # GraphRules.const THEN
			density := GraphRules.general
		END;
		f := GraphStochastic.ClassFunction(node.c, parent);
		IF f # GraphRules.const THEN
			density := GraphRules.general
		END;
		f := GraphStochastic.ClassFunction(node.d, parent);
		IF f # GraphRules.const THEN
			density := GraphRules.general
		END;
		RETURN density
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.general
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (x: REAL): REAL;
		VAR
			a, b, c, culm, d, norm: REAL;
	BEGIN
		a := node.a.Value();
		b := node.b.Value();
		c := node.c.Value();
		d := node.d.Value();
		norm := 0.5 * (c + d - a - b);
		IF x < b THEN
			culm := 0.5 * (x - a) * (x - a) / (norm * (b - a))
		ELSIF x > c THEN
			culm := 1 - 0.5 * (d - x) * (d - x) / (norm * (d - c))
		ELSE
			culm := (0.5 * (b - a) + (x - c)) / norm
		END;
		RETURN culm
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			logDensity: REAL;
	BEGIN
		logDensity := node.LogLikelihood();
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
		GraphNodes.Externalize(node.a, wr);
		GraphNodes.Externalize(node.b, wr);
		GraphNodes.Externalize(node.c, wr);
		GraphNodes.Externalize(node.d, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.a := GraphNodes.Internalize(rd);
		node.b := GraphNodes.Internalize(rd);
		node.c := GraphNodes.Internalize(rd);
		node.d := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.leftNatural, GraphStochastic.rightNatural});
		node.a := NIL;
		node.b := NIL;
		node.c := NIL;
		node.d := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphTrapezium.Install"
	END Install;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			a, b, c, d, logLikelihood, norm, x: REAL;
	BEGIN
		a := node.a.Value();
		b := node.b.Value();
		c := node.c.Value();
		d := node.d.Value();
		x := node.value;
		IF (b < a) OR (c < a) OR (d < a) THEN
			logLikelihood := MathFunc.logOfZero
		ELSIF (c < b) OR (d < b) THEN
			logLikelihood := MathFunc.logOfZero
		ELSIF d < c THEN
			logLikelihood := MathFunc.logOfZero
		ELSIF (x >= a) & (x <= d) THEN
			norm := 0.5 * (c + d - a - b);
			IF x < b THEN
				logLikelihood := Math.Ln((x - a) / (norm * (b - a)))
			ELSIF x > c THEN
				logLikelihood := Math.Ln((d - x) / (norm * (d - c)))
			ELSE
				logLikelihood := Math.Ln(1 / norm)
			END
		ELSE
			logLikelihood := MathFunc.logOfZero
		END;
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

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
		node.a.AddParent(list);
		node.b.AddParent(list);
		node.c.AddParent(list);
		node.d.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			a, b, c, d, logPrior, x: REAL;
	BEGIN
		a := node.a.Value();
		b := node.b.Value();
		c := node.c.Value();
		d := node.d.Value();
		x := node.value;
		IF x < a THEN
			logPrior := MathFunc.logOfZero
		ELSIF x > d THEN
			logPrior := MathFunc.logOfZero
		ELSIF x < b THEN
			logPrior := Math.Ln((x - a) / (b - a))
		ELSIF x > c THEN
			logPrior := Math.Ln((d - x) / (d - c))
		ELSIF x > b THEN
			logPrior := 0
		END;
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			a, d, logDensity, rand, x: REAL;
	BEGIN
		a := node.a.Value();
		d := node.d.Value();
		REPEAT
			x := MathRandnum.Uniform(a, d);
			node.SetValue(x);
			logDensity := node.LogPrior();
			rand := MathRandnum.Rand()
		UNTIL Math.Ln(rand) < logDensity;
		res := {}
	END Sample;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.a := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			node.b := args.scalars[1];
			ASSERT(args.scalars[2] # NIL, 21);
			node.c := args.scalars[2];
			ASSERT(args.scalars[3] # NIL, 21);
			node.d := args.scalars[3]
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
		signature := "ssss"
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
END GraphTrapezium.

