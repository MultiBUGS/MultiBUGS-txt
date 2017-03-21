(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE DevianceMonitors;


	

	IMPORT
		Stores,
		BugsIndex, BugsNames, GraphNodes, GraphStochastic,
		MonitorDeviance;

	TYPE
		Monitor* = POINTER TO ABSTRACT RECORD END;

		StdMonitor = POINTER TO RECORD(Monitor)
			termOffset: INTEGER;
			name: BugsNames.Name;
			deviances: POINTER TO ARRAY OF MonitorDeviance.Monitor
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

	PROCEDURE (monitor: Monitor) Name* (): BugsNames.Name, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) SampleSize* (offset: INTEGER): INTEGER, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) SetNumChains* (numChains: INTEGER), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) TermOffset* (): INTEGER, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Stats* (offset: INTEGER; OUT meanDeviance, meanDeviance2, meanDensity: REAL), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Update*, NEW, ABSTRACT;

	PROCEDURE (f: Factory) New* (name: BugsNames.Name; termOffset: INTEGER): Monitor, NEW, ABSTRACT;

	PROCEDURE (monitor: StdMonitor) Externalize (VAR wr: Stores.Writer);
		VAR
			i, size: INTEGER;
			present: BOOLEAN;
	BEGIN
		wr.WriteString(monitor.name.string);
		wr.WriteInt(monitor.termOffset);
		size := LEN(monitor.deviances);
		wr.WriteInt(size);
		i := 0;
		WHILE i < size DO
			present := monitor.deviances[i] # NIL;
			wr.WriteBool(present);
			IF present THEN
				monitor.deviances[i].Externalize(wr)
			END;
			INC(i)
		END
	END Externalize;

	PROCEDURE (monitor: StdMonitor) Internalize (VAR rd: Stores.Reader);
		VAR
			i, size: INTEGER;
			present: BOOLEAN;
			string: ARRAY 256 OF CHAR;
			node: GraphNodes.Node;
			stoch: GraphStochastic.Node;
	BEGIN
		rd.ReadString(string);
		monitor.name := BugsIndex.Find(string);
		rd.ReadInt(monitor.termOffset);
		rd.ReadInt(size);
		NEW(monitor.deviances, size);
		i := 0;
		WHILE i < size DO
			rd.ReadBool(present);
			IF present THEN
				node := monitor.name.components[i];
				stoch := node(GraphStochastic.Node);
				monitor.deviances[i] := MonitorDeviance.fact.New(stoch);
				monitor.deviances[i].Internalize(rd)
			ELSE
				monitor.deviances[i] := NIL
			END;
			INC(i)
		END
	END Internalize;

	PROCEDURE (monitor: StdMonitor) IsMonitored (offset: INTEGER): BOOLEAN;
	BEGIN
		RETURN monitor.deviances[offset] # NIL
	END IsMonitored;

	PROCEDURE (monitor: StdMonitor) Name (): BugsNames.Name;
	BEGIN
		RETURN monitor.name
	END Name;

	PROCEDURE (monitor: StdMonitor) SampleSize (offset: INTEGER): INTEGER;
	BEGIN
		RETURN monitor.deviances[offset].SampleSize()
	END SampleSize;

	PROCEDURE (monitor: StdMonitor) SetNumChains (numChains: INTEGER);
	END SetNumChains;

	PROCEDURE (monitor: StdMonitor) Stats (offset: INTEGER; OUT meanDeviance, meanDeviance2, meanDensity: REAL);
	BEGIN
		monitor.deviances[offset].Stats(meanDeviance, meanDeviance2, meanDensity)
	END Stats;
	
	PROCEDURE (monitor: StdMonitor) TermOffset (): INTEGER;
	BEGIN
		RETURN monitor.termOffset
	END TermOffset;

	PROCEDURE (monitor: StdMonitor) Update;
		VAR
			i, len: INTEGER;
	BEGIN
		IF monitor.deviances # NIL THEN
			i := 0;
			len := LEN(monitor.deviances);
			WHILE i < len DO
				IF monitor.deviances[i] # NIL THEN
					monitor.deviances[i].Update
				END;
				INC(i)
			END
		END
	END Update;

	PROCEDURE (f: StdFactory) New (name: BugsNames.Name; termOffset: INTEGER): Monitor;
		CONST
			observed = {GraphNodes.data, GraphStochastic.censored};
		VAR
			set: BOOLEAN;
			i, nodeSize, size: INTEGER;
			monitor: StdMonitor;
			node: GraphNodes.Node;
			stochastic: GraphStochastic.Node;
	BEGIN
		NEW(monitor);
		monitor.name := name;
		monitor.termOffset := termOffset;
		IF name # NIL THEN
			set := FALSE;
			size := name.Size();
			NEW(monitor.deviances, size);
			i := 0;
			WHILE i < size DO
				monitor.deviances[i] := NIL;
				INC(i)
			END;
			i := 0;
			WHILE i < size DO
				node := name.components[i];
				IF (node # NIL) & (node IS GraphStochastic.Node) & (observed * node.props # {}) THEN
					stochastic := node(GraphStochastic.Node);
					monitor.deviances[i] := MonitorDeviance.fact.New(stochastic);
					nodeSize := stochastic.Size();
					INC(i, nodeSize);
					set := TRUE
				ELSE
					INC(i)
				END
			END;
			IF ~set THEN monitor := NIL END
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
END DevianceMonitors.

