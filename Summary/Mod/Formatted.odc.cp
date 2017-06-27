(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE SummaryFormatted;


	

	IMPORT
		Fonts,
		BugsFiles, BugsIndex, BugsNames,
		SummaryIndex, SummaryInterface, SummaryMonitors,
		TextMappers, TextModels;
		
	CONST
		skewOpt* = 0; exKurtOpt* = 1; medianOpt* = 2; quant0Opt* = 3; quant1Opt* = 4;
		bold = 770;
		
	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		
	PROCEDURE WriteReal (x: REAL; VAR f: TextMappers.Formatter);
	BEGIN
		f.WriteRealForm(x, BugsFiles.prec, 0, 0, TextModels.digitspace)
	END WriteReal;

	PROCEDURE FormatMeans (IN name: ARRAY OF CHAR; VAR f: TextMappers.Formatter);
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
			WriteReal(mean, f);
			f.WriteString(", ");
			INC(i);
			IF i MOD 5 = 0 THEN f.WriteLn END
		END;
		BugsIndex.MakeLabel(name, offsets[i], label);
		SummaryInterface.Stats(label, mean, sd, skew, exKur, lower, median, upper);
		WriteReal(mean, f);
		f.WriteChar(")")
	END FormatMeans;

	PROCEDURE Means* (variable: ARRAY OF CHAR; VAR f: TextMappers.Formatter);
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

	PROCEDURE Header (options: SET; VAR f: TextMappers.Formatter);
		VAR
			newAttr, oldAttr: TextModels.Attributes;
	BEGIN
		oldAttr := f.rider.attr;
		newAttr := TextModels.NewWeight(oldAttr, bold);
		f.rider.SetAttr(newAttr);
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
		f.rider.SetAttr(oldAttr)
	END Header;

	PROCEDURE FormatStats (IN name: ARRAY OF CHAR; options: SET; VAR writeHeader: BOOLEAN;
	VAR f: TextMappers.Formatter);
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
			WriteReal(mean, f); f.WriteTab;
			IF medianOpt IN options THEN WriteReal(median, f); f.WriteTab END;
			WriteReal(sd, f); f.WriteTab;
			IF skewOpt IN options THEN WriteReal(skew, f); f.WriteTab END;
			IF exKurtOpt IN options THEN WriteReal(exKur, f); f.WriteTab END;
			IF quant0Opt IN options THEN WriteReal(lower, f); f.WriteTab END;
			IF quant1Opt IN options THEN WriteReal(upper, f); f.WriteTab END;
			f.WriteInt(sampleSize);
			f.WriteLn;
			INC(i)
		END
	END FormatStats;

	PROCEDURE Stats* (variable: ARRAY OF CHAR; options: SET; VAR f: TextMappers.Formatter);
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
