(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE CompareCmds;


	

	IMPORT
		Dialog,
		TextModels,
		BugsCmds, BugsDialog, BugsEvaluate, BugsIndex, BugsInterface, 
		BugsMappers, BugsNames, BugsParser, BugsTexts,
		CompareViews,
		GraphNodes,
		PlotsViews,
		SamplesIndex, SamplesInterface, SamplesMonitors, SamplesStatistics,
		UpdaterActions;

	TYPE
		DialogBox* = POINTER TO RECORD (BugsDialog.DialogBox)
			node*, other*, axis*: Dialog.String;
			beg*, end*: INTEGER
		END;

	VAR
		dialog*: DialogBox;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE InitDialog;
	BEGIN
		dialog.node := "";
		dialog.other := "";
		dialog.axis := "";
		dialog.beg := 1;
		dialog.end := 1000000
	END InitDialog;

	PROCEDURE (dialog0: DialogBox) Init-;
	BEGIN
		InitDialog
	END Init;

	PROCEDURE (dialog: DialogBox) Update-;
	BEGIN
		Dialog.Update(dialog)
	END Update;

	PROCEDURE GetData (string: Dialog.String;
	beg, end, thin, firstChain, lastChain: INTEGER): POINTER TO ARRAY OF REAL;
		VAR
			i, index, len, numChains, sampleSize: INTEGER;
			offsets: POINTER TO ARRAY OF INTEGER;
			data: POINTER TO ARRAY OF REAL;
			var: BugsParser.Variable;
			name: BugsNames.Name;
			monitor: SamplesMonitors.Monitor;
			sample: POINTER TO ARRAY OF ARRAY OF REAL;
	BEGIN
		data := NIL;
		var := BugsParser.StringToVariable(string);
		IF var = NIL THEN RETURN NIL END;
		name := var.name;
		monitor := SamplesIndex.Find(name.string);
		offsets := BugsEvaluate.Offsets(var);
		IF offsets = NIL THEN RETURN NIL END;
		len := LEN(offsets);
		NEW(data, len);
		i := 0;
		WHILE i < len DO
			index := offsets[i];
			IF GraphNodes.data IN name.components[index].props THEN
				data[i] := name.components[index].Value()
			ELSIF (monitor # NIL) & monitor.IsMonitored(index) THEN
				sampleSize := monitor.SampleSize(index, beg, end, thin);
				IF sampleSize > 0 THEN
					numChains := lastChain - firstChain + 1;
					sampleSize := sampleSize DIV numChains;
					NEW(sample, numChains, sampleSize);
					monitor.Sample(i, beg, end, thin, firstChain, lastChain, sample);
					data[i] := SamplesStatistics.Mean(sample)
				ELSE
					RETURN NIL
				END
			ELSE
				RETURN NIL
			END;
			INC(i)
		END;
		RETURN data
	END GetData;

	PROCEDURE GetAxis (string: Dialog.String): POINTER TO ARRAY OF REAL;
		VAR
			i, index, len: INTEGER;
			offsets: POINTER TO ARRAY OF INTEGER;
			var: BugsParser.Variable;
			name: BugsNames.Name;
			axis: POINTER TO ARRAY OF REAL;
	BEGIN
		var := BugsParser.StringToVariable(string);
		IF var = NIL THEN RETURN NIL END;
		name := var.name;
		offsets := BugsEvaluate.Offsets(var);
		IF offsets = NIL THEN RETURN NIL END;
		len := LEN(offsets);
		NEW(axis, len);
		i := 0;
		WHILE i < len DO
			index := offsets[i];
			IF GraphNodes.data IN name.components[index].props THEN
				axis[i] := name.components[index].Value()
			ELSE
				RETURN NIL
			END;
			INC(i)
		END;
		RETURN axis
	END GetAxis;

	PROCEDURE Plot*;
		VAR
			beg, end, firstChain, lastChain, thin, w, h, i, j, len, numChains, numFract, size: INTEGER;
			offsets, start, sampleSize: POINTER TO ARRAY OF INTEGER;
			mean, fractions, other, axis, buffer: POINTER TO ARRAY OF REAL;
			sample, percentiles: POINTER TO ARRAY OF ARRAY OF REAL;
			string, title: Dialog.String;
			v: PlotsViews.View;
			t: TextModels.Model;
			f: BugsMappers.Formatter;
			fact: CompareViews.Factory;
			var: BugsParser.Variable;
			labels: POINTER TO ARRAY OF ARRAY 128 OF CHAR;
	BEGIN
		fact := CompareViews.fact;
		ASSERT(fact # NIL, 21); ASSERT(CompareViews.node IN fact.arguments, 21);
		string := dialog.node;
		var := BugsParser.StringToVariable(string);
		IF var = NIL THEN RETURN END;
		thin := 1;
		(*	only include values stored after end of adapting period	*)
		beg := MAX(dialog.beg, UpdaterActions.endOfAdapting) - 1;
		end := dialog.end;
		len := SamplesInterface.NumComponents(string, beg, end, thin);
		IF len = 0 THEN RETURN END;
		NEW(offsets, len);
		NEW(labels, len);
		NEW(mean, len);
		NEW(sampleSize, len);
		NEW(start, len);
		fractions := fact.fractions;
		IF fractions # NIL THEN numFract := LEN(fractions) ELSE numFract := 0 END;
		IF fractions # NIL THEN NEW(percentiles, len, numFract) ELSE percentiles := NIL END;
		firstChain := 1;
		lastChain := BugsCmds.specificationDialog.numChains;
		SamplesInterface.Offsets(string, beg, end, thin, offsets);
		i := 0;
		WHILE i < len DO
			BugsIndex.MakeLabel(string, offsets[i], labels[i]);
			size := SamplesInterface.SampleSize(labels[i], beg, dialog.end, thin, firstChain, lastChain);
			numChains := lastChain - firstChain + 1;
			size := size DIV numChains;
			NEW(sample, numChains, size);
			sampleSize[i] := sampleSize[i] * numChains;
			SamplesInterface.SampleValues(labels[i], beg, dialog.end, thin, firstChain, lastChain, sample);
			mean[i] := SamplesStatistics.Mean(sample);
			buffer := SamplesStatistics.Sort(sample);
			fact.StoreSample(buffer);
			j := 0;
			WHILE j < numFract DO
				percentiles[i, j] := SamplesStatistics.Percentile(buffer, fractions[j]);
				INC(j)
			END;
			sampleSize[i] := size;
			INC(i)
		END;
		IF (CompareViews.other IN fact.arguments) & (dialog.other # "") THEN
			other := GetData(dialog.other, beg, end, thin, firstChain, lastChain);
			IF (other = NIL) OR (LEN(other) # LEN(labels)) THEN RETURN END
		ELSE
			other := NIL
		END;
		IF (CompareViews.axis IN fact.arguments) & (dialog.axis # "") THEN
			axis := GetAxis(dialog.axis);
			IF (axis = NIL) OR (LEN(axis) # LEN(offsets)) THEN RETURN END
		ELSE
			axis := NIL
		END;
		v := fact.New(dialog.node, labels, mean, percentiles, start, sampleSize, other, axis);
		v.MinSize(w, h);
		t := TextModels.dir.New();
		BugsTexts.ConnectFormatter(f, t);
		f.WriteView(v, w, h);
		CompareViews.fact.Title(title);
		Dialog.MapString(title, title);
		f.Register(title)
	END Plot;

	PROCEDURE IsMonitored (IN string: ARRAY OF CHAR): BOOLEAN;
		VAR
			i: INTEGER;
			name: Dialog.String;
	BEGIN
		i := 0;
		WHILE (string[i] # 0X) & (string[i] # "[") DO
			name[i] := string[i];
			INC(i)
		END;
		name[i] := 0X;
		RETURN SamplesIndex.Find(name) # NIL
	END IsMonitored;

	PROCEDURE IsRegistered (IN string: ARRAY OF CHAR): BOOLEAN;
		VAR
			i: INTEGER;
			name: Dialog.String;
	BEGIN
		i := 0;
		WHILE (string[i] # 0X) & (string[i] # "[") DO
			name[i] := string[i];
			INC(i)
		END;
		name[i] := 0X;
		RETURN BugsIndex.Find(name) # NIL
	END IsRegistered;

	PROCEDURE GuardN* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~BugsInterface.IsInitialized() OR BugsInterface.IsAdapting();
		IF ~par.disabled THEN par.disabled := ~IsMonitored(dialog.node) END
	END GuardN;

	PROCEDURE GuardNA* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~BugsInterface.IsInitialized() OR BugsInterface.IsAdapting();
		IF ~par.disabled THEN par.disabled := ~IsMonitored(dialog.node) OR ~IsRegistered(dialog.axis) END
	END GuardNA;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "D.J.Lunn"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		NEW(dialog);
		InitDialog;
		BugsDialog.AddDialog(dialog)
	END Init;

BEGIN
	Init
END CompareCmds.
