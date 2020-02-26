(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"


OSLinuxWindows

*)

MODULE Config;


	

	IMPORT
		Converters, Dialog, Files, Kernel, Meta, Strings
		OS,OleData;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		OS
	PROCEDURE ReadCommandLine (IN line: ARRAY OF CHAR; open: BOOLEAN);
		VAR name, opt: ARRAY 260 OF CHAR; i, l, t, r, b, res: INTEGER;
			ok: BOOLEAN; ln: ARRAY 260 OF CHAR;

		PROCEDURE CopyName;
			VAR ch, tch: CHAR; j: INTEGER;
		BEGIN
			j := 0; ch := line[i]; tch := " ";
			WHILE ch = " " DO INC(i); ch := line[i] END;
			IF (ch = "'") OR (ch = '"') THEN tch := ch; INC(i); ch := line[i] END;
			WHILE (ch >= " ") & (ch # tch) DO
				name[j] := ch;
				IF (ch >= "a") & (ch <= "z") OR (ch >= "à") & (ch <= "ö") OR (ch >= "ø") & (ch <= "þ") THEN ch := CAP(ch)
				ELSIF ch = "-" THEN ch := "/"
				END;
				opt[j] := ch; INC(j); INC(i); ch := line[i]
			END;
			IF ch > " " THEN INC(i); ch := line[i] END;
			WHILE (ch # 0X) & (ch <= " ") DO INC(i); ch := line[i] END;
			name[j] := 0X; opt[j] := 0X
		END CopyName;

	BEGIN
		l := 0; t := 0; r := 0; b := 0; i := 0;
		CopyName; 	(* skip program name *)
		WHILE line[i] > " " DO
			CopyName;
			IF opt = "/LOAD" THEN	(* load module *)
				CopyName; ln := name$;
				IF open THEN Kernel.LoadMod(ln) END
			ELSIF opt = "/USE" THEN	(* use directory *)
				CopyName	(* working directory: handled in HostFiles *)
			ELSIF opt = "/PT" THEN	(* print file to printer *)
				CopyName; CopyName; CopyName; CopyName	(* to be completed !!! *)
			ELSIF opt = "/LTRB" THEN	(* window position *)
				CopyName; ln := name$; Strings.StringToInt(ln, l, res);
				CopyName; ln := name$; Strings.StringToInt(ln, t, res);
				CopyName; ln := name$; Strings.StringToInt(ln, r, res);
				CopyName; ln := name$; Strings.StringToInt(ln, b, res)
			ELSIF opt = "/LANG" THEN
				CopyName; ln := name$;
				IF LEN(ln$) = 2 THEN
					Strings.ToLower(ln, ln);
					Dialog.SetLanguage(ln$, Dialog.nonPersistent)
				END
			ELSIF opt = "/O" THEN	(* open file *)
				CopyName; (*openUsed := TRUE;*)
				l := 0; t := 0; r := 0; b := 0
			ELSIF opt = "/PAR" THEN
				CopyName;
				Dialog.commandLinePars := name$
			ELSE	(* open file *)
				l := 0; t := 0; r := 0; b := 0
			END
		END
	END ReadCommandLine;
	(*	Windows	*)

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
			line: POINTER TO ARRAY OF CHAR;
			i, j, k, index: INTEGER;
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

		OS(*	linux	*)
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

		OSReadCommandLine(line, TRUE); (*	Windows	*)

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

