(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		 *)

MODULE SamplesCorrelat;


	

	IMPORT

		Ports, Stores, Views, 
		TextModels, TextRulers, TextViews, 
		BugsMappers, BugsTexts,
		MathSort, 
		PlotsAxis, PlotsViews, 
		SamplesViews;

	CONST
		(*maxLag = 50;*)
		maxLag = 100;
		minW = 70 * Ports.mm;
		minH = 35 * Ports.mm;

	TYPE
		View = POINTER TO RECORD (PlotsViews.View)
			autoCorr: POINTER TO ARRAY OF ARRAY OF REAL;
			colours: POINTER TO ARRAY OF ARRAY OF INTEGER
		END;

		Factory = POINTER TO RECORD(SamplesViews.Factory) END;

	VAR
		fact: SamplesViews.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE AutoCorrelation (IN sample: ARRAY OF REAL; OUT autoCorrelation: ARRAY OF REAL);
		VAR
			i, j, maxLag, sampleSize: INTEGER;
			mean: REAL;
	BEGIN
		maxLag := LEN(autoCorrelation);
		sampleSize := LEN(sample);
		mean := 0.0;
		i := 0;
		WHILE i < sampleSize DO
			mean := mean + sample[i];
			INC(i)
		END;
		mean := mean / sampleSize;
		i := 0;
		WHILE i < maxLag DO
			autoCorrelation[i] := 0.0;
			INC(i)
		END;
		i := 0;
		WHILE i < maxLag DO
			j := 0;
			WHILE j < sampleSize - i DO
				autoCorrelation[i] := autoCorrelation[i] + (sample[j] - mean) * (sample[j + i] - mean);
				INC(j)
			END;
			autoCorrelation[i] := autoCorrelation[i] / (sampleSize - i);
			INC(i)
		END;
		i := 1;
		WHILE i < maxLag DO
			autoCorrelation[i] := autoCorrelation[i] / autoCorrelation[0];
			INC(i)
		END;
		autoCorrelation[0] := 1.0
	END AutoCorrelation;

	PROCEDURE NewRuler (): TextRulers.Ruler;
		CONST
			numTabs = 10;
		VAR
			p: TextRulers.Prop; i: INTEGER;
	BEGIN
		NEW(p);
		p.right := 200 * Ports.mm;
		p.tabs.len := numTabs;
		i := 0;
		WHILE i < numTabs DO
			p.tabs.tab[i].stop := 20 * Ports.mm * (i + 1);
			p.tabs.tab[i].type := {};
			INC(i)
		END;
		p.valid := {TextRulers.tabs};
		RETURN TextRulers.dir.NewFromProp(p)
	END NewRuler;

	PROCEDURE (v: View) CopyDataFrom (source: PlotsViews.View(*Stores.Store*));
	BEGIN
		WITH source: View DO
			v.autoCorr := source.autoCorr;
			v.colours := source.colours
		END
	END CopyDataFrom;

	PROCEDURE (v: View) DataBounds (OUT minX, maxX, minY, maxY: REAL);
	BEGIN
		minX := 0;
		maxX := maxLag;
		minY := - 1;
		maxY := 1
	END DataBounds;

	PROCEDURE (v: View) ExternalizeData (VAR wr: Stores.Writer);
		VAR
			i, j, len, numChains: INTEGER;
	BEGIN
		len := LEN(v.autoCorr, 0);
		wr.WriteInt(len);
		numChains := LEN(v.autoCorr, 1);
		wr.WriteInt(numChains);
		i := 0;
		WHILE i < len DO
			j := 0;
			WHILE j < numChains DO
				wr.WriteSReal(SHORT(v.autoCorr[i, j]));
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < len DO
			j := 0;
			WHILE j < numChains DO
				wr.WriteInt(v.colours[i, j]);
				INC(j)
			END;
			INC(i)
		END
	END ExternalizeData;

	PROCEDURE (v: View) ShowData;
		VAR
			i, j, len, numChains: INTEGER;
			formatter: BugsMappers.Formatter;
			text: TextModels.Model;
			view: Views.View;
			area: REAL;
	BEGIN
		text := TextModels.dir.New();
		view := TextViews.dir.New(text);
		BugsTexts.ConnectFormatter(formatter, text);
		formatter.WriteView(NewRuler(), 0, 0);
		len := LEN(v.autoCorr, 0);
		numChains := LEN(v.autoCorr, 1);
		j := 0;
		formatter.WriteString("area under autocorrelation function upto lag ");
		formatter.WriteInt(len);
		formatter.WriteLn;
		WHILE j < numChains DO
			i := 1;
			area := 1.0;
			WHILE i < len DO
				area := area + v.autoCorr[i, j]; INC(i)
			END;
			formatter.WriteTab;
			formatter.WriteReal(area);
			INC(j)
		END;
		formatter.WriteLn;
		formatter.WriteString("lag ");
		formatter.WriteLn;
		i := 1;
		WHILE i < len DO
			j := 0;
			formatter.WriteInt(i);
			formatter.WriteTab;
			WHILE j < numChains DO
				formatter.WriteReal(v.autoCorr[i, j]);
				formatter.WriteTab;
				INC(j)
			END;
			formatter.WriteLn; INC(i)
		END;
		j := 0;
		formatter.WriteString("area ");
		WHILE j < numChains DO
			i := 1;
			area := 1.0;
			WHILE i < len DO
				area := area + v.autoCorr[i, j]; INC(i)
			END;
			formatter.WriteTab;
			formatter.WriteReal(area);
			INC(j)
		END;
		Views.OpenAux(view, "Values of auto correlation function")
	END ShowData;

	PROCEDURE (v: View) InternalizeData (VAR rd: Stores.Reader);
		VAR
			i, j, len, numChains: INTEGER;
			x: SHORTREAL;
	BEGIN
		rd.ReadInt(len); rd.ReadInt(numChains);
		NEW(v.autoCorr, len, numChains);
		NEW(v.colours, len, numChains);
		i := 0;
		WHILE i < len DO
			j := 0;
			WHILE j < numChains DO
				rd.ReadSReal(x); v.autoCorr[i, j] := x;
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < len DO
			j := 0;
			WHILE j < numChains DO
				rd.ReadInt(v.colours[i, j]);
				INC(j)
			END;
			INC(i)
		END
	END InternalizeData;

	PROCEDURE (v: View) RestoreData (f: Views.Frame; l, t, r, b: INTEGER);
		VAR
			i, j, len, numChains, w, h: INTEGER;
			lr, tr, rr, br, y: REAL;
	BEGIN
		len := LEN(v.autoCorr, 0);
		numChains := LEN(v.autoCorr, 1);
		v.context.GetSize(w, h);
		i := 0;
		WHILE i < len DO
			j := numChains;
			REPEAT
				DEC(j);
				y := v.autoCorr[i, j];
				lr := i - 0.4;
				rr := i + 0.4;
				IF y > 0.0 THEN
					tr := y;
					br := 0.0
				ELSE
					tr := 0.0;
					br := y
				END;
				v.DrawRectangle(f, lr, tr, rr, br, v.colours[i, j], Ports.fill, w, h)
			UNTIL j = 0;
			INC(i)
		END
	END RestoreData;

	PROCEDURE (f: Factory) New (IN name: ARRAY OF CHAR; IN sample: ARRAY OF ARRAY OF REAL;
	start, step, firstChain, numChains: INTEGER): Views.View;
		CONST
			eps = 1.0E-10;
		VAR
			i, j, lenChain, noChains, l, t, r, b: INTEGER;
			mean: REAL;
			ranks, colours: POINTER TO ARRAY OF INTEGER;
			buffer, autoCorr: POINTER TO ARRAY OF REAL;
			v: View;
			xAxis, yAxis: PlotsAxis.Axis;
	BEGIN
		noChains := LEN(sample, 0);
		lenChain := LEN(sample, 1);
		IF lenChain = 1 THEN RETURN NIL END;
		NEW(autoCorr, maxLag);
		NEW(buffer, lenChain);
		NEW(ranks, noChains);
		NEW(colours, noChains);
		NEW(v);
		NEW(v.autoCorr, maxLag, noChains);
		NEW(v.colours, maxLag, noChains);
		j := 0;
		WHILE j < noChains DO
			colours[j] := PlotsViews.Colour(j + firstChain - 1);
			i := 0; WHILE i < lenChain DO buffer[i] := sample[j, i]; INC(i) END;
			i := 0; mean := 0.0; WHILE i < lenChain DO mean := mean + buffer[i]; INC(i) END;
			mean := mean / lenChain;
			i := 0; WHILE (i < lenChain) & (ABS(buffer[i] - mean) < eps) DO INC(i) END;
			IF i = lenChain THEN RETURN NIL END;
			AutoCorrelation(buffer, autoCorr);
			i := 0; WHILE i < maxLag DO v.autoCorr[i, j] := autoCorr[i]; INC(i) END;
			INC(j)
		END;
		i := 0;
		WHILE i < maxLag DO
			j := 0; WHILE j < noChains DO buffer[j] := ABS(v.autoCorr[i, j]); INC(j) END;
			MathSort.Rank(buffer, noChains, ranks);
			j := 0; WHILE j < noChains DO buffer[j] := v.autoCorr[i, j]; INC(j) END;
			j := 0;
			WHILE j < noChains DO
				v.autoCorr[i, j] := buffer[ranks[j]]; v.colours[i, j] := colours[ranks[j]]; INC(j)
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
		xAxis.SetTitle("lag");
		v.SetXAxis(xAxis);
		yAxis := PlotsAxis.New("PlotsStdaxis.Continuous");
		yAxis.SetTitle("auto correlation");
		v.SetYAxis(yAxis);
		v.SetTitle(name);
		RETURN v
	END New;

	PROCEDURE (f: Factory) Title (OUT title: ARRAY OF CHAR);
	BEGIN
		title := "#Samples:Auto-correlation"
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
END SamplesCorrelat.
