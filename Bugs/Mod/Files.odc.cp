(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsFiles;


	

	IMPORT
		Containers, Converters, Documents, Files, Fonts, Ports, Strings, Views,
		StdLog,
		TextMappers, TextModels, TextRulers, TextViews;

CONST
		file* = 2; (*	use file input/output	*)
		log* = 1; (*	use the log for output	*)
		window* = 0; (*	use window for output	*)

	VAR
		workingLoc-, tempLoc-: Files.Locator;
		prec-: INTEGER; (*	number of sig figures used in output	*)
		whereOut-: INTEGER; (*	where output is written to	*)
		conv: Converters.Converter;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		PROCEDURE PathToFileSpec (IN path: ARRAY OF CHAR; VAR loc: Files.Locator;
	OUT name: Files.Name);
		VAR
			i, j: INTEGER;
			ch: CHAR;
			fullPath: ARRAY 4096 OF CHAR;
	BEGIN
		i := 0;
		j := 0;
		IF loc = NIL THEN RETURN END;
		WHILE (loc.res = 0) & (i < LEN(fullPath) - 1) & (j < LEN(name) - 1) & (fullPath[i] # 0X) DO
			ch := fullPath[i];
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
		IF fullPath[i] = 0X THEN
			name[j] := 0X;
		ELSE
			loc.res := 1;
			name := ""
		END
	END PathToFileSpec;

	PROCEDURE OpenView (v: Views.View; title: ARRAY OF CHAR; w, h: INTEGER);
		VAR
			d: Documents.Document;
			c: Containers.Controller;
	BEGIN
		d := Documents.dir.New(v, w, h);
		c := d.ThisController();
		c.SetOpts(c.opts + {Documents.winHeight} - {Documents.pageHeight}
		 + {Documents.winWidth} - {Documents.pageWidth});
		Views.OpenAux(d, title$)
	END OpenView;

	PROCEDURE Open* (IN title: ARRAY OF CHAR; text: TextModels.Model);
		VAR
			v: TextViews.View;
			view: Views.View;
			attr: TextRulers.Attributes;
			s: TextMappers.Scanner;
			font: Fonts.Font;
			asc, dsc, width, num, w, h: INTEGER;
			loc: Files.Locator;
	BEGIN
		CASE whereOut OF
		|window:
			s.ConnectTo(text);
			s.SetPos(0);
			s.SetOpts({TextMappers.returnViews});
			s.Scan;
			IF s.view # NIL THEN
				view := s.view;
				WITH view: TextRulers.Ruler DO
					attr := view.style.attr;
					w := attr.right - attr.left;
					asc := attr.asc;
					dsc := attr.dsc;
					WHILE ~s.rider.eot DO s.Scan END;
					h := s.lines;
					h := MIN(h, 20);
					h := h * (asc + dsc) + Ports.mm
				ELSE
					w := s.w;
					h := s.h;
					num := 1;
					WHILE ~s.rider.eot DO 
						s.Scan;  
						IF s.type = TextMappers.view THEN INC(num) END
					END;
					IF w < 150 * Ports.mm THEN
						IF num > 1 THEN
							w := 2 * w;
						END;
						num := (num + 1) DIV 2;
					END;
					num := MIN(4, num);
					h := h * num
				END
			ELSE
				h := 0; w := 0
			END;
			v := TextViews.dir.New(text);
			OpenView(v, title, w, h)
		|log:
			StdLog.String(title);
			StdLog.Ln;
			StdLog.text.Append(text)
		|file:
			v := TextViews.dir.New(text);
			loc := tempLoc;
			Converters.Export(loc, "ResultsFile", conv, v)
		END
	END Open;
	
	PROCEDURE FileToText* (IN name: ARRAY OF CHAR): TextModels.Model;
		VAR
			text: TextModels.Model;
			v: Views.View;
			loc: Files.Locator;
			name0: Files.Name;
			pos: INTEGER;
	BEGIN
		loc := workingLoc;
		PathToFileSpec(name, loc, name0);
		Strings.Find(name0, ".txt", 0, pos);
		IF pos =  - 1 THEN
			v := Views.OldView(loc, name0)
		ELSE
			v := Views.Old(Views.dontAsk, loc, name0, conv)
		END;
		IF (v # NIL) & (v IS TextViews.View) THEN
			text := v(TextViews.View).ThisModel()
		ELSE
			text := NIL
		END;
		RETURN text
	END FileToText;
	
	PROCEDURE Save* (IN name: ARRAY OF CHAR; text: TextModels.Model);
		VAR
			v: TextViews.View;
			loc: Files.Locator;
			name0: Files.Name;
			pos: INTEGER;
	BEGIN
		v := TextViews.dir.New(text);
		loc := workingLoc;
		PathToFileSpec(name, loc, name0);
		Strings.Find(name0, ".txt", 0, pos);
		IF pos =  - 1 THEN
			Converters.Export(loc, name0, conv, NIL)
		ELSE
			Converters.Export(loc, name0, conv, v)
		END
	END Save;

	(*	sets where output will be written to	*)
	PROCEDURE SetDest* (dest: INTEGER);
	BEGIN
		whereOut := dest
	END SetDest;
	
	(*	sets number of significant figures used for outputting real numbers	*)
	PROCEDURE SetPrec* (precission: INTEGER);
	BEGIN
		prec := precission
	END SetPrec;

	PROCEDURE SetTempDir* (path: ARRAY OF CHAR);
		VAR
			name: Files.Name;
	BEGIN
		tempLoc := Files.dir.This("");
		PathToFileSpec(path, tempLoc, name);
	END SetTempDir;

	PROCEDURE SetWorkingDir* (path: ARRAY OF CHAR);
		VAR
			name: Files.Name;
	BEGIN
		workingLoc := Files.dir.This("");
		PathToFileSpec(path, workingLoc, name);
	END SetWorkingDir;

	PROCEDURE WriteRuler* (IN tabs: ARRAY OF INTEGER; VAR f: TextMappers.Formatter);
		VAR
			i, len, asc, dsc, w: INTEGER;
			ruler: TextRulers.Ruler;
			attr: TextModels.Attributes;
	BEGIN
		attr := f.rider.attr;
		attr := TextModels.NewSize(attr, Ports.point * 10);
		attr.font.GetBounds(asc, dsc, w);
		f.rider.SetAttr(attr);
		ruler := TextRulers.dir.New(NIL);
		TextRulers.SetAsc(ruler, asc);
		TextRulers.SetDsc(ruler, dsc);
		i := 0;
		len := LEN(tabs);
		WHILE i < len DO
			TextRulers.AddTab(ruler, tabs[i]);
			INC(i)
		END;
		TextRulers.SetRight(ruler, tabs[LEN(tabs) - 1]);
		f.WriteView(ruler);
	END WriteRuler;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		workingLoc := Files.dir.This("");
		tempLoc := Files.dir.This("");
		SetPrec(4);
		conv := Converters.list;
		WHILE (conv # NIL) & (conv.exp # "HostTextConv.ExportText") DO
			conv := conv.next
		END;
		whereOut := window
	END Init;

BEGIN
	Init
END BugsFiles.

