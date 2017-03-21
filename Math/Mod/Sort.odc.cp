(* 		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



  *)

MODULE MathSort;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE HeapSort* (VAR values: ARRAY OF REAL; len: INTEGER);
		VAR
			i, j, k: INTEGER;

		PROCEDURE Less (l, m: INTEGER): BOOLEAN;
		BEGIN
			RETURN values[l - 1] < values[m - 1]
		END Less;

		PROCEDURE Swap (l, m: INTEGER);
			VAR
				temp: REAL;
		BEGIN
			temp := values[l - 1];
			values[l - 1] := values[m - 1];
			values[m - 1] := temp
		END Swap;

	BEGIN
		ASSERT(LEN(values) >= len, 20);
		IF len > 1 THEN
			i := len DIV 2;
			REPEAT
				j := i;
				LOOP
					k := j * 2;
					IF k > len THEN EXIT END;
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
	END HeapSort;

	PROCEDURE Rank* (IN values: ARRAY OF REAL; len: INTEGER; OUT ranks: ARRAY OF INTEGER);
		VAR
			i, j, k: INTEGER;

		PROCEDURE Less (l, m: INTEGER): BOOLEAN;
		BEGIN
			RETURN values[ranks[l - 1]] < values[ranks[m - 1]]
		END Less;

		PROCEDURE Swap (l, m: INTEGER);
			VAR
				temp: INTEGER;
		BEGIN
			temp := ranks[l - 1];
			ranks[l - 1] := ranks[m - 1];
			ranks[m - 1] := temp
		END Swap;

	BEGIN
		ASSERT(LEN(values) >= len, 20);
		ASSERT(LEN(ranks) >= len, 21);
		i := 0;
		WHILE i < len DO
			ranks[i] := i;
			INC(i)
		END;
		IF len > 1 THEN
			i := len DIV 2;
			REPEAT
				j := i;
				LOOP
					k := j * 2; 
					IF k > len THEN EXIT END;
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

	PROCEDURE Ranked* (VAR values: ARRAY OF REAL; rank, len: INTEGER): REAL;
		VAR
			x: REAL;
	BEGIN
		ASSERT(LEN(values) >= len, 20);
		ASSERT(LEN(values) >= rank, 21);
		ASSERT(rank <= len, 22);
		HeapSort(values, len);
		x := values[rank - 1];
		RETURN x
	END Ranked;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END MathSort.

