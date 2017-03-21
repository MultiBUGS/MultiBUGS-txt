MODULE TestDll;

	

	IMPORT Kernel, StdLog;

	CONST
		mproc = 4;

	PROCEDURE LoadMPI*;
		VAR
			done: BOOLEAN;
	BEGIN
		Kernel.LoadDll("msmpi.dll", done);
		IF done THEN StdLog.String("mpi loaded");
		ELSE StdLog.String("mpi not loaded")
		END;
		StdLog.Ln
	END LoadMPI;

	PROCEDURE Do*;
		VAR
			h: INTEGER;
	BEGIN
		h := Kernel.ThisDllObj(mproc, 0, "msmpi", "MPI_Init");
		StdLog.String("address of Init is: "); StdLog.Tab; StdLog.Int(h); StdLog.Ln;
		h := Kernel.ThisDllObj(mproc, 0, "msmpi", "MPI_Finalize");
		StdLog.String("address of Finalize is: "); StdLog.Tab; StdLog.Int(h); StdLog.Ln;
		h := Kernel.ThisDllObj(mproc, 0, "msmpi", "MPI_Allgather");
		StdLog.String("address of Allgather is: "); StdLog.Tab; StdLog.Int(h); StdLog.Ln;
		h := Kernel.ThisDllObj(mproc, 0, "msmpi", "MPI_Allreduce");
		StdLog.String("address of Allreduce is: "); StdLog.Tab; StdLog.Int(h); StdLog.Ln;
		h := Kernel.ThisDllObj(mproc, 0, "msmpi", "MPI_Comm_size");
		StdLog.String("address of Comm_size is: "); StdLog.Tab; StdLog.Int(h); StdLog.Ln;
		h := Kernel.ThisDllObj(mproc, 0, "msmpi", "MPI_Comm_rank");
		StdLog.String("address of Comm_rank is: "); StdLog.Tab; StdLog.Int(h); StdLog.Ln;
		h := Kernel.ThisDllObj(mproc, 0, "msmpi", "MPI_Barrier");
		StdLog.String("address of Barrier is: "); StdLog.Tab; StdLog.Int(h); StdLog.Ln;
	END Do;

END TestDll.

TestDll.LoadMPI

TestDll.Do

MODULE Simple;
	(* simple windows application using MPI *)

	IMPORT SYSTEM, WinApi, MPI, Strings;

	TYPE
		RealArray = POINTER TO ARRAY OF REAL;

	CONST
		iconId = 1;

	VAR
		instance: WinApi.HINSTANCE;
		mainWnd: WinApi.HWND;
		i, n, nProc, rank, newRank, colour: INTEGER;
		args: POINTER TO ARRAY[untagged] OF SHORTCHAR;
		message, title: ARRAY 128 OF SHORTCHAR;
		s, rankString: ARRAY 64 OF CHAR;
		operand, result, sendbuf, recvbuf: RealArray;
		newCom: MPI.Comm;

	PROCEDURE New (OUT a: RealArray; len: INTEGER);
		VAR
			h: INTEGER;
	BEGIN
		h := WinApi.LocalAlloc({}, len * SIZE(REAL));
		a := SYSTEM.VAL(RealArray, h)
	END New;

	PROCEDURE WndHandler (wnd, msg, wParam, lParam: INTEGER): INTEGER;
		VAR res: INTEGER; ps: WinApi.PAINTSTRUCT; dc: WinApi.HDC;
	BEGIN
		IF msg = WinApi.WM_DESTROY THEN
			WinApi.PostQuitMessage(0)
		ELSIF msg = WinApi.WM_PAINT THEN
			dc := WinApi.BeginPaint(wnd, ps);
			res := WinApi.TextOut(dc, 50, 50, message, LEN(message$));
			res := WinApi.EndPaint(wnd, ps)
		ELSIF msg = WinApi.WM_CHAR THEN
			res := WinApi.Beep(800, 200)
		ELSE
			RETURN WinApi.DefWindowProc(wnd, msg, wParam, lParam)
		END;
		RETURN 0
	END WndHandler;

	PROCEDURE OpenWindow;
		VAR class: WinApi.WNDCLASS; res: INTEGER;
	BEGIN
		class.hCursor := WinApi.LoadCursor(0, SYSTEM.VAL(WinApi.PtrSTR,
		WinApi.IDC_ARROW));
		class.hIcon := WinApi.LoadIcon(instance, SYSTEM.VAL(WinApi.PtrSTR, iconId));
		class.lpszMenuName := NIL;
		class.lpszClassName := "Simple";
		class.hbrBackground := WinApi.GetStockObject(WinApi.WHITE_BRUSH);
		class.style := WinApi.CS_VREDRAW + WinApi.CS_HREDRAW
		(* + WinApi.CS_OWNDC + WinApi.CS_PARENTDC *);
		class.hInstance := instance;
		class.lpfnWndProc := WndHandler;
		class.cbClsExtra := 0;
		class.cbWndExtra := 0;
		res := WinApi.RegisterClass(class);
		mainWnd := WinApi.CreateWindowEx({}, "Simple", title,
		WinApi.WS_OVERLAPPEDWINDOW,
		WinApi.CW_USEDEFAULT, WinApi.CW_USEDEFAULT,
		WinApi.CW_USEDEFAULT, WinApi.CW_USEDEFAULT,
		0, 0, instance, 0);
		res := WinApi.ShowWindow(mainWnd, WinApi.SW_SHOWDEFAULT);
		res := WinApi.UpdateWindow(mainWnd);
	END OpenWindow;

	PROCEDURE MainLoop;
		VAR msg: WinApi.MSG; res: INTEGER;
	BEGIN
		WHILE WinApi.GetMessage(msg, 0, 0, 0) # 0 DO
			res := WinApi.TranslateMessage(msg);
			res := WinApi.DispatchMessage(msg);
		END;
		WinApi.ExitProcess(msg.wParam)
	END MainLoop;

BEGIN
	MPI.Init(n, args);
	(*	test Commranl	*)
	MPI.Comm_rank(MPI.COMM_WORLD, rank);
	Strings.IntToString(rank, rankString);
	message := "Hello World this is " + SHORT(rankString) + " of ";
	(*	test Commsize	*)
	MPI.Comm_size(MPI.COMM_WORLD, nProc);
	Strings.IntToString(nProc, s);
	message := message + SHORT(s) + " processors results is ";
	(*	test Allreduce	*)
	NEW(operand, 1); NEW(result, 1);
	operand[0] := rank;
	MPI.Allreduce(SYSTEM.ADR(operand^), SYSTEM.ADR(result^), 
	1, MPI.DOUBLE, MPI.SUM, MPI.COMM_WORLD);
	Strings.RealToString(result[0], s);
	message := message + SHORT(s);
	(*	test Allgather	*)
	NEW(sendbuf, 2); NEW(recvbuf, 2 * nProc);
	sendbuf[0] := rank;
	colour := 1 + rank DIV 2;
	MPI.Comm_split(MPI.COMM_WORLD, colour, 0, newCom);
	ASSERT(newCom # MPI.COMM_WORLD, 66);
	MPI.Comm_rank(newCom, newRank);
	sendbuf[1] := newRank;
	MPI.Allgather(SYSTEM.ADR(sendbuf^), 2, MPI.DOUBLE, SYSTEM.ADR(recvbuf^), 
	2, MPI.DOUBLE, MPI.COMM_WORLD);
	MPI.Finalize;
	i := 0;
	WHILE i < 2 * nProc DO
		Strings.RealToString(recvbuf[i], s);
		message := message + " " + SHORT(s);
		INC(i)
	END;
	instance := WinApi.GetModuleHandle(NIL);
	title := SHORT("Worker rank " + rankString);
	OpenWindow;
	MainLoop
END Simple.

DevLinker.LinkExe
Simple.exe := Kernel+ Math Strings Simple 1 bugslogo.Ico ~

