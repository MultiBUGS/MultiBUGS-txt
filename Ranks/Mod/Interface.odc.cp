
(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE RanksInterface;


	

	IMPORT
		BugsIndex, BugsInterface, BugsNames,
		RanksIndex, RanksMonitors;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE IsStar* (IN string: ARRAY OF CHAR): BOOLEAN;
		VAR
			i: INTEGER;
	BEGIN
		i := 0;
		WHILE
			string[i] = " " DO
			INC(i)
		END;
		RETURN string[i] = "*"
	END IsStar;

	PROCEDURE Clear* (IN name: ARRAY OF CHAR; OUT ok: BOOLEAN);
		VAR
			node: BugsNames.Name;
			monitor: RanksMonitors.Monitor;
	BEGIN
		ok := TRUE;
		IF IsStar(name) THEN
			RanksIndex.Clear
		ELSE
			node := BugsIndex.Find(name);
			IF node = NIL THEN
				ok := FALSE;
				RETURN
			END;
			monitor := RanksIndex.Find(name);
			IF monitor = NIL THEN
				ok := FALSE;
				RETURN
			END;
			RanksIndex.DeRegister(monitor)
		END;
		BugsInterface.MonitorChanged
	END Clear;

	PROCEDURE ClearNI* (IN name: ARRAY OF CHAR);
		VAR
			ok: BOOLEAN;
	BEGIN
		Clear(name, ok)
	END ClearNI;

	PROCEDURE SampleSize* (IN name: ARRAY OF CHAR): INTEGER;
		VAR
			sampleSize: INTEGER;
			node: BugsNames.Name;
			monitor: RanksMonitors.Monitor;
	BEGIN
		node := BugsIndex.Find(name);
		IF node = NIL THEN
			RETURN 0
		END;
		monitor := RanksIndex.Find(name);
		IF monitor = NIL THEN
			RETURN 0
		END;
		sampleSize := monitor.SampleSize();
		RETURN sampleSize
	END SampleSize;

	PROCEDURE Set* (IN name: ARRAY OF CHAR; OUT ok: BOOLEAN);
		VAR
			node: BugsNames.Name;
			monitor: RanksMonitors.Monitor;
	BEGIN
		ok := TRUE;
		node := BugsIndex.Find(name);
		IF node = NIL THEN ok := FALSE; RETURN END;
		IF node.numSlots = 0 THEN ok := FALSE; RETURN END;
		monitor := RanksMonitors.fact.New(node);
		RanksIndex.Register(monitor);
		BugsInterface.MonitorChanged
	END Set;

	PROCEDURE SetNI* (IN name: ARRAY OF CHAR);
		VAR
			ok: BOOLEAN;
	BEGIN
		Set(name, ok)
	END SetNI;

	PROCEDURE Stats* (IN name: ARRAY OF CHAR; IN fractions: ARRAY OF REAL;
	OUT percentiles: POINTER TO ARRAY OF ARRAY OF INTEGER);
		CONST
			eps = 1.0E-10;
		VAR
			sampleSize: INTEGER;
			culm, i, j, k, l, numBins, numFrac: INTEGER;
			histogram: POINTER TO ARRAY OF INTEGER;
			monitor: RanksMonitors.Monitor;
	BEGIN
		sampleSize := SampleSize(name);
		IF sampleSize = 0 THEN
			RETURN
		END;
		monitor := RanksIndex.Find(name);
		numBins := monitor.Name().Size();
		numFrac := LEN(fractions);
		NEW(histogram, numBins);
		NEW(percentiles, numBins, numFrac);
		i := 0;
		WHILE i < numBins DO
			monitor.Histogram(i, histogram);
			j := 0;
			WHILE j < numFrac DO
				culm := SHORT(ENTIER((fractions[j] / 100) * sampleSize + eps));
				k := 0;
				l := 0;
				WHILE k < culm DO
					k := k + histogram[l];
					INC(l)
				END;
				percentiles[i, j] := l;
				INC(j)
			END;
			INC(i)
		END
	END Stats;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END RanksInterface.
