(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	   *)

MODULE SamplesPlots;

	

	IMPORT
		Dialog, Views,
		BugsMappers, BugsNames, BugsParser,
		PlotsViews,
		SamplesIndex, SamplesInterface, SamplesMonitors, SamplesViews,
		TextMappers;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		PROCEDURE Plot (IN name: ARRAY OF CHAR;
	offset, beg, end, step, firstChain, lastChain, numChains: INTEGER): Views.View;
		VAR
			lenChain, noChains, start: INTEGER;
			string: ARRAY 80 OF CHAR;
			node: BugsNames.Name;
			plot: Views.View;
			sample: POINTER TO ARRAY OF ARRAY OF REAL;
			monitor: SamplesMonitors.Monitor;
	BEGIN
		noChains := lastChain - firstChain + 1;
		monitor := SamplesIndex.Find(name);
		ASSERT(monitor.IsMonitored(offset), 21);
		lenChain := monitor.SampleSize(offset, beg, end, step);
		start := monitor.Start(offset);
		start := MAX(start, beg);
		node := monitor.Name();
		node.Indices(offset, string);
		string := name + string;
		IF lenChain > 0 THEN
			NEW(sample, noChains, lenChain);
			monitor.Sample(offset, beg, end, step, firstChain, lastChain, sample);
			plot := SamplesViews.fact.New(string, sample, start, step, firstChain, numChains);
			RETURN plot
		ELSE
			RETURN NIL
		END
	END Plot;

	PROCEDURE MakePlots (IN name: ARRAY OF CHAR;
	beg, end, thin, firstChain, lastChain, numChains: INTEGER):
	POINTER TO ARRAY OF Views.View;
		VAR
			i, num: INTEGER;
			offsets: POINTER TO ARRAY OF INTEGER;
			monitor: SamplesMonitors.Monitor;
			var: BugsParser.Variable;
			plots: POINTER TO ARRAY OF Views.View;
	BEGIN
		var := BugsParser.StringToVariable(name);
		thin := MAX(thin, 1);
		num := SamplesInterface.NumComponents(name, beg, end, thin);
		IF num = 0 THEN RETURN NIL END;
		NEW(offsets, num);
		SamplesInterface.Offsets(name, beg, end, thin, offsets);
		NEW(plots, num);
		i := 0;
		WHILE i < num DO
			plots[i] := Plot(var.name.string, offsets[i], beg, end, thin, firstChain, lastChain, numChains);
			INC(i)
		END;
		RETURN plots
	END MakePlots;

	PROCEDURE FormatPlots (IN string: Dialog.String;
	beg, end, thin, firstChain, lastChain, numChains: INTEGER;
	VAR f: TextMappers.Formatter);
		VAR
			h, i, num, w: INTEGER;
			plots: POINTER TO ARRAY OF Views.View;
	BEGIN
		plots := MakePlots(string, beg, end, thin, firstChain, lastChain, numChains);
		IF plots = NIL THEN
			num := 0
		ELSE
			num := LEN(plots)
		END;
		i := 0;
		WHILE i < num DO
			IF plots[i] # NIL THEN
				plots[i](PlotsViews.View).MinSize(w, h);
				f.WriteView(plots[i])
			END;
			INC(i)
		END
	END FormatPlots;

	PROCEDURE Draw* (IN name: Dialog.String; beg, end, thin, firstChain, lastChain, numChains: INTEGER;
	VAR f: TextMappers.Formatter);
		VAR
			i, len: INTEGER;
			string1: Dialog.String;
			monitors: POINTER TO ARRAY OF SamplesMonitors.Monitor;
	BEGIN
		IF SamplesInterface.IsStar(name) THEN
			monitors := SamplesIndex.GetMonitors();
			i := 0;
			IF monitors # NIL THEN
				len := LEN(monitors)
			ELSE
				len := 0
			END;
			WHILE i < len DO
				string1 := monitors[i].Name().string$;
				FormatPlots(string1, beg, end, thin, firstChain, lastChain, numChains, f);
				INC(i)
			END
		ELSE
			FormatPlots(name, beg, end, thin, firstChain, lastChain, numChains, f)
		END
	END Draw;

	PROCEDURE Title* (OUT title: ARRAY OF CHAR);
	BEGIN
		SamplesViews.fact.Title(title);
		Dialog.MapString(title, title)
	END Title;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END SamplesPlots.
