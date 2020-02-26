(*

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"
*)

MODULE DiffHPS_V2_FB;

	

	IMPORT
		GraphNodes, GraphODEmath, GraphVector,
		MathODE, MathRungeKutta45;

	TYPE
		Equations = POINTER TO RECORD (MathODE.Equations) END;

		Factory = POINTER TO RECORD(GraphVector.Factory) END;

	CONST
		nEq = 6; S = 0; C1 = 1; C2 = 2; I = 3; F1 = 4; F2 = 5;

	VAR
		fact-: GraphVector.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		PROCEDURE (e: Equations) Derivatives (IN theta, P: ARRAY OF REAL; n: INTEGER; t: REAL;
	OUT dPdt: ARRAY OF REAL);
		VAR
			rs, rc, ri, vc, vi, beta, alpha, k, delta, rho1, rho2, ms, mc, mi, epsilon, N, C, T1, T2, D1, D2: REAL;
	BEGIN

		rs := theta[0]; rc := theta[1]; ri := theta[2]; vc := theta[3]; vi := theta[4]; beta := theta[5];
		alpha := theta[6]; k := theta[7]; delta := theta[8]; rho1 := theta[9]; rho2 := theta[10];
		ms := theta[11]; mc := theta[12]; mi := theta[13]; epsilon := theta[14];

		N := P[S] + P[C1] + P[C2] + P[I];
		C := P[C1] + P[C2];
		T1 := beta * P[F1] / (1 + alpha * P[F1]);
		T2 := beta * P[F2] / (1 + alpha * P[F2]);
		D1 := MAX(1.0E-10, delta * (N / k - rho1) / (1 - rho1));
		D2 := MAX(1.0E-10, delta * (N / k - rho2) / (1 - rho2));

		dPdt[S] := (rs * P[S] + rc * (1 - vc) * C + ri * (1 - vi) * P[I]) * (1 - N / k) - (T1 + T2) * P[S] - ms * P[S];
		dPdt[C1] := rc * vc * P[C1] * (1 - N / k) + T1 * P[S] - D1 * P[C1] - mc * P[C1];
		dPdt[C2] := rc * vc * P[C2] * (1 - N / k) + T2 * P[S] - D2 * P[C2] - mc * P[C2];
		dPdt[I] := ri * vi * P[I] * (1 - N / k) + D1 * P[C1] + D2 * P[C2] - mi * P[I];
		dPdt[F1] := - epsilon * P[F1];
		dPdt[F2] := P[I] - epsilon * P[F2];
	END Derivatives;


	PROCEDURE (e: Equations) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "DiffHPS_V2_FB.Install"
	END Install;

	PROCEDURE (e: Equations) SecondDerivatives (IN theta,
	x: ARRAY OF REAL; numEq: INTEGER; t: REAL; OUT d2xdt2: ARRAY OF REAL);
	BEGIN
		HALT(126)
	END SecondDerivatives;

	PROCEDURE (e: Equations) Jacobian (IN theta, x: ARRAY OF REAL;
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
		solver := MathRungeKutta45.New();
		node := GraphODEmath.New(solver, equations, nEq);
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vvvss"
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
END DiffHPS_V2_FB.
