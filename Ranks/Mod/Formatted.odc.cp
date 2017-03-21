(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	   *)

MODULE RanksFormatted;

	

	IMPORT
		BugsIndex, BugsMappers, BugsNames,
		RanksIndex, RanksInterface, RanksMonitors;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Header (IN fractions: ARRAY OF REAL; VAR f: BugsMappers.Formatter);
		CONST
			eps = 1.0E-10;
		VAR
			i, numFrac: INTEGER;
	BEGIN
		numFrac := LEN(fractions);
		f.Bold;
		f.WriteTab;
		f.WriteTab;
		i := 0;
		WHILE i < numFrac DO
			IF ABS(fractions[i] - 50) < eps THEN
				f.WriteString("median")
			ELSE
				f.WriteString("val");
				f.WriteReal(fractions[i]);
				f.WriteString("pc")
			END;
			f.WriteTab;
			INC(i)
		END;
		f.WriteTab;
		f.WriteLn;
		f.Bold
	END Header;

	PROCEDURE FormatStats (IN variable: ARRAY OF CHAR; IN fractions: ARRAY OF REAL;
	VAR f: BugsMappers.Formatter);
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
	VAR f: BugsMappers.Formatter);
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
