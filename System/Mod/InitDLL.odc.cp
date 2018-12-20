MODULE Init;

	IMPORT Kernel, Dialog, Converters;

	PROCEDURE Init;
		VAR res: INTEGER; m: Kernel.Module;
	BEGIN
		(*HostMenus.OpenApp;*)
		(*m := Kernel.ThisMod("DevDebug");
		IF m = NIL THEN Kernel.LoadMod("StdDebug") END;*)
		Converters.Register("Documents.ImportDocument", "Documents.ExportDocument", "", "odc", {});
		(*Dialog.Call("StdMenuTool.UpdateAllMenus", "", res);
		Kernel.LoadMod("OleServer")
		Dialog.Call("Config.Setup", "", res);*)
		Dialog.Call("Startup.Setup", "", res);
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
(*		HostMenus.Run*)
	END Init;

BEGIN
	Init
END Init.

