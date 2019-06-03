(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	   *)

MODULE MonitorPlugin;


	

	IMPORT
		Stores := Stores64,
		GraphLogical, GraphNodes, GraphVector;

	TYPE
		Monitor* = POINTER TO ABSTRACT RECORD END;

		StdMonitor = POINTER TO RECORD(Monitor)
			sampleSize: INTEGER;
			mean: REAL;
			node: GraphNodes.Node
		END;

		Factory* = POINTER TO ABSTRACT RECORD END;

		StdFactory = POINTER TO RECORD(Factory) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 20 OF CHAR;
		fact-: Factory;

	PROCEDURE (monitor: Monitor) Externalize* (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Internalize* (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) SampleSize* (): INTEGER, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Mean* (): REAL, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Update*, NEW, ABSTRACT;

	PROCEDURE (f: Factory) New* (node: GraphNodes.Node): Monitor, NEW, ABSTRACT;

	PROCEDURE (monitor: StdMonitor) Externalize (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteInt(monitor.sampleSize);
		wr.WriteReal(monitor.mean)
	END Externalize;

	PROCEDURE (monitor: StdMonitor) Internalize (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadInt(monitor.sampleSize);
		rd.ReadReal(monitor.mean)
	END Internalize;

	PROCEDURE (monitor: StdMonitor) SampleSize (): INTEGER;
	BEGIN
		RETURN monitor.sampleSize
	END SampleSize;

	PROCEDURE (monitor: StdMonitor) Mean (): REAL;
	BEGIN
		RETURN monitor.mean
	END Mean;

	PROCEDURE (monitor: StdMonitor) Update;
		VAR
			n1: INTEGER;
			delta: REAL;
			node: GraphNodes.Node;
	BEGIN
		node := monitor.node;
		WITH node: GraphVector.Node DO
			IF node.index = 0 THEN
				node.SetProps(node.props + {GraphLogical.dirty})
			END
		ELSE
		END;
		n1 := monitor.sampleSize + 1;
		delta := monitor.sampleSize / n1;
		monitor.mean := delta * monitor.mean + node.Value() / n1;
		INC(monitor.sampleSize)
	END Update;

	PROCEDURE (f: StdFactory) New (node: GraphNodes.Node): Monitor;
		VAR
			monitor: StdMonitor;
	BEGIN
		NEW(monitor);
		monitor.node := node;
		monitor.sampleSize := 0;
		monitor.mean := 0.0;
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
		fact := f
	END Init;

BEGIN
	Init
END MonitorPlugin.

