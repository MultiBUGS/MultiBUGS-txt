(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE DevianceFormatted;


	

	IMPORT
		Fonts,
		BugsFiles, BugsInterface, 
		DevianceIndex, DevianceInterface, DevianceMonitors,
		TextMappers, TextModels;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		
	PROCEDURE WriteReal (x: REAL; VAR f: TextMappers.Formatter);
	BEGIN
		f.WriteRealForm(x, BugsFiles.prec, 0, 0, TextModels.digitspace)
	END WriteReal;
	
	PROCEDURE Header (VAR f: TextMappers.Formatter);
		VAR
			plugin: BOOLEAN;
			newAttr, oldAttr: TextModels.Attributes;
	BEGIN
		plugin := DevianceInterface.PluginSet();
		oldAttr := f.rider.attr;
		newAttr := TextModels.NewWeight(oldAttr, Fonts.bold);
		f.rider.SetAttr(newAttr);
		f.WriteTab;
		f.WriteString("Dbar"); f.WriteTab;
		IF plugin THEN
			f.WriteString("Dhat"); f.WriteTab;
			f.WriteString("DIC"); f.WriteTab
		END;
		f.WriteString("WAIC"); f.WriteTab;
		IF plugin THEN f.WriteString("pD"); f.WriteTab END;
		f.WriteString("pW"); f.WriteTab;
		f.WriteLn;
		f.rider.SetAttr(oldAttr)
	END Header;

	PROCEDURE FormatStats (IN name: ARRAY OF CHAR; dBar, dHat, dic, waic, pD, pW: REAL;
	VAR f: TextMappers.Formatter);
		VAR
			plugin: BOOLEAN;
	BEGIN
		plugin := DevianceInterface.PluginSet();
		f.WriteString(name); f.WriteTab;
		WriteReal(dBar, f); f.WriteTab;
		IF plugin THEN
			WriteReal(dHat, f); f.WriteTab;
			WriteReal(dic, f); f.WriteTab
		END;
		WriteReal(waic, f); f.WriteTab;
		IF plugin THEN WriteReal(pD, f); f.WriteTab END;
		WriteReal(pW, f); f.WriteTab;
		f.WriteLn
	END FormatStats;

	PROCEDURE Stats* (VAR f: TextMappers.Formatter);
		VAR
			i, len: INTEGER;
			dBar, dHat, pD, pW, dic, waic, dBarTot, dHatTot, dicTot, waicTot, pDTot, pWTot: REAL;
			monitors: POINTER TO ARRAY OF DevianceMonitors.Monitor;
	BEGIN
		Header(f);
		monitors := DevianceIndex.GetMonitors();
		IF monitors # NIL THEN
			len := LEN(monitors)
		ELSE
			len := 0
		END;
		i := 0;
		dBarTot := 0.0;
		dHatTot := 0.0;
		dicTot := 0.0;
		waicTot := 0.0;
		pDTot := 0.0;
		pWTot := 0.0;
		WHILE i < len DO
			DevianceInterface.NodeStats(monitors[i].Name().string, dBar, dHat, dic, waic, pD, pW);
			FormatStats(monitors[i].Name().string, dBar, dHat, dic, waic, pD, pW, f);
			dBarTot := dBarTot + dBar;
			dHatTot := dHatTot + dHat;
			dicTot := dicTot + dic;
			waicTot := waicTot + waic;
			pDTot := pDTot + pD;
			pWTot := pWTot + pW;
			INC(i)
		END;
		FormatStats("total", dBarTot, dHatTot, dicTot, waicTot, pDTot, pWTot, f)
	END Stats;

	PROCEDURE DistributedWAIC* (VAR f: TextMappers.Formatter);
		VAR
			i, numChains: INTEGER;
			lpd, pW, waic, dic, pD, meanDeviance, meanDeviance2 : REAL;
			newAttr, oldAttr: TextModels.Attributes;
	BEGIN
		ASSERT(BugsInterface.IsDistributed(), 21);
		oldAttr := f.rider.attr;
		newAttr := TextModels.NewWeight(oldAttr, Fonts.bold);
		f.rider.SetAttr(newAttr);
		f.WriteTab;
		f.WriteString("Chain"); f.WriteTab;
		f.WriteString("Dbar"); f.WriteTab;
		f.WriteString("DIC"); f.WriteTab;
		f.WriteString("WAIC"); f.WriteTab;
		f.WriteString("pD"); f.WriteTab;
		f.WriteString("pW"); f.WriteTab;
		f.WriteLn;
		f.rider.SetAttr(oldAttr);
		numChains := BugsInterface.NumberChains();
		i := 0;
		WHILE i < numChains DO
			DevianceInterface.GetStatistics(lpd, pW, meanDeviance, meanDeviance2, i);
			pD := 0.25 * (meanDeviance2 - meanDeviance * meanDeviance);
			dic := meanDeviance + pD;
			waic := -2 * lpd + 2 * pW;
			f.WriteTab; f.WriteInt(i + 1);
			f.WriteTab; WriteReal(meanDeviance, f); 
			f.WriteTab; WriteReal(dic, f); 
			f.WriteTab; WriteReal(waic, f); 
			f.WriteTab; WriteReal(pD, f);
			f.WriteTab; WriteReal(pW, f);
			f.WriteTab;
			f.WriteLn;
			INC(i)
		END
	END DistributedWAIC;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END DevianceFormatted.
