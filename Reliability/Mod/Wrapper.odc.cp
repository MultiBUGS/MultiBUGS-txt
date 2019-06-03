(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"


*)

MODULE ReliabilityWrapper;


	

	IMPORT
		Math, Stores := Stores64, Strings,
		GraphGrammar, GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE
		Node = POINTER TO RECORD(GraphUnivariate.Node)
			inner, outer: GraphUnivariate.Node;
			numInnerParams, numOuterParams: INTEGER
		END;

		Factory = POINTER TO RECORD(GraphUnivariate.Factory) 
							innerFactory, outerFactory: GraphUnivariate.Factory
						END;

	VAR
		fact: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
	BEGIN
		RETURN GraphRules.general
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.general
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (x: REAL): REAL;
		VAR
			culm, relia, xTran: REAL;
	BEGIN
		culm := node.inner.Cumulative(x);
		relia := 1.0 - culm;
		xTran := culm / relia;
		culm := node.outer.Cumulative(xTran);
		RETURN culm
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
	BEGIN
		RETURN -2.0 * node.LogLikelihood()
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
	BEGIN
		RETURN 0
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
	BEGIN
		RETURN 0
	END DiffLogPrior;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
		VAR
			string: ARRAY 128 OF CHAR;
			descriptor: GraphGrammar.External;
	BEGIN
		node.outer.Install(string);
		descriptor := GraphGrammar.FindInstalled(string);
		install := "ReliabilityWrapper.Install(" + descriptor.name + "+";
		node.inner.Install(string);
		descriptor := GraphGrammar.FindInstalled(string);
		install := install + descriptor.name + ")";
	END Install;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			x, xTran, culm, relia, logLike: REAL;
			inner, outer: GraphUnivariate.Node;
	BEGIN
		x := node.value;
		inner := node.inner;
		outer := node.outer;
		culm := inner.Cumulative(x);
		relia := 1.0 - culm;
		xTran := culm / relia;
		outer.SetValue(xTran);
		inner.SetValue(x);
		logLike := outer.LogLikelihood() + inner.LogLikelihood() - 2.0 * Math.Ln(relia);
		RETURN logLike
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
	BEGIN
		RETURN node.LogLikelihood()
	END LogPrior;

	PROCEDURE (node: Node) Location (): REAL;
	BEGIN
		RETURN 0
	END Location;

	PROCEDURE (node: Node) BoundsUnivariate (OUT left, right: REAL);
	BEGIN
		left := 0.0;
		right := INF
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
	BEGIN
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.outer, wr);
		GraphNodes.Externalize(node.inner, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
		VAR
			p: GraphNodes.Node;
	BEGIN
		p := GraphNodes.Internalize(rd);
		node.outer := p(GraphUnivariate.Node);
		p := GraphNodes.Internalize(rd);
		node.inner := p(GraphUnivariate.Node)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.inner := NIL;
		node.outer := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			cursor, list: GraphNodes.List;
			p: GraphNodes.Node;
	BEGIN
		list := node.outer.Parents(all);
		cursor := list;
		WHILE cursor # NIL DO
			p := cursor.node;
			p.SetProps(p.props + {GraphNodes.mark});
			cursor := cursor.next
		END;
		cursor := node.inner.Parents(all);
		WHILE cursor # NIL DO
			p := cursor.node;
			p.AddParent(list);
			cursor := cursor.next
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			i, numParamInner, numParamOuter: INTEGER;
			argsS: GraphStochastic.Args;
			inner, outer: GraphUnivariate.Node;
	BEGIN
		inner := node.inner;
		outer := node.outer;
		numParamOuter := node.numOuterParams;
		numParamInner := node.numInnerParams;
		WITH args: GraphStochastic.Args DO
			argsS.Init;
			argsS := args;
			argsS.numScalars := numParamOuter;
			outer.Set(argsS, res); 
			argsS.Init;
			argsS.numScalars := numParamInner;
			i := 0;
			WHILE i < numParamInner DO
				argsS.scalars[i] := args.scalars[numParamOuter + i];
				INC(i)
			END;
			inner.Set(argsS, res)
		END
	END SetUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
	BEGIN
	END Sample;

	PROCEDURE (f: Factory) NewInner (): GraphUnivariate.Node, NEW;
		VAR
			univariate: GraphUnivariate.Node;
	BEGIN
		univariate := f.innerFactory.New();
		univariate.Init;
		univariate.SetProps(univariate.props + {GraphStochastic.hidden});
		RETURN univariate
	END NewInner;

	PROCEDURE (f: Factory) NewOuter (): GraphUnivariate.Node, NEW;
		VAR
			univariate: GraphUnivariate.Node;
	BEGIN
		univariate := f.outerFactory.New();
		univariate.Init;
		univariate.SetProps(univariate.props + {GraphStochastic.hidden});
		RETURN univariate
	END NewOuter;

	PROCEDURE (f: Factory) New (): GraphUnivariate.Node;
			VAR
				node: Node;
	BEGIN
		NEW(node);
		node.Init;
		node.inner := f.NewInner();
		node.outer := f.NewOuter();
		node.numInnerParams := f.innerFactory.NumParam();
		node.numOuterParams := f.outerFactory.NumParam();
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
		VAR
			sig: ARRAY 10 OF CHAR;
			i: INTEGER;
	BEGIN
		f.outerFactory.Signature(sig);
		i := 0;
		WHILE i < LEN(sig$) DO
			IF (sig[i] = "C") OR (sig[i] = "T") THEN
				sig[i] := 0X
			END;
			INC(i)
		END;
		signature := sig$;
		f.innerFactory.Signature(sig);
		signature := signature + sig
	END Signature;

	PROCEDURE UnivariateFactory (density: ARRAY OF CHAR): GraphUnivariate.Factory;
		VAR
			external: GraphGrammar.External;
			factory: GraphStochastic.Factory;
	BEGIN
		factory := NIL;
		external := GraphGrammar.FindDensity(density);
		factory := external.fact(GraphStochastic.Factory);
		RETURN factory(GraphUnivariate.Factory)
	END UnivariateFactory;

	PROCEDURE Install*  (string: ARRAY OF CHAR);
		VAR
			f: Factory;
			innerFactory, outerFactory: GraphUnivariate.Factory;
			density: ARRAY 128 OF CHAR;
			pos: INTEGER;
	BEGIN
		Strings.Find(string, "+", 0, pos);;
		Strings.Extract(string, 0, pos, density);
		outerFactory := UnivariateFactory(density);
		Strings.Extract(string, pos + 1, LEN(string), density);
		innerFactory := UnivariateFactory(density);
		NEW(f);
		f.outerFactory := outerFactory;
		f.innerFactory := innerFactory;
		GraphNodes.SetFactory(f)	
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas & V. Kumar"
	END Maintainer;

BEGIN
	Maintainer
END ReliabilityWrapper.

