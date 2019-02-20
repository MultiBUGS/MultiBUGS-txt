(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	 *)

MODULE Config;


	

	IMPORT
		Converters, Dialog, Files, Kernel, Meta, Strings(*,
		OleData*);

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Setup*;
		VAR
			loc, subLoc: Files.Locator;
			locInfo: Files.LocInfo;
			name: Files.Name;
			fileInfo: Files.FileInfo;
			item, modItem: Meta.Item;
			ok: BOOLEAN;
			module: Kernel.Module;
			pos: INTEGER;
			procName: ARRAY 256 OF CHAR;
	BEGIN
		Converters.Register("HostTextConv.ImportText", "HostTextConv.ExportText", "TextViews.View", "txt",
		{Converters.importAll});
		Converters.Register("HostTextConv.ImportRichText", "HostTextConv.ExportRichText",
		"TextViews.View", "rtf", {});
		Converters.Register("HostTextConv.ImportUnicode", "HostTextConv.ExportUnicode",
		"TextViews.View", "utf", {});
		Converters.Register("HostTextConv.ImportDosText", "", "TextViews.View", "txt", {});
		Converters.Register("HostTextConv.ImportHex", "", "TextViews.View", "dat", {Converters.importAll});
		Converters.Register("HostTextConv.ImportText", "HostTextConv.ExportText", "TextViews.View", "xml", {});
		Converters.Register("DevBrowser.ImportSymFile", "", "TextViews.View", "osf", {});
		Converters.Register("DevBrowser.ImportCodeFile", "", "TextViews.View", "ocf", {});

		Converters.Register("MapsViews1.ImportMapFile", "", "MapsView1.View", "map", {});
		Converters.Register("", "XhtmlExporter.ExportText", "TextViews.View", "html", {});

		Converters.Register("HostTextConv.ImportText", "HostTextConv.ExportText", "TextViews.View", "c", {});
		Converters.Register("HostTextConv.ImportText", "HostTextConv.ExportText", "TextViews.View", "h", {});
		Converters.Register("HostTextConv.ImportText", "HostTextConv.ExportText", "TextViews.View", "R", {});
		Converters.Register("HostTextConv.ImportText", "HostTextConv.ExportText", "TextViews.View", "tex", {});
(*
		OleData.Register("OleData.ImportInfo", "OleData.ExportInfo", "BlackBox Info", "", {OleData.info});
		OleData.Register("OleData.ImportNative", "OleData.ExportNative", "BlackBox Data", "", {});
		OleData.Register("", "HostTextConv.ExportDRichText", "Rich Text Format", "TextViews.View", {});
		OleData.Register("OleClient.ImportInfo", "OleClient.ExportInfo", "Object Descriptor", "", {OleData.info});
		OleData.Register("", "OleClient.Export", "Embedded Object", "OleClient.View", {OleData.storage});
		OleData.Register("OleClient.Import", "OleServer.Export", "Embed Source", "", {OleData.storage});
		OleData.Register("OleClient.Import", "", "Embedded Object", "", {OleData.storage});
		OleData.Register("HostTextConv.ImportDRichText", "", "Rich Text Format", "TextViews.View", {});
		OleData.Register("HostTextConv.ImportDUnicode", "HostTextConv.ExportDUnicode", "UNICODETEXT",
		"TextViews.View", {});
		OleData.Register("HostTextConv.ImportDText", "HostTextConv.ExportDText", "TEXT",
		"TextViews.View", {});
		OleData.Register("HostPictures.ImportDPict", "HostPictures.ExportDPict", "METAFILEPICT",
		"HostPictures.View", {});
		OleData.Register("", "OleData.ExportPicture", "METAFILEPICT", "", {});
*)
		Dialog.metricSystem := TRUE;

		(*	initialize subsystems in no particular order only used for unlinked version of BUGS	*)
		loc := Files.dir.This("");
		locInfo := Files.dir.LocList(loc);
		WHILE locInfo # NIL DO
			name := locInfo.name;
			subLoc := loc.This(name + "/Code");
			fileInfo := Files.dir.FileList(subLoc);
			WHILE (fileInfo # NIL) & (fileInfo.name # "Resources.ocf") DO
				fileInfo := fileInfo.next
			END;
			IF fileInfo # NIL THEN
				name := name + "Resources.Load";
				Meta.LookupPath(name, item);
				IF item.Valid() THEN
					item.Call(ok)
				END
			END;
			locInfo := locInfo.next;
		END;

		(* initialize subsystems for linked BUGS	*)
(*		module := Kernel.modList;
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
		END; *)
		
		IF Dialog.commandLinePars # "" THEN
			Meta.Lookup("BugsBatch", modItem)
		END;
		
		Dialog.ShowStatus(Dialog.appName)

	END Setup;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END Config.

