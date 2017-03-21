(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



e		 *)

MODULE SpatialMaternKrig;


	

	IMPORT
		Math, 
		GraphMultivariate, GraphNodes, GraphStochastic,
		MathFunc,
		SpatialStrucMVN;

	TYPE

		Factory = POINTER TO RECORD(GraphMultivariate.Factory) END;

		Kernel = POINTER TO RECORD(SpatialStrucMVN.SpatialKernel) END;

	VAR
		fact-: GraphMultivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		kernel: Kernel;

	PROCEDURE (kernel: Kernel) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "SpatialMaternKrig.Install"
	END Install;

	PROCEDURE (kernel: Kernel) Element (x1, x2: SpatialStrucMVN.Point; params: ARRAY OF REAL): REAL;
		CONST
			eps = 1.0E-10;
		VAR
			dim, i, n: INTEGER;
			dist, element: REAL;
	BEGIN
		dist := 0.0;
		dim := LEN(x1);
		i := 0;
		WHILE i < dim DO
			dist := dist + (x1[i] - x2[i]) * (x1[i] - x2[i]);
			INC(i)
		END;
		dist := Math.Sqrt(dist);
		n := SHORT(ENTIER(params[1] + eps));
		element := 2 * Math.Power(0.5 * params[0] * dist, n) * MathFunc.BesselK(n, params[0] * dist) / 
		Math.Exp(MathFunc.LogGammaFunc(n));
		RETURN element
	END Element;

	PROCEDURE (kernel: Kernel) NumParams (): INTEGER;
	BEGIN
		RETURN 2
	END NumParams;

	PROCEDURE (f: Factory) New (): GraphMultivariate.Node;
		VAR
			node: SpatialStrucMVN.Node;
			p: GraphStochastic.Node;
	BEGIN
		p := SpatialStrucMVN.fact.New();
		node := p(SpatialStrucMVN.Node);
		node.Init;
		node.SetKernel(kernel);
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
		VAR
			i, numParams: INTEGER;
	BEGIN
		signature := "vvvs";
		i := 0;
		numParams := kernel.NumParams();
		WHILE i < numParams DO
			signature := signature + "s";
			INC(i)
		END
	END Signature;

	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f;
		NEW(kernel)
	END Init;

BEGIN
	Init
END SpatialMaternKrig.
