(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"


*)

MODULE ParallelActions1;

	

	IMPORT
		Files,
		MPIworker, Math, Stores,
		GraphNodes, GraphStochastic,
		MonitorDeviance,
		ParallelRandnum,
		UpdaterUpdaters;


	VAR
		updaters: UpdaterUpdaters.Vector;
		id: POINTER TO ARRAY OF INTEGER;
		observations: GraphStochastic.Vector;
		stochastics: POINTER TO ARRAY OF GraphStochastic.Vector;
		distributedDevMonitors: POINTER TO ARRAY OF MonitorDeviance.Monitor;

		version-: INTEGER; 	(*	version number	*)
		maintainer-: ARRAY 40 OF CHAR; 	(*	person maintaining module	*)

	PROCEDURE InternalizeId (VAR rd: Stores.Reader);
		VAR
			i, len: INTEGER;
			id: POINTER TO ARRAY OF INTEGER;
	BEGIN
		i := 0;
		rd.ReadInt(len);
		IF len > 0 THEN NEW(id, len) END;
		WHILE i < len DO
			rd.ReadInt(id[i]);
			INC(i)
		END
	END InternalizeId;

	PROCEDURE InternalizeStochastics (VAR rd: Stores.Reader);
		VAR
			i, j, commSize, num: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		rd.ReadInt(commSize);
		rd.ReadInt(num);
		NEW(stochastics, commSize);
		i := 0;
		WHILE i < commSize DO
			NEW(stochastics[i], num);
			INC(i)
		END;
		i := 0;
		WHILE i < commSize DO
			j := 0;
			WHILE j < num DO
				p := GraphNodes.InternalizePointer(rd);
				stochastics[i, j] := p(GraphStochastic.Node);
				INC(j)
			END;
			INC(i)
		END
	END InternalizeStochastics;

	PROCEDURE Read* (f: Files.File);
		VAR
			rd: Stores.Reader;
			maxSizeParams, i, len, num: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		rd.ConnectTo(f);
		rd.SetPos(0);
		rd.ReadInt(maxSizeParams);
		InternalizeId(rd);
		GraphNodes.BeginInternalize(rd);
		InternalizeStochastics(rd);
		rd.ReadInt(num);
		i := 0; WHILE i < num DO p := GraphNodes.InternalizePointer(rd); INC(i) END;
		rd.ReadInt(len);
		NEW(observations, len);
		i := 0;
		WHILE i < len DO
			p := GraphNodes.InternalizePointer(rd);
			observations[i] := p(GraphStochastic.Node);
			INC(i)
		END;
		GraphNodes.InternalizeNodeData(rd);
		UpdaterUpdaters.BeginInternalize(rd);
		rd.ReadInt(len);
		NEW(updaters, len);
		i := 0; WHILE i < len DO updaters[i] := UpdaterUpdaters.InternalizePointer(rd); INC(i) END;
		rd.ReadInt(len);
		i := 0; WHILE i < len DO UpdaterUpdaters.Internalize(updaters[i], rd); INC(i) END;
		UpdaterUpdaters.EndInternalize(rd);
		GraphNodes.EndInternalize(rd);
		HALT(0);
	END Read;

	PROCEDURE NumUpdaters* (): INTEGER;
	BEGIN
		RETURN LEN(updaters);
	END NumUpdaters;

	PROCEDURE ExternalizeUpdaterData* (VAR wr: Stores.Writer);
		VAR
			j, numUpdaters, oldPos, pos, start: INTEGER;
			updater: UpdaterUpdaters.Updater;
	BEGIN
		numUpdaters := LEN(updaters);
		oldPos := wr.Pos();
		wr.WriteInt( - 1);
		j := 0;
		start := wr.Pos();
		WHILE j < numUpdaters DO
			updater := updaters[j];
			UpdaterUpdaters.Externalize(updater, wr);
			INC(j)
		END;
		pos := wr.Pos();
		wr.SetPos(oldPos);
		wr.WriteInt(start);
		wr.SetPos(pos)
	END ExternalizeUpdaterData;

	PROCEDURE GetUpdater* (index: INTEGER): UpdaterUpdaters.Updater;
	BEGIN
		RETURN updaters[index]
	END GetUpdater;

	PROCEDURE IsAdapting* (): BOOLEAN;
		VAR
			isAdapting: BOOLEAN;
			i, numUpdaters: INTEGER;
			u: UpdaterUpdaters.Updater;
	BEGIN
		isAdapting := FALSE;
		numUpdaters := LEN(updaters);
		i := 0;
		WHILE (i < numUpdaters) & ~isAdapting DO
			u := updaters[i];
			isAdapting := u.IsAdapting();
			INC(i)
		END;
		RETURN isAdapting
	END IsAdapting;

	PROCEDURE Sample* (overRelax: BOOLEAN; OUT res: SET);
		VAR
			i, j, len, num, rank, numUpdaters, offset, end, start, size: INTEGER;
			values, globalValues: POINTER TO ARRAY OF REAL;
	BEGIN
		res := {};
		numUpdaters := LEN(updaters);
		values := MPIworker.values;
		globalValues := MPIworker.globalValues;
		i := 0;
		end := 0;
		IF id # NIL THEN (*	chain is distributed	*)
			WHILE (i < numUpdaters) & (res = {}) DO
				IF id[i] = 0 THEN ParallelRandnum.UseSameStream; END;
				updaters[i].Sample(overRelax, res);
				IF res # {} THEN
					(*	handle error	*)HALT(0)
				END;
				size := updaters[i].Size();
				INC(end, size);
				IF id[i] < 0 THEN
					rank := MPIworker.rank;
					size := ABS(id[i]) * size;
					start := end - size;
					j := 0;
					WHILE j < size DO
						values[j] := stochastics[rank, start + j].value;
						INC(j)
					END;
					MPIworker.GatherValues(size);
					len := LEN(stochastics);
					num := LEN(stochastics[0]);
					i := 0;
					WHILE i < len DO
						IF i # rank THEN
							j := 0;
							WHILE j < size DO
								stochastics[i, start + j].SetValue(globalValues[i * num + j]);
								INC(j)
							END
						END;
						INC(i)
					END
				ELSIF id[i] = 0 THEN
					ParallelRandnum.UsePrivateStream;
				END;
				(*MPIworker.Barrier;*)
				INC(i);
			END;
		ELSE
			WHILE (i < numUpdaters) & (res = {}) DO
				updaters[i].Sample(overRelax, res);
				INC(i)
			END
		END;
		(*MPIworker.Barrier*)
	END Sample;

	PROCEDURE Deviance* (): REAL;
		VAR
			i, numObs: INTEGER;
			deviance: REAL;
	BEGIN
		deviance := 0.0;
		IF observations # NIL THEN
			numObs := LEN(observations)
		ELSE
			numObs := 0
		END;
		i := 0;
		WHILE i < numObs DO
			deviance := deviance + observations[i].Deviance();
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
		numUpdaters := LEN(updaters);
		numObs := LEN(observations);
		jointPDF := 0.0;
		i := 0;
		WHILE i < numObs DO
			jointPDF := jointPDF + observations[i].LogLikelihood();
			INC(i)
		END;
		i := 0;
		WHILE i < numUpdaters DO
			u := updaters[i];
			size := u.Size();
			j := 0;
			WHILE j < size DO
				p := u.Prior(j);
				IF (p # NIL) & (p = p.Representative()) THEN
					IF (MPIworker.rank = 0) OR (id[i] # 0) THEN
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
		numObs := LEN(observations);
		IF numObs # 0 THEN NEW(distributedDevMonitors, numObs) END;
		WHILE i < numObs DO
			observation := observations[i];
			monitor := MonitorDeviance.fact.New(observation);
			distributedDevMonitors[i] := monitor;
			INC(i)
		END;
	END SetWAIC;

	PROCEDURE UpdateWAIC*;
		VAR
			i, numObs: INTEGER;
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

	END Init;

BEGIN
	Init
END ParallelActions1.

