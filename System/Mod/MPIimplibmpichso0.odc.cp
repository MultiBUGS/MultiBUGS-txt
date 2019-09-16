(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"
*)

MODULE MPIimplibmpichso0;

	

	IMPORT
		MPI, MPIlibmpichso0, SYSTEM;

	TYPE
		Hook = POINTER TO RECORD (MPI.Hook) END;
		
	VAR
		version-: INTEGER; 	(*	version number	*)
		maintainer-: ARRAY 40 OF CHAR; 	(*	person maintaining module	*)

	PROCEDURE (hook: Hook) Abort* (comm: MPI.Comm; error: INTEGER);
	BEGIN
		MPIlibmpichso0.Abort(comm, error)
	END Abort;

	PROCEDURE (hook: Hook) Allgather* (
	sendbuf: MPI.Address;
	sendCount: INTEGER;
	sendType: MPI.Datatype;
	recvbuf: MPI.Address;
	recvCount: INTEGER;
	recvType: MPI.Datatype;
	comm: MPI.Comm);
	BEGIN
		MPIlibmpichso0.Allgather(sendbuf, sendCount, sendType, recvbuf, recvCount, recvType, comm);
	END Allgather;

	PROCEDURE (hook: Hook) Allreduce* (
	operand: MPI.Address;
	result: MPI.Address;
	count: INTEGER;
	dataType: MPI.Datatype;
	operator: MPI.Op;
	comm: MPI.Comm);
	BEGIN
		MPIlibmpichso0.Allreduce(operand, result, count, dataType, operator, comm);
	END Allreduce;

	PROCEDURE (hook: Hook) Barrier* (comm: MPI.Comm);
	BEGIN

	END Barrier;

	PROCEDURE (hook: Hook) Bcast* (
	buf: MPI.Address;
	count: INTEGER;
	dataType: MPI.Datatype;
	root: INTEGER;
	comm: MPI.Comm);
	BEGIN
		MPIlibmpichso0.Bcast(buf, count, dataType, root, comm);
	END Bcast;

	PROCEDURE (hook: Hook) Close_port* (port: MPI.Address);
	BEGIN
		MPIlibmpichso0.Close_port(port);
	END Close_port;

	PROCEDURE (hook: Hook) Comm_accept* (
	port: MPI.Address;
	info: MPI.Info;
	root: INTEGER;
	comm: MPI.Comm;
	VAR intercom: MPI.Comm);
	BEGIN
		MPIlibmpichso0.Comm_accept(port, info, root, comm, intercom);
	END Comm_accept;

	PROCEDURE (hook: Hook) Comm_connect* (
	port: MPI.Address;
	info: MPI.Info;
	root: INTEGER;
	comm: MPI.Comm;
	VAR intercom: MPI.Comm);
	BEGIN
		MPIlibmpichso0.Comm_connect(port, info, root, comm, intercom);
	END Comm_connect;

	PROCEDURE (hook: Hook) Comm_disconnect* (
	VAR intercom: MPI.Comm);
	BEGIN
		MPIlibmpichso0.Comm_disconnect(intercom);
	END Comm_disconnect;

	PROCEDURE (hook: Hook) Comm_free* (
	VAR intercom: MPI.Comm);
	BEGIN
		MPIlibmpichso0.Comm_free(intercom);
	END Comm_free;

	PROCEDURE (hook: Hook) Comm_get_parent* (
	VAR parent: MPI.Comm);
	BEGIN
		MPIlibmpichso0.Comm_get_parent(parent);
	END Comm_get_parent;

	PROCEDURE (hook: Hook) Comm_rank* (
	comm: MPI.Comm; VAR rank: INTEGER);
	BEGIN
		MPIlibmpichso0.Comm_rank(comm, rank);
	END Comm_rank;

	PROCEDURE (hook: Hook) Comm_remote_size* (
	parent: MPI.Comm; VAR size: INTEGER);
	BEGIN
		MPIlibmpichso0.Comm_remote_size(parent, size);
	END Comm_remote_size;

	PROCEDURE (hook: Hook) Comm_size* (
	comm: MPI.Comm; VAR numProc: INTEGER);
	BEGIN
		MPIlibmpichso0.Comm_size(comm, numProc);
	END Comm_size;

	PROCEDURE (hook: Hook) Comm_split* (
	oldCom: MPI.Comm;
	colour: INTEGER;
	rankKey: INTEGER;
	VAR newCom: MPI.Comm);
	BEGIN
		MPIlibmpichso0.Comm_split(oldCom, colour, rankKey, newCom);
	END Comm_split;

	PROCEDURE (hook: Hook) Comm_spawn* (
	worker: MPI.Address;
	argv: MPI.Address;
	maxproc: INTEGER;
	info: MPI.Info;
	root: INTEGER;
	comm: MPI.Comm;
	VAR intercomm: MPI.Comm;
	errors: MPI.Address
	);
	BEGIN
		MPIlibmpichso0.Comm_spawn(worker, argv, maxproc, info, root, comm, intercomm, errors)
	END Comm_spawn;

	PROCEDURE (hook: Hook) Finalize*;
	BEGIN
		MPIlibmpichso0.Finalize
	END Finalize;

	PROCEDURE (hook: Hook) Gather* (
	sendBuf: MPI.Address;
	sendCount: INTEGER;
	sendType: MPI.Datatype;
	recBuf: MPI.Address;
	recCount: INTEGER;
	recType: MPI.Datatype;
	root: INTEGER;
	comm: MPI.Comm);
	BEGIN
		MPIlibmpichso0.Gather(sendBuf, sendCount, sendType, recBuf, recCount, recType, root, comm);
	END Gather;

	PROCEDURE (hook: Hook) Init* (
	VAR nargs: INTEGER;
	VAR args: POINTER TO ARRAY[untagged] OF SHORTCHAR);
	BEGIN
		MPIlibmpichso0.Init(nargs, args)
	END Init;

	PROCEDURE (hook: Hook) Open_port* (
	info: MPI.Info; port: MPI.Address);
	BEGIN
		MPIlibmpichso0.Open_port(info, port);
	END Open_port;

	PROCEDURE (hook: Hook) Recv* (
	buff: MPI.Address;
	count: INTEGER;
	datatype: MPI.Datatype;
	source: INTEGER;
	tag: INTEGER;
	comm: MPI.Comm;
	status: MPI.Status);
	BEGIN
		MPIlibmpichso0.Recv(buff, count, datatype, source, tag, comm, status);
	END Recv;

	PROCEDURE (hook: Hook) Send* (
	buff: MPI.Address;
	count: INTEGER;
	datatype: MPI.Datatype;
	dest: INTEGER;
	tag: INTEGER;
	comm: MPI.Comm);
	BEGIN
		MPIlibmpichso0.Send(buff, count, datatype, dest, tag, comm);
	END Send;

	PROCEDURE (hook: Hook) Wtime* (): REAL;
	BEGIN
		RETURN MPIlibmpichso0.Wtime()
	END Wtime;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			h: Hook;
	BEGIN
		NEW(h);
		MPI.SetHook(h);
		Maintainer
	END Init;

BEGIN
	Init
END MPIimplibmpichso0.
