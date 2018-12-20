(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	   *)

MODULE MonitorSamplesDisc;


	

	IMPORT
		Files64, Stores, Stores64,
		GraphDeviance, GraphNodes,
		MonitorMonitors, MonitorSamples;

	CONST
		blockSize = 512;
		indexBlockSize = 128;

	TYPE
		Monitor = POINTER TO RECORD(MonitorSamples.Monitor)
			sampleSize, start: INTEGER;
			filePos: POINTER TO ARRAY OF LONGINT;
			buffer: ARRAY blockSize OF SHORTREAL;
			node: GraphNodes.Node
		END;

		Factory = POINTER TO RECORD(MonitorSamples.Factory) END;

	VAR
		file: Files64.File;
		rd64: Stores64.Reader;
		wr64: Stores64.Writer;
		wrPos: LONGINT;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		fact-: Factory;

	PROCEDURE (monitor: Monitor) Externalize (VAR wr: Stores.Writer);
		VAR
			i, j, numBlocks, numBlocks1: INTEGER;
			pos: LONGINT;
			x: SHORTREAL;
	BEGIN
		wr.WriteInt(monitor.sampleSize);
		wr.WriteInt(monitor.start);
		numBlocks1 := LEN(monitor.filePos);
		numBlocks := 0;
		i := 0;
		WHILE i < numBlocks1 DO
			IF monitor.filePos[i] # - 1 THEN INC(numBlocks) END;
			INC(i)
		END;
		wr.WriteInt(numBlocks);
		i := 0;
		WHILE i < numBlocks DO
			pos := monitor.filePos[i];
			rd64.SetPos(pos);
			j := 0; WHILE j < blockSize DO rd64.ReadSReal(x); wr.WriteSReal(x); INC(j) END;
			INC(i)
		END;
		j := 0; WHILE j < blockSize DO wr.WriteSReal(monitor.buffer[j]); INC(j) END
	END Externalize;

	PROCEDURE (monitor: Monitor) Internalize (VAR rd: Stores.Reader);
		VAR
			i, j, numBlocks, numBlocks1: INTEGER;
			x: SHORTREAL;
	BEGIN
		rd.ReadInt(monitor.sampleSize);
		rd.ReadInt(monitor.start);
		rd.ReadInt(numBlocks);
		numBlocks1 := (numBlocks DIV indexBlockSize) * indexBlockSize;
		IF numBlocks1 < numBlocks THEN INC(numBlocks1, indexBlockSize) END;
		IF numBlocks1 > 0 THEN NEW(monitor.filePos, numBlocks1) END;
		i := 0; WHILE i < numBlocks1 DO monitor.filePos[i] := - 1; INC(i) END;
		i := 0;
		WHILE i < numBlocks DO
			monitor.filePos[i] := wr64.Pos();
			j := 0; WHILE j < blockSize DO rd.ReadSReal(x); wr64.WriteSReal(x); INC(j) END;
			INC(i)
		END;
		j := 0; WHILE j < blockSize DO rd.ReadSReal(monitor.buffer[j]); INC(j) END;
	END Internalize;

	PROCEDURE (monitor: Monitor) MarkMonitored;
	BEGIN
		IF GraphDeviance.IsDeviance(monitor.node) THEN
			MonitorMonitors.MarkDevianceMonitored
		ELSE
			MonitorMonitors.Mark(monitor.node)
		END
	END MarkMonitored;

	PROCEDURE (monitor: Monitor) Sample (OUT sample: ARRAY OF REAL;
	beg, end, step: INTEGER);
		VAR
			blockIndex, blockNum, i, j, k, len, sampleSize: INTEGER;
			block: ARRAY blockSize OF SHORTREAL;
			pos: LONGINT;
	BEGIN
		len := LEN(sample);
		sampleSize := monitor.SampleSize(beg, end, step);
		len := MIN(len, sampleSize);
		end := MIN(end, monitor.start + monitor.sampleSize);
		j := MAX(end - monitor.start - len * step, 0);
		blockNum := j DIV blockSize; (*	get first block	*)
		i := 0;
		pos := monitor.filePos[blockNum];
		IF pos # - 1 THEN
			rd64.SetPos(pos); WHILE i < blockSize DO rd64.ReadSReal(block[i]); INC(i) END;
		ELSE
			WHILE i < blockSize DO block[i] := monitor.buffer[i]; INC(i) END;
		END;
		k := 0;
		WHILE k < len DO
			IF blockNum # j DIV blockSize THEN (*	get new block	*)
				blockNum := j DIV blockSize;
				i := 0;
				pos := monitor.filePos[blockNum];
				IF pos # - 1 THEN
					rd64.SetPos(pos); WHILE i < blockSize DO rd64.ReadSReal(block[i]); INC(i) END;
				ELSE
					WHILE i < blockSize DO block[i] := monitor.buffer[i]; INC(i) END;
				END;
			END;
			blockIndex := j MOD blockSize;
			sample[k] := block[blockIndex];
			INC(j, step);
			INC(k)
		END
	END Sample;

	PROCEDURE (monitor: Monitor) SampleSize (beg, end, step: INTEGER): INTEGER;
		VAR
			sampleSize: INTEGER;
	BEGIN
		beg := MAX(beg, monitor.start);
		end := MIN(end, monitor.start + monitor.sampleSize);
		sampleSize := MAX(0, end - beg) DIV step;
		RETURN sampleSize
	END SampleSize;

	PROCEDURE (monitor: Monitor) Start (): INTEGER;
	BEGIN
		RETURN monitor.start
	END Start;

	PROCEDURE (monitor: Monitor) Update;
		VAR
			i, index, j, len, numBlocks: INTEGER;
			value: REAL;
			temp: POINTER TO ARRAY OF LONGINT;
	BEGIN
		index := monitor.sampleSize MOD blockSize;
		IF index = 0 THEN
			numBlocks := monitor.sampleSize DIV blockSize;
			IF numBlocks > 0 THEN
				len := LEN(monitor.filePos);
				IF numBlocks = len THEN
					NEW(temp, len + indexBlockSize);
					i := 0; WHILE i < len DO temp[i] := monitor.filePos[i]; INC(i) END;
					len := len + indexBlockSize;
					WHILE i < len DO temp[i] := - 1; INC(i) END;
					monitor.filePos := temp
				END;
				wrPos := wr64.Pos();
				monitor.filePos[numBlocks - 1] := wrPos;
				j := 0;
				WHILE j < blockSize DO
					wr64.WriteSReal(monitor.buffer[j]);
					INC(j)
				END
			END
		END;
		value := monitor.node.Value();
		monitor.buffer[index] := SHORT(value);
		INC(monitor.sampleSize)
	END Update;

	PROCEDURE (f: Factory) New (node: GraphNodes.Node; start: INTEGER): Monitor;
		VAR
			i: INTEGER;
			monitor: Monitor;
	BEGIN
		NEW(monitor);
		monitor.node := node;
		monitor.sampleSize := 0;
		monitor.start := start;
		NEW(monitor.filePos, indexBlockSize);
		i := 0;
		WHILE i < indexBlockSize DO
			monitor.filePos[i] := - 1;
			INC(i)
		END;
		monitor.MarkMonitored;
		RETURN monitor
	END New;

	PROCEDURE Clear*;
	BEGIN
		rd64.SetPos(0);
		wr64.SetPos(0)
	END Clear;

	PROCEDURE Install*;
	BEGIN
		MonitorSamples.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			loc: Files64.Locator;
	BEGIN
		Maintainer;
		NEW(fact);
		loc := Files64.dir.This("");
		file := Files64.dir.New(loc, Files64.dontAsk);
		rd64.ConnectTo(file);
		rd64.SetPos(0);
		wr64.ConnectTo(file);
		wr64.SetPos(0)
	END Init;

	PROCEDURE Finalize;
	BEGIN
		rd64.ConnectTo(NIL);
		wr64.ConnectTo(NIL);
		IF file # NIL THEN file.Close END;
		file := NIL
	END Finalize;

BEGIN
	Init
CLOSE
	Finalize
END MonitorSamplesDisc.

