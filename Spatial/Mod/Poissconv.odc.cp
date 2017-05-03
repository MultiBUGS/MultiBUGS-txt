(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE SpatialPoissconv;


	

	IMPORT
		Math, Stores,
		GraphNodes, GraphParamtrans, GraphPoisson, GraphRules, GraphStochastic,
		GraphSumation, GraphUnivariate, 
		MathCumulative, MathFunc, MathRandnum,
		UpdaterActions, UpdaterAuxillary, UpdaterUpdaters;

	TYPE
		Node = POINTER TO RECORD(GraphUnivariate.Node)
			lambda: GraphNodes.Vector;
			poissons: GraphStochastic.Vector;
			sumLambda: GraphNodes.Node;
			lambdaStart, lambdaStep: INTEGER
		END;

		Multinomial = POINTER TO RECORD(UpdaterAuxillary.UpdaterMV) END;

		Factory = POINTER TO RECORD(GraphUnivariate.Factory) END;

		MultinomialFactory = POINTER TO RECORD(UpdaterUpdaters.Factory) END;

	CONST
		eps = 1.0E-6;

	VAR
		fact-: GraphUnivariate.Factory;
		multinomialFact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		prop: POINTER TO ARRAY OF REAL;
		values: POINTER TO ARRAY OF INTEGER;

	PROCEDURE (updater: Multinomial) Clone (): Multinomial;
		VAR
			u: Multinomial;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Multinomial) CopyFromAuxillary (source: UpdaterUpdaters.Updater);
	BEGIN
	END CopyFromAuxillary;

	PROCEDURE (updater: Multinomial) ExternalizeAuxillary (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeAuxillary;

	PROCEDURE (updater: Multinomial) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "SpatialPoissconv.MultinomialInstall"
	END Install;

	PROCEDURE (updater: Multinomial) InternalizeAuxillary (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeAuxillary;

	PROCEDURE (multinomial: Multinomial) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			i, start, n, nElem, step: INTEGER;
			sum: REAL;
			node: Node;
	BEGIN
		res := {};
		node := multinomial.node(Node);
		nElem := LEN(node.poissons);
		IF nElem > LEN(prop) THEN
			NEW(prop, nElem); NEW(values, nElem)
		END;
		i := 0;
		start := node.lambdaStart;
		step := node.lambdaStep;
		sum := 0.0;
		WHILE i < nElem DO
			prop[i] := node.lambda[start + i * step].Value();
			sum := sum + prop[i];
			INC(i)
		END;
		i := 0;
		WHILE i < nElem DO
			prop[i] := prop[i] / sum;
			INC(i)
		END;
		n := SHORT(ENTIER(node.value + eps));
		MathRandnum.Multinomial(prop, n, nElem, values);
		i := 0;
		WHILE i < nElem DO
			node.poissons[i].SetValue(values[i]);
			INC(i)
		END
	END Sample;
	
	PROCEDURE (multinomial: Multinomial) Size (): INTEGER;
		VAR
			node: Node;
	BEGIN
		node := multinomial.node(Node);
		RETURN LEN(node.poissons)
	END Size;
	
	PROCEDURE (multinomial: Multinomial) UpdatedBy (index: INTEGER): GraphStochastic.Node;
		VAR
			node: Node;
	BEGIN
		node := multinomial.node(Node);
		RETURN node.poissons[index]
	END UpdatedBy;

	PROCEDURE (f: MultinomialFactory) GetDefaults;
	BEGIN
	END GetDefaults;

	PROCEDURE (f: MultinomialFactory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "SpatialPoissconv.MultinomialInstall"
	END Install;

	PROCEDURE (f: MultinomialFactory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: MultinomialFactory) Create (): UpdaterUpdaters.Updater;
		VAR
			multinomial: Multinomial;
	BEGIN
		NEW(multinomial);
		RETURN multinomial
	END Create;

	PROCEDURE (node: Node) BoundsUnivariate (OUT lower, upper: REAL);
	BEGIN
		lower := 0;
		upper := MAX(INTEGER)
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
		CONST
			eps = 1.0E-5;
		VAR
			x: INTEGER;
			lambda: REAL;
	BEGIN
		x := SHORT(ENTIER(node.value + eps));
		IF ABS(x - node.value) > eps THEN
			RETURN {GraphNodes.lhs, GraphNodes.integer}
		END;
		IF x <  - eps THEN
			RETURN {GraphNodes.lhs, GraphNodes.invalidInteger}
		END;
		lambda := node.sumLambda.Value();
		IF lambda <  - eps THEN
			RETURN {GraphNodes.arg1, GraphNodes.invalidValue}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
	BEGIN
		(*	this should never get called	*)
		RETURN GraphRules.general
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		(*	this should never get called	*)
		RETURN GraphRules.general
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (x: REAL): REAL;
		VAR
			cumulative, lambda: REAL;
	BEGIN
		lambda := node.sumLambda.Value();
		cumulative := MathCumulative.Poisson(lambda, x);
		RETURN cumulative
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		CONST
			eps = 1.0E-20;
		VAR
			lambda, logDensity, logLambda, x: REAL;
	BEGIN
		x := node.value;
		lambda := node.sumLambda.Value();
		logLambda := Math.Ln(lambda);
		logDensity := x * logLambda - lambda - MathFunc.LogFactorial(SHORT(ENTIER(x + eps)));
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
		VAR
			i, len: INTEGER;
			v: GraphNodes.SubVector;
	BEGIN
		len := LEN(node.poissons);
		v := GraphNodes.NewVector();
		v.components := node.lambda;
		v.start := node.lambdaStart; v.nElem := len; v.step := node.lambdaStep;
		GraphNodes.ExternalizeSubvector(v, wr);
		i := 0; WHILE i < len DO GraphNodes.Externalize(node.poissons[i], wr); INC(i) END
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.lambda := NIL;
		node.poissons := NIL;
		node.lambdaStart :=  - 1;
		node.lambdaStep := 0;
		node.sumLambda := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
		VAR
			i, len: INTEGER;
			v: GraphNodes.SubVector;
			q: GraphNodes.Node;
			argsSum: GraphStochastic.ArgsLogical;
			res: SET;
	BEGIN
		GraphNodes.InternalizeSubvector(v, rd);
		node.lambda := v.components;
		node.lambdaStart := v.start;
		node.lambdaStep := v.step;
		len := v.nElem;
		NEW(node.poissons, len);
		i := 0;
		WHILE i < len DO
			q := GraphNodes.Internalize(rd);
			node.poissons[i] := q(GraphStochastic.Node);
			INC(i)
		END;
		node.sumLambda := GraphSumation.sumFact.New();
		argsSum.vectors[0] := v;
		node.sumLambda.Set(argsSum, res)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "SpatialPoissconv.Install"
	END Install;

	PROCEDURE (node: Node) Location (): REAL;
		VAR
			lambda: REAL;
	BEGIN
		lambda := node.sumLambda.Value();
		RETURN lambda
	END Location;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		CONST
			eps = 1.0E-40;
		VAR
			lambda, logLambda, logLikelihood, x: REAL;
	BEGIN
		x := node.value;
		lambda := node.sumLambda.Value();
		IF x > 0.5 THEN
			logLambda := Math.Ln(lambda);
			logLikelihood := x * logLambda - lambda
		ELSE
			logLikelihood :=  - lambda
		END;
		IF ~(GraphNodes.data IN node.props) THEN
			logLikelihood := logLikelihood - MathFunc.LogFactorial(SHORT(ENTIER(x + eps)))
		END;
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		CONST
			eps = 1.0E-20;
		VAR
			lambda, logLambda, logPrior, x: REAL;
	BEGIN
		x := node.value;
		logPrior := MathFunc.LogFactorial(SHORT(ENTIER(x + eps)));
		IF x > 0.5 THEN
			lambda := node.sumLambda.Value();
			logLambda := Math.Ln(lambda);
			logPrior := logPrior + x * logLambda
		END;
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			p: GraphNodes.Node;
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		IF all THEN
			p := node.sumLambda;
			p.AddParent(list)
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			lambda, value: REAL;
	BEGIN
		lambda := node.sumLambda.Value();
		value := MathRandnum.Poisson(lambda);
		node.SetValue(value);
		res := {}
	END Sample;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			i, nElem, start, step: INTEGER;
			p: GraphStochastic.Node;
			argsPoiss: GraphStochastic.Args;
			argsSum: GraphStochastic.ArgsLogical;
			multinomial: UpdaterUpdaters.Updater;
			zero: BOOLEAN;
	BEGIN
		res := {};
		IF ~(GraphNodes.data IN node.props) THEN
			res := {GraphNodes.lhs, GraphNodes.notData}; RETURN
		END;
		zero := node.value < 0.5;
		WITH args: GraphStochastic.Args DO
			ASSERT(args.vectors[0].components # NIL, 21);
			node.lambda := args.vectors[0].components;
			start := args.vectors[0].start;
			step := args.vectors[0].step;
			nElem := args.vectors[0].nElem;
			node.lambdaStart := start;
			node.lambdaStep := step;
			IF node.poissons = NIL THEN
				argsPoiss.Init;
				NEW(node.poissons, nElem);
				i := 0;
				WHILE i < nElem DO
					p := GraphPoisson.fact.New();
					p.Init;
					argsPoiss.scalars[0] := node.lambda[start + i * step];
					p.Set(argsPoiss, res);
					p.SetProps(p.props + {GraphNodes.data, GraphNodes.hidden});
					p.BuildLikelihood;
					IF ~zero THEN p.SetProps(p.props - {GraphNodes.data}) END;
					p.SetValue(0.0);
					node.poissons[i] := p;
					INC(i)
				END
			END;
			node.sumLambda := GraphSumation.sumFact.New();
			argsSum.vectors[0] := args.vectors[0];
			node.sumLambda.Set(argsSum, res);
			IF ~zero THEN
				multinomial := multinomialFact.New(node);
				UpdaterActions.RegisterUpdater(multinomial)
			END
		END
	END SetUnivariate;

	PROCEDURE (node: Node) ModifyUnivariate (): GraphUnivariate.Node;
		VAR
			p: Node;
	BEGIN
		NEW(p);
		p^ := node^;
		p.sumLambda := GraphParamtrans.LogTransform(p.sumLambda);
		p.poissons := NIL;
		p.lambda := NIL;
		RETURN p
	END ModifyUnivariate;

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
		signature := "v"
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE MultinomialInstall*;
	BEGIN
		UpdaterUpdaters.SetFactory(multinomialFact)
	END MultinomialInstall;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		CONST
			blockSize = 10;
		VAR
			f: Factory;
			fMultinomial: MultinomialFactory;
	BEGIN
		Maintainer;
		NEW(prop, 2);
		NEW(f);
		fact := f;
		NEW(fMultinomial);
		multinomialFact := fMultinomial;
		NEW(prop, blockSize);
		NEW(values, blockSize)
	END Init;

BEGIN
	Init
END SpatialPoissconv.

