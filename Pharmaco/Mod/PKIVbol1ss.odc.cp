(*			(* PKBugs Version 2.0 *)



license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)


MODULE PharmacoPKIVbol1ss;


	IMPORT
		Math,
		GraphScalar, GraphNodes,
		PharmacoModel;

	TYPE
		Node = POINTER TO RECORD (PharmacoModel.Node) END;
		MemNode = POINTER TO RECORD (PharmacoModel.MetNode) END;
		Factory = POINTER TO RECORD (GraphScalar.Factory) END;
		memFactory = POINTER TO RECORD (GraphScalar.Factory) END;

	VAR
		fact-, memFact-: GraphScalar.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE GetNumArgs (OUT numParams, numScalars: INTEGER);
	BEGIN
		numParams := 2; numScalars := 3
	END GetNumArgs;

	PROCEDURE Value (params, scalars: GraphNodes.Vector): REAL;
		CONST
			eps = 1.0E-10;
		VAR
			CL, V, time, dose, omega, value: REAL;

		PROCEDURE ProfileSS (t: REAL): REAL;
		BEGIN
			IF t < 0 THEN RETURN dose / (V * (1 - Math.Exp( - CL * omega / V)))
			ELSE RETURN dose * Math.Exp( - CL * t / V) / (V * (1 - Math.Exp( - CL * omega / V)))
			END
		END ProfileSS;

	BEGIN
		CL := Math.Exp(params[0].Value());
		V := Math.Exp(params[1].Value());
		time := scalars[PharmacoModel.time].Value();
		dose := scalars[PharmacoModel.dose].Value();
		omega := scalars[PharmacoModel.omega].Value();
		IF time < - eps THEN time := time + omega * Math.Ceiling(( - time - eps) / omega) END;
		value := ProfileSS(time);
		IF value < 0 THEN value := 0 END;
		RETURN value
	END Value;

	PROCEDURE (node: Node) GetNumArgs (OUT numParams, numScalars: INTEGER);
	BEGIN
		GetNumArgs(numParams, numScalars)
	END GetNumArgs;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "PharmacoPKIVbol1ss.Install"
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
		install := "PharmacoPKIVbol1ss.MemInstall"
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
		signature := "vsss"
	END Signature;

	PROCEDURE (f: memFactory) New (): GraphScalar.Node;
		VAR
			node: MemNode;
	BEGIN
		NEW(node); node.Init; RETURN node
	END New;

	PROCEDURE (f: memFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vsss"
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
			fM: memFactory;
	BEGIN
		Maintainer;
		NEW(f); fact := f;
		NEW(fM); memFact := fM
	END Init;

BEGIN
	Init
END PharmacoPKIVbol1ss.
