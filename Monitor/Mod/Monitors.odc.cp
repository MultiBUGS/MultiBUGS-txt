(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"


 *)

MODULE MonitorMonitors;

	

	IMPORT
		Kernel, Meta, Stores := Stores64,
		GraphNodes, GraphStochastic;

	TYPE

	(*	Draws visual output	*)
		Drawer* = POINTER TO ABSTRACT RECORD END;

		DrawerList = POINTER TO RECORD
			drawer: Drawer;
			next: DrawerList
		END;

		(*	Monitor type for recording values of chain	*)
		Monitor* = POINTER TO ABSTRACT RECORD END;

		MonitorList = POINTER TO RECORD
			monitor: Monitor;
			next: MonitorList
		END;

		Factory* = POINTER TO ABSTRACT RECORD END;

	VAR
		fact-: Factory;
		devianceMonitored-: BOOLEAN;
		version-: INTEGER; 	(*	version number	*)
		maintainer-: ARRAY 40 OF CHAR; 	(*	person maintaining module	*)
		monitorList: MonitorList;
		drawerList: DrawerList;

	(*	Clears (freezes, cut link to simulation of) drawing	*)
	PROCEDURE (drawer: Drawer) Clear-, NEW, EMPTY;

	(*	Updates the drawing	*)
	PROCEDURE (drawer: Drawer) Update-, NEW, ABSTRACT;

	(*	Clears monitor	*)
	PROCEDURE (monitor: Monitor) Clear-, NEW, EMPTY;

	(*	Externalize monitor	*)
	PROCEDURE (monitor: Monitor) Externalize- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	(*	Internalize monitor	*)
	PROCEDURE (monitor: Monitor) Internalize- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) MarkMonitored-, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Install* (OUT install: ARRAY OF CHAR), NEW, ABSTRACT;

	(*	Set number of chains for monitor	*)
	PROCEDURE (monitor: Monitor) SetNumChains- (numChains: INTEGER), NEW, ABSTRACT;

	(*	Update monitor	*)
	PROCEDURE (monitor: Monitor) Update- (chain: INTEGER), NEW, ABSTRACT;

	PROCEDURE (f: Factory) New* (): Monitor, NEW, ABSTRACT;

	(*	Clears the monitor and draw lists	*)
	PROCEDURE Clear*;
		VAR
			cursor: DrawerList;
			drawer: Drawer;
			item: Meta.Item;
			ok: BOOLEAN;
	BEGIN
		cursor := drawerList;
		WHILE cursor # NIL DO
			drawer := cursor.drawer;
			drawer.Clear;
			cursor := cursor.next
		END;
		devianceMonitored := FALSE;
		monitorList := NIL;
		drawerList := NIL;
		fact := NIL;
		IF Kernel.ThisLoadedMod("MonitorSamplesDisc") # NIL THEN
			Meta.LookupPath("MonitorSamplesDisc.Clear", item);
			IF item.obj = Meta.procObj THEN
				item.Call(ok)
			END
		END
	END Clear;

	(*	Externalize monitors	*)
	PROCEDURE ExternalizeMonitors* (VAR wr: Stores.Writer);
		VAR
			i, numTypes: INTEGER;
			pos, start: LONGINT;
			cursor: MonitorList;
			monitor: Monitor;
			install: ARRAY 128 OF CHAR;
			offsets: POINTER TO ARRAY OF LONGINT;
	BEGIN
		(*	at start write info where data for each type of monitor starts	*)
		numTypes := 0;
		cursor := monitorList;
		WHILE cursor # NIL DO INC(numTypes); cursor := cursor.next END;
		wr.WriteInt(numTypes);
		IF numTypes > 0 THEN
			NEW(offsets, numTypes);
			start := wr.Pos();
			i := 0; WHILE i < numTypes DO wr.WriteLong( - 1); INC(i) END
		END;
		cursor := monitorList;
		i := 0;
		WHILE cursor # NIL DO
			offsets[i] := wr.Pos();
			monitor := cursor.monitor;
			monitor.Install(install);
			wr.WriteString(install);
			monitor.Externalize(wr);
			cursor := cursor.next;
			INC(i)
		END;
		IF numTypes > 0 THEN
			pos := wr.Pos();
			wr.SetPos(start);
			i := 0; WHILE i < numTypes DO wr.WriteLong(offsets[i]); INC(i) END;
			wr.SetPos(pos)
		END
	END ExternalizeMonitors;

	PROCEDURE InstallFactory* (IN install: ARRAY OF CHAR);
		VAR
			ok: BOOLEAN;
			item: Meta.Item;
	BEGIN
		fact := NIL;
		Meta.LookupPath(install, item);
		IF item.obj = Meta.procObj THEN
			item.Call(ok)
		END
	END InstallFactory;

	(*	Procedure to add new type of monitor	*)
	PROCEDURE RegisterMonitor* (monitor: Monitor);
		VAR
			element: MonitorList;
	BEGIN
		NEW(element);
		element.monitor := monitor;
		element.next := monitorList;
		monitorList := element
	END RegisterMonitor;

	(*	Internalize monitors	*)
	PROCEDURE InternalizeMonitors* (VAR rd: Stores.Reader);
		VAR
			cursor: MonitorList;
			i, numTypes: INTEGER;
			start: LONGINT;
			install: ARRAY 256 OF CHAR;
			monitor: Monitor;
	BEGIN
		ASSERT(monitorList = NIL, 21);
		rd.ReadInt(numTypes);
		(*	read start info for each monitor type	*)
		i := 0; WHILE i < numTypes DO rd.ReadLong(start); ASSERT(start # - 1, 66); INC(i) END;
		i := 0;
		WHILE i < numTypes DO
			rd.ReadString(install);
			InstallFactory(install);
			monitor := fact.New();
			monitor.Internalize(rd);
			RegisterMonitor(monitor);
			INC(i)
		END;
		(*	reverse the list	*)
		cursor := monitorList;
		monitorList := NIL;
		WHILE cursor # NIL DO
			monitor := cursor.monitor;
			RegisterMonitor(monitor);
			cursor := cursor.next
		END;
	END InternalizeMonitors;

	PROCEDURE Mark* (node: GraphNodes.Node);
		VAR
			list: GraphStochastic.List;
			props: SET;
		CONST
			all = TRUE;
	BEGIN
		IF node IS GraphStochastic.Node THEN
			props := node.props + {GraphStochastic.mark};
			node.SetProps(props);
		ELSE
			list := GraphStochastic.Parents(node, all);
			WHILE list # NIL DO
				node := list.node;
				props := node.props + {GraphStochastic.mark};
				node.SetProps(props);
				list := list.next
			END
		END
	END Mark;

	PROCEDURE MarkDevianceMonitored*;
	BEGIN
		devianceMonitored := TRUE
	END MarkDevianceMonitored;

	PROCEDURE MarkMonitored*;
		VAR
			cursor: MonitorList;
			monitor: Monitor;
	BEGIN
		devianceMonitored := FALSE;
		cursor := monitorList;
		WHILE cursor # NIL DO
			monitor := cursor.monitor;
			monitor.MarkMonitored;
			cursor := cursor.next
		END
	END MarkMonitored;

	PROCEDURE MonitorOfType* (IN install: ARRAY OF CHAR): Monitor;
		VAR
			cursor: MonitorList;
			monitor: Monitor;
			installName: ARRAY 128 OF CHAR;
	BEGIN
		cursor := monitorList;
		LOOP
			IF cursor = NIL THEN monitor := NIL; EXIT END;
			monitor := cursor.monitor;
			monitor.Install(installName);
			IF installName = install THEN EXIT END;
			cursor := cursor.next
		END;
		RETURN monitor
	END MonitorOfType;

	(*	Procedure to add new type of drawer	*)
	PROCEDURE RegisterDrawer* (drawer: Drawer);
		VAR
			element: DrawerList;
	BEGIN
		NEW(element);
		element.next := drawerList;
		element.drawer := drawer;
		drawerList := element
	END RegisterDrawer;

	PROCEDURE SetFactory* (f: Factory);
	BEGIN
		fact := f
	END SetFactory;

	(*	Set number of chains for each entry in monitor list	*)
	PROCEDURE SetNumChainsMonitors* (numChains: INTEGER);
		VAR
			cursor: MonitorList;
			monitor: Monitor;
	BEGIN
		cursor := monitorList;
		WHILE cursor # NIL DO
			monitor := cursor.monitor;
			monitor.SetNumChains(numChains);
			cursor := cursor.next.next
		END
	END SetNumChainsMonitors;

	(*	Updates each entry in drawer list	*)
	PROCEDURE UpdateDrawers*;
		VAR
			cursor: DrawerList;
			drawer: Drawer;
	BEGIN
		cursor := drawerList;
		WHILE cursor # NIL DO
			drawer := cursor.drawer;
			drawer.Update;
			cursor := cursor.next
		END
	END UpdateDrawers;

	(*	Updates each entry in monitor list	*)
	PROCEDURE UpdateMonitors* (chain: INTEGER);
		VAR
			cursor: MonitorList;
			monitor: Monitor;
	BEGIN
		cursor := monitorList;
		WHILE cursor # NIL DO
			monitor := cursor.monitor;
			monitor.Update(chain);
			cursor := cursor.next
		END
	END UpdateMonitors;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		Clear;
	END Init;

BEGIN
	Init
END MonitorMonitors.
