(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphLimits;

	

	IMPORT
		Stores,
		GraphNodes, GraphStochastic;

	CONST
		non* = 0;
		left* = 1;
		right* = 2;
		both* = 3;

	TYPE
		Limits* = POINTER TO ABSTRACT RECORD END;

		Factory* = POINTER TO ABSTRACT RECORD END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		fact-: Factory;


		(*  Abstract methods for the Limits base class, required to be implemented
		by all concrete Limits classes. *)

	PROCEDURE (limits: Limits) Bounds* (OUT left, right: REAL), NEW, ABSTRACT;

	PROCEDURE (limits: Limits) Externalize* (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (limits: Limits) Init* (), NEW, ABSTRACT;

	PROCEDURE (limits: Limits) Internalize* (VAR rd: Stores.Reader), NEW, ABSTRACT;

		PROCEDURE (limits: Limits) NormalizingConstant* (node: GraphStochastic.Node): REAL,
	NEW, ABSTRACT;

	PROCEDURE (limits: Limits) Parents* (OUT left, right: GraphNodes.Node), NEW, ABSTRACT;

	PROCEDURE (limits: Limits) Set* (Ieft, right: GraphNodes.Node), NEW, ABSTRACT;

	PROCEDURE (limits: Limits) TransformParameters* (): Limits, NEW, ABSTRACT;

	PROCEDURE (limits: Limits) Type* (): INTEGER, NEW, ABSTRACT;

		(* methods for factory *)
	PROCEDURE (factory: Factory) New* (option: INTEGER): Limits, NEW, ABSTRACT;

	PROCEDURE SetFactory* (f: Factory);
	BEGIN
		fact := f
	END SetFactory;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		fact := NIL;
		Maintainer
	END Init;

BEGIN
	Init
END GraphLimits. 
