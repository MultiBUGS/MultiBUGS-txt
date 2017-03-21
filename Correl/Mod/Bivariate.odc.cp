(*		


	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	   *)

MODULE CorrelBivariate;


	

	IMPORT
		Ports, Stores, Views, 
		PlotsAxis, PlotsViews;

	CONST
		minW = 55 * Ports.mm;
		minH = 50 * Ports.mm;

	TYPE
		View = POINTER TO RECORD (PlotsViews.View);
			x, y: POINTER TO ARRAY OF ARRAY OF REAL
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (v: View) ExternalizeData (VAR wr: Stores.Writer);
		VAR
			i, j, lenSeries, numChains: INTEGER;
	BEGIN
		numChains := LEN(v.x, 0);
		wr.WriteInt(numChains);
		lenSeries := LEN(v.x, 1);
		wr.WriteInt(lenSeries);
		i := 0;
		WHILE i < numChains DO
			j := 0;
			WHILE j < lenSeries DO
				wr.WriteSReal(SHORT(v.x[i, j]));
				wr.WriteSReal(SHORT(v.y[i, j]));
				INC(j)
			END;
			INC(i)
		END
	END ExternalizeData;

	PROCEDURE (v: View) InternalizeData (VAR rd: Stores.Reader);
		VAR
			i, j, lenSeries, numChains: INTEGER;
			x: SHORTREAL;
	BEGIN
		rd.ReadInt(numChains);
		rd.ReadInt(lenSeries);
		NEW(v.x, numChains, lenSeries);
		NEW(v.y, numChains, lenSeries);
		i := 0;
		WHILE i < numChains DO
			j := 0;
			WHILE j < lenSeries DO
				rd.ReadSReal(x); v.x[i, j] := x;
				rd.ReadSReal(x); v.y[i, j] := x;
				INC(j)
			END;
			INC(i)
		END
	END InternalizeData;

	PROCEDURE (v: View) CopyDataFrom (source: PlotsViews.View(*Stores.Store*));
	BEGIN
		WITH source: View DO
			v.x := source.x;
			v.y := source.y
		END
	END CopyDataFrom;

	PROCEDURE (v: View) RestoreData (f: Views.Frame; l, t, r, b: INTEGER);
		VAR
			colour, h, i, j, lenSeries, numChains, w: INTEGER;
	BEGIN
		v.context.GetSize(w, h);
		numChains := LEN(v.x, 0);
		lenSeries := LEN(v.x, 1);
		i := 0;
		WHILE i < numChains DO
			colour := PlotsViews.Colour(i);
			j := 0;
			WHILE j < lenSeries DO
				v.DrawPoint(f, v.x[i, j], v.y[i, j], colour, Ports.mm DIV 2, Ports.fill, w, h);
				INC(j)
			END;
			INC(i)
		END
	END RestoreData;

	PROCEDURE (v: View) DataBounds (OUT minX, maxX, minY, maxY: REAL);
		VAR
			i, j, numChains, sampleSize: INTEGER;
	BEGIN
		numChains := LEN(v.x, 0);
		sampleSize := LEN(v.x, 1);
		minX := v.x[0, 0];
		maxX := v.x[0, 0];
		minY := v.y[0, 0];
		maxY := v.y[0, 0];
		i := 0;
		WHILE i < numChains DO
			j := 0;
			WHILE j < sampleSize DO
				minX := MIN(minX, v.x[i, j]);
				maxX := MAX(maxX, v.x[i, j]);
				minY := MIN(minY, v.y[i, j]);
				maxY := MAX(maxY, v.y[i, j]);
				INC(j)
			END;
			INC(i)
		END
	END DataBounds;

	PROCEDURE New* (IN xTitle, yTitle: ARRAY OF CHAR;
	IN sampleX, sampleY: ARRAY OF ARRAY OF REAL): PlotsViews.View;
		VAR
			b, i, j, l, numChains, r, sampleSize, t: INTEGER;
			v: View;
			xAxis, yAxis: PlotsAxis.Axis;
	BEGIN
		numChains := LEN(sampleX, 0);
		sampleSize := LEN(sampleX, 1);
		ASSERT(LEN(sampleY, 1) = sampleSize, 25);
		NEW(v);
		NEW(v.x, numChains, sampleSize);
		NEW(v.y, numChains, sampleSize);
		i := 0;
		WHILE i < numChains DO
			j := 0;
			WHILE j < sampleSize DO
				v.x[i, j] := sampleX[i, j];
				v.y[i, j] := sampleY[i, j];
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
		xAxis := PlotsAxis.New("PlotsStdaxis.Continuous");
		xAxis.SetTitle(xTitle);
		v.SetXAxis(xAxis);
		yAxis := PlotsAxis.New("PlotsStdaxis.Continuous");
		yAxis.SetTitle(yTitle);
		v.SetYAxis(yAxis);
		RETURN v
	END New;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END CorrelBivariate.
