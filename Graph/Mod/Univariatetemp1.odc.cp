(*			

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

(*
This template module has been set up to implement the normal distribution but not making use of
conjugate properties. Code specific to this example is in red.
*)

MODULE GraphUnivariatetemp1;


	

	IMPORT
		Math, Stores,
		GraphNodes, GraphParamtrans, GraphStochastic, GraphUnivariate, GraphUnivariateT,
		MathFunc, MathRandnum;

	TYPE
		Node = POINTER TO RECORD(GraphUnivariateT.Node) END;

		Factory = POINTER TO RECORD (GraphUnivariate.Factory) END;

	VAR
		fact-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		log2Pi: REAL;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
		VAR
			res: SET;
	BEGIN
		res := {};
		(*
		put in any code to check that node has sensible parameters, value etc
		*)
		RETURN res
	END CheckUnivariate;

	PROCEDURE (node: Node) Cumulative (x: REAL): REAL;
		VAR
			cumulative, mu, tau: REAL;
	BEGIN
		(*
		Calculate the cumulative distribution function

		This is compulsory if the distribution can be censored and the cumulative exists in closed
		form. If either of these conditions is false this method will not be called and can be given a
		dummy implementation containg a HALT(0) statement eg

		HALT(0);
		RETURN 0.0

		*)
		mu := node.scalars[0].Value();
		tau := node.scalars[1].Value();
		cumulative := MathFunc.Phi(Math.Sqrt(tau) * (x - mu));
		RETURN cumulative
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		VAR
			logTau, logDensity, mu, tau, x: REAL;
	BEGIN
		(*
		Calculate the deviance

		This is compulsory
		*)
		x := node.value;
		mu := node.scalars[0].Value();
		tau := node.scalars[1].Value();
		logTau := MathFunc.Ln(tau);
		logDensity := 0.5 * logTau - 0.5 * tau * (x - mu) * (x - mu) - 0.5 * log2Pi;
		RETURN - 2.0 * logDensity
	END DevianceUnivariate;

	PROCEDURE (node: Node) DiffLogLikelihood (x: GraphStochastic.Node): REAL;
		VAR
			differential, diffMu, diffTau, mu, tau, val: REAL;
	BEGIN
		(*
		Calculate the derivative of the likelihood

		This is compulsory
		*)
		val := node.value;
		node.scalars[0].ValDiff(x, mu, diffMu);
		node.scalars[1].ValDiff(x, tau, diffTau);
		differential :=  - diffMu * tau * (val - mu) + 0.5 * diffTau * (1 / tau - (val - mu) * (val - mu));
		RETURN differential
	END DiffLogLikelihood;

	PROCEDURE (node: Node) DiffLogPrior (): REAL;
		VAR
			differential, mu, tau, x: REAL;
	BEGIN
		(*
		Calculate the derivative of the prior

		This is compulsory
		*)
		x := node.value;
		mu := node.scalars[0].Value();
		tau := node.scalars[1].Value();
		differential := tau * (x - mu);
		RETURN differential
	END DiffLogPrior;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphUnivariatetemp1.Install"
	END Install;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		VAR
			logTau, mu, tau, x: REAL;
	BEGIN
		(*
		Calculate the log likelihood. Any purely numerical normalizing constants can be droped.

		This is compulsory
		*)
		x := node.value;
		mu := node.scalars[0].Value();
		tau := node.scalars[1].Value();
		logTau := MathFunc.Ln(tau);
		RETURN 0.50 * (logTau - tau * (x - mu) * (x - mu))
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			mu, tau, x: REAL;
	BEGIN
		(*
		Calculates the log prior. Any terms not involving the value of the node can be droped

		This is compulsory
		*)
		x := node.value;
		mu := node.scalars[0].Value();
		tau := node.scalars[1].Value();
		RETURN - 0.50 * tau * (x - mu) * (x - mu)
	END LogPrior;

	PROCEDURE (node: Node) Location (): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END Location;

	PROCEDURE (node: Node) BoundsUnivariate (OUT lower, upper: REAL);
	BEGIN
		(*
		Returns the natural bound of distribution (ie does not take into account possible censoring)

		This is compulsory
		*)
		lower :=  - INF;
		upper := INF
	END BoundsUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			mu, tau, x, lower, upper: REAL;
	BEGIN
		(*
		Generates a deviate from the distibution.

		This is compulsory
		*)
		res := {};
		mu := node.scalars[0].Value();
		tau := node.scalars[1].Value();
		node.Bounds(lower, upper);
		REPEAT
			x := MathRandnum.Normal(mu, tau)
		UNTIL (x >= lower) & (x <= upper);
		node.SetValue(x)
	END Sample;

	PROCEDURE (node: Node) ModifyUnivariate (): GraphUnivariate.Node;
		VAR
			p: Node;
	BEGIN
		NEW(p);
		p^ := node^;
		(*	make any parameter transformatiuons required	*)
		p.scalars[0] := GraphParamtrans.IdentTransform(p.scalars[0]);
		p.scalars[1] := GraphParamtrans.LogTransform(p.scalars[1]);
		RETURN p
	END ModifyUnivariate;

	PROCEDURE (f: Factory) New (): GraphUnivariate.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		(*
		If the cumulative distibution function exists in closed form add GraphStochastic.closedForm to
		node's properties.

		IF the distribution has integer support add GraphStochastic.integer to node's properties
		*)
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "ssCT"
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		(*
		Set version number and maintainer
		eg
		version := 500;
		maintainer := "Fred Fish"
		This is compulsory
		*)
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		log2Pi := Math.Ln(2 * Math.Pi());
		NEW(f);
		fact := f
	END Init;

BEGIN
	Init
END GraphUnivariatetemp1.

