(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"


*)

MODULE ParallelActions;

	

	IMPORT
		MPIworker,
		Math, Stores,
		GraphStochastic,
		MonitorDeviance,
		ParallelRandnum,
		UpdaterParallel, UpdaterUpdaters;


	VAR
		distributedUpdaters: UpdaterUpdaters.Vector;
		distributedId: POINTER TO ARRAY OF INTEGER;
		distributedBlocks: POINTER TO ARRAY OF GraphStochastic.Vector;
		distributedObservations: GraphStochastic.Vector;
		distributedDevMonitors: POINTER TO ARRAY OF MonitorDeviance.Monitor;
		commSize: INTEGER;

		version-: INTEGER; 	(*	version number	*)
		maintainer-: ARRAY 40 OF CHAR; 	(*	person maintaining module	*)

	PROCEDURE NumUpdaters* (): INTEGER;
	BEGIN
		RETURN LEN(distributedUpdaters);
	END NumUpdaters;

	PROCEDURE ExternalizeUpdaterData* (VAR wr: Stores.Writer);
		VAR
			j, numUpdaters, oldPos, pos, start: INTEGER;
			updater: UpdaterUpdaters.Updater;
	BEGIN
		numUpdaters := LEN(distributedUpdaters);
		oldPos := wr.Pos();
		wr.WriteInt( - 1);
		j := 0;
		start := wr.Pos();
		WHILE j < numUpdaters DO
			updater := distributedUpdaters[j];
			UpdaterUpdaters.Externalize(updater, wr);
			INC(j)
		END;
		pos := wr.Pos();
		wr.SetPos(oldPos);
		wr.WriteInt(start);
		wr.SetPos(pos)
	END ExternalizeUpdaterData;

	PROCEDURE GetBlock* (index: INTEGER): GraphStochastic.Vector;
	BEGIN
		RETURN distributedBlocks[index]
	END GetBlock;

	PROCEDURE GetUpdater* (index: INTEGER): UpdaterUpdaters.Updater;
	BEGIN
		RETURN distributedUpdaters[index]
	END GetUpdater;

	PROCEDURE IsAdapting* (): BOOLEAN;
		VAR
			isAdapting: BOOLEAN;
			i, numUpdaters: INTEGER;
			u: UpdaterUpdaters.Updater;
	BEGIN
		isAdapting := FALSE;
		numUpdaters := LEN(distributedUpdaters);
		i := 0;
		WHILE (i < numUpdaters) & ~isAdapting DO
			u := distributedUpdaters[i];
			isAdapting := u.IsAdapting();
			INC(i)
		END;
		RETURN isAdapting
	END IsAdapting;

	PROCEDURE Sample* (OUT res: SET);
		CONST
			overRelax = FALSE;
		VAR
			dataSize, i, j, len, rank, numUpdaters, offset: INTEGER;
			values, globalValues: POINTER TO ARRAY OF REAL;
			block: GraphStochastic.Vector;
	BEGIN
		res := {};
		numUpdaters := LEN(distributedUpdaters);
		i := 0;
		WHILE (i < numUpdaters) & (res = {}) DO
			IF distributedId[i] = 0 THEN ParallelRandnum.UseSameStream END;
			distributedUpdaters[i].Sample(overRelax, res);
			IF res # {} THEN
				(*	handle error	*)
			END;
			block := distributedBlocks[i];
			IF block # NIL THEN
				rank := MPIworker.rank;
				values := MPIworker.values;
				globalValues := MPIworker.globalValues;
				len := LEN(block);
				dataSize := len DIV MPIworker.commSize;
				offset := rank * dataSize;
				j := 0;
				WHILE j < dataSize DO
					values[j] := block[offset + j].value;
					INC(j)
				END;
				MPIworker.GatherValues(dataSize);
				j := 0;
				WHILE j < len DO
					block[j].SetValue(globalValues[j]);
					INC(j)
				END;
				MPIworker.Barrier
			ELSIF distributedId[i] = 0 THEN
				ParallelRandnum.UsePrivateStream;
				MPIworker.Barrier
			END;
			INC(i)
		END;
		MPIworker.Barrier
	END Sample;

	PROCEDURE ConfigureModel* (updaters: POINTER TO ARRAY OF UpdaterUpdaters.Vector;
	id: POINTER TO ARRAY OF INTEGER;
	observations: POINTER TO ARRAY OF GraphStochastic.Vector;
	rank: INTEGER);
		VAR
			i, numBlocks, numUpdaters: INTEGER;
			block: GraphStochastic.Vector;
	BEGIN
		commSize := LEN(updaters);
		numUpdaters := LEN(updaters[0]);
		NEW(distributedBlocks, numUpdaters);
		distributedUpdaters := updaters[rank];
		distributedObservations := observations[rank];
		distributedId := id;
		i := 0;
		numBlocks := 0;
		WHILE i < numUpdaters DO
			IF ABS(id[i]) = 1 THEN
				INC(numBlocks);
				block := UpdaterParallel.ParallelBlock(updaters, id, numBlocks);
			END;
			IF id[i] < 0 THEN
				distributedBlocks[i] := block
			ELSE
				distributedBlocks[i] := NIL
			END;
			INC(i)
		END
	END ConfigureModel;

	PROCEDURE Deviance* (): REAL;
		VAR
			i, numObs: INTEGER;
			deviance: REAL;
	BEGIN
		deviance := 0.0;
		numObs := LEN(distributedObservations);
		i := 0;
		WHILE i < numObs DO
			deviance := deviance + distributedObservations[i].Deviance();
			INC(i)
		END;
		RETURN deviance
	END Deviance;

	PROCEDURE JointPDF* (): REAL;
		VAR
			jointPDF: REAL;
			i, j, numUpdaters, numObs, size: INTEGER;
			p: GraphStochastic.Node;
			u: UpdaterUpdaters.Updater;
	BEGIN
		numUpdaters := LEN(distributedUpdaters);
		numObs := LEN(distributedObservations);
		jointPDF := 0.0;
		i := 0;
		WHILE i < numObs DO
			jointPDF := jointPDF + distributedObservations[i].LogLikelihood();
			INC(i)
		END;
		i := 0;
		WHILE i < numUpdaters DO
			u := distributedUpdaters[i];
			size := u.Size();
			j := 0;
			WHILE j < size DO
				p := u.Prior(j);
				IF (p # NIL) & (p = p.Representative()) THEN
					IF (distributedId[i] # 0) OR (MPIworker.rank = 0) THEN
						jointPDF := jointPDF + p.LogLikelihood()
					END
				END;
				INC(j)
			END;
			INC(i)
		END;
		RETURN jointPDF;
	END JointPDF;

	PROCEDURE ClearWAIC*;
	BEGIN
		distributedDevMonitors := NIL
	END ClearWAIC;

	PROCEDURE SetWAIC*;
		VAR
			i, numObs: INTEGER;
			monitor: MonitorDeviance.Monitor;
			observation: GraphStochastic.Node;
	BEGIN
		i := 0;
		numObs := LEN(distributedObservations);
		NEW(distributedDevMonitors, numObs);
		WHILE i < numObs DO
			observation := distributedObservations[i];
			monitor := MonitorDeviance.fact.New(observation);
			distributedDevMonitors[i] := monitor;
			INC(i)
		END;
	END SetWAIC;

	PROCEDURE UpdateWAIC*;
		VAR
			i, numObs: INTEGER;
			meanDensity, meanDeviance, meanDeviance2, variance: REAL;
			monitor: MonitorDeviance.Monitor;
	BEGIN
		IF distributedDevMonitors = NIL THEN RETURN END;
		numObs := LEN(distributedDevMonitors);
		i := 0;
		WHILE i < numObs DO
			monitor := distributedDevMonitors[i];
			monitor.Update;
			INC(i)
		END
	END UpdateWAIC;

	PROCEDURE CalculateWAIC* (OUT lpd, pW: REAL);
		VAR
			i, numObs: INTEGER;
			meanDensity, meanDeviance, meanDeviance2, variance: REAL;
			monitor: MonitorDeviance.Monitor;
	BEGIN
		IF distributedDevMonitors = NIL THEN RETURN END;
		numObs := LEN(distributedDevMonitors);
		i := 0;
		lpd := 0.0;
		pW := 0.0;
		WHILE i < numObs DO
			monitor := distributedDevMonitors[i];
			monitor.Stats(meanDeviance, meanDeviance2, meanDensity);
			variance := meanDeviance2 - meanDeviance * meanDeviance;
			pW := pW + 0.25 * variance;
			lpd := lpd + Math.Ln(meanDensity);
			INC(i)
		END
	END CalculateWAIC;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		distributedId := NIL;
		distributedUpdaters := NIL;
		distributedBlocks := NIL;
		distributedObservations := NIL;
		distributedDevMonitors := NIL
	END Init;

BEGIN
	Init
END ParallelActions.

