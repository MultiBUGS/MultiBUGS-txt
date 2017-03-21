(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		  *)

(*
This template module has been set up to calulate the geometric mean of a vector
Code specific to this example is in red.
*)

MODULE GraphScalartemp1;

	

	IMPORT
		Math, 
		GraphNodes, GraphScalar, GraphScalarT;

	TYPE

		Node = POINTER TO RECORD(GraphScalarT.Node)
			(*
			Add any internal fields you wish to node
			*)
		END;

		Factory = POINTER TO RECORD(GraphScalar.Factory) END;

	VAR
		fact-: GraphNodes.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Node) CheckSpecial (VAR res: SET);
	BEGIN
		(*
		Do any special checks on the set values of the nodes parameters and use a non empty res
		to return the result. This is optional.
		*)
	END CheckSpecial;

	PROCEDURE (node: Node) ClassSpecial (parent: GraphNodes.Node; VAR class: INTEGER);
	BEGIN
		(*
		Do any special classification of functional form of logical node.
		This is optional.
		*)
	END ClassSpecial;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "GraphScalartemp1.Install"
	END Install;

	PROCEDURE (node: Node) Value (): REAL;
		VAR
			value: REAL;
			i, size, start, step: INTEGER;
			log: REAL;
	BEGIN
		(*
		Calculate value of node here and put its value into variable value.
		This is compulsory
		*)
		i := 0;
		size := node.size[0];
		start := node.start[0];
		step := node.step[0];
		log := 0.0;
		WHILE i < size DO
			log := log + Math.Ln(node.vectors[0][start + i * step].Value());
			INC(i)
		END;
		log := log / size;
		value := Math.Exp(log);
		RETURN value
	END Value;

	PROCEDURE (node: Node) ValDiff (x: GraphNodes.Node; OUT val, diff: REAL);
	BEGIN
		HALT(126)
	END ValDiff;

	PROCEDURE (f: Factory) New (): GraphScalar.Node;
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
		NEW(f);
		fact := f
	END Init;

BEGIN
	Init
END GraphScalartemp1.
