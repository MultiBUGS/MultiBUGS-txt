(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE SamplesFormatted;

	

	IMPORT
		BugsIndex, BugsMappers, BugsNames, BugsParser,
		SamplesIndex, SamplesInterface, SamplesMonitors;

	CONST
		essOpt* = 0;
		medianOpt* = 1; modeOpt* = 2; 
		skewOpt* = 3; exKurtOpt* = 4;
		quant0Opt* = 5; quant1Opt* = 6; quant2Opt* = 7; quant3Opt* = 8;
		quant4Opt* = 9; quant5Opt* = 10; quant6Opt* = 11; quant7Opt* = 12;
		hpd0Opt* = 13; hpd1Opt* = 14; 

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 20 OF CHAR;

	PROCEDURE Header (IN fract: ARRAY OF REAL; options: SET; VAR f: BugsMappers.Formatter);
		CONST
			eps = 1.0E-10;
		VAR
			i, numFrac, oldPrec: INTEGER;
	BEGIN
		oldPrec := BugsMappers.prec;
		BugsMappers.SetPrec(4);
		f.Bold;
		f.WriteTab;
		f.WriteTab;
		f.WriteString("mean"); f.WriteTab;
		IF medianOpt IN options THEN f.WriteString("median"); f.WriteTab END;
		IF modeOpt IN options THEN f.WriteString("mode"); f.WriteTab END;
		f.WriteString("sd"); f.WriteTab;
		IF skewOpt IN options THEN f.WriteString("skew"); f.WriteTab END;
		IF exKurtOpt IN options THEN f.WriteString("ex.kur"); f.WriteTab END;
		f.WriteString("MC_error"); f.WriteTab;
		i := 0;
		numFrac := LEN(fract);
		WHILE i < numFrac - 2 DO
			IF i + quant0Opt IN options THEN
				f.WriteString("val");
				f.WriteReal(fract[i]);
				f.WriteString("pc");
				f.WriteTab
			END;
			INC(i)
		END;
		WHILE i < numFrac DO
			IF i + quant0Opt IN options THEN
				f.WriteString("HPDL");
				f.WriteReal(fract[i]);
				f.WriteTab;
				f.WriteString("HPDU");
				f.WriteReal(fract[i]);
				f.WriteTab
			END;
			INC(i)
		END;
		f.WriteString("start");
		f.WriteTab;
		f.WriteString("sample");
		IF essOpt IN options THEN f.WriteTab; f.WriteString("ESS") END;
		f.WriteLn;
		f.Bold;
		BugsMappers.SetPrec(oldPrec)
	END Header;

	PROCEDURE FormatStats (variable: ARRAY OF CHAR;
	beg, end, thin, firstChain, lastChain: INTEGER;
	options: SET; IN fractions: ARRAY OF REAL;
	VAR writeHeader: BOOLEAN; VAR f: BugsMappers.Formatter);
		VAR
			ess, i, j, num, sampleSize, start: INTEGER;
			offsets: POINTER TO ARRAY OF INTEGER;
			error, mean, median, mode, sd, skew, exKur: REAL;
			scalar: ARRAY 120 OF CHAR;
			percentiles: ARRAY 8 OF REAL;
			lower, upper: ARRAY 2 OF REAL;
			var: BugsParser.Variable;
			name: BugsNames.Name;
			sort: BOOLEAN;
	BEGIN
		num := SamplesInterface.NumComponents(variable, beg, end, thin);
		IF num = 0 THEN RETURN END;
		NEW(offsets, num);
		SamplesInterface.Offsets(variable, beg, end, thin, offsets);
		IF writeHeader THEN
			Header(fractions, options, f);
			writeHeader := FALSE
		END;
		var := BugsParser.StringToVariable(variable);
		name := var.name;
		i := 0;
		WHILE i < num DO
			name.Indices(offsets[i], scalar);
			scalar := name.string + scalar;
			SamplesInterface.Statistics(scalar, beg, end, thin, firstChain, lastChain, fractions,
			mean, median, mode, sd, skew, exKur, error, percentiles, lower, upper, start, sampleSize,ess);
			f.WriteTab;
			f.WriteString(scalar); f.WriteTab;
			f.WriteReal(mean); f.WriteTab;
			IF medianOpt IN options THEN f.WriteReal(median); f.WriteTab END;
			IF modeOpt IN options THEN f.WriteReal(mode); f.WriteTab END;
			f.WriteReal(sd); f.WriteTab;
			IF skewOpt IN options THEN f.WriteReal(skew); f.WriteTab END;
			IF exKurtOpt IN options THEN f.WriteReal(exKur); f.WriteTab END;
			IF error > 0 THEN
				f.WriteReal(error)
			ELSE
				f.WriteString("---")
			END;
			f.WriteTab;
			j := 0;
			WHILE j < LEN(percentiles) DO
				IF j + quant0Opt IN options THEN
					f.WriteReal(percentiles[j]); f.WriteTab
				END;
				INC(j)
			END;
			j := 0;
			WHILE j < LEN(lower) DO
				IF j + hpd0Opt IN options THEN
					f.WriteReal(lower[j]); f.WriteTab;
					f.WriteReal(upper[j]); f.WriteTab
				END;
				INC(j)
			END;
			f.WriteInt(start);
			f.WriteTab;
			f.WriteInt(sampleSize);
			IF essOpt IN options THEN f.WriteTab; f.WriteInt(ess) END;
			f.WriteLn;
			INC(i)
		END
	END FormatStats;

	PROCEDURE StatsSummary* (variable: ARRAY OF CHAR; beg, end, thin, firstChain, lastChain: INTEGER;
	options: SET; IN fractions: ARRAY OF REAL; VAR f: BugsMappers.Formatter);
		VAR
			writeHeader: BOOLEAN;
			i, len: INTEGER;
			monitors: POINTER TO ARRAY OF SamplesMonitors.Monitor;
	BEGIN
		writeHeader := TRUE;
		IF SamplesInterface.IsStar(variable) THEN
			monitors := SamplesIndex.GetMonitors();
			IF monitors = NIL THEN RETURN END;
			i := 0;
			len := LEN(monitors);
			WHILE i < len DO
				variable := monitors[i].Name().string$;
				FormatStats(variable, beg, end, thin, firstChain, lastChain, options, fractions, writeHeader, f);
				INC(i)
			END
		ELSE
			FormatStats(variable, beg, end, thin, firstChain, lastChain, options, fractions, writeHeader, f)
		END
	END StatsSummary;

	PROCEDURE FormatCODA (IN variable: ARRAY OF CHAR; beg, end, thin, firstChain, lastChain: INTEGER;
	VAR f: ARRAY OF BugsMappers.Formatter);
		VAR
			chain, i, iteration, j, k, num, line,
			numChains, sampleSize: INTEGER;
			sample: POINTER TO ARRAY OF ARRAY OF REAL;
			label: ARRAY 128 OF CHAR;
			offsets: POINTER TO ARRAY OF INTEGER;
	BEGIN
		num := SamplesInterface.NumComponents(variable, beg, end, thin);
		IF num = 0 THEN RETURN END;
		NEW(offsets, num);
		SamplesInterface.Offsets(variable, beg, end, thin, offsets);
		numChains := lastChain - firstChain + 1;
		i := 0;
		line := f[1].lines;
		WHILE i < num DO
			BugsIndex.MakeLabel(variable, offsets[i], label);
			sampleSize := SamplesInterface.SampleSize(label, beg, end, thin, firstChain, lastChain);
			sampleSize := sampleSize DIV numChains;
			f[0].WriteString(label);
			f[0].WriteTab;
			f[0].WriteInt(line + 1);
			f[0].WriteTab;
			INC(line, sampleSize);
			f[0].WriteInt(line);
			f[0].WriteLn;
			NEW(sample, 1, sampleSize);
			j := 0;
			WHILE j < numChains DO
				iteration := SamplesInterface.SampleStart(label, beg, end, thin) + 1;
				chain := firstChain + j;
				SamplesInterface.SampleValues(label, beg, end, thin, chain, chain, sample);
				k := 0;
				WHILE k < sampleSize DO
					f[j + 1].WriteInt(iteration);
					f[j + 1].WriteTab;
					f[j + 1].WriteReal(sample[0, k]);
					f[j + 1].WriteLn;
					INC(iteration, thin);
					INC(k)
				END;
				INC(j)
			END;
			INC(i)
		END
	END FormatCODA;

	PROCEDURE CODA* (variable: ARRAY OF CHAR; beg, end, thin, firstChain, lastChain: INTEGER;
	VAR f: ARRAY OF BugsMappers.Formatter);
		VAR
			i, len: INTEGER;
			monitors: POINTER TO ARRAY OF SamplesMonitors.Monitor;
	BEGIN
		IF SamplesInterface.IsStar(variable) THEN
			monitors := SamplesIndex.GetMonitors();
			IF monitors = NIL THEN RETURN END;
			i := 0;
			len := LEN(monitors);
			WHILE i < len DO
				variable := monitors[i].Name().string$;
				FormatCODA(variable, beg, end, thin, firstChain, lastChain, f);
				INC(i)
			END
		ELSE
			FormatCODA(variable, beg, end, thin, firstChain, lastChain, f)
		END
	END CODA;

	PROCEDURE FormatLabels (variable: ARRAY OF CHAR; beg, end, thin: INTEGER;
	VAR f: BugsMappers.Formatter);
		VAR
			i, len, num: INTEGER;
			offsets: POINTER TO ARRAY OF INTEGER;
			label: ARRAY 128 OF CHAR;
	BEGIN
		num := SamplesInterface.NumComponents(variable, beg, end, thin);
		IF num = 0 THEN RETURN END;
		NEW(offsets, num);
		SamplesInterface.Offsets(variable, beg, end, thin, offsets);
		IF offsets # NIL THEN len := LEN(offsets) ELSE len := 0 END;
		i := 0;
		WHILE i < len DO
			BugsIndex.MakeLabel(variable, offsets[i], label);
			f.WriteString(label);
			f.WriteLn;
			INC(i)
		END
	END FormatLabels;

	PROCEDURE Labels* (variable: ARRAY OF CHAR; beg, end, thin: INTEGER; VAR f: BugsMappers.Formatter);
		VAR
			i, len: INTEGER;
			monitors: POINTER TO ARRAY OF SamplesMonitors.Monitor;
	BEGIN
		IF SamplesInterface.IsStar(variable) THEN
			monitors := SamplesIndex.GetMonitors();
			IF monitors = NIL THEN RETURN END;
			i := 0;
			len := LEN(monitors);
			WHILE i < len DO
				variable := monitors[i].Name().string$;
				FormatLabels(variable, beg, end, thin, f);
				INC(i)
			END
		ELSE
			FormatLabels(variable, beg, end, thin, f)
		END
	END Labels;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END SamplesFormatted.
