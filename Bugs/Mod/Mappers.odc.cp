(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsMappers;

	(*	formatted input and output	*)
	

	IMPORT
		Strings;

	CONST

		char* = 0; (*	scaned token was character	*)
		string* = 1; (*	scanned token was a string	*)
		function* = 2; (*	scanned token was a function	*)
		int* = 3; (*	scanned token was an integer	*)
		real* = 4; (*	scanned token was a real	*)
		inval* = 31; (*	scanned token was invalid	*)

		file* = 2; (*	use file input/output	*)
		log* = 1; (*	use the log for output	*)
		window* = 0; (*	use window for output	*)

		tab = 09X;
		line = 0DX;
		para = 0EX;
		lf = 0AX;

		stringLen = 1024;

	TYPE
		(*	class for reading characters from a stream	*)
		Reader* = POINTER TO ABSTRACT RECORD
			eot*: BOOLEAN (*	true if end of input strean reached	*)
		END;

		(*	class for scanning tokens	*)
		Scanner* = RECORD
			char*: CHAR; (*	value of character scanned	*)
			nextCh-: CHAR; (*	character after last scanned token	*)
			real*: REAL; (*	value of real scanned	*)
			int*: INTEGER; (*	value of integer scanned	*)
			type*: INTEGER; (*	type of token scanned	*)
			len*: INTEGER; (*	length of scanned token	*)
			linesRead-: INTEGER; (*	number of lines read	*)
			string*: ARRAY 1024 OF CHAR; (*	value of string scanned	*)
			eot-: BOOLEAN; (*	true if end of input stream has been reached	*)
			rd-: Reader
		END;

		(*	class for writing characters to a stream	*)
		Writer* = POINTER TO ABSTRACT RECORD END;

		(*	class for writing formatted output	*)
		Formatter* = RECORD
			lines-: INTEGER; (*	number of lines written	*)
			views-: INTEGER; (*	number of views written	*)
			width-: INTEGER; (*	width of output stream	*)
			viewW-: INTEGER; (*	width of view written	*)
			viewH-: INTEGER; (*	height of view written	*)
			wr: Writer
		END;

	VAR
		version-: INTEGER; (*	version number	*)
		maintainer-: ARRAY 40 OF CHAR; (*	person maintaining module	*)
		prec-: INTEGER; (*	number of sig figures used in output	*)
		whereOut-: INTEGER; (*	where output is written to	*)

	(*	reads a character from input stream	*)
	PROCEDURE (rd: Reader) ReadChar- (OUT ch: CHAR), NEW, ABSTRACT;

	(*	position in input stream	*)
	PROCEDURE (rd: Reader) Pos- (): INTEGER, NEW, ABSTRACT;

	(*	get position in input stream	*)
	PROCEDURE (rd: Reader) SetPos- (pos: INTEGER), NEW, ABSTRACT;

	(*	sets the input stream used by scanner	*)
	PROCEDURE (VAR s: Scanner) SetReader* (rd: Reader), NEW;
	BEGIN
		s.rd := rd
	END SetReader;

	PROCEDURE (VAR s: Scanner) ReadChar (VAR ch: CHAR), NEW;
	BEGIN
		ASSERT(s.rd # NIL, 20);
		s.rd.ReadChar(ch);
		s.eot := s.rd.eot
	END ReadChar;

	(*	position of scanner on input stream	*)
	PROCEDURE (VAR s: Scanner) Pos* (): INTEGER, NEW;
	BEGIN
		RETURN s.rd.Pos() - 1
	END Pos;

	(*	scans a token from input stream	*)
	PROCEDURE (VAR s: Scanner) Scan*, NEW;
		VAR
			ch, quoteChar: CHAR;
			l, res: INTEGER;

		PROCEDURE ReadFractionPart;
		BEGIN
			WHILE (ch >= "0") & (ch <= "9") DO
				INC(l); s.string[l] := ch; s.ReadChar(ch)
			END;
			IF CAP(ch) = "E" THEN
				INC(l);
				s.string[l] := ch;
				s.ReadChar(ch);
				IF (ch = "+") OR (ch = "-") THEN
					INC(l); s.string[l] := ch; s.ReadChar(ch)
				END;
				IF (ch < "0") OR (ch > "9") THEN
					s.type := inval; s.nextCh := ch; RETURN
				END;
				INC(l);
				s.string[l] := ch;
				s.ReadChar(ch);
				IF (ch >= "0") & (ch <= "9") THEN
					INC(l); s.string[l] := ch; s.ReadChar(ch)
				END
			END;
			INC(l);
			s.len := l;
			s.string[l] := 0X
		END ReadFractionPart;

	BEGIN
		ch := s.nextCh;
		LOOP
			(*	strip out white spaces	*)
			CASE ch OF
			|line: INC(s.linesRead); s.ReadChar(ch); IF ch = lf THEN s.ReadChar(ch) END;
			|lf: INC(s.linesRead); s.ReadChar(ch)
			|" ", tab, para: s.ReadChar(ch)
			|"#": REPEAT s.ReadChar(ch) UNTIL (ch = line) OR (ch = lf) OR s.eot
			ELSE EXIT END
		END;
		CASE ch OF
		|"A".."Z", "a".."z", "α" .. "ρ", "σ".."ω", "Α" .. "Ρ", "Σ" .. "Ω":
			l := 0;
			s.string[l] := ch;
			INC(l);
			s.ReadChar(ch);
			WHILE (((CAP(ch) >= "A") & (CAP(ch) <= "Z"))
				OR ((ch >= "0") & (ch <= "9"))
				OR ((ch >= "α") & (ch <= "ρ"))
				OR ((ch >= "σ") & (ch <= "ω"))
				OR ((ch >= "Α") & (ch <= "Ρ"))
				OR ((ch >= "Σ") & (ch <= "Ω"))
				OR (ch = ".") OR (ch = "_")) DO
				s.string[l] := ch;
				INC(l);
				s.ReadChar(ch)
			END;
			s.string[l] := 0X;
			s.len := l;
			s.type := string;
			IF ch = "(" THEN
				s.type := function
			END
		|"'", '"':
			quoteChar := ch;
			l := 0;
			s.ReadChar(ch);
			WHILE (ch # quoteChar) & (ch # line) & ~s.eot & (l < stringLen) DO
				s.string[l] := ch; INC(l); s.ReadChar(ch)
			END;
			IF (ch = line) OR s.eot OR (l = stringLen) THEN
				s.type := inval
			ELSE
				s.string[l] := 0X;
				s.len := l;
				s.type := string;
				s.ReadChar(ch);
			END
		|"0".."9":
			l := 0;
			s.string[l] := ch;
			s.ReadChar(ch);
			WHILE (ch >= "0") & (ch <= "9") DO
				INC(l); s.string[l] := ch; s.ReadChar(ch)
			END;
			IF ch = "." THEN
				s.type := real;
				INC(l);
				s.string[l] := ch;
				s.ReadChar(ch);
				ReadFractionPart;
				Strings.StringToReal(s.string, s.real, res);
				IF res # 0 THEN s.type := inval END
			ELSE
				s.type := int;
				INC(l);
				s.len := l;
				s.string[l] := 0X;
				Strings.StringToInt(s.string, s.int, res);
				IF res # 0 THEN s.type := inval END
			END
		|".":
			l := 0;
			s.string[l] := ".";
			s.ReadChar(ch);
			IF (ch >= "0") & (ch <= "9") THEN
				s.type := real;
				ReadFractionPart;
				s.string := "0" + s.string;
				Strings.StringToReal(s.string, s.real, res);
				IF res # 0 THEN s.type := inval END
			ELSE
				s.type := char;
				s.char := "."
			END
		ELSE
			s.type := char;
			s.char := ch;
			s.ReadChar(ch)
		END;
		s.nextCh := ch;
		s.eot := s.rd.eot
	END Scan;

	(*	sets position of scanner on input stream	*)
	PROCEDURE (VAR s: Scanner) SetPos* (pos: INTEGER), NEW;
	BEGIN
		s.rd.eot := FALSE;
		s.rd.SetPos(pos);
		s.ReadChar(s.nextCh);
		s.linesRead := 0;
		s.eot := s.rd.eot
	END SetPos;

	(*	make subsiquent output be in bold type	*)
	PROCEDURE (wr: Writer) Bold-, NEW, ABSTRACT;

		(*	height of a line of text	*)
	PROCEDURE (wr: Writer) LineHeight- (): INTEGER, NEW, ABSTRACT;

		(*	position in the output stream	*)
	PROCEDURE (wr: Writer) Pos- (): INTEGER, NEW, ABSTRACT;

		(*	registers the output stream	*)
	PROCEDURE (wr: Writer) Register- (IN name: ARRAY OF CHAR; w, h: INTEGER), NEW, ABSTRACT;

		(*	sets position in output stream	*)
	PROCEDURE (wr: Writer) SetPos- (pos: INTEGER), NEW, ABSTRACT;

		(*	registers the out put stream in a standard way	*)
	PROCEDURE (wr: Writer) StdRegister-, NEW, ABSTRACT;

		(*	writes a character to output stream	*)
	PROCEDURE (wr: Writer) WriteChar- (ch: CHAR), NEW, ABSTRACT;

		(*	writes a new line to output stream	*)
	PROCEDURE (wr: Writer) WriteLn-, NEW, ABSTRACT;

		(*	writes a ruler (tab spacing control) to output stream	*)
	PROCEDURE (wr: Writer) WriteRuler- (IN tabs: ARRAY OF INTEGER), NEW, ABSTRACT;

		(*	writes a string to output stream	*)
	PROCEDURE (wr: Writer) WriteString- (IN string: ARRAY OF CHAR), NEW, ABSTRACT;

		(*	writes a tab to out put stream (depends on ruler)	*)
	PROCEDURE (wr: Writer) WriteTab-, NEW, ABSTRACT;

		(*	writes a view to output stream	*)
	PROCEDURE (wr: Writer) WriteView- (v: ANYPTR; w, h: INTEGER), NEW, ABSTRACT;

		(*	make subsiquent output be in bold type	*)
	PROCEDURE (VAR f: Formatter) Bold*, NEW;
	BEGIN
		f.wr.Bold
	END Bold;

	(*	height of a line of text	*)
	PROCEDURE (VAR f: Formatter) LineHeight* (): INTEGER, NEW;
	BEGIN
		RETURN f.wr.LineHeight()
	END LineHeight;

	(*	position in the output stream	*)
	PROCEDURE (VAR f: Formatter) Pos* (): INTEGER, NEW;
	BEGIN
		RETURN f.wr.Pos()
	END Pos;

	(*	registers the output stream	*)
	PROCEDURE (VAR f: Formatter) Register* (IN name: ARRAY OF CHAR), NEW;
		VAR
			w, h: INTEGER;
	BEGIN
		w := f.width;
		IF f.lines # 0 THEN
			h := f.lines;
			h := MIN(h, 25);
			h := h * f.LineHeight();
			f.wr.Register(name, w, h)
		ELSIF f.views > 0 THEN
			h := f.views;
			IF h = 1 THEN
				h := f.viewH;
				w := f.viewW
			ELSE
				h := (h + 1) DIV 2;
				h := MIN(4, h);
				h := h * f.viewH;
				w := 2 * f.viewW
			END;
			f.wr.Register(name, w, h)
		END
	END Register;

	(*	sets position in output stream	*)
	PROCEDURE (VAR f: Formatter) SetPos* (pos: INTEGER), NEW;
	BEGIN
		f.wr.SetPos(pos)
	END SetPos;

	(*	sets output stream for output	*)
	PROCEDURE (VAR f: Formatter) SetWriter* (wr: Writer), NEW;
	BEGIN
		f.lines := 0;
		f.views := 0;
		f.width := 0;
		f.viewW := 0;
		f.viewH := 0;
		f.wr := wr
	END SetWriter;

	(*	registers the out put stream in a standard way	*)
	PROCEDURE (VAR f: Formatter) StdRegister*, NEW;
	BEGIN
		f.wr.StdRegister
	END StdRegister;

	(*	writes a character to output stream	*)
	PROCEDURE (VAR f: Formatter) WriteChar* (ch: CHAR), NEW;
	BEGIN
		f.wr.WriteChar(ch)
	END WriteChar;

	(*	writes an integer to output stream	*)
	PROCEDURE (VAR f: Formatter) WriteInt* (x: INTEGER), NEW;
		VAR
			str: ARRAY 80 OF CHAR;
	BEGIN
		Strings.IntToString(x, str);
		f.wr.WriteString(str)
	END WriteInt;

	(*	writes a new line to output stream	*)
	PROCEDURE (VAR f: Formatter) WriteLn*, NEW;
	BEGIN
		INC(f.lines);
		f.wr.WriteLn
	END WriteLn;

	(*	writes a real to output stream	*)
	PROCEDURE (VAR f: Formatter) WriteReal* (x: REAL), NEW;
		VAR
			str: ARRAY 80 OF CHAR;
	BEGIN
		Strings.RealToStringForm(x, prec, 0, 0, " ", str);
		f.wr.WriteString(str)
	END WriteReal;

	(*	writes a string to output stream	*)
	PROCEDURE (VAR f: Formatter) WriteString* (IN str: ARRAY OF CHAR), NEW;
	BEGIN
		f.wr.WriteString(str)
	END WriteString;

	(*	writes a ruler (tab spacing control) to output stream	*)
	PROCEDURE (VAR f: Formatter) WriteRuler* (IN tabs: ARRAY OF INTEGER), NEW;
		VAR
			last: INTEGER;
	BEGIN
		f.wr.WriteRuler(tabs);
		last := LEN(tabs) - 1;
		f.width := MAX(f.width, tabs[last])
	END WriteRuler;

	(*	writes a tab to out put stream (depends on ruler)	*)
	PROCEDURE (VAR f: Formatter) WriteTab*, NEW;
	BEGIN
		f.wr.WriteTab
	END WriteTab;

	(*	writes a view to output stream	*)
	PROCEDURE (VAR f: Formatter) WriteView* (v: ANYPTR; w, h: INTEGER), NEW;
	BEGIN
		INC(f.views);
		f.viewW := MAX(f.viewW, w);
		f.viewH := MAX(f.viewH, h);
		f.wr.WriteView(v, w, h)
	END WriteView;

	(*	sets number of significant figures used for outputting real numbers	*)
	PROCEDURE SetPrec* (precission: INTEGER);
	BEGIN
		prec := precission
	END SetPrec;

	(*	sets where output will be written to	*)
	PROCEDURE SetDest* (dest: INTEGER);
	BEGIN
		whereOut := dest
	END SetDest;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		SetPrec(4);
		whereOut := window
	END Init;

BEGIN
	Init
END BugsMappers.

