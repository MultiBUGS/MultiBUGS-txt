(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		 *)

MODULE SamplesTrace;


	

	IMPORT
		Models, Ports, Stores, Views,
		BugsEvaluate, BugsParser,
		MonitorMonitors,
		PlotsAxis, PlotsViews,
		SamplesIndex, SamplesMonitors, SamplesViews,
		UpdaterActions;

	CONST
		size = 2000;
		minW = 70 * Ports.mm;
		minH = 35 * Ports.mm;
		lM = 20 * Ports.mm;
		tM = 5 * Ports.mm;
		rM = 0;
		bM = 12 * Ports.mm;

	TYPE
		View = POINTER TO RECORD (PlotsViews.View)
			sample: POINTER TO ARRAY OF ARRAY OF REAL;
			name: ARRAY 80 OF CHAR;
			offset, numChains, firstChain, start, step, finish: INTEGER;
			modelId: INTEGER
		END;

		Factory = POINTER TO RECORD(SamplesViews.Factory) END;

		Msg = RECORD (Models.Message) END;

		Drawer = POINTER TO RECORD(MonitorMonitors.Drawer) END;

	VAR
		fact: SamplesViews.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		modelId: INTEGER;

	PROCEDURE (v: View) CopyDataFrom (source: PlotsViews.View);
		VAR
			i, j, noChains: INTEGER;
	BEGIN
		WITH source: View DO
			v.modelId := source.modelId;
			v.numChains := source.numChains;
			v.firstChain := source.firstChain;
			v.name := source.name;
			v.offset := source.offset;
			v.start := source.start;
			v.step := source.step;
			v.finish := source.finish;
			noChains := LEN(source.sample, 0);
			NEW(v.sample, noChains, size);
			i := 0;
			WHILE i < noChains DO
				j := 0;
				WHILE j < size DO
					v.sample[i, j] := source.sample[i, j];
					INC(j)
				END;
				INC(i)
			END
		END
	END CopyDataFrom;

	PROCEDURE (v: View) DataBounds (OUT minX, maxX, minY, maxY: REAL);
		VAR
			i, j, lenChain, noChains: INTEGER;
	BEGIN
		minX := v.start + 1;
		maxX := v.finish + 1;
		lenChain := (v.finish - v.start + 1) DIV v.step;
		IF lenChain # 1 THEN
			noChains := LEN(v.sample, 0);
			minY := v.sample[0, 0]; maxY := v.sample[0, 0];
			i := 0;
			WHILE i < noChains DO
				j := 0;
				WHILE j < lenChain DO
					minY := MIN(minY, v.sample[i, j]); maxY := MAX(maxY, v.sample[i, j]); INC(j)
				END;
				INC(i)
			END
		ELSE
			maxX := 1 + minX;
			minY := 0;
			maxY := 1
		END
	END DataBounds;

	PROCEDURE (v: View) ExcludeInvalidProps (VAR valid: SET);
	BEGIN
		valid := valid - PlotsViews.allXBounds - PlotsViews.allYBounds
	END ExcludeInvalidProps;

	PROCEDURE (v: View) ExternalizeData (VAR wr: Stores.Writer);
		VAR
			i, j, noChains: INTEGER;
	BEGIN
		wr.WriteInt(v.numChains);
		wr.WriteInt(v.firstChain);
		wr.WriteString(v.name);
		wr.WriteInt(v.offset);
		wr.WriteInt(v.start);
		wr.WriteInt(v.step);
		wr.WriteInt(v.finish);
		noChains := LEN(v.sample, 0);
		wr.WriteInt(noChains);
		i := 0;
		WHILE i < noChains DO
			j := 0;
			WHILE j < size DO
				wr.WriteSReal(SHORT(v.sample[i, j]));
				INC(j)
			END;
			INC(i)
		END
	END ExternalizeData;

	PROCEDURE (v: View) HandleModelMsg (VAR msg: Models.Message);
	BEGIN
		WITH msg: Msg DO
			Views.Update(v, Views.rebuildFrames)
		ELSE
		END
	END HandleModelMsg;

	PROCEDURE (v: View) InternalizeData (VAR rd: Stores.Reader);
		VAR
			i, j, noChains: INTEGER;
			x: SHORTREAL;
	BEGIN
		v.modelId := - 1;
		rd.ReadInt(v.numChains);
		rd.ReadInt(v.firstChain);
		rd.ReadString(v.name);
		rd.ReadInt(v.offset);
		rd.ReadInt(v.start);
		rd.ReadInt(v.step);
		rd.ReadInt(v.finish);
		rd.ReadInt(noChains);
		NEW(v.sample, noChains, size);
		i := 0;
		WHILE i < noChains DO
			j := 0;
			WHILE j < size DO
				rd.ReadSReal(x);
				v.sample[i, j] := x;
				INC(j)
			END;
			INC(i)
		END
	END InternalizeData;

	PROCEDURE (v: View) UpdateData, NEW;
		VAR
			lastChain, noChains, sampleSize: INTEGER;
			monitor: SamplesMonitors.Monitor;
	BEGIN
		noChains := LEN(v.sample, 0);
		lastChain := v.firstChain + noChains - 1;
		monitor := SamplesIndex.Find(v.name);
		IF (monitor = NIL) OR ~monitor.IsMonitored(v.offset) THEN
			RETURN
		END;
		sampleSize := monitor.SampleSize(v.offset, 0, MAX(INTEGER), v.step);
		v.start := monitor.Start(v.offset);
		v.finish := v.start + (sampleSize - 1) * v.step;
		sampleSize := MIN(sampleSize, size);
		v.start := v.finish - (sampleSize - 1) * v.step;
		monitor.Sample(v.offset, 0, MAX(INTEGER), v.step, v.firstChain, lastChain, v.sample)
	END UpdateData;

	PROCEDURE (v: View) RestoreData (f: Views.Frame; l, t, r, b: INTEGER);
		VAR
			colour, h, i, lenChain, noChains, w: INTEGER;
			minX, maxX, minY, maxY: REAL;
			x: ARRAY size OF REAL;
	BEGIN
		v.context.GetSize(w, h);
		IF v.modelId = modelId THEN
			v.UpdateData
		END;
		v.DataBounds(minX, maxX, minY, maxY);
		IF v.finish - v.start < 2 THEN
			RETURN
		END;
		v.SetBounds(maxX, maxX, minY, maxY);
		noChains := LEN(v.sample, 0);
		i := 0;
		lenChain := (v.finish - v.start + 1) DIV v.step;
		WHILE i < lenChain DO
			x[i] := v.start + i * v.step;
			INC(i)
		END;
		i := 0;
		WHILE i < noChains DO
			colour := PlotsViews.Colour(v.firstChain + i - 1);
			v.DrawPath(f, x, v.sample[i], colour, lenChain, Ports.openPoly, Ports.mm DIV 2, w, h);
			INC(i)
		END
	END RestoreData;

	PROCEDURE (f: Factory) New (IN name: ARRAY OF CHAR; IN sample: ARRAY OF ARRAY OF REAL;
	start, step, firstChain, numChains: INTEGER): Views.View;
		VAR
			first, i, j, lenChain, noChains: INTEGER;
			slice: POINTER TO ARRAY OF INTEGER;
			var: BugsParser.Variable;
			v: View;
			xAxis, yAxis: PlotsAxis.Axis;
	BEGIN
		var := BugsParser.StringToVariable(name);
		noChains := LEN(sample, 0);
		lenChain := LEN(sample, 1);
		NEW(v);
		v.step := step;
		v.numChains := numChains;
		v.firstChain := firstChain;
		v.finish := start + lenChain - 1;
		lenChain := MIN(lenChain, size);
		v.start := v.finish - lenChain + 1;
		v.name := var.name.string$;
		slice := BugsEvaluate.Offsets(var);
		v.offset := slice[0];
		NEW(v.sample, noChains, size);
		i := 0;
		WHILE i < noChains DO
			j := 0;
			WHILE j < size DO
				v.sample[i, j] := 0.0;
				INC(j)
			END;
			INC(i)
		END;
		IF lenChain # 1 THEN
			i := 0;
			first := MAX(0, LEN(sample, 1) - lenChain);
			WHILE i < noChains DO
				j := 0;
				WHILE j < lenChain DO
					v.sample[i, j] := sample[i, first + j];
					INC(j)
				END;
				INC(i)
			END
		END;
		v.Init;
		v.SetSizes(minW, minH, lM, tM, rM, bM);
		xAxis := PlotsAxis.New("PlotsStdaxis.Pixel");
		xAxis.SetTitle("iteration");
		v.SetXAxis(xAxis);
		yAxis := PlotsAxis.New("PlotsStdaxis.Continuous");
		yAxis.SetTitle(name);
		v.SetYAxis(yAxis);
		v.modelId := modelId;
		RETURN v
	END New;

	PROCEDURE (f: Factory) Title (OUT title: ARRAY OF CHAR);
	BEGIN
		title := "#Samples:Dynamic trace"
	END Title;

	PROCEDURE (drawer: Drawer) Update;
		VAR
			msg: Msg;
	BEGIN
		Views.Omnicast(msg)
	END Update;

	PROCEDURE (drawer: Drawer) Clear;
	BEGIN
		INC(modelId)
	END Clear;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory; drawer: Drawer;
	BEGIN
		Maintainer;
		modelId := 0;
		NEW(f);
		fact := f;
		NEW(drawer);
		MonitorMonitors.RegisterDrawer(drawer)
	END Init;

	PROCEDURE Install*;
	BEGIN
		SamplesViews.SetFactory(fact)
	END Install;

BEGIN
	Init
END SamplesTrace.

