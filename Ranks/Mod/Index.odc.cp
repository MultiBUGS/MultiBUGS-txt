(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE RanksIndex;


	

	IMPORT
		Stores,
		MonitorMonitors,
		RanksMonitors;

	TYPE
		List = POINTER TO LIMITED RECORD
			monitor: RanksMonitors.Monitor;
			next: List
		END;

		Monitor = POINTER TO RECORD(MonitorMonitors.Monitor)
			list: List
		END;

		Factory = POINTER TO RECORD(MonitorMonitors.Factory) END;

	VAR
		fact-: MonitorMonitors.Factory;
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
		m := MonitorMonitors.MonitorOfType("RanksIndex.Install");
		IF m = NIL THEN
			Install;
			m := MonitorMonitors.fact.New();
			MonitorMonitors.SetFactory(NIL);
			MonitorMonitors.RegisterMonitor(m)
		END;
		monitor := m(Monitor);
		RETURN monitor
	END GetMonitor;

	PROCEDURE Find* (string: ARRAY OF CHAR): RanksMonitors.Monitor;
		VAR
			cursor: List;
			monitor: Monitor;
	BEGIN
		monitor := GetMonitor();
		cursor := monitor.list;
		WHILE (cursor # NIL) & ((cursor.monitor.Name() = NIL)
			OR (cursor.monitor.Name().string # string)) DO
			cursor := cursor.next
		END;
		IF cursor # NIL THEN
			RETURN cursor.monitor
		ELSE
			RETURN NIL
		END
	END Find;

	PROCEDURE NumberOfMonitors* (): INTEGER;
		VAR
			num: INTEGER;
			cursor: List;
			monitor: Monitor;
	BEGIN
		monitor := GetMonitor();
		cursor := monitor.list;
		num := 0;
		WHILE cursor # NIL DO
			INC(num);
			cursor := cursor.next
		END;
		RETURN num
	END NumberOfMonitors;

	PROCEDURE GetMonitors* (): POINTER TO ARRAY OF RanksMonitors.Monitor;
		VAR
			i, len: INTEGER;
			cursor: List;
			monitors: POINTER TO ARRAY OF RanksMonitors.Monitor;
			monitor: Monitor;
	BEGIN
		monitor := GetMonitor();
		len := NumberOfMonitors();
		IF len > 0 THEN
			NEW(monitors, len)
		ELSE
			monitors := NIL
		END;
		cursor := monitor.list;
		i := 0;
		WHILE cursor # NIL DO
			monitors[i] := cursor.monitor;
			INC(i);
			cursor := cursor.next
		END;
		RETURN monitors
	END GetMonitors;

	PROCEDURE Register* (ranksMonitors: RanksMonitors.Monitor);
		VAR
			cursor, entry: List;
			monitor: Monitor;
	BEGIN
		monitor := GetMonitor();
		NEW(entry);
		entry.monitor := ranksMonitors;
		IF (monitor.list = NIL) OR (ranksMonitors.Name().string < monitor.list.monitor.Name().string) THEN
			entry.next := monitor.list;
			monitor.list := entry
		ELSE
			cursor := monitor.list;
			LOOP
				IF cursor.next = NIL THEN
					cursor.next := entry; entry.next := NIL; EXIT
				ELSIF ranksMonitors.Name().string < cursor.next.monitor.Name().string THEN
					entry.next := cursor.next; cursor.next := entry; EXIT
				END;
				cursor := cursor.next
			END
		END
	END Register;

	PROCEDURE DeRegister* (ranksMonitors: RanksMonitors.Monitor);
		VAR
			cursor: List;
			monitor: Monitor;
	BEGIN
		monitor := GetMonitor();
		cursor := monitor.list;
		IF cursor.monitor = ranksMonitors THEN
			monitor.list := cursor.next
		ELSE
			WHILE (cursor.next # NIL) & (cursor.next.monitor # ranksMonitors) DO
				cursor := cursor.next
			END;
			IF cursor.next # NIL THEN
				cursor.next := cursor.next.next
			END
		END
	END DeRegister;

	PROCEDURE (m: Monitor) Externalize- (VAR wr: Stores.Writer);
		VAR
			i, numMonitors, pos, start: INTEGER;
			cursor: List;
			filePos: POINTER TO ARRAY OF INTEGER;
	BEGIN
		cursor := m.list;
		numMonitors := 0;
		WHILE cursor # NIL DO INC(numMonitors); cursor := cursor.next END;
		wr.WriteInt(numMonitors);
		IF numMonitors > 0 THEN
			start := wr.Pos();
			i := 0; WHILE i < numMonitors DO wr.WriteInt( - 1); INC(i) END;
			NEW(filePos, numMonitors)
		END;
		cursor := m.list;
		i := 0;
		WHILE cursor # NIL DO
			filePos[i] := wr.Pos();
			cursor.monitor.Externalize(wr);
			INC(i);
			cursor := cursor.next
		END;
		IF numMonitors > 0 THEN
			pos := wr.Pos();
			wr.SetPos(start);
			i := 0;
			WHILE i < numMonitors DO
				wr.WriteInt(filePos[i]);
				INC(i)
			END;
			wr.SetPos(pos)
		END
	END Externalize;

	PROCEDURE (monitor: Monitor) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "RanksIndex.Install";
	END Install;

	PROCEDURE (m: Monitor) MarkMonitored;
		VAR
			cursor: List;
			sampleMonitor: RanksMonitors.Monitor;
	BEGIN
		cursor := m.list;
		WHILE cursor # NIL DO
			sampleMonitor := cursor.monitor;
			sampleMonitor.MarkMonitored;
			cursor := cursor.next
		END
	END MarkMonitored;

	PROCEDURE (m: Monitor) Internalize- (VAR rd: Stores.Reader);
		VAR
			i, filePos, numMonitors: INTEGER;
			monitor: RanksMonitors.Monitor;
	BEGIN
		rd.ReadInt(numMonitors);
		i := 0;
		WHILE i < numMonitors DO
			rd.ReadInt(filePos); ASSERT(filePos #  - 1, 66);
			INC(i)
		END;
		i := 0;
		WHILE i < numMonitors DO
			monitor := RanksMonitors.fact.New(NIL);
			monitor.Internalize(rd);
			Register(monitor);
			INC(i)
		END
	END Internalize;

	PROCEDURE (m: Monitor) Clear;
	BEGIN
		m.list := NIL
	END Clear;

	PROCEDURE (m: Monitor) SetNumChains (numChains: INTEGER);
		VAR
			monitor: RanksMonitors.Monitor;
	BEGIN
		IF m.list # NIL THEN
			monitor := m.list.monitor;
			monitor.SetNumChains(numChains)
		END
	END SetNumChains;

	PROCEDURE (m: Monitor) Update (chain: INTEGER);
		VAR
			cursor: List;
	BEGIN
		cursor := m.list;
		WHILE cursor # NIL DO
			cursor.monitor.Update(chain);
			cursor := cursor.next
		END
	END Update;

	PROCEDURE (f: Factory) New (): Monitor;
		VAR
			monitor: Monitor;
	BEGIN
		NEW(monitor);
		RETURN monitor
	END New;

	PROCEDURE Clear*;
		VAR
			monitor: Monitor;
	BEGIN
		monitor := GetMonitor();
		IF monitor # NIL THEN
			monitor.list := NIL
		END
	END Clear;

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
END RanksIndex.
