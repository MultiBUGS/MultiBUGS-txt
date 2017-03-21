(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE MapsCmds;


	

	IMPORT
		Containers, Dialog, Files, Ports, Strings, Views,
		StdLog,
		TextModels, TextViews,
		BugsCmds, BugsIndex, BugsMappers, BugsMsg, BugsNames, BugsTexts,
		MapsAdjacency, MapsIndex, MapsMap, MapsViews, MapsViews1,
		MathSort,
		PlotsAxis, PlotsViews,
		SamplesIndex, SamplesInterface, SamplesMonitors, SamplesStatistics,
		SummaryIndex, SummaryMonitors;

	CONST
		blues* = 0;
		grey* = 1;
		rainbow* = 2;
		custom = 3;
		maxCuts = 6;
		default* = 0;
		value* = 0;
		samplesMean* = 1;
		quantile* = 2;
		greaterThan* = 3;
		lessThan* = 4;
		summaryMean* = 5;
		summaryMedian* = 6;
		eps = 1.0E-10;

	TYPE

		Dialog0* = RECORD
			maps*: Dialog.List;
			quantity*: ARRAY 80 OF CHAR;
			cutType*: INTEGER;
			numCuts*: Dialog.List;
			numberCuts: INTEGER;
			cuts*: ARRAY 6 OF REAL;
			paletteOptions*: Dialog.List;
			palette*: ARRAY 7 OF Ports.Color;
			plotType*: Dialog.List;
			beg*, end*: INTEGER;
			fraction*, threshold*: REAL
		END;

		Dialog1* = RECORD
			maps*: Dialog.List;
			id*: INTEGER
		END;

		String = ARRAY 80 OF CHAR;

	VAR

		dialog*: Dialog0;
		dialog1*: Dialog1;
		bluePalette, greyPalette, rainbowPalette, customPalette: ARRAY maxCuts + 1 OF Ports.Color;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Error (errorNum: INTEGER);
		VAR
			numToString: ARRAY 8 OF CHAR;
			errorMes: ARRAY 1024 OF CHAR;
	BEGIN
		Strings.IntToString(errorNum, numToString);
		BugsMsg.MapMsg("MapsCmds" + numToString, errorMes);
		BugsMsg.StoreError(errorMes);
		BugsMsg.GetError(errorMes);
		BugsTexts.ShowMsg(errorMes);
	END Error;

	PROCEDURE UpdateNames*;
		VAR
			i, num: INTEGER;
			mapNames: POINTER TO ARRAY OF Dialog.String;
	BEGIN
		mapNames := MapsIndex.GetMaps();
		IF mapNames # NIL THEN
			num := LEN(mapNames, 0)
		ELSE
			num := 0
		END;
		i := 0;
		WHILE i < num DO
			dialog.maps.SetItem(i, mapNames[i]);
			dialog1.maps.SetItem(i, mapNames[i]);
			INC(i)
		END;
		dialog.maps.SetLen(num);
		Dialog.UpdateList(dialog);
		dialog1.maps.SetLen(num);
		Dialog.UpdateList(dialog1)
	END UpdateNames;

	PROCEDURE Load (IN name: ARRAY OF CHAR): MapsMap.Map;
		VAR
			map: MapsMap.Map;
			file: Files.File;
			loc: Files.Locator;
	BEGIN
		map := MapsIndex.Find(name);
		IF map = NIL THEN
			loc := Files.dir.This("Maps/Rsrc");
			file := Files.dir.Old(loc, name + ".map", Files.shared);
			map := MapsMap.New(file, name);
			MapsIndex.Store(map)
		END;
		RETURN map
	END Load;

	PROCEDURE AdjacencyMatrix*;
		VAR
			i, numRegions, sumNumNeigh: INTEGER;
			cursor: MapsAdjacency.List;
			adjacency: MapsAdjacency.Adjacency;
			t: TextModels.Model;
			f: BugsMappers.Formatter;
			v: TextViews.View;
	BEGIN
		adjacency := MapsViews1.FocusAdjacency();
		t := TextModels.dir.New();
		BugsTexts.ConnectFormatter(f, t);
		v := TextViews.dir.New(t);
		i := 0;
		numRegions := LEN(adjacency.neighbours);
		f.WriteString("list( num = c(");
		i := 0;
		WHILE i < numRegions DO
			f.WriteInt(adjacency.NumNeighbours(i + 1));
			INC(i);
			IF i < numRegions THEN
				f.WriteString(", ")
			END;
			IF i MOD 10 = 0 THEN
				f.WriteLn
			END
		END;
		f.WriteLn;
		f.WriteString("),");
		f.WriteLn;
		f.WriteString("adj = c(");
		f.WriteLn;
		i := 0;
		sumNumNeigh := 0;
		WHILE i < numRegions DO
			cursor := adjacency.neighbours[i];
			INC(i);
			WHILE cursor # NIL DO
				IF cursor.index > 0 THEN
					f.WriteInt(cursor.index)
				END;
				IF ((cursor.next # NIL) OR (i < numRegions)) & (cursor.index > 0) THEN
					f.WriteString(", ")
				END;
				INC(sumNumNeigh);
				cursor := cursor.next
			END;
			f.WriteLn
		END;
		f.WriteString("),");
		f.WriteLn; f.WriteString("sumNumNeigh = ");
		f.WriteInt(sumNumNeigh);
		f.WriteString(")");
		Views.OpenAux(v, "Adjacency Matrix")
	END AdjacencyMatrix;

	PROCEDURE OpenAux (v: Views.View; title: ARRAY OF CHAR);
		VAR
			c: Containers.Controller;
			v1: Views.View;
	BEGIN
		Views.OpenAux(v, title$);
		c := Containers.Focus();
		IF c # NIL THEN
			c.GetFirstView(Containers.any, v1);
			IF v = v1 THEN
				c.SetOpts(c.opts - {Containers.noSelection})
			END
		END
	END OpenAux;

	PROCEDURE AdjacencyMap*;
		VAR
			index: INTEGER;
			name: Dialog.String;
			view: PlotsViews.View;
			map: MapsMap.Map;
	BEGIN
		index := dialog1.maps.index;
		dialog.maps.GetItem(index, name);
		map := Load(name);
		view := MapsViews1.New("Adjacency Map", map);
		OpenAux(view, "Adjacency Map")
	END AdjacencyMap;

	PROCEDURE AbsoluteCuts (min, max: REAL): POINTER TO ARRAY OF REAL;
		VAR
			i, num: INTEGER;
			end, range, spacing: REAL;
			cuts: POINTER TO ARRAY OF REAL;
	BEGIN
		range := max - min;
		spacing := PlotsAxis.Spacing(range);
		num := SHORT(ENTIER(range / spacing));
		IF num < 3 THEN
			spacing := 0.5 * spacing
		END;
		end := min;
		WHILE end < max DO
			end := end + spacing
		END;
		end := ABS(end);
		num := SHORT(ENTIER(end / spacing));
		end := num * spacing; IF max < 0 THEN end := - end END;
		IF end < max THEN end := end + spacing END; max := end;
		num := 0; end := max;
		WHILE end > min DO
			end := end - spacing;
			INC(num)
		END;
		min := max - num * spacing;
		NEW(cuts, num - 1);
		i := 1;
		WHILE i < num DO
			cuts[i - 1] := min + i * spacing;
			INC(i)
		END;
		RETURN cuts
	END AbsoluteCuts;

	PROCEDURE DrawMap (title: ARRAY OF CHAR; min, max: REAL; IN values: ARRAY OF REAL;
	IN mask: ARRAY OF BOOLEAN);
		VAR
			i, index, numCuts: INTEGER;
			cuts: POINTER TO ARRAY OF REAL;
			view: PlotsViews.View;
			map: MapsMap.Map;
			mapName: Dialog.String;
	BEGIN
		CASE dialog.cutType OF
		|MapsViews.absCut:
			IF dialog.numCuts.index = default THEN
				cuts := AbsoluteCuts(min, max)
			ELSE
				NEW(cuts, dialog.numCuts.index);
				i := 0;
				WHILE i < dialog.numCuts.index DO
					cuts[i] := dialog.cuts[i];
					INC(i)
				END;
				MathSort.HeapSort(cuts, dialog.numCuts.index)
			END
		|MapsViews.percentileCut:
			NEW(cuts, 3);
			cuts[0] := 10;
			cuts[1] := 50;
			cuts[2] := 90
		END;
		numCuts := LEN(cuts);
		dialog.numberCuts := numCuts;
		i := 0;
		WHILE i < numCuts DO
			dialog.cuts[i] := cuts[i];
			INC(i)
		END;
		index := dialog.maps.index;
		dialog.maps.GetItem(index, mapName);
		map := Load(mapName);
		IF LEN(values) = map.numReg THEN
			view := MapsViews.New(title, map, numCuts, dialog.cutType, cuts,
			values, mask, dialog.palette);
			IF BugsMappers.whereOut = BugsMappers.window THEN
				OpenAux(view, title)
			ELSE
				StdLog.View(view)
			END
		ELSE
			Error(1); RETURN
		END;
		Dialog.Update(dialog)
	END DrawMap;

	PROCEDURE GetValues (OUT min, max: REAL; OUT values: POINTER TO ARRAY OF REAL;
	OUT mask: POINTER TO ARRAY OF BOOLEAN; OUT ok: BOOLEAN);
		VAR
			i, len: INTEGER;
			name: BugsNames.Name;
	BEGIN
		values := NIL;
		mask := NIL;
		ok := TRUE;
		name := BugsIndex.Find(dialog.quantity);
		IF name = NIL THEN
			ok := FALSE; Error(2); RETURN
		END;
		len := name.Size();
		i := 0;
		WHILE (i < len) & (name.components[i] = NIL) DO
			INC(i)
		END;
		IF i < len THEN
			min := name.components[i].Value();
			max := min
		ELSE
			ok := FALSE; Error(3); RETURN
		END;
		NEW(values, len);
		NEW(mask, len);
		i := 0;
		WHILE i < len DO
			IF name.components[i] # NIL THEN
				values[i] := name.components[i].Value();
				min := MIN(values[i], min);
				max := MAX(values[i], max);
				mask[i] := TRUE
			ELSE
				values[i] := 0.0;
				mask[i] := FALSE
			END;
			INC(i)
		END;
		IF max - min < eps THEN
			ok := FALSE; Error(4); RETURN
		END
	END GetValues;

	PROCEDURE GetSummaryMeans (OUT min, max: REAL; OUT values: POINTER TO ARRAY OF REAL;
	OUT mask: POINTER TO ARRAY OF BOOLEAN; OUT ok: BOOLEAN);
		VAR
			i, len: INTEGER;
			lower, median, sd, skew, exKur, upper: REAL;
			monitor: SummaryMonitors.Monitor;
			name: BugsNames.Name;
	BEGIN
		monitor := SummaryIndex.Find(dialog.quantity);
		ok := TRUE;
		IF monitor = NIL THEN
			ok := FALSE; Error(5); RETURN
		END;
		name := monitor.Name();
		len := name.Size();
		NEW(values, len);
		NEW(mask, len);
		i := 0;
		WHILE (i < len) & ~monitor.IsMonitored(i)
			DO
		INC(i) END;
		IF i < len THEN
			monitor.Statistics(i, min, sd, skew, exKur, lower, median, upper);
			max := min
		ELSE
			ok := FALSE; Error(3); RETURN
		END;
		i := 0;
		WHILE i < len DO
			IF monitor.IsMonitored(i) THEN
				monitor.Statistics(i, values[i], sd, skew, exKur, lower, median, upper);
				min := MIN(values[i], min);
				max := MAX(values[i], max);
				mask[i] := TRUE
			ELSE
				values[i] := 0.0;
				mask[i] := FALSE
			END;
			INC(i)
		END;
		IF max - min < eps THEN
			ok := FALSE; Error(4); RETURN
		END
	END GetSummaryMeans;

	PROCEDURE GetSummaryMedians (OUT min, max: REAL;
	OUT values: POINTER TO ARRAY OF REAL; OUT mask: POINTER TO ARRAY OF BOOLEAN;
	OUT ok: BOOLEAN);
		VAR
			i, len: INTEGER;
			lower, mean, sd, skew, exKur, upper: REAL;
			monitor: SummaryMonitors.Monitor;
			name: BugsNames.Name;
	BEGIN
		monitor := SummaryIndex.Find(dialog.quantity);
		ok := TRUE;
		IF monitor = NIL THEN
			ok := FALSE; Error(5); RETURN
		END;
		name := monitor.Name();
		len := name.Size();
		NEW(values, len);
		NEW(mask, len);
		i := 0;
		WHILE (i < len) & ~monitor.IsMonitored(i) DO
			INC(i)
		END;
		IF i < len THEN
			monitor.Statistics(i, mean, sd, skew, exKur, lower, min, upper);
			max := min
		ELSE
			ok := FALSE; Error(3); RETURN
		END;
		i := 0;
		WHILE i < len DO
			IF monitor.IsMonitored(i) THEN
				monitor.Statistics(i, mean, sd, skew, exKur, lower, values[i], upper);
				min := MIN(values[i], min);
				max := MAX(values[i], max);
				mask[i] := TRUE
			ELSE
				values[i] := 0.0;
				mask[i] := FALSE
			END;
			INC(i)
		END;
		IF max - min < eps THEN
			ok := FALSE; Error(4); RETURN
		END
	END GetSummaryMedians;

	PROCEDURE GetSamplesMeans (OUT min, max: REAL;
	OUT values: POINTER TO ARRAY OF REAL; OUT mask: POINTER TO ARRAY OF BOOLEAN;
	OUT ok: BOOLEAN);
		VAR
			i, len, step, beg, end, firstChain, lastChain, numChains, sampleSize: INTEGER;
			sd, skew, exKur, error: REAL;
			monitor: SamplesMonitors.Monitor;
			name: BugsNames.Name;
			sample: POINTER TO ARRAY OF ARRAY OF REAL;
			scalar: ARRAY 128 OF CHAR;
	BEGIN
		step := 1;
		beg := dialog.beg;
		end := dialog.end;
		ok := TRUE;
		firstChain := 1;
		lastChain := BugsCmds.specificationDialog.numChains;
		monitor := SamplesIndex.Find(dialog.quantity);
		IF monitor = NIL THEN
			ok := FALSE; Error(6); RETURN
		END;
		name := monitor.Name();
		len := name.Size();
		NEW(values, len);
		NEW(mask, len);
		i := 0;
		WHILE (i < len) & ~monitor.IsMonitored(i) DO
			INC(i)
		END;
		IF i < len THEN
			max := - INF;
			min := + INF;
		ELSE
			ok := FALSE; Error(3); RETURN
		END;
		i := 0;
		WHILE i < len DO
			IF monitor.IsMonitored(i) THEN
				name.Indices(i, scalar);
				scalar := name.string + scalar;
				sampleSize := SamplesInterface.SampleSize(scalar, beg, end, step, firstChain, lastChain);
				numChains := lastChain - firstChain + 1;
				sampleSize := sampleSize DIV numChains;
				NEW(sample, numChains, sampleSize);
				monitor.Sample(i, beg, end, step, firstChain, lastChain, sample);
				values[i] := SamplesStatistics.Mean(sample);
				min := MIN(values[i], min);
				max := MAX(values[i], max);
				mask[i] := TRUE
			ELSE
				values[i] := 0.0;
				mask[i] := FALSE
			END;
			INC(i)
		END;
		IF max - min < eps THEN
			ok := FALSE; Error(4); RETURN
		END
	END GetSamplesMeans;

	PROCEDURE GetQuantiles (OUT min, max: REAL;
	OUT values: POINTER TO ARRAY OF REAL; OUT mask: POINTER TO ARRAY OF BOOLEAN;
	OUT ok: BOOLEAN);
		VAR
			i, j, len, beg, end, firstChain, lastChain, numChains, step, sampleSize: INTEGER;
			quantile, fraction: REAL;
			monitor: SamplesMonitors.Monitor;
			name: BugsNames.Name;
			scalar: ARRAY 128 OF CHAR;
			sample: POINTER TO ARRAY OF ARRAY OF REAL;
			buffer: POINTER TO ARRAY OF REAL;
	BEGIN
		step := 1;
		beg := dialog.beg;
		end := dialog.end;
		ok := TRUE;
		firstChain := 1;
		lastChain := BugsCmds.specificationDialog.numChains;
		fraction := dialog.fraction;
		monitor := SamplesIndex.Find(dialog.quantity);
		IF monitor = NIL THEN
			ok := FALSE; Error(6); RETURN
		END;
		name := monitor.Name();
		len := name.Size();
		NEW(values, len);
		NEW(mask, len);
		i := 0;
		WHILE (i < len) & ~monitor.IsMonitored(i) DO
			INC(i)
		END;
		IF i < len THEN
			min := + INF;
			max := - INF
		ELSE
			ok := FALSE; Error(3); RETURN
		END;
		i := 0;
		WHILE i < len DO
			IF monitor.IsMonitored(i) THEN
				name.Indices(i, scalar);
				scalar := name.string + scalar;
				sampleSize := SamplesInterface.SampleSize(scalar, beg, end, step, firstChain, lastChain);
				numChains := lastChain - firstChain + 1;
				SamplesInterface.SampleValues(scalar, beg, end, step, firstChain, lastChain, sample);
				buffer := SamplesStatistics.Sort(sample);
				quantile := SamplesStatistics.Percentile(buffer, fraction);
				values[i] := quantile;
				min := MIN(values[i], min);
				max := MAX(values[i], max);
				mask[i] := TRUE
			ELSE
				values[i] := 0.0;
				mask[i] := FALSE
			END;
			INC(i)
		END;
		IF max - min < eps THEN
			ok := FALSE; Error(4); RETURN
		END
	END GetQuantiles;

	PROCEDURE ExceedenceProb (monitor: SamplesMonitors.Monitor;
	offset, beg, end, step, firstChain, lastChain: INTEGER; threshold: REAL): REAL;
		VAR
			i, j, noChains, sampleSize, prob: INTEGER;
			sample: POINTER TO ARRAY OF ARRAY OF REAL;
	BEGIN
		sampleSize := monitor.SampleSize(offset, beg, end, step);
		ASSERT(sampleSize > 0, 60);
		noChains := lastChain - firstChain + 1;
		NEW(sample, noChains, sampleSize);
		monitor.Sample(offset, beg, end, step, firstChain, lastChain, sample);
		i := 0; prob := 0;
		WHILE i < noChains DO
			j := 0;
			WHILE j < sampleSize DO
				IF sample[i, j] > threshold THEN
					INC(prob)
				END;
				INC(j)
			END;
			INC(i)
		END;
		RETURN prob / (sampleSize * noChains)
	END ExceedenceProb;

	PROCEDURE GetThresholds (lessThan: BOOLEAN; OUT min, max: REAL;
	OUT values: POINTER TO ARRAY OF REAL; OUT mask: POINTER TO ARRAY OF BOOLEAN;
	OUT ok: BOOLEAN);
		VAR
			i, len, beg, end, firstChain, lastChain, step: INTEGER;
			threshold: REAL;
			monitor: SamplesMonitors.Monitor;
			name: BugsNames.Name;
	BEGIN
		step := 1;
		beg := dialog.beg;
		end := dialog.end;
		threshold := dialog.threshold;
		ok := TRUE;
		firstChain := 1;
		lastChain := BugsCmds.specificationDialog.numChains;
		monitor := SamplesIndex.Find(dialog.quantity);
		IF monitor = NIL THEN
			ok := FALSE; Error(6); RETURN
		END;
		name := monitor.Name();
		len := name.Size();
		NEW(values, len);
		NEW(mask, len);
		i := 0;
		WHILE (i < len) & ~monitor.IsMonitored(i) DO
			INC(i)
		END;
		IF i < len THEN
			values[i] := ExceedenceProb(monitor, i, beg, end, step, firstChain, lastChain, threshold);
			IF lessThan THEN
				values[i] := 1.0 - values[i]
			END;
			min := values[i];
			max := min
		ELSE
			ok := FALSE; Error(3); RETURN
		END;
		i := 0;
		WHILE i < len DO
			IF monitor.IsMonitored(i) THEN
				values[i] := ExceedenceProb(monitor, i, beg, end, step, firstChain, lastChain, threshold);
				IF lessThan THEN
					values[i] := 1.0 - values[i]
				END;
				min := MIN(values[i], min);
				max := MAX(values[i], max);
				mask[i] := TRUE
			ELSE
				values[i] := 0.0;
				mask[i] := FALSE
			END;
			INC(i)
		END;
		IF max - min < eps THEN
			ok := FALSE; Error(4); RETURN
		END
	END GetThresholds;

	PROCEDURE Plot*;
		VAR
			ok: BOOLEAN;
			min, max: REAL;
			title: ARRAY 80 OF CHAR;
			values: POINTER TO ARRAY OF REAL;
			mask: POINTER TO ARRAY OF BOOLEAN;
	BEGIN
		CASE dialog.plotType.index OF
		|value:
			GetValues(min, max, values, mask, ok);
			title := "values for " + dialog.quantity
		|samplesMean:
			GetSamplesMeans(min, max, values, mask, ok);
			title := "(samples)means for " + dialog.quantity
		|quantile:
			GetQuantiles(min, max, values, mask, ok);
			Strings.RealToStringForm(dialog.fraction, 3, 4, 0, " ", title);
			IF ABS(dialog.fraction - 50) < eps THEN
				title := "median for " + dialog.quantity
			ELSE
				title := title + "% for " + dialog.quantity
			END
		|greaterThan:
			GetThresholds(FALSE, min, max, values, mask, ok);
			Strings.RealToStringForm(dialog.threshold, 3, 4, 0, " ", title);
			title := "probability of " + dialog.quantity + " greater than " + title
		|lessThan:
			GetThresholds(TRUE, min, max, values, mask, ok);
			Strings.RealToStringForm(dialog.threshold, 3, 4, 0, " ", title);
			title := "probability of " + dialog.quantity + " less than " + title
		|summaryMean:
			GetSummaryMeans(min, max, values, mask, ok);
			title := "(summary)means for " + dialog.quantity
		|summaryMedian:
			GetSummaryMedians(min, max, values, mask, ok);
			title := "(summary)medians for " + dialog.quantity
		END;
		IF ~ok THEN RETURN END;
		DrawMap(title, min, max, values, mask)
	END Plot;

	PROCEDURE GetCuts*;
		VAR
			cuts: ARRAY 10 OF REAL; i, numCuts, typeCuts: INTEGER;
	BEGIN
		MapsViews.GetCuts(dialog.cutType, cuts, numCuts, typeCuts);
		dialog.numCuts.index := numCuts;
		dialog.numberCuts := numCuts;
		dialog.cutType := typeCuts;
		i := 0;
		WHILE i < numCuts DO
			dialog.cuts[i] := cuts[i];
			INC(i)
		END;
		Dialog.Update(dialog)
	END GetCuts;

	PROCEDURE SetCuts*;
		VAR
			i, numCuts, typeCuts: INTEGER;
			cuts: POINTER TO ARRAY OF REAL;
	BEGIN
		numCuts := dialog.numCuts.index;
		IF numCuts = 0 THEN
			numCuts := dialog.numberCuts
		END;
		typeCuts := dialog.cutType;
		NEW(cuts, numCuts);
		i := 0;
		WHILE i < numCuts DO
			cuts[i] := dialog.cuts[i];
			INC(i)
		END;
		MathSort.HeapSort(cuts, numCuts);
		MapsViews.SetCuts(cuts, numCuts, typeCuts, dialog.palette)
	END SetCuts;

	PROCEDURE SetRegion*;
	BEGIN
		MapsViews1.FocusSetRegion(dialog1.id)
	END SetRegion;

	PROCEDURE Print*;
		VAR
			map: MapsMap.Map;
			f: BugsMappers.Formatter;
			t: TextModels.Model;
			v: TextViews.View;
	BEGIN
		map := MapsViews.FocusMap();
		IF map = NIL THEN
			map := MapsViews1.FocusMap()
		END;
		IF map # NIL THEN
			t := TextModels.dir.New();
			BugsTexts.ConnectFormatter(f, t);
			map.Print(f);
			v := TextViews.dir.New(t);
			Views.OpenAux(v, "Splus map")
		END
	END Print;

	PROCEDURE ValueGuard* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := BugsIndex.Find(dialog.quantity) = NIL
	END ValueGuard;

	PROCEDURE SummaryGuard* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := SummaryIndex.Find(dialog.quantity) = NIL
	END SummaryGuard;

	PROCEDURE SamplesGuard* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := SamplesIndex.Find(dialog.quantity) = NIL
	END SamplesGuard;

	PROCEDURE PlotGuard* (VAR par: Dialog.Par);
	BEGIN
		CASE dialog.plotType.index OF
		|value:
			ValueGuard(par)
		|summaryMean, summaryMedian:
			SummaryGuard(par)
		|samplesMean, quantile, greaterThan, lessThan:
			SamplesGuard(par)
		END
	END PlotGuard;

	PROCEDURE FractionGuard* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := dialog.plotType.index # quantile
	END FractionGuard;

	PROCEDURE ThresholdGuard* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~(dialog.plotType.index IN {greaterThan, lessThan})
	END ThresholdGuard;

	PROCEDURE AdjacencyMatrixGuard* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := MapsViews1.Focus() = NIL
	END AdjacencyMatrixGuard;

	PROCEDURE GetCutsGuard* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := MapsViews.Focus() = NIL
	END GetCutsGuard;

	PROCEDURE SetCutsGuard* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := (MapsViews.Focus() = NIL) OR (dialog.numberCuts = 0)
	END SetCutsGuard;


	PROCEDURE CutGuard0* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := dialog.numberCuts < 1
	END CutGuard0;

	PROCEDURE CutGuard1* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := dialog.numberCuts < 2
	END CutGuard1;

	PROCEDURE CutGuard2* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := dialog.numberCuts < 3
	END CutGuard2;

	PROCEDURE CutGuard3* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := dialog.numberCuts < 4
	END CutGuard3;

	PROCEDURE CutGuard4* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := dialog.numberCuts < 5
	END CutGuard4;

	PROCEDURE CutGuard5* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := dialog.numberCuts < 6
	END CutGuard5;

	PROCEDURE PrintGuard* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := (MapsViews.FocusMap() = NIL) & (MapsViews1.FocusMap() = NIL)
	END PrintGuard;

	PROCEDURE PaletteGuard* (VAR par: Dialog.Par);
	BEGIN
		par.readOnly := dialog.paletteOptions.index IN {blues, grey, rainbow}
	END PaletteGuard;

	PROCEDURE NumCutNotifier* (op, to, from: INTEGER);
	BEGIN
		IF dialog.cutType = MapsViews.percentileCut THEN
			IF dialog.numCuts.index = default THEN
				dialog.cuts[0] := 10.0;
				dialog.cuts[1] := 50.0;
				dialog.cuts[2] := 90.0;
				dialog.numberCuts := 3
			END
		ELSE
			dialog.numberCuts := dialog.numCuts.index
		END;
		Dialog.Update(dialog)
	END NumCutNotifier;

	PROCEDURE TypeCutNotifier* (op, to, from: INTEGER);
	BEGIN
		IF (dialog.cutType = MapsViews.percentileCut) & (dialog.numCuts.index = default) THEN
			dialog.cuts[0] := 10.0;
			dialog.cuts[1] := 50.0;
			dialog.cuts[2] := 90.0;
			dialog.numberCuts := 3;
			Dialog.Update(dialog)
		END
	END TypeCutNotifier;

	PROCEDURE PaletteNotifier* (op, to, from: INTEGER);
		VAR
			i, len: INTEGER;
	BEGIN
		CASE dialog.paletteOptions.index OF
		|blues:
			i := 0;
			len := LEN(bluePalette);
			WHILE i < len DO
				dialog.palette[i] := bluePalette[i];
				INC(i)
			END;
			Dialog.Update(dialog)
		|grey:
			i := 0;
			len := LEN(greyPalette);
			WHILE i < len DO
				dialog.palette[i] := greyPalette[i];
				INC(i)
			END;
			Dialog.Update(dialog)
		|rainbow:
			i := 0;
			len := LEN(rainbowPalette);
			WHILE i < len DO
				dialog.palette[i] := rainbowPalette[i];
				INC(i)
			END;
			Dialog.Update(dialog)
		|custom:
			i := 0;
			len := LEN(customPalette);
			WHILE i < len DO
				dialog.palette[i] := customPalette[i];
				INC(i)
			END;
			Dialog.Update(dialog)
		END
	END PaletteNotifier;

	PROCEDURE ColourNotifier* (op, to, from: INTEGER);
		VAR
			i, len: INTEGER;
	BEGIN
		IF dialog.paletteOptions.index = custom THEN
			i := 0;
			len := LEN(customPalette);
			WHILE i < len DO
				customPalette[i] := dialog.palette[i];
				INC(i)
			END;
			Dialog.Update(dialog)
		END
	END ColourNotifier;

	PROCEDURE SetAbsoluteCut*;
	BEGIN
		dialog.cutType := MapsViews.absCut;
		Dialog.Update(dialog)
	END SetAbsoluteCut;

	PROCEDURE SetDefaultCuts*;
	BEGIN
		dialog.numCuts.index := default;
		NumCutNotifier(0, 0, 0)
	END SetDefaultCuts;

	PROCEDURE SetMap* (name: ARRAY OF CHAR);
		VAR
			i, num: INTEGER;
			mapNames: POINTER TO ARRAY OF Dialog.String;
	BEGIN
		mapNames := MapsIndex.GetMaps();
		IF mapNames # NIL THEN
			num := LEN(mapNames, 0)
		ELSE
			num := 0
		END;
		i := 0;
		WHILE (i < num) & (name # mapNames[i]) DO
			INC(i)
		END;
		IF i < num THEN
			dialog.maps.index := i;
			Dialog.UpdateList(dialog.maps);
			Dialog.Update(dialog);
			dialog1.maps.index := i;
			Dialog.UpdateList(dialog1.maps);
			Dialog.Update(dialog1);
		END
	END SetMap;

	PROCEDURE SetPalette* (palette: INTEGER);
	BEGIN
		IF (palette < blues) OR (palette > rainbow) THEN RETURN END;
		dialog.paletteOptions.index := palette;
		PaletteNotifier(0, 0, 0);
		Dialog.UpdateList(dialog.paletteOptions);
		Dialog.Update(dialog)
	END SetPalette;

	PROCEDURE SetPercentileCut*;
	BEGIN
		dialog.cutType := MapsViews.percentileCut;
		Dialog.Update(dialog)
	END SetPercentileCut;

	PROCEDURE SetQuantile* (threshold: REAL);
	BEGIN
		dialog.fraction := threshold;
		Dialog.Update(dialog)
	END SetQuantile;

	PROCEDURE SetQuantity* (quantity: ARRAY OF CHAR);
	BEGIN
		dialog.quantity := quantity$;
		Dialog.Update(dialog)
	END SetQuantity;

	PROCEDURE SetType* (type: INTEGER);
	BEGIN
		dialog.plotType.index := type;
		Dialog.Update(dialog)
	END SetType;

	PROCEDURE SetThreshold* (threshold: REAL);
	BEGIN
		dialog.threshold := threshold;
		Dialog.Update(dialog)
	END SetThreshold;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			i, len: INTEGER;
	BEGIN
		Maintainer;
		bluePalette[0] := Ports.RGBColor(207, 255, 255); customPalette[0] := bluePalette[0];
		bluePalette[1] := Ports.RGBColor(161, 255, 255); customPalette[1] := bluePalette[1];
		bluePalette[2] := Ports.RGBColor(18, 255, 255); customPalette[2] := bluePalette[2];
		bluePalette[3] := Ports.RGBColor(18, 176, 255); customPalette[3] := bluePalette[3];
		bluePalette[4] := Ports.RGBColor(18, 82, 255); customPalette[4] := bluePalette[4];
		bluePalette[5] := Ports.RGBColor(0, 18, 180); customPalette[5] := bluePalette[5];
		bluePalette[6] := Ports.RGBColor(0, 18, 96); customPalette[6] := bluePalette[6];
		greyPalette[0] := Ports.RGBColor(215, 215, 215);
		greyPalette[1] := Ports.RGBColor(190, 190, 190);
		greyPalette[2] := Ports.RGBColor(165, 165, 165);
		greyPalette[3] := Ports.RGBColor(140, 140, 140);
		greyPalette[4] := Ports.RGBColor(115, 115, 115);
		greyPalette[5] := Ports.RGBColor(90, 90, 90);
		greyPalette[6] := Ports.RGBColor(65, 65, 65);
		rainbowPalette[0] := Ports.RGBColor(128, 128, 255);
		rainbowPalette[1] := Ports.RGBColor(0, 128, 255);
		rainbowPalette[2] := Ports.RGBColor(128, 255, 128);
		rainbowPalette[3] := Ports.RGBColor(255, 255, 0);
		rainbowPalette[4] := Ports.RGBColor(255, 128, 64);
		rainbowPalette[5] := Ports.RGBColor(128, 64, 64);
		rainbowPalette[6] := Ports.RGBColor(255, 0, 0);
		i := 0;
		len := LEN(bluePalette);
		WHILE i < len DO
			dialog.palette[i] := bluePalette[i];
			INC(i)
		END;
		dialog.maps.index := 0;
		UpdateNames;
		dialog.quantity := "";
		dialog.cutType := MapsViews.absCut;
		dialog.numCuts.SetLen(7);
		dialog.numCuts.SetResources("#Maps:numCuts");
		dialog.numCuts.index := default;
		dialog.paletteOptions.SetLen(3);
		dialog.paletteOptions.SetResources("#Maps:palette");
		dialog.paletteOptions.index := 0;
		dialog.plotType.SetResources("#Maps:plotType");
		dialog.plotType.index := 0;
		dialog.numberCuts := 0;
		dialog.beg := 1;
		dialog.end := 1000000;
		dialog.fraction := 50.0;
		dialog.threshold := 0.0;
		dialog1.id := 1;
	END Init;

BEGIN
	Init
END MapsCmds.
