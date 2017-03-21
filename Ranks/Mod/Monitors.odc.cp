(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE RanksMonitors;


	

	IMPORT
		Stores,
		BugsIndex, BugsNames,
		MonitorRanks;

	TYPE
		Histogram = POINTER TO ARRAY OF INTEGER;

		Monitor* = POINTER TO ABSTRACT RECORD END;

		StdMonitor = POINTER TO RECORD(Monitor);
			name: BugsNames.Name;
			monitor: MonitorRanks.Monitor
		END;

		Factory* = POINTER TO ABSTRACT RECORD END;

		StdFactory = POINTER TO RECORD(Factory) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		fact-, stdFact-: Factory;

	PROCEDURE (monitor: Monitor) Externalize* (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Internalize* (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) MarkMonitored*, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Histogram* (offset: INTEGER;
	OUT histogram: ARRAY OF INTEGER), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Name* (): BugsNames.Name, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) SampleSize* (): INTEGER, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) SetNumChains* (numChains: INTEGER), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Update* (iteration: INTEGER), NEW, ABSTRACT;

	PROCEDURE (f: Factory) New* (name: BugsNames.Name): Monitor, NEW, ABSTRACT;

	PROCEDURE (monitor: StdMonitor) Externalize (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteString(monitor.name.string);
		monitor.monitor.Externalize(wr)
	END Externalize;

	PROCEDURE (monitor: StdMonitor) Internalize (VAR rd: Stores.Reader);
		VAR
			string: ARRAY 256 OF CHAR;
	BEGIN
		rd.ReadString(string);
		monitor.name := BugsIndex.Find(string);
		ASSERT(monitor.name # NIL, 66);
		monitor.monitor := MonitorRanks.fact.New(monitor.name.components);
		monitor.monitor.Internalize(rd)
	END Internalize;

	PROCEDURE (monitor: StdMonitor) Histogram (offset: INTEGER; OUT histogram: ARRAY OF INTEGER);
	BEGIN
		monitor.monitor.Histogram(offset, histogram)
	END Histogram;

	PROCEDURE (monitor: StdMonitor) MarkMonitored;
	BEGIN
		monitor.monitor.MarkMonitored;
	END MarkMonitored;

	PROCEDURE (monitor: StdMonitor) Name (): BugsNames.Name;
	BEGIN
		RETURN monitor.name
	END Name;

	PROCEDURE (monitor: StdMonitor) SampleSize (): INTEGER;
	BEGIN
		RETURN monitor.monitor.SampleSize()
	END SampleSize;

	PROCEDURE (monitor: StdMonitor) SetNumChains (numChains: INTEGER);
	BEGIN
	END SetNumChains;
	
	PROCEDURE (monitor: StdMonitor) Update (iteration: INTEGER);
	BEGIN
		monitor.monitor.Update(iteration)
	END Update;

	PROCEDURE (f: StdFactory) New (name: BugsNames.Name): Monitor;
		VAR
			monitor: StdMonitor;
	BEGIN
		NEW(monitor);
		monitor.name := name;
		IF name # NIL THEN
			monitor.monitor := MonitorRanks.fact.New(name.components)
		ELSE
			monitor.monitor := NIL
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
END RanksMonitors.
