(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"


*)

(*	This is the worker program for the parallel version of the BUGS software using MPI.
The worker program of rank zero sends new sampled values back to the master	*)



MODULE ParallelWorker;

	IMPORT
		Files, Kernel, MPIimp, MPIworker, Services, Stores, Strings,
		BugsRandnum,
		ParallelActions, ParallelRandnum;

	CONST
		fileStemName = "Bugs";
		update = 0;
		terminate = 1;
		recvMonitorInfo = 2;
		sendMCMCState = 3;
		toggleWAIC = 4;

	TYPE
		Command = ARRAY 5 OF INTEGER;

	VAR
		sampleSize: INTEGER;
		devianceMonitored, waicSet, devianceExists: BOOLEAN;
		informationCriteria: ARRAY 2 OF REAL;
		deviance, meanDeviance, meanDeviance2: REAL;
		col, row: POINTER TO ARRAY OF INTEGER;
		monitoredValues: POINTER TO ARRAY OF REAL;

		version-: INTEGER; 	(*	version number	*)
		maintainer-: ARRAY 40 OF CHAR; 	(*	person maintaining module	*)

	PROCEDURE Update (thin, iteration: INTEGER; overRelax: BOOLEAN; endOfAdapting: INTEGER);
		VAR
			i: INTEGER;
			res: SET;
	BEGIN
		i := 0;
		res := {};
		WHILE (i < thin) & (res = {}) DO
			ParallelActions.Update(overRelax, res);
			INC(i)
		END;
		ASSERT(res = {}, 77);
		IF endOfAdapting > iteration THEN
			IF ~ParallelActions.IsAdapting() THEN
				endOfAdapting := iteration + 1
			END
		END;
		MPIworker.SendInteger(endOfAdapting);
		IF devianceMonitored OR waicSet THEN
			deviance := ParallelActions.Deviance();
			deviance := MPIworker.SumReal(deviance)
		END;
		IF waicSet THEN
			ParallelActions.UpdateWAIC;
			INC(sampleSize);
			meanDeviance := meanDeviance + (deviance - meanDeviance) / sampleSize;
			meanDeviance2 := meanDeviance2 + (deviance * deviance - meanDeviance2) / sampleSize
		END;
		IF MPIworker.rank = 0 THEN
			IF col # NIL THEN
				ParallelActions.CollectMonitored(col, row, monitoredValues);
				MPIworker.SendReals(monitoredValues);
			END;
			IF devianceMonitored THEN
				MPIworker.SendReal(deviance)
			END;
		END;
	END Update;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Read;
		VAR
			chain, commSize, numChains, rank, worldRank, worldSize, i: INTEGER;
			pos: POINTER TO ARRAY OF INTEGER;
			worldRankString: ARRAY 64 OF CHAR;
			f: Files.File;
			rd: Stores.Reader;
			loc: Files.Locator;
			allThis: BOOLEAN;
	BEGIN
		devianceMonitored := FALSE;
		waicSet := FALSE;
		loc := Files.dir.This("");
		f := Files.dir.Old(loc, fileStemName + ".bug", Files.shared);
		ASSERT(f # NIL, 21);
		rd.ConnectTo(f);
		rd.SetPos(0);
		rd.ReadBool(devianceExists);
		BugsRandnum.InternalizeRNGenerators(rd);
		rd.ReadBool(allThis);
		numChains := BugsRandnum.numberChains;
		MPIworker.InitMPI(numChains);
		chain := MPIworker.chain;
		rank := MPIworker.rank;
		worldRank := MPIworker.worldRank;
		commSize := MPIworker.commSize;
		worldSize := MPIworker.worldSize;
		ParallelRandnum.SetUp(chain, worldRank, numChains, worldSize);
		Strings.IntToString(worldRank, worldRankString);
		NEW(pos, commSize);
		i := 0; WHILE i < commSize DO rd.ReadInt(pos[i]); INC(i) END;
		rd.SetPos(pos[rank]); 
		ParallelActions.Read(chain, rd);
		rd.ConnectTo(NIL);
		f.Close;
		f := NIL
	END Read;

	PROCEDURE Init;
		VAR
			action, endOfAdapting, iteration, memory, setUpTime, thin, numMonitored: INTEGER;
			resultParams: ARRAY 3 OF INTEGER;
			command: Command;
			buffer: ARRAY 4 OF REAL;
			endTime, startTime: LONGINT;
			overRelax: BOOLEAN;
	BEGIN
		Maintainer;
		startTime := Services.Ticks();
		Read;
		endTime := Services.Ticks();
		setUpTime := SHORT(endTime - startTime);
		memory := Kernel.Allocated();
		resultParams[0] := setUpTime;
		resultParams[1] := memory;
		resultParams[2] := MPIworker.rank;
		MPIworker.SendIntegers(resultParams);
		ParallelActions.LoadSample;
		(*	loop to handle request from master	*)
		LOOP
			MPIworker.RecvIntegers(command);
			action := command[0];
			CASE action OF
			|recvMonitorInfo:
				numMonitored := command[1];
				devianceMonitored := command[2] > 0;
				IF (MPIworker.rank = 0) & (numMonitored > 0) THEN
					NEW(col, numMonitored); NEW(row, numMonitored);
					NEW(monitoredValues, numMonitored);
					MPIworker.RecvIntegers(col);
					MPIworker.RecvIntegers(row)
				ELSE
					col := NIL; row := NIL; monitoredValues := NIL
				END;
				MPIworker.RecvIntegers(command);
				thin := command[1]; iteration := command[2]; endOfAdapting := command[3];
				overRelax := command[4] = 1;
				Update(thin, iteration, overRelax, endOfAdapting);
			|sendMCMCState:
				IF devianceExists THEN
					IF ~devianceMonitored & ~waicSet THEN
						(*	deviance not calculated in Update so calculate it	*)
						deviance := ParallelActions.Deviance();
						deviance := MPIworker.SumReal(deviance);
					END;
					IF waicSet THEN
						ParallelActions.CalculateWAIC(informationCriteria[0], informationCriteria[1]);
						MPIworker.SumReals(informationCriteria);
					END
				END;
				IF MPIworker.rank = 0 THEN
					ParallelActions.SendSample;
					IF devianceExists THEN
						MPIworker.SendReal(deviance);
						IF waicSet THEN
							buffer[0] := informationCriteria[0];
							buffer[1] := informationCriteria[1];
							buffer[2] := meanDeviance;
							buffer[3] := meanDeviance2;
							MPIworker.SendReals(buffer)
						END
					END
				END
			|terminate:
				EXIT
			|update:
				thin := command[1]; iteration := command[2]; endOfAdapting := command[3];
				overRelax := command[4] = 1;
				Update(thin, iteration, overRelax, endOfAdapting);
			|toggleWAIC:
				waicSet := ~waicSet;
				IF waicSet THEN
					ParallelActions.SetWAIC;
					sampleSize := 0;
					meanDeviance := 0.0;
					meanDeviance2 := 0.0
				ELSE
					ParallelActions.ClearWAIC;
				END;
			END
		END;
		MPIworker.Barrier;
		MPIworker.FinalizeMPI
	END Init;

BEGIN
	Init
END ParallelWorker.

