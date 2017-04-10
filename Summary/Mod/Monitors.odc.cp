(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		 *)

MODULE SummaryMonitors;


	

	IMPORT
		Stores,
		BugsIndex, BugsNames, GraphNodes,
		GraphStochastic,
		MonitorSummary;

	TYPE
		Monitor* = POINTER TO ABSTRACT RECORD END;

		StdMonitor = POINTER TO RECORD(Monitor)
			name: BugsNames.Name;
			summaries: POINTER TO ARRAY OF MonitorSummary.Monitor
		END;

		Factory* = POINTER TO ABSTRACT RECORD END;

		StdFactory = POINTER TO RECORD(Factory) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		fact-, stdFact-: Factory;

	PROCEDURE (monitor: Monitor) Externalize* (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Internalize* (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) IsMonitored* (offset: INTEGER): BOOLEAN, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) MarkMonitored*, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Name* (): BugsNames.Name, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) SampleSize* (offset: INTEGER): INTEGER, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) SetNumChains* (numChains: INTEGER), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Statistics* (offset: INTEGER; OUT mean, sd, skew, exKur, lower, median, upper: REAL),
	NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Update*, NEW, ABSTRACT;

	PROCEDURE (f: Factory) New* (name: BugsNames.Name): Monitor, NEW, ABSTRACT;

	PROCEDURE (monitor: StdMonitor) Externalize (VAR wr: Stores.Writer);
		VAR
			i, size: INTEGER;
			present: BOOLEAN;
	BEGIN
		wr.WriteString(monitor.name.string);
		i := 0;
		size := LEN(monitor.summaries);
		wr.WriteInt(size);
		WHILE i < size DO
			present := monitor.summaries[i] # NIL;
			wr.WriteBool(present);
			IF present THEN
				monitor.summaries[i].Externalize(wr)
			END;
			INC(i)
		END
	END Externalize;

	PROCEDURE (monitor: StdMonitor) Internalize (VAR rd: Stores.Reader);
		VAR
			i, size: INTEGER;
			present: BOOLEAN;
			string: ARRAY 128 OF CHAR;
			node: GraphNodes.Node;
	BEGIN
		rd.ReadString(string);
		monitor.name := BugsIndex.Find(string);
		rd.ReadInt(size);
		NEW(monitor.summaries, size);
		i := 0;
		WHILE i < size DO
			rd.ReadBool(present);
			IF present THEN
				node := monitor.name.components[i];
				monitor.summaries[i] := MonitorSummary.fact.New(node);
				monitor.summaries[i].Internalize(rd)
			ELSE
				monitor.summaries[i] := NIL
			END;
			INC(i)
		END
	END Internalize;

	PROCEDURE (monitor: StdMonitor) IsMonitored (offset: INTEGER): BOOLEAN;
	BEGIN
		RETURN monitor.summaries[offset] # NIL
	END IsMonitored;

	PROCEDURE (monitor: StdMonitor) MarkMonitored;
		VAR
			i, size: INTEGER;
			summaries: MonitorSummary.Monitor;
	BEGIN
		i := 0;
		size := LEN(monitor.summaries);
		WHILE i < size DO
			summaries := monitor.summaries[i];
			IF summaries # NIL THEN summaries.MarkMonitored END;
			INC(i)
		END
	END MarkMonitored;

	PROCEDURE (monitor: StdMonitor) Name (): BugsNames.Name;
	BEGIN
		RETURN monitor.name
	END Name;

	PROCEDURE (monitor: StdMonitor) SampleSize (offset: INTEGER): INTEGER;
	BEGIN
		RETURN monitor.summaries[offset].SampleSize()
	END SampleSize;

	PROCEDURE (monitor: StdMonitor) SetNumChains (offset: INTEGER);
	BEGIN
	END SetNumChains;
	
	PROCEDURE (monitor: StdMonitor) Statistics (offset: INTEGER; OUT mean, sd, skew, exKur, lower, median, upper: REAL);
	BEGIN
		monitor.summaries[offset].Statistics(mean, sd, skew, exKur, lower, median, upper)
	END Statistics;

	PROCEDURE (monitor: StdMonitor) Update;
		VAR
			i, len: INTEGER;
	BEGIN
		i := 0;
		len := LEN(monitor.summaries);
		WHILE i < len DO
			IF monitor.summaries[i] # NIL THEN
				monitor.summaries[i].Update
			END;
			INC(i)
		END
	END Update;

	PROCEDURE (f: StdFactory) New (name: BugsNames.Name): Monitor;
		VAR
			set: BOOLEAN;
			monitor: StdMonitor;
			i, size: INTEGER;
			node: GraphNodes.Node;
	BEGIN
		NEW(monitor);
		set := FALSE;
		monitor.name := name;
		IF name # NIL THEN
			size := name.Size();
			NEW(monitor.summaries, size);
			i := 0;
			WHILE i < size DO
				node := name.components[i];
				IF (node # NIL) & ~(GraphNodes.data IN node.props)
					 & ~(GraphStochastic.nR IN node.props) THEN
					monitor.summaries[i] := MonitorSummary.fact.New(node);
					set := TRUE
				ELSE
					monitor.summaries[i] := NIL
				END;
				INC(i)
			END;
			IF ~set THEN monitor := NIL END
		ELSE
			monitor.summaries := NIL
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
END SummaryMonitors.

