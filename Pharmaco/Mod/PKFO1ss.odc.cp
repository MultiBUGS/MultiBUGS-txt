(*			(* PKBugs Version 2.0 *)



license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)


MODULE PharmacoPKFO1ss;


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
		numParams := 3; numScalars := 3
	END GetNumArgs;

	PROCEDURE Value (params, scalars: GraphNodes.Vector): REAL;
		VAR
			CL, V, time, dose, omega, ka, value: REAL;

		PROCEDURE ProfileSS (t: REAL): REAL;
			VAR
				Ca, Ce: REAL;
		BEGIN
			ASSERT(t >= 0, 77);
			Ca := Math.Exp( - ka * t) / (1 - Math.Exp( - ka * omega));
			Ce := Math.Exp( - CL * t / V) / (1 - Math.Exp( - CL * omega / V));
			RETURN dose * ka * (Ca - Ce) / (CL - ka * V)
		END ProfileSS;

	BEGIN
		CL := Math.Exp(params[0].value);
		V := Math.Exp(params[1].value);
		ka := Math.Exp(params[2].value) + CL / V;
		time := scalars[PharmacoModel.time].value;
		dose := scalars[PharmacoModel.dose].value;
		omega := scalars[PharmacoModel.omega].value;
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
		install := "PharmacoPKFO1ss.Install"
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
END PharmacoPKFO1ss.
