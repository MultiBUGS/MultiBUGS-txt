(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	   *)

MODULE MonitorSamples;


	

	IMPORT
		Services, Stores := Stores64,
		GraphDeviance, GraphNodes,
		MonitorMonitors;

	CONST
		blockSize = 500;
		indexBlockSize = 100;

	TYPE
		Data = POINTER TO ARRAY blockSize OF SHORTREAL;

		Monitor* = POINTER TO ABSTRACT RECORD END;

		StdMonitor = POINTER TO RECORD(Monitor)
			sampleSize, start: INTEGER;
			blocks: POINTER TO ARRAY OF Data;
			node: GraphNodes.Node
		END;

		Factory* = POINTER TO ABSTRACT RECORD END;

		StdFactory = POINTER TO RECORD(Factory) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		fact-: Factory;

	PROCEDURE (monitor: Monitor) Externalize* (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Internalize* (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) MarkMonitored*, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Sample* (OUT sample: ARRAY OF REAL;
	beg, end, step: INTEGER), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) SampleSize* (beg, end, step: INTEGER): INTEGER, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Start* (): INTEGER, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Update*, NEW, ABSTRACT;

	PROCEDURE (f: Factory) New* (node: GraphNodes.Node; start: INTEGER): Monitor, NEW, ABSTRACT;

	PROCEDURE (monitor: StdMonitor) Externalize (VAR wr: Stores.Writer);
		VAR
			i, j, numBlocks, numBlocks1: INTEGER;
	BEGIN
		wr.WriteInt(monitor.sampleSize);
		wr.WriteInt(monitor.start);
		numBlocks1 := LEN(monitor.blocks);
		numBlocks := 0;
		i := 0;
		WHILE i < numBlocks1 DO
			IF monitor.blocks[i] # NIL THEN INC(numBlocks) END;
			INC(i)
		END;
		wr.WriteInt(numBlocks);
		i := 0;
		WHILE i < numBlocks DO
			j := 0;
			WHILE j < blockSize DO
				wr.WriteSReal(monitor.blocks[i, j]);
				INC(j)
			END;
			INC(i)
		END
	END Externalize;

	PROCEDURE (monitor: StdMonitor) Internalize (VAR rd: Stores.Reader);
		VAR
			i, j, numBlocks, numBlocks1: INTEGER;
	BEGIN
		rd.ReadInt(monitor.sampleSize);
		rd.ReadInt(monitor.start);
		rd.ReadInt(numBlocks);
		numBlocks1 := (numBlocks DIV indexBlockSize) * indexBlockSize;
		IF numBlocks1 < numBlocks THEN INC(numBlocks1, indexBlockSize) END;
		IF numBlocks1 > 0 THEN NEW(monitor.blocks, numBlocks1) END;
		i := 0;
		WHILE i < numBlocks DO
			NEW(monitor.blocks[i]);
			j := 0; WHILE j < blockSize DO rd.ReadSReal(monitor.blocks[i, j]); INC(j) END;
			INC(i)
		END;
		WHILE i < numBlocks1 DO monitor.blocks[i] := NIL; INC(i) END
	END Internalize;

	PROCEDURE (monitor: StdMonitor) MarkMonitored;
	BEGIN
		IF GraphDeviance.IsDeviance(monitor.node) THEN
			MonitorMonitors.MarkDevianceMonitored
		ELSE
			MonitorMonitors.Mark(monitor.node)
		END
	END MarkMonitored;

	PROCEDURE (monitor: StdMonitor) Sample (OUT sample: ARRAY OF REAL;
	beg, end, step: INTEGER);
		VAR
			blockIndex, blockNum, i, j, len, sampleSize: INTEGER;
	BEGIN
		len := LEN(sample);
		sampleSize := monitor.SampleSize(beg, end, step);
		len := MIN(len, sampleSize);
		end := MIN(end, monitor.start + monitor.sampleSize);
		i := 0; j := MAX(end - monitor.start - len * step, 0);
		WHILE i < len DO
			blockNum := j DIV blockSize; blockIndex := j MOD blockSize;
			sample[i] := monitor.blocks[blockNum, blockIndex];
			INC(j, step);
			INC(i)
		END
	END Sample;

	PROCEDURE (monitor: StdMonitor) SampleSize (beg, end, step: INTEGER): INTEGER;
		VAR
			sampleSize: INTEGER;
	BEGIN
		beg := MAX(beg, monitor.start);
		end := MIN(end, monitor.start + monitor.sampleSize);
		sampleSize := MAX(0, end - beg) DIV step;
		RETURN sampleSize
	END SampleSize;

	PROCEDURE (monitor: StdMonitor) Start (): INTEGER;
	BEGIN
		RETURN monitor.start
	END Start;

	PROCEDURE (monitor: StdMonitor) Update;
		VAR
			i, index, numBlocks: INTEGER;
			value: REAL;
			blocks: POINTER TO ARRAY OF Data;
	BEGIN
		index := monitor.sampleSize MOD blockSize;
		numBlocks := monitor.sampleSize DIV blockSize;
		IF numBlocks >= LEN(monitor.blocks) THEN
			NEW(blocks, numBlocks + indexBlockSize);
			IF blocks = NIL THEN
				Services.Collect;
				NEW(blocks, numBlocks + indexBlockSize)
			END;
			i := 0;
			WHILE i < numBlocks DO
				blocks[i] := monitor.blocks[i];
				INC(i)
			END;
			WHILE i < LEN(blocks) DO
				blocks[i] := NIL;
				INC(i)
			END;
			monitor.blocks := blocks
		END;
		IF index = 0 THEN
			NEW(monitor.blocks[numBlocks])
		END;
		value := monitor.node.value;
		monitor.blocks[numBlocks, index] := SHORT(value);
		INC(monitor.sampleSize)
	END Update;

	PROCEDURE (f: StdFactory) New (node: GraphNodes.Node; start: INTEGER): Monitor;
		VAR
			i: INTEGER;
			monitor: StdMonitor;
	BEGIN
		NEW(monitor);
		monitor.node := node;
		monitor.sampleSize := 0;
		monitor.start := start;
		NEW(monitor.blocks, indexBlockSize);
		i := 0;
		WHILE i < indexBlockSize DO
			monitor.blocks[i] := NIL;
			INC(i)
		END;
		monitor.MarkMonitored;
		RETURN monitor
	END New;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: StdFactory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f
	END Init;

	PROCEDURE SetFactory* (f: Factory);
	BEGIN
		fact := f
	END SetFactory;

BEGIN
	Init
END MonitorSamples.

