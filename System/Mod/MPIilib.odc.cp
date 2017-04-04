(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"
*)

MODULE MPIilib["impi"];

	IMPORT
		MPI;
			
	PROCEDURE Allgather*["MPI_Allgather"] (
	sendbuf: MPI.Address;
	sendCount: INTEGER;
	sendType: MPI.Datatype;
	recvbuf: MPI.Address;
	recvCount: INTEGER;
	recvType: MPI.Datatype;
	comm: MPI.Comm);

	PROCEDURE Allreduce*["MPI_Allreduce"] (
	operand: MPI.Address;
	result: MPI.Address;
	count: INTEGER;
	dataType: MPI.Datatype;
	operator: MPI.Op;
	comm: MPI.Comm);

	PROCEDURE Barrier*["MPI_Barrier"] (comm: MPI.Comm);

	PROCEDURE Bcast*["MPI_Bcast"] (
	buf: MPI.Address;
	count: INTEGER;
	dataType: MPI.Datatype;
	root: INTEGER;
	comm: MPI.Comm);

	PROCEDURE Close_port*["MPI_Close_port"] (port: MPI.Address);

	PROCEDURE Comm_accept*["MPI_Comm_accept"] (
	port: MPI.Address;
	info: MPI.Info;
	root: INTEGER;
	comm: MPI.Comm;
	VAR intercom: MPI.Comm);

	PROCEDURE Comm_connect*["MPI_Comm_connect"] (
	port: MPI.Address;
	info: MPI.Info;
	root: INTEGER;
	comm: MPI.Comm;
	VAR intercom: MPI.Comm);

	PROCEDURE Comm_disconnect*["MPI_Comm_disconnect"] (
	VAR intercom: MPI.Comm);

	PROCEDURE Comm_get_parent*["MPI_Comm_get_parent"] (
	VAR parent: MPI.Comm);

	PROCEDURE Comm_rank*["MPI_Comm_rank"] (
	comm: MPI.Comm; VAR rank: INTEGER);

	PROCEDURE Comm_remote_size*["MPI_Comm_remote_size"] (
	parent: MPI.Comm; VAR size: INTEGER);

	PROCEDURE Comm_size*["MPI_Comm_size"] (
	comm: MPI.Comm; VAR numProc: INTEGER);

	PROCEDURE Comm_split*["MPI_Comm_split"] (
	oldComm: MPI.Comm;
	colour: INTEGER;
	rankKey: INTEGER;
	VAR newCom: MPI.Comm);

	PROCEDURE Finalize*["MPI_Finalize"];

	PROCEDURE Gather*["MPI_Gather"] (
	sendBuf: MPI.Address;
	sendCount: INTEGER;
	sendType: MPI.Datatype;
	recBuf: MPI.Address;
	recCount: INTEGER;
	recType: MPI.Datatype;
	root: INTEGER;
	comm: MPI.Comm);

	PROCEDURE Init*["MPI_Init"] (
	VAR nargs: INTEGER;
	VAR args: POINTER TO ARRAY[untagged] OF SHORTCHAR);

	PROCEDURE Open_port*["MPI_Open_port"] (
	info: MPI.Info; port: MPI.Address);

	PROCEDURE Recv*["MPI_Recv"] (
	buff: MPI.Address;
	count: INTEGER;
	datatype: MPI.Datatype;
	source: INTEGER;
	tag: INTEGER;
	comm: MPI.Comm;
	status: MPI.Status);

	PROCEDURE Send*["MPI_Send"] (
	buff: MPI.Address;
	count: INTEGER;
	datatype: MPI.Datatype;
	dest: INTEGER;
	tag: INTEGER;
	comm: MPI.Comm);
	
	PROCEDURE Wtime*["MPI_Wtime"] (): REAL;

END MPIilib.
