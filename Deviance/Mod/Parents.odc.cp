(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE DevianceParents;

	


	IMPORT
		Stores := Stores64,
		DeviancePlugin,
		GraphNodes,
		MonitorPlugin;

	TYPE
		Monitor* = POINTER TO ABSTRACT RECORD END;

		StdMonitor = POINTER TO RECORD(Monitor)
			plugin: DeviancePlugin.Plugin;
			monitors: POINTER TO ARRAY OF MonitorPlugin.Monitor
		END;

		Factory* = POINTER TO ABSTRACT RECORD END;

		StdFactory = POINTER TO RECORD(Factory) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		fact-, stdFact-: Factory;

	PROCEDURE (monitor: Monitor) Externalize* (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Internalize* (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Means* (): POINTER TO ARRAY OF REAL, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Parents* (): GraphNodes.Vector, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Plugin* (): DeviancePlugin.Plugin, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) SetNumChains* (numChains: INTEGER), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) SetPlugin* (plugin: DeviancePlugin.Plugin), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) SetValues* (IN values: ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Size* (): INTEGER, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Values* (): POINTER TO ARRAY OF REAL, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Update*, NEW, ABSTRACT;

	PROCEDURE (f: Factory) New* (plugin: DeviancePlugin.Plugin): Monitor, NEW, ABSTRACT;

	PROCEDURE (monitor: StdMonitor) Externalize (VAR wr: Stores.Writer);
		VAR
			i, len: INTEGER;
	BEGIN
		IF monitor.monitors # NIL THEN len := LEN(monitor.monitors) ELSE len := 0 END;
		wr.WriteInt(len);
		i := 0;
		WHILE i < len DO monitor.monitors[i].Externalize(wr); INC(i) END
	END Externalize;

	PROCEDURE (monitor: StdMonitor) Internalize (VAR rd: Stores.Reader);
		VAR
			i, len: INTEGER;
			parents: GraphNodes.Vector;
	BEGIN
		rd.ReadInt(len);
		IF len > 0 THEN NEW(monitor.monitors, len) ELSE monitor.monitors := NIL END;
		parents := monitor.plugin.Parents();
		i := 0;
		WHILE i < len DO
			monitor.monitors[i] := MonitorPlugin.fact.New(parents[i]);
			monitor.monitors[i].Internalize(rd);
			INC(i)
		END
	END Internalize;

	PROCEDURE (monitor: StdMonitor) Means (): POINTER TO ARRAY OF REAL;
		VAR
			means: POINTER TO ARRAY OF REAL;
			i, len: INTEGER;
	BEGIN
		i := 0;
		len := LEN(monitor.monitors);
		NEW(means, len);
		i := 0;
		WHILE i < len DO
			means[i] := monitor.monitors[i].Mean();
			INC(i)
		END;
		RETURN means
	END Means;

	PROCEDURE (monitor: StdMonitor) Parents (): GraphNodes.Vector;
	BEGIN
		IF monitor.plugin = NIL THEN
			RETURN NIL
		ELSE
			RETURN monitor.plugin.Parents()
		END
	END Parents;

	PROCEDURE (monitor: StdMonitor) Plugin (): DeviancePlugin.Plugin;
	BEGIN
		RETURN monitor.plugin
	END Plugin;

	PROCEDURE (monitor: StdMonitor) SetNumChains (numChains: INTEGER);
	BEGIN
	END SetNumChains;

	PROCEDURE (monitor: StdMonitor) SetPlugin (plugin: DeviancePlugin.Plugin);
	BEGIN
		monitor.plugin := plugin
	END SetPlugin;

	PROCEDURE (monitor: StdMonitor) SetValues (IN values: ARRAY OF REAL);
	BEGIN
		monitor.plugin.SetValues(values)
	END SetValues;

	PROCEDURE (monitor: StdMonitor) Size (): INTEGER;
	BEGIN
		IF monitor.monitors # NIL THEN
			RETURN LEN(monitor.monitors)
		ELSE
			RETURN 0
		END
	END Size;

	PROCEDURE (monitor: StdMonitor) Update;
		VAR
			i, len: INTEGER;
	BEGIN
		i := 0;
		len := LEN(monitor.monitors);
		i := 0;
		WHILE i < len DO
			monitor.monitors[i].Update;
			INC(i)
		END
	END Update;

	PROCEDURE (monitor: StdMonitor) Values (): POINTER TO ARRAY OF REAL;
	BEGIN
		RETURN monitor.plugin.Values()
	END Values;

	PROCEDURE (f: StdFactory) New (plugin: DeviancePlugin.Plugin): Monitor;
		VAR
			monitor: StdMonitor;
			parents: GraphNodes.Vector;
			i, len: INTEGER;
	BEGIN
		IF ~plugin.IsValid() THEN RETURN NIL END;
		NEW(monitor);
		parents := plugin.Parents();
		monitor.plugin := plugin;
		i := 0;
		len := LEN(parents); 
		NEW(monitor.monitors, len);
		WHILE i < len DO
			monitor.monitors[i] := MonitorPlugin.fact.New(parents[i]);
			INC(i)
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
END DevianceParents.
