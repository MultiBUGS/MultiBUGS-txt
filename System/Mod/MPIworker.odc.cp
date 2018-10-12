(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"


*)

(*	This module implements MPI stuff used on the worker side	*)



MODULE MPIworker;

	IMPORT
		MPI, SYSTEM;

	VAR
		mpi: MPI.Hook;
		comm, intercomm: MPI.Comm;
		buffer: POINTER TO ARRAY OF REAL;
		chain-, commSize-, rank-, worldRank-, worldSize-: INTEGER;
		version-: INTEGER; 	(*	version number	*)
		maintainer-: ARRAY 40 OF CHAR; 	(*	person maintaining module	*)

	PROCEDURE RankToChain (rank, numChains, numProc: INTEGER): INTEGER;
		VAR
			c: INTEGER;
	BEGIN
		c := rank DIV (numProc DIV numChains);
		RETURN c
	END RankToChain;

	PROCEDURE Abort*;
		CONST
			error = 0;
	BEGIN
		mpi.Abort(comm, error)
	END Abort;

	PROCEDURE AllGather* (IN x: ARRAY OF REAL; nElem: INTEGER; OUT y: ARRAY OF REAL);
	BEGIN
		mpi.Allgather(SYSTEM.ADR(x[0]), nElem, MPI.DOUBLE,
		SYSTEM.ADR(y[0]), nElem, MPI.DOUBLE, comm);
	END AllGather;

	PROCEDURE Barrier*;
	BEGIN
		IF commSize > 1 THEN mpi.Barrier(comm) END;
	END Barrier;

	PROCEDURE FinalizeMPI*;
	BEGIN
		mpi.Comm_disconnect(intercomm);
		ASSERT(intercomm = MPI.COMM_NULL, 21);
		mpi.Finalize
	END FinalizeMPI;

	PROCEDURE CommRemoteSize* (): INTEGER;
		VAR
			size: INTEGER;
	BEGIN
		mpi.Comm_remote_size(intercomm, size);
		RETURN size;
	END CommRemoteSize;

	PROCEDURE InitMPI* (numChains: INTEGER);
		VAR
			args: POINTER TO ARRAY[untagged] OF SHORTCHAR;
			nargs: INTEGER;
			size: INTEGER;
	BEGIN
		mpi.Init(nargs, args);
		mpi.Comm_rank(MPI.COMM_WORLD, worldRank);
		mpi.Comm_size(MPI.COMM_WORLD, worldSize);
		chain := RankToChain(worldRank, numChains, worldSize);
		IF numChains > 1 THEN
			mpi.Comm_split(MPI.COMM_WORLD, 1 + chain, worldRank, comm)
		ELSE
			comm := MPI.COMM_WORLD;
		END;
		mpi.Comm_size(comm, commSize);
		mpi.Comm_rank(comm, rank);
		mpi.Comm_get_parent(intercomm);
		mpi.Comm_remote_size(intercomm, size);
		ASSERT(size = 1, 60);
	END InitMPI;

	PROCEDURE MaxReal* (x: REAL): REAL;
		VAR
			temp: REAL;
	BEGIN
		mpi.Allreduce(SYSTEM.ADR(x), SYSTEM.ADR(temp), 1, MPI.DOUBLE, MPI.MAX, comm);
		RETURN temp
	END MaxReal;

	PROCEDURE RecvIntegers* (OUT x: ARRAY OF INTEGER);
		VAR
			len: INTEGER;
		CONST
			source = 0; tag = 2;
	BEGIN
		len := LEN(x);
		mpi.Recv(SYSTEM.ADR(x[0]), len, MPI.INT, source, tag, intercomm, MPI.STATUS_IGNORE);
	END RecvIntegers;

	PROCEDURE SendInteger* (x: INTEGER);
		CONST
			dest = 0; tag = 0;
	BEGIN
		mpi.Send(SYSTEM.ADR(x), 1, MPI.INT, dest, tag, intercomm);
	END SendInteger;

	PROCEDURE SendIntegers* (IN x: ARRAY OF INTEGER);
		VAR
			len: INTEGER;
		CONST
			dest = 0; tag = 1;
	BEGIN
		len := LEN(x);
		mpi.Send(SYSTEM.ADR(x[0]), len, MPI.INT, dest, tag, intercomm);
	END SendIntegers;

	PROCEDURE SendReal* (x: REAL);
		CONST
			dest = 0; tag = 2;
	BEGIN
		mpi.Send(SYSTEM.ADR(x), 1, MPI.DOUBLE, dest, tag, intercomm)
	END SendReal;

	PROCEDURE SendReals* (IN x: ARRAY OF REAL);
		VAR
			len: INTEGER;
		CONST
			dest = 0; tag = 3;
	BEGIN
		len := LEN(x);
		mpi.Send(SYSTEM.ADR(x[0]), len, MPI.DOUBLE, dest, tag, intercomm);
	END SendReals;

	PROCEDURE SumReal* (x: REAL): REAL;
		VAR
			temp: REAL;
	BEGIN
		mpi.Allreduce(SYSTEM.ADR(x), SYSTEM.ADR(temp), 1, MPI.DOUBLE, MPI.SUM, comm);
		RETURN temp
	END SumReal;

	PROCEDURE SumReals* (VAR x: ARRAY OF REAL);
		VAR
			i, len: INTEGER;
	BEGIN
		len := LEN(x);
		IF len > LEN(buffer) THEN NEW(buffer, len) END;
		mpi.Allreduce(SYSTEM.ADR(x[0]), SYSTEM.ADR(buffer[0]), len, MPI.DOUBLE, MPI.SUM, comm);
		i := 0; WHILE i < len DO x[i] := buffer[i]; INC(i) END
	END SumReals;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		CONST
			len = 100;
	BEGIN
		Maintainer;
		mpi := MPI.hook;
		rank := - 1;
		commSize := - 1;
		worldRank := - 1;
		worldSize := - 1;
		NEW(buffer, len);
		intercomm := MPI.COMM_NULL;
		comm := MPI.COMM_NULL
	END Init;

BEGIN
	Init
END MPIworker.

