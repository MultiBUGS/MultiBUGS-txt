(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)

MODULE SpatialDiscKrig;


	

	IMPORT
		Math,
		GraphChain, GraphGPprior, GraphMultivariate, GraphNodes;

	TYPE

		Factory2 = POINTER TO RECORD(GraphMultivariate.Factory) END;

		Kernel = POINTER TO RECORD(GraphGPprior.Kernel) END;

	VAR
		fact2-: GraphMultivariate.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		pi: REAL;
		kernel: Kernel;

	PROCEDURE (kernel: Kernel) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "SpatialDiscKrig.Install"
	END Install;

	PROCEDURE (kernel: Kernel) Element (x1, x2: GraphGPprior.Point; params: ARRAY OF REAL): REAL;
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

	PROCEDURE (f: Factory2) New (): GraphChain.Node;
		VAR
			node: GraphChain.Node;
	BEGIN
		node := GraphGPprior.New(kernel);
		RETURN node
	END New;

	PROCEDURE (f: Factory2) Signature (OUT signature: ARRAY OF CHAR);
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

	PROCEDURE Install2*;
	BEGIN
		GraphNodes.SetFactory(fact2)
	END Install2;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f2: Factory2;
	BEGIN
		Maintainer;
		NEW(f2);
		fact2 := f2;
		NEW(kernel);
		pi := Math.Pi()
	END Init;

BEGIN
	Init
END SpatialDiscKrig.
