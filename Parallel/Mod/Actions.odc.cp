(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"


*)

MODULE ParallelActions;

	

	IMPORT
		SYSTEM, 
		MPIworker, Math, Stores := Stores64,
		GraphLogical, GraphNodes, GraphStochastic,
		MonitorDeviance,
		ParallelRandnum,
		UpdaterUpdaters;

	VAR
		debug: BOOLEAN;
		updaters-: UpdaterUpdaters.Vector;
		id-, addresses: POINTER TO ARRAY OF INTEGER;
		observations-: GraphStochastic.Vector;
		pointers: GraphNodes.Vector;
		globalStochs-: POINTER TO ARRAY OF GraphStochastic.Vector;
		globalLogicals-: GraphLogical.Vector;
		waicMonitors: POINTER TO ARRAY OF MonitorDeviance.Monitor;
		values-, globalValues-: POINTER TO ARRAY OF REAL;
		sizeUpdaters: INTEGER;

		version-: INTEGER; 	(*	version number	*)
		maintainer-: ARRAY 40 OF CHAR; 	(*	person maintaining module	*)

	PROCEDURE Clear;
	BEGIN
		debug := FALSE;
		updaters := NIL;
		id := NIL;
		addresses := NIL;
		observations := NIL;
		pointers := NIL;
		globalStochs := NIL;
		waicMonitors := NIL;
		values := NIL;
		globalValues := NIL;
		sizeUpdaters := 0
	END Clear;

	PROCEDURE InternalizeId (VAR rd: Stores.Reader);
		VAR
			i, len: INTEGER;
	BEGIN
		id := NIL;
		rd.ReadInt(len);
		IF len > 0 THEN NEW(id, len) END;
		i := 0; WHILE i < len DO rd.ReadInt(id[i]); INC(i) END
	END InternalizeId;

	PROCEDURE InternalizeStochastics (commSize, numStochPointers: INTEGER; VAR rd: Stores.Reader);
		VAR
			i, j: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		NEW(globalStochs, commSize);
		i := 0;
		WHILE i < commSize DO
			NEW(globalStochs[i], numStochPointers);
			j := 0;
			WHILE j < numStochPointers DO
				p := GraphNodes.InternalizePointer(rd);
				globalStochs[i, j] := p(GraphStochastic.Node);
				INC(j)
			END;
			INC(i)
		END
	END InternalizeStochastics;

	PROCEDURE CalculateWAIC* (OUT lpd, pW: REAL);
		VAR
			i, numObs: INTEGER;
			meanDensity, meanDeviance, meanDeviance2, variance: REAL;
			monitor: MonitorDeviance.Monitor;
	BEGIN
		lpd := 0.0;
		pW := 0.0;
		IF waicMonitors # NIL THEN
			numObs := LEN(waicMonitors);
			i := 0;
			WHILE i < numObs DO
				monitor := waicMonitors[i];
				monitor.Stats(meanDeviance, meanDeviance2, meanDensity);
				variance := meanDeviance2 - meanDeviance * meanDeviance;
				pW := pW + 0.25 * variance;
				lpd := lpd + Math.Ln(meanDensity);
				INC(i)
			END
		END
	END CalculateWAIC;

	PROCEDURE ClearWAIC*;
	BEGIN
		waicMonitors := NIL
	END ClearWAIC;

	PROCEDURE CollectMonitored* (IN col, row: ARRAY OF INTEGER;
	OUT monitorValues: ARRAY OF REAL);
		VAR
			i, j, k, numMonitored: INTEGER;
			p: GraphStochastic.Node;
	BEGIN
		numMonitored := LEN(monitorValues);
		i := 0;
		WHILE i < numMonitored DO
			j := col[i];
			k := row[i];
			p := globalStochs[j, k];
			monitorValues[i] := p.value;
			INC(i)
		END
	END CollectMonitored;

	PROCEDURE Deviance* (): REAL;
		VAR
			i, numObs: INTEGER;
			deviance: REAL;
	BEGIN
		deviance := 0.0;
		IF observations # NIL THEN
			numObs := LEN(observations);
			i := 0;
			WHILE i < numObs DO
				deviance := deviance + observations[i].Deviance();
				INC(i)
			END
		END;
		RETURN deviance
	END Deviance;

	PROCEDURE ExternalizeMutable* (VAR wr: Stores.Writer);
		VAR
			i, j, numObs, numStochs, numUpdaters, rank: INTEGER;
			updater: UpdaterUpdaters.Updater;
	BEGIN
		numUpdaters := LEN(updaters);
		ParallelRandnum.ExternalizeGenerators(wr);
		j := 0;
		WHILE j < numUpdaters DO
			updater := updaters[j];
			UpdaterUpdaters.Externalize(updater, wr);
			INC(j)
		END;
		numStochs := LEN(globalStochs[0]);
		rank := MPIworker.rank;
		i := 0; WHILE i < numStochs DO wr.WriteReal(globalStochs[rank, i].value); INC(i) END;
		IF waicMonitors # NIL THEN
			numObs := LEN(waicMonitors);
			wr.WriteInt(numObs);
			i := 0;
			WHILE i < numObs DO
				waicMonitors[i].Externalize(wr);
				INC(i)
			END
		ELSE
			wr.WriteInt(0)
		END
	END ExternalizeMutable;

	PROCEDURE ExternalizeMutableSize* (): INTEGER;
		VAR
			numObs, sizeRandnum, sizeWAIC, size: INTEGER;
	BEGIN
		IF waicMonitors # NIL THEN
			numObs := LEN(waicMonitors)
		ELSE
			numObs := 0
		END;
		sizeWAIC := SIZE(INTEGER) + numObs * (SIZE(INTEGER) + 3 * SIZE(REAL));
		sizeRandnum := ParallelRandnum.ExternalizeSize();
		size := sizeRandnum + sizeUpdaters + sizeWAIC;
		RETURN size
	END ExternalizeMutableSize;

	PROCEDURE FindStochasticPointer* (p: INTEGER; OUT col, row: INTEGER);
		VAR
			num0, num1: INTEGER;
	BEGIN
		IF globalStochs # NIL THEN
			num0 := LEN(globalStochs);
			num1 := LEN(globalStochs[0]);
			col := 0;
			WHILE col < num0 DO
				row := 0;
				WHILE row < num1 DO
					IF SYSTEM.VAL(INTEGER, globalStochs[col, row]) = p THEN RETURN END;
					INC(row)
				END;
				INC(col)
			END
		END;
		row := - 1; col := - 1
	END FindStochasticPointer;

	PROCEDURE GetUpdater* (index: INTEGER): UpdaterUpdaters.Updater;
	BEGIN
		RETURN updaters[index]
	END GetUpdater;

	PROCEDURE InternalizeMutable* (VAR rd: Stores.Reader);
		VAR
			i, j, numObs, numStochs, numUpdaters, rank: INTEGER;
			updater: UpdaterUpdaters.Updater;
			monitor: MonitorDeviance.Monitor;
			observation: GraphStochastic.Node;
	BEGIN
		numUpdaters := LEN(updaters);
		ParallelRandnum.InternalizeGenerators(rd);
		j := 0;
		WHILE j < numUpdaters DO
			updater := updaters[j];
			UpdaterUpdaters.Internalize(updater, rd);
			INC(j)
		END;
		numStochs := LEN(globalStochs[0]);
		rank := MPIworker.rank;
		i := 0; WHILE i < numStochs DO rd.ReadReal(globalStochs[rank, i].value); INC(i) END;
		rd.ReadInt(numObs);
		IF numObs # 0 THEN
			NEW(waicMonitors, numObs);
			i := 0;
			WHILE i < numObs DO
				observation := observations[i];
				monitor := MonitorDeviance.fact.New(observation);
				monitor.Internalize(rd);
				waicMonitors[i] := monitor;
				INC(i)
			END
		END
	END InternalizeMutable;

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

	PROCEDURE JointPDF* (map: BOOLEAN): REAL;
		VAR
			jointPDF: REAL;
			i, numObs, numStochPointers, rank: INTEGER;
			p: GraphStochastic.Node;
	BEGIN
		jointPDF := 0.0;
		IF observations # NIL THEN
			numObs := LEN(observations);
			i := 0;
			WHILE i < numObs DO
				p := observations[i];
				IF ~(GraphStochastic.censored IN p.props) THEN
					jointPDF := jointPDF + p.LogLikelihood()
				END;
				INC(i)
			END
		END;
		rank := MPIworker.rank;
		numStochPointers := LEN(globalStochs[0]);
		i := 0;
		WHILE i < numStochPointers DO
			p := globalStochs[rank, i];
			IF (p # NIL) & (p = p.Representative()) THEN
				IF (MPIworker.rank = 0) OR ~(GraphStochastic.distributed IN p.props) THEN
					jointPDF := jointPDF + p.LogLikelihood();
					IF map THEN jointPDF := jointPDF + p.LogDetJacobian() END
				END
			END;
			INC(i)
		END;
		RETURN jointPDF;
	END JointPDF;

	PROCEDURE MapNamedPointer* (p: INTEGER): INTEGER;
		VAR
			add, i, num: INTEGER;
	BEGIN
		add := p;
		IF debug & (addresses # NIL) THEN
			num := LEN(addresses);
			i := 0;
			WHILE (i < num) & (p # SYSTEM.VAL(INTEGER, pointers[i])) DO INC(i) END;
			IF i # num THEN add := addresses[i] END
		END;
		RETURN add
	END MapNamedPointer;

	PROCEDURE NumUpdaters* (): INTEGER;
	BEGIN
		RETURN LEN(updaters);
	END NumUpdaters;

	PROCEDURE Read* (chain: INTEGER; VAR rd: Stores.Reader);
		VAR
			commSize, i, j, num, numChains, numLogicalPointers, numStochasticPointers,
			numDataPointers: INTEGER;
			uEndPos: LONGINT;
			dummy, dummyMV, p: GraphNodes.Node;
			pos: POINTER TO ARRAY OF LONGINT;
	BEGIN
		rd.ReadBool(debug);
		rd.ReadInt(commSize);
		rd.ReadInt(numChains);
		rd.ReadInt(numStochasticPointers);
		rd.ReadInt(numLogicalPointers);
		rd.ReadInt(numDataPointers);
		NEW(values, numStochasticPointers * commSize);
		NEW(globalValues, numStochasticPointers * commSize);
		InternalizeId(rd);
		GraphNodes.BeginInternalize(rd);
		(*	read in pointer to dummy	*)
		dummy := GraphNodes.InternalizePointer(rd);
		(*	read in pointer to dummy MV	*)
		dummyMV := GraphNodes.InternalizePointer(rd);
		InternalizeStochastics(commSize, numStochasticPointers, rd);
		IF numLogicalPointers > 0 THEN NEW(globalLogicals, numLogicalPointers) END;
		i := 0;
		WHILE i < numLogicalPointers DO
			p := GraphNodes.InternalizePointer(rd);
			globalLogicals[i] := p(GraphLogical.Node);
			INC(i)
		END;
		IF debug THEN
			num := numDataPointers + numLogicalPointers;
			IF num > 0 THEN NEW(addresses, num); NEW(pointers, num) END;
			i := 0;
			WHILE i < numLogicalPointers DO pointers[i] := globalLogicals[i]; INC(i) END;
			j := 0;
			WHILE j < numDataPointers DO pointers[i + j] := GraphNodes.InternalizePointer(rd); INC(j) END;
			i := 0; WHILE i < num DO rd.ReadInt(addresses[i]); INC(i) END;
		ELSE
			i := 0;
			WHILE i < numDataPointers DO p := GraphNodes.InternalizePointer(rd); INC(i) END
		END;
		rd.ReadInt(num);
		IF num > 0 THEN NEW(observations, num) ELSE observations := NIL END;
		i := 0;
		WHILE i < num DO
			p := GraphNodes.InternalizePointer(rd);
			observations[i] := p(GraphStochastic.Node);
			INC(i)
		END;
		GraphNodes.InternalizeNodeData(rd);
		UpdaterUpdaters.BeginInternalize(rd);
		rd.ReadInt(num);
		NEW(updaters, num);
		i := 0; WHILE i < num DO updaters[i] := UpdaterUpdaters.InternalizePointer(rd); INC(i) END;
		NEW(pos, numChains);
		i := 0; WHILE i < numChains DO rd.ReadLong(pos[i]); INC(i) END;
		rd.ReadLong(uEndPos);
		rd.SetPos(pos[chain]);
		i := 0;
		WHILE i < num DO
			UpdaterUpdaters.Internalize(updaters[i], rd);
			INC(i)
		END;
		sizeUpdaters := SHORT(rd.Pos() + 1 - pos[chain]);
		rd.SetPos(uEndPos);
		UpdaterUpdaters.EndInternalize(rd);
		GraphNodes.EndInternalize(rd)
	END Read;

	PROCEDURE SendSample*;
		VAR
			offset, i, j, numWorker, numStoch: INTEGER;
	BEGIN
		numWorker := MPIworker.commSize;
		numStoch := LEN(globalStochs[0]);
		i := 0;
		WHILE i < numWorker DO
			j := 0;
			WHILE j < numStoch DO
				offset := i * numStoch + j; globalValues[offset] := globalStochs[i, j].value; INC(j)
			END;
			INC(i)
		END;
		MPIworker.SendReals(globalValues);
	END SendSample;

	PROCEDURE SetWAIC*;
		VAR
			i, numObs: INTEGER;
			monitor: MonitorDeviance.Monitor;
			observation: GraphStochastic.Node;
	BEGIN
		i := 0;
		IF observations # NIL THEN
			numObs := LEN(observations);
			NEW(waicMonitors, numObs);
			WHILE i < numObs DO
				observation := observations[i];
				monitor := MonitorDeviance.fact.New(observation);
				waicMonitors[i] := monitor;
				INC(i)
			END
		END
	END SetWAIC;

	PROCEDURE Update* (seperable, overRelax: BOOLEAN; OUT res: SET; OUT updater: INTEGER);
		VAR
			blockSize, i, j, k, numWorker, rank, numUpdaters, end, start, size: INTEGER;
	BEGIN
		res := {};
		updater := -1;
		numUpdaters := LEN(updaters);
		numWorker := MPIworker.commSize;
		rank := MPIworker.rank;
		i := 0;
		end := 0;
		IF id # NIL THEN (*	chain is distributed	*)
			WHILE i < numUpdaters DO
				IF id[i] = 0 THEN ParallelRandnum.UseSameStream END;
				IF res = {} THEN 
					updaters[i].Sample(overRelax, res) 
				ELSIF updater = -1 THEN
					updater := i
				END;
				size := updaters[i].Size();
				INC(end, size);
				IF id[i] < 0 THEN (*	end of parallel block	*)
					blockSize := ABS(id[i]) * size;
					start := end - blockSize;
					k := 0;
					WHILE k < blockSize DO values[k] := globalStochs[rank, start + k].value; INC(k) END;
					MPIworker.AllGather(values, blockSize, globalValues);
					j := 0;
					WHILE j < numWorker DO
						k := 0;
						WHILE k < blockSize DO
							globalStochs[j, start + k].value := globalValues[j * blockSize + k];
							INC(k)
						END;
						INC(j)
					END;
					IF ~seperable THEN GraphLogical.EvaluateAllDiffs END
				ELSIF id[i] = 0 THEN
					ParallelRandnum.UsePrivateStream
				END;
				INC(i)
			END;
		ELSE
			WHILE (i < numUpdaters) & (res = {}) DO
				updaters[i].Sample(overRelax, res); IF res # {} THEN updater := i END; INC(i)
			END
		END;
	END Update;

	PROCEDURE UpdateWAIC*;
		VAR
			i, numObs: INTEGER;
			monitor: MonitorDeviance.Monitor;
	BEGIN
		IF waicMonitors = NIL THEN RETURN END;
		numObs := LEN(waicMonitors);
		i := 0;
		WHILE i < numObs DO
			monitor := waicMonitors[i];
			monitor.Update;
			INC(i)
		END
	END UpdateWAIC;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		Clear
	END Init;

BEGIN
	Init
END ParallelActions.


