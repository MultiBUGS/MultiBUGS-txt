(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphProbit;


	

	IMPORT
		Math,
		GraphLinkfunc, GraphNodes, GraphRules, GraphScalar, GraphStochastic,
		MathFunc;

	TYPE
		Node = POINTER TO RECORD(GraphLinkfunc.Node) END;

		Factory = POINTER TO RECORD(GraphLinkfunc.Factory) END;

	CONST
		maxRange = 8.5;

	VAR
		fact-: GraphLinkfunc.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 20 OF CHAR;

	PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			form: INTEGER;
			predictor: GraphNodes.Node;
	BEGIN
		predictor := node.predictor;
		form := GraphStochastic.ClassFunction(predictor, parent);
		form := GraphRules.probitF[form];
		RETURN form
	END ClassFunction;

	PROCEDURE (node: Node) Evaluate;
		VAR
			x, value: REAL;
			predictor: GraphNodes.Node;
	BEGIN
		predictor := node.predictor;
		x := predictor.value;
		(*	IF ABS(x) > maxRange THEN
		HALT(0)
		END;*)
		node.value := MathFunc.Phi(x);
	END Evaluate;

	PROCEDURE (node: Node) EvaluateDiffs;
		VAR
			predictor: GraphNodes.Node;
			x: GraphNodes.Vector;
			val, sqrt: REAL;
			i, N: INTEGER;
	BEGIN
		x := node.diffWRT;
		N := LEN(x);
		predictor := node.predictor;
		val := predictor.value;
		sqrt := Math.Sqrt(2 * Math.Pi());
		i := 0;
		WHILE i < N DO
			node.diffs[i] := predictor.Diff(x[i]) * Math.Exp( - 0.5 * val * val) / sqrt;
			INC(i)
		END;
		node.value := MathFunc.Phi(val)
	END EvaluateDiffs;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphProbit.Install"
	END Install;

	PROCEDURE (f: Factory) New (): GraphLinkfunc.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

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
END GraphProbit.
