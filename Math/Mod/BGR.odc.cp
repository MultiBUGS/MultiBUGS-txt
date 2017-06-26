(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		 *)

MODULE MathBGR;

	

	IMPORT
		MathSort;

	CONST
		minBin* = 100;
		maxPoints* = 100;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE NumBGRPoints* (IN sample: ARRAY OF ARRAY OF REAL): INTEGER;
		VAR
			bin, noChains, lenChain: INTEGER;
	BEGIN
		noChains := LEN(sample, 0);
		lenChain := LEN(sample, 1);
		IF lenChain = 1 THEN RETURN 0 END;
		IF noChains = 1 THEN RETURN 0 END;
		bin := MAX(minBin, lenChain DIV maxPoints);
		IF lenChain < bin THEN RETURN 0 END;
		RETURN lenChain DIV bin;
	END NumBGRPoints;


	PROCEDURE Rank (IN values: ARRAY OF REAL; start, end: INTEGER; OUT ranks: ARRAY OF INTEGER);
		CONST
			trap = 99;
		VAR
			i, j, k, len: INTEGER;

		PROCEDURE Less (l, m: INTEGER): BOOLEAN;
		BEGIN
			RETURN values[ranks[l - 1 + start]] < values[ranks[m - 1 + start]]
		END Less;

		PROCEDURE Swap (l, m: INTEGER);
			VAR
				temp: INTEGER;
		BEGIN
			temp := ranks[l - 1 + start];
			ranks[l - 1 + start] := ranks[m - 1 + start];
			ranks[m - 1 + start] := temp
		END Swap;

	BEGIN
		ASSERT(LEN(values) = LEN(ranks), trap);
		ASSERT(start >= 0, trap);
		ASSERT(end >= start, trap);
		ASSERT(end < LEN(values), trap);
		len := end - start + 1;
		i := start; WHILE i <= end DO ranks[i] := i; INC(i) END;
		IF len > 1 THEN
			i := len DIV 2;
			REPEAT
				j := i;
				LOOP
					k := j * 2; IF k > len THEN EXIT END;
					IF (k < len) & Less(k, k + 1) THEN INC(k) END;
					IF Less(j, k) THEN Swap(j, k) ELSE EXIT END;
					j := k
				END;
				DEC(i)
			UNTIL i = 0;
			i := len;
			REPEAT
				j := 1; Swap(j, i); DEC(i);
				LOOP
					k := j * 2; IF k > i THEN EXIT END;
					IF (k < i) & Less(k, k + 1) THEN INC(k) END;
					Swap(j, k);
					j := k
				END;
				LOOP
					k := j DIV 2;
					IF (k > 0) & Less(k, j) THEN Swap(j, k); j := k ELSE EXIT END
				END
			UNTIL i = 0
		END
	END Rank;

	PROCEDURE BGR* (IN sample: ARRAY OF ARRAY OF REAL; OUT numer, denom, ratio: ARRAY OF REAL);
		VAR
			i, j, k, len, beg, end, num, bin, noChains, lenChain, sampleSize: INTEGER;
			buffer: POINTER TO ARRAY OF REAL;
			map: POINTER TO ARRAY OF INTEGER;
	BEGIN
		len := NumBGRPoints(sample);
		ASSERT(len > 1, 66);
		noChains := LEN(sample, 0);
		lenChain := LEN(sample, 1);
		bin := MAX(minBin, lenChain DIV maxPoints);
		sampleSize := lenChain * noChains;
		k := 0;
		WHILE k < len DO
			denom[k] := 0.0;
			INC(k)
		END;
		NEW(buffer, sampleSize);
		NEW(map, sampleSize);
		(*	put sample in order by iteration	*)
		i := 0;
		WHILE i < lenChain DO
			j := 0;
			WHILE j < noChains DO
				buffer[j + noChains * i] := sample[j, i];
				INC(j);
				INC(k)
			END;
			INC(i)
		END;
		k := 0;
		WHILE k < len DO
			end := (k + 1) * bin; beg := end DIV 2;
			end := end * noChains - 1;
			beg := beg * noChains;
			num := end - beg + 1;
			Rank(buffer, beg, end, map);
			numer[k] := buffer[map[beg + (9 * num) DIV 10]] - buffer[map[beg + num DIV 10]];
			INC(k)
		END;
		i := 0;
		WHILE i < noChains DO
			j := 0;
			WHILE j < lenChain DO
				buffer[j] := sample[i, j];
				INC(j)
			END;
			k := 0;
			WHILE k < len DO
				end := (k + 1) * bin;
				beg := end DIV 2; end := end - 1;
				num := end - beg + 1;
				Rank(buffer, beg, end, map);
				denom[k] := denom[k] + buffer[map[beg + (9 * num) DIV 10]] - buffer[map[beg + num DIV 10]];
				INC(k)
			END;
			INC(i)
		END;
		k := 0;
		WHILE k < len DO
			denom[k] := denom[k] / noChains;
			ratio[k] := numer[k] / denom[k];
			INC(k)
		END;
	END BGR;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END MathBGR.
