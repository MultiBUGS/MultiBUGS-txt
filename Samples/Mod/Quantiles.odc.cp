(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		 *)

MODULE SamplesQuantiles;


	

	IMPORT
		Ports, Stores, Views,
		MathSort,
		PlotsAxis, PlotsViews, 
		SamplesViews;

	CONST
		minW = 70 * Ports.mm;
		minH = 35 * Ports.mm;

	TYPE
		View = POINTER TO RECORD (PlotsViews.View);
			firstChain: INTEGER;
			x: POINTER TO ARRAY OF INTEGER;
			median, lower, upper: POINTER TO ARRAY OF ARRAY OF REAL
		END;

		Factory = POINTER TO RECORD (SamplesViews.Factory) END;

	VAR
		fact: SamplesViews.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (v: View) CopyDataFrom (source: PlotsViews.View);
	BEGIN
		WITH source: View DO
			v.firstChain := source.firstChain;
			v.x := source.x;
			v.median := source.median;
			v.lower := source.lower;
			v.upper := source.upper
		END
	END CopyDataFrom;

	PROCEDURE (v: View) DataBounds (OUT minX, maxX, minY, maxY: REAL);
		VAR
			i, j, numBins, noChains: INTEGER;
	BEGIN
		noChains := LEN(v.upper, 0);
		numBins := LEN(v.upper, 1);
		minX := v.x[0];
		maxX := v.x[numBins - 1];
		minY := v.lower[0, 0];
		maxY := v.upper[0, 0];
		j := 0;
		WHILE j < noChains DO
			i := 0;
			WHILE i < numBins DO
				maxY := MAX(maxY, v.upper[j, i]);
				minY := MIN(minY, v.lower[j, i]);
				INC(i)
			END;
			INC(j)
		END
	END DataBounds;

	PROCEDURE (v: View) ExternalizeData (VAR wr: Stores.Writer);
		VAR
			i, j, numBins, numChains: INTEGER;
	BEGIN
		wr.WriteInt(v.firstChain);
		numBins := LEN(v.x);
		wr.WriteInt(numBins);
		numChains := LEN(v.median, 0);
		wr.WriteInt(numChains);
		j := 0;
		WHILE j < numChains DO
			i := 0;
			WHILE i < numBins DO
				wr.WriteInt(v.x[i]);
				wr.WriteReal(v.median[j, i]);
				wr.WriteReal(v.lower[j, i]);
				wr.WriteReal(v.upper[j, i]);
				INC(i)
			END;
			INC(j)
		END
	END ExternalizeData;

	PROCEDURE (v: View) InternalizeData (VAR rd: Stores.Reader);
		VAR
			i, j, numBins, numChains: INTEGER;
	BEGIN
		rd.ReadInt(v.firstChain);
		rd.ReadInt(numBins);
		rd.ReadInt(numChains);
		NEW(v.x, numBins);
		NEW(v.median, numChains, numBins);
		NEW(v.lower, numChains, numBins);
		NEW(v.upper, numChains, numBins);
		j := 0;
		WHILE j < numChains DO
			i := 0;
			WHILE i < numBins DO
				rd.ReadInt(v.x[i]);
				rd.ReadReal(v.median[j, i]);
				rd.ReadReal(v.lower[j, i]);
				rd.ReadReal(v.upper[j, i]);
				INC(i)
			END;
			INC(j)
		END
	END InternalizeData;

	PROCEDURE (v: View) RestoreData (f: Views.Frame; l, t, r, b: INTEGER);
		VAR
			w, h, i, j, k, numBins, firstChain, noChains: INTEGER;
	BEGIN
		v.context.GetSize(w, h);
		noChains := LEN(v.median, 0); numBins := LEN(v.median, 1);
		j := 0;
		WHILE j < noChains DO
			i := 1;
			k := j + v.firstChain - 1;
			WHILE i < numBins DO
				v.DrawLine(f, v.x[i - 1], v.median[j, i - 1], v.x[i], v.median[j, i], PlotsViews.Colour(k), Ports.mm DIV 2, w, h);
				v.DrawLine(f, v.x[i - 1], v.lower[j, i - 1], v.x[i], v.lower[j, i], PlotsViews.Colour(k), Ports.mm DIV 4, w, h);
				v.DrawLine(f, v.x[i - 1], v.upper[j, i - 1], v.x[i], v.upper[j, i], PlotsViews.Colour(k), Ports.mm DIV 4, w, h);
				INC(i)
			END;
			INC(j)
		END
	END RestoreData;

	PROCEDURE (f: Factory) New (IN title: ARRAY OF CHAR; IN sample: ARRAY OF ARRAY OF REAL;
	start, step, firstChain, numChains: INTEGER): Views.View;
		CONST
			eps = 1.0E-20;
			numBins = 25;
		VAR
			i, j, index, binWidth, iteration, lenChain, noChains, l, t, r, b: INTEGER;
			buffer: POINTER TO ARRAY OF REAL;
			v: View;
			xAxis, yAxis: PlotsAxis.Axis;
	BEGIN
		noChains := LEN(sample, 0);
		lenChain := LEN(sample, 1);
		IF lenChain = 1 THEN
			RETURN NIL
		END;
		NEW(v);
		v.firstChain := firstChain;
		binWidth := lenChain DIV numBins;
		NEW(v.x, numBins);
		NEW(v.median, noChains, numBins);
		NEW(v.lower, noChains, numBins);
		NEW(v.upper, noChains, numBins);
		NEW(buffer, lenChain);
		i := 0;
		WHILE i < noChains DO
			j := 0;
			WHILE j < lenChain DO
				buffer[j] := sample[i, j];
				INC(j)
			END;
			iteration := binWidth; j := 0;
			WHILE j < numBins DO
				v.x[j] := start + iteration;
				MathSort.HeapSort(buffer, iteration);
				index := SHORT(ENTIER(0.5 * iteration + eps)) - 1;
				index := MAX(index, 0);
				v.median[i, j] := buffer[index];
				index := SHORT(ENTIER(0.025 * iteration + eps)) - 1;
				index := MAX(index, 0);
				v.lower[i, j] := buffer[index];
				index := SHORT(ENTIER(0.975 * iteration + eps)) - 1;
				index := MAX(index, 0);
				v.upper[i, j] := buffer[index];
				INC(iteration, binWidth);
				iteration := MIN(lenChain, iteration);
				INC(j)
			END;
			INC(i)
		END;
		v.Init;
		l := 20 * Ports.mm;
		t := 5 * Ports.mm;
		r := 0;
		b := 12 * Ports.mm;
		v.SetSizes(minW, minH, l, t, r, b);
		xAxis := PlotsAxis.New("PlotsStdaxis.Integer");
		xAxis.SetTitle("iteration");
		v.SetXAxis(xAxis);
		yAxis := PlotsAxis.New("PlotsStdaxis.Continuous");
		v.SetYAxis(yAxis);
		v.SetTitle(title);
		RETURN v
	END New;

	PROCEDURE (f: Factory) Title (OUT title: ARRAY OF CHAR);
	BEGIN
		title := "#Samples:Running quantiles"
	END Title;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f
	END Init;

	PROCEDURE Install*;
	BEGIN
		SamplesViews.SetFactory(fact)
	END Install;

BEGIN
	Init
END SamplesQuantiles.
