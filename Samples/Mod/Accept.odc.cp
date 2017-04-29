(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		 *)

MODULE SamplesAccept;


	

	IMPORT
		Dialog, Ports, Stores, Views, 
		BugsEvaluate, BugsMsg, BugsNames, BugsParser,
		GraphNodes, GraphStochastic,
		PlotsAxis, PlotsViews, 
		SamplesViews, 
		UpdaterActions, UpdaterUpdaters;

	CONST
		minW = 155 * Ports.mm;
		minH = 45 * Ports.mm;
		window = 100;

	TYPE
		View = POINTER TO RECORD (PlotsViews.View);
			firstChain, start, end: INTEGER;
			acceptRate: POINTER TO ARRAY OF ARRAY OF REAL
		END;

		Factory = POINTER TO RECORD (SamplesViews.Factory) END;

	VAR
		fact: SamplesViews.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (v: View) CopyDataFrom (source: PlotsViews.View(*Stores.Store*));
	BEGIN
		WITH source: View DO
			v.firstChain := source.firstChain;
			v.start := source.start;
			v.end := source.end;
			v.acceptRate := source.acceptRate
		END
	END CopyDataFrom;

	PROCEDURE (v: View) DataBounds (OUT minX, maxX, minY, maxY: REAL);
		VAR
			i, j, noChains, numWin: INTEGER;
	BEGIN
		noChains := LEN(v.acceptRate, 0);
		numWin := LEN(v.acceptRate, 1);
		minX := v.start;
		maxX := v.end;
		minY := v.acceptRate[0, 0];
		maxY := v.acceptRate[0, 0];
		i := 0;
		i := 0;
		WHILE i < noChains DO
			j := 0;
			WHILE j < numWin DO
				minY := MIN(minY, v.acceptRate[i, j]);
				maxY := MAX(maxY, v.acceptRate[i, j]);
				INC(j)
			END;
			INC(i)
		END
	END DataBounds;

	PROCEDURE (v: View) ExternalizeData (VAR wr: Stores.Writer);
		VAR
			i, j, noChains, numWin: INTEGER;
	BEGIN
		wr.WriteInt(v.firstChain);
		wr.WriteInt(v.start);
		wr.WriteInt(v.end);
		noChains := LEN(v.acceptRate, 0);
		wr.WriteInt(noChains);
		numWin := LEN(v.acceptRate, 1);
		wr.WriteInt(numWin);
		i := 0;
		WHILE i < noChains DO
			j := 0;
			WHILE j < numWin DO
				wr.WriteSReal(SHORT(v.acceptRate[i, j]));
				INC(j)
			END;
			INC(i)
		END
	END ExternalizeData;

	PROCEDURE (v: View) InternalizeData (VAR rd: Stores.Reader);
		VAR
			i, j, noChains, numWin: INTEGER;
			x: SHORTREAL;
	BEGIN
		rd.ReadInt(v.firstChain);
		rd.ReadInt(v.start);
		rd.ReadInt(v.end);
		rd.ReadInt(noChains);
		rd.ReadInt(numWin);
		NEW(v.acceptRate, noChains, numWin);
		i := 0;
		WHILE i < noChains DO
			j := 0;
			WHILE j < numWin DO
				rd.ReadSReal(x);
				v.acceptRate[i, j] := x;
				INC(j)
			END;
			INC(i)
		END
	END InternalizeData;

	PROCEDURE (v: View) RestoreData (f: Views.Frame; l, t, r, b: INTEGER);
		CONST
			width = Ports.mm DIV 3;
		VAR
			colour, w, h, i, noChains, numWin, start: INTEGER;
			x: POINTER TO ARRAY OF REAL;
	BEGIN
		v.context.GetSize(w, h);
		start := v.start;
		noChains := LEN(v.acceptRate, 0);
		numWin := LEN(v.acceptRate, 1);
		NEW(x, numWin);
		i := 0;
		WHILE i < numWin DO
			x[i] := start + i * window + window DIV 2;
			INC(i)
		END;
		i := 0;
		WHILE i < noChains DO
			colour := PlotsViews.Colour(i + v.firstChain - 1);
			v.DrawPath(f, x, v.acceptRate[i], colour, numWin, Ports.openPoly, width, w, h);
			INC(i)
		END
	END RestoreData;

	PROCEDURE (f: Factory) New (IN title: ARRAY OF CHAR; IN sample: ARRAY OF ARRAY OF REAL;
	start, step, firstChain, numChains: INTEGER): Views.View;
		VAR
			deltaIter, i, j, k, l, t, r, b, lenChain, noChains, numWin, offset: INTEGER;
			v: View;
			xAxis, yAxis: PlotsAxis.Axis;
			var: BugsParser.Variable;
			name: BugsNames.Name;
			offsets: POINTER TO ARRAY OF INTEGER;
			p: GraphNodes.Node;
			updater: UpdaterUpdaters.Updater;
			typeName: Dialog.String;
			l1Norm: REAL;
			acceptRate: INTEGER;
		CONST
			eps = 1.0E-6;
	BEGIN
		IF step # 1 THEN RETURN NIL END;
		var := BugsParser.StringToVariable(title);
		IF var = NIL THEN RETURN NIL END;
		name := var.name;
		offsets := BugsEvaluate.Offsets(var);
		offset := offsets[0];
		p := name.components[offset];
		IF ~(p IS GraphStochastic.Node) OR (GraphNodes.data IN p.props) THEN RETURN NIL END;
		updater := UpdaterActions.FindMVSampler(p(GraphStochastic.Node));
		updater.Install(typeName);
		BugsMsg.Lookup(typeName, typeName);
		noChains := LEN(sample, 0);
		lenChain := LEN(sample, 1);
		IF lenChain = 1 THEN RETURN NIL END;
		NEW(v);
		v.firstChain := firstChain;
		IF start MOD window = 0 THEN
			v.start := start + 1;
			deltaIter := 0
		ELSE
			v.start := (start DIV window) * window + window + 1;
			deltaIter := v.start - start - 1;
		END;
		v.end := ((start + lenChain) DIV window) * window;
		numWin := (v.end - v.start + 1) DIV window;
		IF numWin < 2 THEN RETURN NIL END;
		NEW(v.acceptRate, noChains, numWin);
		i := 0;
		WHILE i < noChains DO
			j := 0;
			WHILE j < numWin DO
				l1Norm := 0.0;
				k := 0;
				WHILE k < window DO
					l1Norm := l1Norm + ABS(sample[i, deltaIter + j * window + k]);
					INC(k)
				END;
				l1Norm := l1Norm / window;
				acceptRate := 0;
				k := 1;
				WHILE k < window DO
					IF ABS(sample[i, deltaIter + j * window + k] - sample[i, deltaIter + j * window + k - 1]) > 
					l1Norm * eps THEN INC(acceptRate) END;
					INC(k)
				END;
				v.acceptRate[i, j] := acceptRate / (window - 1);
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
		yAxis.SetTitle("acceptance rate");
		v.SetYAxis(yAxis);
		v.SetTitle(title + " :  " + typeName);
		RETURN v
	END New;

	PROCEDURE (f: Factory) Title (OUT title: ARRAY OF CHAR);
	BEGIN
		title := "#Samples:Acceptance rate"
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
END SamplesAccept.
