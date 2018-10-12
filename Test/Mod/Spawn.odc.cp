MODULE TestSpawn;


	

	IMPORT 
		Dialog, MPI, SYSTEM, MPImaster;

	VAR
		args: POINTER TO ARRAY[untagged] OF SHORTCHAR;
		nargs, rank, worldSize, source, j, res: INTEGER;

	PROCEDURE Spawn*;
		VAR
			worker: ARRAY 64 OF SHORTCHAR;
			size: INTEGER;
	BEGIN
		MPI.Init(nargs, args);
		MPI.Comm_size(MPI.COMM_WORLD, size);
		worker := "Test.exe";
		MPImaster.Spawn(worker, 6);
		HALT(0);
		MPI.Finalize;
	END Spawn;

	PROCEDURE Spawn1*;
		VAR
			command: ARRAY 128 OF CHAR;
	BEGIN
		command := "mpiexec -n 6 Test.exe";
		Dialog.RunExternal(command)
	END Spawn1;

END TestSpawn.

TestSpawn.Spawn

TestSpawn.Spawn1


