MODULE ParallelFiles;

	(*
		Иван Денисов, 2 августа 2017
		Реализация Files.File для RAM, поддерживается только один временный файл
		
		Ivan Denisov, August 2, 2017
		Implementing Files.File for RAM, only one temporary file is supported
		
	*)

	IMPORT Files := Files64;

	TYPE
		File = POINTER TO RECORD (Files.File)
			data:  POINTER TO ARRAY OF BYTE
		END;

		Reader = POINTER TO RECORD (Files.Reader)
			base: File; 
			pos: INTEGER;
		END;

		Writer = POINTER TO RECORD (Files.Writer)
			base: File; 
			pos: INTEGER;
		END;

		Directory = POINTER TO RECORD (Files.Directory) END;
			VAR
		dir-: Files.Directory;
		maxLength-: INTEGER;

	(* File *)
	
	PROCEDURE (f: File) NewReader (old: Files.Reader): Files.Reader;
		VAR r: Reader;
	BEGIN
		IF (old # NIL) & (old IS Reader) THEN
			r := old(Reader)
		ELSE
			NEW(r)
		END;
		r.base := f;
		r.pos := 0;
		RETURN r
	END NewReader;

	PROCEDURE (f: File) NewWriter (old: Files.Writer): Files.Writer;
		VAR w: Writer;
	BEGIN
		IF (old # NIL) & (old IS Writer) THEN
			w := old(Writer)
		ELSE
			NEW(w)
		END;
		w.base := f;
		w.pos := 0;
		RETURN w
	END NewWriter;

	PROCEDURE (f: File) Length (): LONGINT;
	BEGIN
		RETURN LEN(f.data)
	END Length;
	
	PROCEDURE (f: File) Flush;
	BEGIN
	END Flush;

	PROCEDURE (f: File) Register (name: Files.Name; type: Files.Type; ask: BOOLEAN; OUT res: INTEGER);
	BEGIN 
		HALT(0)
	END Register;

	PROCEDURE (f: File) Close;
	BEGIN
		f.data := NIL;
	END Close;

	PROCEDURE (f: File) Closed (): BOOLEAN;
	BEGIN 
		RETURN f.data # NIL
	END Closed;

	PROCEDURE (f: File) Shared (): BOOLEAN;
	BEGIN
		RETURN TRUE
	END Shared;

	(* Reader *)

	PROCEDURE (r: Reader) Base (): Files.File;
	BEGIN
		RETURN r.base
	END Base;

	PROCEDURE (r: Reader) SetPos (pos: LONGINT);
	BEGIN
		r.pos := SHORT(pos)
	END SetPos;

	PROCEDURE (r: Reader) Pos (): LONGINT;
	BEGIN
		RETURN r.pos
	END Pos;

	PROCEDURE (r: Reader) ReadByte (OUT x: BYTE);
	BEGIN
		x := r.base.data[r.pos];
		INC(r.pos);
	END ReadByte;

	PROCEDURE (r: Reader) ReadBytes (VAR x: ARRAY OF BYTE; beg, len: INTEGER);
	BEGIN
		len := beg + len;
		WHILE beg < len DO
			x[beg] := r.base.data[r.pos];
			INC(r.pos); INC(beg)
		END 
	END ReadBytes;

	(* Writer *)

	PROCEDURE (w: Writer) Base (): Files.File;
	BEGIN
		RETURN w.base
	END Base;

	PROCEDURE (w: Writer) SetPos (pos: LONGINT);
	BEGIN
		w.pos := SHORT(pos)
	END SetPos;

	PROCEDURE (w: Writer) Pos (): LONGINT;
	BEGIN
		RETURN w.pos
	END Pos;

	PROCEDURE (w: Writer) WriteByte (x: BYTE);
	BEGIN
		w.base.data[w.pos] := x;
		INC(w.pos)
	END WriteByte;

	PROCEDURE (w: Writer) WriteBytes (IN x: ARRAY OF BYTE; beg, len: INTEGER);
	BEGIN
		len := beg + len;
		WHILE beg < len DO
			w.base.data[w.pos] := x[beg];
			INC(w.pos); INC(beg)
		END
	END WriteBytes;

	(* Directory *)

	PROCEDURE (d: Directory) This (IN path: ARRAY OF CHAR): Files.Locator;
	BEGIN
		HALT(0);
		RETURN NIL
	END This;

	PROCEDURE (d: Directory) New (loc: Files.Locator; ask: BOOLEAN): Files.File;
	BEGIN
		HALT(0);
		RETURN NIL
	END New;
	
	PROCEDURE (d: Directory) Temp (): Files.File;
	VAR 
		file: File;
	BEGIN
		NEW(file);
		NEW(file.data, maxLength);
		RETURN file
	END Temp;

	PROCEDURE (d: Directory) Old (loc: Files.Locator; name: Files.Name; shrd: BOOLEAN): Files.File;
	BEGIN
		HALT(0);
		RETURN NIL
	END Old;

	PROCEDURE (d: Directory) Delete (loc: Files.Locator; name: Files.Name);
	BEGIN
	END Delete;

	PROCEDURE (d: Directory) Rename (loc: Files.Locator; old, new: Files.Name; ask: BOOLEAN);
	BEGIN
	END Rename;

	PROCEDURE (d: Directory) SameFile (loc0: Files.Locator; name0: Files.Name;
		loc1: Files.Locator; name1: Files.Name): BOOLEAN;
	BEGIN 
		RETURN TRUE
	END SameFile;

	PROCEDURE (d: Directory) FileList (loc: Files.Locator): Files.FileInfo;
	BEGIN
		RETURN NIL
	END FileList;
	
	PROCEDURE (d: Directory) LocList (loc: Files.Locator): Files.LocInfo;
	BEGIN
		RETURN NIL
	END LocList;

	PROCEDURE (d: Directory) GetFileName (name: Files.Name; type: Files.Type; OUT filename: Files.Name);
	BEGIN
	END GetFileName;
	
	PROCEDURE NewSize* (size: INTEGER);
	BEGIN
		maxLength := size
	END NewSize;

	PROCEDURE Init;
		VAR
			d: Directory;
	BEGIN
		NEW(d);
		dir := d;
		maxLength := 1024 * 64
	END Init;
	
BEGIN
	Init
END ParallelFiles.

