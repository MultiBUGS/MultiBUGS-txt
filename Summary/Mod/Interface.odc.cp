(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE SummaryInterface;


	

	IMPORT
		BugsEvaluate, BugsIndex, BugsInterface, BugsNames, BugsParser,
		SummaryIndex, SummaryMonitors;

	VAR

		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE IsStar* (IN name: ARRAY OF CHAR): BOOLEAN;
		VAR
			i: INTEGER;
			isStar: BOOLEAN;
	BEGIN
		i := 0;
		WHILE (name[i] # 0X) & (name[i] = " ") DO INC(i) END;
		isStar := name[i] = "*";
		INC(i);
		WHILE (name[i] # 0X) & (name[i] = " ") DO INC(i) END;
		RETURN isStar & (name[i] = 0X)
	END IsStar;

	PROCEDURE Clear* (IN name: ARRAY OF CHAR; OUT ok: BOOLEAN);
		VAR
			sum: SummaryMonitors.Monitor;
	BEGIN
		ok := TRUE;
		IF IsStar(name) THEN
			SummaryIndex.Clear
		ELSE
			sum := SummaryIndex.Find(name);
			IF sum = NIL THEN
				ok := FALSE;
				RETURN
			END;
			SummaryIndex.DeRegister(sum)
		END;
		BugsInterface.MonitorChanged
	END Clear;

	PROCEDURE ClearNI* (IN name: ARRAY OF CHAR);
		VAR
			ok: BOOLEAN;
	BEGIN
		Clear(name, ok)
	END ClearNI;

	PROCEDURE NumComponents* (IN name: ARRAY OF CHAR): INTEGER;
		VAR
			i, num, size: INTEGER;
			summary: SummaryMonitors.Monitor;
			node: BugsNames.Name;
	BEGIN
		summary := SummaryIndex.Find(name);
		IF summary = NIL THEN RETURN 0 END;
		node := summary.Name();
		i := 0;
		size := node.Size();
		num := 0;
		WHILE i < size DO
			IF summary.IsMonitored(i) & (summary.SampleSize(i) > 0) THEN
				INC(num)
			END;
			INC(i)
		END;
		RETURN num
	END NumComponents;

	PROCEDURE Offsets* (IN name: ARRAY OF CHAR; OUT offsets: ARRAY OF INTEGER);
		VAR
			i, num, size: INTEGER;
			summary: SummaryMonitors.Monitor;
			node: BugsNames.Name;
	BEGIN
		summary := SummaryIndex.Find(name);
		IF summary = NIL THEN RETURN END;
		node := summary.Name();
		i := 0;
		size := node.Size();
		num := 0;
		WHILE i < size DO
			IF summary.IsMonitored(i) & (summary.SampleSize(i) > 0) THEN
				INC(num)
			END;
			INC(i)
		END;
		IF num = 0 THEN RETURN END;
		i := 0;
		num := 0;
		WHILE i < size DO
			IF summary.IsMonitored(i) & (summary.SampleSize(i) > 0) THEN
				offsets[num] := i;
				INC(num)
			END;
			INC(i)
		END
	END Offsets;

	PROCEDURE SampleSize* (IN name: ARRAY OF CHAR): INTEGER;
		VAR
			num: INTEGER;
			offsets: POINTER TO ARRAY OF INTEGER;
			sum: SummaryMonitors.Monitor;
	BEGIN
		num := NumComponents(name);
		IF num = 0 THEN RETURN 0 END;
		NEW(offsets, num);
		Offsets(name, offsets);
		sum := SummaryIndex.Find(name);
		RETURN sum.SampleSize(offsets[0])
	END SampleSize;

	PROCEDURE Set* (IN name: ARRAY OF CHAR; OUT ok: BOOLEAN);
		VAR
			sum: SummaryMonitors.Monitor;
			node: BugsNames.Name;
	BEGIN
		ok := TRUE;
		node := BugsIndex.Find(name);
		IF node = NIL THEN
			ok := FALSE;
			RETURN
		END;
		sum := SummaryMonitors.fact.New(node);
		IF sum = NIL THEN
			ok := FALSE;
			RETURN
		END;
		SummaryIndex.Register(sum);
		BugsInterface.MonitorChanged
	END Set;

	PROCEDURE Stats* (IN scalar: ARRAY OF CHAR; OUT mean, sd, skew, exKur, lower, median, upper: REAL);
		VAR
			var: BugsParser.Variable;
			sum: SummaryMonitors.Monitor;
			offsets: POINTER TO ARRAY OF INTEGER;
	BEGIN
		var := BugsParser.StringToVariable(scalar);
		sum := SummaryIndex.Find(var.name.string);
		offsets := BugsEvaluate.Offsets(var);
		sum.Statistics(offsets[0], mean, sd, skew, exKur, lower, median, upper);
	END Stats;

	PROCEDURE SetNI* (IN name: ARRAY OF CHAR);
		VAR
			ok: BOOLEAN;
	BEGIN
		Set(name, ok)
	END SetNI;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END SummaryInterface.
