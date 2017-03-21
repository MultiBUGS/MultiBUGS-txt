(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	   *)

MODULE CorrelInterface;

	

	IMPORT
		BugsNames, BugsParser,
		MathFunc,
		SamplesIndex, SamplesInterface, SamplesMonitors;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 20 OF CHAR;

		PROCEDURE BivariateSample* (monitor0, monitor1: SamplesMonitors.Monitor;
		beg, end, firstChain, lastChain, off0, off1: INTEGER;
	OUT samples0, samples1: POINTER TO ARRAY OF ARRAY OF REAL);
		VAR
			end0, end1, numChains, sampleSize, sampleSize0, sampleSize1,
			start, start0, start1: INTEGER;
	BEGIN
		ASSERT(monitor0 # NIL, 21);
		ASSERT(monitor1 # NIL, 21);
		sampleSize0 := monitor0.SampleSize(off0, beg, end, 1);
		start0 := MAX(monitor0.Start(off0), beg);
		end0 := start0 + sampleSize0;
		sampleSize1 := monitor1.SampleSize(off1, beg, end, 1);
		start1 := MAX(monitor1.Start(off1), beg);
		end1 := start1 + sampleSize1;
		start := MAX(start0, start1);
		end := MIN(end0, end1);
		numChains := lastChain - firstChain + 1;
		sampleSize := end - start;
		IF sampleSize = 0 THEN
			samples0 := NIL;
			samples1 := NIL;
			RETURN
		END;
		NEW(samples0, numChains, sampleSize);
		NEW(samples1, numChains, sampleSize);
		monitor0.Sample(off0, start, end, 1, firstChain, lastChain, samples0);
		monitor1.Sample(off1, start, end, 1, firstChain, lastChain, samples1)
	END BivariateSample;

	PROCEDURE BivariateSample1 (monitor0, monitor1: SamplesMonitors.Monitor;
	beg, end, firstChain, lastChain, off0, off1: INTEGER;
	OUT x0, x1: POINTER TO ARRAY OF REAL);
		VAR
			i, j, len, len0, len1: INTEGER;
			samples0, samples1: POINTER TO ARRAY OF ARRAY OF REAL;
	BEGIN
		BivariateSample(monitor0, monitor1, beg, end, firstChain, lastChain, off0, off1, samples0, samples1);
		IF samples0 = NIL THEN
			x0 := NIL;
			x1 := NIL;
			RETURN
		END;
		len0 := LEN(samples0, 0);
		len1 := LEN(samples0, 1);
		len := len0 * len1;
		NEW(x0, len);
		i := 0;
		WHILE i < len0 DO
			j := 0;
			WHILE j < len1 DO
				x0[i * len1 + j] := samples0[i, j];
				INC(j)
			END;
			INC(i)
		END;
		len0 := LEN(samples1, 0);
		len1 := LEN(samples1, 1);
		len := len0 * len1;
		NEW(x1, len);
		i := 0;
		WHILE i < len0 DO
			j := 0;
			WHILE j < len1 DO
				x1[i * len1 + j] := samples1[i, j];
				INC(j)
			END;
			INC(i)
		END
	END BivariateSample1;

	PROCEDURE CorrelationMatrix* (name0, name1: ARRAY OF CHAR;
	beg, end, thin, firstChain, lastChain: INTEGER): POINTER TO ARRAY OF ARRAY OF REAL;
		VAR
			col, i, j, len, num0, num1, row: INTEGER;
			offsets0, offsets1: POINTER TO ARRAY OF INTEGER;
			x0, x1: POINTER TO ARRAY OF REAL;
			matrix: POINTER TO ARRAY OF ARRAY OF REAL;
			var0, var1: BugsParser.Variable;
			name: BugsNames.Name;
			monitor0, monitor1: SamplesMonitors.Monitor;
	BEGIN
		var0 := BugsParser.StringToVariable(name0);
		name := var0.name;
		monitor0 := SamplesIndex.Find(name.string);
		num0 := SamplesInterface.NumComponents(name0, beg, end, thin);
		IF num0 = 0 THEN RETURN NIL END;
		NEW(offsets0, num0);
		SamplesInterface.Offsets(name0, beg, end, thin, offsets0);
		var1 := BugsParser.StringToVariable(name1);
		name := var1.name;
		monitor1 := SamplesIndex.Find(name.string);
		num1 := SamplesInterface.NumComponents(name1, beg, end, thin);
		IF num1 = 0 THEN RETURN NIL END;
		NEW(offsets1, num1);
		SamplesInterface.Offsets(name1, beg, end, thin, offsets1);
		num1 := LEN(offsets1);
		NEW(matrix, num0, num1);
		i := 0;
		WHILE i < num0 DO
			row := offsets0[i];
			j := 0;
			WHILE j < num1 DO
				col := offsets1[j];
				BivariateSample1(monitor0, monitor1, beg, end, firstChain, lastChain, row, col, x0, x1);
				IF x0 # NIL THEN
					len := LEN(x0);
					matrix[i, j] := MathFunc.Correlation(x0, x1, len)
				ELSE
					matrix[i, j] := 0.0
				END;
				INC(j)
			END;
			INC(i)
		END;
		RETURN matrix
	END CorrelationMatrix;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END CorrelInterface.
