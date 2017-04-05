(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"


*)

(*	This is the worker program for the parallel version of the BUGS software using MPI.
Communication with the master (user interface program) is via a port. The worker program
of rank zero sends new sampled values back to the master	*)



MODULE ParallelWorker;

	IMPORT
		Files, Kernel, MPIworker, Services, Stores, Strings,
		GraphCenTrunc, GraphNodes, GraphStochastic,
		MonitorMonitors,
		ParallelActions, ParallelRandnum, ParallelTraphandler,
		UpdaterActions, UpdaterParallel, UpdaterUpdaters;

	CONST
		fileStemName = "Bugs";
		update = 0;
		terminate = 1;
		sendMonitors = 2;
		sendSamples = 3;
		toggleWAIC = 4;

	VAR
		restartLoc: Files.Locator;
		numMonitored, sampleSize: INTEGER;
		devianceMonitored, waicSet: BOOLEAN;
		informationCriteria: ARRAY 2 OF REAL;
		deviance: REAL;

		version-: INTEGER; 	(*	version number	*)
		maintainer-: ARRAY 40 OF CHAR; 	(*	person maintaining module	*)

	PROCEDURE ModifyModel;
		VAR
			updaters: POINTER TO ARRAY OF UpdaterUpdaters.Vector;
			deviances: POINTER TO ARRAY OF GraphStochastic.Vector;
			id: POINTER TO ARRAY OF INTEGER;
			blockSize, maxSizeParams, commSize, rank, numStochastics: INTEGER;
		CONST
			chain = 0;
	BEGIN
		commSize := MPIworker.commSize;
		rank := MPIworker.rank;
		UpdaterParallel.ModifyUpdaters(commSize, rank, chain, updaters, id, deviances);
		ParallelActions.ConfigureModel(updaters, id, deviances, rank);
		blockSize := UpdaterParallel.MaxBlockSize(updaters[0], id);
		maxSizeParams := UpdaterParallel.MaxSizeParams(updaters[0]);
		numStochastics := GraphStochastic.numStochastics;
		MPIworker.AllocateStorage(maxSizeParams, blockSize, numStochastics)
	END ModifyModel;

	PROCEDURE Sample (thin, iteration: INTEGER; VAR endOfAdapting: INTEGER);
		VAR
			i: INTEGER;
			res: SET;
	BEGIN
		i := 0;
		res := {};
		WHILE (i < thin) & (res = {}) DO
			ParallelActions.Sample(res);
			INC(i)
		END;
		IF endOfAdapting > iteration THEN
			IF ~ParallelActions.IsAdapting() THEN
				endOfAdapting := iteration + 1
			END
		END
	END Sample;

	PROCEDURE Update (thin, iteration, endOfAdapting: INTEGER);
	BEGIN
		Sample(thin, iteration, endOfAdapting);
		IF devianceMonitored OR waicSet THEN
			deviance := ParallelActions.Deviance();
			MPIworker.SumReal(deviance)
		END;
		MPIworker.SendInteger(endOfAdapting);
		IF (MPIworker.rank = 0) & (numMonitored > 0) THEN
			MonitorMonitors.SendMonitored(MPIworker.samples, MPIworker.monitors, numMonitored);
			MPIworker.SendSamples(numMonitored);
		END
	END Update;

	PROCEDURE WriteMutable (tStamp, wRank: ARRAY OF CHAR);
		VAR
			wr: Stores.Writer;
			f: Files.File;
			res: INTEGER;
	BEGIN
		f := Files.dir.Old(restartLoc, fileStemName + wRank + "_" + tStamp + ".bug", Files.exclusive);
		IF f = NIL THEN
			f := Files.dir.New(restartLoc, Files.dontAsk);
			f.Register(fileStemName + wRank + "_" + tStamp, "bug", Files.dontAsk, res);
			f := Files.dir.Old(restartLoc, fileStemName + wRank + "_" + tStamp + ".bug", Files.exclusive);
		END;
		wr.ConnectTo(f);
		wr.SetPos(0);
		ParallelActions.ExternalizeUpdaterData(wr);
		wr.ConnectTo(NIL);
		f.Flush;
		f.Close
	END WriteMutable;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			action, endOfAdapting, memory, numChains, setUpTime, thin, iteration: INTEGER;
			resultParams: ARRAY 3 OF INTEGER;
			command: ARRAY 4 OF INTEGER;
			buffer: ARRAY 4 OF REAL;
			meanDeviance, meanDeviance2: REAL;
			endTime, startTime: LONGINT;
			timeStamp, worldRank: ARRAY 64 OF CHAR;
			f: Files.File;
			rd: Stores.Reader;
	BEGIN
		devianceMonitored := FALSE;
		waicSet := FALSE;
		Maintainer;
		MPIworker.InitMPI;
		Strings.IntToString(GraphNodes.timeStamp, timeStamp);
		Strings.IntToString(MPIworker.worldRank, worldRank);
		ParallelTraphandler.SetTrapViewer(fileStemName + worldRank + "_" + timeStamp);
		GraphCenTrunc.Install;
		restartLoc := Files.dir.This("Restart");
		startTime := Services.Ticks();
		Strings.IntToString(GraphNodes.timeStamp, timeStamp);
		f := Files.dir.Old(restartLoc, fileStemName + "_" + timeStamp + ".bug", Files.shared);
		ASSERT(f # NIL, 21);
		rd.ConnectTo(f);
		rd.SetPos(0);
		MPIworker.ReadPort(rd);
		rd.ReadInt(numChains);
		MPIworker.SetUpMPI(numChains);
		ParallelRandnum.SetUp(MPIworker.chain, numChains, MPIworker.worldRank);
		UpdaterParallel.ReadGraph(MPIworker.chain, rd);
		rd.ConnectTo(NIL);
		f.Close;
		f := NIL;
		UpdaterActions.LoadSamples(MPIworker.chain);
		ModifyModel;
		endTime := Services.Ticks();
		setUpTime := SHORT(endTime - startTime);
		memory := Kernel.Allocated();
		MPIworker.ConnectToMaster;
		resultParams[0] := setUpTime;
		resultParams[1] := memory;
		resultParams[2] := MPIworker.rank;
		MPIworker.SendIntegers(resultParams);
		numMonitored := 0;
		(*	loop to handle request from master	*)
		LOOP
			MPIworker.ReceiveCommand(command);
			action := command[0];
			CASE action OF
			|sendMonitors:
				numMonitored := command[1];
				devianceMonitored := command[2] > 0;
				IF (MPIworker.rank = 0) & (numMonitored > 0) THEN
					MPIworker.ReceiveMonitors(numMonitored);
				END;
				MPIworker.ReceiveCommand(command);
				thin := command[1]; iteration := command[2]; endOfAdapting := command[3];
				Update(thin, iteration, endOfAdapting);
				IF (MPIworker.rank = 0) & devianceMonitored THEN
					MPIworker.SendReal(deviance)
				END;
				IF waicSet THEN
					ParallelActions.UpdateWAIC;
					INC(sampleSize);
					meanDeviance := meanDeviance + (deviance - meanDeviance) / sampleSize;
					meanDeviance2 := meanDeviance2 + (deviance * deviance - meanDeviance2) / sampleSize
				END
			|sendSamples:
				IF ~devianceMonitored & ~waicSet THEN
					(*	deviance not calculated in Update so calculate it	*)
					deviance := ParallelActions.Deviance();
					MPIworker.SumReal(deviance);
				END;
				IF waicSet THEN
					ParallelActions.CalculateWAIC(informationCriteria[0], informationCriteria[1]);
					MPIworker.SumReals(informationCriteria)
				END;
				IF MPIworker.rank = 0 THEN
					GraphStochastic.WriteSample(MPIworker.samples);
					MPIworker.SendSamples(GraphStochastic.numStochastics);
					MPIworker.SendReal(deviance);
					IF waicSet THEN
						buffer[0] := informationCriteria[0];
						buffer[1] := informationCriteria[1];
						buffer[2] := meanDeviance;
						buffer[3] := meanDeviance2;
						MPIworker.SendReals(buffer)
					END
				END
			|terminate:
				WriteMutable(timeStamp, worldRank);
				EXIT
			|update:
				thin := command[1]; iteration := command[2]; endOfAdapting := command[3];
				Update(thin, iteration, endOfAdapting);
				IF (MPIworker.rank = 0) & devianceMonitored THEN
					MPIworker.SendReal(deviance)
				END;
				IF waicSet THEN
					ParallelActions.UpdateWAIC;
					INC(sampleSize);
					meanDeviance := meanDeviance + (deviance - meanDeviance) / sampleSize;
					meanDeviance2 := meanDeviance2 + (deviance * deviance - meanDeviance2) / sampleSize
				END
			|toggleWAIC:
				waicSet := ~waicSet;
				IF waicSet THEN
					ParallelActions.SetWAIC;
					sampleSize := 0;
					meanDeviance := 0.0;
					meanDeviance2 := 0.0
				ELSE
					ParallelActions.ClearWAIC;
				END
			END
		END;
		MPIworker.Close
	END Init;

BEGIN
	Init
END ParallelWorker.

