(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE SamplesMonitors;


	

	IMPORT
		Math, Stores,
		BugsIndex, BugsNames,
		GraphNodes,
		MonitorSamples;

	TYPE
		Monitor* = POINTER TO ABSTRACT RECORD END;

		StdMonitor = POINTER TO RECORD(Monitor)
			name: BugsNames.Name;
			samples: POINTER TO ARRAY OF POINTER TO ARRAY OF MonitorSamples.Monitor
		END;

		Factory* = POINTER TO ABSTRACT RECORD END;

		StdFactory = POINTER TO RECORD(Factory) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		fact-, stdFact-: Factory;

	PROCEDURE (monitor: Monitor) Clear* (offset: INTEGER), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Externalize* (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Internalize* (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) IsMonitored* (offset: INTEGER): BOOLEAN, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) MarkMonitored*, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Name* (): BugsNames.Name, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) SampleSize* (offset, beg, end, step: INTEGER): INTEGER, NEW, ABSTRACT;

		PROCEDURE (monitor: Monitor) Sample* (offset, beg, end, step, firstChain, lastChain: INTEGER;
	OUT sample: ARRAY OF ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Set* (offset, start: INTEGER), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Start* (offset: INTEGER): INTEGER, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Update* (chain: INTEGER), NEW, ABSTRACT;

	PROCEDURE (f: Factory) New* (name: BugsNames.Name; numChains: INTEGER): Monitor, NEW, ABSTRACT;

	PROCEDURE (monitor: StdMonitor) Clear (offset: INTEGER);
		VAR
			i, numChains: INTEGER;
	BEGIN
		ASSERT(monitor.samples[offset] # NIL, 21);
		numChains := LEN(monitor.samples[offset]);
		i := 0;
		WHILE i < numChains DO
			monitor.samples[offset, i] := NIL;
			INC(i)
		END
	END Clear;

	PROCEDURE (monitor: StdMonitor) Externalize (VAR wr: Stores.Writer);
		VAR
			beg, end, i, j, numChains, size: INTEGER;
			filePos: POINTER TO ARRAY OF INTEGER;
	BEGIN
		wr.WriteString(monitor.name.string);
		IF monitor.samples # NIL THEN
			size := LEN(monitor.samples);
			numChains := LEN(monitor.samples[0]);
			wr.WriteInt(size);
			wr.WriteInt(numChains);
			beg := wr.Pos();
			i := 0; WHILE i < size DO wr.WriteInt(MIN(INTEGER)); INC(i) END;
			NEW(filePos, size);
			i := 0;
			WHILE i < size DO
				filePos[i] := wr.Pos();
				j := 0;
				WHILE j < numChains DO
					IF monitor.samples[i, j] # NIL THEN
						wr.WriteBool(TRUE);
						monitor.samples[i, j].Externalize(wr)
					ELSE
						wr.WriteBool(FALSE)
					END;
					INC(j)
				END;
				INC(i)
			END;
			end := wr.Pos();
			wr.SetPos(beg);
			i := 0; WHILE i < size DO wr.WriteInt(filePos[i]); INC(i) END;
			wr.SetPos(end)
		ELSE
			size := 0;
			wr.WriteInt(size)
		END
	END Externalize;

	PROCEDURE (monitor: StdMonitor) Internalize (VAR rd: Stores.Reader);
		VAR
			i, filePos, j, numChains, size: INTEGER;
			present: BOOLEAN;
			string: ARRAY 128 OF CHAR;
			node: GraphNodes.Node;
	BEGIN
		rd.ReadString(string);
		monitor.name := BugsIndex.Find(string);
		rd.ReadInt(size);
		IF size > 0 THEN
			NEW(monitor.samples, size);
			rd.ReadInt(numChains);
			i := 0;
			WHILE i < size DO
				rd.ReadInt(filePos);
				ASSERT(filePos # MIN(INTEGER), 66);
				INC(i)
			END;
			i := 0;
			WHILE i < size DO
				NEW(monitor.samples[i], numChains);
				j := 0;
				WHILE j < numChains DO
					rd.ReadBool(present);
					IF present THEN
						node := monitor.name.components[i];
						monitor.samples[i, j] := MonitorSamples.fact.New(node, 0);
						monitor.samples[i, j].Internalize(rd)
					ELSE
						monitor.samples[i, j] := NIL
					END;
					INC(j)
				END;
				INC(i)
			END;
		ELSE
			monitor.samples := NIL
		END
	END Internalize;

	PROCEDURE (monitor: StdMonitor) MarkMonitored;
		VAR
			i, size: INTEGER;
			samples: MonitorSamples.Monitor;
	BEGIN
		i := 0;
		size := LEN(monitor.samples);
		WHILE i < size DO
			samples := monitor.samples[i, 0];
			IF samples # NIL THEN samples.MarkMonitored END;
			INC(i)
		END
	END MarkMonitored;

	PROCEDURE (monitor: StdMonitor) IsMonitored (offset: INTEGER): BOOLEAN;
	BEGIN
		RETURN monitor.samples[offset, 0] # NIL
	END IsMonitored;

	PROCEDURE (monitor: StdMonitor) Name (): BugsNames.Name;
	BEGIN
		RETURN monitor.name
	END Name;

	PROCEDURE (monitor: StdMonitor) Sample (offset, beg, end, step, firstChain, lastChain: INTEGER;
	OUT sample: ARRAY OF ARRAY OF REAL);
		VAR
			i, noChains: INTEGER;
			chain: MonitorSamples.Monitor;
	BEGIN
		ASSERT(monitor.samples[offset] # NIL, 21);
		noChains := lastChain - firstChain + 1;
		ASSERT(noChains = LEN(sample, 0), 22);
		i := firstChain - 1;
		WHILE i < lastChain DO
			chain := monitor.samples[offset, i];
			chain.Sample(sample[i - firstChain + 1], beg, end, step);
			INC(i)
		END
	END Sample;

	PROCEDURE (monitor: StdMonitor) SampleSize (offset, beg, end, step: INTEGER): INTEGER;
		VAR
			sampleSize: INTEGER;
			chain: MonitorSamples.Monitor;
	BEGIN
		ASSERT(monitor.samples[offset] # NIL, 21);
		chain := monitor.samples[offset, 0];
		sampleSize := chain.SampleSize(beg, end, step);
		RETURN sampleSize
	END SampleSize;

	PROCEDURE (monitor: StdMonitor) Set (offset, start: INTEGER);
		VAR
			i, numChains: INTEGER;
			node: GraphNodes.Node;
	BEGIN
		node := monitor.name.components[offset];
		i := 0;
		numChains := LEN(monitor.samples[0], 0);
		WHILE i < numChains DO
			IF (node # NIL) & ~(GraphNodes.data IN node.props) THEN
				monitor.samples[offset, i] := MonitorSamples.fact.New(node, start)
			ELSE
				monitor.samples[offset, i] := NIL
			END;
			INC(i)
		END
	END Set;

	PROCEDURE (monitor: StdMonitor) Start (offset: INTEGER): INTEGER;
		VAR
			start: INTEGER;
			chain: MonitorSamples.Monitor;
	BEGIN
		ASSERT(monitor.samples[offset] # NIL, 21);
		chain := monitor.samples[offset, 0];
		start := chain.Start();
		RETURN start
	END Start;

	PROCEDURE (monitor: StdMonitor) Update (chain: INTEGER);
		VAR
			i, size: INTEGER;
	BEGIN
		IF monitor.samples # NIL THEN
			i := 0;
			size := LEN(monitor.samples);
			WHILE i < size DO
				IF monitor.samples[i, chain] # NIL THEN
					monitor.samples[i, chain].Update
				END;
				INC(i)
			END
		END
	END Update;

	PROCEDURE (f: StdFactory) New* (name: BugsNames.Name; numChains: INTEGER): Monitor;
		VAR
			i, j, size: INTEGER;
			monitor: StdMonitor;
	BEGIN
		NEW(monitor);
		monitor.name := name;
		IF name # NIL THEN
			size := name.Size();
			NEW(monitor.samples, size);
			i := 0;
			WHILE i < size DO
				NEW(monitor.samples[i], numChains);
				j := 0;
				WHILE j < numChains DO
					monitor.samples[i, j] := NIL;
					INC(j)
				END;
				INC(i)
			END
		ELSE
			monitor.samples := NIL
		END;
		RETURN monitor
	END New;

	PROCEDURE SetFact* (f: Factory);
	BEGIN
		fact := f
	END SetFact;

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
		stdFact := f
	END Init;

BEGIN
	Init
END SamplesMonitors.

