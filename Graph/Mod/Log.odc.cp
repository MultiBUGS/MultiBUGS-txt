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
			stochastic: GraphStochastic.Node;
	BEGIN
		stochastic := parent(GraphStochastic.Node);
		predictor := node.predictor;
		form := GraphStochastic.ClassFunction(predictor, stochastic);
		form := GraphRules.logF[form];
		RETURN form
	END ClassFunction;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphLog.Install"
	END Install;

	PROCEDURE (node: Node) Evaluate;
		VAR
			ln: REAL;
			predictor: GraphNodes.Node;
	BEGIN
		predictor := node.predictor;
		ln := predictor.value;
		node.value := Math.Exp(ln);
	END Evaluate;

	PROCEDURE (node: Node) EvaluateDiffs ;
		VAR
			predictor: GraphNodes.Node;
			x: GraphNodes.Vector;
			val: REAL;
			i, N: INTEGER;
	BEGIN
		x := node.parents;
		N := LEN(x);
		predictor := node.predictor;
		val := predictor.value;
		val := Math.Exp(val);
		i := 0; WHILE i < N DO node.work[i] := predictor.Diff(x[i]) * val; INC(i) END;
		node.value := val
	END EvaluateDiffs;
	
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
