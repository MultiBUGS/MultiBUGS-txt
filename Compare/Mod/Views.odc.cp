(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE CompareViews;


	

	IMPORT
		PlotsViews;

	CONST
		node* = 0;
		other* = 1;
		axis* = 2;

	TYPE
		Factory* = POINTER TO ABSTRACT RECORD
			fractions-: POINTER TO ARRAY OF REAL;
			arguments-: SET
		END;

	VAR
		fact-: Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (f: Factory) New* (IN name: ARRAY OF CHAR;
	IN label: ARRAY OF ARRAY OF CHAR;
	IN mean: ARRAY OF REAL;
	IN percentiles: ARRAY OF ARRAY OF REAL;
	IN start, sampleSize: ARRAY OF INTEGER;
	other, axis: POINTER TO ARRAY OF REAL): PlotsViews.View,
	NEW, ABSTRACT;

	PROCEDURE (f: Factory) SetFractions* (fractions: POINTER TO ARRAY OF REAL), NEW;
	BEGIN
		f.fractions := fractions
	END SetFractions;

	PROCEDURE (f: Factory) SetArguments* (arguments: SET), NEW;
	BEGIN
		f.arguments := arguments
	END SetArguments;

	PROCEDURE (f: Factory) StoreSample* (sample: POINTER TO ARRAY OF REAL), NEW, EMPTY;
	
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
END CompareViews.
