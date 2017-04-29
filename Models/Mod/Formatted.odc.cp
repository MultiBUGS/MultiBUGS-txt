(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE ModelsFormatted;

	

	IMPORT
		Fonts, 
		MathSort,
		BugsFiles,
		ModelsIndex, ModelsInterface, ModelsMonitors, MonitorModel,
		TextMappers, TextModels;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE WriteReal (x: REAL; VAR f: TextMappers.Formatter);
	BEGIN
		f.WriteRealForm(x, BugsFiles.prec, 0, 0, TextModels.digitspace)
	END WriteReal;
		
	PROCEDURE FormatComponentProbs (IN variable: ARRAY OF CHAR; VAR f: TextMappers.Formatter);
		VAR
			i, len, sampleSize: INTEGER;
			compProbs: POINTER TO ARRAY OF REAL;
			newAttr, oldAttr: TextModels.Attributes;
	BEGIN
		sampleSize := ModelsInterface.SampleSize(variable);
		IF sampleSize = 0 THEN RETURN END;
		oldAttr := f.rider.attr;
		newAttr := TextModels.NewWeight(oldAttr,Fonts. bold);
		f.rider.SetAttr(newAttr);
		f.WriteTab; f.WriteString("variable: "); f.WriteString(variable); 
		f.WriteTab; f.WriteString("sample size: "); f.WriteInt(sampleSize);f.WriteTab;  f.WriteLn;
		f.WriteTab; f.WriteString("component");
		f.WriteTab; f.WriteString("pobability"); f.WriteLn;
		f.rider.SetAttr(oldAttr);
		ModelsInterface.ComponentProbs(variable, compProbs);
		len := LEN(compProbs);
		i := 0;
		WHILE i < len DO
			f.WriteTab;
			f.WriteInt(i + 1);
			f.WriteTab;
			WriteReal(compProbs[i], f);
			f.WriteTab;
			f.WriteLn;
			INC(i)
		END
	END FormatComponentProbs;

	PROCEDURE FormatModelProbs (IN variable: ARRAY OF CHAR; cumulative, minProb: REAL;
	maxNum: INTEGER; VAR f: TextMappers.Formatter);
		VAR
			i, j, len, numberComponents, sampleSize: INTEGER;
			sumProbs: REAL;
			first: BOOLEAN;
			modelProbs: POINTER TO ARRAY OF REAL;
			models: POINTER TO ARRAY OF MonitorModel.Model;
			model: POINTER TO ARRAY OF BOOLEAN;
			ranks: POINTER TO ARRAY OF INTEGER;
			newAttr, oldAttr: TextModels.Attributes;
	BEGIN
		sampleSize := ModelsInterface.SampleSize(variable);
		IF sampleSize = 0 THEN RETURN END;
		oldAttr := f.rider.attr;
		newAttr := TextModels.NewWeight(oldAttr, Fonts.bold);
		f.rider.SetAttr(newAttr);
		f.WriteTab; f.WriteString("variable: "); f.WriteString(variable); 
		f.WriteTab; f.WriteString("sample size: "); f.WriteInt(sampleSize); f.WriteLn;
		f.WriteTab; f.WriteString("rank");
		f.WriteTab; f.WriteString("probability");
		f.WriteTab; f.WriteString("cumulative");
		f.WriteTab; f.WriteString("structure");
		f.WriteTab; f.WriteLn;
		f.rider.SetAttr(oldAttr);
		ModelsInterface.ModelProbs(variable, models, modelProbs);
		len := LEN(modelProbs);
		i := 0; WHILE i < len DO modelProbs[i] := -modelProbs[i]; INC(i) END;
		NEW(ranks, len);
		MathSort.Rank(modelProbs, len, ranks);
		i := 0; WHILE i < len DO modelProbs[i] := -modelProbs[i]; INC(i) END;
		i := 0;
		sumProbs := 0.0;
		numberComponents := ModelsInterface.NumberComponents(variable);
		NEW(model, numberComponents);
		WHILE (i < len) & (i < maxNum) & (sumProbs < cumulative) & (modelProbs[ranks[i]] > minProb) DO
			sumProbs := sumProbs + modelProbs[ranks[i]];
			f.WriteTab;
			f.WriteInt(i + 1);
			f.WriteTab;
			WriteReal(modelProbs[ranks[i]], f);
			f.WriteTab;
			WriteReal(sumProbs, f);
			f.WriteTab;
			f.WriteString(" {");
			models[ranks[i]].Display(model);
			j := 0;
			first := TRUE;
			WHILE j < numberComponents DO
				IF model[j] THEN
					IF ~first THEN f.WriteChar(",") END;
					f.WriteInt(j + 1);
					first := FALSE;
				END;
				INC(j)
			END;
			f.WriteString("} ");
			f.WriteLn;
			INC(i)
		END
	END FormatModelProbs;

	PROCEDURE ComponentProbs* (variable: ARRAY OF CHAR; VAR f: TextMappers.Formatter);
		VAR
			i, len: INTEGER;
			monitors: POINTER TO ARRAY OF ModelsMonitors.Monitor;
	BEGIN
		IF ModelsInterface.IsStar(variable) THEN
			monitors := ModelsIndex.GetMonitors();
			IF monitors # NIL THEN
				len := LEN(monitors)
			ELSE
				len := 0
			END;
			i := 0;
			WHILE i < len DO
				FormatComponentProbs(monitors[i].Name().string, f);
				INC(i)
			END
		ELSE
			FormatComponentProbs(variable, f)
		END
	END ComponentProbs;

	PROCEDURE ModelProbs* (variable: ARRAY OF CHAR; cumulative, minProb: REAL;
	maxNum: INTEGER; VAR f: TextMappers.Formatter);
		VAR
			i, len: INTEGER;
			monitors: POINTER TO ARRAY OF ModelsMonitors.Monitor;
	BEGIN
		IF ModelsInterface.IsStar(variable) THEN
			monitors := ModelsIndex.GetMonitors();
			IF monitors # NIL THEN
				len := LEN(monitors)
			ELSE
				len := 0
			END;
			i := 0;
			WHILE i < len DO
				FormatModelProbs(monitors[i].Name().string, cumulative, minProb, maxNum, f);
				INC(i)
			END
		ELSE
			FormatModelProbs(variable, cumulative, minProb, maxNum, f)
		END
	END ModelProbs;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END ModelsFormatted.
