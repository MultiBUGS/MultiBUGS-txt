MODULE Init;
(**
	project	= "BlackBox"
	organization	= "www.oberon.ch"
	contributors	= "Oberon microsystems"
	version	= "System/Rsrc/About"
	copyright	= "System/Rsrc/About"
	license	= "Docu/BB-License"
	changes	= "##=>

	- YYYYMMDD, nn, ...
	##<="
	issues	= "##=>

	- ...
	##<="

**)

	IMPORT Kernel, Dialog, Converters, HostMenus, Environment, HostFiles;

	PROCEDURE Init;
		VAR res: INTEGER; m: Kernel.Module;
	BEGIN
		IF Environment.RunningUnderMPI() THEN
			Dialog.appName := "MultiBUGS"
		ELSE
 			Dialog.appName := "OpenBUGS"
		END;
		HostMenus.OpenApp;
		HostFiles.IgnoreAsk;
		m := Kernel.ThisMod("DevDebug");
		IF m = NIL THEN Kernel.LoadMod("StdDebug") END;
		Converters.Register("Documents.ImportDocument", "Documents.ExportDocument", "", "odc", {});
		Dialog.Call("StdMenuTool.UpdateAllMenus", "", res);
		Kernel.LoadMod("OleServer");
		Dialog.Call("Config.Setup", "", res);
		HostMenus.Run
	END Init;

BEGIN
	Init
END Init.

