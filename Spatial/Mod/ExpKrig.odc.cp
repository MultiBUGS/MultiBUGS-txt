(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)

MODULE SpatialExpKrig;


	

	IMPORT
		Math,
		GraphChain, GraphGPprior, GraphMultivariate, GraphNodes, GraphScalar, GraphStochastic;

	TYPE

		Kernel = POINTER TO RECORD(GraphGPprior.Kernel) END;

		FactoryKernel = POINTER TO RECORD(GraphScalar.Factory) END;

		Factory = POINTER TO ABSTRACT RECORD(GraphMultivariate.Factory) 
							kernel: Kernel
					   END;

		Factory1 = POINTER TO RECORD(Factory)  END;

		Factory2 = POINTER TO RECORD(Factory) END;

	VAR
		factKernel-: GraphScalar.Factory;
		maintainer-: ARRAY 40 OF CHAR;
		version-: INTEGER;

	PROCEDURE (node: Kernel) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "SpatialExpKrig.InstallKernel"
	END Install;

	PROCEDURE (node: Kernel) Element (x1, x2: GraphGPprior.Point; IN params: ARRAY OF REAL): REAL;
		VAR
			dim, i: INTEGER;
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
		element := Math.Exp( - Math.Power(params[0] * dist, params[1]));
		RETURN element
	END Element;

	PROCEDURE (node: Kernel) NumParams (): INTEGER;
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

	PROCEDURE (f: Factory1) Signature (OUT signature: ARRAY OF CHAR);
		VAR
			i, numParams: INTEGER;
	BEGIN
		signature := "vvs"; 	(*	mu[], x[], tau	*)
		i := 0;
		numParams := f.kernel.NumParams();
		WHILE i < numParams DO
			signature := signature + "s";
			INC(i)
		END
	END Signature;

	PROCEDURE (f: Factory2) Signature (OUT signature: ARRAY OF CHAR);
		VAR
			i, numParams: INTEGER;
	BEGIN
		signature := "vvvs"; 	(*	mu[], x[], y[], tau	*)
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

	PROCEDURE Install1*;
		VAR
			fact: Factory1;
	BEGIN
		NEW(fact);
		fact.kernel := factKernel.New()(Kernel);
		GraphNodes.SetFactory(fact)
	END Install1;

	PROCEDURE Install2*;
		VAR
			fact: Factory2;
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
END SpatialExpKrig.
