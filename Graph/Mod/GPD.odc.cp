(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"





*)

MODULE GraphGPD;


	

	IMPORT
		Math, Stores,
		GraphNodes, GraphParamtrans, GraphRules, GraphStochastic,
		GraphUnivariate,
		MathCumulative, MathFunc, MathRandnum;

	TYPE

		Node = POINTER TO RECORD(GraphUnivariate.Node)
			eta, mu, sigma: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD (GraphUnivariate.Factory) END;

	CONST
		eps = 1.0E-10;

	VAR
		fact-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
		VAR
			f0, f1, f2: INTEGER;
	BEGIN
		f0 := GraphStochastic.ClassFunction(node.mu, parent);
		f1 := GraphStochastic.ClassFunction(node.sigma, parent);
		f2 := GraphStochastic.ClassFunction(node.eta, parent);
		IF (f0 = GraphRules.const) & (f1 = GraphRules.const) & (f2 = GraphRules.const) THEN
			RETURN GraphRules.unif
		ELSIF (f0 # GraphRules.other) & (f1 # GraphRules.other) & (f2 # GraphRules.other) THEN
			RETURN GraphRules.genDiff
		ELSE
			RETURN GraphRules.general
		END
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.general
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (x: REAL): REAL;
		VAR
			eta, mu, sigma: REAL;
	BEGIN
		mu := node.mu.Value();
		sigma := node.sigma.Value();
		eta := node.eta.Value();
		RETURN MathCumulative.GPD(mu, sigma, eta, x)
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			eta, etaInv, factor, logLikelihood, mu, sigma, z, x: REAL;
		CONST
			eps = 1.0E-20;
	BEGIN
		mu := node.mu.Value();
		sigma := node.sigma.Value();
		eta := node.eta.Value();
		etaInv := 1 / eta;
		x := node.value;
		z := (x - mu) / sigma;
		factor := 1.0 + eta * z;
		IF (factor < 0) OR (x < mu) THEN
			logLikelihood := MathFunc.logOfZero
		ELSIF ABS(eta) > eps THEN
			logLikelihood :=  - Math.Ln(sigma) - (1 + etaInv) * Math.Ln(factor)
		ELSE
			logLikelihood :=  - Math.Ln(sigma) - z
		END;
		RETURN - 2 * logLikelihood
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
	BEGIN
		HALT(126);
		RETURN 0.0
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			eta, etaInv, factor, differential, mu, sigma, z, diffFactor, x: REAL;
		CONST
			eps = 1.0E-20;
	BEGIN
		mu := node.mu.Value();
		sigma := node.sigma.Value();
		eta := node.eta.Value();
		etaInv := 1 / eta;
		x := node.value;
		z := (x - mu) / sigma;
		diffFactor := eta / sigma;
		factor := 1.0 + eta * z;
		IF (factor < 0) OR (x < mu) THEN
			differential := 0
		ELSIF ABS(eta) > eps THEN
			differential :=  - (1 + etaInv) * diffFactor / factor
		ELSE
			differential :=  - 1 / sigma
		END;
		RETURN differential
	END DiffLogPrior;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphGPD.Install"
	END Install;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			eta, etaInv, factor, logLikelihood, mu, sigma, z, x: REAL;
		CONST
			eps = 1.0E-20;
	BEGIN
		mu := node.mu.Value();
		sigma := node.sigma.Value();
		eta := node.eta.Value();
		etaInv := 1 / eta;
		x := node.value;
		z := (x - mu) / sigma;
		factor := 1.0 + eta * z;
		IF (factor < 0) OR (x < mu) THEN
			logLikelihood := MathFunc.logOfZero
		ELSIF ABS(eta) > eps THEN
			logLikelihood :=  - Math.Ln(sigma) - (1 + etaInv) * Math.Ln(factor)
		ELSE
			logLikelihood :=  - Math.Ln(sigma) - z
		END;
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
	BEGIN
		RETURN node.LogLikelihood()
	END LogPrior;

	PROCEDURE (node: Node) Location (): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END Location;

	PROCEDURE (node: Node) BoundsUnivariate (OUT left, right: REAL);
		VAR
			eta, mu, sigma: REAL;
		CONST
			eps = 1.0E-20;
	BEGIN
		mu := node.mu.Value();
		sigma := node.sigma.Value();
		eta := node.eta.Value();
		IF eta > eps THEN
			right := INF;
			left := mu - (sigma / eta)
		ELSIF eta <  - eps THEN
			left :=  - INF;
			right := mu - (sigma / eta)
		ELSE
			left :=  - INF;
			right := INF
		END
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
		VAR
			eta, factor, mu, sigma, x: REAL;
	BEGIN
		IF ~(GraphNodes.data IN node.mu.props) THEN
			RETURN {GraphNodes.notData, GraphNodes.arg1}
		END;
		mu := node.mu.Value();
		sigma := node.sigma.Value();
		eta := node.eta.Value();
		x := node.value;
		IF sigma <  - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.arg2}
		END;
		factor := 1.0 + eta * (x - mu) / sigma;
		IF factor < 0 THEN
			RETURN {GraphNodes.invalidPosative, GraphNodes.lhs}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ExternalizeUnivariate (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(node.eta, wr);
		GraphNodes.Externalize(node.mu, wr);
		GraphNodes.Externalize(node.sigma, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
	BEGIN
		node.eta := GraphNodes.Internalize(rd);
		node.mu := GraphNodes.Internalize(rd);
		node.sigma := GraphNodes.Internalize(rd)
	END InternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.mu := NIL;
		node.sigma := NIL;
		node.eta := NIL
	END InitUnivariate;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list: GraphNodes.List;
	BEGIN
		list := NIL;
		node.mu.AddParent(list);
		node.sigma.AddParent(list);
		node.eta.AddParent(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) ModifyUnivariate (): GraphUnivariate.Node;
		VAR
			p: Node;
	BEGIN
		NEW(p);
		p^ := node^;
		p.mu := GraphParamtrans.IdentTransform(p.mu);
		p.sigma := GraphParamtrans.LogTransform(p.sigma);
		p.eta := GraphParamtrans.LogTransform(p.eta);
		RETURN p
	END ModifyUnivariate;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.scalars[0] # NIL, 21);
			node.mu := args.scalars[0];
			ASSERT(args.scalars[1] # NIL, 21);
			node.sigma := args.scalars[1];
			ASSERT(args.scalars[2] # NIL, 21);
			node.eta := args.scalars[2]
		END
	END SetUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			eta, mu, sigma, x, lower, upper: REAL;
			bounds: SET;
	BEGIN
		mu := node.mu.Value();
		sigma := node.sigma.Value();
		eta := node.eta.Value();
		bounds := node.props * {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
		IF bounds = {} THEN
			x := MathRandnum.GPD(mu, sigma, eta);
		ELSE
			node.Bounds(lower, upper);
			IF bounds = {GraphStochastic.leftImposed} THEN
				x := MathRandnum.GPDLB(mu, sigma, eta, lower)
			ELSIF bounds = {GraphStochastic.rightImposed} THEN
				x := MathRandnum.GPDRB(mu, sigma, eta, upper)
			ELSE
				x := MathRandnum.GPDIB(mu, sigma, eta, lower, upper)
			END
		END;
		node.SetValue(x);
		res := {}
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
		signature := "sssCT"
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "T.Jagger"
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
END GraphGPD.

