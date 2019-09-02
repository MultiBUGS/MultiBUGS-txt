(*			(* PKBugs Version 2.0 *)



license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)


MODULE PharmacoPKIVinf3ss;


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
		numParams := 6; numScalars := 4
	END GetNumArgs;

	PROCEDURE Value (params, scalars: GraphNodes.Vector): REAL;
		VAR
			logCL, logQ12, logQ13, logV1, logV2, V3, time, dose, TI, omega, phi,
			k10, k12, k21, k13, k31, alpha, beta, gamma, a, b, A, B, C, value: REAL;
			lambda, temp: ARRAY 3 OF REAL; i: INTEGER;

		PROCEDURE ProfileSS (t: REAL): REAL;
			VAR
				C1, C2, C3: REAL;
		BEGIN
			ASSERT(t >= 0, 77);
			IF t < TI THEN
				C1 := 1 + ((Math.Exp( - lambda[0] * (omega - TI)) - 1) / 
				(1 - Math.Exp( - lambda[0] * omega))) * Math.Exp( - lambda[0] * t);
				C2 := 1 + ((Math.Exp( - lambda[1] * (omega - TI)) - 1) / 
				(1 - Math.Exp( - lambda[1] * omega))) * Math.Exp( - lambda[1] * t);
				C3 := 1 + ((Math.Exp( - lambda[2] * (omega - TI)) - 1) / 
				(1 - Math.Exp( - lambda[2] * omega))) * Math.Exp( - lambda[2] * t);
				C1 := A * C1 / lambda[0]; C2 := B * C2 / lambda[1]; C3 := C * C3 / lambda[2]
			ELSE
				C1 := A * ((Math.Exp( - lambda[0] * (t - TI)) - Math.Exp( - lambda[0] * t)) / 
				(1 - Math.Exp( - lambda[0] * omega))) / lambda[0];
				C2 := B * ((Math.Exp( - lambda[1] * (t - TI)) - Math.Exp( - lambda[1] * t)) / 
				(1 - Math.Exp( - lambda[1] * omega))) / lambda[1];
				C3 := C * ((Math.Exp( - lambda[2] * (t - TI)) - Math.Exp( - lambda[2] * t)) / 
				(1 - Math.Exp( - lambda[2] * omega))) / lambda[2]
			END;
			RETURN dose * (C1 + C2 + C3) / (Math.Exp(logV1) * TI)
		END ProfileSS;

	BEGIN
		logCL := params[0].value;
		logQ12 := params[1].value;
		logQ13 := params[2].value;
		logV1 := params[3].value;
		logV2 := params[4].value;
		V3 := Math.Exp(logV2) + Math.Exp(params[5].value);
		time := scalars[PharmacoModel.time].value;
		dose := scalars[PharmacoModel.dose].value;
		TI := scalars[PharmacoModel.TISS].value;
		omega := scalars[PharmacoModel.omega].value;
		k10 := Math.Exp(logCL - logV1);
		k12 := Math.Exp(logQ12 - logV1); k21 := Math.Exp(logQ12 - logV2);
		k13 := Math.Exp(logQ13 - logV1); k31 := Math.Exp(logQ13) / V3;
		alpha := k10 + k12 + k21 + k13 + k31;
		beta := k31 * (k10 + k12 + k21) + k21 * (k10 + k13);
		gamma := k10 * k21 * k31;
		a := beta - Math.IntPower(alpha, 2) / 3;
		b := 2 * Math.IntPower(alpha, 3) / 27 - alpha * beta / 3 + gamma;
		phi := Math.ArcCos( - 0.5 * b / Math.Sqrt( - Math.IntPower(a, 3) / 27));
		i := 0;
		WHILE i < 3 DO
			temp[i] := alpha / 3 - 2 * Math.Sqrt( - a / 3) * Math.Cos((phi + 2 * i * Math.Pi()) / 3);
			INC(i)
		END;
		lambda[0] := MAX(MAX(temp[0], temp[1]), temp[2]); lambda[2] := MIN(MIN(temp[0], temp[1]), temp[2]);
		IF (temp[0] > MAX(temp[1], temp[2])) THEN lambda[1] := MAX(temp[1], temp[2])
		ELSE lambda[1] := MAX(temp[0], MIN(temp[1], temp[2]))
		END;
		A := (k31 - lambda[0]) * (k21 - lambda[0]) / ((lambda[1] - lambda[0]) * (lambda[2] - lambda[0]));
		B := (k31 - lambda[1]) * (k21 - lambda[1]) / ((lambda[0] - lambda[1]) * (lambda[2] - lambda[1]));
		C := 1 - A - B;
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
		install := "PharmacoPKIVinf3ss.Install"
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
		signature := "vssss"
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
END PharmacoPKIVinf3ss.
