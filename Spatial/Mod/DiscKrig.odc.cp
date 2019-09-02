(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)

MODULE SpatialDiscKrig;


	

	IMPORT
		Math,
		GraphChain, GraphGPprior, GraphMultivariate, GraphNodes, GraphScalar;

	TYPE

		Kernel = POINTER TO RECORD(GraphGPprior.Kernel) END;

		FactoryKernel = POINTER TO RECORD(GraphScalar.Factory) END;
		
		Factory2 = POINTER TO RECORD(GraphMultivariate.Factory) 
							kernel: Kernel
						 END;

	VAR
		factKernel-: GraphScalar.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		pi: REAL;

	PROCEDURE (node: Kernel) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "SpatialDiscKrig.InstallKernel"
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
		dist := Math.Sqrt(dist) / params[0];
		IF dist < 1 THEN
			element := 2 * (Math.ArcCos(dist) - dist * Math.Sqrt(1 - dist * dist)) / pi
		ELSE
			element := 0
		END;
		RETURN element
	END Element;

	PROCEDURE (kernel: Kernel) NumParams (): INTEGER;
	BEGIN
		RETURN 1
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
	
	PROCEDURE (f: Factory2) New (): GraphChain.Node;
		VAR
			node: GraphChain.Node;
			kernel: Kernel;
	BEGIN
		kernel := factKernel.New()(Kernel);
		node := GraphGPprior.New(kernel);
		RETURN node
	END New;

	PROCEDURE (f: Factory2) Signature (OUT signature: ARRAY OF CHAR);
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
			f2: Factory2;
			fKernel: FactoryKernel;
	BEGIN
		Maintainer;
		NEW(fKernel);
		factKernel := fKernel;
		pi := Math.Pi()
	END Init;

BEGIN
	Init
END SpatialDiscKrig.
