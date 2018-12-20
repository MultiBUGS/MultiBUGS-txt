(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	   *)

MODULE RanksFormatted;

	

	IMPORT
		BugsFiles, BugsIndex, BugsNames,
		RanksIndex, RanksInterface, RanksMonitors,
		TextMappers, TextModels;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	CONST
		bold = 700;
		
	PROCEDURE WriteReal (x: REAL; VAR f: TextMappers.Formatter);
	BEGIN
		f.WriteRealForm(x, BugsFiles.prec, 0, 0, TextModels.digitspace)
	END WriteReal;
		
	PROCEDURE Header (IN fractions: ARRAY OF REAL; VAR f: TextMappers.Formatter);
		CONST
			eps = 1.0E-10;
		VAR
			i, numFrac: INTEGER;
			
			newAttr, oldAttr: TextModels.Attributes;
	BEGIN
		numFrac := LEN(fractions);
		oldAttr := f.rider.attr;
		newAttr := TextModels.NewWeight(oldAttr, bold);
		f.rider.SetAttr(newAttr);
		f.WriteTab;
		f.WriteTab;
		i := 0;
		WHILE i < numFrac DO
			IF ABS(fractions[i] - 50) < eps THEN
				f.WriteString("median")
			ELSE
				f.WriteString("val");
				WriteReal(fractions[i], f);
				f.WriteString("pc")
			END;
			f.WriteTab;
			INC(i)
		END;
		f.WriteTab;
		f.WriteLn;
		f.rider.SetAttr(oldAttr)
	END Header;

	PROCEDURE FormatStats (IN variable: ARRAY OF CHAR; IN fractions: ARRAY OF REAL;
	VAR f: TextMappers.Formatter);
		VAR
			i, j, len, numFrac, sampleSize: INTEGER;
			label: ARRAY 120 OF CHAR;
			percentiles: POINTER TO ARRAY OF ARRAY OF INTEGER;
			node: BugsNames.Name;
	BEGIN
		sampleSize := RanksInterface.SampleSize(variable);
		IF sampleSize = 0 THEN
			RETURN
		END;
		node := BugsIndex.Find(variable);
		RanksInterface.Stats(variable, fractions, percentiles);
		len := LEN(percentiles, 0);
		numFrac := LEN(fractions);
		i := 0;
		Header(fractions, f);
		WHILE i < len DO
			f.WriteTab;
			f.WriteString(variable);
			node.Indices(i, label);
			f.WriteString(label);
			f.WriteTab;
			j := 0;
			WHILE j < numFrac DO
				f.WriteInt(percentiles[i, j]);
				f.WriteTab;
				INC(j)
			END;
			f.WriteLn;
			INC(i)
		END
	END FormatStats;

	PROCEDURE Stats* (variable: ARRAY OF CHAR; fractions: ARRAY OF REAL;
	VAR f: TextMappers.Formatter);
		VAR
			i, len: INTEGER;
			monitors: POINTER TO ARRAY OF RanksMonitors.Monitor;
	BEGIN
		IF RanksInterface.IsStar(variable) THEN
			monitors := RanksIndex.GetMonitors();
			IF monitors # NIL THEN
				len := LEN(monitors)
			ELSE
				len := 0
			END;
			i := 0;
			WHILE i < len DO
				FormatStats(monitors[i].Name().string, fractions, f);
				INC(i)
			END
		ELSE
			FormatStats(variable, fractions, f)
		END
	END Stats;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END RanksFormatted.
