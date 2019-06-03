(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE MonitorRanks;


	

	IMPORT
		Stores := Stores64,
		GraphNodes, GraphStochastic,
		MonitorMonitors;

	TYPE
		Histogram = POINTER TO ARRAY OF INTEGER;

		Monitor* = POINTER TO ABSTRACT RECORD END;

		StdMonitor = POINTER TO RECORD(Monitor);
			nodes: GraphNodes.Vector;
			histograms: POINTER TO ARRAY OF Histogram;
			values: POINTER TO ARRAY OF REAL;
			sampleSize: INTEGER
		END;

		Factory* = POINTER TO ABSTRACT RECORD END;

		StdFactory = POINTER TO RECORD(Factory) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		fact-: Factory;

	PROCEDURE (monitor: Monitor) Externalize* (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Internalize* (VAR rd: Stores.Reader), NEW, ABSTRACT;

		PROCEDURE (monitor: Monitor) Histogram* (offset: INTEGER;
	OUT histogram: ARRAY OF INTEGER), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) MarkMonitored*, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) SampleSize* (): INTEGER, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) SetNumChains* (numChains: INTEGER), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Update* (iteration: INTEGER), NEW, ABSTRACT;

	PROCEDURE (f: Factory) New* (nodes: GraphNodes.Vector): Monitor, NEW, ABSTRACT;

	PROCEDURE (monitor: StdMonitor) Externalize (VAR wr: Stores.Writer);
		VAR
			i, j, len: INTEGER;
	BEGIN
		len := LEN(monitor.nodes);
		wr.WriteInt(len);
		wr.WriteInt(monitor.sampleSize);
		i := 0;
		WHILE i < len DO
			j := 0; WHILE j < len DO wr.WriteInt(monitor.histograms[i, j]); INC(j) END;
			INC(i)
		END
	END Externalize;

	PROCEDURE (monitor: StdMonitor) Internalize (VAR rd: Stores.Reader);
		VAR
			i, j, len: INTEGER;
	BEGIN
		rd.ReadInt(len);
		rd.ReadInt(monitor.sampleSize);
		i := 0;
		WHILE i < len DO
			j := 0; WHILE j < len DO rd.ReadInt(monitor.histograms[i, j]); INC(j) END;
			INC(i)
		END
	END Internalize;

	PROCEDURE (monitor: StdMonitor) Histogram (offset: INTEGER; OUT histogram: ARRAY OF INTEGER);
		VAR
			i, len: INTEGER;
	BEGIN
		i := 0;
		len := LEN(monitor.values);
		WHILE i < len DO
			histogram[i] := monitor.histograms[offset, i];
			INC(i)
		END
	END Histogram;

	PROCEDURE (monitor: StdMonitor) MarkMonitored;
		VAR
			i, len: INTEGER;
	BEGIN
		i := 0;
		len := LEN(monitor.nodes);
		WHILE i < len DO
			MonitorMonitors.Mark(monitor.nodes[i]);
			INC(i)
		END
	END MarkMonitored;

	PROCEDURE (monitor: StdMonitor) SampleSize (): INTEGER;
	BEGIN
		RETURN monitor.sampleSize
	END SampleSize;

	PROCEDURE (monitor: StdMonitor) SetNumChains (numChains: INTEGER);
	END SetNumChains;

	PROCEDURE (monitor: StdMonitor) Update (iteration: INTEGER);
		VAR
			i, j, numGreater, len: INTEGER;
			node: GraphNodes.Node;
	BEGIN
		i := 0;
		len := LEN(monitor.values);
		WHILE i < len DO
			node := monitor.nodes[i];
			monitor.values[i] := node.Value();
			INC(i)
		END;
		i := 0;
		WHILE i < len DO
			j := 0;
			numGreater := 0;
			WHILE j < len DO
				IF monitor.values[i] > monitor.values[j] THEN
					INC(numGreater)
				END;
				INC(j)
			END;
			INC(monitor.histograms[i][numGreater]);
			INC(i)
		END;
		INC(monitor.sampleSize)
	END Update;

	PROCEDURE (f: StdFactory) New (nodes: GraphNodes.Vector): Monitor;
		VAR
			i, j, len: INTEGER;
			monitor: StdMonitor;
	BEGIN
		NEW(monitor);
		monitor.nodes := nodes;
		monitor.sampleSize := 0;
		len := LEN(monitor.nodes);
		NEW(monitor.histograms, len);
		i := 0;
		WHILE i < len DO
			NEW(monitor.histograms[i], len);
			j := 0;
			WHILE j < len DO
				monitor.histograms[i][j] := 0;
				INC(j)
			END;
			MonitorMonitors.Mark(nodes[i]);
			INC(i)
		END;
		NEW(monitor.values, len);
		RETURN monitor
	END New;

	PROCEDURE SetFactory* (f: Factory);
	BEGIN
		fact := f
	END SetFactory;

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
		fact := f;
	END Init;

BEGIN
	Init
END MonitorRanks.
