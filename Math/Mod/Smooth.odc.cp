(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		 *)

(*

Algorithm is from Smoothing Techniques With Implementation in S by Wolfgang Hardle publisher Springer

*)

MODULE MathSmooth;

	

	IMPORT
		Math;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE IsDiscrete* (IN sample: ARRAY OF REAL; sampleSize: INTEGER): BOOLEAN;
		CONST
			eps = 1.0E-20;
		VAR
			isDiscrete: BOOLEAN;
			i: INTEGER;
			abs: REAL;
	BEGIN
		i := 0;
		isDiscrete := TRUE;
		WHILE (i < sampleSize) & isDiscrete DO
			abs := ABS(sample[i]);
			isDiscrete := ABS(abs - SHORT(ENTIER(abs + eps))) < eps;
			INC(i)
		END;
		RETURN isDiscrete
	END IsDiscrete;

	PROCEDURE Max* (IN sample: ARRAY OF REAL; sampleSize: INTEGER): REAL;
		VAR
			i: INTEGER;
			max: REAL;
	BEGIN
		i := 0;
		max := sample[0];
		WHILE i < sampleSize DO
			max := MAX(max, sample[i]);
			INC(i)
		END;
		RETURN max
	END Max;

	PROCEDURE Min* (IN sample: ARRAY OF REAL; sampleSize: INTEGER): REAL;
		VAR
			i: INTEGER;
			min: REAL;
	BEGIN
		i := 0;
		min := sample[0];
		WHILE i < sampleSize DO
			min := MIN(min, sample[i]);
			INC(i)
		END;
		RETURN min
	END Min;

	PROCEDURE BandWidth* (IN sample: ARRAY OF REAL; sampleSize: INTEGER; smooth: REAL): REAL;
		VAR
			i: INTEGER;
			bandWidth, mean, variance: REAL;
	BEGIN
		i := 0;
		mean := 0;
		variance := 0;
		WHILE i < sampleSize DO
			mean := mean + sample[i];
			variance := variance + sample[i] * sample[i];
			INC(i)
		END;
		mean := mean / sampleSize;
		variance := variance / sampleSize - mean * mean;
		bandWidth := Math.Sqrt(variance) / Math.Power(sampleSize, smooth);
		RETURN bandWidth
	END BandWidth;

	PROCEDURE TriWeight* (OUT weights: ARRAY OF REAL);
		VAR
			k, m: INTEGER; norm: REAL;
	BEGIN
		m := LEN(weights);
		norm := 35 * Math.IntPower(m, 6) / (32 * Math.IntPower(m, 6) + 14 * m * m / 3 - 5 / 3);
		k := 0;
		WHILE k < m DO
			weights[k] := norm * Math.IntPower(1 - k * k / (m * m), 3);
			INC(k)
		END
	END TriWeight;

	PROCEDURE BinInfo* (IN sample: ARRAY OF REAL; bandWidth: REAL; sampleSize, m: INTEGER;
	OUT min, delta: REAL; OUT numBins: INTEGER);
		VAR
			start, minVal, maxVal: REAL;
	BEGIN
		delta := bandWidth / m;
		minVal := Min(sample, sampleSize);
		maxVal := Max(sample, sampleSize);
		numBins := SHORT(ENTIER((maxVal - minVal) / delta)) + 2 * (m + 1 + SHORT(ENTIER(Math.Ceiling(0.1 * m))));
		start := minVal - bandWidth - 0.1 * delta;
		min := (SHORT(ENTIER(start / delta)) - 0.5) * delta
	END BinInfo;

	PROCEDURE Bin* (IN sample: ARRAY OF REAL; min, delta: REAL; sampleSize, numBins: INTEGER;
	OUT counts: ARRAY OF INTEGER);
		VAR
			i, j: INTEGER;
	BEGIN
		j := 0; WHILE j < numBins DO counts[j] := 0; INC(j) END;
		i := 0;
		WHILE i < sampleSize DO
			j := SHORT(ENTIER((sample[i] - min) / delta));
			INC(counts[j]);
			INC(i)
		END;
	END Bin;

	PROCEDURE Warp* (IN counts: ARRAY OF INTEGER; IN weights: ARRAY OF REAL;
	sampleSize, numBins, m: INTEGER; bandWidth: REAL; OUT density: ARRAY OF REAL);
		VAR
			j, k: INTEGER;
	BEGIN
		j := 0; WHILE j < numBins DO density[j] := 0; INC(j)END;
		j := m;
		WHILE j < numBins - m DO
			k := 1 - m;
			WHILE k < m DO
				density[j] := density[j] + weights[ABS(k)] * counts[j + k];
				INC(k)
			END;
			INC(j)
		END;
		j := 0;
		WHILE j < numBins DO
			density[j] := density[j] / (sampleSize * bandWidth);
			INC(j)
		END
	END Warp;

	PROCEDURE Smooth* (IN sample, weights: ARRAY OF REAL; sampleSize, numBins, m: INTEGER;
	bandWidth, min, delta: REAL; OUT density: ARRAY OF REAL);
		VAR
			counts: POINTER TO ARRAY OF INTEGER;
	BEGIN
		NEW(counts, numBins);
		Bin(sample, min, delta, sampleSize, numBins, counts);
		Warp(counts, weights, sampleSize, numBins, m, bandWidth, density)
	END Smooth;


	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END MathSmooth.
