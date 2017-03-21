(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	   *)

MODULE CorrelPlots;

	

	IMPORT
		BugsMappers, BugsNames, BugsParser, 
		CorrelBivariate, CorrelInterface, CorrelMatrix,
		PlotsViews, 
		SamplesIndex, SamplesInterface, SamplesMonitors;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		PROCEDURE ScatterPlot (monitor0, monitor1: SamplesMonitors.Monitor;
	beg, end, firstChain, lastChain, off0, off1: INTEGER): PlotsViews.View;
		VAR
			indices0, indices1, label0, label1: ARRAY 80 OF CHAR;
			samples0, samples1: POINTER TO ARRAY OF ARRAY OF REAL;
			v: PlotsViews.View;
			name0, name1: BugsNames.Name;
	BEGIN
		ASSERT(monitor0 # NIL, 21);
		ASSERT(monitor1 # NIL, 21);
		name0 := monitor0.Name();
		label0 := name0.string$;
		name0.Indices(off0, indices0);
		label0 := label0 + indices0;
		name1 := monitor1.Name();
		label1 := name1.string$;
		name1.Indices(off1, indices1);
		label1 := label1 + indices1;
		CorrelInterface.BivariateSample(monitor0, monitor1, beg, end, firstChain, lastChain, off0, off1,
		samples0, samples1);
		IF samples0 = NIL THEN
			RETURN NIL
		END;
		v := CorrelBivariate.New(label0, label1, samples0, samples1);
		RETURN v
	END ScatterPlot;

	PROCEDURE MakePlots (string0, string1: ARRAY OF CHAR; beg, end, thin, firstChain, lastChain: INTEGER;
	VAR f: BugsMappers.Formatter);
		VAR
			col, i, h, j, num0, num1, row, w: INTEGER;
			offsets0, offsets1: POINTER TO ARRAY OF INTEGER;
			var0, var1: BugsParser.Variable;
			name: BugsNames.Name;
			monitor0, monitor1: SamplesMonitors.Monitor;
			v: PlotsViews.View;
	BEGIN
		var0 := BugsParser.StringToVariable(string0);
		name := var0.name;
		monitor0 := SamplesIndex.Find(name.string);
		num0 := SamplesInterface.NumComponents(string0, beg, end, thin);
		IF num0 = 0 THEN RETURN END;
		NEW(offsets0, num0);
		SamplesInterface.Offsets(string0, beg, end, thin, offsets0);
		var1 := BugsParser.StringToVariable(string1);
		name := var1.name;
		monitor1 := SamplesIndex.Find(name.string);
		num1 := SamplesInterface.NumComponents(string1, beg, end, thin);
		IF num1 = 0 THEN RETURN END;
		NEW(offsets1, num1);
		SamplesInterface.Offsets(string1, beg, end, thin, offsets1);
		i := 0;
		WHILE i < num0 DO
			row := offsets0[i];
			j := 0;
			WHILE j < num1 DO
				col := offsets1[j];
				IF (string0 # string1) OR (row < col) THEN
					v := ScatterPlot(monitor0, monitor1, beg, end, firstChain, lastChain, row, col);
					IF v # NIL THEN
						v.MinSize(w, h);
						f.WriteView(v, w, h)
					END
				END;
				INC(j)
			END;
			INC(i)
		END
	END MakePlots;

	PROCEDURE ScatterPlots* (string0, string1: ARRAY OF CHAR;
	beg, end, thin, firstChain, lastChain: INTEGER; VAR f: BugsMappers.Formatter);
		VAR
			var0, var1: BugsParser.Variable;
	BEGIN
		var0 := BugsParser.StringToVariable(string0);
		IF var0 = NIL THEN
			RETURN
		END;
		IF string1 = "" THEN
			string1 := string0$
		END;
		var1 := BugsParser.StringToVariable(string1);
		IF var1 = NIL THEN
			RETURN
		END;
		MakePlots(string0, string1, beg, end, thin, firstChain, lastChain, f)
	END ScatterPlots;

	PROCEDURE MatrixPlot* (xName, yName: ARRAY OF CHAR;
	beg, end, thin, firstChain, lastChain: INTEGER; VAR f: BugsMappers.Formatter);
		VAR
			xNum, yNum, w, h: INTEGER;
			matrix: POINTER TO ARRAY OF ARRAY OF REAL;
			xVar, yVar: BugsParser.Variable;
			plot: PlotsViews.View;
	BEGIN
		xVar := BugsParser.StringToVariable(xName);
		IF xVar = NIL THEN RETURN END;
		xNum := SamplesInterface.NumComponents(xName, beg, end, thin);
		IF xNum = 0 THEN RETURN END;
		IF yName = "" THEN yName := xName$ END;
		yVar := BugsParser.StringToVariable(yName);
		IF yVar = NIL THEN RETURN END;
		yNum := SamplesInterface.NumComponents(yName, beg, end, thin);
		IF yNum = 0 THEN RETURN END;
		matrix := CorrelInterface.CorrelationMatrix(xName, yName, beg, end, thin, firstChain, lastChain);
		IF matrix = NIL THEN RETURN END;
		plot := CorrelMatrix.New(matrix, xName, yName, beg, end, thin);
		IF plot # NIL THEN
			plot.MinSize(w, h);
			f.WriteView(plot, w, h)
		END
	END MatrixPlot;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END CorrelPlots.
