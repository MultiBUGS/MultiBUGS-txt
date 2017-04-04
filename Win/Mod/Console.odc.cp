(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)


MODULE WinConsole;
	

	IMPORT
		SYSTEM,
		Console, Dialog,
		WinApi;

	CONST
		black = 0;
		white = 15;

	TYPE
		Stream = RECORD
			buffer: ARRAY 1024 OF SHORTCHAR;
			i, len: INTEGER
		END;

		StdConsole = POINTER TO RECORD (Console.Console) END;

	VAR
		stdOut, stdIn: WinApi.HANDLE;
		stream: Stream;
		open: BOOLEAN;
		console: StdConsole;

	PROCEDURE (VAR s: Stream) Init, NEW;
	BEGIN
		s.len := 0;
		s.i := 0
	END Init;

	PROCEDURE (VAR s: Stream) GetChar (OUT ch: CHAR), NEW;
		VAR
			ok: INTEGER;
	BEGIN
		IF s.i >= s.len THEN
			ok := WinApi.ReadConsoleA(stdIn, SYSTEM.ADR(s.buffer), LEN(s.buffer), s.len, 0);
			s.i := 0
		END;
		ch := s.buffer[s.i];
		INC(s.i)
	END GetChar;

	PROCEDURE (VAR s: Stream) LookAhead (OUT ch: CHAR; OUT success: BOOLEAN), NEW;
	BEGIN
		success := s.i < s.len;
		IF success THEN ch := s.buffer[s.i] END
	END LookAhead;

	PROCEDURE (cons: StdConsole) WriteStr (IN text: ARRAY OF CHAR);
		VAR
			res: WinApi.BOOL;
			written: INTEGER;
			aux: POINTER TO ARRAY OF SHORTCHAR;
	BEGIN
		NEW(aux, LEN(text$) + 1);
		aux^ := SHORT(text);
		res := WinApi.WriteConsoleA(stdOut, SYSTEM.ADR(aux^), LEN(aux$), written, 0)
	END WriteStr;

	PROCEDURE (con: StdConsole) WriteChar (c: CHAR);
		VAR
			res: WinApi.BOOL;
			written: INTEGER;
			aux: ARRAY 2 OF SHORTCHAR;
	BEGIN
		aux[0] := SHORT(c);
		aux[1] := 0X;
		res := WinApi.WriteConsoleA(stdOut, SYSTEM.ADR(aux), 2, written, 0)
	END WriteChar;

	PROCEDURE (cons: StdConsole) WriteLn;
		VAR
			txt: ARRAY 3 OF CHAR;
	BEGIN
		txt[0] := 0DX;
		txt[1] := 0AX;
		txt[2] := 0X;
		Console.WriteStr(txt)
	END WriteLn;

	PROCEDURE (cons: StdConsole) ReadLn (OUT cad: ARRAY OF CHAR);
		VAR
			i: INTEGER;
			aux: CHAR;
			ok: BOOLEAN;
			cadAux: ARRAY 3 OF CHAR;
	BEGIN
		i := 0;
		REPEAT
			stream.GetChar(cad[i]);
			INC(i)
		UNTIL (i = LEN(cad) - 1) OR (cad[i - 1] = 0DX);
		IF cad[i - 1] = 0DX THEN
			(* preserve any newline in read string *)
			stream.GetChar(aux)
		ELSE
			cad[i] := 00X
		END;
		stream.LookAhead(aux, ok);
		IF ok & (aux = 0DX) THEN
			stream.GetChar(aux);
			stream.GetChar(aux)
		END
	END ReadLn;

	PROCEDURE OpenConsole;
		VAR
			ok: INTEGER;
	BEGIN
		ok := WinApi.AllocConsole();
		IF ok # 0 THEN
			stdOut := WinApi.GetStdHandle(WinApi.STD_OUTPUT_HANDLE);
			stdIn := WinApi.GetStdHandle(WinApi.STD_INPUT_HANDLE)
		END
	END OpenConsole;

	PROCEDURE CloseConsole;
		VAR
			ok: INTEGER;
	BEGIN
		IF open THEN ok := WinApi.FreeConsole() END
	END CloseConsole;

	PROCEDURE InitConsole;
	BEGIN
		stdOut := WinApi.GetStdHandle(WinApi.STD_OUTPUT_HANDLE);
		stdIn := WinApi.GetStdHandle(WinApi.STD_INPUT_HANDLE)
	END InitConsole;

	PROCEDURE SetForeground (color: INTEGER);
		VAR
			ok: INTEGER;
			attr: SHORTINT;
			bufferInfo: WinApi.CONSOLE_SCREEN_BUFFER_INFO;
	BEGIN
		ok := WinApi.GetConsoleScreenBufferInfo(stdOut, bufferInfo);
		attr := SHORT(ORD((BITS(bufferInfo.wAttributes) * {4..31} + BITS(color))));
		ok := WinApi.SetConsoleTextAttribute(stdOut, attr)
	END SetForeground;

	PROCEDURE SetBackground (color: INTEGER);
		VAR
			ok: INTEGER;
			attr: SHORTINT;
			bufferInfo: WinApi.CONSOLE_SCREEN_BUFFER_INFO;
	BEGIN
		ok := WinApi.GetConsoleScreenBufferInfo(stdOut, bufferInfo);
		attr := SHORT(ORD((BITS(bufferInfo.wAttributes) * {0..3, 8..15} + BITS(ASH(color, 4)))));
		ok := WinApi.SetConsoleTextAttribute(stdOut, attr)
	END SetBackground;

	PROCEDURE ClearScreen (color: INTEGER);
		VAR
			attr: SHORTINT;
			home: WinApi.COORD;
			ok: INTEGER;
			bufferInfo: WinApi.CONSOLE_SCREEN_BUFFER_INFO;
			numberOfAttrWritten: INTEGER;
	BEGIN
		home.X := 0;
		home.Y := 0;
		ok := WinApi.GetConsoleScreenBufferInfo(stdOut, bufferInfo);
		attr := SHORT(ORD((BITS(bufferInfo.wAttributes) * {0..3, 8..15} + BITS(ASH(color, 4)))));
		ok := WinApi.SetConsoleTextAttribute(stdOut, attr);
		ok := WinApi.FillConsoleOutputAttribute(stdOut, attr, 80 * 25, home, numberOfAttrWritten);
		ok := WinApi.FillConsoleOutputCharacter(stdOut, " ", 80 * 25, home, numberOfAttrWritten);
		ok := WinApi.SetConsoleCursorPosition(stdOut, home)
	END ClearScreen;

	PROCEDURE (con: StdConsole) Open;
	BEGIN
		IF ~open THEN
			open := TRUE;
			OpenConsole;
			InitConsole;
			SetBackground(white);
			SetForeground(black);
			ClearScreen(white)
		END
	END Open;

	PROCEDURE (cons: StdConsole) Close;
	BEGIN
		CloseConsole
	END Close;

BEGIN
	stream.Init;
	open := FALSE;
	NEW(console);
	Console.SetConsole(console)
CLOSE
	CloseConsole;
	open := FALSE
END WinConsole.

