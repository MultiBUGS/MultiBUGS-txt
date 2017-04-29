(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsMappers;

	(*	formatted input	*)
	

	IMPORT
		Strings,
		TextModels;

	CONST

		char* = 0; (*	scaned token was character	*)
		string* = 1; (*	scanned token was a string	*)
		function* = 2; (*	scanned token was a function	*)
		int* = 3; (*	scanned token was an integer	*)
		real* = 4; (*	scanned token was a real	*)
		inval* = 31; (*	scanned token was invalid	*)

		tab = 09X;
		line = 0DX;
		para = 0EX;
		lf = 0AX;

		stringLen = 1024;

	TYPE
		(*	class for reading characters from a stream	*)
		Reader = POINTER TO ABSTRACT RECORD
			eot: BOOLEAN (*	true if end of input strean reached	*)
		END;
	
		StringReader = POINTER TO RECORD(Reader) 
			string: POINTER TO ARRAY OF CHAR;
			len, pos: INTEGER
		END;
	
		TextReader = POINTER TO RECORD(Reader) 
			textRd: TextModels.Reader
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

	VAR
		version-: INTEGER; (*	version number	*)
		maintainer-: ARRAY 40 OF CHAR; (*	person maintaining module	*)
		
	(*	reads a character from input stream	*)
	PROCEDURE (rd: Reader) ReadChar (OUT ch: CHAR), NEW, ABSTRACT;

	(*	position in input stream	*)
	PROCEDURE (rd: Reader) Pos (): INTEGER, NEW, ABSTRACT;

	(*	get position in input stream	*)
	PROCEDURE (rd: Reader) SetPos (pos: INTEGER), NEW, ABSTRACT;


	PROCEDURE (rd: StringReader) ReadChar (OUT ch: CHAR);
		VAR
			len: INTEGER;
	BEGIN
		IF ~rd.eot THEN
			len := rd.len;
			IF rd.pos < len THEN
				ch := rd.string[rd.pos];
				INC(rd.pos);
				IF ch = 0X THEN
					rd.eot := TRUE
				ELSE
					rd.eot := rd.pos = len
				END
			ELSE
				rd.eot := TRUE;
				ch := 0X
			END
		ELSE
			ch := 0X
		END
	END ReadChar;

	PROCEDURE (rd: StringReader) Pos (): INTEGER;
	BEGIN
		RETURN rd.pos
	END Pos;

	PROCEDURE (rd: StringReader) SetPos (pos: INTEGER);
	BEGIN
		rd.pos := pos
	END SetPos;

	PROCEDURE (rd: TextReader) ReadChar (OUT ch: CHAR);
	BEGIN
		IF ~rd.eot THEN
			rd.textRd.ReadChar(ch);
			rd.eot := rd.textRd.eot
		ELSE
			ch := 0X
		END
	END ReadChar;

	PROCEDURE (rd: TextReader) Pos (): INTEGER;
	BEGIN
		RETURN rd.textRd.Pos()
	END Pos;

	PROCEDURE (rd: TextReader) SetPos (pos: INTEGER);
	BEGIN
		rd.textRd.SetPos(pos);
		rd.eot := rd.textRd.eot
	END SetPos;

	(*	sets the input stream used by scanner	*)
	PROCEDURE (VAR s: Scanner) SetReader (rd: Reader), NEW;
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

	PROCEDURE (VAR s: Scanner) ConnectToString* (IN string: ARRAY OF CHAR), NEW;
	VAR
		i: INTEGER;
		rd: StringReader;
	BEGIN
		i := 0;
		WHILE string[i] # 0X DO INC(i) END;
		NEW(rd);
		NEW(rd.string, i + 1);
		rd.string^ := string$;
		rd.len := i + 1;
		rd.pos := 0;
		rd.eot := FALSE;
		s.rd := rd
	END ConnectToString;

	PROCEDURE (VAR s: Scanner) ConnectToText* (text: TextModels.Model), NEW;
		VAR
			rd: TextReader;
	BEGIN
		NEW(rd);
		rd.textRd := text.NewReader(NIL);
		s.SetReader(rd)
	END ConnectToText;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END BugsMappers.

