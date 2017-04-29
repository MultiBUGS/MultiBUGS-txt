(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	   *)

MODULE BugsVersion;


	

	IMPORT
		(*Fonts,*) Kernel, Meta, 
		TextMappers, TextModels;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	CONST
		bold = 700;
		
	PROCEDURE Version (IN module: ARRAY OF CHAR): INTEGER;
		VAR
			int: INTEGER;
			item, newItem: Meta.Item;
	BEGIN
		Meta.Lookup(module, item);
		ASSERT(item.obj = Meta.modObj, 21);
		item.Lookup("version", newItem);
		IF newItem.obj = Meta.varObj THEN
			int := newItem.IntVal()
		ELSE
			int := 0
		END;
		RETURN int
	END Version;

	PROCEDURE WriteInt (VAR f: TextMappers.Formatter; i: INTEGER);
	BEGIN
		IF i < 10 THEN
			f.WriteChar("0")
		END;
		f.WriteInt(i)
	END WriteInt;

	PROCEDURE ModuleMaintainer (IN module: ARRAY OF CHAR; OUT maintainer: ARRAY OF CHAR);
		VAR
			ok: BOOLEAN;
			item, newItem: Meta.Item;
	BEGIN
		Meta.Lookup(module, item);
		ASSERT(item.obj = Meta.modObj, 21);
		item.Lookup("maintainer", newItem);
		IF newItem.obj = Meta.varObj THEN
			newItem.GetStringVal(maintainer, ok);
			IF ~ok THEN
				maintainer := "unknown"
			END
		ELSE
			maintainer := "unknown"
		END
	END ModuleMaintainer;

	PROCEDURE Module (VAR f: TextMappers.Formatter; module: Kernel.Module);
		VAR
			version: INTEGER;
			maintainer: ARRAY 80 OF CHAR;
	BEGIN
		version := Version(module.name$);
		ModuleMaintainer(module.name$, maintainer);
		f.WriteTab;
		f.WriteString(module.name$);
		f.WriteTab;
		f.WriteInt(module.refcnt);
		f.WriteTab;
		IF version < 10 THEN
			f.WriteString("  ")
		ELSIF version < 100 THEN
			f.WriteChar(" ")
		END;
		f.WriteInt(version);
		f.WriteTab;
		f.WriteString(maintainer);
		f.WriteTab;
		WriteInt(f, module.compTime[2]);
		f.WriteChar("/");
		WriteInt(f, module.compTime[1]);
		f.WriteChar("/");
		WriteInt(f, module.compTime[0]);
		f.WriteTab;
		IF (module.loadTime[3] # 0) OR (module.loadTime[4] # 0) OR (module.loadTime[5] # 0) THEN
			WriteInt(f, module.loadTime[3]);
			f.WriteChar(":");
			WriteInt(f, module.loadTime[4]);
			f.WriteChar(":");
			WriteInt(f, module.loadTime[5])
		ELSE
			f.WriteString("linked")
		END;
		f.WriteLn
	END Module;

	PROCEDURE Modules* (VAR f: TextMappers.Formatter);
		VAR
			module: Kernel.Module;
			newAttr, oldAttr: TextModels.Attributes;
	BEGIN
		oldAttr := f.rider.attr;
		newAttr := TextModels.NewWeight(oldAttr, bold);
		f.rider.SetAttr(newAttr);
		f.WriteTab;
		f.WriteTab;
		f.WriteString("Clients");
		f.WriteTab;
		f.WriteString("Version");
		f.WriteTab;
		f.WriteString("Maintainer");
		f.WriteTab;
		f.WriteString("Compiled");
		f.WriteTab;
		f.WriteString("Loaded");
		f.WriteLn;
		f.rider.SetAttr(oldAttr);
		module := Kernel.modList;
		WHILE module # NIL DO
			IF module.refcnt >= 0 THEN Module(f, module) END;
			module := module.next
		END
	END Modules;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END BugsVersion.

