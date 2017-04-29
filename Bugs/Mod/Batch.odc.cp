(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	 *)

MODULE BugsBatch;

	(*	Runs a BUGS script in batch mode, output is written to the log window, the scipt is in a file with the name of the first parameter on the command line, see the short cut BackBugs. This module exports no procedures etc, the module just executes its initialization code on loading	*)

	

	IMPORT
		Dialog,
		StdLog,
		BugsCmds, BugsFiles;

	VAR
		version-: INTEGER; (*	version number	*)
		maintainer-: ARRAY 40 OF CHAR; (*	person maintaining module	*)

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		BugsFiles.SetDest(BugsFiles.log);
		StdLog.Open;
		BugsCmds.ScriptFile(Dialog.commandLinePars)
	END Init;

BEGIN
	Init
END BugsBatch.
