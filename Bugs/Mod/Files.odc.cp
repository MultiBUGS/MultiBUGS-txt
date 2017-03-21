(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsFiles;


	

	IMPORT
		SYSTEM,
		Console, Files,
		BugsMappers;

	TYPE

		Reader = POINTER TO RECORD(BugsMappers.Reader)
			fileRd: Files.Reader
		END;

		Writer = POINTER TO RECORD(BugsMappers.Writer)
			fileWr: Files.Writer;
			tabArray: ARRAY 40 OF INTEGER;
			linePos, numTabs, tabNum: INTEGER
		END;

	VAR
		workingDir-, tempDir-: ARRAY 1024 OF CHAR;

		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		PROCEDURE PathToFileSpec* (IN path: ARRAY OF CHAR; OUT loc: Files.Locator;
	OUT name: Files.Name);
		VAR
			i, j: INTEGER;
			ch: CHAR;
			fullPath: ARRAY 4096 OF CHAR;
	BEGIN
		i := 0;
		j := 0;
		IF workingDir # "" THEN fullPath := workingDir + "/" + path ELSE fullPath := path$ END;
		loc := Files.dir.This("");
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

	PROCEDURE SetWorkingDir* (path: ARRAY OF CHAR);
	BEGIN
		workingDir := path$
	END SetWorkingDir;

	PROCEDURE SetTempDir* (path: ARRAY OF CHAR);
	BEGIN
		tempDir := path$
	END SetTempDir;

	PROCEDURE (rd: Reader) ReadChar (OUT ch: CHAR);
		VAR
			byte: BYTE;
	BEGIN
		IF ~rd.eot THEN
			rd.fileRd.ReadByte(byte);
			ch := SYSTEM.VAL(SHORTCHAR, byte);
			rd.eot := rd.fileRd.eof
		ELSE
			ch := 0X
		END
	END ReadChar;

	PROCEDURE (rd: Reader) Pos (): INTEGER;
	BEGIN
		RETURN rd.fileRd.Pos()
	END Pos;

	PROCEDURE (rd: Reader) SetPos (pos: INTEGER);
	BEGIN
		rd.fileRd.SetPos(pos)
	END SetPos;

	PROCEDURE (wr: Writer) Bold;
	BEGIN
	END Bold;

	PROCEDURE (wr: Writer) LineHeight (): INTEGER;
	BEGIN
		RETURN 1
	END LineHeight;

	PROCEDURE (wr: Writer) Pos (): INTEGER;
	BEGIN
		RETURN wr.fileWr.Pos()
	END Pos;

	PROCEDURE (wr: Writer) Register (IN name: ARRAY OF CHAR; w, h: INTEGER);
		VAR
			res: INTEGER;
			fileName: Files.Name;
			file: Files.File;
			type: Files.Type;
	BEGIN
		IF wr.fileWr # NIL THEN
			file := wr.fileWr.Base();
			type := "txt";
			Files.dir.GetFileName(name$, type, fileName);
			file.Register(name$, type, Files.dontAsk, res)
		END
	END Register;

	PROCEDURE (wr: Writer) SetPos (pos: INTEGER);
	BEGIN
		IF wr.fileWr # NIL THEN
			wr.fileWr.SetPos(pos)
		END
	END SetPos;

	PROCEDURE (wr: Writer) StdRegister;
		VAR
			res: INTEGER;
			fileName: Files.Name;
			file: Files.File;
			type: Files.Type;
	BEGIN
		IF wr.fileWr # NIL THEN
			file := wr.fileWr.Base();
			type := "txt";
			Files.dir.GetFileName("buffer", type, fileName);
			file.Register(fileName, type, Files.dontAsk, res);
			ASSERT(res = 0, 66);
			wr.fileWr := NIL;
			file.Close
		END
	END StdRegister;

	PROCEDURE (wr: Writer) WriteChar (ch: CHAR);
		VAR
			byte: BYTE;
	BEGIN
		IF wr.fileWr # NIL THEN
			byte := SYSTEM.VAL(BYTE, SHORT(ch));
			wr.fileWr.WriteByte(byte);
		ELSE
			Console.WriteChar(ch);
			INC(wr.linePos)
		END
	END WriteChar;

	PROCEDURE (wr: Writer) WriteLn;
		VAR
			byte: BYTE;
			cr, lf: CHAR;
	BEGIN
		IF wr.fileWr # NIL THEN
			cr := 0DX;
			lf := 0AX;
			byte := SYSTEM.VAL(BYTE, SHORT(cr));
			wr.fileWr.WriteByte(byte);
			byte := SYSTEM.VAL(BYTE, SHORT(lf));
			wr.fileWr.WriteByte(byte);
		ELSE
			Console.WriteLn;
		END;
		wr.linePos := 0;
		wr.tabNum := 0
	END WriteLn;

	PROCEDURE (wr: Writer) WriteRuler (IN tabs: ARRAY OF INTEGER);
		VAR
			i: INTEGER;
	BEGIN
		wr.numTabs := LEN(tabs);
		i := 0;
		WHILE i < LEN(tabs) DO
			wr.tabArray[i] := tabs[i];
			INC(i)
		END
	END WriteRuler;

	PROCEDURE (wr: Writer) WriteString (IN string: ARRAY OF CHAR);
		VAR
			byte: BYTE;
			i: INTEGER;
	BEGIN
		IF wr.fileWr # NIL THEN
			i := 0;
			WHILE string[i] # 0X DO
				byte := SYSTEM.VAL(BYTE, SHORT(string[i]));
				wr.fileWr.WriteByte(byte);
				INC(i)
			END;
		ELSE
			Console.WriteStr(string)
		END;
		INC(wr.linePos, LEN(string$))
	END WriteString;

	PROCEDURE (wr: Writer) WriteTab;
		VAR
			byte: BYTE;
			blank: CHAR;
	BEGIN
		blank := " ";
		byte := SYSTEM.VAL(BYTE, SHORT(blank));
		IF wr.tabNum > wr.numTabs THEN
			IF wr.fileWr # NIL THEN
				wr.fileWr.WriteByte(byte);
			ELSE
				Console.WriteChar(blank)
			END;
			INC(wr.linePos)
		ELSIF wr.linePos >= wr.tabArray[wr.tabNum] THEN
			IF wr.fileWr # NIL THEN
				wr.fileWr.WriteByte(byte);
			ELSE
				Console.WriteChar(blank)
			END;
			INC(wr.linePos)
		ELSE
			WHILE wr. linePos < wr.tabArray[wr.tabNum] DO
				IF wr.fileWr # NIL THEN
					wr.fileWr.WriteByte(byte);
				ELSE
					Console.WriteChar(blank)
				END;
				INC(wr.linePos)
			END
		END;
		INC(wr.tabNum)
	END WriteTab;

	PROCEDURE (wr: Writer) WriteView (v: ANYPTR; w, h: INTEGER);
	BEGIN
	END WriteView;

	PROCEDURE ConnectScanner* (VAR s: BugsMappers.Scanner; file: Files.File);
		VAR
			rd: Reader;
	BEGIN
		NEW(rd);
		rd.fileRd := file.NewReader(NIL);
		ASSERT(rd.fileRd # NIL, 60);
		s.SetReader(rd);
		s.SetPos(0);
	END ConnectScanner;

	PROCEDURE ConnectFormatter* (VAR f: BugsMappers.Formatter; file: Files.File);
		VAR
			wr: Writer;
	BEGIN
		NEW(wr);
		IF file # NIL THEN
			wr.fileWr := file.NewWriter(NIL);
			wr.fileWr.SetPos(0);
		ELSE
			wr.fileWr := NIL
		END;
		wr.linePos := 0;
		wr.numTabs := 0;
		wr.tabNum := 0;
		f.SetWriter(wr)
	END ConnectFormatter;

	PROCEDURE BufferFile* (): Files.File;
		VAR
			loc: Files.Locator;
	BEGIN
		loc := Files.dir.This(tempDir);
		RETURN Files.dir.New(loc, Files.dontAsk)
	END BufferFile;

	PROCEDURE StdConnect* (VAR f: BugsMappers.Formatter);
		VAR
			file: Files.File;
	BEGIN
		IF BugsMappers.whereOut = BugsMappers.file THEN
			file := BufferFile();
		ELSE
			file := NIL
		END;
		ConnectFormatter(f, file);
	END StdConnect;

	PROCEDURE ShowMsg* (s: ARRAY OF CHAR);
		VAR
			f: BugsMappers.Formatter;
			file: Files.File;
	BEGIN
		IF BugsMappers.whereOut = BugsMappers.file THEN
			file := BufferFile();
			ConnectFormatter(f, file);
			f.SetPos(0);
			f.WriteString(s);
			f.WriteLn;
			f.StdRegister
		ELSE
			Console.WriteStr(s); Console.WriteLn
		END
	END ShowMsg;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		tempDir := ""
	END Init;

BEGIN
	Init
END BugsFiles.

