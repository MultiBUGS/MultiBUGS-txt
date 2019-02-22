(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE StdCmds1;


	

	IMPORT
		Containers, Dialog, Files, StdApi,
		StdCmds,
		TextCmds,
		Views,
		Windows;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		PROCEDURE PathToFileSpec (IN path: ARRAY OF CHAR; VAR loc: Files.Locator;
	OUT name: Files.Name);
		VAR
			i, j: INTEGER;
			ch: CHAR;
	BEGIN
		i := 0;
		j := 0;
		IF loc = NIL THEN name := ""; RETURN END;
		WHILE (loc.res = 0) & (i < LEN(path) - 1) & (j < LEN(name) - 1) & (path[i] # 0X) DO
			ch := path[i];
			INC(i);
			IF (j > 0) & (ch = "/") THEN
				name[j] := 0X;
				IF name # "/" THEN
					j := 0;
					loc := loc.This(name);
					IF loc.res # 0 THEN RETURN END
				ELSE (*	network drive	*)
					name := "//";
					j := 2
				END
			ELSE
				name[j] := ch;
				INC(j)
			END
		END;
		IF path[i] = 0X THEN
			name[j] := 0X;
		ELSE
			loc.res := 1;
			name := ""
		END
	END PathToFileSpec;

	PROCEDURE OpenBrowser* (file, title: ARRAY OF CHAR);
		VAR
			c: Containers.Controller;
			v: Views.View;
			w: Windows.Window;
	BEGIN
		StdApi.OpenDoc(file, v);
		IF (v # NIL) & (v IS Containers.View) THEN
			c := v(Containers.View).ThisController();
			IF c # NIL THEN
				Views.BeginModification(Views.invisible, v);
				c.SetOpts(c.opts - {Containers.noFocus, Containers.noSelection} + {Containers.noCaret});
				Views.EndModification(Views.invisible, v);
				w := Windows.dir.First();
				WHILE (w # NIL) & (w.doc.ThisView() # v) DO
					w := Windows.dir.Next(w)
				END;
				IF w # NIL THEN
					w.SetTitle(title$)
				END
			END
		END
	END OpenBrowser;

	PROCEDURE OpenToolDialog* (file, title: ARRAY OF CHAR);
		VAR
			f: Files.File;
			loc: Files.Locator;
			name, linuxFile: Files.Name;
	BEGIN
		IF Dialog.platform = Dialog.linux THEN
			linuxFile := file + "Linux";
			PathToFileSpec(linuxFile,loc,name);
			f := Files.dir.Old(loc, name, TRUE);
			IF f # NIL THEN
				file := linuxFile$;
				f.Close;
			END
		END;
		StdCmds.OpenToolDialog(file, title)
	END OpenToolDialog;

	PROCEDURE Find* (pat: ARRAY OF CHAR);
	BEGIN
		TextCmds.find.find := pat$;
		TextCmds.FindFirst("")
	END Find;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END StdCmds1.

