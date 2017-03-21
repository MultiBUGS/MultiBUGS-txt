(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsData;


	

	IMPORT
		Meta, BugsMappers;

	TYPE
		Loader* = POINTER TO ABSTRACT RECORD END;

		Factory* = POINTER TO ABSTRACT RECORD END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		fact: Factory;

	PROCEDURE (l: Loader) Data* (VAR s: BugsMappers.Scanner; OUT ok: BOOLEAN), NEW, ABSTRACT;

	PROCEDURE (l: Loader) Inits* (VAR s: BugsMappers.Scanner; OUT ok: BOOLEAN), NEW, ABSTRACT;

	PROCEDURE (f: Factory) New* (): Loader, NEW, ABSTRACT;

	PROCEDURE SetFactory* (f: Factory);
	BEGIN
		fact := f
	END SetFactory;

	PROCEDURE InstallFactory* (IN install: ARRAY OF CHAR): Factory;
		VAR
			ok: BOOLEAN;
			item: Meta.Item;
	BEGIN
		fact := NIL;
		Meta.LookupPath(install, item);
		IF item.obj = Meta.procObj THEN
			item.Call(ok)
		END;
		RETURN fact
	END InstallFactory;

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
END BugsData.
