(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		 *)

MODULE CompareBoxplot;


	

	IMPORT
		Controllers, Dialog, Files, Fonts, Ports, Properties, Stores, Strings, Views,
		TextModels, TextRulers, TextViews, 
		BugsMappers, BugsTexts, 
		CompareViews,
		MathSort, 
		PlotsAxis, PlotsDialog, PlotsViews;


	CONST
		numFract = 5;
		minW = 100 * Ports.mm; minH = 100 * Ports.mm;
		left = 10 * Ports.mm; top = 5 * Ports.mm; right = 0 * Ports.mm; bottom = 10 * Ports.mm;
		showMeans = 0; showMedians = 1; boxVert = 0; boxHoriz = 1;
		propMode = 0; propBaseline = 1; propShowLine = 2; propShowLabels = 3;
		propRanked = 4; propLogScale = 5; propOrientation = 6; propBoxColour = 7;
		propAll = {propMode..propBoxColour};
		labelLen = 128;

	TYPE
		View = POINTER TO RECORD (PlotsViews.View)
			mode, orientation, boxColour: INTEGER;
			baseline: REAL;
			showLine, showLabels, ranked, canLog, logScale: BOOLEAN;
			label: POINTER TO ARRAY OF ARRAY OF CHAR;
			index: POINTER TO ARRAY OF INTEGER;
			mean, median, bottom, lower, upper, top: POINTER TO ARRAY OF REAL
		END;

		Factory = POINTER TO RECORD (CompareViews.Factory) END;

		BoxProp* = POINTER TO RECORD (Properties.Property)
			mode*, orientation*, boxColour*: INTEGER;
			showLine*, showLabels*, ranked*, logScale*: BOOLEAN
		END;

		SpecialDialog = POINTER TO RECORD (PlotsDialog.SpecialDialog) END;

	VAR
		boxDialog*: BoxProp;
		fact: CompareViews.Factory;
		dialogView: Views.View;
		special: SpecialDialog;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (p: BoxProp) IntersectWith* (q: Properties.Property; OUT equal: BOOLEAN);
		(* not implemented yet! *)
	END IntersectWith;

	PROCEDURE (v: View) InsertProperties (VAR list: Properties.Property);
		VAR
			prop: BoxProp;
	BEGIN
		NEW(prop);
		prop.mode := v.mode;
		prop.orientation := v.orientation;
		prop.boxColour := v.boxColour;
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
		WITH prop: BoxProp DO
			IF propMode IN prop.valid THEN
				BeginMod;
				v.mode := prop.mode;
				EndMod
			END;
			IF (propOrientation IN prop.valid) & (prop.orientation # v.orientation) THEN
				BeginMod;
				v.orientation := prop.orientation;
				IF v.logScale THEN realType := log ELSE realType := cont END;
				IF v.orientation = boxVert THEN
					xAxis := PlotsAxis.New(empty);
					v.SetXAxis(xAxis);
					yAxis := PlotsAxis.New(realType);
					v.SetYAxis(yAxis)
				ELSE
					xAxis := PlotsAxis.New(realType);
					v.SetXAxis(xAxis);
					yAxis := PlotsAxis.New(empty);
					v.SetYAxis(yAxis)
				END;
				EndMod
			END;
			IF propBoxColour IN prop.valid THEN
				BeginMod;
				v.boxColour := prop.boxColour;
				EndMod
			END;
			IF propShowLine IN prop.valid THEN
				BeginMod;
				v.showLine := prop.showLine;
				EndMod
			END;
			IF propShowLabels IN prop.valid THEN
				BeginMod;
				v.showLabels := prop.showLabels;
				EndMod
			END;
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
				IF v.orientation = boxVert THEN
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
		|boxVert: valid := valid - PlotsViews.allXBounds
		|boxHoriz: valid := valid - PlotsViews.allYBounds
		END
	END ExcludeInvalidProps;

	PROCEDURE (v: View) ExternalizeData (VAR wr: Stores.Writer);
		VAR
			i, len: INTEGER;
	BEGIN
		wr.WriteInt(v.mode);
		wr.WriteInt(v.orientation);
		wr.WriteInt(v.boxColour);
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
			wr.WriteReal(v.bottom[i]);
			wr.WriteReal(v.lower[i]);
			wr.WriteReal(v.upper[i]);
			wr.WriteReal(v.top[i]);
			INC(i)
		END
	END ExternalizeData;

	PROCEDURE (v: View) InternalizeData (VAR rd: Stores.Reader);
		VAR
			i, len: INTEGER;
	BEGIN
		rd.ReadInt(v.mode);
		rd.ReadInt(v.orientation);
		rd.ReadInt(v.boxColour);
		rd.ReadReal(v.baseline);
		rd.ReadBool(v.showLine);
		rd.ReadBool(v.showLabels);
		rd.ReadBool(v.ranked);
		rd.ReadBool(v.canLog);
		rd.ReadBool(v.logScale);
		rd.ReadInt(len);
		NEW(v.label, len, labelLen);
		NEW(v.index, len);
		NEW(v.mean, len);
		NEW(v.median, len);
		NEW(v.bottom, len);
		NEW(v.lower, len);
		NEW(v.upper, len);
		NEW(v.top, len);
		i := 0;
		WHILE i < len DO
			rd.ReadString(v.label[i]);
			rd.ReadInt(v.index[i]);
			rd.ReadReal(v.mean[i]);
			rd.ReadReal(v.median[i]);
			rd.ReadReal(v.bottom[i]);
			rd.ReadReal(v.lower[i]);
			rd.ReadReal(v.upper[i]);
			rd.ReadReal(v.top[i]);
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
			v.boxColour := source.boxColour;
			v.baseline := source.baseline;
			v.showLine := source.showLine;
			v.showLabels := source.showLabels;
			v.ranked := source.ranked;
			v.canLog := source.canLog;
			v.logScale := source.logScale;
			len := LEN(source.label);
			NEW(v.label, len, labelLen);
			NEW(v.index, len);
			NEW(v.mean, len);
			NEW(v.median, len);
			NEW(v.bottom, len);
			NEW(v.lower, len);
			NEW(v.upper, len);
			NEW(v.top, len);
			i := 0;
			WHILE i < len DO
				v.label[i] := source.label[i]$;
				v.index[i] := source.index[i];
				v.mean[i] := source.mean[i];
				v.median[i] := source.median[i];
				v.bottom[i] := source.bottom[i];
				v.lower[i] := source.lower[i];
				v.upper[i] := source.upper[i];
				v.top[i] := source.top[i];
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
		minX := 0;
		maxX := len;
		minY := v.bottom[v.index[len - 1]];
		maxY := v.top[v.index[len - 1]];
		i := 0;
		WHILE i < len DO
			minY := MIN(minY, v.mean[i]);
			maxY := MAX(maxY, v.mean[i]);
			minY := MIN(minY, v.median[i]);
			maxY := MAX(maxY, v.median[i]);
			minY := MIN(minY, v.bottom[i]);
			maxY := MAX(maxY, v.bottom[i]);
			minY := MIN(minY, v.lower[i]);
			maxY := MAX(maxY, v.lower[i]);
			minY := MIN(minY, v.upper[i]);
			maxY := MAX(maxY, v.upper[i]);
			minY := MIN(minY, v.top[i]);
			maxY := MAX(maxY, v.top[i]);
			INC(i)
		END;
		IF v.orientation = boxHoriz THEN
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
			thick = Ports.mm DIV 3;
			width = 0.8;
			shrink = 0.75;
		VAR
			w, h, len, i, index: INTEGER;
			minX, maxX, minY, maxY, clipX, clipY, left, right, bottom, lower, middle,
			upper, top, pos, barLeft, barRight: REAL;
	BEGIN
		v.context.GetSize(w, h);
		v.Bounds(minX, maxX, minY, maxY);
		v.Translate(f, minX, minY, thick DIV 2, thick DIV 2, w, h, clipX, clipY);
		len := LEN(v.label);
		IF v.showLine THEN
			CASE v.orientation OF
			|boxVert:
				IF (v.baseline >= minY) & (v.baseline <= maxY) THEN
					v.DrawLine(f, 0, v.baseline, len, v.baseline, Ports.black, thick, w, h)
				END
			|boxHoriz:
				IF (v.baseline >= minX) & (v.baseline <= maxX) THEN
					v.DrawLine(f, v.baseline, 0, v.baseline, len, Ports.black, thick, w, h)
				END
			END
		END;
		i := 0;
		WHILE i < len DO
			index := v.index[i];
			bottom := v.bottom[index];
			lower := v.lower[index];
			upper := v.upper[index];
			top := v.top[index];
			CASE v.mode OF
			|showMeans:
				middle := v.mean[index]
			|showMedians:
				middle := v.median[index]
			END;
			IF v.orientation = boxVert THEN
				pos := i + 0.5
			ELSE
				pos := len - i - 0.5
			END;
			left := pos - width / 2;
			right := pos + width / 2;
			barLeft := pos - shrink * width / 2;
			barRight := pos + shrink * width / 2;
			CASE v.orientation OF
			|boxVert:
				v.DrawRectangle(f, left, upper, right, lower, v.boxColour, Ports.fill, w, h);
				v.DrawRectangle(f, left, upper, right, lower, Ports.black, thick, w, h);
				v.DrawLine(f, left + clipX, middle, right - clipX, middle, Ports.black, thick, w, h);
				v.DrawLine(f, pos, lower, pos, bottom, Ports.black, thick, w, h);
				v.DrawLine(f, pos, upper, pos, top, Ports.black, thick, w, h);
				v.DrawLine(f, barLeft, bottom, barRight, bottom, Ports.black, thick, w, h);
				v.DrawLine(f, barLeft, top, barRight, top, Ports.black, thick, w, h)
			|boxHoriz:
				v.DrawRectangle(f, lower, right, upper, left, v.boxColour, Ports.fill, w, h);
				v.DrawRectangle(f, lower, right, upper, left, Ports.black, thick, w, h);
				v.DrawLine(f, middle, right + clipY, middle, left - clipY, Ports.black, thick, w, h);
				v.DrawLine(f, lower, pos, bottom, pos, Ports.black, thick, w, h);
				v.DrawLine(f, upper, pos, top, pos, Ports.black, thick, w, h);
				v.DrawLine(f, bottom, barRight, bottom, barLeft, Ports.black, thick, w, h);
				v.DrawLine(f, top, barRight, top, barLeft, Ports.black, thick, w, h)
			END;
			INC(i)
		END
	END RestoreData;

	PROCEDURE (v: View) DrawKey (f: Views.Frame; font: Fonts.Font; col: INTEGER);
		CONST
			thick = Ports.mm DIV 3;
		VAR
			w, h, len, i, asc, dsc, ww, dx, dy, index, stringSize: INTEGER;
			minX, maxX, minY, maxY, clipX, clipY, pos: REAL;
			string: ARRAY 80 OF CHAR;
	BEGIN
		v.context.GetSize(w, h);
		font.GetBounds(asc, dsc, ww);
		IF v.showLabels THEN
			v.Bounds(minX, maxX, minY, maxY);
			v.Translate(f, minX, minY, thick DIV 2, thick DIV 2, w, h, clipX, clipY);
			len := LEN(v.label);
			i := 0;
			WHILE i < len DO
				index := v.index[i];
				IF v.orientation = boxVert THEN
					pos := i + 0.5
				ELSE
					pos := len - i - 0.5
				END;
				v.StripLabel(index, string);
				CASE v.orientation OF
				|boxVert:
					dx := - font.StringWidth(string) DIV 2;
					dy := dsc + Ports.mm;
					v.DrawString(f, pos, v.top[index], dx, dy, string, col, font, w, h)
				|boxHoriz:
					dx := Ports.mm;
					dy := - (asc + dsc) DIV 3;
					v.DrawString(f, v.top[index], pos, dx, dy, string, col, font, w, h)
				END;
				INC(i)
			END
		END;
		IF v.showLine THEN
			Strings.RealToStringForm(v.baseline, 3, 0, 0, 0X, string);
			stringSize := font.StringWidth(string);
			CASE v.orientation OF
			|boxVert:
				dx := Ports.mm;
				v.DrawString(f, LEN(v.label), v.baseline, dx, 0, string, Ports.red, font, w, h)
			|boxHoriz:
				dx := - (stringSize DIV 2);
				dy := asc + dsc;
				v.DrawString(f, v.baseline, 0, dx, - 2 * Ports.mm - dy, string, Ports.red, font, w, h)
			END
		END
	END DrawKey;

	PROCEDURE WriteData (v: View);
		CONST
			cell = 22 * Ports.mm;
			first = 3 * cell DIV 2;
			numTabs = 6;
			width = first + numTabs * cell;
		VAR
			i, len, index: INTEGER;
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
		f.WriteTab;
		f.WriteString("mean");
		f.WriteTab;
		f.WriteString("median");
		f.WriteTab;
		f.WriteString("2.5%");
		f.WriteTab;
		f.WriteString("25%");
		f.WriteTab;
		f.WriteString("75%");
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
			f.WriteReal(v.bottom[index]);
			f.WriteTab;
			f.WriteReal(v.lower[index]);
			f.WriteTab;
			f.WriteReal(v.upper[index]);
			f.WriteTab;
			f.WriteReal(v.top[index]);
			f.WriteLn;
			INC(i)
		END;
		Views.OpenAux(tv, "Box plot data")
	END WriteData;

	PROCEDURE (v: View) HandleDataCtrlMsg (f: Views.Frame; VAR msg: Controllers.Message;
	VAR focus: Views.View);
		VAR
			isDown: BOOLEAN;
			x0, y0, x, y, w, h: INTEGER;
			buttons: SET;
	BEGIN
		WITH msg: Controllers.TrackMsg DO
			x0 := msg.x;
			y0 := msg.y;
			REPEAT
				f.Input(x, y, buttons, isDown);
				IF (x # x0) OR (y # y0) THEN
					x0 := x;
					y0 := y
				END
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
			means, medians: POINTER TO ARRAY OF REAL;
			v: View;
			xAxis, yAxis: PlotsAxis.Axis;
	BEGIN
		len := LEN(label);
		NEW(v);
		v.mode := showMeans;
		v.orientation := boxVert;
		v.boxColour := Ports.green;
		v.showLine := TRUE;
		v.showLabels := TRUE;
		v.ranked := FALSE;
		v.logScale := FALSE;
		NEW(v.label, len, labelLen);
		NEW(v.index, len);
		NEW(v.mean, len);
		NEW(v.median, len);
		NEW(v.bottom, len);
		NEW(v.lower, len);
		NEW(v.upper, len);
		NEW(v.top, len);
		NEW(means, len);
		NEW(medians, len);
		i := 0;
		sum := 0;
		WHILE i < len DO
			v.index[i] := i;
			v.label[i] := label[i]$;
			v.mean[i] := mean[i];
			sum := sum + v.mean[i];
			v.bottom[i] := percentiles[i, 0];
			v.lower[i] := percentiles[i, 1];
			v.median[i] := percentiles[i, 2];
			v.upper[i] := percentiles[i, 3];
			v.top[i] := percentiles[i, 4];
			INC(i)
		END;
		v.baseline := sum / len;
		i := 0;
		WHILE i < len DO
			means[i] := v.mean[i];
			medians[i] := v.median[i];
			INC(i)
		END;
		v.DataBounds(minX, maxX, minY, maxY);
		v.canLog := minY > 0;
		v.Init;
		v.SetSizes(minW, minH, left, top, right, bottom);
		xAxis := PlotsAxis.New("PlotsEmptyaxis.Empty");
		v.SetXAxis(xAxis);
		yAxis := PlotsAxis.New("PlotsStdaxis.Continuous");
		yAxis.SetTitle(name);
		v.SetYAxis(yAxis);
		v.SetTitle("box plot: " + name);
		RETURN v
	END New;

	PROCEDURE (f: Factory) Title (OUT title: ARRAY OF CHAR);
	BEGIN
		title := "#Compare:Box plot"
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
		v := PlotsDialog.Singleton();
		par.disabled := ~IsView(v)
	END Guard;

	PROCEDURE GuardBaseline* (VAR par: Dialog.Par);
		VAR
			v: PlotsViews.View;
	BEGIN
		v := PlotsDialog.Singleton();
		par.disabled := ~IsView(v);
		IF ~par.disabled THEN
			WITH v: View DO
				par.disabled := ~v.showLine
			END
		END
	END GuardBaseline;

	PROCEDURE GuardLog* (VAR par: Dialog.Par);
		VAR
			v: PlotsViews.View;
	BEGIN
		v := PlotsDialog.Singleton();
		par.disabled := ~IsView(v);
		IF ~par.disabled THEN
			WITH v: View DO
				par.disabled := ~v.canLog
			END
		END
	END GuardLog;

	PROCEDURE Notifier* (op, from, to: INTEGER);
		VAR
			v: PlotsViews.View;
	BEGIN
		IF op = Dialog.changed THEN
			v := PlotsDialog.Singleton();
			Controllers.SetCurrentPath(Controllers.targetPath);
			Properties.EmitProp(NIL, boxDialog);
			Controllers.ResetCurrentPath
		END
	END Notifier;

	PROCEDURE (s: SpecialDialog) Dialog (): Views.View;
		VAR
			loc: Files.Locator;
	BEGIN
		IF dialogView = NIL THEN
			loc := Files.dir.This("Compare/Rsrc");
			dialogView := Views.OldView(loc, "BoxDialog");
		END;
		RETURN dialogView
	END Dialog;

	PROCEDURE (s: SpecialDialog) Set (prop: Properties.Property);
	BEGIN
		WITH prop: BoxProp DO
			boxDialog^ := prop^;
			Dialog.Update(boxDialog)
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
	BEGIN
		Maintainer;
		NEW(boxDialog);
		boxDialog.mode := showMeans;
		boxDialog.orientation := boxVert;
		boxDialog.boxColour := Ports.green;
		boxDialog.showLine := TRUE;
		boxDialog.showLabels := TRUE;
		boxDialog.ranked := FALSE;
		boxDialog.logScale := FALSE;
		boxDialog.valid := propAll;
		NEW(f);
		NEW(fractions, numFract);
		fractions[0] := 2.5;
		fractions[1] := 25;
		fractions[2] := 50;
		fractions[3] := 75;
		fractions[4] := 97.5;
		f.SetFractions(fractions);
		f.SetArguments({CompareViews.node});
		fact := f;
		dialogView := NIL;
		NEW(special)
	END Init;

BEGIN
	Init
END CompareBoxplot.
