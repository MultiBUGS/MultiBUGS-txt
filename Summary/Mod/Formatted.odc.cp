(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE SummaryFormatted;


	

	IMPORT
		BugsIndex, BugsMappers, BugsNames,
		SummaryIndex, SummaryInterface, SummaryMonitors;
	CONST
		skewOpt* = 0; exKurtOpt* = 1; medianOpt* = 2; quant0Opt* = 3; quant1Opt* = 4;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE FormatMeans (IN name: ARRAY OF CHAR; VAR f: BugsMappers.Formatter);
		VAR
			i, num: INTEGER;
			offsets: POINTER TO ARRAY OF INTEGER;
			lower, mean, median, sd, skew, exKur, upper: REAL;
			label: ARRAY 128 OF CHAR;
	BEGIN
		num := SummaryInterface.NumComponents(name);
		IF num = 0 THEN RETURN END;
		NEW(offsets, num);
		SummaryInterface.Offsets(name, offsets);
		f.WriteString("mean.");
		f.WriteString(name);
		f.WriteString(" = c(");
		f.WriteLn;
		i := 0;
		WHILE i < num - 1 DO
			BugsIndex.MakeLabel(name, offsets[i], label);
			SummaryInterface.Stats(label, mean, sd, skew, exKur, lower, median, upper);
			f.WriteReal(mean);
			f.WriteString(", ");
			INC(i);
			IF i MOD 5 = 0 THEN f.WriteLn END
		END;
		BugsIndex.MakeLabel(name, offsets[i], label);
		SummaryInterface.Stats(label, mean, sd, skew, exKur, lower, median, upper);
		f.WriteReal(mean);
		f.WriteChar(")")
	END FormatMeans;

	PROCEDURE Means* (variable: ARRAY OF CHAR; VAR f: BugsMappers.Formatter);
		VAR
			i, len: INTEGER;
			monitors: POINTER TO ARRAY OF SummaryMonitors.Monitor;
	BEGIN
		IF SummaryInterface.IsStar(variable) THEN
			monitors := SummaryIndex.GetMonitors();
			IF monitors # NIL THEN
				len := LEN(monitors)
			ELSE
				len := 0
			END;
			f.WriteString("list(");
			f.WriteLn;
			i := 0;
			WHILE i < len DO
				FormatMeans(monitors[i].Name().string, f);
				INC(i);
				IF i < len THEN
					f.WriteChar(",");
					f.WriteLn
				END
			END;
			f.WriteChar(")")
		ELSE
			FormatMeans(variable, f)
		END;
		f.WriteLn
	END Means;

	PROCEDURE Header (options: SET; VAR f: BugsMappers.Formatter);
	BEGIN
		f.Bold;
		f.WriteTab;
		f.WriteTab;
		f.WriteString("mean"); f.WriteTab;
		IF medianOpt IN options THEN f.WriteString("median"); f.WriteTab END;
		f.WriteString("sd"); f.WriteTab;
		IF skewOpt IN options THEN f.WriteString("skew"); f.WriteTab END;
		IF exKurtOpt IN options THEN f.WriteString("ex.kur"); f.WriteTab END;
		IF quant0Opt IN options THEN f.WriteString("val2.5pc"); f.WriteTab END;
		IF quant1Opt IN options THEN f.WriteString("val97.5pc"); f.WriteTab END;
		f.WriteString("sample"); f.WriteLn;
		f.Bold
	END Header;

	PROCEDURE FormatStats (IN name: ARRAY OF CHAR; options: SET; VAR writeHeader: BOOLEAN;
	VAR f: BugsMappers.Formatter);
		VAR
			i, num, sampleSize: INTEGER;
			indices: ARRAY 128 OF CHAR;
			offsets: POINTER TO ARRAY OF INTEGER;
			lower, mean, skew, exKur, median, sd, upper: REAL;
			node: BugsNames.Name;
			label: ARRAY 128 OF CHAR;
	BEGIN
		num := SummaryInterface.NumComponents(name);
		IF num = 0 THEN RETURN END;
		NEW(offsets, num);
		SummaryInterface.Offsets(name, offsets);
		sampleSize := SummaryInterface.SampleSize(name);
		i := 0;
		IF writeHeader THEN
			Header(options, f); writeHeader := FALSE
		END;
		node := BugsIndex.Find(name);
		WHILE i < num DO
			BugsIndex.MakeLabel(name, offsets[i], label);
			SummaryInterface.Stats(label, mean, sd, skew, exKur, lower, median, upper);
			f.WriteTab;
			f.WriteString(name);
			node.Indices(offsets[i], indices);
			f.WriteString(indices);
			f.WriteTab;
			f.WriteReal(mean); f.WriteTab;
			IF medianOpt IN options THEN f.WriteReal(median); f.WriteTab END;
			f.WriteReal(sd); f.WriteTab;
			IF skewOpt IN options THEN f.WriteReal(skew); f.WriteTab END;
			IF exKurtOpt IN options THEN f.WriteReal(exKur); f.WriteTab END;
			IF quant0Opt IN options THEN f.WriteReal(lower); f.WriteTab END;
			IF quant1Opt IN options THEN f.WriteReal(upper); f.WriteTab END;
			f.WriteInt(sampleSize);
			f.WriteLn;
			INC(i)
		END
	END FormatStats;

	PROCEDURE Stats* (variable: ARRAY OF CHAR; options: SET; VAR f: BugsMappers.Formatter);
		VAR
			i, len: INTEGER;
			writeHeader: BOOLEAN;
			monitors: POINTER TO ARRAY OF SummaryMonitors.Monitor;
	BEGIN
		writeHeader := TRUE;
		IF SummaryInterface.IsStar(variable) THEN
			monitors := SummaryIndex.GetMonitors();
			IF monitors # NIL THEN
				len := LEN(monitors)
			ELSE
				len := 0
			END;
			i := 0;
			WHILE i < len DO
				FormatStats(monitors[i].Name().string, options, writeHeader, f);
				INC(i)
			END
		ELSE
			FormatStats(variable, options, writeHeader, f)
		END
	END Stats;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END SummaryFormatted.
