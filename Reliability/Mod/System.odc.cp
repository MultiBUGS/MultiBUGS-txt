(*	



license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

(*	Usage : dSystem(components[])

Each component of the system must have a closed form of the cumulative distribution function

Reliability (survival) of system is product of reliability (survival) of each component of the system

*)

MODULE ReliabilitySystem;


	

	IMPORT
		Math, Stores,
		GraphNodes, GraphRules, GraphStochastic, GraphUnivariate,
		MathFunc, MathRandnum;

	TYPE

		Node = POINTER TO RECORD(GraphUnivariate.Node)
			components: GraphNodes.Vector;
			nElem, start, step: INTEGER
		END;

		Factory = POINTER TO RECORD (GraphUnivariate.Factory) END;

	CONST
		maxIts = 100000;
		eps = 1.0E-10;

	VAR
		fact-: GraphUnivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		log2Pi: REAL;

	PROCEDURE (node: Node) BoundsUnivariate (OUT left, right: REAL);
	BEGIN
		left := 0.0;
		right := INF
	END BoundsUnivariate;

	PROCEDURE (node: Node) CheckUnivariate (): SET;
	BEGIN
		IF node.value < - eps THEN
			RETURN {GraphNodes.posative, GraphNodes.lhs}
		END;
		RETURN {}
	END CheckUnivariate;

	PROCEDURE (node: Node) ClassifyLikelihoodUnivariate (parent: GraphStochastic.Node): INTEGER;
		VAR
			class, i, nElem, start, step: INTEGER;
			stochastic: GraphStochastic.Node;
	BEGIN
		i := 0;
		nElem := node.nElem;
		start := node.start;
		step := node.step;
		WHILE i < nElem DO
			stochastic := node.components[start + i * step](GraphStochastic.Node);
			class := stochastic.ClassifyLikelihood(parent);
			IF class # GraphRules.const THEN RETURN GraphRules.general END
		END;
		RETURN GraphRules.const
	END ClassifyLikelihoodUnivariate;

	PROCEDURE (node: Node) ClassifyPrior (): INTEGER;
	BEGIN
		RETURN GraphRules.general
	END ClassifyPrior;

	PROCEDURE (node: Node) Cumulative (x: REAL): REAL;
		VAR
			i, nElem, start, step: INTEGER;
			reliability: REAL;
			stochastic: GraphUnivariate.Node;
	BEGIN
		i := 0;
		nElem := node.nElem;
		start := node.start;
		step := node.step;
		reliability := 1.0;
		WHILE i < nElem DO
			stochastic := node.components[start + i * step](GraphUnivariate.Node);
			reliability := reliability * stochastic.Cumulative(x);
		END;
		RETURN 1.0 - reliability
	END Cumulative;

	PROCEDURE (node: Node) DevianceUnivariate (): REAL;
		CONST
			eps = 1.0E-10;
		VAR
			density, deviance, logLikelihood, logReliability, reliability, temp, x: REAL;
			stochastic: GraphUnivariate.Node;
			i, nElem, start, step: INTEGER;
			oldProps: SET;
	BEGIN
		i := 0;
		logReliability := 0.0;
		density := 0.0;
		x := node.value;
		nElem := node.nElem;
		start := node.start;
		step := node.step;
		WHILE i < nElem DO
			stochastic := node.components[start + i * step](GraphUnivariate.Node);
			reliability := 1.0 - stochastic.Cumulative(x);
			IF reliability < eps THEN RETURN MathFunc.logOfZero END;
			oldProps := stochastic.props;
			(*	add data to nodes props so that Deviance() woks correctly	*)
			stochastic.SetProps(oldProps + {GraphNodes.data});
			temp := stochastic.value;
			logReliability := logReliability + Math.Ln(reliability);
			stochastic.SetValue(x);
			deviance := stochastic.Deviance();
			density := density + (Math.Exp( - 0.5 * deviance) / reliability);
			stochastic.SetValue(temp);
			stochastic.SetProps(oldProps);
			INC(i)
		END;
		logLikelihood := Math.Ln(density) + logReliability;
		RETURN - 2 * logLikelihood
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
			v: GraphNodes.SubVector;
	BEGIN
		v := GraphNodes.NewVector();
		v.components := node.components;
		v.nElem := node.nElem;
		v.start := node.start;
		v.step := node.step;
		GraphNodes.ExternalizeSubvector(v, wr)
	END ExternalizeUnivariate;

	PROCEDURE (node: Node) InitUnivariate;
	BEGIN
		node.SetProps(node.props + {GraphStochastic.leftNatural});
		node.components := NIL;
		node.nElem := 0;
		node.start := - 1;
		node.step := - 1
	END InitUnivariate;

	PROCEDURE (node: Node) InternalizeUnivariate (VAR rd: Stores.Reader);
		VAR
			v: GraphNodes.SubVector;
	BEGIN
		GraphNodes.InternalizeSubvector(v, rd);
		node.components := v.components;
		node.nElem := v.nElem;
		node.start := v.start;
		node.step := v.step
	END InternalizeUnivariate;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "ReliabilitySystem.Install"
	END Install;

	PROCEDURE (node: Node) Location (): REAL;
	BEGIN
		HALT(0);
		RETURN 0.0
	END Location;

	PROCEDURE (node: Node) LogLikelihoodUnivariate (): REAL;
		CONST
			eps = 1.0E-10;
		VAR
			density, deviance, logLikelihood, logReliability, reliability, temp, x: REAL;
			stochastic: GraphUnivariate.Node;
			i, nElem, start, step: INTEGER;
			oldProps: SET;
	BEGIN
		i := 0;
		logReliability := 0.0;
		density := 0.0;
		x := node.value;
		nElem := node.nElem;
		start := node.start;
		step := node.step;
		WHILE i < nElem DO
			stochastic := node.components[start + i * step](GraphUnivariate.Node);
			reliability := 1.0 - stochastic.Cumulative(x);
			IF reliability < eps THEN RETURN MathFunc.logOfZero END;
			oldProps := stochastic.props;
			(*	add data to nodes props so that Deviance() works correctly	*)
			stochastic.SetProps(oldProps + {GraphNodes.data});
			temp := stochastic.value;
			logReliability := logReliability + Math.Ln(reliability);
			stochastic.SetValue(x);
			deviance := stochastic.Deviance();
			density := density + (Math.Exp( - 0.5 * deviance) / reliability);
			stochastic.SetValue(temp);
			stochastic.SetProps(oldProps);
			INC(i)
		END;
		logLikelihood := Math.Ln(density) + logReliability;
		RETURN logLikelihood
	END LogLikelihoodUnivariate;

	PROCEDURE (node: Node) LogPrior (): REAL;
		VAR
			logPrior, x: REAL;
	BEGIN
		logPrior := node.LogLikelihood();
		RETURN logPrior
	END LogPrior;

	PROCEDURE (node: Node) ParentsUnivariate (all: BOOLEAN): GraphNodes.List;
		VAR
			list, list0: GraphNodes.List;
			i, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		list := NIL;
		i := 0;
		nElem := node.nElem;
		start := node.start;
		step := node.step;
		WHILE i < nElem DO
			list0 := node.components[start + i * step].Parents(all);
			WHILE list0 # NIL DO
				p := list0.node;
				IF ~(GraphStochastic.mark IN p.props) THEN
					p.SetProps(p.props + {GraphStochastic.mark});
					p.AddParent(list);
				END;
				list0 := list0.next
			END;
			INC(i)
		END;
		list0 := list;
		WHILE list0 # NIL DO
			p := list0.node;
			p.SetProps(p.props - {GraphStochastic.mark});
			list0 := list0.next;
		END;
		GraphNodes.ClearList(list);
		RETURN list
	END ParentsUnivariate;

	PROCEDURE (node: Node) Sample (OUT res: SET);
		VAR
			i, step, start, nElem: INTEGER;
			left, right, x, temp: REAL;
			stochastic: GraphStochastic.Node;
	BEGIN
		node.Bounds(left, right);
		i := maxIts;
		REPEAT
			i := 0;
			nElem := node.nElem;
			start := node.start;
			step := node.step;
			res := {};
			x := MAX(REAL);
			WHILE (i < nElem) & (res = {}) DO
				stochastic := node.components[start + i * step](GraphStochastic.Node);
				temp := stochastic.value;
				stochastic.Sample(res);
				IF res = {} THEN x := MIN(x, stochastic.value) END;
				stochastic.SetValue(temp);
				INC(i)
			END;
			node.SetValue(x);
			DEC(i)
		UNTIL ((node.value > left) & (node.value < right)) OR (i = 0) OR (res # {});
		IF i = 0 THEN
			res := {GraphNodes.lhs, GraphNodes.tooManyIts}
		END
	END Sample;

	PROCEDURE (node: Node) SetUnivariate (IN args: GraphNodes.Args; OUT res: SET);
		VAR
			i, nElem, start, step: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		res := {};
		WITH args: GraphStochastic.Args DO
			ASSERT(args.vectors[0].components # NIL, 21);
			node.components := args.vectors[0].components;
			ASSERT(args.vectors[0].start >= 0, 21);
			node.start := args.vectors[0].start;
			node.step := args.vectors[0].step;
			node.nElem := args.vectors[0].nElem;
			i := 0;
			nElem := node.nElem;
			start := node.start;
			step := node.step;
			WHILE i < nElem DO
				p := node.components[start + i * step];
				IF p = NIL THEN
					res := {GraphNodes.arg1, GraphNodes.nil}; RETURN
				ELSIF ~(p IS GraphUnivariate.Node) THEN
					res := {GraphNodes.arg1, GraphNodes.notStochastic}; RETURN
				ELSIF (GraphStochastic.noCDF IN p.props) THEN
					res := {GraphNodes.arg1, GraphNodes.invalidParameters}; RETURN
				END;
				INC(i)
			END;
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
		signature := "vC"
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A Thomas"
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
END ReliabilitySystem.

