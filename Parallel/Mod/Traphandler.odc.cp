(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE ParallelTraphandler;

	

	IMPORT
		Files, Kernel, Stores;

	VAR
		fileName: Files.Name;
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
			msg: ARRAY 256 OF CHAR;
			loc: Files.Locator;
			f: Files.File;
			wr: Stores.Writer;
			res: INTEGER;
	BEGIN
		GetTrapMsg(msg);
		loc := Files.dir.This("");
		f := Files.dir.New(loc, Files.dontAsk);
		wr.ConnectTo(f);
		wr.SetPos(0);
		wr.WriteString(msg);
		wr.ConnectTo(NIL);
		f.Close;
		f.Register(fileName$, "txt", Files.dontAsk, res)
	END TrapViewer;

	PROCEDURE SetTrapViewer* (IN fName: ARRAY OF CHAR);
	BEGIN
		fileName := fName$;
		Kernel.InstallTrapViewer(TrapViewer)
	END SetTrapViewer;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas";
	END Maintainer;

BEGIN
	Maintainer
END ParallelTraphandler.
