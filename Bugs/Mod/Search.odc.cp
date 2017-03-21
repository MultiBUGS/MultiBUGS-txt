MODULE BugsSearch;
(**
	project	= "BlackBox"
	organization	= "www.oberon.ch"
	contributors	= "Oberon microsystems"
	version	= "System/Rsrc/About"
	copyright	= "System/Rsrc/About"
	license	= "Docu/BB-License"
	changes	= "##=>

	- 20070307, bh, caseSens handling in Find corrected
	- 20150403, center #37, fixing traps with long search patterns in DevSearch
	- 20151201, center #88, support for localizable documentation
	##<="
	issues	= "##=>

	- ...
	##<="

**)

	IMPORT
		Kernel, Files, Fonts, Ports, Models, Views, Containers, Dialog, 
		TextModels, TextRulers, TextViews, TextMappers, TextCmds, StdLinks;

	CONST
		maxPat = LEN(TextCmds.find.find);
		noMatchFoundKey = "#Std:NoMatchFound";
		locationKey = "#Std:Location";
		countKey = "#Std:Count";
		searchingKey = "#Std:Searching";

	TYPE
		Pattern = ARRAY maxPat OF CHAR;
		Text = POINTER TO RECORD
			next: Text;
			num: INTEGER;
			title: Files.Name
		END;

	VAR
		w: TextMappers.Formatter;

	PROCEDURE Find (t: TextModels.Model; pat: Pattern; title: Files.Name; VAR list: Text);
		VAR r: TextModels.Reader; num: INTEGER; i, j, b, e, n: INTEGER; ch: CHAR; ref: Pattern; l: Text;
	BEGIN
		n := 0; num := 0;
		WHILE pat[n] # 0X DO
			INC(n)
		END;
		r := t.NewReader(NIL);
		r.SetPos(0); r.ReadChar(ch);
		WHILE ~r.eot DO
			ref[0] := ch; i := 0; j := 0; b := 0; e := 1;
			WHILE ~r.eot & (i < n) DO
				IF pat[i] = ch THEN
					INC(i); j := (j + 1) MOD maxPat
				ELSE
					i := 0; b := (b + 1) MOD maxPat; j := b
				END;
				IF j # e THEN ch := ref[j]
				ELSE r.ReadChar(ch); ref[j] := ch; e := (e + 1) MOD maxPat
				END
			END;
			IF i = n THEN INC(num) END
		END;
		IF num > 0 THEN
			NEW(l); l.num := num; l.title := title; l.next := list; list := l
		END
	END Find;
	
	PROCEDURE List (list: Text; pat: Pattern);
		VAR a0: TextModels.Attributes; this, t: Text; max: INTEGER;
			cmd: ARRAY maxPat + LEN(t.title) + 50  OF CHAR;
	BEGIN
		IF list = NIL THEN
			Dialog.MapString(noMatchFoundKey, cmd);
			w.WriteString(cmd)
		ELSE
			a0 := w.rider.attr;
			w.rider.SetAttr(TextModels.NewStyle(w.rider.attr, {Fonts.italic}));
			Dialog.MapString(locationKey, cmd);
			w.WriteString(cmd); w.WriteTab;
			Dialog.MapString(countKey, cmd);
			w.WriteString(cmd); w.WriteLn;
			w.rider.SetAttr(a0);
			REPEAT
				t := list; max := 1; this := NIL;
				WHILE t # NIL DO
					IF t.num >= max THEN max := t.num; this := t END;
					t := t.next
				END;
				IF this # NIL THEN
					w.rider.SetAttr(TextModels.NewStyle(w.rider.attr, {Fonts.underline}));
					w.rider.SetAttr(TextModels.NewColor(w.rider.attr, Ports.blue));
					cmd := "StdCmds.OpenBrowser('" + this.title + "', '" + this.title + "'); " +
								"BugsSearch.SelectCaseSens('" + pat + "')";
					w.WriteView(StdLinks.dir.NewLink(cmd));
					w.WriteString(this.title);
					w.WriteView(StdLinks.dir.NewLink(""));
					w.rider.SetAttr(a0);
					w.WriteTab; w.WriteInt(this.num); w.WriteLn;
					this.num := 0
				END
			UNTIL this = NIL
		END
	END List;

	PROCEDURE NewRuler (): TextRulers.Ruler;
		CONST mm = Ports.mm;
		VAR p: TextRulers.Prop;
	BEGIN
		NEW(p);
		p.valid := {TextRulers.right, TextRulers.tabs, TextRulers.opts};
		p.opts.val := {TextRulers.rightFixed}; p.opts.mask := p.opts.val;
		p.right := 100 * mm;
		p.tabs.len := 1;
		p.tabs.tab[0].stop := 70 * mm;
		RETURN TextRulers.dir.NewFromProp(p)
	END NewRuler;

	PROCEDURE ThisText (loc: Files.Locator; VAR name: Files.Name): TextModels.Model;
		VAR v: Views.View; m: Models.Model;
	BEGIN
		v := Views.OldView(loc, name);
		IF v # NIL THEN
			m := v.ThisModel();
			IF m # NIL THEN
				WITH m: TextModels.Model DO RETURN m ELSE END
			END
		END;
		RETURN NIL
	END ThisText;

	PROCEDURE GetTitle(IN pat: ARRAY OF CHAR; OUT title: Views.Title);
		VAR pos: INTEGER; i, j: INTEGER; ch: CHAR;
	BEGIN
		title := 'Search for "'; i := LEN(title$); j := 0; ch := pat[0];
		WHILE (ch # 0X) & (i < LEN(title) - 2) DO
			IF ch < " " THEN title[i] := " " ELSE title[i] := ch END ; (* replace tabs and line feeds by spaces *)
			INC(i); INC(j); ch := pat[j]
		END ;
		IF ch # 0X THEN title[i - 3] := "."; title[i - 2] := "."; title[i - 1] := "." END ;
		title[i] := '"'; title[i + 1] := 0X
	END GetTitle;

	PROCEDURE Search (IN locName: ARRAY OF CHAR);
		VAR pat: Pattern; t, log: TextModels.Model; v: Views.View; title: Views.Title; c: Containers.Controller;
			files: Files.FileInfo; dirs: Files.LocInfo;
			loc: Files.Locator; path, p: Files.Name; list: Text;
	BEGIN
		(*TextCmds.InitFindDialog; *)
		pat := TextCmds.find.find$;
		IF pat # "" THEN
			Dialog.ShowStatus(searchingKey);
			TextCmds.find.find := pat$;
			log := TextModels.dir.New();
			w.ConnectTo(log); w.SetPos(0);		
			(*IF source THEN loc := Files.dir.This("Mod"); path := "Mod"
			ELSE loc := Files.dir.This("Docu"); path := "Docu"
			END;*)
			loc := Files.dir.This(locName); 
			path := locName$;
			files := Files.dir.FileList(loc); list := NIL;
			WHILE files # NIL DO
				IF files.type = Kernel.docType THEN
					p := path + "/" + files.name;
					Find(ThisText(loc, files.name), pat, p, list)
				END;
				files := files.next
			END;
			loc := Files.dir.This("");
			dirs := Files.dir.LocList(loc);
			WHILE dirs # NIL DO
				loc := Files.dir.This(dirs.name); path := dirs.name + "/";
				(*IF source THEN loc := loc.This("Mod"); path := path + "Mod"
				ELSE loc := loc.This("Docu"); path := path +"Docu"
				END;*)
				loc := loc.This(locName); 
				path := path + locName;
				files := Files.dir.FileList(loc);
				WHILE files # NIL DO
					IF files.type = Kernel.docType THEN
						p := path + "/" + files.name;
						t := ThisText(loc, files.name);
						IF t # NIL THEN
							Find(t, pat, p, list)
						END
					END;
					files := files.next
				END;
				dirs := dirs.next
			END;
			List(list, pat);
			v := TextViews.dir.New(log);
			GetTitle(pat, title);
			v(TextViews.View).SetDefaults(NewRuler(), TextViews.dir.defAttr);
			Views.OpenAux(v, title);
			c := v(Containers.View).ThisController();
			c.SetOpts(c.opts + {Containers.noCaret});
			w.ConnectTo(NIL);
			Dialog.ShowStatus("")
		END
	END Search;

	PROCEDURE SearchInManuals*;
	BEGIN
		Search("Manuals")
	END SearchInManuals;

	PROCEDURE SearchInExamples*;
	BEGIN
		Search("Examples")
	END SearchInExamples;
	
	PROCEDURE SelectCaseSens* (pat: ARRAY OF CHAR);
	BEGIN
		TextCmds.find.find := pat$;
		TextCmds.find.ignoreCase := FALSE;
		Dialog.Update(TextCmds.find);
		TextCmds.FindFirst("")
	END SelectCaseSens;

END BugsSearch.

