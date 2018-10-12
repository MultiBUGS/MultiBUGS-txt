(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	 *)

MODULE MathFunctional;

	TYPE

		Function* = POINTER TO ABSTRACT RECORD END;

		Functional* = POINTER TO ABSTRACT RECORD
			function-: Function
		END;

		Factory* = POINTER TO ABSTRACT RECORD END;

	VAR
		fact-: Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (f: Function) Value* (x: REAL; IN theta: ARRAY OF REAL): REAL, NEW, ABSTRACT;

	PROCEDURE (f: Functional) Init* (function: Function), NEW;
	BEGIN
		f.function := function
	END Init;

	PROCEDURE (f: Functional) Install* (OUT install: ARRAY OF CHAR), NEW, ABSTRACT;


	PROCEDURE (f: Functional) Value* (x0, x1, tol: REAL; IN theta: ARRAY OF REAL): REAL, NEW, ABSTRACT;

	PROCEDURE (f: Factory) New* (): Functional, NEW, ABSTRACT;

	PROCEDURE SetFactory* (f: Factory);
	BEGIN
		fact := f
	END SetFactory;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END MathFunctional.
