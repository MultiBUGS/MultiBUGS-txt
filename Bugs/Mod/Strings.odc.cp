(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	*)

MODULE BugsStrings;

	

	IMPORT
		BugsMappers;

	TYPE

		Reader = POINTER TO RECORD(BugsMappers.Reader)
			string: POINTER TO ARRAY OF CHAR;
			pos: INTEGER
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (rd: Reader) ReadChar (OUT ch: CHAR);
		VAR
			len: INTEGER;
	BEGIN
		IF ~rd.eot THEN
			len := LEN(rd.string);
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

	PROCEDURE (rd: Reader) Pos (): INTEGER;
	BEGIN
		RETURN rd.pos
	END Pos;

	PROCEDURE (rd: Reader) SetPos (pos: INTEGER);
	BEGIN
		rd.pos := pos
	END SetPos;

	PROCEDURE ConnectScanner* (VAR s: BugsMappers.Scanner; IN string: ARRAY OF CHAR);
		VAR
			i: INTEGER;
			rd: Reader;
	BEGIN
		i := 0;
		WHILE string[i] # 0X DO INC(i) END;
		NEW(rd);
		NEW(rd.string, i + 1);
		rd.string^ := string$;
		rd.eot := FALSE;
		s.SetReader(rd)
	END ConnectScanner;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END BugsStrings.
