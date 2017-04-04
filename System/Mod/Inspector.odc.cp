MODULE Inspector;

	

	IMPORT
		Files,
		Strings, StdLog;

	CONST
		FileAlign = 512;

	PROCEDURE PathToLoc (IN path: ARRAY OF CHAR; OUT loc: Files.Locator);
		VAR
			i, j: INTEGER;
			ch: CHAR;
			name: ARRAY 256 OF CHAR;
	BEGIN
		loc := Files.dir.This("");
		IF path # "" THEN
			i := 0; j := 0;
			REPEAT
				ch := path[i]; INC(i);
				IF (ch = "/") OR (ch = 0X) THEN name[j] := 0X; j := 0; loc := loc.This(name)
				ELSE name[j] := ch; INC(j)
				END
			UNTIL (ch = 0X) OR (loc.res # 0)
		END
	END PathToLoc;

	PROCEDURE ReadHeader* (name: ARRAY OF CHAR);
		VAR
			in: Files.File;
			r: Files.Reader;
			loc: Files.Locator;
			addressRVA, codeBase, eRVA, i, imageBase, j, len, nameRVA, numEntry, numObjects,
			offset, peOffset, pos, start, timeStamp, x: INTEGER;
			path: Files.Name;
			string: ARRAY 64 OF CHAR;
			isDll: BOOLEAN;
			ch: CHAR;
			addressTable, namePtrTable, ordinalTable: POINTER TO ARRAY OF INTEGER;


		PROCEDURE Read2 (VAR x: INTEGER);
			VAR b: BYTE;
		BEGIN
			r.ReadByte(b); x := b MOD 256;
			r.ReadByte(b); x := x + 100H * (b MOD 256);
		END Read2;

		PROCEDURE Read4 (VAR x: INTEGER);
			VAR b: BYTE;
		BEGIN
			r.ReadByte(b); x := b MOD 256;
			r.ReadByte(b); x := x + 100H * (b MOD 256);
			r.ReadByte(b); x := x + 10000H * (b MOD 256);
			r.ReadByte(b); x := x + 1000000H * b
		END Read4;

		PROCEDURE ReadCh (VAR ch: CHAR);
			VAR b: BYTE;
		BEGIN
			r.ReadByte(b);
			ch := CHR(b)
		END ReadCh;

		PROCEDURE ReadName (VAR name: ARRAY OF CHAR; len: INTEGER);
			VAR i: INTEGER; b: BYTE;
		BEGIN
			i := 0; WHILE i < LEN(name) DO name[i] := 0X; INC(i) END;
			i := 0;
			REPEAT
				r.ReadByte(b); name[i] := CHR(b); INC(i)
			UNTIL (b = 0) OR (i = len);
			IF i < len THEN name[i] := 0X END;
			WHILE i < len DO
				r.ReadByte(b); INC(i)
			END
		END ReadName;

	BEGIN
		path := name$;
		start := 0;
		Strings.Find(name, "/", start, pos);
		WHILE pos #  - 1 DO
			start := pos + 1;
			Strings.Find(name, "/", start, pos)
		END;
		path[start] := 0X;
		PathToLoc(path, loc);
		len := LEN(name$);
		Strings.Extract(name, start, len - start, name);

		in := Files.dir.Old(loc, name$, Files.shared);
		ASSERT(in # NIL, 21);
		len := in.Length();
		StdLog.String("File length is: "); StdLog.Int(len); StdLog.Ln;
		r := in.NewReader(NIL);
		r.SetPos(0);

		start := 0;
		Strings.Find(name, ".dll", start, pos);
		isDll := pos #  - 1;


		(* DOS header *)
		Read4(x); Read4(x); Read4(x); Read4(x);
		Read4(x); Read4(x); Read4(x); Read4(x);
		Read4(x); Read4(x); Read4(x); Read4(x);
		Read4(x); Read4(x); Read4(x); Read4(x); peOffset := x;
		Read4(x); Read4(x); Read4(x); Read2(x);
		ReadName(string, 39); StdLog.String(string); StdLog.Ln;
		ReadCh(ch); ReadCh(ch); ReadCh(ch);
		Read4(x); Read4(x);

		(*	Jump to PE header	*)
		r.SetPos(peOffset);

		(* Win32 header *)
		ReadName(string, 4); StdLog.String(string); StdLog.Ln;
		Read2(x); StdLog.String("CPU type is: "); StdLog.Int(x); StdLog.Ln;
		Read2(numObjects); StdLog.String("Number of objects is: "); StdLog.Int(numObjects); StdLog.Ln;
		Read4(timeStamp); StdLog.String("Time stamp: "); StdLog.Int(timeStamp); StdLog.Ln;
		Read4(x); Read4(x);
		Read2(x); (* NT header size *)
		Read2(x); StdLog.String("Image flags: "); StdLog.Int(x); StdLog.Ln;
		Read2(x); StdLog.String("Magic number: "); StdLog.Int(x); StdLog.Ln;
		Read2(x); StdLog.String("Linker version: "); StdLog.Int(x); StdLog.Ln;
		Read4(x); StdLog.String("Code size: "); StdLog.Int(x); StdLog.Ln;
		Read4(x); StdLog.String("Initialized data size: "); StdLog.Int(x); StdLog.Ln;
		Read4(x); StdLog.String("Uninitialized data size: "); StdLog.Int(x); StdLog.Ln;
		Read4(x); StdLog.String("Entry point: "); StdLog.Int(x); StdLog.Ln;
		Read4(codeBase); StdLog.String("Base of code: "); StdLog.Int(codeBase); StdLog.Ln;
		Read4(x); StdLog.String("Base of data: "); StdLog.Int(x); StdLog.Ln;
		Read4(imageBase); StdLog.String("Image base: "); StdLog.Int(imageBase); StdLog.Ln;
		Read4(x); StdLog.String("Object align is: "); StdLog.Int(x); StdLog.Ln;
		Read4(x); StdLog.String("File align is: "); StdLog.Int(x); StdLog.Ln;
		Read4(x); StdLog.String("OS version: "); StdLog.Int(x); StdLog.Ln;
		Read4(x); StdLog.String("User version: "); StdLog.Int(x); StdLog.Ln;
		Read4(x); StdLog.String("Subsys version: "); StdLog.Int(x); StdLog.Ln;
		Read4(x);
		Read4(x); StdLog.String("Image size: "); StdLog.Int(x); StdLog.Ln;
		Read4(x); StdLog.String("Header size: "); StdLog.Int(x); StdLog.Ln;
		Read4(x); StdLog.String("Check sum: "); StdLog.Int(x); StdLog.Ln;
		Read2(x); StdLog.String("Subsystem type: "); StdLog.Int(x); StdLog.Ln;
		Read2(x); (* dll flags *)
		Read4(x); StdLog.String("Stack reserve size: "); StdLog.Int(x); StdLog.Ln;
		Read4(x); StdLog.String("Stack commit size: "); StdLog.Int(x); StdLog.Ln;
		Read4(x); StdLog.String("Heap reserve size: "); StdLog.Int(x); StdLog.Ln;
		Read4(x); StdLog.String("Heap commit size: "); StdLog.Int(x); StdLog.Ln;
		Read4(x); StdLog.String("DLL info: "); StdLog.Int(x); StdLog.Ln;
		Read4(x); StdLog.String("num of rva/sizes: "); StdLog.Int(x); StdLog.Ln;
		Read4(eRVA); Read4(x); StdLog.String("Export table RVA and size: ");
		StdLog.Int(eRVA); StdLog.Char(" "); StdLog.Int(x); StdLog.Ln;
		Read4(x); Read4(x); (* import table *)	(* !!! *)
		Read4(x); Read4(x); (* resource table *)	(* !!! *)
		Read4(x); Read4(x); (* exception table *)
		Read4(x); Read4(x); (* security table *)
		Read4(x); Read4(x); (* fixup table *)	(* !!! *)
		Read4(x); Read4(x); (* debug table *)
		Read4(x); Read4(x); (* image description *)
		Read4(x); Read4(x); (* machine specific *)
		Read4(x); Read4(x); (* thread local storage *)
		Read4(x); Read4(x); (* ??? *)
		Read4(x); Read4(x); (* ??? *)
		Read4(x); Read4(x); (* ??? *)
		Read4(x); Read4(x); (* ??? *)
		Read4(x); Read4(x); (* ??? *)
		Read4(x); Read4(x); (* ??? *)

		i := 0;
		WHILE i < numObjects DO
			ReadName(string, 8); StdLog.String(string); StdLog.Ln;
			Read4(x); (* object size (always 0) *)
			Read4(x); StdLog.Tab; StdLog.String("rva is: "); StdLog.Int(x); StdLog.Ln;
			Read4(x); StdLog.Tab; StdLog.String("physical size: "); StdLog.Int(x); StdLog.Ln;
			Read4(x); StdLog.Tab; StdLog.String("physical offset: "); StdLog.Int(x); StdLog.Ln;
			IF string = ".text" THEN offset := x END;
			Read4(x); Read4(x); Read4(x);
			Read4(x); StdLog.Tab; StdLog.String("flags: "); StdLog.Set(BITS(x)); StdLog.Ln;
			INC(i)
		END;

		pos := offset + (eRVA - codeBase);
		StdLog.String("Jump to file position "); StdLog.Int(pos); StdLog.Ln;
		r.SetPos(pos);
		Read4(x);
		Read4(timeStamp); StdLog.String("Time stamp: "); StdLog.Int(timeStamp); StdLog.Ln;
		Read4(x);
		Read4(nameRVA); StdLog.String("name rva: "); StdLog.Int(nameRVA); StdLog.Ln;
		Read4(x); StdLog.String("ordinal base: "); StdLog.Int(x); StdLog.Ln;
		Read4(numEntry); StdLog.String("number entries: "); StdLog.Int(numEntry); StdLog.Ln;
		Read4(x); StdLog.String("number names: "); StdLog.Int(x); StdLog.Ln;
		Read4(addressRVA); StdLog.String("address table rva: "); StdLog.Int(addressRVA); StdLog.Ln;
		Read4(x); StdLog.String("name ptr table rva: "); StdLog.Int(x); StdLog.Ln;
		Read4(x); StdLog.String("ordinal table rva: "); StdLog.Int(x); StdLog.Ln;
		NEW(addressTable, numEntry);
		i := 0;
		WHILE i < numEntry DO
			Read4(addressTable[i]); INC(i)
		END;
		NEW(namePtrTable, numEntry);
		i := 0;
		WHILE i < numEntry DO
			Read4(namePtrTable[i]); INC(i)
		END;
		NEW(ordinalTable, numEntry);
		i := 0;
		WHILE i < numEntry DO
			Read2(ordinalTable[i]); INC(i)
		END;
		pos := offset + (nameRVA - codeBase);
		StdLog.String("Jump to file position "); StdLog.Int(pos); StdLog.Ln;
		r.SetPos(pos);
		i := 0;
		j := 0;
		WHILE j <= numEntry DO
			ReadCh(ch);
			IF ch = 0X THEN
				IF j > 0 THEN
					StdLog.Tab; StdLog.Int(j); 
					StdLog.Tab; StdLog.Int(addressTable[j - 1]);
					StdLog.Tab; StdLog.Int(namePtrTable[j - 1]);
					StdLog.Tab; StdLog.Int(ordinalTable[j - 1]);
				END;
				StdLog.Ln;
				INC(j);
			ELSE
				StdLog.Char(ch)
			END;
			INC(i)
		END;
		r := NIL;
		in.Close;
	END ReadHeader;

	PROCEDURE Do*;
	BEGIN
		ReadHeader("c:/Windows/System32/msmpi.dll")
	END Do;

END Inspector.

Inspector.Do
