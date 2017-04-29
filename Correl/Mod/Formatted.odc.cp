(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	   *)

MODULE CorrelFormatted;


	

	IMPORT
		BugsFiles, BugsIndex, BugsParser,
		CorrelInterface,
		SamplesInterface,
		TextMappers, TextModels;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE WriteReal (x: REAL; VAR f: TextMappers.Formatter);
	BEGIN
		f.WriteRealForm(x, BugsFiles.prec, 0, 0, TextModels.digitspace)
	END WriteReal;

		PROCEDURE PrintMatrix* (string0, string1: ARRAY OF CHAR;
	beg, end, thin, firstChain, lastChain: INTEGER; VAR f: TextMappers.Formatter);
		VAR
			i, j, num0, num1: INTEGER;
			offsets0, offsets1: POINTER TO ARRAY OF INTEGER;
			matrix: POINTER TO ARRAY OF ARRAY OF REAL;
			var0, var1: BugsParser.Variable;
			label0, label1: ARRAY 128 OF CHAR;
	BEGIN
		IF string1 = "" THEN string1 := string0$ END;
		var0 := BugsParser.StringToVariable(string0);
		IF var0 = NIL THEN RETURN END;
		num0 := SamplesInterface.NumComponents(string0, beg, end, thin);
		IF num0 = 0 THEN RETURN END;
		NEW(offsets0, num0);
		var1 := BugsParser.StringToVariable(string1);
		IF var1 = NIL THEN RETURN END;
		num1 := SamplesInterface.NumComponents(string1, beg, end, thin);
		IF num1 = 0 THEN RETURN END;
		NEW(offsets1, num1);
		matrix := CorrelInterface.CorrelationMatrix(string0, string1, beg, end, thin, firstChain, lastChain);
		SamplesInterface.Offsets(string0, beg, end, thin, offsets0);
		SamplesInterface.Offsets(string1, beg, end, thin, offsets1);
		string0 := var0.name.string$;
		string1 := var1.name.string$;
		j := 0;
		f.WriteTab;
		WHILE j < num1 DO
			f.WriteTab;
			BugsIndex.MakeLabel(string1, offsets1[j], label1);
			f.WriteString(label1);
			INC(j)
		END;
		f.WriteLn;
		i := 0;
		WHILE i < num0 DO
			BugsIndex.MakeLabel(string0, offsets0[i], label0);
			f.WriteString(label0);
			j := 0;
			WHILE j < num1 DO
				f.WriteTab;
				WriteReal(matrix[i, j], f);
				INC(j)
			END;
			f.WriteLn;
			INC(i)
		END
	END PrintMatrix;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END CorrelFormatted.

