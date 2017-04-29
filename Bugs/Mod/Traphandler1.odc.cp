(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	  *)

MODULE BugsTraphandler1;

	

	IMPORT
		Kernel;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE GetTrapMsg (OUT msg: ARRAY OF CHAR);
		VAR
			ref, end, a: INTEGER;
			mod: Kernel.Module;
			name: Kernel.Utf8Name;
	BEGIN
		msg := " ******";
		a := Kernel.pc; mod := Kernel.modList;
		WHILE (mod # NIL) & ((a < mod.code) OR (a >= mod.code + mod.csize)) DO mod := mod.next END;
		IF mod # NIL THEN
			DEC(a, mod.code); ref := mod.refs;
			REPEAT Kernel.GetRefProc(ref, end, name) UNTIL (end = 0) OR (a < end);
			IF a < end THEN
				msg := " in procedure " + name + " in module " + mod.name + " ******"
			END
		END
	END GetTrapMsg;

	PROCEDURE TrapViewer;
		VAR
			msg: ARRAY 128 OF CHAR;
	BEGIN
		GetTrapMsg(msg);
		(*BugsFiles.SetDest(BugsMappers.file);
		BugsFiles.ShowMsg("****** Sorry something went wrong" + msg)*)
	END TrapViewer;


	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas";
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		Kernel.InstallTrapViewer(TrapViewer)
	END Init;

BEGIN
	Init
END BugsTraphandler1.
