(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	   *)

MODULE MonitorDeviance;


	

	IMPORT
		Math, Stores := Stores64, 
		GraphStochastic;

	TYPE
		Monitor* = POINTER TO ABSTRACT RECORD
			sampleSize: INTEGER;
			meanDeviance, meanDeviance2, meanDensity: REAL;
			node: GraphStochastic.Node
		END;

		StdMonitor = POINTER TO RECORD (Monitor) END;

		Factory* = POINTER TO ABSTRACT RECORD END;

		StdFactory = POINTER TO RECORD (Factory) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		fact-: Factory;

	PROCEDURE (monitor: Monitor) Externalize* (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Internalize* (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) SampleSize* (): INTEGER, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Stats* (OUT meanDeviance, meanDeviance2, meanDensity: REAL), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Update*, NEW, ABSTRACT;

	PROCEDURE (monitor: StdMonitor) SampleSize (): INTEGER;
	BEGIN
		RETURN monitor.sampleSize
	END SampleSize;

	PROCEDURE (monitor: StdMonitor) Externalize (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteInt(monitor.sampleSize);
		wr.WriteReal(monitor.meanDeviance);
		wr.WriteReal(monitor.meanDeviance2);
		wr.WriteReal(monitor.meanDensity)
	END Externalize;

	PROCEDURE (monitor: StdMonitor) Internalize (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadInt(monitor.sampleSize);
		rd.ReadReal(monitor.meanDeviance);
		rd.ReadReal(monitor.meanDeviance2);
		rd.ReadReal(monitor.meanDensity)
	END Internalize;

	PROCEDURE (monitor: StdMonitor) Stats (OUT meanDeviance, meanDeviance2, meanDensity: REAL);
	BEGIN
		meanDeviance := monitor.meanDeviance;
		meanDeviance2 := monitor.meanDeviance2;
		meanDensity := monitor.meanDensity
	END Stats;

	PROCEDURE (monitor: StdMonitor) Update;
		VAR
			node: GraphStochastic.Node;
			n1: INTEGER;
			delta, deviance: REAL;
	BEGIN
		n1 := monitor.sampleSize + 1;
		delta := monitor.sampleSize / n1;
		node := monitor.node;
		deviance := node.Deviance();
		monitor.meanDeviance := delta * monitor.meanDeviance + deviance / n1;
		monitor.meanDensity := delta * monitor.meanDensity + Math.Exp(-0.5 * deviance) / n1;
		monitor.meanDeviance2 := delta * monitor.meanDeviance2 + deviance * deviance / n1;
		INC(monitor.sampleSize)
	END Update;

	PROCEDURE (f: Factory) New* (node: GraphStochastic.Node): Monitor, NEW, ABSTRACT;

	PROCEDURE (f: StdFactory) New (node: GraphStochastic.Node): Monitor;
		VAR
			monitor: StdMonitor;
	BEGIN
		NEW(monitor);
		monitor.node := node;
		monitor.sampleSize := 0;
		monitor.meanDeviance := 0.0;
		monitor.meanDensity := 0.0;
		monitor.meanDeviance2 := 0.0;
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
END MonitorDeviance.

