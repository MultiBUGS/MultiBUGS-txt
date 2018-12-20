(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)

MODULE SpatialExpKrig;


	

	IMPORT
		Math,
		GraphChain, GraphGPprior, GraphMultivariate, GraphNodes, GraphStochastic;

	TYPE

		Factory = POINTER TO ABSTRACT RECORD(GraphMultivariate.Factory) END;

		Factory1 = POINTER TO RECORD(Factory) END;

		Factory2 = POINTER TO RECORD(Factory) END;

		Kernel = POINTER TO RECORD(GraphGPprior.Kernel) END;

	VAR
		fact1-, fact2-: GraphMultivariate.Factory;
		maintainer-: ARRAY 40 OF CHAR;
		version-: INTEGER;
		kernel: Kernel;

	PROCEDURE (kernel: Kernel) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "SpatialExpKrig.Install"
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
		dist := Math.Sqrt(dist);
		element := Math.Exp( - Math.Power(params[0] * dist, params[1]));
		RETURN element
	END Element;

	PROCEDURE (kernel: Kernel) NumParams (): INTEGER;
	BEGIN
		RETURN 2
	END NumParams;

	PROCEDURE (f: Factory) New (): GraphChain.Node;
		VAR
			node: GraphChain.Node;
	BEGIN
		node := GraphGPprior.New(kernel);
		RETURN node
	END New;

	PROCEDURE (f: Factory1) Signature (OUT signature: ARRAY OF CHAR);
		VAR
			i, numParams: INTEGER;
	BEGIN
		signature := "vvs"; 	(*	mu[], x[], tau	*)
		i := 0;
		numParams := kernel.NumParams();
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
		numParams := kernel.NumParams();
		WHILE i < numParams DO
			signature := signature + "s";
			INC(i)
		END
	END Signature;

	PROCEDURE Install1*;
	BEGIN
		GraphNodes.SetFactory(fact1)
	END Install1;

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
			f1: Factory1;
			f2: Factory2;
	BEGIN
		Maintainer;
		NEW(f1);
		fact1 := f1;
		NEW(f2);
		fact2 := f2;
		NEW(kernel)
	END Init;

BEGIN
	Init
END SpatialExpKrig.
