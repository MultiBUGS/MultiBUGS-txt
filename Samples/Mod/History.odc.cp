(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		 *)

MODULE SamplesHistory;


	

	IMPORT
		Ports, Stores, Views,
		PlotsAxis, PlotsViews,
		SamplesViews;

	CONST
		minW = 155 * Ports.mm;
		minH = 45 * Ports.mm;

	TYPE
		View = POINTER TO RECORD (PlotsViews.View);
			firstChain, start, step: INTEGER;
			sample: POINTER TO ARRAY OF ARRAY OF REAL
		END;

		Factory = POINTER TO RECORD (SamplesViews.Factory) END;

	VAR
		fact: SamplesViews.Factory;
		xTitle: ARRAY 128 OF CHAR;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (v: View) CopyDataFrom (source: PlotsViews.View(*Stores.Store*));
	BEGIN
		WITH source: View DO
			v.firstChain := source.firstChain;
			v.start := source.start;
			v.step := source.step;
			v.sample := source.sample
		END
	END CopyDataFrom;

	PROCEDURE (v: View) DataBounds (OUT minX, maxX, minY, maxY: REAL);
		VAR
			i, j, lenChain, noChains: INTEGER;
	BEGIN
		noChains := LEN(v.sample, 0);
		lenChain := LEN(v.sample, 1);
		minX := v.start;
		maxX := minX + (lenChain - 1) * v.step;
		minY := v.sample[0, 0];
		maxY := v.sample[0, 0];
		i := 0;
		i := 0;
		WHILE i < noChains DO
			j := 0;
			WHILE j < lenChain DO
				minY := MIN(minY, v.sample[i, j]);
				maxY := MAX(maxY, v.sample[i, j]);
				INC(j)
			END;
			INC(i)
		END
	END DataBounds;

	PROCEDURE (v: View) ExternalizeData (VAR wr: Stores.Writer);
		VAR
			i, j, lenChain, noChains: INTEGER;
	BEGIN
		wr.WriteInt(v.firstChain);
		wr.WriteInt(v.start);
		wr.WriteInt(v.step);
		noChains := LEN(v.sample, 0);
		wr.WriteInt(noChains);
		lenChain := LEN(v.sample, 1);
		wr.WriteInt(lenChain);
		i := 0;
		WHILE i < noChains DO
			j := 0;
			WHILE j < lenChain DO
				wr.WriteSReal(SHORT(v.sample[i, j]));
				INC(j)
			END;
			INC(i)
		END
	END ExternalizeData;

	PROCEDURE (v: View) InternalizeData (VAR rd: Stores.Reader);
		VAR
			i, j, lenChain, noChains: INTEGER;
			x: SHORTREAL;
	BEGIN
		rd.ReadInt(v.firstChain);
		rd.ReadInt(v.start);
		rd.ReadInt(v.step);
		rd.ReadInt(noChains);
		rd.ReadInt(lenChain);
		NEW(v.sample, noChains, lenChain);
		i := 0;
		WHILE i < noChains DO
			j := 0;
			WHILE j < lenChain DO
				rd.ReadSReal(x);
				v.sample[i, j] := x;
				INC(j)
			END;
			INC(i)
		END
	END InternalizeData;

	PROCEDURE (v: View) RestoreData (f: Views.Frame; l, t, r, b: INTEGER);
		CONST
			width = Ports.mm DIV 3;
		VAR
			colour, w, h, i, noChains, lenChain, start: INTEGER;
			x: POINTER TO ARRAY OF REAL;
	BEGIN
		v.context.GetSize(w, h);
		start := v.start;
		noChains := LEN(v.sample, 0);
		lenChain := LEN(v.sample, 1);
		NEW(x, lenChain);
		i := 0;
		WHILE i < lenChain DO
			x[i] := start + i * v.step;
			INC(i)
		END;
		i := 0;
		WHILE i < noChains DO
			colour := PlotsViews.Colour(i + v.firstChain - 1);
			v.DrawPath(f, x, v.sample[i], colour, lenChain, Ports.openPoly, width, w, h);
			INC(i)
		END
	END RestoreData;

	PROCEDURE (f: Factory) New (IN title: ARRAY OF CHAR; IN sample: ARRAY OF ARRAY OF REAL;
	start, step, firstChain, numChains: INTEGER): Views.View;
		VAR
			i, j, l, t, r, b, lenChain, noChains: INTEGER;
			v: View;
			xAxis, yAxis: PlotsAxis.Axis;
	BEGIN
		noChains := LEN(sample, 0);
		lenChain := LEN(sample, 1);
		IF lenChain = 1 THEN RETURN NIL END;
		NEW(v);
		v.firstChain := firstChain;
		v.start := start;
		v.step := step;
		NEW(v.sample, noChains, lenChain);
		i := 0;
		WHILE i < noChains DO
			j := 0;
			WHILE j < lenChain DO
				v.sample[i, j] := sample[i, j];
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
		xAxis.SetTitle(xTitle);
		v.SetXAxis(xAxis);
		yAxis := PlotsAxis.New("PlotsStdaxis.Continuous");
		yAxis.SetTitle(title);
		v.SetYAxis(yAxis);
		RETURN v
	END New;

	PROCEDURE (f: Factory) Title (OUT title: ARRAY OF CHAR);
	BEGIN
		title := "#Samples:History"
	END Title;

	PROCEDURE SetXTitle* (title: ARRAY OF CHAR);
	BEGIN
		xTitle := title$
	END SetXTitle;

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
		xTitle := "iteration";
		NEW(f);
		fact := f
	END Init;

	PROCEDURE Install*;
	BEGIN
		SamplesViews.SetFactory(fact)
	END Install;

BEGIN
	Init
END SamplesHistory.
