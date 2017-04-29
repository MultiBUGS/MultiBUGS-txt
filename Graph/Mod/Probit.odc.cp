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

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphProbit.Install"
	END Install;

	PROCEDURE (node: Node) Value (): REAL;
		VAR
			x, value: REAL;
			predictor: GraphNodes.Node;
	BEGIN
		predictor := node.predictor;
		x := predictor.Value();
	(*	IF ABS(x) > maxRange THEN
			HALT(0)
		END;*)
		value := MathFunc.Phi(x);
		RETURN value
	END Value;

	PROCEDURE (node: Node) ValDiff (x: GraphNodes.Node; OUT val, diff: REAL);
		VAR
			predictor: GraphNodes.Node;
	BEGIN
		predictor := node.predictor;
		predictor.ValDiff(x, val, diff);
		diff := diff * Math.Exp( - 0.5 * val * val) / Math.Sqrt(2 * Math.Pi());
		val := MathFunc.Phi(val)
	END ValDiff;

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
