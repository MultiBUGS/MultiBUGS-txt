(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	   *)

MODULE GraphLog;


	

	IMPORT
		Math, 
		GraphLinkfunc, GraphNodes, GraphRules, GraphScalar, GraphStochastic;

	TYPE
		Node = POINTER TO RECORD(GraphLinkfunc.Node) END;

		Factory = POINTER TO RECORD(GraphLinkfunc.Factory) END;

	VAR
		fact-: GraphLinkfunc.Factory;

		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) ClassFunction (parent: GraphNodes.Node): INTEGER;
		VAR
			form: INTEGER;
			predictor: GraphNodes.Node;
	BEGIN
		predictor := node.predictor;
		form := GraphStochastic.ClassFunction(predictor, parent);
		form := GraphRules.logF[form];
		RETURN form
	END ClassFunction;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphLog.Install"
	END Install;

	PROCEDURE (node: Node) Value (): REAL;
		VAR
			ln, value: REAL;
			predictor: GraphNodes.Node;
	BEGIN
		predictor := node.predictor;
		ln := predictor.Value();
		value := Math.Exp(ln);
		RETURN value
	END Value;

	PROCEDURE (node: Node) ValDiff (x: GraphNodes.Node; OUT val, diff: REAL);
		VAR
			predictor: GraphNodes.Node;
	BEGIN
		predictor := node.predictor;
		predictor.ValDiff(x, val, diff);
		val := Math.Exp(val);
		diff := diff * val
	END ValDiff;
	
	PROCEDURE (f: Factory) New (): GraphLinkfunc.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node);
		node.Init;
		RETURN node
	END New;

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

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

BEGIN
	Init
END GraphLog.
