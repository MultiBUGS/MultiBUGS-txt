(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



e		 *)

MODULE SpatialMaternKrig;


	

	IMPORT
		Math,
		GraphChain, GraphGPprior, GraphMultivariate, GraphNodes, GraphScalar, GraphStochastic,
		MathFunc;

	TYPE

		Kernel = POINTER TO RECORD(GraphGPprior.Kernel) END;
		
		FactoryKernel = POINTER TO RECORD(GraphScalar.Factory) END;

		Factory = POINTER TO RECORD(GraphMultivariate.Factory) 
							kernel: Kernel
						END;

	VAR
		factKernel-: GraphScalar.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (node: Kernel) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "SpatialMaternKrig.Install"
	END Install;

	PROCEDURE (node: Kernel) Element (x1, x2: GraphGPprior.Point; IN params: ARRAY OF REAL): REAL;
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

	PROCEDURE (f: FactoryKernel) New (): GraphGPprior.Kernel;
		VAR
			kernel: Kernel;
	BEGIN
		NEW(kernel);
		kernel.Init;
		RETURN kernel
	END New;

	PROCEDURE (f: FactoryKernel) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "s"
	END Signature;

	PROCEDURE (f: Factory) New (): GraphChain.Node;
		VAR
			node: GraphChain.Node;
			kernel: Kernel;
	BEGIN
		kernel := factKernel.New()(Kernel);
		node := GraphGPprior.New(kernel);
		RETURN node
	END New;

	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
		VAR
			i, numParams: INTEGER;
	BEGIN
		signature := "vvvs";
		i := 0;
		numParams := f.kernel.NumParams();
		WHILE i < numParams DO
			signature := signature + "s";
			INC(i)
		END
	END Signature;

	PROCEDURE InstallKernel*;
	BEGIN
		GraphNodes.SetFactory(factKernel)
	END InstallKernel;

	PROCEDURE Install2*;
		VAR
			fact: Factory;
	BEGIN
		NEW(fact);
		fact.kernel := factKernel.New()(Kernel);
		GraphNodes.SetFactory(fact)	
	END Install2;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			fKernel: FactoryKernel;
	BEGIN
		Maintainer;
		NEW(fKernel);
		factKernel := fKernel
	END Init;

BEGIN
	Init
END SpatialMaternKrig.
