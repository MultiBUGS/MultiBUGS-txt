(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"
*)

MODULE MPI["libmpich.so.12"];

	

	IMPORT
		SYSTEM;
TYPE

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
	PROCEDURE Abort*["MPI_Abort"] (comm: Comm; error: INTEGER);

	PROCEDURE Allgather*["MPI_Allgather"] (
	sendbuf: Address;
	sendCount: INTEGER;
	sendType: Datatype;
	recvbuf: Address;
	recvCount: INTEGER;
	recvType: Datatype;
	comm: Comm);

	PROCEDURE Allreduce*["MPI_Allreduce"] (
	operand: Address;
	result: Address;
	count: INTEGER;
	dataType: Datatype;
	operator: Op;
	comm: Comm);

	PROCEDURE Barrier*["MPI_Barrier"] (comm: Comm);

	PROCEDURE Bcast*["MPI_Bcast"] (
	buf: Address;
	count: INTEGER;
	dataType: Datatype;
	root: INTEGER;
	comm: Comm);

	PROCEDURE Close_port*["MPI_Close_port"] (port: Address);

	PROCEDURE Comm_accept*["MPI_Comm_accept"] (
	port: Address;
	info: Info;
	root: INTEGER;
	comm: Comm;
	VAR intercom: Comm);

	PROCEDURE Comm_connect*["MPI_Comm_connect"] (
	port: Address;
	info: Info;
	root: INTEGER;
	comm: Comm;
	VAR intercom: Comm);

	PROCEDURE Comm_disconnect*["MPI_Comm_disconnect"] (
	VAR intercom: Comm);

	PROCEDURE Comm_free*["MPI_Comm_free"] (
	VAR intercom: Comm);

	PROCEDURE Comm_get_parent*["MPI_Comm_get_parent"] (
	VAR parent: Comm);

	PROCEDURE Comm_rank*["MPI_Comm_rank"] (
	comm: Comm; VAR rank: INTEGER);

	PROCEDURE Comm_remote_size*["MPI_Comm_remote_size"] (
	parent: Comm; VAR size: INTEGER);

	PROCEDURE Comm_size*["MPI_Comm_size"] (
	comm: Comm; VAR numProc: INTEGER);

	PROCEDURE Comm_split*["MPI_Comm_split"] (
	oldComm: Comm;
	colour: INTEGER;
	rankKey: INTEGER;
	VAR newCom: Comm);
	
	PROCEDURE Comm_spawn*["MPI_Comm_spawn"] (
	worker: Address;
	argv: Address;
	maxproc: INTEGER;
	info: Info;
	root: INTEGER;
	comm: Comm;
	VAR intercomm: Comm;
	errors: Address
	);

	PROCEDURE Finalize*["MPI_Finalize"];

	PROCEDURE Gather*["MPI_Gather"] (
	sendBuf: Address;
	sendCount: INTEGER;
	sendType: Datatype;
	recBuf: Address;
	recCount: INTEGER;
	recType: Datatype;
	root: INTEGER;
	comm: Comm);
	
	
	PROCEDURE Init*["MPI_Init"] (
	VAR nargs: INTEGER;
	VAR args: POINTER TO ARRAY[untagged] OF SHORTCHAR);

	PROCEDURE Open_port*["MPI_Open_port"] (
	info: Info; port: Address);

	PROCEDURE Recv*["MPI_Recv"] (
	buff: Address;
	count: INTEGER;
	datatype: Datatype;
	source: INTEGER;
	tag: INTEGER;
	comm: Comm;
	status: Status);

	PROCEDURE Send*["MPI_Send"] (
	buff: Address;
	count: INTEGER;
	datatype: Datatype;
	dest: INTEGER;
	tag: INTEGER;
	comm: Comm);

	PROCEDURE Wtime*["MPI_Wtime"] (): REAL;

END MPI.
