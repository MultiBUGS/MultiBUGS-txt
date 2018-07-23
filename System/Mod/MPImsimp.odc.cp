(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"
*)

MODULE MPImsimp;

	IMPORT
		MPI, MPIlib := MPImslib, SYSTEM;
		
	TYPE
		Hook = POINTER TO RECORD (MPI.Hook) END;

	VAR
		hook: Hook;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		
		PROCEDURE (h: Hook) Allgather  (
		sendbuf: MPI.Address;
		sendCount: INTEGER;
		sendType: MPI.Datatype;
		recvbuf: MPI.Address;
		recvCount: INTEGER;
		recvType: MPI.Datatype;
		comm: MPI.Comm);
		BEGIN
			MPIlib.Allgather(sendbuf, sendCount, sendType, recvbuf, recvCount, recvType, comm)
		END Allgather;
		
		PROCEDURE (h: Hook) Abort (comm: MPI.Comm; error: INTEGER);
		BEGIN
			MPIlib.Abort(comm, error)
		END Abort;
		
		PROCEDURE (h: Hook) Allreduce (
		operand: MPI.Address;
		result: MPI.Address;
		count: INTEGER;
		dataType: MPI.Datatype;
		operator: MPI.Op;
		comm: MPI.Comm);
		BEGIN
			MPIlib.Allreduce(operand, result, count, dataType, operator, comm)
		END Allreduce;
		
		PROCEDURE (h: Hook) Barrier (comm: MPI.Comm);
		BEGIN
			MPIlib.Barrier(comm)
		END Barrier;
		
		PROCEDURE (h: Hook) Bcast (
		buf: MPI.Address;
		count: INTEGER;
		dataType: MPI.Datatype;
		root: INTEGER;
		comm: MPI.Comm);
		BEGIN
			MPIlib.Bcast(buf, count, dataType, root, comm)
		END Bcast;
		
		PROCEDURE (h: Hook) Close_port (port: MPI.Address);
		BEGIN
			MPIlib.Close_port(port)
		END Close_port;
		
		PROCEDURE (h: Hook) Comm_accept (
		port: MPI.Address;
		info: MPI.Info;
		root: INTEGER;
		comm: MPI.Comm;
		VAR intercom: MPI.Comm);
		BEGIN
			MPIlib.Comm_accept(port, info, root, comm, intercom)
		END Comm_accept;
		
		PROCEDURE (h: Hook) Comm_connect (
		port: MPI.Address;
		info: MPI.Info;
		root: INTEGER;
		comm: MPI.Comm;
		VAR intercom: MPI.Comm);
		BEGIN
			MPIlib.Comm_connect(port, info, root, comm, intercom)
		END Comm_connect;
		
		PROCEDURE (h: Hook) Comm_disconnect (
		VAR intercom: MPI.Comm);
		BEGIN
			MPIlib.Comm_disconnect(intercom)
		END Comm_disconnect;
		
		PROCEDURE (h: Hook) Comm_free(
		VAR intercom: MPI.Comm);
		BEGIN
			MPIlib.Comm_free(intercom)
		END Comm_free;
		
		PROCEDURE (h: Hook) Comm_get_parent (
		VAR parent: MPI.Comm);
		BEGIN
			MPIlib.Comm_get_parent(parent)
		END Comm_get_parent;
		
		PROCEDURE (h: Hook) Comm_rank (
		comm: MPI.Comm; VAR rank: INTEGER);
		BEGIN
			MPIlib.Comm_rank(comm, rank)
		END Comm_rank;
		
		PROCEDURE (h: Hook) Comm_remote_size (
		parent: MPI.Comm; VAR size: INTEGER);
		BEGIN
			MPIlib.Comm_remote_size(parent, size)
		END Comm_remote_size;
		
		PROCEDURE (h: Hook) Comm_size (
		comm: MPI.Comm; VAR size: INTEGER);
		BEGIN
			MPIlib.Comm_size(comm, size)
		END Comm_size;
		
		PROCEDURE (h: Hook) Comm_split (
		oldComm: MPI.Comm;
		colour: INTEGER;
		rankKey: INTEGER;
		VAR newCom: MPI.Comm);
		BEGIN
			MPIlib.Comm_split(oldComm, colour, rankKey, newCom)
		END Comm_split;
		
		PROCEDURE (h: Hook) Comm_spawn- (
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
			MPIlib.Comm_spawn(worker, argv, maxproc, info, root, comm, intercomm, errors)
		END Comm_spawn;
		
		PROCEDURE (h: Hook) Finalize;
		BEGIN
			MPIlib.Finalize
		END Finalize;
		
		PROCEDURE (h: Hook) Gather (
		sendBuf: MPI.Address;
		sendCount: INTEGER;
		sendType: MPI.Datatype;
		recBuf: MPI.Address;
		recCount: INTEGER;
		recType: MPI.Datatype;
		root: INTEGER;
		comm: MPI.Comm);
		BEGIN
			MPIlib.Gather(sendBuf, sendCount, sendType, recBuf, recCount, recType, root, comm)
		END Gather;
		
		PROCEDURE (h: Hook) Init (
		VAR nargs: INTEGER;
		VAR args: POINTER TO ARRAY[untagged] OF SHORTCHAR);
		BEGIN
			MPIlib.Init(nargs, args)
		END Init;

		PROCEDURE (h: Hook) Open_port (
		info: MPI.Info; port: MPI.Address);
		BEGIN
			MPIlib.Open_port(info, port)
		END Open_port;
		
		PROCEDURE (h: Hook) Recv (
		buff: MPI.Address;
		count: INTEGER;
		datatype: MPI.Datatype;
		source: INTEGER;
		tag: INTEGER;
		comm: MPI.Comm;
		status: MPI.Status);
		BEGIN
			MPIlib.Recv(buff, count, datatype, source, tag, comm, status)
		END Recv;
		
		PROCEDURE (h: Hook) Send (
		buff: MPI.Address;
		count: INTEGER;
		datatype: MPI.Datatype;
		dest: INTEGER;
		tag: INTEGER;
		comm: MPI.Comm);
		BEGIN
			MPIlib.Send(buff, count, datatype, dest, tag, comm)
		END Send;
		
		PROCEDURE (h: Hook) Wtime (): REAL;
		BEGIN
			RETURN MPIlib.Wtime()
		END Wtime;

		PROCEDURE Maintainer;
		BEGIN
			version := 500;
			maintainer := "A.Thomas";
		END Maintainer;
			
		PROCEDURE Init;
		BEGIN
			Maintainer;
			NEW(hook);
			MPI.SetHook(hook);
			hook := NIL
		END Init;
		
BEGIN
	Init
END MPImsimp.
