(*			(* PKBugs Version 2.0 *)

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)

MODULE PharmacoPKIVinf1;

	

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
		numParams := 2; numScalars := 3
	END GetNumArgs;

	PROCEDURE Value (params, scalars: GraphNodes.Vector): REAL;
		VAR
			CL, V, time, dose, TI, value: REAL;
	BEGIN
		CL := Math.Exp(params[0].Value());
		V := Math.Exp(params[1].Value());
		time := scalars[PharmacoModel.time].Value();
		dose := scalars[PharmacoModel.dose].Value();
		TI := scalars[PharmacoModel.TI].Value();
		IF time < 0 THEN value := 0
		ELSE
			IF time < TI THEN value := dose * (1 - Math.Exp( - CL * time / V)) / (CL * TI)
			ELSE value := dose * (Math.Exp( - CL * (time - TI) / V) - Math.Exp( - CL * time / V)) / (CL * TI)
			END
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
		install := "PharmacoPKIVinf1.Install"
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
END PharmacoPKIVinf1.
