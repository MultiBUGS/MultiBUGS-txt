(*			(* PKBugs Version 2.0 *)


license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)


MODULE PharmacoPKFO1;


	IMPORT
		Math,
		GraphNodes, GraphScalar,
		PharmacoModel;

	TYPE
		Node = POINTER TO RECORD (PharmacoModel.Node) END;
		MemNode = POINTER TO RECORD (PharmacoModel.MetNode) END;
		Factory = POINTER TO RECORD (GraphScalar.Factory) END;
		MemFactory = POINTER TO RECORD (GraphScalar.Factory) END;

	VAR
		fact-, memFact-: GraphScalar.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE GetNumArgs (OUT numParams, numScalars: INTEGER);
	BEGIN
		numParams := 3; numScalars := 2
	END GetNumArgs;

	PROCEDURE Value (params, scalars: GraphNodes.Vector): REAL;
		VAR
			CL, V, time, dose, ka, value: REAL;
	BEGIN
		CL := Math.Exp(params[0].Value());
		V := Math.Exp(params[1].Value());
		ka := Math.Exp(params[2].Value()) + CL / V;
		time := scalars[PharmacoModel.time].Value();
		dose := scalars[PharmacoModel.dose].Value();
		IF time < 0 THEN value := 0
		ELSE value := dose * ka * (Math.Exp( - ka * time) - Math.Exp( - CL * time / V)) / (CL - ka * V)
		END;
		IF value < 0 THEN value := 0 END;
		RETURN value
	END Value;

	PROCEDURE (node: Node) GetNumArgs (OUT numParams, numScalars: INTEGER);
	BEGIN
		GetNumArgs(numParams, numScalars)
	END GetNumArgs;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "PharmacoPKFO1.Install"
	END Install;

	PROCEDURE (node: Node) Value (): REAL;
	BEGIN
		RETURN Value(node.params, node.scalars)
	END Value;

	PROCEDURE (node: MemNode) GetNumArgs (OUT numParams, numScalars: INTEGER);
	BEGIN
		GetNumArgs(numParams, numScalars)
	END GetNumArgs;

	PROCEDURE (node: MemNode) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "PharmacoPKFO1.MemInstall"
	END Install;

	PROCEDURE (node: MemNode) Evaluate (OUT value: REAL);
	BEGIN
		value := Value(node.params, node.scalars)
	END Evaluate;

	PROCEDURE (f: Factory) New (): GraphScalar.Node;
		VAR
			node: Node;
	BEGIN
		NEW(node); node.Init; RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vss"
	END Signature;

	PROCEDURE (f: MemFactory) New (): GraphScalar.Node;
		VAR
			node: MemNode;
	BEGIN
		NEW(node); node.Init; RETURN node
	END New;

	PROCEDURE (f: MemFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vss"
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE MemInstall*;
	BEGIN
		GraphNodes.SetFactory(memFact)
	END MemInstall;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "D.Lunn"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory;
			fM: MemFactory;
	BEGIN
		Maintainer;
		NEW(f); fact := f;
		NEW(fM); memFact := fM
	END Init;

BEGIN
	Init
END PharmacoPKFO1.
