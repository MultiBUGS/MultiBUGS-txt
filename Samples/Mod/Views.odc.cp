(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	  *)

MODULE SamplesViews;


	

	IMPORT
		Views;

	TYPE
		Factory* = POINTER TO ABSTRACT RECORD END;

	VAR
		fact-: Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (f: Factory) New* (IN name: ARRAY OF CHAR;
	IN sample: ARRAY OF ARRAY OF REAL;
	start, step, firstChain, numChains: INTEGER): Views.View, NEW, ABSTRACT;

	PROCEDURE (f: Factory) Title* (OUT title: ARRAY OF CHAR), NEW, ABSTRACT;
		
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
		Maintainer;
		fact := NIL
	END Init;

BEGIN
	Init
END SamplesViews.
