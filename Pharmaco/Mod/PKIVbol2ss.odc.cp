(*			(* PKBugs Version 2.0 *)



license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)


MODULE PharmacoPKIVbol2ss;


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
		numParams := 4; numScalars := 3
	END GetNumArgs;

	PROCEDURE  Value (params, scalars: GraphNodes.Vector): REAL;
		CONST
			eps = 1.0E-10;
		VAR
			logCL, logQ, logV1, logV2, k10, k12, k21, sum, lambda1, lambda2,
			A, time, dose, omega, value: REAL;

		PROCEDURE ProfileSS (t: REAL): REAL;
			VAR
				C1, C2: REAL;
		BEGIN
			IF t < 0 THEN
				C1 := A / (1 - Math.Exp( - lambda1 * omega)); C2 := (1 - A) / (1 - Math.Exp( - lambda2 * omega))
			ELSE
				C1 := A * Math.Exp( - lambda1 * t) / (1 - Math.Exp( - lambda1 * omega));
				C2 := (1 - A) * Math.Exp( - lambda2 * t) / (1 - Math.Exp( - lambda2 * omega))
			END;
			RETURN dose * (C1 + C2) / Math.Exp(logV1)
		END ProfileSS;

	BEGIN
		logCL := params[0].Value();
		logQ := params[1].Value();
		logV1 := params[2].Value();
		logV2 := params[3].Value();
		time := scalars[PharmacoModel.time].Value();
		dose := scalars[PharmacoModel.dose].Value();
		omega := scalars[PharmacoModel.omega].Value();
		k10 := Math.Exp(logCL - logV1);
		k12 := Math.Exp(logQ - logV1); k21 := Math.Exp(logQ - logV2);
		sum := k10 + k12 + k21;
		lambda1 := 0.5 * (sum + Math.Sqrt(sum * sum - 4 * k10 * k21));
		lambda2 := sum - lambda1;
		A := (lambda1 - k21) / (lambda1 - lambda2);
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
		install := "PharmacoPKIVbol2ss.Install"
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
		install := "PharmacoPKIVbol2ss.MemInstall"
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
END PharmacoPKIVbol2ss.
