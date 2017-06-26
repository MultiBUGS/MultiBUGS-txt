(*			(* PKBugs Version 2.0 *)


license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)

MODULE PharmacoPKIVbol1;

	

	IMPORT
		Math,
		GraphNodes, GraphScalar,
		PharmacoModel;

	TYPE
		Node = POINTER TO RECORD (PharmacoModel.Node) END;
		Factory = POINTER TO RECORD (GraphScalar.Factory) END;

	VAR
		fact-: GraphScalar.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;


	PROCEDURE Value (params, scalars: GraphNodes.Vector): REAL;
		CONST
			eps = 1.0E-10;
		VAR
			CL, V, time, dose, value: REAL;
	BEGIN
		CL := Math.Exp(params[0].Value());
		V := Math.Exp(params[1].Value());
		time := scalars[PharmacoModel.time].Value();
		dose := scalars[PharmacoModel.dose].Value();
		IF time < - eps THEN value := 0
		ELSIF time < 0 THEN value := dose / V
		ELSE value := dose * Math.Exp( - CL * time / V) / V
		END;
		IF value < 0 THEN value := 0 END;
		RETURN value
	END Value;

	PROCEDURE GetNumArgs (OUT numParams, numScalars: INTEGER);
	BEGIN
		numParams := 2; numScalars := 2
	END GetNumArgs;

	PROCEDURE (node: Node) GetNumArgs (OUT numParams, numScalars: INTEGER);
	BEGIN
		GetNumArgs(numParams, numScalars)
	END GetNumArgs;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "PharmacoPKIVbol1.Install"
	END Install;

	PROCEDURE (node: Node) Value (): REAL;
	BEGIN
		RETURN Value(node.params, node.scalars)
	END Value;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vss"
	END Signature;

	PROCEDURE (f: Factory) New ((*option: INTEGER*)): GraphScalar.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node); node.Init; RETURN node
	END New;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "D.Lunn"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f); fact := f;
	END Init;

BEGIN
	Init
END PharmacoPKIVbol1.
