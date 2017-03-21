(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	  *)

(*	Must have BugsStdInterpreter in IMPORT list to initailize interpreter	*)

MODULE Startup;

	

	IMPORT
		Kernel, Meta, Strings,
		BugsStdInterpreter, (*	needed for side effect	*)
		MathTT800;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		setup: BOOLEAN;

	PROCEDURE Setup*;
		VAR
			item: Meta.Item;
			ok: BOOLEAN;
			module: Kernel.Module;
			pos: INTEGER;
			nameM: Kernel.Utf8Name;
			procName: ARRAY 256 OF CHAR;
	BEGIN
		IF setup THEN RETURN END;
		setup := TRUE;
		MathTT800.Install;
		(*	initialize linked subsystem of loaded modules	*)
		module := Kernel.modList;
		WHILE module # NIL DO
			nameM := module.name;
			Strings.Find(LONG(nameM), "Resources", 0, pos);
			IF (pos # - 1) & (module.refcnt >= 0) THEN
				procName := nameM + ".Load";
				Meta.LookupPath(procName, item);
				IF item.obj = Meta.procObj THEN
					item.Call(ok)
				END
			END;
			module := module.next
		END;
	END Setup;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		setup := FALSE
	END Init;

BEGIN
	Init
END Startup.

