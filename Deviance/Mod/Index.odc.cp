(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

(*

This type of monitor stores the value of two distinct type of information: firstly the deviance of observed
quanities and secondly the mean values of the "parents" of these deviance quanties. The exact meaning of "parents" depends on which plug-in is used.

*)

MODULE DevianceIndex;


	

	IMPORT
		Stores := Stores64,
		BugsIndex, BugsNames,
		DevianceMonitors, DevianceParents, DeviancePlugin,
		MonitorMonitors,
		SummaryMonitors;

	TYPE
		List = POINTER TO LIMITED RECORD
			monitor: DevianceMonitors.Monitor;
			next: List
		END;

		Monitor = POINTER TO RECORD(MonitorMonitors.Monitor)
			updated: BOOLEAN;
			parentMonitor: DevianceParents.Monitor;
			devianceMonitors: List;
			deviance: SummaryMonitors.Monitor
		END;

		Factory = POINTER TO RECORD(MonitorMonitors.Factory) END;

	VAR
		fact-: Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Install*;
	BEGIN
		MonitorMonitors.SetFactory(fact)
	END Install;

	PROCEDURE GetMonitor (): Monitor;
		VAR
			monitor: Monitor;
			m: MonitorMonitors.Monitor;
	BEGIN
		m := MonitorMonitors.MonitorOfType("DevianceIndex.Install");
		IF m = NIL THEN
			Install;
			m := MonitorMonitors.fact.New();
			MonitorMonitors.SetFactory(NIL);
			MonitorMonitors.RegisterMonitor(m)
		END;
		monitor := m(Monitor);
		RETURN monitor
	END GetMonitor;

	PROCEDURE Register* (devianceMonitors: DevianceMonitors.Monitor);
		VAR
			cursor, entry: List;
			monitor: Monitor;
	BEGIN
		monitor := GetMonitor();
		ASSERT(devianceMonitors # NIL, 20);
		NEW(entry);
		entry.monitor := devianceMonitors;
		IF (monitor.devianceMonitors = NIL) OR
			(devianceMonitors.Name().string < monitor.devianceMonitors.monitor.Name().string) THEN
			entry.next := monitor.devianceMonitors;
			monitor.devianceMonitors := entry
		ELSE
			cursor := monitor.devianceMonitors;
			LOOP
				IF cursor.next = NIL THEN
					cursor.next := entry; entry.next := NIL; EXIT
				ELSIF devianceMonitors.Name().string < cursor.next.monitor.Name().string THEN
					entry.next := cursor.next; cursor.next := entry; EXIT
				END;
				cursor := cursor.next
			END
		END
	END Register;

	PROCEDURE Clear*;
		VAR
			monitor: Monitor;
	BEGIN
		monitor := GetMonitor();
		IF monitor # NIL THEN
			monitor.updated := FALSE;
			monitor.parentMonitor := NIL;
			monitor.devianceMonitors := NIL;
			monitor.deviance := NIL
		END
	END Clear;

	PROCEDURE InitMonitor*;
		VAR
			fact: DeviancePlugin.Factory;
			plugin: DeviancePlugin.Plugin;
			monitor: Monitor;
			name: BugsNames.Name;
	BEGIN
		fact := DeviancePlugin.fact;
		monitor := GetMonitor();
		IF fact # NIL THEN
			monitor.deviance := NIL;
			plugin := fact.New();
			IF (plugin = NIL) OR ~plugin.IsValid() THEN
				monitor.parentMonitor := NIL;
			ELSE
				monitor.parentMonitor := DevianceParents.fact.New(plugin)
			END
		ELSE
			name := BugsIndex.Find("deviance");
			IF name # NIL THEN monitor.deviance := SummaryMonitors.fact.New(name) END
		END
	END InitMonitor;

	PROCEDURE Plugin* (): DeviancePlugin.Plugin;
		VAR
			monitor: Monitor;
			parentMonitor: DevianceParents.Monitor;
	BEGIN
		monitor := GetMonitor();
		IF monitor # NIL THEN
			parentMonitor := monitor.parentMonitor;
			IF parentMonitor # NIL THEN RETURN parentMonitor.Plugin() ELSE RETURN NIL END
		ELSE
			RETURN NIL
		END
	END Plugin;

	PROCEDURE (monitor: Monitor) Clear;
	BEGIN
		monitor.updated := FALSE;
		monitor.parentMonitor := NIL;
		monitor.devianceMonitors := NIL
	END Clear;

	PROCEDURE (monitor: Monitor) Externalize (VAR wr: Stores.Writer);
		VAR
			i, numMonitors, numParents: INTEGER;
			cursor: List;
	BEGIN
		wr.WriteBool(monitor.updated);
		IF monitor.parentMonitor # NIL THEN 
			numParents := monitor.parentMonitor.Size()
		ELSE
			numParents := 0
		END;
		wr.WriteInt(numParents);
		IF numParents = 0 THEN RETURN END;
		monitor.parentMonitor.Externalize(wr);
		cursor := monitor.devianceMonitors;
		numMonitors := 0;
		WHILE cursor # NIL DO INC(numMonitors); cursor := cursor.next END;
		wr.WriteInt(numMonitors);
		i := 0;
		cursor := monitor.devianceMonitors;
		WHILE cursor # NIL DO cursor.monitor.Externalize(wr); INC(i); cursor := cursor.next END
	END Externalize;

	(*	reads data into variable monitor	*)
	PROCEDURE (monitor: Monitor) Internalize (VAR rd: Stores.Reader);
		VAR
			i, numMonitors, numParents: INTEGER;
			devianceMonitor: DevianceMonitors.Monitor;
			plugin: DeviancePlugin.Plugin;
	BEGIN
		rd.ReadBool(monitor.updated);
		rd.ReadInt(numParents);
		IF numParents = 0 THEN RETURN END;
		plugin := DeviancePlugin.fact.New();
		monitor.parentMonitor := DevianceParents.fact.New(plugin);
		monitor.parentMonitor.Internalize(rd);
		rd.ReadInt(numMonitors);
		monitor.devianceMonitors := NIL;
		IF numMonitors > 0 THEN
			i := 0;
			WHILE i < numMonitors DO
				devianceMonitor := DevianceMonitors.fact.New(NIL);
				devianceMonitor.Internalize(rd);
				Register(devianceMonitor);
				INC(i)
			END
		END;
	END Internalize;

	PROCEDURE (monitor: Monitor) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "DevianceIndex.Install";
	END Install;

	PROCEDURE (m: Monitor) MarkMonitored;
	BEGIN
	END MarkMonitored;

	PROCEDURE (monitor: Monitor) SetNumChains (numChains: INTEGER);
		VAR
			devianceMonitor: DevianceMonitors.Monitor;
			parentMonitor: DevianceParents.Monitor;
	BEGIN
		IF monitor.parentMonitor # NIL THEN
			parentMonitor := monitor.parentMonitor; parentMonitor.SetNumChains(numChains)
		END;
		IF monitor.devianceMonitors # NIL THEN
			devianceMonitor := monitor.devianceMonitors.monitor; devianceMonitor.SetNumChains(numChains)
		END
	END SetNumChains;

	PROCEDURE (monitor: Monitor) Update (chain: INTEGER);
		VAR
			cursor: List;
	BEGIN
		monitor.updated := TRUE;
		IF monitor.parentMonitor # NIL THEN monitor.parentMonitor.Update END;
		cursor := monitor.devianceMonitors;
		WHILE cursor # NIL DO cursor.monitor.Update; cursor := cursor.next END;
		IF monitor.deviance # NIL THEN monitor.deviance.Update END
	END Update;

	PROCEDURE (f: Factory) New (): Monitor;
		VAR
			monitor: Monitor;
	BEGIN
		NEW(monitor);
		RETURN monitor
	END New;

	PROCEDURE Find* (IN string: ARRAY OF CHAR): DevianceMonitors.Monitor;
		VAR
			cursor: List;
			monitor: Monitor;
	BEGIN
		monitor := GetMonitor();
		cursor := monitor.devianceMonitors;
		WHILE (cursor # NIL)
			 & ((cursor.monitor.Name() = NIL) OR (cursor.monitor.Name().string # string)) DO
			cursor := cursor.next
		END;
		IF cursor # NIL THEN RETURN cursor.monitor ELSE RETURN NIL END
	END Find;

	PROCEDURE GetMonitors* (): POINTER TO ARRAY OF DevianceMonitors.Monitor;
		VAR
			i, len: INTEGER;
			cursor: List;
			monitors: POINTER TO ARRAY OF DevianceMonitors.Monitor;
			monitor: Monitor;
	BEGIN
		monitor := GetMonitor();
		cursor := monitor.devianceMonitors;
		len := 0;
		WHILE cursor # NIL DO INC(len); cursor := cursor.next END;
		IF len = 0 THEN RETURN NIL ELSE NEW(monitors, len) END;
		cursor := monitor.devianceMonitors;
		len := LEN(monitors);
		i := 0; WHILE i < len DO monitors[i] := cursor.monitor; INC(i); cursor := cursor.next END;
		RETURN monitors
	END GetMonitors;

	PROCEDURE IsUpdated* (): BOOLEAN;
		VAR
			monitor: Monitor;
	BEGIN
		monitor := GetMonitor();
		RETURN (monitor # NIL) & monitor.updated
	END IsUpdated;

	PROCEDURE NumberOfMonitors* (): INTEGER;
		VAR
			num: INTEGER;
			cursor: List;
			monitor: Monitor;
	BEGIN
		monitor := GetMonitor();
		cursor := monitor.devianceMonitors;
		num := 0;
		WHILE cursor # NIL DO INC(num); cursor := cursor.next END;
		RETURN num
	END NumberOfMonitors;

	PROCEDURE Means* (): POINTER TO ARRAY OF REAL;
		VAR
			monitor: Monitor;
	BEGIN
		monitor := GetMonitor();
		RETURN monitor.parentMonitor.Means()
	END Means;

	PROCEDURE VarianceOfDeviance* (): REAL;
		VAR
			monitor: Monitor;
			mean, sd, skew, exKur, lower, median, upper: REAL;
	BEGIN
		monitor := GetMonitor();
		ASSERT(monitor.deviance # NIL, 21);
		monitor.deviance.Statistics(0, mean, sd, skew, exKur, lower, median, upper);
		RETURN sd * sd
	END VarianceOfDeviance;

	PROCEDURE SetPluginValues* (IN values: ARRAY OF REAL);
		VAR
			monitor: Monitor;
	BEGIN
		monitor := GetMonitor();
		monitor.parentMonitor.SetValues(values)
	END SetPluginValues;

	PROCEDURE GetPluginValues* (): POINTER TO ARRAY OF REAL;
		VAR
			monitor: Monitor;
	BEGIN
		monitor := GetMonitor();
		RETURN monitor.parentMonitor.Values()
	END GetPluginValues;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f
	END Init;

BEGIN
	Init
END DevianceIndex.
