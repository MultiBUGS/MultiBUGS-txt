(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		 *)

MODULE SamplesDiagnostics;


	

	IMPORT
		Ports, Stores, Strings, Views,
		TextModels, TextRulers, TextViews,
		BugsMappers, BugsTexts,
		MathBGR, MathFunc,
		PlotsAxis, PlotsViews,
		SamplesViews;

	CONST
		minW = 70 * Ports.mm;
		minH = 35 * Ports.mm;

	TYPE
		View = POINTER TO RECORD (PlotsViews.View);
			numer, denom, ratio: POINTER TO ARRAY OF REAL;
			start, step: INTEGER;
			max: REAL
		END;

		Factory = POINTER TO RECORD (SamplesViews.Factory) END;

	VAR
		fact: SamplesViews.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE NewRuler (): TextRulers.Ruler;
		CONST
			numTabs = 10;
		VAR
			i: INTEGER;
			p: TextRulers.Prop;
	BEGIN
		NEW(p);
		p.right := 200 * Ports.mm;
		p.tabs.len := numTabs;
		p.tabs.tab[0].stop := 30 * Ports.mm;
		p.tabs.tab[0].type := {};
		i := 1;
		WHILE i < numTabs DO
			(*	p.tabs.tab[i].stop := 20 * Ports.mm * (i + 1);	*)
			p.tabs.tab[i].stop := p.tabs.tab[i - 1].stop + 20 * Ports.mm;
			p.tabs.tab[i].type := {};
			INC(i)
		END;
		p.valid := {TextRulers.tabs};
		RETURN TextRulers.dir.NewFromProp(p)
	END NewRuler;

	PROCEDURE (v: View) CopyDataFrom (source: PlotsViews.View);
	BEGIN
		WITH source: View DO
			v.step := source.step;
			v.start := source.start;
			v.max := source.max;
			v.numer := source.numer; v.denom := source.denom;
			v.ratio := source.ratio
		END
	END CopyDataFrom;

	PROCEDURE (v: View) ExternalizeData (VAR wr: Stores.Writer);
		VAR
			i, lenSeries: INTEGER;
	BEGIN
		wr.WriteInt(v.step);
		wr.WriteInt(v.start);
		wr.WriteReal(v.max);
		lenSeries := LEN(v.numer);
		wr.WriteInt(lenSeries);
		i := 0;
		WHILE i < lenSeries DO
			wr.WriteReal(v.numer[i]);
			wr.WriteReal(v.denom[i]);
			wr.WriteReal(v.ratio[i]);
			INC(i)
		END
	END ExternalizeData;

	PROCEDURE (v: View) ShowData;
		VAR
			i, lenSeries, beg, end: INTEGER;
			formatter: BugsMappers.Formatter;
			text: TextModels.Model;
			view: Views.View;
	BEGIN
		text := TextModels.dir.New(); view := TextViews.dir.New(text);
		BugsTexts.ConnectFormatter(formatter, text);
		lenSeries := LEN(v.numer);
		formatter.WriteView(NewRuler(), 0, 0);
		formatter.WriteTab;
		formatter.WriteString("----------------------------80% interval----------------------------");
		formatter.WriteLn;
		formatter.WriteTab;
		formatter.WriteString("Unnormalized"); formatter.WriteTab;
		formatter.WriteTab;
		formatter.WriteString("Normalized as plotted"); formatter.WriteLn;
		formatter.WriteString("iteration range"); formatter.WriteTab;
		formatter.WriteString("of pooled"); formatter.WriteTab;
		formatter.WriteString("mean within"); formatter.WriteTab;
		formatter.WriteString("of pooled"); formatter.WriteTab;
		formatter.WriteString("mean within"); formatter.WriteTab;
		formatter.WriteString("BGR ratio"); formatter.WriteLn;
		(*	formatter.WriteString("   of bin"); formatter.WriteTab;	*)
		formatter.WriteTab;
		formatter.WriteString("chains"); formatter.WriteTab;
		formatter.WriteString("chain"); formatter.WriteTab;
		formatter.WriteString("chains"); formatter.WriteTab;
		formatter.WriteString("chain"); formatter.WriteLn;
		i := 0; end := 0;
		(*	beg := v.start;	*)
		WHILE i < lenSeries DO
			(*	beg := beg + v.step;	*)
			end := end + v.step;
			beg := v.start + end DIV 2;
			formatter.WriteInt(beg);
			formatter.WriteString("--");
			formatter.WriteInt(end + v.start - 1);
			formatter.WriteTab;
			formatter.WriteReal(v.numer[i]);
			formatter.WriteTab;
			formatter.WriteReal(v.denom[i]);
			formatter.WriteTab;
			formatter.WriteReal(v.numer[i] / v.max);
			formatter.WriteTab;
			formatter.WriteReal(v.denom[i] / v.max);
			formatter.WriteTab;
			formatter.WriteReal(v.ratio[i]);
			formatter.WriteLn;
			INC(i)
		END;
		Views.OpenAux(view, "Values of Brooks-Gelman-Rubin statistic")
	END ShowData;

	PROCEDURE (v: View) InternalizeData (VAR rd: Stores.Reader);
		VAR
			i, lenSeries: INTEGER;
			x: REAL;
	BEGIN
		rd.ReadInt(v.step);
		rd.ReadInt(v.start);
		rd.ReadReal(v.max);
		rd.ReadInt(lenSeries);
		NEW(v.numer, lenSeries);
		NEW(v.denom, lenSeries);
		NEW(v.ratio, lenSeries);
		i := 0;
		WHILE i < lenSeries DO
			rd.ReadReal(x);
			v.numer[i] := x;
			rd.ReadReal(x);
			v.denom[i] := x;
			rd.ReadReal(x);
			v.ratio[i] := x;
			INC(i)
		END
	END InternalizeData;

	PROCEDURE (v: View) RestoreData (f: Views.Frame; l, t, r, b: INTEGER);
		VAR
			w, h, i, lenSeries, beg1, beg2, end1, end2: INTEGER;
			max, phase: REAL;
	BEGIN
		v.context.GetSize(w, h);
		lenSeries := LEN(v.numer);
		i := 0; end1 := 0; phase := 0;
		(*	beg := v.start;	*)
		max := v.max;
		WHILE i < lenSeries - 1 DO
			(*	beg := beg + v.step;	*)
			(*	end := beg + v.step;	*)
			end1 := end1 + v.step;
			end2 := end1 + v.step;
			beg1 := v.start + end1 DIV 2;
			beg2 := v.start + end2 DIV 2;
			v.DrawLineStyle(f, beg1, 1.0, beg2, 1.0, Ports.black, phase, Ports.mm DIV 3, PlotsViews.dashed, w, h);
			v.DrawLine(f, beg1, v.numer[i] / max, beg2, v.numer[i + 1] / max, Ports.green, Ports.mm DIV 3, w, h);
			v.DrawLine(f, beg1, v.denom[i] / max, beg2, v.denom[i + 1] / max, Ports.blue, Ports.mm DIV 3, w, h);
			v.DrawLine(f, beg1, v.ratio[i], beg2, v.ratio[i + 1], Ports.red, Ports.mm DIV 3, w, h);
			INC(i)
		END
	END RestoreData;

	PROCEDURE (v: View) DataBounds (OUT minX, maxX, minY, maxY: REAL);
		VAR
			i, len: INTEGER;
	BEGIN
		len := LEN(v.numer);
		(*	minX := v.start;	*)
		minX := v.start + v.step DIV 2;
		(*	maxX := v.start + len * v.step;	*)
		maxX := v.start + (len * v.step) DIV 2;
		minY := 0.0;
		maxY := 1;
		i := 0;
		WHILE i < len DO
			maxY := MAX(maxY, v.ratio[i]);
			INC(i)
		END
	END DataBounds;

	PROCEDURE (f: Factory) New (IN title: ARRAY OF CHAR; IN sample: ARRAY OF ARRAY OF REAL;
	start, step, firstChain, numChains: INTEGER): Views.View;
		VAR
			l, t, r, b, len, bin, noChains, lenChain: INTEGER;
			numMax, denomMax: REAL;
			v: View;
			xAxis, yAxis: PlotsAxis.Axis;
			numberStr, plotTitle: ARRAY 128 OF CHAR;
	BEGIN
		noChains := LEN(sample, 0);
		lenChain := LEN(sample, 1);
		len := MathBGR.NumBGRPoints(sample);
		IF len < 5 THEN RETURN NIL END;
		NEW(v);
		bin := MAX(MathBGR.minBin, lenChain DIV MathBGR.maxPoints);
		v.step := bin * step;
		v.start := start + 1;
		NEW(v.numer, len);
		NEW(v.denom, len);
		NEW(v.ratio, len);
		MathBGR.BGR(sample, v.numer, v.denom, v.ratio);
		numMax := MathFunc.Max(v.numer, len);
		denomMax := MathFunc.Max(v.denom, len);
		v.max := MAX(numMax, denomMax);
		v.Init;
		l := 20 * Ports.mm;
		t := 5 * Ports.mm;
		r := 0;
		b := 12 * Ports.mm;
		v.SetSizes(minW, minH, l, t, r, b);
		xAxis := PlotsAxis.New("PlotsStdaxis.Integer");
		xAxis.SetTitle("start-iteration");
		v.SetXAxis(xAxis);
		yAxis := PlotsAxis.New("PlotsStdaxis.Continuous");
		yAxis.SetTitle("bgr diagnostic");
		v.SetYAxis(yAxis);
		Strings.IntToString(firstChain, numberStr);
		plotTitle := title + " chains " + numberStr;
		Strings.IntToString(firstChain + noChains - 1, numberStr);
		plotTitle := plotTitle + " : " + numberStr;
		v.SetTitle(plotTitle);
		RETURN v
	END New;

	PROCEDURE (f: Factory) Title (OUT title: ARRAY OF CHAR);
	BEGIN
		title := "#Samples:bgr diagnostic"
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
END SamplesDiagnostics.
