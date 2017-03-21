(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE ModelsMonitors;


	

	IMPORT
		Stores,
		BugsIndex, BugsNames,
		MonitorModel;

	TYPE

		Monitor* = POINTER TO ABSTRACT RECORD END;

		StdMonitor = POINTER TO RECORD(Monitor);
			name: BugsNames.Name;
			monitor: MonitorModel.Monitor
		END;

		Factory* = POINTER TO ABSTRACT RECORD END;

		StdFactory = POINTER TO RECORD(Factory) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		fact-, stdFact-: Factory;

	PROCEDURE (monitor: Monitor) ComponentProbs* (OUT probs: ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Copy* (): Monitor, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Externalize* (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Internalize* (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) ModelProbs* (OUT models: POINTER TO ARRAY OF MonitorModel.Model;
	OUT probs: POINTER TO ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Name* (): BugsNames.Name, NEW, ABSTRACT;


	PROCEDURE (monitor: Monitor) NumberComponents* (): INTEGER, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) SampleSize* (): INTEGER, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) SetNumChains* (numChains: INTEGER), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Store* (IN key: ARRAY OF BOOLEAN; count: INTEGER), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Update*, NEW, ABSTRACT;

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
		monitor.monitor := MonitorModel.fact.New(monitor.name.components);
		monitor.monitor.Internalize(rd)
	END Internalize;

	PROCEDURE (monitor: StdMonitor) ComponentProbs (OUT probs: ARRAY OF REAL);
	BEGIN
		monitor.monitor.ComponentProbs(probs)
	END ComponentProbs;

	PROCEDURE (monitor: StdMonitor) Copy (): Monitor;
		VAR
			m: StdMonitor;
	BEGIN
		NEW(m);
		m.name := monitor.name;
		m.monitor := monitor.monitor.Copy();
		RETURN m
	END Copy;

	PROCEDURE (monitor: StdMonitor) ModelProbs (
	OUT models: POINTER TO ARRAY OF MonitorModel.Model;
	OUT probs: POINTER TO ARRAY OF REAL);
	BEGIN
		monitor.monitor.ModelProbs(models, probs)
	END ModelProbs;

	PROCEDURE (monitor: StdMonitor) Name (): BugsNames.Name;
	BEGIN
		RETURN monitor.name
	END Name;

	PROCEDURE (monitor: StdMonitor) NumberComponents (): INTEGER;
	BEGIN
		RETURN monitor.monitor.NumberComponents()
	END NumberComponents;

	PROCEDURE (monitor: StdMonitor) SampleSize (): INTEGER;
	BEGIN
		RETURN monitor.monitor.SampleSize()
	END SampleSize;

	PROCEDURE (monitor: StdMonitor) SetNumChains (numChains: INTEGER);
	BEGIN
	END SetNumChains;

	PROCEDURE (monitor: StdMonitor) Store (IN key: ARRAY OF BOOLEAN; count: INTEGER);
	BEGIN
		monitor.monitor.Store(key, count)
	END Store;

	PROCEDURE (monitor: StdMonitor) Update;
	BEGIN
		monitor.monitor.Update
	END Update;

	PROCEDURE (f: StdFactory) New (name: BugsNames.Name): Monitor;
		VAR
			monitor: StdMonitor;
	BEGIN
		NEW(monitor);
		monitor.name := name;
		IF name # NIL THEN
			monitor.monitor := MonitorModel.fact.New(name.components)
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
END ModelsMonitors.
