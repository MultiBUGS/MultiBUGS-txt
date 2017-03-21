(*

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"
*)

MODULE DiffFiveCompModel;

	

	IMPORT
		Math,
		GraphNodes, GraphODEmath, GraphVector,
		MathODE, MathRungeKutta45;

	TYPE
		Equations = POINTER TO RECORD (MathODE.Equations) END;

		Factory = POINTER TO RECORD(GraphVector.Factory) END;

	CONST
		nEq = 7;

		Q_PP = 0; Q_RP = 1; Q_GU = 2; Q_LI = 3; Q_LU = 4;
		V_PP = 5; V_RP = 6; V_GU = 7; V_LI = 8; V_LU = 9;
		V_VEN = 10; V_ART = 11; log_KP_PP = 12; log_KP_RP = 13;
		log_KP_GU = 14; log_KP_LI = 15; log_KP_LU = 16;
		Vmax = 17; Km = 18; dose = 19; frac = 20; log_ka = 21;

		PP = 0; RP = 1; GU = 2; LI = 3; LU = 4; VEN = 5; ART = 6;

	VAR
		fact-: GraphNodes.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (e: Equations) Derivatives (IN parameters, C: ARRAY OF REAL;
		n: INTEGER; t: REAL;
	OUT dCdt: ARRAY OF REAL);
		VAR
			i: INTEGER;
			ka, RM, RA, QH, QTOT: REAL;
			KI, KO: ARRAY 5 OF REAL;
	BEGIN
		i := 0;
		WHILE i < 5 DO;
			KI[i] := parameters[Q_PP + i] / parameters[V_PP + i];
			KO[i] := KI[i] / Math.Exp(parameters[log_KP_PP + i]);
			INC(i);
		END;
		ka := Math.Exp(parameters[log_ka]);
		RA := ka * parameters[frac] * parameters[dose] * Math.Exp( - ka * t);
		RM := parameters[Vmax] * C[LI] / (parameters[Km] + C[LI]);
		QH := parameters[Q_LI] - parameters[Q_GU]; QTOT := parameters[Q_LU];
		dCdt[PP] := KI[PP] * C[ART] - KO[PP] * C[PP];
		dCdt[RP] := KI[RP] * C[ART] - KO[RP] * C[RP];
		dCdt[GU] := KI[GU] * C[ART] - KO[GU] * C[GU];
		dCdt[LI] := (QH * C[ART] + KO[GU] * parameters[V_GU] * C[GU] + RA - RM)
		 / parameters[V_LI] - KO[LI] * C[LI];
		dCdt[LU] := KI[LU] * C[VEN] - KO[LU] * C[LU];
		dCdt[VEN] := (KO[PP] * parameters[V_PP] * C[PP] + KO[RP] * parameters[V_RP] * C[RP]
		 + KO[LI] * parameters[V_LI] * C[LI] - KI[LU] * parameters[V_LU] * C[VEN])
		 / parameters[V_VEN];
		dCdt[ART] := (KO[LU] * parameters[V_LU] * C[LU] - QTOT * C[ART])
		 / parameters[V_ART];
	END Derivatives;

	PROCEDURE (e: Equations) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "DiffFiveCompModel.Install"
	END Install;

	PROCEDURE (e: Equations) SecondDerivatives (IN parameters, x: ARRAY OF REAL;
	numEq: INTEGER; t: REAL;
	OUT d2xdt2: ARRAY OF REAL);
	BEGIN
		HALT(126)
	END SecondDerivatives;

	PROCEDURE (e: Equations) Jacobian (IN parameters, x: ARRAY OF REAL;
	numEq: INTEGER; t: REAL;
	OUT jacob: ARRAY OF ARRAY OF REAL);
	BEGIN
		HALT(126)
	END Jacobian;

	PROCEDURE (f: Factory) New (): GraphVector.Node;
		VAR
			node: GraphVector.Node;
			equations: Equations;
			solver: MathODE.Solver;
	BEGIN
		NEW(equations);
		solver := MathRungeKutta45.fact.New();
		node := GraphODEmath.New(solver, equations, nEq);
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vvvsv"
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "Dave Lunn"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f
	END Init;

BEGIN
	Init
END DiffFiveCompModel.
