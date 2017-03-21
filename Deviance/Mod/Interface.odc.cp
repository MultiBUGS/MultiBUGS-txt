(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE DevianceInterface;


	

	IMPORT
		Math, BugsIndex, BugsInterface,
		BugsNames, DevianceIndex, DevianceMonitors, DevianceParents,
		DeviancePlugin,
		GraphStochastic;

	TYPE
		SetDeviance = POINTER TO RECORD(BugsNames.Visitor)
			termOffset: INTEGER
		END;

	VAR
		lpdGlobal, pWGlobal: POINTER TO ARRAY OF REAL;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Clear*;
	BEGIN
		IF ~BugsInterface.IsDistributed() THEN
			DevianceIndex.Clear
		ELSE
			lpdGlobal := NIL;
			pWGlobal := NIL
		END
	END Clear;

	PROCEDURE NumComponents* (IN name: ARRAY OF CHAR): INTEGER;
		VAR
			i, num, size: INTEGER;
			monitor: DevianceMonitors.Monitor;
			node: BugsNames.Name;
	BEGIN
		monitor := DevianceIndex.Find(name);
		IF monitor = NIL THEN RETURN 0 END;
		node := monitor.Name();
		i := 0;
		size := node.Size();
		num := 0;
		WHILE i < size DO
			IF monitor.IsMonitored(i) & (monitor.SampleSize(i) > 0) THEN INC(num) END;
			INC(i)
		END;
		RETURN num
	END NumComponents;

	PROCEDURE Offsets* (IN name: ARRAY OF CHAR): POINTER TO ARRAY OF INTEGER;
		VAR
			i, num, size: INTEGER;
			offsets: POINTER TO ARRAY OF INTEGER;
			monitor: DevianceMonitors.Monitor;
			node: BugsNames.Name;
	BEGIN
		monitor := DevianceIndex.Find(name);
		IF monitor = NIL THEN RETURN NIL END;
		node := monitor.Name();
		i := 0;
		size := node.Size();
		num := 0;
		WHILE i < size DO
			IF monitor.IsMonitored(i) & (monitor.SampleSize(i) > 0) THEN INC(num) END;
			INC(i)
		END;
		IF num = 0 THEN RETURN NIL ELSE NEW(offsets, num) END;
		i := 0;
		num := 0;
		WHILE i < size DO
			IF monitor.IsMonitored(i) & (monitor.SampleSize(i) > 0) THEN
				offsets[num] := i;
				INC(num)
			END;
			INC(i)
		END;
		RETURN offsets
	END Offsets;

	PROCEDURE SampleSize* (IN name: ARRAY OF CHAR): INTEGER;
		VAR
			offsets: POINTER TO ARRAY OF INTEGER;
			sum: DevianceMonitors.Monitor;
	BEGIN
		offsets := Offsets(name);
		IF offsets = NIL THEN
			RETURN 0
		END;
		sum := DevianceIndex.Find(name);
		RETURN sum.SampleSize(offsets[0])
	END SampleSize;

	PROCEDURE Stats* (IN name: ARRAY OF CHAR;
	OUT dBar, dHat, dic, waic, pD, pW: POINTER TO ARRAY OF REAL);
		VAR
			i, j, len, num, termOffset: INTEGER;
			mean, mean2, meanDen, variance, lpd: REAL;
			monitor: DevianceMonitors.Monitor;
			means, values: POINTER TO ARRAY OF REAL;
			plugin: DeviancePlugin.Plugin;
	BEGIN
		plugin := DevianceIndex.Plugin();
		IF plugin # NIL THEN
			means := DevianceIndex.Means();
			values := DevianceIndex.Values();
			DevianceIndex.SetValues(means);
		END;
		monitor := DevianceIndex.Find(name);
		i := 0;
		len := monitor.Name().Size();
		num := 0;
		WHILE i < len DO
			IF monitor.IsMonitored(i) THEN
				INC(num)
			END;
			INC(i)
		END;
		NEW(dBar, num);
		NEW(waic, num);
		NEW(pW, num);
		IF plugin # NIL THEN
			NEW(dHat, num);
			NEW(pD, num);
			NEW(dic, num)
		ELSE
			dHat := NIL; pD := NIL; dic := NIL
		END;
		i := 0;
		j := 0;
		termOffset := monitor.TermOffset();
		WHILE i < len DO
			IF monitor.IsMonitored(i) THEN
				monitor.Stats(i, mean, mean2, meanDen);
				dBar[j] := mean;
				variance := mean2 - mean * mean;
				lpd := Math.Ln(meanDen);
				IF plugin # NIL THEN
					dHat[j] := plugin.DevianceTerm(termOffset + j);
					pD[j] := dBar[j] - dHat[j];
					dic[j] := dBar[j] + pD[j];
				END;
				pW[j] := 0.25 * variance;
				waic[j] :=  - 2 * lpd + 2 * pW[j];
				INC(j)
			END;
			INC(i)
		END;
		IF plugin # NIL THEN DevianceIndex.SetValues(values) END
	END Stats;

	PROCEDURE NodeStats* (IN name: ARRAY OF CHAR; OUT dBarTot, dHatTot, dicTot, waicTot, pDTot, pWTot: REAL);
		VAR
			i, len: INTEGER;
			offsets: POINTER TO ARRAY OF INTEGER;
			dBar, dHat, dic, waic, pD, pW, variance: POINTER TO ARRAY OF REAL;
	BEGIN
		dBarTot := 0.0;
		dHatTot := 0.0;
		dicTot := 0.0;
		waicTot := 0.0;
		pDTot := 0.0;
		pWTot := 0.0;
		offsets := Offsets(name);
		IF offsets = NIL THEN RETURN END;
		len := LEN(offsets);
		i := 0;
		Stats(name, dBar, dHat, dic, waic, pD, pW);
		WHILE i < len DO
			dBarTot := dBarTot + dBar[i];
			waicTot := waicTot + waic[i];
			pWTot := pWTot + pW[i];
			IF dHat # NIL THEN
				pDTot := pDTot + pD[i];
				dHatTot := dHatTot + dHat[i];
				dicTot := dicTot + dic[i]
			ELSE
				pDTot := 0.0; dHatTot := 0.0; dicTot := 0.0
			END;
			INC(i)
		END
	END NodeStats;

	PROCEDURE PluginSet* (): BOOLEAN;
	BEGIN
		RETURN DevianceIndex.Plugin() # NIL
	END PluginSet;

	PROCEDURE (v: SetDeviance) Do (node: BugsNames.Name);
		VAR
			monitor: DevianceMonitors.Monitor;
			i, size: INTEGER;
	BEGIN
		monitor := DevianceMonitors.fact.New(node, v.termOffset);
		IF monitor # NIL THEN
			DevianceIndex.Register(monitor);
			i := 0;
			size := node.Size();
			WHILE i < size DO
				IF monitor.IsMonitored(i) THEN INC(v.termOffset) END;
				INC(i)
			END;
		END
	END Do;

	PROCEDURE Set*;
		VAR
			v: SetDeviance;
			parents: DevianceParents.Monitor;
			name: BugsNames.Name;
			ok: BOOLEAN;
			i, numChains: INTEGER;
	BEGIN
		IF ~BugsInterface.IsDistributed() THEN
			name := BugsIndex.Find("deviance");
			IF name = NIL THEN DeviancePlugin.SetFact(NIL); RETURN END;
			NEW(v);
			v.termOffset := 0;
			BugsIndex.Accept(v);
			DevianceIndex.InitMonitor
		ELSE
			numChains := BugsInterface.NumberChains();
			NEW(lpdGlobal, numChains);
			NEW(pWGlobal, numChains);
			i := 0;
			WHILE i < numChains DO
				lpdGlobal[i] :=  - 1.0;
				pWGlobal[i] :=  - 1.0;
				INC(i)
			END
		END
	END Set;

	PROCEDURE GetWAIC* (OUT lpd, pW: REAL; chain: INTEGER);
	BEGIN
		lpd := lpdGlobal[chain];
		pW := pWGlobal[chain]
	END GetWAIC;

	PROCEDURE StoreWAIC* (lpd, pW: REAL; chain: INTEGER);
	BEGIN
		lpdGlobal[chain] := lpd;
		pWGlobal[chain] := pW
	END StoreWAIC;

	PROCEDURE IsUpdated* (): BOOLEAN;
		VAR
			isUpdated: BOOLEAN;
	BEGIN
		IF ~BugsInterface.IsDistributed() THEN
			isUpdated := DevianceIndex.IsUpdated()
		ELSE
			isUpdated := (pWGlobal # NIL) & (pWGlobal[0] > 0.0)
		END;
		RETURN isUpdated
	END IsUpdated;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		lpdGlobal := NIL;
		pWGlobal := NIL
	END Init;

BEGIN
	Init
END DevianceInterface.
