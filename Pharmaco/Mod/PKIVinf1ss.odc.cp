(*			(* PKBugs Version 2.0 *)



license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)


MODULE PharmacoPKIVinf1ss;


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
		numParams := 2; numScalars := 4
	END GetNumArgs;

	PROCEDURE Value (params, scalars: GraphNodes.Vector): REAL;
		VAR
			CL, V, time, dose, omega, TI, value: REAL;

		PROCEDURE ProfileSS (t: REAL): REAL;
			VAR
				C: REAL;
		BEGIN
			ASSERT(t >= 0, 77);
			IF t < TI THEN
				C := 1 + ((Math.Exp( - CL * (omega - TI) / V) - 1) / 
				(1 - Math.Exp( - CL * omega / V))) * Math.Exp( - CL * t / V)
			ELSE
				C := (Math.Exp( - CL * (t - TI) / V) - Math.Exp( - CL * t / V)) / (1 - Math.Exp( - CL * omega / V))
			END;
			RETURN dose * C / (TI * CL)
		END ProfileSS;

	BEGIN
		CL := Math.Exp(params[0].Value());
		V := Math.Exp(params[1].Value());
		time := scalars[PharmacoModel.time].Value();
		dose := scalars[PharmacoModel.dose].Value();
		TI := scalars[PharmacoModel.TISS].Value();
		omega := scalars[PharmacoModel.omega].Value();
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
		install := "PharmacoPKIVinf1ss.Install"
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
		install := "PharmacoPKIVinf1ss.MemInstall"
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
		signature := "vssss"
	END Signature;

	PROCEDURE (f: memFactory) New (): GraphScalar.Node;
		VAR
			node: MemNode;
	BEGIN
		NEW(node); node.Init; RETURN node
	END New;

	PROCEDURE (f: memFactory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vssss"
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
END PharmacoPKIVinf1ss.
