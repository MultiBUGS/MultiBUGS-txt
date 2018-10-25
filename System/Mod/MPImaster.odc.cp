(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"


*)

(*	This module implements MPI stuff used on the master side	*)



MODULE MPImaster;

	IMPORT
		MPI, MPIimp, SYSTEM;

	VAR
		mpi: MPI.Hook;
		intercomm: MPI.Comm;
		version-: INTEGER; 	(*	version number	*)
		maintainer-: ARRAY 40 OF CHAR; 	(*	person maintaining module	*)

	PROCEDURE CommDisconnect*;
	BEGIN
		IF intercomm # MPI.COMM_NULL THEN
			mpi.Comm_disconnect(intercomm)
		END
	END CommDisconnect;

	PROCEDURE CommRemoteSize* (): INTEGER;
		VAR
			size: INTEGER;
	BEGIN
		mpi.Comm_remote_size(intercomm, size);
		RETURN size;
	END CommRemoteSize;

	PROCEDURE RecvInteger* (source: INTEGER): INTEGER;
		VAR
			x: INTEGER;
		CONST
			tag = 0;
	BEGIN
		mpi.Recv(SYSTEM.ADR(x), 1, MPI.INT, source, tag, intercomm, MPI.STATUS_IGNORE);
		RETURN x
	END RecvInteger;

	PROCEDURE RecvIntegers* (OUT x: ARRAY OF INTEGER; source: INTEGER);
		VAR
			len: INTEGER;
		CONST
			tag = 1;
	BEGIN
		len := LEN(x);
		mpi.Recv(SYSTEM.ADR(x[0]), len, MPI.INT, source, tag, intercomm, MPI.STATUS_IGNORE);
	END RecvIntegers;

	PROCEDURE RecvReal* (source: INTEGER): REAL;
		VAR
			x: REAL;
		CONST
			tag = 2;
	BEGIN
		mpi.Recv(SYSTEM.ADR(x), 1, MPI.DOUBLE, source, tag, intercomm, MPI.STATUS_IGNORE);
		RETURN x
	END RecvReal;

	PROCEDURE RecvReals* (OUT x: ARRAY OF REAL; source: INTEGER);
		VAR
			len: INTEGER;
		CONST
			tag = 3;
	BEGIN
		len := LEN(x);
		mpi.Recv(SYSTEM.ADR(x[0]), len, MPI.DOUBLE, source, tag, intercomm, MPI.STATUS_IGNORE);
	END RecvReals;

	PROCEDURE SendIntegers* (IN x: ARRAY OF INTEGER; dest: INTEGER);
		VAR
			len: INTEGER;
		CONST
			tag = 2;
	BEGIN
		len := LEN(x);
		mpi.Send(SYSTEM.ADR(x[0]), len, MPI.INT, dest, tag, intercomm)
	END SendIntegers;

	PROCEDURE Spawn* (IN worker: ARRAY OF SHORTCHAR; numWorker: INTEGER);

	BEGIN
		mpi.Comm_spawn(SYSTEM.ADR(worker[0]), MPI.ARGV_NULL, numWorker, MPI.INFO_NULL,
		0, MPI.COMM_WORLD, intercomm, MPI.ERRCODES_IGNORE)
	END Spawn;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			nargs: INTEGER;
			args: POINTER TO ARRAY[untagged] OF SHORTCHAR;	
	BEGIN
		Maintainer;
		mpi := MPI.hook;
		mpi.Init(nargs, args);
		intercomm := MPI.COMM_NULL
	END Init;

BEGIN
	Init
CLOSE
	mpi.Finalize
END MPImaster.
