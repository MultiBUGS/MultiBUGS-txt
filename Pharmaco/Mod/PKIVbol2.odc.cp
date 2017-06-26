(*			(* PKBugs Version 2.0 *)


license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)

MODULE PharmacoPKIVbol2;

	

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

	PROCEDURE GetNumArgs (OUT numParams, numScalars: INTEGER);
	BEGIN
		numParams := 4; numScalars := 2
	END GetNumArgs;

	PROCEDURE Value (params, scalars: GraphNodes.Vector): REAL;
		CONST
			eps = 1.0E-10;
		VAR
			logCL, logQ, logV1, logV2, k10, k12, k21,
			sum, lambda1, lambda2, A, time, dose, value: REAL;
	BEGIN
		logCL := params[0].Value();
		logQ := params[1].Value();
		logV1 := params[2].Value();
		logV2 := params[3].Value();
		time := scalars[PharmacoModel.time].Value();
		dose := scalars[PharmacoModel.dose].Value();
		k10 := Math.Exp(logCL - logV1);
		k12 := Math.Exp(logQ - logV1); k21 := Math.Exp(logQ - logV2);
		sum := k10 + k12 + k21;
		lambda1 := 0.5 * (sum + Math.Sqrt(sum * sum - 4 * k10 * k21));
		lambda2 := sum - lambda1;
		A := (lambda1 - k21) / (lambda1 - lambda2);
		IF time < - eps THEN value := 0
		ELSIF time < 0 THEN value := dose / Math.Exp(logV1)
		ELSE
			value := dose * (A * Math.Exp( - lambda1 * time)
			 + (1 - A) * Math.Exp( - lambda2 * time)) / Math.Exp(logV1)
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
		install := "PharmacoPKIVbol2.Install"
	END Install;

	PROCEDURE (node: Node) Value (): REAL;
	BEGIN
		RETURN Value(node.params, node.scalars)
	END Value;

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
END PharmacoPKIVbol2.
