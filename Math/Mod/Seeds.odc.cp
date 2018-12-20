MODULE MathSeeds;

	

	IMPORT
		SYSTEM,
		Files, Strings, Views,
		StdLog,
		TextControllers, TextMappers, TextModels, TextViews;

	CONST
		numSeeds = 256;
		M = 7;
		N = 25;
		s = 7;
		t = 15;
		mag01 = BITS(8EBFD028H);
		b = BITS(2B5B2500H);
		c = BITS(0DB8B0000H);
		trillion = 1000000000000;

	VAR
		x: ARRAY N OF SET;
		k: INTEGER;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Generate*;
		VAR
			i, j: INTEGER;
			int: LONGINT;
			y: SET;
			count: LONGINT;
			string: ARRAY 64 OF CHAR;
			v: Views.View;
			loc: Files.Locator;
	BEGIN
		j := 0;
		WHILE j < numSeeds DO
			count := 0;
			k := 0;
			WHILE count < trillion DO
				IF k = N THEN
					i := 0;
					WHILE i < N - M DO
						IF MIN(SET) IN x[i] THEN
							x[i] := x[i + M] / SYSTEM.LSH(x[i], - 1) / mag01
						ELSE
							x[i] := x[i + M] / SYSTEM.LSH(x[i], - 1)
						END;
						INC(i)
					END;
					WHILE i < N DO
						IF MIN(SET) IN x[i] THEN
							x[i] := x[i + M - N] / SYSTEM.LSH(x[i], - 1) / mag01
						ELSE
							x[i] := x[i + M - N] / SYSTEM.LSH(x[i], - 1)
						END;
						INC(i)
					END;
					k := 0
				END;
				INC(k);
				INC(count, 25)
			END;
			i := 0;
			WHILE i < N DO
				StdLog.String("x["); StdLog.Int(j+1); StdLog.String(" , "); StdLog.Int(i); StdLog.String("] := ");
				StdLog.Set(x[i]); StdLog.String(";"); StdLog.Ln;
				INC(i)
			END;
			Strings.IntToString(j + 1, string);
			string := "seed" + string;
			v := TextViews.dir.New(StdLog.text);
			loc := Files.dir.This("");
			Views.RegisterView(v, loc, string$);
			INC(j)
		END
	END Generate;


	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer;
	x[0] := {0, 1, 3, 5, 7, 8, 10, 11, 14, 17, 20..24, 26, 28, 31};
	x[1] := {0, 2, 4, 9, 12, 14, 19, 21, 22, 24, 25, 27};
	x[2] := {0..2, 5..7, 9, 11, 14, 15, 18, 19, 21, 22, 24..26, 29..31};
	x[3] := {0, 3..5, 9, 14, 15, 17..21, 24..27, 29, 31};
	x[4] := {0, 1, 5, 8, 10, 11, 13, 15..20, 22, 24, 28..30};
	x[5] := {0, 2, 3, 5, 7, 12, 15, 16, 18, 21, 23, 26, 29};
	x[6] := {0..3, 5..8, 10, 12, 13, 15, 18, 21..24, 27, 29, 30};
	x[7] := {0, 6, 8, 13, 14, 16, 18, 22, 24..29, 31};
	x[8] := {0, 1, 3..6, 8, 9, 11, 12, 18..21, 23, 25, 26, 28, 31};
	x[9] := {1, 4, 6, 11..16, 18..21, 23..26, 29, 31};
	x[10] := {0..2, 4, 5, 7, 8, 10, 12..14, 17..20, 22..24, 30, 31};
	x[11] := {0, 3, 6..8, 11, 13, 15, 19, 20, 22, 27, 31};
	x[12] := {0, 1, 4, 7, 9, 10, 12..14, 19, 21, 23, 24, 26, 27, 29};
	x[13] := {0, 2..4, 6..8, 11..18, 20, 22, 25, 26, 28, 29, 31};
	x[14] := {0..4, 7, 9, 11, 15, 18..20, 22..31};
	x[15] := {0, 4..6, 9, 11, 12, 14..16, 21, 24, 31};
	x[16] := {0, 1, 3, 6, 7, 9..13, 17, 23..25, 27, 31};
	x[17] := {0, 2, 4..8, 10, 16, 18..20, 22, 27, 31};
	x[18] := {0..2, 6, 8, 10, 11, 14, 15, 21, 25..27, 30};
	x[19] := {0, 3, 4, 6..8, 10, 12, 14, 15, 17, 19, 20, 23, 25, 27, 28, 30};
	x[20] := {0, 1, 10, 11, 18, 19, 21, 24, 28, 30};
	x[21] := {0, 2, 3, 6, 7, 10..14, 16, 18, 23, 25, 27, 29..31};
	x[22] := {0..3, 8, 9, 12, 14..16, 22, 23, 26, 27, 30};
	x[23] := {0, 5, 7, 11, 13, 15, 16, 20, 23, 27, 31};
	x[24] := {0, 1, 3, 4, 6, 7, 9, 11, 13, 15..18, 20, 21, 23, 25, 26, 29, 31};
	k := 0;
END MathSeeds.

MathSeeds.Generate
