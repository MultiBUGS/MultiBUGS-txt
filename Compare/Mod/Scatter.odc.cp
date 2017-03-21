(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



 *)

MODULE CompareScatter;


	

	IMPORT
		Controllers, Dialog, Files, Math, Ports, Properties, Stores, Views, 
		CompareViews, 
		MathSort, 
		PlotsAxis, PlotsDialog, PlotsViews;

	CONST
		numFract = 3;
		minW = 100 * Ports.mm;
		minH = 50 * Ports.mm;
		left = 10 * Ports.mm;
		top = 5 * Ports.mm;
		right = 0 * Ports.mm;
		bottom = 10 * Ports.mm;
		showMeans = 0;
		linear = 0;
		smooth = 1;
		propLogX = 0;
		propLogY = 1;
		propSmooth = 2;
		propMode = 3;
		propLine = 4;
		propShowLine = 5;
		propShowBars = 6;
		propIntercept = 7;
		propGradient = 8;
		propPCol = 9;
		propLineCol = 10;
		propAll = {propLogX..propLineCol};

	TYPE
		View = POINTER TO RECORD (PlotsViews.View)
			mode, line, pCol, lineCol: INTEGER;
			smooth, intercept, gradient: REAL;
			x, mean, median, lower, upper, meanS, medianS: POINTER TO ARRAY OF REAL;
			showLine, showBars, canLogX, canLogY, logX, logY: BOOLEAN
		END;

		Factory = POINTER TO RECORD (CompareViews.Factory) END;

		ScatterProp* = POINTER TO RECORD (Properties.Property)
			mode*, line*, pCol*, lineCol*: INTEGER;
			smooth*, intercept*, gradient*: REAL;
			showLine*, showBars*, logX*, logY*: BOOLEAN
		END;

		SpecialDialog = POINTER TO RECORD (PlotsDialog.SpecialDialog) END;

	VAR
		scatterDialog*: ScatterProp;
		fact: CompareViews.Factory;
		dialogView: Views.View;
		special: SpecialDialog;
		version-: INTEGER;
		maintainer-: ARRAY 20 OF CHAR;
		lineX, lineY: POINTER TO ARRAY OF REAL;

	PROCEDURE (p: ScatterProp) IntersectWith* (q: Properties.Property; OUT equal: BOOLEAN);
		(* not implemented yet! *)
	END IntersectWith;

	PROCEDURE (v: View) InsertProperties (VAR list: Properties.Property);
		VAR
			prop: ScatterProp;
	BEGIN
		NEW(prop);
		prop.mode := v.mode; prop.line := v.line; prop.pCol := v.pCol; prop.lineCol := v.lineCol;
		prop.smooth := v.smooth; prop.intercept := v.intercept; prop.gradient := v.gradient;
		prop.showLine := v.showLine; prop.showBars := v.showBars; prop.logX := v.logX; prop.logY := v.logY;
		prop.valid := propAll; prop.known := prop.valid;
		Properties.Insert(list, prop)
	END InsertProperties;

	PROCEDURE (v: View) Smooth, NEW;
		VAR
			len, i, j: INTEGER; w, wSum: REAL;
	BEGIN
		len := LEN(v.x);
		i := 0;
		WHILE i < len DO
			v.meanS[i] := 0;
			v.medianS[i] := 0;
			j := 0;
			wSum := 0;
			WHILE j < len DO
				w := Math.Exp( - ABS(v.x[i] - v.x[j]) / v.smooth);
				v.meanS[i] := v.meanS[i] + w * v.mean[j];
				v.medianS[i] := v.medianS[i] + w * v.median[j];
				wSum := wSum + w;
				INC(j)
			END;
			v.meanS[i] := v.meanS[i] / wSum;
			v.medianS[i] := v.medianS[i] / wSum;
			INC(i)
		END
	END Smooth;

	PROCEDURE (v: View) SetProperties (prop: Properties.Property);
		CONST
			log = "PlotsStdaxis.Log";
			cont = "PlotsStdaxis.Continuous";
		VAR
			type: ARRAY 80 OF CHAR;
			xAxis, yAxis: PlotsAxis.Axis;

		PROCEDURE BeginMod;
		BEGIN
			Views.BeginModification(Views.notUndoable, v)
		END BeginMod;

		PROCEDURE EndMod;
		BEGIN
			PlotsViews.IncEra; Views.EndModification(Views.notUndoable, v); Views.Update(v, Views.keepFrames)
		END EndMod;

	BEGIN
		WITH prop: ScatterProp DO
			IF propMode IN prop.valid THEN BeginMod; v.mode := prop.mode; EndMod END;
			IF propLine IN prop.valid THEN BeginMod; v.line := prop.line; EndMod END;
			IF propPCol IN prop.valid THEN BeginMod; v.pCol := prop.pCol; EndMod END;
			IF propLineCol IN prop.valid THEN BeginMod; v.lineCol := prop.lineCol; EndMod END;
			IF propSmooth IN prop.valid THEN BeginMod; v.smooth := prop.smooth; v.Smooth; EndMod END;
			IF propIntercept IN prop.valid THEN BeginMod; v.intercept := prop.intercept; EndMod END;
			IF propGradient IN prop.valid THEN BeginMod; v.gradient := prop.gradient; EndMod END;
			IF propShowLine IN prop.valid THEN BeginMod; v.showLine := prop.showLine; EndMod END;
			IF propShowBars IN prop.valid THEN BeginMod; v.showBars := prop.showBars; EndMod END;
			IF (propLogX IN prop.valid) & (prop.logX # v.logX) THEN
				BeginMod; v.logX := prop.logX;
				IF v.logX THEN type := log ELSE type := cont END;
				xAxis := PlotsAxis.New(type);
				v.SetXAxis(xAxis);
				EndMod
			END;
			IF (propLogY IN prop.valid) & (prop.logY # v.logY) THEN
				BeginMod; v.logY := prop.logY;
				IF v.logY THEN type := log ELSE type := cont END;
				yAxis := PlotsAxis.New(type);
				v.SetYAxis(yAxis);
				EndMod
			END
		ELSE
		END
	END SetProperties;

	PROCEDURE (v: View) ExternalizeData (VAR wr: Stores.Writer);
		VAR
			len, i: INTEGER;
	BEGIN
		len := LEN(v.x);
		wr.WriteInt(len);
		i := 0;
		WHILE i < len DO
			wr.WriteReal(v.x[i]);
			wr.WriteReal(v.mean[i]);
			wr.WriteReal(v.median[i]);
			wr.WriteReal(v.lower[i]);
			wr.WriteReal(v.upper[i]);
			wr.WriteReal(v.meanS[i]);
			wr.WriteReal(v.medianS[i]);
			INC(i)
		END;
		wr.WriteInt(v.mode);
		wr.WriteInt(v.line);
		wr.WriteInt(v.pCol);
		wr.WriteInt(v.lineCol);
		wr.WriteReal(v.smooth);
		wr.WriteReal(v.intercept);
		wr.WriteReal(v.gradient);
		wr.WriteBool(v.showLine);
		wr.WriteBool(v.showBars);
		wr.WriteBool(v.canLogX);
		wr.WriteBool(v.canLogY);
		wr.WriteBool(v.logX);
		wr.WriteBool(v.logY)
	END ExternalizeData;

	PROCEDURE (v: View) InternalizeData (VAR rd: Stores.Reader);
		VAR
			len, i: INTEGER;
	BEGIN
		rd.ReadInt(len);
		NEW(v.x, len);
		NEW(v.mean, len);
		NEW(v.median, len);
		NEW(v.lower, len);
		NEW(v.upper, len);
		NEW(v.meanS, len);
		NEW(v.medianS, len);
		i := 0;
		WHILE i < len DO
			rd.ReadReal(v.x[i]);
			rd.ReadReal(v.mean[i]);
			rd.ReadReal(v.median[i]);
			rd.ReadReal(v.lower[i]);
			rd.ReadReal(v.upper[i]);
			rd.ReadReal(v.meanS[i]);
			rd.ReadReal(v.medianS[i]);
			INC(i)
		END;
		rd.ReadInt(v.mode);
		rd.ReadInt(v.line);
		rd.ReadInt(v.pCol);
		rd.ReadInt(v.lineCol);
		rd.ReadReal(v.smooth);
		rd.ReadReal(v.intercept);
		rd.ReadReal(v.gradient);
		rd.ReadBool(v.showLine);
		rd.ReadBool(v.showBars);
		rd.ReadBool(v.canLogX);
		rd.ReadBool(v.canLogY);
		rd.ReadBool(v.logX);
		rd.ReadBool(v.logY)
	END InternalizeData;

	PROCEDURE (v: View) CopyDataFrom (source: PlotsViews.View(*Stores.Store*));
		VAR
			i, len: INTEGER;
	BEGIN
		WITH source: View DO
			len := LEN(source.x);
			NEW(v.x, len);
			NEW(v.mean, len);
			NEW(v.median, len);
			NEW(v.lower, len);
			NEW(v.upper, len);
			NEW(v.meanS, len);
			NEW(v.medianS, len);
			i := 0;
			WHILE i < len DO
				v.x[i] := source.x[i];
				v.mean[i] := source.mean[i];
				v.median[i] := source.median[i];
				v.lower[i] := source.lower[i];
				v.upper[i] := source.upper[i];
				v.meanS[i] := source.meanS[i];
				v.medianS[i] := source.medianS[i];
				INC(i)
			END;
			v.mode := source.mode;
			v.line := source.line;
			v.pCol := source.pCol;
			v.lineCol := source.lineCol;
			v.smooth := source.smooth;
			v.intercept := source.intercept;
			v.gradient := source.gradient;
			v.showLine := source.showLine;
			v.showBars := source.showBars;
			v.canLogX := source.canLogX;
			v.canLogY := source.canLogY;
			v.logX := source.logX;
			v.logY := source.logY
		END
	END CopyDataFrom;

	PROCEDURE (v: View) DataBounds (OUT minX, maxX, minY, maxY: REAL);
		VAR
			i, len: INTEGER;
	BEGIN
		len := LEN(v.x);
		minX := v.x[0];
		maxX := v.x[len - 1];
		minY := v.mean[0];
		maxY := v.mean[0];
		i := 0;
		WHILE i < len DO
			minY := MIN(minY, v.mean[i]);
			maxY := MAX(maxY, v.mean[i]);
			minY := MIN(minY, v.median[i]);
			maxY := MAX(maxY, v.median[i]);
			minY := MIN(minY, v.lower[i]);
			maxY := MAX(maxY, v.lower[i]);
			minY := MIN(minY, v.upper[i]);
			maxY := MAX(maxY, v.upper[i]);
			INC(i)
		END
	END DataBounds;

	PROCEDURE GetLine (intercept, gradient, minX, maxX, minY, maxY: REAL;
	OUT visible: BOOLEAN; OUT x0, y0, x1, y1: REAL);
		VAR
			cut0, cut1, diag, bY0, bY1: REAL; inBox0, inBox1: BOOLEAN;
	BEGIN
		diag := (maxY - minY) / (maxX - minX);
		IF gradient > 0 THEN bY0 := minY; bY1 := maxY ELSE bY0 := maxY; bY1 := minY END;
		IF ABS(gradient) > diag THEN
			cut0 := (bY0 - intercept) / gradient; cut1 := (bY1 - intercept) / gradient;
			inBox0 := (cut0 > minX) & (cut0 < maxX); inBox1 := (cut1 > minX) & (cut1 < maxX);
			visible := inBox0 OR inBox1;
			IF visible THEN
				IF inBox0 THEN
					y0 := bY0; x0 := (y0 - intercept) / gradient;
					IF ABS(gradient) > (maxY - minY) / (maxX - x0) THEN y1 := bY1; x1 := (y1 - intercept) / gradient
					ELSE x1 := maxX; y1 := intercept + gradient * x1
					END
				ELSE
					x0 := minX; y0 := intercept + gradient * x0; y1 := bY1; x1 := (y1 - intercept) / gradient
				END
			END
		ELSE
			cut0 := intercept + gradient * minX; cut1 := intercept + gradient * maxX;
			inBox0 := (cut0 > minY) & (cut0 < maxY); inBox1 := (cut1 > minY) & (cut1 < maxY);
			visible := inBox0 OR inBox1;
			IF visible THEN
				IF inBox0 THEN
					x0 := minX; y0 := intercept + gradient * x0;
					IF ABS(gradient) < ABS(bY1 - y0) / (maxX - minX) THEN x1 := maxX; y1 := intercept + gradient * x1
					ELSE y1 := bY1; x1 := (y1 - intercept) / gradient
					END
				ELSE
					y0 := bY0; x0 := (y0 - intercept) / gradient; x1 := maxX; y1 := intercept + gradient * x1
				END
			END
		END
	END GetLine;

	PROCEDURE NewLine (len: INTEGER);
	BEGIN
		IF (lineX = NIL) OR (len > LEN(lineX)) THEN
			NEW(lineX, len);
			NEW(lineY, len)
		END
	END NewLine;

	PROCEDURE (v: View) DrawStraightLine (f: Views.Frame; thick, w, h: INTEGER), NEW;
		CONST
			eps = 1.0E-40;
			minLen = 10 * Ports.mm;
		VAR
			visible, moveY: BOOLEAN;
			lenX, lenY, i, dx, dy, len: INTEGER;
			minXD, maxXD, minYD, maxYD, minXA, maxXA, minYA, maxYA, x0, y0, x1, y1, nextX, nextY: REAL;
	BEGIN
		v.DataBounds(minXD, maxXD, minYD, maxYD);
		v.Bounds(minXA, maxXA, minYA, maxYA);
		IF ABS(v.gradient) < eps THEN
			IF ~(v.intercept < minYA) & ~(v.intercept > maxYA) THEN
				v.DrawLine(f, minXD, v.intercept, maxXD, v.intercept, v.lineCol, thick, w, h)
			END;
			RETURN
		END;
		GetLine(v.intercept, v.gradient, minXD, maxXD, minYA, maxYA, visible, x0, y0, x1, y1);
		IF visible THEN v.GetDistance(f, x0, y0, x1, y1, w, h, lenX, lenY) END;
		IF ~visible OR (Math.Sqrt(Math.Power(lenX, 2) + Math.Power(ABS(lenY), 2)) < minLen) THEN
			GetLine(v.intercept, v.gradient, minXA, maxXA, minYA, maxYA, visible, x0, y0, x1, y1)
		END;
		IF ~visible THEN RETURN END;
		IF ~v.logX & ~v.logY THEN
			v.DrawLine(f, x0, y0, x1, y1, v.lineCol, thick, w, h)
		ELSE
			v.GetDistance(f, x0, y0, x1, y1, w, h, lenX, lenY); moveY := ABS(lenY) < lenX;
			IF moveY THEN
				dx := 0; IF lenY < 0 THEN dy := - f.dot ELSE dy := f.dot END;
				len := ABS(lenY) DIV f.dot + 2
			ELSE
				dx := f.dot; dy := 0; len := lenX DIV f.dot + 1
			END;
			NewLine(len);
			i := 1; lineX[0] := x0; lineY[0] := y0;
			WHILE i < len - 1 DO
				v.Translate(f, lineX[i - 1], lineY[i - 1], dx, dy, w, h, nextX, nextY);
				IF moveY THEN
					lineY[i] := nextY; lineX[i] := (lineY[i] - v.intercept) / v.gradient
				ELSE
					lineX[i] := nextX; lineY[i] := v.intercept + v.gradient * lineX[i]
				END;
				INC(i)
			END;
			lineX[len - 1] := x1; lineY[len - 1] := y1;
			v.DrawPath(f, lineX, lineY, v.lineCol, len, Ports.openPoly, thick, w, h)
		END
	END DrawStraightLine;

	PROCEDURE (v: View) RestoreData (f: Views.Frame; l, t, r, b: INTEGER);
		CONST
			pSize = Ports.mm DIV 2;
			thick = Ports.mm DIV 3;
		VAR
			w, h, i, len: INTEGER;
			y0, y1: REAL;
	BEGIN
		v.context.GetSize(w, h);
		len := LEN(v.x);
		i := 0;
		WHILE i < len DO
			IF v.mode = showMeans THEN
				y0 := v.mean[i]
			ELSE
				y0 := v.median[i]
			END;
			v.DrawPoint(f, v.x[i], y0, v.pCol, pSize, Ports.fill, w, h);
			IF v.showBars THEN
				v.DrawLine(f, v.x[i], v.lower[i], v.x[i], v.upper[i], v.pCol, thick, w, h)
			END;
			INC(i)
		END;
		IF v.showLine THEN
			CASE v.line OF
			|linear:
				v.DrawStraightLine(f, thick, w, h)
			|smooth:
				i := 1;
				WHILE i < len DO
					IF v.mode = showMeans THEN
						y0 := v.meanS[i - 1];
						y1 := v.meanS[i]
					ELSE
						y0 := v.medianS[i - 1];
						y1 := v.medianS[i]
					END;
					v.DrawLine(f, v.x[i - 1], y0, v.x[i], y1, v.lineCol, thick, w, h);
					INC(i)
				END
			END
		END
	END RestoreData;

	PROCEDURE (f: Factory) New (IN name: ARRAY OF CHAR;
	IN label: ARRAY OF ARRAY OF CHAR;
	IN mean: ARRAY OF REAL;
	IN percentiles: ARRAY OF ARRAY OF REAL;
	IN start, sampleSize: ARRAY OF INTEGER;
	other, axis: POINTER TO ARRAY OF REAL): PlotsViews.View;
		VAR
			i, len: INTEGER;
			minX, maxX, minY, maxY: REAL;
			x: POINTER TO ARRAY OF REAL;
			map: POINTER TO ARRAY OF INTEGER;
			v: View;
			xAxis, yAxis: PlotsAxis.Axis;
	BEGIN
		NEW(v);
		v.pCol := Ports.blue;
		v.lineCol := Ports.black;
		v.mode := showMeans;
		v.line := smooth;
		v.gradient := 0;
		v.showLine := TRUE;
		v.showBars := FALSE;
		v.logX := FALSE;
		v.logY := FALSE;
		len := LEN(axis);
		NEW(x, len);
		NEW(map, len);
		i := 0;
		WHILE i < len DO
			x[i] := axis[i];
			INC(i)
		END;
		NEW(v.x, len);
		NEW(v.mean, len);
		NEW(v.median, len);
		NEW(v.lower, len);
		NEW(v.upper, len);
		NEW(v.meanS, len);
		NEW(v.medianS, len);
		MathSort.Rank(x, len, map);
		i := 0;
		v.intercept := 0;
		WHILE i < len DO
			v.x[i] := axis[map[i]];
			v.mean[i] := mean[map[i]];
			v.median[i] := percentiles[map[i], 1];
			v.lower[i] := percentiles[map[i], 0];
			v.upper[i] := percentiles[map[i], 2];
			v.intercept := v.intercept + v.mean[i];
			INC(i)
		END;
		v.intercept := v.intercept / len;
		v.DataBounds(minX, maxX, minY, maxY);
		v.canLogX := minX > 0;
		v.canLogY := minY > 0;
		v.smooth := (maxX - minX) / 20;
		v.Smooth;
		v.Init;
		v.SetSizes(minW, minH, left, top, right, bottom);
		xAxis := PlotsAxis.New("PlotsStdaxis.Continuous");
		v.SetXAxis(xAxis);
		yAxis := PlotsAxis.New("PlotsStdaxis.Continuous");
		yAxis.SetTitle(name);
		v.SetYAxis(yAxis);
		RETURN v
	END New;

	PROCEDURE (f: Factory) Title (OUT title: ARRAY OF CHAR);
	BEGIN
		title := "#Compare:Scatter plot"
	END Title;

	PROCEDURE IsView (v: PlotsViews.View): BOOLEAN;
	BEGIN
		IF v # NIL THEN
			RETURN v IS View
		ELSE
			RETURN FALSE
		END
	END IsView;

	PROCEDURE Guard* (VAR par: Dialog.Par);
		VAR
			v: PlotsViews.View;
	BEGIN
		v := PlotsDialog.Singleton(); par.disabled := ~IsView(v)
	END Guard;

	PROCEDURE GuardLogX* (VAR par: Dialog.Par);
		VAR
			v: PlotsViews.View;
	BEGIN
		v := PlotsDialog.Singleton(); par.disabled := ~IsView(v);
		IF ~par.disabled THEN
			WITH v: View DO par.disabled := ~v.canLogX END
		END
	END GuardLogX;

	PROCEDURE GuardLogY* (VAR par: Dialog.Par);
		VAR
			v: PlotsViews.View;
	BEGIN
		v := PlotsDialog.Singleton(); par.disabled := ~IsView(v);
		IF ~par.disabled THEN
			WITH v: View DO par.disabled := ~v.canLogY END
		END
	END GuardLogY;

	PROCEDURE GuardLine* (VAR par: Dialog.Par);
		VAR
			v: PlotsViews.View;
	BEGIN
		v := PlotsDialog.Singleton(); par.disabled := ~IsView(v);
		IF ~par.disabled THEN
			WITH v: View DO par.disabled := ~v.showLine END
		END
	END GuardLine;

	PROCEDURE GuardSmooth* (VAR par: Dialog.Par);
		VAR
			v: PlotsViews.View;
	BEGIN
		v := PlotsDialog.Singleton(); par.disabled := ~IsView(v);
		IF ~par.disabled THEN
			WITH v: View DO par.disabled := ~v.showLine OR (v.line # smooth) END
		END
	END GuardSmooth;

	PROCEDURE GuardLinear* (VAR par: Dialog.Par);
		VAR
			v: PlotsViews.View;
	BEGIN
		v := PlotsDialog.Singleton(); par.disabled := ~IsView(v);
		IF ~par.disabled THEN
			WITH v: View DO par.disabled := ~v.showLine OR (v.line # linear) END
		END
	END GuardLinear;

	PROCEDURE GuardRedraw* (VAR par: Dialog.Par);
		VAR
			v: PlotsViews.View;
	BEGIN
		v := PlotsDialog.Singleton(); par.disabled := ~IsView(v);
		IF ~par.disabled THEN
			WITH v: View DO par.disabled := ~v.showLine OR ~(scatterDialog.smooth > 0) END
		END
	END GuardRedraw;

	PROCEDURE Notifier* (op, from, to: INTEGER);
		CONST
			ignore = {propSmooth, propIntercept, propGradient};
		VAR
			v: PlotsViews.View; old: SET;
	BEGIN
		IF op = Dialog.changed THEN
			v := PlotsDialog.Singleton();
			Controllers.SetCurrentPath(Controllers.targetPath);
			old := scatterDialog.valid; scatterDialog.valid := scatterDialog.valid - ignore;
			Properties.EmitProp(NIL, scatterDialog);
			scatterDialog.valid := old;
			Controllers.ResetCurrentPath
		END
	END Notifier;

	PROCEDURE RedrawLine*;
		VAR
			v: PlotsViews.View;
	BEGIN
		v := PlotsDialog.Singleton();
		Controllers.SetCurrentPath(Controllers.targetPath);
		Properties.EmitProp(NIL, scatterDialog);
		Controllers.ResetCurrentPath
	END RedrawLine;

	PROCEDURE (s: SpecialDialog) Dialog (): Views.View;
	BEGIN
		RETURN dialogView
	END Dialog;

	PROCEDURE (s: SpecialDialog) Set (prop: Properties.Property);
	BEGIN
		WITH prop: ScatterProp DO
			scatterDialog^ := prop^;
			Dialog.Update(scatterDialog)
		ELSE
		END
	END Set;

	PROCEDURE ViewInstall*;
	BEGIN
		PlotsDialog.SetSpecial(special)
	END ViewInstall;

	PROCEDURE Install*;
	BEGIN
		CompareViews.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "D.J.Lunn"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory;
			fractions: POINTER TO ARRAY OF REAL;
			loc: Files.Locator;
	BEGIN
		Maintainer;
		NEW(scatterDialog);
		scatterDialog.pCol := Ports.blue;
		scatterDialog.lineCol := Ports.black;
		scatterDialog.mode := showMeans;
		scatterDialog.line := smooth;
		scatterDialog.gradient := 0;
		scatterDialog.showLine := TRUE;
		scatterDialog.showBars := FALSE;
		scatterDialog.logX := FALSE;
		scatterDialog.logY := FALSE;
		scatterDialog.valid := propAll;
		(*NEW(scatterDialog.prop);
		PlotsDialog.InsertDialog(scatterDialog);*)
		lineX := NIL;
		lineY := NIL;
		NEW(f);
		NEW(fractions, numFract);
		fractions[0] := 2.5;
		fractions[1] := 50;
		fractions[2] := 97.5;
		f.SetFractions(fractions);
		f.SetArguments({CompareViews.node, CompareViews.axis});
		fact := f;
		loc := Files.dir.This("Compare/Rsrc");
		dialogView := Views.OldView(loc, "ScatterDialog");
		NEW(special)
	END Init;

BEGIN
	Init
END CompareScatter.
