MODULE TestMPI;

	

	(*	module MPI is the CP interface module to the MPI dynamic link library	*)
	IMPORT Files, MPI, SYSTEM;

	CONST
		tag = 0; dest = 0; 
		
	VAR
		args: POINTER TO ARRAY[untagged] OF SHORTCHAR;
		nargs, rank, worldSize, source, j, res: INTEGER;
		message: ARRAY 100 OF CHAR;
		ch: SHORTCHAR;
		byte: BYTE;
		f: Files.File;
		loc: Files.Locator;
		wr: Files.Writer;


	PROCEDURE IntToString (x: LONGINT; OUT s: ARRAY OF CHAR); (* copied from Strings *)
		CONST minLongIntRev = "8085774586302733229";
		VAR j, k: INTEGER; ch: CHAR; a: ARRAY 32 OF CHAR;
	BEGIN
		IF x # MIN(LONGINT) THEN
			IF x < 0 THEN s[0] := "-"; k := 1; x := - x ELSE k := 0 END;
			j := 0; REPEAT a[j] := CHR(x MOD 10 + ORD("0")); x := x DIV 10; INC(j) UNTIL x = 0
		ELSE
			a := minLongIntRev; s[0] := "-"; k := 1; j := LEN(minLongIntRev);
		END;
		ASSERT(k + j < LEN(s), 23);
		REPEAT DEC(j); ch := a[j]; s[k] := ch; INC(k) UNTIL j = 0;
		s[k] := 0X
	END IntToString;

BEGIN
	MPI.Init(nargs, args); 
	MPI.Comm_rank(MPI.COMM_WORLD, rank);
	MPI.Comm_size(MPI.COMM_WORLD, worldSize);
	IF rank = 0 THEN
		loc := Files.dir.This("");
		f := Files.dir.New(loc, Files.dontAsk);
		wr := f.NewWriter(NIL);
		wr.SetPos(0);
		source := 1;
		WHILE source < worldSize DO
			MPI.Recv(SYSTEM.ADR(message), LEN(message), MPI.CHAR, source, tag,
			MPI.COMM_WORLD, MPI.STATUS_IGNORE);
			j := 0;
			WHILE j < LEN(message$) DO
				ch := SHORT(message[j]);
				byte := SYSTEM.VAL(BYTE, ch);
				wr.WriteByte(byte);
				INC(j)
			END;
			INC(source)
		END;
		f.Register("results", "txt", Files.dontAsk, res);
	ELSE
		IntToString(rank, message);
		message := "Greetings from process " + message + 0DX;
		MPI.Send(SYSTEM.ADR(message), LEN(message), MPI.CHAR, dest, tag,
		MPI.COMM_WORLD)
	END;
	MPI.Finalize
END TestMPI.

DevLinker.LinkExe Test.exe := Kernel+ Files  HostFiles TestMPI ~

Open the DOS prompt and then change directory to the one containg Test.exe file and type "mpiexec -n 4 Test.exe" at the prompt followed by return. Output should appear in the file results.txt.
