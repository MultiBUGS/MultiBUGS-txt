(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	   *)

MODULE RanksDensity;

	

	IMPORT
		Ports, Stores, Views, 
		PlotsAxis, PlotsViews;

	CONST
		minW = 70 * Ports.mm; minH = 35 * Ports.mm;

	TYPE
		View = POINTER TO RECORD (PlotsViews.View)
			histogram: POINTER TO ARRAY OF REAL
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (v: View) ExternalizeData (VAR wr: Stores.Writer);
		VAR
			i, len: INTEGER;
	BEGIN
		len := LEN(v.histogram);
		wr.WriteInt(len);
		i := 0;
		WHILE i < len DO
			wr.WriteReal(v.histogram[i]);
			INC(i)
		END
	END ExternalizeData;

	PROCEDURE (v: View) InternalizeData (VAR rd: Stores.Reader);
		VAR
			i, len: INTEGER;
	BEGIN
		rd.ReadInt(len);
		NEW(v.histogram, len);
		i := 0;
		WHILE i < len DO
			rd.ReadReal(v.histogram[i]);
			INC(i)
		END
	END InternalizeData;

	PROCEDURE (v: View) CopyDataFrom (source: PlotsViews.View(*Stores.Store*));
	BEGIN
		WITH source: View DO
			v.histogram := source.histogram
		END
	END CopyDataFrom;

	PROCEDURE (v: View) RestoreData (f: Views.Frame; l, t, r, b: INTEGER);
		VAR
			w, h, i: INTEGER;
			br, lr, maxX, maxY, minX, minY, rr, tr, y: REAL;
	BEGIN
		v.context.GetSize(w, h);
		v.Bounds(minX, maxX, minY, maxY);
		i := 0;
		WHILE i < LEN(v.histogram) DO
			y := v.histogram[i];
			lr := 1 + minX + i - 0.4;
			rr := 1 + minX + i + 0.4;
			tr := y;
			br := 0.0;
			v.DrawRectangle(f, lr, tr, rr, br, Ports.red, Ports.fill, w, h);
			INC(i)
		END
	END RestoreData;

	PROCEDURE (v: View) DataBounds (OUT minX, maxX, minY, maxY: REAL);
		VAR
			i, numBins: INTEGER;
	BEGIN
		numBins := LEN(v.histogram);
		minX := 0;
		maxX := numBins + 1;
		minY := 0;
		maxY := v.histogram[0];
		i := 0;
		WHILE i < numBins DO
			maxY := MAX(maxY, v.histogram[i]);
			INC(i)
		END
	END DataBounds;

	PROCEDURE New* (IN title: ARRAY OF CHAR; IN histogram: ARRAY OF INTEGER;
	sampleSize: INTEGER): PlotsViews.View;
		VAR
			b, i, l, numBins, r, t: INTEGER;
			v: View;
			xAxis, yAxis: PlotsAxis.Axis;
	BEGIN
		NEW(v);
		numBins := LEN(histogram);
		NEW(v.histogram, numBins);
		i := 0;
		WHILE i < numBins DO
			v.histogram[i] := histogram[i] / sampleSize;
			INC(i)
		END;
		v.Init;
		l := 20 * Ports.mm;
		t := 5 * Ports.mm;
		r := 0;
		b := 12 * Ports.mm;
		v.SetSizes(minW, minH, l, t, r, b);
		xAxis := PlotsAxis.New("PlotsStdaxis.Integer");
		xAxis.SetTitle("rank");
		v.SetXAxis(xAxis);
		yAxis := PlotsAxis.New("PlotsStdaxis.Continuous");
		yAxis.SetTitle("rank of " + title);
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
END RanksDensity.

