(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	   *)

MODULE MonitorSummary;


	

	IMPORT
		Math, Stores := Stores64,
		GraphDeviance, GraphNodes, GraphStochastic,
		MonitorMonitors;

	TYPE
		Monitor* = POINTER TO ABSTRACT RECORD END;

		StdMonitor = POINTER TO RECORD(Monitor)
			sampleSize: INTEGER;
			mean, mean2, mean3, mean4, median, fM, lower, fL, upper, fU: REAL;
			node: GraphNodes.Node
		END;

		Factory* = POINTER TO ABSTRACT RECORD END;

		StdFactory = POINTER TO RECORD(Factory) END;

	CONST
		minSample = 100;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 20 OF CHAR;
		fact-: Factory;

	PROCEDURE (monitor: Monitor) Externalize* (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Internalize* (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) MarkMonitored*, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) SampleSize* (): INTEGER, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Statistics* (OUT mean, sd, skew, exKur, lower, median, upper: REAL), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Update*, NEW, ABSTRACT;

	PROCEDURE (monitor: StdMonitor) Externalize (VAR wr: Stores.Writer);
	BEGIN
		wr.WriteInt(monitor.sampleSize);
		wr.WriteReal(monitor.mean);
		wr.WriteReal(monitor.mean2);
		wr.WriteReal(monitor.mean3);
		wr.WriteReal(monitor.mean4);
		wr.WriteReal(monitor.median);
		wr.WriteReal(monitor.fM);
		wr.WriteReal(monitor.lower);
		wr.WriteReal(monitor.fL);
		wr.WriteReal(monitor.upper);
		wr.WriteReal(monitor.fU)
	END Externalize;

	PROCEDURE (monitor: StdMonitor) Internalize (VAR rd: Stores.Reader);
	BEGIN
		rd.ReadInt(monitor.sampleSize);
		rd.ReadReal(monitor.mean);
		rd.ReadReal(monitor.mean2);
		rd.ReadReal(monitor.mean3);
		rd.ReadReal(monitor.mean4);
		rd.ReadReal(monitor.median);
		rd.ReadReal(monitor.fM);
		rd.ReadReal(monitor.lower);
		rd.ReadReal(monitor.fL);
		rd.ReadReal(monitor.upper);
		rd.ReadReal(monitor.fU)
	END Internalize;

	PROCEDURE (monitor: StdMonitor) MarkMonitored;
	BEGIN
		IF GraphDeviance.IsDeviance(monitor.node) THEN
			MonitorMonitors.MarkDevianceMonitored
		ELSE
			MonitorMonitors.Mark(monitor.node)
		END
	END MarkMonitored;

	PROCEDURE (monitor: StdMonitor) UpdatePercentile (alpha, value, sd: REAL; VAR percentile, f: REAL), NEW;
		CONST
			eps = 1.0E-10;
		VAR
			hN1, dN: REAL; n: INTEGER;
	BEGIN
		n := monitor.sampleSize - minSample;
		hN1 := 1 / Math.Sqrt(n + 1);
		IF ABS(percentile - value) <= hN1 THEN
			f := (n * f + 1 / (2 * hN1)) / (n + 1)
		ELSE
			f := n * f / (n + 1)
		END;
		IF f < eps THEN dN := 2 * sd * Math.Power(n, 0.25) ELSE dN := 1 / f END;
		IF value <= percentile THEN
			percentile := percentile - dN * (1 - alpha) / (n + 1)
		ELSE
			percentile := percentile + dN * alpha / (n + 1)
		END
	END UpdatePercentile;

	PROCEDURE (monitor: StdMonitor) UpdateStatistics (value: REAL), NEW;
		CONST
			eps = 1.0E-30;
		VAR
			n, n1: INTEGER;
			delta, sd, var: REAL;
	BEGIN
		n1 := monitor.sampleSize + 1;
		delta := monitor.sampleSize / n1;
		monitor.mean := delta * monitor.mean + value / n1;
		monitor.mean2 := delta * monitor.mean2 + value * value / n1;
		monitor.mean3 := delta * monitor.mean3 + value * value * value / n1;
		monitor.mean4 := delta * monitor.mean4 + value * value * value * value / n1;
		INC(monitor.sampleSize);
		var := monitor.mean2 - monitor.mean * monitor.mean;
		IF var > 0 THEN
			sd := Math.Sqrt(var + eps);
		ELSE
			sd := 0
		END;
		n := monitor.sampleSize - minSample;
		IF n <= 0 THEN
			monitor.median := monitor.mean;
			monitor.fM := 0.0;
			monitor.lower := monitor.mean - 2 * sd;
			monitor.fL := 0.0;
			monitor.upper := monitor.mean + 2 * sd;
			monitor.fU := 0.0
		ELSE
			monitor.UpdatePercentile(0.5, value, sd, monitor.median, monitor.fM);
			monitor.UpdatePercentile(0.025, value, sd, monitor.lower, monitor.fL);
			monitor.UpdatePercentile(0.975, value, sd, monitor.upper, monitor.fU)
		END
	END UpdateStatistics;

	PROCEDURE (monitor: StdMonitor) SampleSize (): INTEGER;
	BEGIN
		RETURN monitor.sampleSize
	END SampleSize;

	PROCEDURE (monitor: StdMonitor) Statistics (OUT mean, sd, skew, exKur, lower, median, upper: REAL);
		CONST
			eps = 1.0E-30;
		VAR
			mean2, mean3, mean4, var: REAL;
	BEGIN
		median := monitor.median;
		mean := monitor.mean;
		mean2 := monitor.mean2;
		mean3 := monitor.mean3;
		mean4 := monitor.mean4;
		var := mean2 - mean * mean;
		IF var > 0 THEN
			sd := Math.Sqrt(var + eps);
			skew := (mean3 - 3 * mean2 * mean + 2 * mean * mean * mean) / (sd * sd * sd);
			exKur := (mean4 - 4 * mean3 * mean + 6 * mean2 * mean * mean - 3 * mean * mean * mean * mean) / 
			(sd * sd * sd * sd) - 3
		ELSE
			sd := 0
		END;
		lower := monitor.lower;
		median := monitor.median;
		upper := monitor.upper
	END Statistics;

	PROCEDURE (monitor: StdMonitor) Update;
		VAR
			value: REAL;
	BEGIN
		value := monitor.node.value;
		monitor.UpdateStatistics(value)
	END Update;

	PROCEDURE (f: Factory) New* (node: GraphNodes.Node): Monitor, NEW, ABSTRACT;

	PROCEDURE (f: StdFactory) New (node: GraphNodes.Node): Monitor;
		VAR
			monitor: StdMonitor;
	BEGIN
		NEW(monitor);
		monitor.node := node;
		monitor.sampleSize := 0;
		monitor.mean := 0.0;
		monitor.mean2 := 0.0;
		monitor.mean3 := 0.0;
		monitor.mean4 := 0.0;
		IF GraphDeviance.IsDeviance(node) THEN
			MonitorMonitors.MarkDevianceMonitored
		ELSE
			MonitorMonitors.Mark(node)
		END;
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
END MonitorSummary.

