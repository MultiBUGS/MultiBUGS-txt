(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	  *)

MODULE BugsTexts;


	

	IMPORT
		Containers, Dialog, Documents, Fonts, Ports, Views,
		StdLog,
		TextModels, TextRulers, TextViews,
		BugsMappers;

	TYPE

		Reader = POINTER TO RECORD(BugsMappers.Reader)
			textRd: TextModels.Reader
		END;

		Writer = POINTER TO RECORD(BugsMappers.Writer)
			textWr: TextModels.Writer
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (rd: Reader) ReadChar (OUT ch: CHAR);
	BEGIN
		IF ~rd.eot THEN
			rd.textRd.ReadChar(ch);
			rd.eot := rd.textRd.eot
		ELSE
			ch := 0X
		END
	END ReadChar;

	PROCEDURE (rd: Reader) Pos (): INTEGER;
	BEGIN
		RETURN rd.textRd.Pos()
	END Pos;

	PROCEDURE (rd: Reader) SetPos (pos: INTEGER);
	BEGIN
		rd.textRd.SetPos(pos);
		rd.eot := rd.textRd.eot
	END SetPos;

	PROCEDURE (wr: Writer) Bold;
		VAR
			textWr: TextModels.Writer;
			new, old: TextModels.Attributes;
	BEGIN
		textWr := wr.textWr;
		old := textWr.attr;
		IF old.font.weight = Fonts.bold THEN
			new := TextModels.NewWeight(old, Fonts.normal)
		ELSE
			new := TextModels.NewWeight(old, Fonts.bold)
		END;
		textWr.SetAttr(new)
	END Bold;

	PROCEDURE (wr: Writer) LineHeight (): INTEGER;
		VAR
			asc, dsc, w: INTEGER;
			font: Fonts.Font;
	BEGIN
		font := wr.textWr.attr.font;
		font.GetBounds(asc, dsc, w);
		RETURN asc + dsc
	END LineHeight;

	PROCEDURE (wr: Writer) Pos (): INTEGER;
	BEGIN
		RETURN wr.textWr.Pos()
	END Pos;

	PROCEDURE Open (v: Views.View; title: ARRAY OF CHAR; w, h: INTEGER);
		VAR
			d: Documents.Document;
			c: Containers.Controller;
	BEGIN
		d := Documents.dir.New(v, w, h);
		c := d.ThisController();
		c.SetOpts(c.opts + {Documents.winHeight} - {Documents.pageHeight}
		 + {Documents.winWidth} - {Documents.pageWidth});
		Views.OpenAux(d, title$)
	END Open;

	PROCEDURE (wr: Writer) Register (IN name: ARRAY OF CHAR; w, h: INTEGER);
		VAR
			text: TextModels.Model;
			v: TextViews.View;
	BEGIN
		text := wr.textWr.Base();
		IF BugsMappers.whereOut = BugsMappers.window THEN
			v := TextViews.dir.New(text);
			IF w = 0 THEN
				w := 100 * Ports.mm
			END;
			h := h + Ports.mm;
			Open(v, name, w, h)
		ELSE
			StdLog.String(name);
			StdLog.Ln;
			StdLog.text.Append(text)
		END
	END Register;

	PROCEDURE (wr: Writer) SetPos (pos: INTEGER);
	BEGIN
		wr.textWr.SetPos(pos)
	END SetPos;

	PROCEDURE (wr: Writer) StdRegister;
		VAR
			text: TextModels.Model;
	BEGIN
		text := wr.textWr.Base();
		StdLog.text.Append(text);
		StdLog.Open
	END StdRegister;

	PROCEDURE (wr: Writer) WriteChar (ch: CHAR);
	BEGIN
		wr.textWr.WriteChar(ch)
	END WriteChar;

	PROCEDURE (wr: Writer) WriteLn;
	BEGIN
		wr.textWr.WriteChar(TextModels.line)
	END WriteLn;

	PROCEDURE (wr: Writer) WriteRuler (IN tabs: ARRAY OF INTEGER);
		VAR
			i, len: INTEGER;
			ruler: TextRulers.Ruler;
	BEGIN
		ruler := TextRulers.dir.New(NIL);
		i := 0;
		len := LEN(tabs);
		WHILE i < len DO
			TextRulers.AddTab(ruler, tabs[i]);
			INC(i)
		END;
		TextRulers.SetRight(ruler, tabs[LEN(tabs) - 1]);
		wr.textWr.WriteView(ruler, 0, 0)
	END WriteRuler;

	PROCEDURE (wr: Writer) WriteString (IN string: ARRAY OF CHAR);
		VAR
			i: INTEGER;
	BEGIN
		i := 0;
		WHILE string[i] # 0X DO
			wr.textWr.WriteChar(string[i]);
			INC(i)
		END
	END WriteString;

	PROCEDURE (wr: Writer) WriteTab;
	BEGIN
		wr.textWr.WriteChar(TextModels.tab)
	END WriteTab;

	PROCEDURE (wr: Writer) WriteView (v: ANYPTR; w, h: INTEGER);
	BEGIN
		WITH v: Views.View DO
			wr.textWr.WriteView(v, w, h)
		ELSE
		END
	END WriteView;

	PROCEDURE ConnectScanner* (VAR s: BugsMappers.Scanner; text: TextModels.Model);
		VAR
			rd: Reader;
	BEGIN
		NEW(rd);
		rd.textRd := text.NewReader(NIL);
		s.SetReader(rd)
	END ConnectScanner;

	PROCEDURE ConnectFormatter* (VAR f: BugsMappers.Formatter; text: TextModels.Model);
		VAR
			wr: Writer;
			new, old: TextModels.Attributes;
	BEGIN
		NEW(wr);
		wr.textWr := text.NewWriter(NIL);
		old := wr.textWr.attr;
		new := TextModels.NewSize(old, 9 * Fonts.point);
		wr.textWr.SetAttr(new);
		wr.textWr.SetPos(0);
		f.SetWriter(wr)
	END ConnectFormatter;

	PROCEDURE ShowMsg* (s: ARRAY OF CHAR);
	BEGIN
		Dialog.ShowStatus(s);
		StdLog.Ln; StdLog.String(s)
	END ShowMsg;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END BugsTexts.

