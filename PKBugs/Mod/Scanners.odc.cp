(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE PKBugsScanners;

		(* PKBugs Version 1.1 *)

	IMPORT
		Math;

	CONST
		char* = 0; string* = 1; int* = 2; real* = 3; inval* = 4; function* = 5; clock* = 6; date* = 7;

		tab = 09X; para = 0EX; line = 0DX;

	TYPE
		(*	class for reading characters from a stream	*)
		Reader* = POINTER TO ABSTRACT RECORD
			eot*: BOOLEAN (*	true if end of input strean reached	*)
		END;

		Scanner* = RECORD
			field-: ARRAY 3 OF LONGINT;
			nDigits-: ARRAY 3 OF INTEGER;
			neg-: ARRAY 3 OF BOOLEAN;
			char-, nextCh-: CHAR;
			real-: REAL;
			(*pos-: INTEGER;*)
			type-, len-, nFields-: INTEGER;
			string-: ARRAY 80 OF CHAR;
			reader: Reader;
			eot-: BOOLEAN
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;


	(*	reads a character from input stream	*)
	PROCEDURE (rd: Reader) ReadChar- (OUT ch: CHAR), NEW, ABSTRACT;

	(*	position in input stream	*)
	PROCEDURE (rd: Reader) Pos- (): INTEGER, NEW, ABSTRACT;

	(*	get position in input stream	*)
	PROCEDURE (rd: Reader) SetPos- (pos: INTEGER), NEW, ABSTRACT;

	PROCEDURE (VAR s: Scanner) SetReader* (reader: Reader), NEW;
	BEGIN
		s.reader := reader
	END SetReader;

	PROCEDURE (VAR s: Scanner) Pos* (): INTEGER, NEW;
	BEGIN
		RETURN s.reader.Pos() - 1
	END Pos;
	
	PROCEDURE (VAR s: Scanner) ReadChar* (VAR ch: CHAR), NEW;
	BEGIN
		IF s.eot THEN
			ch := 0X
		ELSE
			s.reader.ReadChar(ch)
		END
	END ReadChar;

	PROCEDURE ReadFields (VAR s: Scanner; VAR l, n: INTEGER; VAR ch: CHAR);

		PROCEDURE ReadField;
		BEGIN
			s.nDigits[n] := 0; s.field[n] := 0;
			WHILE (ch >= "0") & (ch <= "9") DO
				INC(s.nDigits[n]); s.field[n] := 10 * s.field[n] + ORD(ch) - 48; s.string[l] := ch; INC(l); s.ReadChar(ch)
			END
		END ReadField;

		PROCEDURE CheckForSpace;
		BEGIN
			CASE ch OF
			|" ", ",", tab, line, para, 0X: s.string[l] := 0X; s.len := l
			ELSE s.type := inval END
		END CheckForSpace;

	BEGIN
		ReadField;
		IF (n = 1) THEN
			CASE ch OF
			|" ", ",", tab, line, para, 0X: s.string[l] := 0X; s.len := l
			|"e", "E": s.string[l] := ch; INC(l); s.ReadChar(ch);
				CASE ch OF
				|"0".."9": INC(n); s.neg[n] := FALSE; ReadField; CheckForSpace
				|"+": s.ReadChar(ch);
					IF (ch >= "0") & (ch <= "9") THEN
						INC(n); s.neg[n] := FALSE; ReadField; CheckForSpace
					ELSE s.type := inval END
				|"-": s.string[l] := "-"; INC(l); s.ReadChar(ch);
					IF (ch >= "0") & (ch <= "9") THEN
						INC(n); s.neg[n] := TRUE; ReadField; CheckForSpace
					ELSE s.type := inval END
				ELSE s.type := inval END
			ELSE s.type := inval END
		ELSIF (n = 0) THEN
			CASE ch OF
			|" ", ",", tab, line, para, 0X: s.type := int; s.string[l] := 0X; s.len := l
			|"e", "E": s.string[l] := ch; INC(l); s.ReadChar(ch);
				CASE ch OF
				|"0".."9": INC(n); s.neg[n] := FALSE; ReadField;
					CASE ch OF
					|" ", ",", tab, line, para, 0X:
						s.type := int; s.string[l] := 0X; s.len := l
					ELSE
						s.string[l] := ch; INC(l); s.ReadChar(ch);
						IF (ch >= "0") & (ch <= "9") & ~s.neg[0] THEN
							INC(n); s.neg[n] := FALSE; ReadField; s.type := date; CheckForSpace
						ELSE s.type := inval END
					END
				|"+": s.ReadChar(ch);
					IF (ch >= "0") & (ch <= "9") THEN
						INC(n); s.neg[n] := FALSE; ReadField; s.type := int; CheckForSpace
					ELSE s.type := inval END
				|"-": s.string[l] := "-"; INC(l); s.ReadChar(ch);
					IF (ch >= "0") & (ch <= "9") THEN
						INC(n); s.neg[n] := FALSE; s.field[n] := 0; s.nDigits[n] := 1;
						INC(n); s.neg[n] := TRUE; ReadField; s.type := real; CheckForSpace
					ELSE s.type := inval END
				ELSE s.type := inval END
			|".": s.string[l] := "."; INC(l); s.ReadChar(ch);
				CASE ch OF
				|" ", ",", tab, line, para, 0X:
					s.type := real; INC(n); s.neg[n] := FALSE; s.field[n] := 0; s.nDigits[n] := 1;
					s.string[l] := "0"; INC(l); s.string[l] := 0X; s.len := l
				|"0".."9": INC(n); s.neg[n] := FALSE; ReadField;
					CASE ch OF
					|" ", ",", tab, line, para, 0X:
						s.type := real; s.string[l] := 0X; s.len := l
					|"e", "E": s.string[l] := ch; INC(l); s.ReadChar(ch);
						CASE ch OF
						|"0".."9": INC(n); s.neg[n] := FALSE; ReadField; s.type := real; CheckForSpace
						|"+": s.ReadChar(ch);
							IF (ch >= "0") & (ch <= "9") THEN
								INC(n); s.neg[n] := FALSE; ReadField; s.type := real; CheckForSpace
							ELSE s.type := inval END
						|"-": s.string[l] := "-"; INC(l); s.ReadChar(ch);
							IF (ch >= "0") & (ch <= "9") THEN
								INC(n); s.neg[n] := TRUE; ReadField; s.type := real; CheckForSpace
							ELSE s.type := inval END
						ELSE s.type := inval END
					ELSE
						s.string[l] := ch; INC(l); s.ReadChar(ch);
						IF (ch >= "0") & (ch <= "9") & ~s.neg[0] THEN
							INC(n); s.neg[n] := FALSE; ReadField; s.type := date; CheckForSpace
						ELSE s.type := inval END
					END
				|"e", "E":
					INC(n); s.neg[n] := FALSE; s.field[n] := 0; s.nDigits[n] := 1; s.string[l] := "0"; INC(l);
					s.string[l] := ch; INC(l); s.ReadChar(ch);
					CASE ch OF
					|"0".."9": INC(n); s.neg[n] := FALSE; ReadField; s.type := real; CheckForSpace
					|"+": s.ReadChar(ch);
						IF (ch >= "0") & (ch <= "9") THEN
							INC(n); s.neg[n] := FALSE; ReadField; s.type := real; CheckForSpace
						ELSE s.type := inval END
					|"-": s.string[l] := "-"; INC(l); s.ReadChar(ch);
						IF (ch >= "0") & (ch <= "9") THEN
							INC(n); s.neg[n] := TRUE; ReadField; s.type := real; CheckForSpace
						ELSE s.type := inval END
					ELSE s.type := inval END
				ELSE s.type := inval END
			|":": s.string[l] := ":"; INC(l); s.ReadChar(ch);
				IF ~s.neg[0] THEN
					CASE ch OF
					|" ", ",", tab, line, para, 0X:
						s.type := clock; INC(n); s.neg[n] := FALSE; s.field[n] := 0; s.nDigits[n] := 2;
						s.string[l] := "0"; INC(l); s.string[l] := "0"; INC(l); s.string[l] := 0X; s.len := l
					|"0".."9": INC(n); s.neg[n] := FALSE; ReadField;
						CASE ch OF
						|" ", ",", tab, line, para, 0X: s.string[l] := 0X; s.len := l;
							IF (s.nDigits[n] = 2) & (s.field[n] < 60) THEN s.type := clock ELSE s.type := date END
						ELSE
							s.string[l] := ch; INC(l); s.ReadChar(ch);
							IF (ch >= "0") & (ch <= "9") THEN
								INC(n); s.neg[n] := FALSE; ReadField; s.type := date; CheckForSpace
							ELSE s.type := inval END
						END
					ELSE s.type := inval END
				ELSE s.type := inval END
			ELSE
				s.string[l] := ch; INC(l); s.ReadChar(ch);
				IF (ch >= "0") & (ch <= "9") & ~s.neg[0] THEN
					INC(n); s.neg[n] := FALSE; ReadField;
					CASE ch OF
					|" ", ",", tab, line, para, 0X: s.type := date;
						s.string[l] := 0X; s.len := l
					ELSE
						s.string[l] := ch; INC(l); s.ReadChar(ch);
						IF (ch >= "0") & (ch <= "9") THEN
							INC(n); s.neg[n] := FALSE; ReadField; s.type := date; CheckForSpace
						ELSE s.type := inval END
					END
				ELSE s.type := inval END
			END
		END
	END ReadFields;

	PROCEDURE (VAR s: Scanner) FindToken* (VAR ch: CHAR), NEW;
	BEGIN
		LOOP
			CASE ch OF
			|" ", tab, line, para: s.ReadChar(ch)
			ELSE EXIT
			END
		END
	END FindToken;

	PROCEDURE (VAR s: Scanner) Scan*, NEW;
		VAR
			ch: CHAR; l, n, power: INTEGER;

		PROCEDURE CheckClock;
		BEGIN
			ASSERT(s.nFields = 2, 25); ASSERT(~s.neg[0], 25); ASSERT(~s.neg[1], 25);
			ASSERT(s.nDigits[1] = 2, 25); ASSERT(s.field[1] < 60, 25)
		END CheckClock;

		PROCEDURE CheckReal;
		BEGIN
			ASSERT((s.nFields = 2) OR (s.nFields = 3), 25); ASSERT(~s.neg[1], 25)
		END CheckReal;

		PROCEDURE CheckInt;
		BEGIN
			ASSERT((s.nFields = 1) OR (s.nFields = 2), 25); IF (s.nFields = 2) THEN ASSERT(~s.neg[1], 25) END
		END CheckInt;

	BEGIN
		ch := s.nextCh; s.FindToken(ch);
		CASE ch OF
		|"a".."z", "A".."Z":
			l := 0; s.string[l] := ch; INC(l); s.ReadChar(ch);
			WHILE (((CAP(ch) >= "A") & (CAP(ch) <= "Z")) OR ((ch >= "0") & (ch <= "9")) OR (ch = ".")) DO
				s.string[l] := ch; INC(l); s.ReadChar(ch)
			END;
			s.len := l; s.string[l] := 0X; s.type := string; IF ch = "(" THEN s.type := function END
		|"0".."9":
			l := 0; n := 0; s.neg[n] := FALSE; ReadFields(s, l, n, ch); s.nFields := n + 1
		|"-":
			l := 0; s.string[l] := "-"; INC(l); s.ReadChar(ch);
			CASE ch OF
			|"0".."9": n := 0; s.neg[n] := TRUE; ReadFields(s, l, n, ch); s.nFields := n + 1
			|".": s.ReadChar(ch);
				IF (ch >= "0") & (ch <= "9") THEN
					s.type := real; s.string[l] := "0"; INC(l); s.string[l] := "."; INC(l);
					n := 0; s.neg[n] := TRUE; s.field[n] := 0; s.nDigits[n] := 1; INC(n); s.neg[n] := FALSE;
					ReadFields(s, l, n, ch); s.nFields := n + 1
				ELSE s.type := inval END
			ELSE s.type := inval END
		|".":
			s.ReadChar(ch);
			CASE ch OF
			|" ", ",", tab, line, para, 0X: s.type := char; s.char := "."
			|"0".."9":
				s.type := real; l := 0; s.string[l] := "0"; INC(l); s.string[l] := "."; INC(l);
				n := 0; s.neg[n] := FALSE; s.field[n] := 0; s.nDigits[n] := 1; INC(n); s.neg[n] := FALSE;
				ReadFields(s, l, n, ch); s.nFields := n + 1
			ELSE s.type := inval END
		|":":
			s.ReadChar(ch);
			IF (ch >= "0") & (ch <= "5") THEN
				s.type := clock; l := 0; s.string[l] := "0"; INC(l); s.string[l] := ":"; INC(l); s.string[l] := ch; INC(l);
				n := 0; s.neg[n] := FALSE; s.field[n] := 0; s.nDigits[n] := 1;
				INC(n); s.neg[n] := FALSE; s.field[n] := 10 * (ORD(ch) - 48);
				s.ReadChar(ch);
				IF (ch >= "0") & (ch <= "9") THEN
					s.string[l] := ch; INC(l); s.field[n] := s.field[n] + ORD(ch) - 48; s.ReadChar(ch);
					CASE ch OF
					|" ", ",", tab, line, para, 0X:
						s.nDigits[n] := 2; s.string[l] := 0X; s.len := l; s.nFields := n + 1
					ELSE s.type := inval END
				ELSE s.type := inval END
			ELSE s.type := inval END
		ELSE
			s.type := char; s.char := ch; s.ReadChar(ch)
		END;
		s.nextCh := ch;
		CASE s.type OF
		|clock: CheckClock; s.real := s.field[0] + s.field[1] / 60
		|real: CheckReal; s.real := s.field[0];
			IF (s.field[1] # 0) THEN s.real := s.real + s.field[1] / Math.IntPower(10, s.nDigits[1]) END;
			IF s.neg[0] THEN s.real := - s.real END;
			IF (s.nFields = 3) THEN
				power := SHORT(s.field[2]); IF s.neg[2] THEN power := - power END;
				s.real := s.real * Math.IntPower(10, power)
			END
		|int: CheckInt; s.real := s.field[0];
			IF s.neg[0] THEN s.real := - s.real END;
			IF (s.nFields = 2) THEN power := SHORT(s.field[1]); s.real := s.real * Math.IntPower(10, power) END
		ELSE
		END
	END Scan;

	PROCEDURE (VAR s: Scanner) SetPos* (pos: INTEGER), NEW;
	BEGIN
		s.eot := FALSE;
		s.reader.SetPos(pos);
		s.ReadChar(s.nextCh)
	END SetPos;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "D.Lunn"
	END Maintainer;

BEGIN
	Maintainer
END PKBugsScanners.
