(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"
*)

MODULE MPI;

	

	IMPORT
		SYSTEM, WinApi;

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
		hook: Hook;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		comm: Comm;

	PROCEDURE (h: Hook) Abort- (comm: Comm; error: INTEGER), NEW, ABSTRACT;

		PROCEDURE (h: Hook) Allgather- (
		sendbuf: Address;
		sendCount: INTEGER;
		sendType: Datatype;
		recvbuf: Address;
		recvCount: INTEGER;
		recvType: Datatype;
	comm: Comm), NEW, ABSTRACT;

		PROCEDURE (h: Hook) Allreduce- (
		operand: Address;
		result: Address;
		count: INTEGER;
		dataType: Datatype;
		operator: Op;
	comm: Comm), NEW, ABSTRACT;

	PROCEDURE (h: Hook) Barrier- (comm: Comm), NEW, ABSTRACT;

		PROCEDURE (h: Hook) Bcast- (
		buf: Address;
		count: INTEGER;
		dataType: Datatype;
		root: INTEGER;
	comm: Comm), NEW, ABSTRACT;

	PROCEDURE (h: Hook) Close_port- (port: Address), NEW, ABSTRACT;

		PROCEDURE (h: Hook) Comm_accept- (
		port: Address;
		info: Info;
		root: INTEGER;
		comm: Comm;
	VAR intercom: Comm), NEW, ABSTRACT;

		PROCEDURE (h: Hook) Comm_connect- (
		port: Address;
		info: Info;
		root: INTEGER;
		comm: Comm;
	VAR intercom: Comm), NEW, ABSTRACT;

		PROCEDURE (h: Hook) Comm_disconnect- (
	VAR intercom: Comm), NEW, ABSTRACT;

		PROCEDURE (h: Hook) Comm_free- (
	VAR intercom: Comm), NEW, ABSTRACT;

		PROCEDURE (h: Hook) Comm_get_parent- (
	VAR parent: Comm), NEW, ABSTRACT;

		PROCEDURE (h: Hook) Comm_rank- (
	comm: Comm; VAR rank: INTEGER), NEW, ABSTRACT;

		PROCEDURE (h: Hook) Comm_remote_size- (
	parent: Comm; VAR size: INTEGER), NEW, ABSTRACT;

		PROCEDURE (h: Hook) Comm_size- (
	comm: Comm; VAR numProc: INTEGER), NEW, ABSTRACT;

		PROCEDURE (h: Hook) Comm_split- (
		oldComm: Comm;
		colour: INTEGER;
		rankKey: INTEGER;
	VAR newCom: Comm), NEW, ABSTRACT;

		PROCEDURE (h: Hook) Comm_spawn- (
		worker: Address;
		argv: Address;
		maxproc: INTEGER;
		info: Info;
		root: INTEGER;
		comm: Comm;
		VAR intercomm: Comm;
		errors: Address
	), NEW, ABSTRACT;

	PROCEDURE (h: Hook) Finalize-, NEW, ABSTRACT;

		PROCEDURE (h: Hook) Gather- (
		sendBuf: Address;
		sendCount: INTEGER;
		sendType: Datatype;
		recBuf: Address;
		recCount: INTEGER;
		recType: Datatype;
		root: INTEGER;
	comm: Comm), NEW, ABSTRACT;

		PROCEDURE (h: Hook) Init- (
		VAR nargs: INTEGER;
	VAR args: POINTER TO ARRAY[untagged] OF SHORTCHAR), NEW, ABSTRACT;

		PROCEDURE (h: Hook) Open_port- (
	info: Info; port: Address), NEW, ABSTRACT;

		PROCEDURE (h: Hook) Recv- (
		buff: Address;
		count: INTEGER;
		datatype: Datatype;
		source: INTEGER;
		tag: INTEGER;
		comm: Comm;
	status: Status), NEW, ABSTRACT;

		PROCEDURE (h: Hook) Send- (
		buff: Address;
		count: INTEGER;
		datatype: Datatype;
		dest: INTEGER;
		tag: INTEGER;
	comm: Comm), NEW, ABSTRACT;

	PROCEDURE (h: Hook) Wtime- (): REAL, NEW, ABSTRACT;

	PROCEDURE Abort* (comm: Comm; error: INTEGER);
	BEGIN
		hook.Abort(comm, error)
	END Abort;

	PROCEDURE Allgather* (
	sendbuf: Address;
	sendCount: INTEGER;
	sendType: Datatype;
	recvbuf: Address;
	recvCount: INTEGER;
	recvType: Datatype;
	comm: Comm);
	BEGIN
		hook.Allgather(sendbuf, sendCount, sendType, recvbuf, recvCount, recvType, comm)
	END Allgather;

	PROCEDURE Allreduce* (
	operand: Address;
	result: Address;
	count: INTEGER;
	dataType: Datatype;
	operator: Op;
	comm: Comm);
	BEGIN
		hook.Allreduce(operand, result, count, dataType, operator, comm)
	END Allreduce;

	PROCEDURE Barrier* (comm: Comm);
	BEGIN
		hook.Barrier(comm)
	END Barrier;

	PROCEDURE Bcast* (
	buf: Address;
	count: INTEGER;
	dataType: Datatype;
	root: INTEGER;
	comm: Comm);
	BEGIN
		hook.Bcast(buf, count, dataType, root, comm)
	END Bcast;

	PROCEDURE Close_port* (port: Address);
	BEGIN
		hook.Close_port(port)
	END Close_port;

	PROCEDURE Comm_accept* (
	port: Address;
	info: Info;
	root: INTEGER;
	comm: Comm;
	VAR intercom: Comm);
	BEGIN
		hook.Comm_accept(port, info, root, comm, intercom)
	END Comm_accept;

	PROCEDURE Comm_connect* (
	port: Address;
	info: Info;
	root: INTEGER;
	comm: Comm;
	VAR intercom: Comm);
	BEGIN
		hook.Comm_connect(port, info, root, comm, intercom)
	END Comm_connect;

	PROCEDURE Comm_disconnect* (
	VAR intercom: Comm);
	BEGIN
		hook.Comm_disconnect(intercom)
	END Comm_disconnect;

	PROCEDURE Comm_free* (
	VAR intercom: Comm);
	BEGIN
		hook.Comm_free(intercom)
	END Comm_free;

	PROCEDURE Comm_get_parent* (
	VAR parent: Comm);
	BEGIN
		hook.Comm_get_parent(parent)
	END Comm_get_parent;

	PROCEDURE Comm_rank* (
	comm: Comm; VAR rank: INTEGER);
	BEGIN
		hook.Comm_rank(comm, rank)
	END Comm_rank;

	PROCEDURE Comm_remote_size* (
	parent: Comm; VAR size: INTEGER);
	BEGIN
		hook.Comm_remote_size(parent, size)
	END Comm_remote_size;

	PROCEDURE Comm_size* (
	comm: Comm; VAR numProc: INTEGER);
	BEGIN
		hook.Comm_size(comm, numProc)
	END Comm_size;

	PROCEDURE Comm_split* (
	oldComm: Comm;
	colour: INTEGER;
	rankKey: INTEGER;
	VAR newCom: Comm);
	BEGIN
		hook.Comm_split(oldComm, colour, rankKey, newCom)
	END Comm_split;

	PROCEDURE Comm_spawn* (
	worker: Address;
	argv: Address;
	maxproc: INTEGER;
	info: Info;
	root: INTEGER;
	comm: Comm;
	VAR intercomm: Comm;
	errors: Address
	);
	BEGIN
		hook.Comm_spawn(worker, argv, maxproc, info, root, comm, intercomm, errors)
	END Comm_spawn;

	PROCEDURE Finalize*;
	BEGIN
		hook.Finalize
	END Finalize;

	PROCEDURE Gather* (
	sendBuf: Address;
	sendCount: INTEGER;
	sendType: Datatype;
	recBuf: Address;
	recCount: INTEGER;
	recType: Datatype;
	root: INTEGER;
	comm: Comm);
	BEGIN
		hook.Gather(sendBuf, sendCount, sendType, recBuf, recCount, recType, root, comm)
	END Gather;

	PROCEDURE Init* (
	VAR nargs: INTEGER;
	VAR args: POINTER TO ARRAY[untagged] OF SHORTCHAR);
	BEGIN
		hook.Init(nargs, args)
	END Init;

	PROCEDURE Open_port* (
	info: Info; port: Address);
	BEGIN
		hook.Open_port(info, port)
	END Open_port;

	PROCEDURE Recv* (
	buff: Address;
	count: INTEGER;
	datatype: Datatype;
	source: INTEGER;
	tag: INTEGER;
	comm: Comm;
	status: Status);
	BEGIN
		hook.Recv(buff, count, datatype, source, tag, comm, status)
	END Recv;

	PROCEDURE RunningUnderMPI* (): BOOLEAN;
		VAR
			out: POINTER TO ARRAY OF CHAR;
			envVarValue: WinApi.PtrWSTR;
	BEGIN
		NEW(out, 256);
		envVarValue := out^;
		RETURN WinApi.GetEnvironmentVariableW("PMI_SIZE", envVarValue, 256) # 0;
	END RunningUnderMPI;

	PROCEDURE Send* (
	buff: Address;
	count: INTEGER;
	datatype: Datatype;
	dest: INTEGER;
	tag: INTEGER;
	comm: Comm);
	BEGIN
		hook.Send(buff, count, datatype, dest, tag, comm)
	END Send;

	PROCEDURE Wtime* (): REAL;
	BEGIN
		RETURN hook.Wtime()
	END Wtime;

	PROCEDURE SetHook* (h: Hook);
	BEGIN
		hook := h
	END SetHook;


	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas";
	END Maintainer;

	PROCEDURE InitModule;
	BEGIN
		Maintainer;
		hook := NIL
	END InitModule;

BEGIN
	InitModule
END MPI.
