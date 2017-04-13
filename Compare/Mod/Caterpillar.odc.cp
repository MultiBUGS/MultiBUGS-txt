(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		 *)

MODULE CompareCaterpillar;


	

	IMPORT
		Controllers, Dialog, Files, Fonts, Ports, Properties, Stores, Strings, Views, 
		TextModels, TextRulers, TextViews, 
		BugsMappers, BugsTexts, 
		CompareViews, MathSort, 
		PlotsAxis, PlotsDialog, PlotsViews;

	CONST
		numFract = 3;
		minW = 100 * Ports.mm; minH = 100 * Ports.mm;
		left = 10 * Ports.mm; top = 5 * Ports.mm; right = 0 * Ports.mm; bottom = 10 * Ports.mm;
		showMeans = 0; showMedians = 1; barVert = 0; barHoriz = 1;
		propMode = 0; propOrientation = 1; propBaseline = 2; propShowLine = 3;
		propShowLabels = 4; propRanked = 5; propLogScale = 6;
		propAll = {propMode..propLogScale};
		lenLabel = 128;

	TYPE
		View = POINTER TO RECORD (PlotsViews.View)
			mode, orientation: INTEGER;
			baseline: REAL;
			showLine, showLabels, ranked, canLog, logScale: BOOLEAN;
			label: POINTER TO ARRAY OF ARRAY OF CHAR;
			index: POINTER TO ARRAY OF INTEGER;
			mean, median, lower, upper: POINTER TO ARRAY OF REAL
		END;

		Factory = POINTER TO RECORD (CompareViews.Factory) END;

		CatProp* = POINTER TO RECORD (Properties.Property)
			mode*, orientation*: INTEGER;
			showLine*, showLabels*, ranked*, logScale*: BOOLEAN
		END;

		SpecialDialog = POINTER TO RECORD (PlotsDialog.SpecialDialog) END;

	VAR
		catDialog*: CatProp;
		fact: CompareViews.Factory;
		dialogView: Views.View;
		special: SpecialDialog;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (p: CatProp) IntersectWith* (q: Properties.Property; OUT equal: BOOLEAN);
		(* not implemented yet! *)
	END IntersectWith;

	PROCEDURE (v: View) InsertProperties (VAR list: Properties.Property);
		VAR
			prop: CatProp;
	BEGIN
		NEW(prop);
		prop.mode := v.mode;
		prop.orientation := v.orientation;
		(*prop.baseline := v.baseline;*)
		prop.showLine := v.showLine;
		prop.showLabels := v.showLabels;
		prop.ranked := v.ranked;
		prop.logScale := v.logScale;
		prop.valid := propAll;
		prop.known := prop.valid;
		Properties.Insert(list, prop)
	END InsertProperties;

	PROCEDURE (v: View) SetProperties (prop: Properties.Property);
		CONST
			log = "PlotsStdaxis.Log";
			cont = "PlotsStdaxis.Continuous";
			empty = "PlotsEmptyaxis.Empty";
		VAR
			i, len: INTEGER;
			realType: ARRAY 80 OF CHAR;
			xAxis, yAxis: PlotsAxis.Axis;

		PROCEDURE BeginMod;
		BEGIN
			Views.BeginModification(Views.notUndoable, v)
		END BeginMod;

		PROCEDURE EndMod;
		BEGIN
			PlotsViews.IncEra;
			Views.EndModification(Views.notUndoable, v);
			Views.Update(v, Views.keepFrames)
		END EndMod;

	BEGIN
		WITH prop: CatProp DO
			IF propMode IN prop.valid THEN BeginMod; v.mode := prop.mode; EndMod END;
			IF (propOrientation IN prop.valid) & (prop.orientation # v.orientation) THEN
				BeginMod;
				v.orientation := prop.orientation;
				IF v.logScale THEN realType := log ELSE realType := cont END;
				IF v.orientation = barVert THEN
					xAxis := PlotsAxis.New(empty);
					yAxis := PlotsAxis.New(realType)
				ELSE
					xAxis := PlotsAxis.New(realType);
					yAxis := PlotsAxis.New(empty)
				END;
				v.SetXAxis(xAxis);
				v.SetYAxis(yAxis);
				EndMod
			END;
			IF propShowLine IN prop.valid THEN BeginMod; v.showLine := prop.showLine; EndMod END;
			IF propShowLabels IN prop.valid THEN BeginMod; v.showLabels := prop.showLabels; EndMod END;
			IF (propRanked IN prop.valid) & (prop.ranked # v.ranked)THEN
				BeginMod;
				v.ranked := prop.ranked;
				IF v.ranked THEN
					len := LEN(v.mean);
					CASE v.mode OF
					|showMeans: MathSort.Rank(v.mean, len, v.index)
					|showMedians: MathSort.Rank(v.median, len, v.index)
					END
				ELSE
					len := LEN(v.index);
					i := 0;
					WHILE i < len DO
						v.index[i] := i;
						INC(i)
					END
				END;
				EndMod
			END;
			IF (propLogScale IN prop.valid) & (prop.logScale # v.logScale) THEN
				BeginMod;
				v.logScale := prop.logScale;
				IF v.logScale THEN realType := log ELSE realType := cont END;
				IF v.orientation = barVert THEN
					yAxis := PlotsAxis.New(realType);
					v.SetYAxis(yAxis)
				ELSE
					xAxis := PlotsAxis.New(realType);
					v.SetXAxis(xAxis)
				END;
				EndMod
			END
		ELSE
		END
	END SetProperties;

	PROCEDURE (v: View) ExcludeInvalidProps (VAR valid: SET);
	BEGIN
		CASE v.orientation OF
		|barVert: valid := valid - PlotsViews.allXBounds
		|barHoriz: valid := valid - PlotsViews.allYBounds
		END
	END ExcludeInvalidProps;

	PROCEDURE (v: View) ExternalizeData (VAR wr: Stores.Writer);
		VAR
			i, len: INTEGER;
	BEGIN
		wr.WriteInt(v.mode);
		wr.WriteInt(v.orientation);
		wr.WriteReal(v.baseline);
		wr.WriteBool(v.showLine);
		wr.WriteBool(v.showLabels);
		wr.WriteBool(v.ranked);
		wr.WriteBool(v.canLog);
		wr.WriteBool(v.logScale);
		len := LEN(v.label);
		wr.WriteInt(len);
		i := 0;
		WHILE i < len DO
			wr.WriteString(v.label[i]);
			wr.WriteInt(v.index[i]);
			wr.WriteReal(v.mean[i]);
			wr.WriteReal(v.median[i]);
			wr.WriteReal(v.lower[i]);
			wr.WriteReal(v.upper[i]);
			INC(i)
		END
	END ExternalizeData;

	PROCEDURE (v: View) InternalizeData (VAR rd: Stores.Reader);
		VAR
			i, len: INTEGER;
	BEGIN
		rd.ReadInt(v.mode);
		rd.ReadInt(v.orientation);
		rd.ReadReal(v.baseline);
		rd.ReadBool(v.showLine);
		rd.ReadBool(v.showLabels);
		rd.ReadBool(v.ranked);
		rd.ReadBool(v.canLog);
		rd.ReadBool(v.logScale);
		rd.ReadInt(len);
		NEW(v.label, len, lenLabel);
		NEW(v.index, len);
		NEW(v.mean, len);
		NEW(v.median, len);
		NEW(v.lower, len);
		NEW(v.upper, len);
		i := 0;
		WHILE i < len DO
			rd.ReadString(v.label[i]);
			rd.ReadInt(v.index[i]);
			rd.ReadReal(v.mean[i]);
			rd.ReadReal(v.median[i]);
			rd.ReadReal(v.lower[i]);
			rd.ReadReal(v.upper[i]);
			INC(i)
		END
	END InternalizeData;

	PROCEDURE (v: View) CopyDataFrom (source: PlotsViews.View(*Stores.Store*));
		VAR
			i, len: INTEGER;
	BEGIN
		WITH source: View DO
			v.mode := source.mode;
			v.orientation := source.orientation;
			v.baseline := source.baseline;
			v.showLine := source.showLine;
			v.showLabels := source.showLabels;
			v.ranked := source.ranked;
			v.canLog := source.canLog;
			v.logScale := source.logScale;
			len := LEN(source.label);
			NEW(v.label, len, lenLabel);
			NEW(v.index, len);
			NEW(v.mean, len);
			NEW(v.median, len);
			NEW(v.lower, len);
			NEW(v.upper, len);
			i := 0;
			WHILE i < len DO
				v.label[i] := source.label[i]$;
				v.index[i] := source.index[i];
				v.mean[i] := source.mean[i];
				v.median[i] := source.median[i];
				v.lower[i] := source.lower[i];
				v.upper[i] := source.upper[i];
				INC(i)
			END
		END
	END CopyDataFrom;

	PROCEDURE (v: View) DataBounds (OUT minX, maxX, minY, maxY: REAL);
		VAR
			i, len: INTEGER;
			minTemp, maxTemp: REAL;
	BEGIN
		len := LEN(v.label);
		minY := 0; maxY := len;
		minX := v.lower[v.index[len - 1]];
		maxX := v.upper[v.index[len - 1]];
		i := 0;
		WHILE i < len DO
			minX := MIN(minX, v.mean[i]);
			maxX := MAX(maxX, v.mean[i]);
			minX := MIN(minX, v.median[i]);
			maxX := MAX(maxX, v.median[i]);
			minX := MIN(minX, v.lower[i]);
			maxX := MAX(maxX, v.lower[i]);
			minX := MIN(minX, v.upper[i]);
			maxX := MAX(maxX, v.upper[i]);
			INC(i)
		END;
		IF v.orientation = barVert THEN
			minTemp := minX;
			minX := minY;
			minY := minTemp;
			maxTemp := maxX;
			maxX := maxY;
			maxY := maxTemp
		END
	END DataBounds;

	PROCEDURE (v: View) StripLabel (index: INTEGER; OUT string: ARRAY OF CHAR), NEW;
		VAR
			i, j: INTEGER;
	BEGIN
		i := 0;
		WHILE (v.label[index, i] # 0X) & (v.label[index, i] # "[") DO
			INC(i)
		END;
		j := 0;
		WHILE v.label[index, i + j] # 0X DO
			string[j] := v.label[index, i + j];
			INC(j)
		END;
		string[j] := 0X
	END StripLabel;

	PROCEDURE (v: View) RestoreData (f: Views.Frame; l, t, r, b: INTEGER);
		CONST
			pSize = Ports.mm DIV 2;
			thick = Ports.mm DIV 3;
		VAR
			w, h, len, i, index: INTEGER;
			minX, maxX, minY, maxY, pos, point: REAL;
	BEGIN
		v.context.GetSize(w, h);
		v.Bounds(minX, maxX, minY, maxY);
		len := LEN(v.label);
		IF v.showLine THEN
			CASE v.orientation OF
			|barVert:
				IF (v.baseline >= minY) & (v.baseline <= maxY) THEN
					v.DrawLine(f, 0, v.baseline, len, v.baseline, Ports.red, thick, w, h)
				END
			|barHoriz:
				IF (v.baseline >= minX) & (v.baseline <= maxX) THEN
					v.DrawLine(f, v.baseline, 0, v.baseline, len, Ports.red, thick, w, h);
				END
			END
		END;
		i := 0;
		WHILE i < len DO
			index := v.index[i];
			CASE v.mode OF
			|showMeans: point := v.mean[index]
			|showMedians: point := v.median[index]
			END;
			IF v.orientation = barVert THEN
				pos := i + 0.5
			ELSE
				pos := len - i - 0.5
			END;
			CASE v.orientation OF
			|barVert:
				v.DrawPoint(f, pos, point, Ports.black, pSize, Ports.fill, w, h);
				v.DrawLine(f, pos, v.lower[index], pos, v.upper[index], Ports.black, thick, w, h)
			|barHoriz:
				v.DrawPoint(f, point, pos, Ports.black, pSize, Ports.fill, w, h);
				v.DrawLine(f, v.lower[index], pos, v.upper[index], pos, Ports.black, thick, w, h)
			END;
			INC(i)
		END
	END RestoreData;

	PROCEDURE (v: View) DrawKey (f: Views.Frame; font: Fonts.Font; col: INTEGER);
		VAR
			w, h, len, i, asc, dsc, ww, dx, dy, index, stringSize: INTEGER;
			minX, maxX, minY, maxY, pos: REAL;
			string: ARRAY 80 OF CHAR;
	BEGIN
		v.context.GetSize(w, h);
		font.GetBounds(asc, dsc, ww);
		IF v.showLabels THEN
			v.Bounds(minX, maxX, minY, maxY);
			len := LEN(v.label);
			i := 0;
			WHILE i < len DO
				index := v.index[i];
				IF v.orientation = barVert THEN
					pos := i + 0.5
				ELSE
					pos := len - i - 0.5
				END;
				v.StripLabel(index, string);
				CASE v.orientation OF
				|barVert:
					dx := - font.StringWidth(string) DIV 2; dy := dsc + Ports.mm;
					v.DrawString(f, pos, v.upper[index], dx, dy, string, col, font, w, h)
				|barHoriz:
					dx := Ports.mm;
					dy := - (asc + dsc) DIV 3;
					v.DrawString(f, v.upper[index], pos, dx, dy, string, col, font, w, h)
				END;
				INC(i)
			END
		END;
		IF v.showLine THEN
			Strings.RealToStringForm(v.baseline, 3, 0, 0, 0X, string);
			stringSize := font.StringWidth(string);
			CASE v.orientation OF
			|barVert:
				dx := Ports.mm;
				v.DrawString(f, LEN(v.label), v.baseline, dx, 0, string, Ports.red, font, w, h)
			|barHoriz:
				dx := - (stringSize DIV 2);
				dy := asc + dsc;
				v.DrawString(f, v.baseline, 0, dx, - 2 * Ports.mm - dy, string, Ports.red, font, w, h)
			END
		END
	END DrawKey;

	PROCEDURE WriteData (v: View);
		CONST
			cell = 25 * Ports.mm;
			first = 3 * cell DIV 2;
			numTabs = 4;
			width = first + numTabs * cell;
		VAR
			i, index, len: INTEGER;
			f: BugsMappers.Formatter;
			t: TextModels.Model;
			tv: Views.View;
			p: TextRulers.Prop;
	BEGIN
		t := TextModels.dir.New();
		tv := TextViews.dir.New(t);
		BugsTexts.ConnectFormatter(f, t);
		NEW(p);
		p.right := width;
		p.tabs.len := numTabs;
		p.tabs.tab[0].stop := first;
		i := 1;
		WHILE i < numTabs DO
			p.tabs.tab[i].stop := p.tabs.tab[i - 1].stop + cell;
			INC(i)
		END;
		p.valid := {TextRulers.right, TextRulers.tabs};
		f.Bold;
		f.WriteString("node");
		f.WriteTab; f.WriteString("mean");
		f.WriteTab; f.WriteString("median");
		f.WriteTab;
		f.WriteString("2.5%");
		f.WriteTab;
		f.WriteString("97.5%");
		f.WriteLn;
		f.Bold;
		len := LEN(v.label);
		i := 0;
		WHILE i < len DO
			index := v.index[i];
			f.WriteString(v.label[index]);
			f.WriteTab;
			f.WriteReal(v.mean[index]);
			f.WriteTab;
			f.WriteReal(v.median[index]);
			f.WriteTab;
			f.WriteReal(v.lower[index]);
			f.WriteTab;
			f.WriteReal(v.upper[index]);
			f.WriteLn;
			INC(i)
		END;
		Views.OpenAux(tv, "Caterpillar plot data")
	END WriteData;

	PROCEDURE (v: View) HandleDataCtrlMsg (f: Views.Frame; VAR msg: Controllers.Message;
	VAR focus: Views.View);
		VAR
			isDown: BOOLEAN;
			x0, y0, x, y, w, h: INTEGER;
			buttons: SET;
	BEGIN
		WITH msg: Controllers.TrackMsg DO
			x0 := msg.x; y0 := msg.y;
			REPEAT
				f.Input(x, y, buttons, isDown);
				IF (x # x0) OR (y # y0) THEN x0 := x; y0 := y END
			UNTIL ~isDown;
			v.context.GetSize(w, h);
			IF (x0 < w) & (y0 < h) & (Controllers.modify IN buttons) THEN
				WriteData(v)
			END
		ELSE
		END
	END HandleDataCtrlMsg;

	PROCEDURE (f: Factory) New (IN name: ARRAY OF CHAR;
	IN label: ARRAY OF ARRAY OF CHAR;
	IN mean: ARRAY OF REAL;
	IN percentiles: ARRAY OF ARRAY OF REAL;
	IN start, sampleSize: ARRAY OF INTEGER;
	other, axis: POINTER TO ARRAY OF REAL): PlotsViews.View;
		VAR
			i, len: INTEGER;
			sum, minX, maxX, minY, maxY: REAL;
			v: View;
			xAxis, yAxis: PlotsAxis.Axis;
	BEGIN
		len := LEN(label);
		NEW(v);
		v.mode := showMeans;
		v.orientation := barHoriz;
		v.showLine := TRUE;
		v.showLabels := TRUE;
		v.ranked := FALSE;
		v.logScale := FALSE;
		NEW(v.label, len, lenLabel);
		NEW(v.index, len);
		NEW(v.mean, len);
		NEW(v.median, len);
		NEW(v.lower, len);
		NEW(v.upper, len);
		i := 0;
		sum := 0;
		WHILE i < len DO
			v.index[i] := i;
			v.label[i] := label[i]$;
			v.mean[i] := mean[i];
			sum := sum + v.mean[i];
			v.median[i] := percentiles[i, 0];
			v.lower[i] := percentiles[i, 1];
			v.upper[i] := percentiles[i, 2];
			INC(i)
		END;
		v.baseline := sum / len;
		v.DataBounds(minX, maxX, minY, maxY);
		v.canLog := minX > 0;
		v.Init;
		v.SetSizes(minW, minH, left, top, right, bottom);
		xAxis := PlotsAxis.New("PlotsStdaxis.Continuous");
		xAxis.SetTitle(name);
		v.SetXAxis(xAxis);
		yAxis := PlotsAxis.New("PlotsEmptyaxis.Empty");
		v.SetYAxis(yAxis);
		v.SetTitle("caterpillar plot: " + name);
		RETURN v
	END New;

	PROCEDURE (f: Factory) Title (OUT title: ARRAY OF CHAR);
	BEGIN
		title := "#Compare:Caterpillar plot"
	END Title;

	PROCEDURE IsView (v: PlotsViews.View): BOOLEAN;
	BEGIN
		IF v # NIL THEN RETURN v IS View
		ELSE RETURN FALSE
		END
	END IsView;

	PROCEDURE Guard* (VAR par: Dialog.Par);
		VAR
			v: PlotsViews.View;
	BEGIN
		v := PlotsDialog.Singleton(); par.disabled := ~IsView(v)
	END Guard;

	PROCEDURE GuardBaseline* (VAR par: Dialog.Par);
		VAR
			v: PlotsViews.View;
	BEGIN
		v := PlotsDialog.Singleton(); par.disabled := ~IsView(v);
		IF ~par.disabled THEN
			WITH v: View DO par.disabled := ~v.showLine END
		END
	END GuardBaseline;

	PROCEDURE GuardLog* (VAR par: Dialog.Par);
		VAR
			v: PlotsViews.View;
	BEGIN
		v := PlotsDialog.Singleton(); par.disabled := ~IsView(v);
		IF ~par.disabled THEN
			WITH v: View DO par.disabled := ~v.canLog END
		END
	END GuardLog;

	PROCEDURE Notifier* (op, from, to: INTEGER);
		VAR
			v: PlotsViews.View;
	BEGIN
		IF op = Dialog.changed THEN
			v := PlotsDialog.Singleton();
			Controllers.SetCurrentPath(Controllers.targetPath);
			Properties.EmitProp(NIL, catDialog);
			Controllers.ResetCurrentPath
		END
	END Notifier;

	PROCEDURE (s: SpecialDialog) Dialog (): Views.View;
		VAR
			loc: Files.Locator;
	BEGIN
		IF dialogView = NIL THEN
			loc := Files.dir.This("Compare/Rsrc");
			dialogView := Views.OldView(loc, "CatDialog");
		END;
		RETURN dialogView
	END Dialog;

	PROCEDURE (s: SpecialDialog) Set (prop: Properties.Property);
	BEGIN
		WITH prop: CatProp DO
			catDialog^ := prop^;
			Dialog.Update(catDialog)
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
			f: Factory; fractions: POINTER TO ARRAY OF REAL;
	BEGIN
		Maintainer;
		NEW(catDialog);
		catDialog.mode := showMeans;
		catDialog.orientation := barHoriz;
		catDialog.showLine := TRUE;
		catDialog.showLabels := TRUE;
		catDialog.ranked := FALSE;
		catDialog.logScale := FALSE;
		catDialog.valid := propAll;
		NEW(f);
		NEW(fractions, numFract);
		fractions[0] := 50;
		fractions[1] := 2.5;
		fractions[2] := 97.5;
		f.SetFractions(fractions); f
		.SetArguments({CompareViews.node});
		fact := f;
		dialogView := NIL;
		NEW(special)
	END Init;

BEGIN
	Init
END CompareCaterpillar.
