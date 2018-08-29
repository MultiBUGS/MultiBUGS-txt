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

	

	IMPORT Converters, Dialog, HostMenus, Kernel, WinApi;


		(* From https://community.blackboxframework.org/viewtopic.php?f=16&t=102&p=520&hilit=environment#p520 *)
	PROCEDURE EnvVarValue (IN pName: ARRAY OF CHAR): POINTER TO ARRAY OF CHAR;
		(* Returns the value of the environment variable named pName, e.g. EnvVarValue('HOMEPATH') *)
		VAR arrOut: POINTER TO ARRAY OF CHAR;
			envVarName, envVarValue: WinApi.PtrWSTR;
			lenValue: INTEGER;
	BEGIN
		envVarName := pName;
		NEW(arrOut, 80); envVarValue := arrOut^;
		lenValue := WinApi.GetEnvironmentVariableW(envVarName, envVarValue, LEN(arrOut));
		IF lenValue = 0 THEN
			(* There is no such environment variable; or error *)
			envVarValue := ""
		ELSIF lenValue > LEN(arrOut) THEN
			(* The out array is not large enough to store the value of the environment variable. Now, the out array will be allocated with the exact size. *)
			NEW(arrOut, lenValue); envVarValue := arrOut^;
			lenValue := WinApi.GetEnvironmentVariableW(envVarName, envVarValue, LEN(arrOut));
			ASSERT(lenValue <= LEN(arrOut), 60)
		END;
		arrOut^ := envVarValue$;
		RETURN arrOut;
	END EnvVarValue;

	PROCEDURE Init;
		VAR res: INTEGER; m: Kernel.Module;
			sizeMPI: POINTER TO ARRAY OF CHAR;
	BEGIN
		sizeMPI := EnvVarValue("PMI_SIZE");
		IF sizeMPI$ = "" THEN
			(* Not run under MPI *)
			Dialog.appName := "OpenBUGS"
		ELSE
			(* Is run under MPI *)
			Dialog.appName := "MultiBUGS"
		END;
		HostMenus.OpenApp;
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

