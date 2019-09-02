(*			(* PKBugs Version 2.0 *)



license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)


MODULE PharmacoPKZO2ss;


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
		numParams := 5; numScalars := 3
	END GetNumArgs;

	PROCEDURE Value (params, scalars: GraphNodes.Vector): REAL;
		VAR
			logCL, logQ, logV1, logV2, k10, k12, k21, sum, lambda1, lambda2,
			A, time, dose, omega, TI, value: REAL;

		PROCEDURE ProfileSS (t: REAL): REAL;
			VAR
				C1, C2: REAL;
		BEGIN
			ASSERT(t >= 0, 77);
			IF t < TI THEN
				C1 := 1 + ((Math.Exp( - lambda1 * (omega - TI)) - 1) / 
				(1 - Math.Exp( - lambda1 * omega))) * Math.Exp( - lambda1 * t);
				C2 := 1 + ((Math.Exp( - lambda2 * (omega - TI)) - 1) / 
				(1 - Math.Exp( - lambda2 * omega))) * Math.Exp( - lambda2 * t);
				C1 := A * C1 / lambda1; C2 := (1 - A) * C2 / lambda2
			ELSE
				C1 := A * ((Math.Exp( - lambda1 * (t - TI)) - Math.Exp( - lambda1 * t)) / 
				(1 - Math.Exp( - lambda1 * omega))) / lambda1;
				C2 := (1 - A) * ((Math.Exp( - lambda2 * (t - TI)) - Math.Exp( - lambda2 * t)) / 
				(1 - Math.Exp( - lambda2 * omega))) / lambda2
			END;
			RETURN dose * (C1 + C2) / (Math.Exp(logV1) * TI)
		END ProfileSS;

	BEGIN
		logCL := params[0].value;
		logQ := params[1].value;
		logV1 := params[2].value;
		logV2 := params[3].value;
		TI := Math.Exp(params[4].value);
		time := scalars[PharmacoModel.time].value;
		dose := scalars[PharmacoModel.dose].value;
		omega := scalars[PharmacoModel.omega].value;
		k10 := Math.Exp(logCL - logV1);
		k12 := Math.Exp(logQ - logV1); k21 := Math.Exp(logQ - logV2);
		sum := k10 + k12 + k21;
		lambda1 := 0.5 * (sum + Math.Sqrt(sum * sum - 4 * k10 * k21));
		lambda2 := sum - lambda1;
		A := (lambda1 - k21) / (lambda1 - lambda2);
		IF time < 0 THEN time := time + omega * Math.Ceiling( - time / omega) END;
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
		install := "PharmacoPKZO2ss.Install"
	END Install;

	PROCEDURE (node: Node) Evaluate;
	BEGIN
		node.value := Value(node.params, node.scalars)
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
END PharmacoPKZO2ss.
