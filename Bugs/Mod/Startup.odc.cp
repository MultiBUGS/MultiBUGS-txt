(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	  *)

MODULE BugsStartup;

	

	IMPORT
		Kernel, Meta, Strings, 
		MathTT800;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Setup;
		VAR
			item: Meta.Item;
			ok: BOOLEAN;
			module: Kernel.Module;
			pos: INTEGER;
			procName: ARRAY 256 OF CHAR;
	BEGIN
		MathTT800.Install; 
		(*	initialize linked subsystem of loaded modules	*)
		module := Kernel.modList;
		WHILE module # NIL DO
			Strings.Find(module.name$, "Resources", 0, pos);
			IF (pos # - 1) & (module.refcnt >= 0) THEN
				procName := module.name + ".Load";
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
		Setup
	END Init;

BEGIN
	Init
END BugsStartup.

