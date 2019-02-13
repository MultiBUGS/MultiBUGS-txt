(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"
*)

MODULE MPI;

	

	IMPORT
		SYSTEM;

	TYPE
		Hook* = POINTER TO ABSTRACT RECORD END;

		Address* = INTEGER;

		Comm* = INTEGER;

		Datatype* = INTEGER;

		Info* = INTEGER;

		Op* = INTEGER;

		Status* = INTEGER;

	CONST

		COMM_WORLD* = 44000000H;
		COMM_SELF* = 44000001H;
		COMM_NULL* = 04000000H;

		BOOL* = 4C00010DH;
		CHAR* = 04C000101H;
		DOUBLE* = 4C00080BH;
		INT* = 4C000405H;

		BOR* = 58000008H;
		MAX* = 58000001H;
		SUM* = 58000003H;

		ARGV_NULL* = 0;
		INFO_NULL* = 1C000000H;
		ERRCODES_IGNORE* = 0;

		MAX_PORT_NAME* = 256;

		STATUS_IGNORE* = 1;

	VAR
		hook-: Hook;
		version-: INTEGER; 	(*	version number	*)
		(*maintainer-: ARRAY 40 OF CHAR; *)	(*	person maintaining module	*)

	PROCEDURE (hook: Hook) Abort* (comm: Comm; error: INTEGER), NEW, ABSTRACT;

		PROCEDURE (hook: Hook) Allgather* (
		sendbuf: Address;
		sendCount: INTEGER;
		sendType: Datatype;
		recvbuf: Address;
		recvCount: INTEGER;
		recvType: Datatype;
	comm: Comm), NEW, ABSTRACT;

		PROCEDURE (hook: Hook) Allreduce* (
		operand: Address;
		result: Address;
		count: INTEGER;
		dataType: Datatype;
		operator: Op;
	comm: Comm), NEW, ABSTRACT;

	PROCEDURE (hook: Hook) Barrier* (comm: Comm), NEW, ABSTRACT;

		PROCEDURE (hook: Hook) Bcast* (
		buf: Address;
		count: INTEGER;
		dataType: Datatype;
		root: INTEGER;
	comm: Comm), NEW, ABSTRACT;

	PROCEDURE (hook: Hook) Close_port* (port: Address), NEW, ABSTRACT;

		PROCEDURE (hook: Hook) Comm_accept* (
		port: Address;
		info: Info;
		root: INTEGER;
		comm: Comm;
	VAR intercom: Comm), NEW, ABSTRACT;

		PROCEDURE (hook: Hook) Comm_connect* (
		port: Address;
		info: Info;
		root: INTEGER;
		comm: Comm;
	VAR intercom: Comm), NEW, ABSTRACT;

		PROCEDURE (hook: Hook) Comm_disconnect* (
	VAR intercom: Comm), NEW, ABSTRACT;

		PROCEDURE (hook: Hook) Comm_free* (
	VAR intercom: Comm), NEW, ABSTRACT;

		PROCEDURE (hook: Hook) Comm_get_parent* (
	VAR parent: Comm), NEW, ABSTRACT;

		PROCEDURE (hook: Hook) Comm_rank* (
	comm: Comm; VAR rank: INTEGER), NEW, ABSTRACT;

		PROCEDURE (hook: Hook) Comm_remote_size* (
	parent: Comm; VAR size: INTEGER), NEW, ABSTRACT;

		PROCEDURE (hook: Hook) Comm_size* (
	comm: Comm; VAR numProc: INTEGER), NEW, ABSTRACT;

		PROCEDURE (hook: Hook) Comm_split* (
		oldCom: Comm;
		colour: INTEGER;
		rankKey: INTEGER;
	VAR newCom: Comm), NEW, ABSTRACT;

		PROCEDURE (hook: Hook) Comm_spawn* (
		worker: Address;
		argv: Address;
		maxproc: INTEGER;
		info: Info;
		root: INTEGER;
		comm: Comm;
		VAR intercomm: Comm;
		errors: Address
	), NEW, ABSTRACT;

	PROCEDURE (hook: Hook) Finalize*, NEW, ABSTRACT;

		PROCEDURE (hook: Hook) Gather* (
		sendBuf: Address;
		sendCount: INTEGER;
		sendType: Datatype;
		recBuf: Address;
		recCount: INTEGER;
		recType: Datatype;
		root: INTEGER;
	comm: Comm), NEW, ABSTRACT;


		PROCEDURE (hook: Hook) Init* (
		VAR nargs: INTEGER;
	VAR args: POINTER TO ARRAY[untagged] OF SHORTCHAR), NEW, ABSTRACT;

		PROCEDURE (hook: Hook) Open_port* (
	info: Info; port: Address), NEW, ABSTRACT;

		PROCEDURE (hook: Hook) Recv* (
		buff: Address;
		count: INTEGER;
		datatype: Datatype;
		source: INTEGER;
		tag: INTEGER;
		comm: Comm;
	status: Status), NEW, ABSTRACT;

		PROCEDURE (hook: Hook) Send* (
		buff: Address;
		count: INTEGER;
		datatype: Datatype;
		dest: INTEGER;
		tag: INTEGER;
	comm: Comm), NEW, ABSTRACT;

	PROCEDURE (hook: Hook) Wtime* (): REAL, NEW, ABSTRACT;

	PROCEDURE SetHook* (h: Hook);
	BEGIN
		hook := h
	END SetHook;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		(*maintainer := "A.Thomas"*)
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		hook := NIL;
		Maintainer
	END Init;

BEGIN
	Init
END MPI.

