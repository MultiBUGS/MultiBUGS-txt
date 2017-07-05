(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"


*)

(*	This module implements MPI stuff used on the worker side	*)



MODULE MPIworker;

	IMPORT
		MPI, SYSTEM, Stores;

	VAR
		chain-, commSize-, rank-, worldRank-, worldSize-: INTEGER;
		globalValues-, samples-, values-: POINTER TO ARRAY OF REAL;
		monitors-: POINTER TO ARRAY OF INTEGER;
		xIn, xOut: POINTER TO ARRAY OF REAL;
		real: REAL;
		comm, intercomm: MPI.Comm;
		bufferA, bufferRealA, globalParamsA, globalValuesA, monitorsA, paramsA, realA,
		samplesA, portA, valuesA: INTEGER;
		port: ARRAY MPI.MAX_PORT_NAME OF SHORTCHAR;
		buffer: ARRAY 10 OF INTEGER;
		bufferReal: ARRAY 100 OF REAL;

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
		MPI.Abort(comm, error)
	END Abort;
	
	PROCEDURE AllocateStorage* (maxSizeParams, blockSize, numStochastics: INTEGER);
	BEGIN
		NEW(xIn, maxSizeParams);
		NEW(xOut, maxSizeParams);
		IF blockSize > 0 THEN
			NEW(values, blockSize);
			NEW(globalValues, blockSize * commSize);
			valuesA := SYSTEM.ADR(values[0]);
			globalValuesA := SYSTEM.ADR(globalValues[0])
		END;
		IF rank = 0 THEN
			NEW(samples, numStochastics);
			samplesA := SYSTEM.ADR(samples[0]);
			NEW(monitors, numStochastics);
			monitorsA := SYSTEM.ADR(monitors[0])
		END
	END AllocateStorage;

	PROCEDURE ReadPort* (VAR rd: Stores.Reader);
		VAR
			i: INTEGER;
	BEGIN
		i := 0;
		WHILE i < MPI.MAX_PORT_NAME DO
			rd.ReadSChar(port[i]);
			INC(i)
		END
	END ReadPort;

	PROCEDURE ConnectToMaster*;
		VAR
			size: INTEGER;
	BEGIN
		MPI.Comm_connect(portA, MPI.INFO_NULL, 0, MPI.COMM_WORLD, intercomm);
		MPI.Comm_remote_size(intercomm, size);
		ASSERT(size = 1, 60);
	END ConnectToMaster;

	PROCEDURE InitMPI*;
		VAR
			args: POINTER TO ARRAY[untagged] OF SHORTCHAR;
			nargs: INTEGER;
	BEGIN
		MPI.Init(nargs, args);
		MPI.Comm_rank(MPI.COMM_WORLD, worldRank);
		MPI.Comm_size(MPI.COMM_WORLD, worldSize);
	END InitMPI;

	PROCEDURE SetUpMPI* (numChains: INTEGER);
	BEGIN
		(*	set up comminicators for each chain	*)
		chain := RankToChain(worldRank, numChains, worldSize);
		IF numChains > 1 THEN
			MPI.Comm_split(MPI.COMM_WORLD, 1 + chain, worldRank, comm)
		ELSE
			comm := MPI.COMM_WORLD;
		END;
		MPI.Comm_size(comm, commSize);
		MPI.Comm_rank(comm, rank);
	END SetUpMPI;

	PROCEDURE ReceiveInteger* (OUT int: INTEGER);
	BEGIN
		MPI.Recv(bufferA, 1, MPI.DOUBLE, 0, 2, intercomm, MPI.STATUS_IGNORE);
		int := buffer[0]
	END ReceiveInteger;

	PROCEDURE ReceiveCommand* (OUT ints: ARRAY OF INTEGER);
		VAR
			i, len: INTEGER;
	BEGIN
		len := LEN(ints);
		MPI.Recv(bufferA, len, MPI.INT, 0, 1, intercomm, MPI.STATUS_IGNORE);
		i := 0;
		WHILE i < len DO
			ints[i] := buffer[i];
			INC(i)
		END
	END ReceiveCommand;

	PROCEDURE ReceiveReal* (OUT x: REAL);
	BEGIN
		MPI.Recv(realA, 1, MPI.DOUBLE, 0, 2, intercomm, MPI.STATUS_IGNORE);
		x := real
	END ReceiveReal;

	PROCEDURE SendIntegers* (IN ints: ARRAY OF INTEGER);
		VAR
			i, len: INTEGER;
	BEGIN
		len := LEN(ints);
		i := 0;
		WHILE i < len DO
			buffer[i] := ints[i];
			INC(i)
		END;
		MPI.Send(bufferA, len, MPI.INT, 0, 0, intercomm);
	END SendIntegers;

	PROCEDURE SendInteger* (int: INTEGER);
	BEGIN
		buffer[0] := int;
		MPI.Send(bufferA, 1, MPI.INT, 0, 0, intercomm);
	END SendInteger;

	PROCEDURE SendReal* (x: REAL);
	BEGIN
		real := x;
		MPI.Send(realA, 1, MPI.DOUBLE, 0, 2, intercomm)
	END SendReal;

	PROCEDURE SendReals* (IN x: ARRAY OF REAL);
		VAR
			i, len: INTEGER;
	BEGIN
		len := LEN(x);
		i := 0;
		WHILE i < len DO
			bufferReal[i] := x[i];
			INC(i)
		END;
		MPI.Send(bufferRealA, len, MPI.DOUBLE, 0, 2, intercomm);
	END SendReals;

	PROCEDURE MaxParams* (nElem: INTEGER);
	BEGIN
		MPI.Allreduce(paramsA, globalParamsA, nElem, MPI.DOUBLE, MPI.MAX, comm);
	END MaxParams;

	PROCEDURE MaxReal* (VAR x: REAL);
	BEGIN
		xIn[0] := x;
		MPI.Allreduce(SYSTEM.ADR(xIn[0]), SYSTEM.ADR(xOut[0]), 1, MPI.DOUBLE, MPI.MAX, comm);
		x := xOut[0]
	END MaxReal;

	PROCEDURE SumReal* (VAR x: REAL);
	BEGIN
		xIn[0] := x;
		MPI.Allreduce(SYSTEM.ADR(xIn[0]), SYSTEM.ADR(xOut[0]), 1, MPI.DOUBLE, MPI.SUM, comm);
		x := xOut[0]
	END SumReal;

	PROCEDURE SumReals* (VAR x: ARRAY OF REAL);
		VAR
			i, n: INTEGER;
	BEGIN
		n := LEN(x);
		i := 0; WHILE i < n DO xIn[i] := x[i]; INC(i) END;
		MPI.Allreduce(SYSTEM.ADR(xIn[0]), SYSTEM.ADR(xOut[0]), n, MPI.DOUBLE, MPI.SUM, comm);
		i := 0; WHILE i < n DO x[i] := xOut[i]; INC(i) END
	END SumReals;

	PROCEDURE GatherValues* (nElem: INTEGER);
	BEGIN
		MPI.Allgather(valuesA, nElem, MPI.DOUBLE, globalValuesA, nElem, MPI.DOUBLE, comm);
	END GatherValues;

	PROCEDURE ReceiveMonitors* (numMonitors: INTEGER);
	BEGIN
		MPI.Recv(monitorsA, numMonitors, MPI.INT, 0, 1, intercomm, MPI.STATUS_IGNORE)
	END ReceiveMonitors;

	PROCEDURE SendSamples* (len: INTEGER);
	BEGIN
		MPI.Send(samplesA, len, MPI.DOUBLE, 0, 2, intercomm)
	END SendSamples;

	PROCEDURE Barrier*;
	BEGIN
		IF commSize > 1 THEN MPI.Barrier(comm) END;
	END Barrier;

	PROCEDURE Close*;
	BEGIN
		MPI.Comm_disconnect(intercomm);
		MPI.Finalize
	END Close;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		globalValues := NIL;
		samples := NIL;
		values := NIL;
		xIn := NIL;
		xOut := NIL;
		bufferA := SYSTEM.ADR(buffer[0]);
		bufferRealA := SYSTEM.ADR(bufferReal[0]);
		portA := SYSTEM.ADR(port[0]);
		realA := SYSTEM.ADR(real);
		rank :=  - 1;
		commSize :=  - 1;
		worldRank :=  - 1;
		worldSize :=  - 1
	END Init;

BEGIN
	Init
END MPIworker.

