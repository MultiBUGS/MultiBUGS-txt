(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE SamplesInterface;


	

	IMPORT
		Math,
		BugsEvaluate, BugsInterface, BugsMsg, BugsNames, BugsParser,
		GraphNodes, GraphStochastic,
		MathSort,
		SamplesIndex, SamplesMonitors, SamplesStatistics;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE IsStar* (IN variable: ARRAY OF CHAR): BOOLEAN;
		VAR
			i: INTEGER;
			isStar: BOOLEAN;
	BEGIN
		i := 0;
		WHILE (variable[i] # 0X) & (variable[i] = " ") DO INC(i) END;
		isStar := variable[i] = "*";
		INC(i);
		WHILE (variable[i] # 0X) & (variable[i] = " ") DO INC(i) END;
		RETURN isStar & (variable[i] = 0X)
	END IsStar;

	PROCEDURE Set* (IN variable: ARRAY OF CHAR; beg, numChains: INTEGER; OUT ok: BOOLEAN);
		VAR
			newMonitor: BOOLEAN;
			i, index, num, size: INTEGER;
			offsets: POINTER TO ARRAY OF INTEGER;
			var: BugsParser.Variable;
			name: BugsNames.Name;
			monitor: SamplesMonitors.Monitor;
			p: GraphNodes.Node;
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		var := BugsParser.StringToVariable(variable);
		IF var = NIL THEN ok := FALSE; RETURN END;
		name := var.name;
		offsets := BugsEvaluate.Offsets(var);
		IF offsets = NIL THEN ok := FALSE; RETURN END;
		size := LEN(offsets);
		monitor := SamplesIndex.Find(name.string);
		newMonitor := monitor = NIL;
		IF newMonitor THEN
			monitor := SamplesMonitors.fact.New(name, numChains)
		END;
		i := 0;
		num := 0;
		WHILE i < size DO
			index := offsets[i];
			p := name.components[index];
			IF ~monitor.IsMonitored(index) & (p # NIL) & 
				({GraphNodes.data, GraphStochastic.hidden} * p.props = {}) THEN
				monitor.Set(index, beg);
				INC(num)
			END;
			INC(i)
		END;
		IF num = 0 THEN
			BugsMsg.Lookup("SamplesCmds:NoMonitors", msg);
			BugsMsg.StoreMsg(msg);
			ok := FALSE; RETURN
		END;
		IF newMonitor THEN
			SamplesIndex.Register(monitor)
		END;
		BugsInterface.MonitorChanged;
		ok := TRUE
	END Set;

	PROCEDURE SetNI* (IN variable: ARRAY OF CHAR; beg, numChains: INTEGER);
		VAR
			ok: BOOLEAN;
	BEGIN
		Set(variable, beg, numChains, ok)
	END SetNI;

	PROCEDURE Clear* (IN variable: ARRAY OF CHAR; OUT ok: BOOLEAN);
		VAR
			i, index, size: INTEGER;
			offsets: POINTER TO ARRAY OF INTEGER;
			var: BugsParser.Variable;
			name: BugsNames.Name;
			monitor: SamplesMonitors.Monitor;
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		IF IsStar(variable) THEN
			ok := TRUE; SamplesIndex.Clear
		ELSE
			ok := FALSE;
			var := BugsParser.StringToVariable(variable);
			IF var = NIL THEN RETURN END;
			name := var.name;
			offsets := BugsEvaluate.Offsets(var);
			IF offsets = NIL THEN RETURN END;
			monitor := SamplesIndex.Find(name.string);
			IF monitor = NIL THEN RETURN END;
			size := LEN(offsets);
			i := 0;
			WHILE i < size DO
				index := offsets[i];
				IF monitor.IsMonitored(index) THEN
					monitor.Clear(index); ok := TRUE
				END;
				INC(i)
			END;
			i := 0;
			size := name.Size();
			WHILE (i < size) & ~monitor.IsMonitored(i) DO
				INC(i)
			END;
			IF i = size THEN SamplesIndex.DeRegister(monitor) END;
		END;
		IF ok THEN 
			BugsInterface.MonitorChanged 
		ELSE
			BugsMsg.Lookup("SamplesCmds:MonitorNotCleared", msg);
			BugsMsg.StoreMsg(msg);
		END
	END Clear;

	PROCEDURE ClearNI* (IN variable: ARRAY OF CHAR);
		VAR
			ok: BOOLEAN;
	BEGIN
		Clear(variable, ok)
	END ClearNI;

	PROCEDURE NumComponents* (IN variable: ARRAY OF CHAR; beg, end, thin: INTEGER): INTEGER;
		VAR
			i, index, num, size, start: INTEGER;
			offsets: POINTER TO ARRAY OF INTEGER;
			monitor: SamplesMonitors.Monitor;
			var: BugsParser.Variable;
	BEGIN
		var := BugsParser.StringToVariable(variable);
		IF var = NIL THEN RETURN 0 END;
		monitor := SamplesIndex.Find(var.name.string);
		IF monitor = NIL THEN RETURN 0 END;
		offsets := BugsEvaluate.Offsets(var);
		IF offsets = NIL THEN RETURN 0 END;
		size := LEN(offsets);
		i := 0;
		num := 0;
		WHILE i < size DO
			index := offsets[i];
			IF monitor.IsMonitored(index) THEN
				start := monitor.Start(index);
				start := MAX(beg, start);
				IF monitor.SampleSize(index, start, end, thin) > 0 THEN
					INC(num)
				END
			END;
			INC(i)
		END;
		RETURN num
	END NumComponents;

	PROCEDURE Offsets* (IN variable: ARRAY OF CHAR; beg, end, thin: INTEGER;
	OUT offsets: ARRAY OF INTEGER);
		VAR
			i, index, num, size, start: INTEGER;
			allOffsets: POINTER TO ARRAY OF INTEGER;
			monitor: SamplesMonitors.Monitor;
			var: BugsParser.Variable;
			name: BugsNames.Name;
	BEGIN
		num := NumComponents(variable, beg, end, thin);
		ASSERT(num # 0, 66);
		var := BugsParser.StringToVariable(variable);
		allOffsets := BugsEvaluate.Offsets(var);
		name := var.name;
		size := LEN(allOffsets);
		monitor := SamplesIndex.Find(name.string);
		i := 0;
		WHILE i < size DO
			index := allOffsets[i];
			IF monitor.IsMonitored(index) THEN
				start := monitor.Start(index);
				start := MAX(beg, start);
				IF monitor.SampleSize(index, start, end, thin) = 0 THEN
					allOffsets[i] :=  - 1
				END
			ELSE
				allOffsets[i] :=  - 1
			END;
			INC(i)
		END;
		i := 0;
		num := 0;
		WHILE i < size DO
			index := allOffsets[i];
			IF index #  - 1 THEN
				offsets[num] := index;
				INC(num)
			END;
			INC(i)
		END
	END Offsets;

	PROCEDURE SampleSize* (IN scalar: ARRAY OF CHAR;
	beg, end, thin, firstChain, lastChain: INTEGER): INTEGER;
		VAR
			numChains, sampleSize, start: INTEGER;
			offsets: POINTER TO ARRAY OF INTEGER;
			name: BugsNames.Name;
			var: BugsParser.Variable;
			monitor: SamplesMonitors.Monitor;
	BEGIN
		var := BugsParser.StringToVariable(scalar);
		offsets := BugsEvaluate.Offsets(var);
		ASSERT(LEN(offsets) = 1, 66);
		name := var.name;
		monitor := SamplesIndex.Find(name.string);
		start := monitor.Start(offsets[0]);
		start := MAX(beg, start);
		numChains := lastChain - firstChain + 1;
		sampleSize := monitor.SampleSize(offsets[0], start, end, thin) * numChains;
		RETURN sampleSize
	END SampleSize;

	PROCEDURE SampleValues* (IN scalar: ARRAY OF CHAR;
	beg, end, thin, firstChain, lastChain: INTEGER;
	OUT sample: ARRAY OF ARRAY OF REAL);
		VAR
			offsets: POINTER TO ARRAY OF INTEGER;
			name: BugsNames.Name;
			var: BugsParser.Variable;
			monitor: SamplesMonitors.Monitor;
	BEGIN
		var := BugsParser.StringToVariable(scalar);
		offsets := BugsEvaluate.Offsets(var);
		ASSERT(LEN(offsets) = 1, 66);
		name := var.name;
		monitor := SamplesIndex.Find(name.string);
		monitor.Sample(offsets[0], beg, end, thin, firstChain, lastChain, sample);
	END SampleValues;

	PROCEDURE SampleStart* (IN scalar: ARRAY OF CHAR; beg, end, thin: INTEGER): INTEGER;
		VAR
			start: INTEGER;
			offsets: POINTER TO ARRAY OF INTEGER;
			name: BugsNames.Name;
			var: BugsParser.Variable;
			monitor: SamplesMonitors.Monitor;
	BEGIN
		var := BugsParser.StringToVariable(scalar);
		offsets := BugsEvaluate.Offsets(var);
		ASSERT(LEN(offsets) = 1, 66);
		name := var.name;
		monitor := SamplesIndex.Find(name.string);
		start := monitor.Start(offsets[0]);
		start := MAX(beg, start);
		RETURN start
	END SampleStart;

	PROCEDURE Statistics* (IN scalar: ARRAY OF CHAR;
	beg, end, thin, firstChain, lastChain: INTEGER;
	IN fractions: ARRAY OF REAL;
	OUT mean, median, mode, sd, skew, exKur, error: REAL;
	OUT percentiles, lower, upper: ARRAY OF REAL;
	OUT start, sampleSize, ess: INTEGER);
		VAR
			offsets: POINTER TO ARRAY OF INTEGER;
			name: BugsNames.Name;
			var: BugsParser.Variable;
			monitor: SamplesMonitors.Monitor;
			sample: POINTER TO ARRAY OF ARRAY OF REAL;
			buffer: POINTER TO ARRAY OF REAL;
			i, j, n, n1, numChains: INTEGER;
			l, u: REAL;
	BEGIN
		ASSERT((LEN(fractions) = LEN(percentiles)) OR (LEN(fractions) = LEN(percentiles) + LEN(lower)), 20);
		var := BugsParser.StringToVariable(scalar);
		offsets := BugsEvaluate.Offsets(var);
		ASSERT(LEN(offsets) = 1, 66);
		name := var.name;
		monitor := SamplesIndex.Find(name.string);
		start := monitor.Start(offsets[0]);
		start := MAX(beg, start);
		numChains := lastChain - firstChain + 1;
		sampleSize := monitor.SampleSize(offsets[0], start, end, thin);
		NEW(sample, numChains, sampleSize);
		SampleValues(scalar, start, end, thin, firstChain, lastChain, sample);
		buffer := SamplesStatistics.Sort(sample);
		SamplesStatistics.Moments(sample, mean, sd, skew, exKur, error);
		ess := SHORT(ENTIER(sd * sd / (error * error)));
		median := SamplesStatistics.Percentile(buffer, 50.0);
		mode := SamplesStatistics.Mode(buffer);
		i := 0;
		n := LEN(percentiles);
		WHILE i < n DO
			percentiles[i] := SamplesStatistics.Percentile(buffer, fractions[i]);
			INC(i)
		END;
		n1 := LEN(fractions);
		WHILE i < n1 DO
			SamplesStatistics.HPDInterval(buffer, fractions[i], lower[i - n], upper[i - n]);
			INC(i)
		END;
		sampleSize := sampleSize * numChains;
		INC(start)
	END Statistics;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END SamplesInterface.
