(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"


*)

(*	This is the worker program for the parallel version of the BUGS software using MPI.
The worker program of rank zero sends new sampled values back to the master	*)



MODULE ParallelWorker;

	IMPORT
		Files := Files64, Kernel, MPIworker, Services, Stores := Stores64, Strings,
		BugsRandnum,
		GraphStochastic,
		ParallelActions, ParallelFiles, ParallelHMC, ParallelRandnum;

	CONST
		fileStemName = "Bugs";
		update = 0;
		terminate = 1;
		recvMonitorInfo = 2;
		sendMCMCState = 3;
		setWAIC = 4;
		clearWAIC = 5;
		checkPoint = 6;
		restart = 7;

	TYPE
		Command = ARRAY 5 OF INTEGER;

	VAR
		sampleSize: INTEGER;
		devianceExists, devianceMonitored, seperable, waicSet: BOOLEAN;
		informationCriteria: ARRAY 2 OF REAL;
		deviance, meanDeviance, meanDeviance2: REAL;
		col, row: POINTER TO ARRAY OF INTEGER;
		monitoredValues: POINTER TO ARRAY OF REAL;
		worldRankString: ARRAY 64 OF CHAR;
		numSteps, warmUpPeriod: INTEGER;
		eps: REAL;
		version-: INTEGER; 	(*	version number	*)
		maintainer-: ARRAY 40 OF CHAR; 	(*	person maintaining module	*)

	PROCEDURE Update (thin, iteration: INTEGER; seperable, overRelax: BOOLEAN; endOfAdapting: INTEGER);
		VAR
			i: INTEGER;
			res: SET;
			reject: BOOLEAN;
	BEGIN
		i := 0;
		res := {};
		WHILE (i < thin) & (res = {}) DO
			ParallelActions.Update(seperable, overRelax, res);
			INC(i)
		END;
		ASSERT(res = {}, 77);
		IF endOfAdapting > iteration THEN
			IF ~ParallelActions.IsAdapting() THEN
				endOfAdapting := iteration + 1
			END
		END;
		MPIworker.SendInteger(endOfAdapting);
		(*ParallelHMC.Update(seperable, numSteps, iteration, warmUpPeriod, eps, reject);
		MPIworker.SendInteger(warmUpPeriod + 1);*)
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

	PROCEDURE Read;
		VAR
			chain, commSize, numChains, rank, worldRank, worldSize, i: INTEGER;
			pos: POINTER TO ARRAY OF LONGINT;
			f: Files.File;
			rd: Stores.Reader;
			loc: Files.Locator;
	BEGIN
		devianceMonitored := FALSE;
		waicSet := FALSE;
		ASSERT(Files.dir # NIL, 21);
		loc := Files.dir.This("");
		f := Files.dir.Old(loc, fileStemName + ".bug", Files.shared);
		ASSERT(f # NIL, 21);
		rd.ConnectTo(f);
		rd.SetPos(0);
		rd.ReadBool(devianceExists);
		BugsRandnum.InternalizeRNGenerators(rd);
		rd.ReadBool(seperable);
		IF MPIworker.commSize = 1 THEN seperable := TRUE END;
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
		i := 0; WHILE i < commSize DO rd.ReadLong(pos[i]); INC(i) END;
		rd.SetPos(pos[rank]);
		ParallelActions.Read(chain, rd);
		rd.ConnectTo(NIL);
		f.Close;
		f := NIL
	END Read;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			action, endOfAdapting, i, iteration, memory, mutableSize, numMonitored,
			res, setUpTime, thin: INTEGER;
			resultParams: ARRAY 3 OF INTEGER;
			command: Command;
			buffer: ARRAY 4 OF REAL;
			endTime, startTime: LONGINT;
			overRelax: BOOLEAN;
			p: GraphStochastic.Node;
			fileName: Files.Name;
			f: Files.File;
			loc: Files.Locator;
			rd: Stores.Reader;
			wr: Stores.Writer;
			mutableState: POINTER TO ARRAY OF BYTE;
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
		ParallelActions.LoadSample;(*
		ParallelHMC.Setup;
		numSteps := 8;
		warmUpPeriod := 1000;
		eps := 0.25;*)
		(*	loop to handle request from master	*)
		LOOP
			MPIworker.RecvIntegers(command);
			action := command[0];
			CASE action OF
			|recvMonitorInfo:
				numMonitored := command[1];
				devianceMonitored := command[2] > 0;
				IF (numMonitored > 0) & (MPIworker.rank = 0) THEN
					NEW(monitoredValues, numMonitored);
					NEW(col, numMonitored);
					NEW(row, numMonitored);
					MPIworker.RecvIntegers(col);
					MPIworker.RecvIntegers(row)
				ELSE
					col := NIL; row := NIL; monitoredValues := NIL
				END;
				MPIworker.RecvIntegers(command);
				thin := command[1]; iteration := command[2]; endOfAdapting := command[3];
				overRelax := command[4] = 1;
				Update(thin, iteration, seperable, overRelax, endOfAdapting);
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
				Update(thin, iteration, seperable, overRelax, endOfAdapting);
			|setWAIC:
				waicSet := TRUE;
				ParallelActions.SetWAIC;
				sampleSize := 0;
				meanDeviance := 0.0;
				meanDeviance2 := 0.0
			|clearWAIC:
				waicSet := FALSE;
				ParallelActions.ClearWAIC;
			|checkPoint:
				f := ParallelFiles.dir.Temp();
				ASSERT(f # NIL, 55);
				wr.ConnectTo(f);
				wr.SetPos(0);
				mutableSize := ParallelActions.ExternalizeMutableSize();
				NEW(mutableState, mutableSize);
				ParallelActions.ExternalizeMutable(wr);
				rd.ConnectTo(f);
				rd.SetPos(0);
				i := 0; WHILE i < mutableSize DO rd.ReadByte(mutableState[i]); INC(i) END;
				MPIworker.SendInteger(mutableSize);
				MPIworker.SendBytes(mutableState);
				f.Close;
			|restart:
				f := ParallelFiles.dir.Temp();
				ASSERT(f # NIL, 55);
				wr.ConnectTo(f);
				wr.SetPos(0);
				mutableSize := MPIworker.RecvInteger();
				NEW(mutableState, mutableSize);
				MPIworker.RecvBytes(mutableState);
				i := 0; WHILE i < mutableSize DO wr.WriteByte(mutableState[i]); INC(i) END;
				rd.ConnectTo(f);
				rd.SetPos(0);
				ParallelActions.InternalizeMutable(rd);
				wr.ConnectTo(NIL);
				rd.ConnectTo(NIL);
				f.Close;
			END
		END;
		MPIworker.Barrier;
		MPIworker.FinalizeMPI
	END Init;

BEGIN
	Init
END ParallelWorker.

