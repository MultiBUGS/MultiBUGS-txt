(*			(* PKBugs Version 2.0 *)


license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)


MODULE PharmacoPKFO3;


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
		numParams := 7; numScalars := 2
	END GetNumArgs;

	PROCEDURE Value (params, scalars: GraphNodes.Vector): REAL;
		CONST
			eps = 1.0E-10;
		VAR
			logCL, logQ12, logQ13, logV1, logV2, V3, time, dose, phi,
			k10, k12, k21, k13, k31, alpha, beta, gamma, a, b, A, B, C, value: REAL;
			Ca, Ce, ka: REAL;
			lambda, temp, exp: ARRAY 3 OF REAL; i: INTEGER;
	BEGIN
		logCL := params[0].Value();
		logQ12 := params[1].Value();
		logQ13 := params[2].Value();
		logV1 := params[3].Value();
		logV2 := params[4].Value();
		V3 := Math.Exp(logV2) + Math.Exp(params[5].Value());
		time := scalars[PharmacoModel.time].Value();
		dose := scalars[PharmacoModel.dose].Value();
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
		ka := lambda[0] + Math.Exp(params[6].Value());
		IF time < 0 THEN
			Ca := 0; Ce := 0
		ELSE
			Ca := (A / (lambda[0] - ka) + B / (lambda[1] - ka) + C / (lambda[2] - ka)) * Math.Exp( - ka * time);
			i := 0; WHILE i < 3 DO exp[i] := Math.Exp( - lambda[i] * time); INC(i) END;
			Ce := A * exp[0] / (ka - lambda[0]) + B * exp[1] / (ka - lambda[1]) + C * exp[2] / (ka - lambda[2])
		END;
		value := dose * ka * (Ca + Ce) / Math.Exp(logV1);
		IF value < 0 THEN value := 0 END;
		RETURN value
	END Value;

	PROCEDURE (node: Node) GetNumArgs (OUT numParams, numScalars: INTEGER);
	BEGIN
		GetNumArgs(numParams, numScalars)
	END GetNumArgs;

	PROCEDURE (node: Node) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "PharmacoPKFO3.Install"
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
END PharmacoPKFO3.
