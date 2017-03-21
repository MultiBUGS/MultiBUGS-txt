(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE SamplesStatistics;

	

	IMPORT
		Math,
		MathSort;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Sort* (IN sample: ARRAY OF ARRAY OF REAL): POINTER TO ARRAY OF REAL;
		VAR
			i, j, numChains, sampleSize: INTEGER;
			buffer: POINTER TO ARRAY OF REAL;
	BEGIN
		numChains := LEN(sample, 0);
		sampleSize := LEN(sample, 1);
		NEW(buffer, sampleSize * numChains);
		i := 0;
		WHILE i < numChains DO
			j := 0;
			WHILE j < sampleSize DO
				buffer[i * sampleSize + j] := sample[i, j];
				INC(j)
			END;
			INC(i)
		END;
		MathSort.HeapSort(buffer, LEN(buffer));
		RETURN buffer
	END Sort;

	PROCEDURE HPDInterval* (IN sample: ARRAY OF REAL; fraction: REAL; OUT lower, upper: REAL);
		CONST
			eps = 1.0E-20;
		VAR
			j, index, range, sampleSize: INTEGER;
			interval: REAL;
	BEGIN
		sampleSize := LEN(sample);
		interval := sample[sampleSize - 1] - sample[0];
		range := SHORT(ENTIER(sampleSize * (fraction / 100) + eps));
		j := 0;
		index := 0;
		WHILE j + range < sampleSize DO
			IF sample[j + range] - sample[j] < interval THEN
				interval := sample[j + range] - sample[j];
				index := j
			END;
			INC(j)
		END;
		lower := sample[index];
		upper := sample[index + range];
	END HPDInterval;

	PROCEDURE Mode* (IN sample: ARRAY OF REAL): REAL;
		CONST
			delta = 1.0;
		VAR
			lower, upper: REAL;
	BEGIN
		HPDInterval(sample, delta, lower, upper);
		RETURN 0.5 * (lower + upper);
	END Mode;

	PROCEDURE Percentile* (IN sample: ARRAY OF REAL; fraction: REAL): REAL;
		CONST
			eps = 1.0E-20;
		VAR
			index, sampleSize: INTEGER;
	BEGIN
		sampleSize := LEN(sample);
		index := SHORT(ENTIER(sampleSize * (fraction / 100) + eps));
		RETURN sample[index];
	END Percentile;

	PROCEDURE Mean* (IN sample: ARRAY OF ARRAY OF REAL): REAL;
		VAR
			i, j, n, n1, numChains, sampleSize: INTEGER;
			delta, mean: REAL;
	BEGIN
		mean := 0.0;
		n := 0;
		i := 0;
		numChains := LEN(sample, 0);
		sampleSize := LEN(sample, 1);
		WHILE i < numChains DO
			j := 0;
			WHILE j < sampleSize DO
				n1 := n + 1;
				delta := n / n1;
				mean := delta * mean + sample[i, j] / n1;
				INC(n);
				INC(j)
			END;
			INC(i)
		END;
		RETURN mean
	END Mean;

	PROCEDURE Moments* (IN sample: ARRAY OF ARRAY OF REAL; OUT mean, sd, skew, exKur, error: REAL);
		CONST
			eps = 1.0E-20;
		VAR
			binSize, i, j, k, l, l0, l1, n, n1, noChains, numBins, sampleSize: INTEGER;
			batchMean, delta, mean2, mean3, mean4, var, x: REAL;
	BEGIN
		noChains := LEN(sample, 0);
		sampleSize := LEN(sample, 1);
		mean := 0.0;
		mean2 := 0.0;
		mean3 := 0.0;
		mean4 := 0.0;
		n := 0;
		i := 0;
		WHILE i < noChains DO
			j := 0;
			WHILE j < sampleSize DO
				x := sample[i, j];
				n1 := n + 1;
				delta := n / n1;
				mean := delta * mean + x / n1;
				mean2 := delta * mean2 + x * x / n1;
				mean3 := delta * mean3 + x * x * x / n1;
				mean4 := delta * mean4 + x * x * x * x / n1;
				INC(n);
				INC(j)
			END;
			INC(i)
		END;
		var := mean2 - mean * mean;
		IF var > 0 THEN
			sd := Math.Sqrt(var + eps);
			skew := (mean3 - 3 * mean2 * mean + 2 * mean * mean * mean) / (sd * sd * sd);
			exKur := (mean4 - 4 * mean3 * mean + 6 * mean2 * mean * mean - 3 * mean * mean * mean * mean) / 
			(sd * sd * sd * sd) - 3
		ELSE
			sd := 0
		END;
		binSize := SHORT(ENTIER(Math.Sqrt(sampleSize)));
		numBins := sampleSize DIV binSize;
		l0 := (binSize + 1) * numBins - sampleSize; l1 := sampleSize - binSize * numBins;
		mean2 := 0.0;
		l := 0;
		WHILE l < noChains DO
			i := 0;
			j := 0;
			WHILE j < l0 DO
				k := 0;
				batchMean := 0.0;
				WHILE k < binSize DO
					batchMean := batchMean + sample[l, i];
					INC(k);
					INC(i)
				END;
				batchMean := batchMean / binSize;
				mean2 := mean2 + (batchMean - mean) * (batchMean - mean);
				INC(j)
			END;
			j := 0;
			WHILE j < l1 DO
				k := 0;
				batchMean := 0.0;
				WHILE k < binSize + 1 DO
					batchMean := batchMean + sample[l, i];
					INC(k);
					INC(i)
				END;
				batchMean := batchMean / (binSize + 1);
				mean2 := mean2 + (batchMean - mean) * (batchMean - mean);
				INC(j)
			END;
			INC(l)
		END;
		IF numBins > 1 THEN
			error := Math.Sqrt(binSize * mean2 / (numBins * noChains - 1) + eps) / Math.Sqrt(sampleSize * noChains)
		ELSE
			error := - 100
		END
	END Moments;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END SamplesStatistics.
