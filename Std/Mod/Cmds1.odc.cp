(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	 *)

MODULE StdCmds1;


	

	IMPORT
		Containers, Views, Windows,
		StdApi,
		TextCmds;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

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

