(*	

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE CompareDenstrip;

	

	IMPORT
		Controllers, Dialog, Files, Fonts, Math,
		Ports, Properties, Stores, Strings, Views, StdCmds,
		TextMappers, TextModels, TextRulers, TextViews, CompareViews,
		MathSmooth, MathSort, PlotsAxis, PlotsDialog, PlotsViews;

	CONST
		numFract = 5;
		(* blue code is different from boxplot *)
		minW = 100 * Ports.mm; minH = 100 * Ports.mm;
		left = 10 * Ports.mm; top = 5 * Ports.mm; right = 0 * Ports.mm; bottom = 10 * Ports.mm;
		showMeans = 0; showMedians = 1; stripVert = 0; stripHoriz = 1;
		propMode = 0; propBaseline = 1;
		propShowLine = 2; propShowLabels = 3; propShowPoint = 4; propShow50 = 5; propShow95 = 6;
		propRanked = 7; propLogScale = 8; propOrientation = 9; propSmooth = 10; propGamma = 11;
		propStripColour = 12;
		propAll = {propMode..propStripColour};
		labelLen = 128;

	TYPE
		View = POINTER TO RECORD (PlotsViews.View)
			(* properties *)
			mode, orientation, stripColour: INTEGER;
			baseline, smooth, gamma: REAL;
			showLine, showLabels, showPoint, show50, show95, ranked, canLog, logScale: BOOLEAN;
			(* data *)
			label: POINTER TO ARRAY OF ARRAY OF CHAR;
			index: POINTER TO ARRAY OF INTEGER;
			mean, median, bottom, lower, upper, top: POINTER TO ARRAY OF REAL;
			sample: POINTER TO ARRAY OF POINTER TO ARRAY OF REAL;
			sampleSize: POINTER TO ARRAY OF INTEGER;
			densX: POINTER TO ARRAY OF POINTER TO ARRAY OF REAL;
			densY: POINTER TO ARRAY OF POINTER TO ARRAY OF REAL
		END;

		List = POINTER TO RECORD
			sample: POINTER TO ARRAY OF REAL;
			next: List
		END;

		Factory = POINTER TO RECORD (CompareViews.Factory)
			sampleList: List
		END;

		DensProp* = POINTER TO RECORD (Properties.Property)
			mode*, orientation*, stripColour*: INTEGER; 
			baseline*, smooth*, gamma*: REAL;
			showLine*, showLabels*, showPoint*, show50*, show95*, ranked*, logScale*: BOOLEAN
		END;

		SpecialDialog = POINTER TO RECORD (PlotsDialog.SpecialDialog) END;
		SpecialApplyDialog = POINTER TO RECORD (PlotsDialog.SpecialApplyDialog) END;

	VAR
		densDialog*: DensProp;
		fact: CompareViews.Factory;
		dialogView: Views.View;
		special: SpecialDialog;
		specialApply: SpecialApplyDialog;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		(* BuildDensity, BuildHistogram copied from SamplesDensity with changed input and output types *)

		PROCEDURE BuildDensity (sample: POINTER TO ARRAY OF REAL; sampleSize: INTEGER;
		smooth: REAL;
		OUT densX: POINTER TO ARRAY OF REAL;
	OUT densY: POINTER TO ARRAY OF REAL);
		VAR
			numBins, m: INTEGER; bandWidth, delta, minX: REAL;
			i: INTEGER;
			weights: ARRAY 10 OF REAL;
	BEGIN
		bandWidth := MathSmooth.BandWidth(sample, sampleSize, smooth);
		MathSmooth.TriWeight(weights);
		m := LEN(weights);
		MathSmooth.BinInfo(sample, bandWidth, sampleSize, m, minX, delta, numBins);
		NEW(densX, numBins);
		NEW(densY, numBins);
		MathSmooth.Smooth(sample, weights, sampleSize, numBins, m, bandWidth, minX, delta, densY);
		i := 0;
		WHILE i < numBins DO
			densX[i] := minX + delta * i;
			INC(i)
		END
	END BuildDensity;

	(* Standard deviation of a sample *)
	PROCEDURE Stdev (sample: POINTER TO ARRAY OF REAL; sampleSize: INTEGER): REAL;
		VAR
			i: INTEGER; sum, sumsq: REAL;
	BEGIN
		i := 0; sum := 0; sumsq := 0;
		WHILE i < sampleSize DO
			sum := sum + sample[i];
			sumsq := sumsq + sample[i] * sample[i];
			INC(i)
		END;
		sum := sum / sampleSize;
		sumsq := sumsq / sampleSize;
		RETURN Math.Sqrt(sumsq - sum * sum)
	END Stdev;

	PROCEDURE BuildHistogram (sample: POINTER TO ARRAY OF REAL; sampleSize: INTEGER; smooth: REAL;
	OUT densX: POINTER TO ARRAY OF REAL;
	OUT densY: POINTER TO ARRAY OF REAL);
		CONST
			eps = 1.0E-10;
		VAR
			minX, maxX: REAL;
			binSize, bin, i, j, rangeX, numBins, xMin, start, rem, extra, outer, inner: INTEGER;
			density: POINTER TO ARRAY OF REAL;
	BEGIN
		minX := MathSmooth.Min(sample, sampleSize); maxX := MathSmooth.Max(sample, sampleSize);
		rangeX := SHORT(ENTIER(maxX - minX + eps)) + 1;
		NEW(density, rangeX);
		(*
		binSize = sd(x) / n ^ smooth
		varies between sd (for smooth = 0, very smooth)  and sd / n (for smooth = 1, very lumpy)
		*)
		binSize := MAX(SHORT(ENTIER(Stdev(sample, sampleSize) / Math.Power(sampleSize, smooth))), 1);
		i := 0; WHILE i < rangeX DO density[i] := 0; INC(i) END;
		i := 0;
		WHILE i < sampleSize DO
			bin := SHORT(ENTIER(sample[i] - minX + eps)); density[bin] := density[bin] + 1;
			INC(i)
		END;
		i := 0; WHILE i < rangeX DO density[i] := density[i] / sampleSize; INC(i) END;
		xMin := SHORT(ENTIER(minX + eps));
		rem := rangeX MOD binSize;
		IF rem = 0 THEN numBins := rangeX DIV binSize; extra := 0
		ELSE numBins := rangeX DIV binSize + 1; extra := binSize - rem
		END;
		NEW(densY, numBins + 1);
		NEW(densX, numBins + 1);
		start := xMin - extra DIV 2;
		i := 0; WHILE i < numBins DO densY[i] := 0; INC(i) END;
		i := 0; outer := start; inner := xMin; bin := 0;
		WHILE i < numBins DO
			j := 0;
			WHILE j < binSize DO
				IF outer = inner THEN
					IF bin < rangeX THEN
						densY[i] := densY[i] + density[bin];
						INC(inner); INC(bin)
					END
				END;
				INC(outer);
				INC(j)
			END;
			densY[i] := densY[i] / binSize;
			densX[i] := start + i * binSize - 0.5;
			INC(i)
		END;
		densY[numBins] := 0;
		densX[numBins] := start + numBins * binSize - 0.5 - extra
	END BuildHistogram;

	PROCEDURE (p: DensProp) IntersectWith* (q: Properties.Property; OUT equal: BOOLEAN);
		(* not implemented yet! *)
	END IntersectWith;

	PROCEDURE (v: View) InsertProperties (VAR list: Properties.Property);
		VAR
			prop: DensProp;
	BEGIN
		NEW(prop);
		prop.mode := v.mode;
		prop.orientation := v.orientation; prop.stripColour := v.stripColour;
		prop.baseline := v.baseline; prop.smooth := v.smooth; prop.gamma := v.gamma;
		prop.showLine := v.showLine;
		prop.showLabels := v.showLabels; prop.showPoint := v.showPoint;
		prop.show50 := v.show50; prop.show95 := v.show95;
		prop.ranked := v.ranked; prop.logScale := v.logScale;
		prop.valid := propAll; prop.known := prop.valid;
		Properties.Insert(list, prop)
	END InsertProperties;

	PROCEDURE (v: View) SetProperties (prop: Properties.Property);
		CONST
			log = "PlotsStdaxis.Log"; cont = "PlotsStdaxis.Continuous"; empty = "PlotsEmptyaxis.Empty";
		VAR
			realAxis, emptyAxis: CHAR; realType: ARRAY 80 OF CHAR; i, len: INTEGER;
			xAxis, yAxis: PlotsAxis.Axis;

		PROCEDURE BeginMod;
		BEGIN
			Views.BeginModification(Views.notUndoable, v)
		END BeginMod;

		PROCEDURE EndMod;
		BEGIN
			PlotsViews.IncEra; Views.EndModification(Views.notUndoable, v); 
			Views.Update(v, Views.keepFrames)
		END EndMod;

	BEGIN
		WITH prop: DensProp DO
			IF propMode IN prop.valid THEN BeginMod; v.mode := prop.mode; EndMod END;
			IF (propOrientation IN prop.valid) & (prop.orientation # v.orientation) THEN
				BeginMod;
				v.orientation := prop.orientation;
				IF v.logScale THEN realType := log ELSE realType := cont END;
				IF v.orientation = stripVert THEN
					xAxis := PlotsAxis.New(empty);
					yAxis := PlotsAxis.New(realType);
				ELSE
					xAxis := PlotsAxis.New(realType);
					yAxis := PlotsAxis.New(empty)
				END;
				v.SetXAxis(xAxis);
				v.SetYAxis(yAxis);
				EndMod
			END;
			IF propStripColour IN prop.valid THEN BeginMod; v.stripColour := prop.stripColour; EndMod END;
			IF propBaseline IN prop.valid THEN BeginMod; v.baseline := prop.baseline; EndMod END;
			IF propShowLine IN prop.valid THEN BeginMod; v.showLine := prop.showLine; EndMod END;
			IF propShowLabels IN prop.valid THEN BeginMod; v.showLabels := prop.showLabels; EndMod END;
			IF propShowPoint IN prop.valid THEN BeginMod; v.showPoint := prop.showPoint; EndMod END;
			IF propShow50 IN prop.valid THEN BeginMod; v.show50 := prop.show50; EndMod END;
			IF propShow95 IN prop.valid THEN BeginMod; v.show95 := prop.show95; EndMod END;
			IF propRanked IN prop.valid THEN
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
				IF v.orientation = stripVert THEN
					yAxis := PlotsAxis.New(realType);
					v.SetYAxis(yAxis)
				ELSE
					xAxis := PlotsAxis.New(realType);
					v.SetXAxis(xAxis)
				END;
				EndMod
			END;
			(* strips don't update when new value for smooth is typed, unless apply button is clicked,
			since recalculating density may be slow for large samples *)
			IF (propSmooth IN prop.valid) THEN BeginMod; v.smooth := prop.smooth; EndMod END;
			IF (propGamma IN prop.valid) & (prop.gamma > 0) THEN BeginMod; v.gamma := prop.gamma; EndMod END
		END
	END SetProperties;

	PROCEDURE (v: View) ExcludeInvalidProps (VAR valid: SET);
	BEGIN
		CASE v.orientation OF
		|stripVert: valid := valid - PlotsViews.allXBounds
		|stripHoriz: valid := valid - PlotsViews.allYBounds
		END
	END ExcludeInvalidProps;

	PROCEDURE (v: View) ExternalizeData (VAR wr: Stores.Writer);
		VAR
			len, i, j: INTEGER;
			lenDens: POINTER TO ARRAY OF INTEGER;
	BEGIN
		wr.WriteInt(v.mode);
		wr.WriteInt(v.orientation); wr.WriteInt(v.stripColour);
		wr.WriteReal(v.baseline); wr.WriteReal(v.smooth); wr.WriteReal(v.gamma); wr.WriteBool(v.showLine);
		wr.WriteBool(v.showPoint); wr.WriteBool(v.show50); wr.WriteBool(v.show95);
		wr.WriteBool(v.showLabels); wr.WriteBool(v.ranked);
		wr.WriteBool(v.canLog); wr.WriteBool(v.logScale);
		len := LEN(v.label);
		wr.WriteInt(len);
		NEW(lenDens, len);
		i := 0;
		WHILE i < len DO
			wr.WriteString(v.label[i]);
			wr.WriteInt(v.index[i]);
			wr.WriteReal(v.mean[i]); wr.WriteReal(v.median[i]);
			wr.WriteReal(v.bottom[i]); wr.WriteReal(v.lower[i]); wr.WriteReal(v.upper[i]); wr.WriteReal(v.top[i]);
			wr.WriteInt(v.sampleSize[i]);
			lenDens[i] := LEN(v.densX[i]);
			wr.WriteInt(lenDens[i]);
			j := 0;
			WHILE j < lenDens[i] DO
				wr.WriteReal(v.densX[i, j]);
				wr.WriteReal(v.densY[i, j]);
				INC(j)
			END;
			j := 0;
			WHILE j < v.sampleSize[i] DO
				wr.WriteReal(v.sample[i, j]);
				INC(j)
			END;
			INC(i)
		END
	END ExternalizeData;

	PROCEDURE (v: View) InternalizeData (VAR rd: Stores.Reader);
		VAR
			len, i, j: INTEGER;
			lenDens: POINTER TO ARRAY OF INTEGER;
	BEGIN
		rd.ReadInt(v.mode);
		rd.ReadInt(v.orientation); rd.ReadInt(v.stripColour);
		rd.ReadReal(v.baseline); rd.ReadReal(v.smooth); rd.ReadReal(v.gamma); rd.ReadBool(v.showLine);
		rd.ReadBool(v.showPoint); rd.ReadBool(v.show50); rd.ReadBool(v.show95);
		rd.ReadBool(v.showLabels); rd.ReadBool(v.ranked);
		rd.ReadBool(v.canLog); rd.ReadBool(v.logScale);
		rd.ReadInt(len);
		NEW(v.label, len, labelLen); NEW(v.index, len);
		NEW(v.mean, len); NEW(v.median, len);
		NEW(v.bottom, len); NEW(v.lower, len); NEW(v.upper, len); NEW(v.top, len);
		NEW(v.sampleSize, len); NEW(v.densX, len); NEW(v.densY, len); NEW(v.sample, len);
		NEW(lenDens, len);
		i := 0;
		WHILE i < len DO
			rd.ReadString(v.label[i]);
			rd.ReadInt(v.index[i]);
			rd.ReadReal(v.mean[i]); rd.ReadReal(v.median[i]);
			rd.ReadReal(v.bottom[i]); rd.ReadReal(v.lower[i]); rd.ReadReal(v.upper[i]); rd.ReadReal(v.top[i]);
			rd.ReadInt(v.sampleSize[i]);
			rd.ReadInt(lenDens[i]);
			j := 0;
			NEW(v.densX[i], lenDens[i]);
			NEW(v.densY[i], lenDens[i]);
			WHILE j < lenDens[i] DO
				rd.ReadReal(v.densX[i, j]);
				rd.ReadReal(v.densY[i, j]);
				INC(j)
			END;
			NEW(v.sample[i], v.sampleSize[i]);
			j := 0;
			WHILE j < v.sampleSize[i] DO
				rd.ReadReal(v.sample[i, j]);
				INC(j)
			END;
			INC(i)
		END
	END InternalizeData;

	PROCEDURE (v: View) CopyDataFrom (source: PlotsViews.View(*Stores.Store*));
		VAR
			len, i, j: INTEGER;
	BEGIN
		WITH source: View DO
			v.mode := source.mode; v.orientation := source.orientation; v.stripColour := source.stripColour;
			v.baseline := source.baseline; v.smooth := source.smooth; v.gamma := source.gamma;
			v.showLine := source.showLine; v.showLabels := source.showLabels; v.ranked := source.ranked;
			v.showPoint := source.showPoint; v.show50 := source.show50; v.show95 := source.show95;
			v.canLog := source.canLog; v.logScale := source.logScale;
			len := LEN(source.label);
			NEW(v.label, len, labelLen); NEW(v.index, len);
			NEW(v.mean, len); NEW(v.median, len);
			NEW(v.bottom, len); NEW(v.lower, len); NEW(v.upper, len); NEW(v.top, len);
			NEW(v.sample, len); NEW(v.sampleSize, len); NEW(v.densX, len); NEW(v.densY, len);
			i := 0;
			WHILE i < len DO
				v.label[i] := source.label[i]$;
				v.index[i] := source.index[i];
				v.mean[i] := source.mean[i]; v.median[i] := source.median[i];
				v.bottom[i] := source.bottom[i]; v.lower[i] := source.lower[i];
				v.upper[i] := source.upper[i]; v.top[i] := source.top[i];
				v.sampleSize[i] := source.sampleSize[i];
				NEW(v.sample[i], v.sampleSize[i]);
				NEW(v.densX[i], LEN(source.densX[i]));
				NEW(v.densY[i], LEN(source.densY[i]));
				j := 0;
				WHILE j < LEN(source.densX[i]) DO
					v.densX[i, j] := source.densX[i, j];
					v.densY[i, j] := source.densY[i, j];
					INC(j)
				END;
				j := 0;
				WHILE j < v.sampleSize[i] DO
					v.sample[i, j] := source.sample[i, j];
					INC(j)
				END;
				INC(i)
			END
		END
	END CopyDataFrom;

	PROCEDURE (v: View) DataBounds (OUT minX, maxX, minY, maxY: REAL);
		VAR
			len, i: INTEGER; minTemp, maxTemp: REAL;
	BEGIN
		len := LEN(v.label);
		minX := 0; maxX := len; maxX := maxX;
		minY := v.bottom[v.index[len - 1]];
		maxY := v.top[v.index[len - 1]];
		i := 0;
		WHILE i < len DO
			minY := MIN(minY, v.mean[i]); maxY := MAX(maxY, v.mean[i]);
			minY := MIN(minY, v.median[i]); maxY := MAX(maxY, v.median[i]);
			minY := MIN(minY, v.bottom[i]); maxY := MAX(maxY, v.bottom[i]);
			minY := MIN(minY, v.lower[i]); maxY := MAX(maxY, v.lower[i]);
			minY := MIN(minY, v.upper[i]); maxY := MAX(maxY, v.upper[i]);
			minY := MIN(minY, v.top[i]); maxY := MAX(maxY, v.top[i]);
			(* include this if want default y axis range to span the calculated densities, slightly wider than 95% CI *)
			(* minY := MIN(minY, Min(v.densX[i], LEN(v.densX[i])));
			maxY := MAX(maxY, Max(v.densX[i], LEN(v.densX[i]))) *)
			INC(i)
		END;
		IF v.orientation = stripHoriz THEN
			minTemp := minX; minX := minY; minY := minTemp; maxTemp := maxX; maxX := maxY; maxY := maxTemp
		END
	END DataBounds;

	PROCEDURE (v: View) StripLabel (index: INTEGER; OUT string: ARRAY OF CHAR), NEW;
		VAR
			i, j: INTEGER;
	BEGIN
		i := 0; WHILE (v.label[index, i] # 0X) & (v.label[index, i] # "[") DO INC(i) END;
		j := 0; WHILE v.label[index, i + j] # 0X DO string[j] := v.label[index, i + j]; INC(j) END;
		string[j] := 0X
	END StripLabel;

	(* Get RGB values in 0 - 255 for a 24 bit integer colour *)

	PROCEDURE Color2RGB (col: Ports.Color; OUT red, green, blue: INTEGER);
	BEGIN
		blue := col DIV 65536;
		green := (col - blue * 65536) DIV 256;
		red := col - blue * 65536 - green * 256
	END Color2RGB;

	(* Return the colour which is a proportion p of cdark and 1-p of clight  *)

	PROCEDURE ColorAverage (cdark, clight: Ports.Color; p: REAL): Ports.Color;
		VAR rdark, gdark, bdark, rlight, glight, blight, rave, gave, bave: INTEGER;
	BEGIN
		Color2RGB(cdark, rdark, gdark, bdark);
		Color2RGB(clight, rlight, glight, blight);
		rave := SHORT(ENTIER(p * rdark + (1 - p) * rlight + 0.5));
		gave := SHORT(ENTIER(p * gdark + (1 - p) * glight + 0.5));
		bave := SHORT(ENTIER(p * bdark + (1 - p) * blight + 0.5));
		RETURN Ports.RGBColor(rave, gave, bave)
	END ColorAverage;

	PROCEDURE (v: View) RestoreData (f: Views.Frame; l, t, r, b: INTEGER);
		CONST
			thick = Ports.mm DIV 3; width = 0.8;
		VAR
			minX, maxX, minY, maxY, clipX, clipY, left, right, bottom, lower, middle, upper, top, pos, barLeft, barRight,
			currDens, currX, newX, maxDens, tickExt: REAL;
			w, h, len, i, j, asc, dsc, ww, dx, dy, index: INTEGER; string: ARRAY 80 OF CHAR;
			col, meanColor: Ports.Color;
	BEGIN
		v.context.GetSize(w, h);
		v.Bounds(minX, maxX, minY, maxY);
		v.Translate(f, minX, minY, thick DIV 2, thick DIV 2, w, h, clipX, clipY);
		len := LEN(v.label);

		i := 0;
		WHILE i < len DO
			index := v.index[i];
			bottom := v.bottom[index]; lower := v.lower[index]; upper := v.upper[index]; top := v.top[index];
			CASE v.mode OF
			|showMeans: middle := v.mean[index]
			|showMedians: middle := v.median[index]
			END;
			IF v.orientation = stripVert THEN pos := i + 0.5 ELSE pos := len - i - 0.5 END;
			left := pos - width / 2; right := pos + width / 2;
			barLeft := pos - width / 2; barRight := pos + width / 2;

			meanColor := Ports.white;
			maxDens := MathSmooth.Max(v.densY[index], LEN(v.densY[index]));
			currDens := v.densY[index, 0];
			currX := v.densX[index, 0];
			j := 1;
			WHILE j < LEN(v.densY[index]) DO

				(* draw a new rectangle with new color when the density changes *)
				IF (v.densY[index, j] # currDens) THEN
					col := ColorAverage(v.stripColour, Ports.white, Math.Power(currDens / maxDens, v.gamma));
					newX := v.densX[index, j];
					CASE v.orientation OF
					|stripVert:
					v.DrawRectangle(f, left, newX, right, currX, col, Ports.fill, w, h); 	 			 			 													|stripHoriz:
					v.DrawRectangle(f, currX, right, newX, left, col, Ports.fill, w, h)	 			 			 													END;
					(* draw mean in black instead of white if its density is < 0.2 max dens *)
					IF ((currX < middle) & (middle < newX)) OR
						((newX < middle) & (middle < currX)) THEN
						IF currDens / maxDens < 0.2 THEN
							meanColor := Ports.black;
						END
					END;
					currX := newX;
					currDens := v.densY[index, j];
				END;
				INC(j)
			END;

			tickExt := 0.1 * width;
			CASE v.orientation OF
			|stripVert:
				IF v.showPoint THEN
					v.DrawLine(f, left, middle, right, middle, meanColor, thick, w, h)
				END;
				IF v.show50 THEN
					v.DrawLine(f, barLeft - tickExt, lower, barRight + tickExt, lower, Ports.black, thick, w, h);
					v.DrawLine(f, barLeft - tickExt, upper, barRight + tickExt, upper, Ports.black, thick, w, h)
				END;
				IF v.show95 THEN
					v.DrawLine(f, barLeft - tickExt, bottom, barRight + tickExt, bottom, Ports.black, thick, w, h);
					v.DrawLine(f, barLeft - tickExt, top, barRight + tickExt, top, Ports.black, thick, w, h)
				END;
			|stripHoriz:
				IF v.showPoint THEN
					v.DrawLine(f, middle, right, middle, left, meanColor, thick, w, h)
				END;
				IF v.show50 THEN
					v.DrawLine(f, lower, barRight + tickExt, lower, barLeft - tickExt, Ports.black, thick, w, h);
					v.DrawLine(f, upper, barRight + tickExt, upper, barLeft - tickExt, Ports.black, thick, w, h)
				END;
				IF v.show95 THEN
					v.DrawLine(f, bottom, barRight + tickExt, bottom, barLeft - tickExt, Ports.black, thick, w, h);
					v.DrawLine(f, top, barRight + tickExt, top, barLeft - tickExt, Ports.black, thick, w, h)
				END
			END;
			INC(i)
		END;

		(* baseline line drawn over the strips. in boxplots it goes under *)
		IF v.showLine THEN
			CASE v.orientation OF
			|stripVert:
				IF (v.baseline >= minY) & (v.baseline <= maxY) THEN
					v.DrawLine(f, 0, v.baseline, len, v.baseline, Ports.black, thick, w, h)
				END
			|stripHoriz:
				IF (v.baseline >= minX) & (v.baseline <= maxX) THEN
					v.DrawLine(f, v.baseline, 0, v.baseline, len, Ports.black, thick, w, h)
				END
			END
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
				IF v.orientation = stripVert THEN
					pos := i + 0.5
				ELSE
					pos := len - i - 0.5
				END;
				v.StripLabel(index, string);
				CASE v.orientation OF
				|stripVert:
					dx := - font.StringWidth(string) DIV 2;
					dy := dsc + Ports.mm;
					v.DrawString(f, pos, v.top[index], dx, dy, string, col, font, w, h)
				|stripHoriz:
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
			|stripVert:
				dx := Ports.mm;
				v.DrawString(f, LEN(v.label), v.baseline, dx, 0, string, Ports.red, font, w, h)
			|stripHoriz:
				dx := - (stringSize DIV 2);
				dy := asc + dsc;
				v.DrawString(f, v.baseline, 0, dx, - 2 * Ports.mm - dy, string, Ports.red, font, w, h)
			END
		END
	END DrawKey;

	PROCEDURE WriteData (v: View);
		CONST
			cell = 22 * Ports.mm; first = 3 * cell DIV 2; numTabs = 6; width = first + numTabs * cell;
			prec = 5; minW = 2; expW = 0; fillCh = " ";
		VAR
			f: TextMappers.Formatter; t: TextModels.Model; tv: Views.View;
			p: TextRulers.Prop; len, i, j, index: INTEGER;
			old, new: TextModels.Attributes;
	BEGIN
		t := TextModels.dir.New(); tv := TextViews.dir.New(t); f.ConnectTo(t);
		NEW(p); p.right := width; p.tabs.len := numTabs; p.tabs.tab[0].stop := first;
		i := 1; WHILE i < numTabs DO p.tabs.tab[i].stop := p.tabs.tab[i - 1].stop + cell; INC(i) END;
		p.valid := {TextRulers.right, TextRulers.tabs};
		f.WriteView(TextRulers.dir.NewFromProp(p));
		old := f.rider.attr; new := TextModels.NewWeight(old, Fonts.bold); f.rider.SetAttr(new);
		f.WriteString("node"); f.WriteTab; f.WriteString("mean"); f.WriteTab; f.WriteString("median"); f.WriteTab;
		f.WriteString("2.5%"); f.WriteTab; f.WriteString("25%"); f.WriteTab;
		f.WriteString("75%"); f.WriteTab; f.WriteString("97.5%"); f.WriteLn;
		f.rider.SetAttr(old);
		len := LEN(v.label);
		i := 0;
		WHILE i < len DO
			index := v.index[i];
			f.WriteString(v.label[index]);
			f.WriteTab; f.WriteRealForm(v.mean[index], prec, minW, expW, fillCh);
			f.WriteTab; f.WriteRealForm(v.median[index], prec, minW, expW, fillCh);
			f.WriteTab; f.WriteRealForm(v.bottom[index], prec, minW, expW, fillCh);
			f.WriteTab; f.WriteRealForm(v.lower[index], prec, minW, expW, fillCh);
			f.WriteTab; f.WriteRealForm(v.upper[index], prec, minW, expW, fillCh);
			f.WriteTab; f.WriteRealForm(v.top[index], prec, minW, expW, fillCh);
			(* Don't write the density here, as it isn't written by SamplesDensity *)
			f.WriteLn;
			INC(i)
		END;
		Views.OpenAux(tv, "Density strip data")
	END WriteData;

	PROCEDURE (v: View) HandleDataCtrlMsg (f: Views.Frame; VAR msg: Controllers.Message;
	VAR focus: Views.View);
		VAR
			x0, y0, x, y, w, h: INTEGER; buttons: SET; isDown: BOOLEAN;
	BEGIN
		WITH msg: Controllers.TrackMsg DO
			x0 := msg.x; y0 := msg.y;
			REPEAT
				f.Input(x, y, buttons, isDown);
				IF (x # x0) OR (y # y0) THEN x0 := x; y0 := y END
			UNTIL ~isDown;
			v.context.GetSize(w, h);
			IF (x0 < w) & (y0 < h) & (Controllers.modify IN buttons) THEN WriteData(v) END
		ELSE
		END
	END HandleDataCtrlMsg;

	PROCEDURE (v: View) ShowData;
	BEGIN
		WriteData(v);
	END ShowData;

	PROCEDURE (f: Factory) New (IN name: ARRAY OF CHAR;
	IN label: ARRAY OF ARRAY OF CHAR;
	IN mean: ARRAY OF REAL;
	IN percentiles: ARRAY OF ARRAY OF REAL;
	IN start, sampleSize: ARRAY OF INTEGER;
	other, axis: POINTER TO ARRAY OF REAL): PlotsViews.View;
		VAR
			len, i, j: INTEGER; v: View;
			sum, minX, maxX, minY, maxY: REAL;
			sample: POINTER TO ARRAY OF POINTER TO ARRAY OF REAL;
			xAxis, yAxis: PlotsAxis.Axis;
	BEGIN
		len := LEN(label);
		NEW(v);
		v.mode := showMeans; v.orientation := stripVert; v.stripColour := Ports.black;
		v.showLine := TRUE; v.showLabels := TRUE; v.ranked := FALSE; v.logScale := FALSE;
		v.showPoint := TRUE; v.show50 := FALSE; v.show95 := FALSE;
		v.smooth := 0.2; v.gamma := 1;
		NEW(v.label, len, 80); NEW(v.index, len);
		NEW(v.mean, len); NEW(v.median, len);
		NEW(v.bottom, len); NEW(v.lower, len); NEW(v.upper, len); NEW(v.top, len);
		NEW(v.sampleSize, len); NEW(v.densX, len); NEW(v.densY, len);
		NEW(v.sample, len);
		(*	copy samples from factory to view at end of copying samples not linked to factory	*)
		i := len - 1;
		WHILE i >= 0 DO
			v.sample[i] := f.sampleList.sample;
			f.sampleList := f.sampleList.next;
			DEC(i)
		END;
		i := 0; sum := 0;
		WHILE i < len DO
			v.index[i] := i;
			v.label[i] := label[i]$;
			v.sampleSize[i] := sampleSize[i];
			IF MathSmooth.IsDiscrete(v.sample[i], v.sampleSize[i]) THEN
				BuildHistogram(v.sample[i], v.sampleSize[i], v.smooth, v.densX[i], v.densY[i])
			ELSE BuildDensity(v.sample[i], v.sampleSize[i], v.smooth, v.densX[i], v.densY[i])
			END;
			v.mean[i] := mean[i]; sum := sum + v.mean[i];
			ASSERT(LEN(percentiles[i]) = numFract, 21);
			v.bottom[i] := percentiles[i, 0]; v.lower[i] := percentiles[i, 1];
			v.median[i] := percentiles[i, 2]; v.upper[i] := percentiles[i, 3]; v.top[i] := percentiles[i, 4];
			INC(i)
		END;
		v.baseline := sum / len;
		v.DataBounds(minX, maxX, minY, maxY); v.canLog := minY > 0;
		v.Init;
		v.SetSizes(minW, minH, left, top, right, bottom);
		xAxis := PlotsAxis.New("PlotsEmptyaxis.Empty");
		yAxis := PlotsAxis.New("PlotsStdaxis.Continuous");
		yAxis.SetTitle(name);
		v.SetXAxis(xAxis);
		v.SetYAxis(yAxis);
		v.SetTitle("density strips: " + name);
		(* update default baseline displayed in text box with value calculated from the data *)
		densDialog.baseline := v.baseline;
		Dialog.Update(densDialog);
		RETURN v
	END New;

	PROCEDURE (f: Factory) StoreSample (sample: POINTER TO ARRAY OF REAL);
		VAR
			list: List;
	BEGIN
		NEW(list);
		list.sample := sample;
		list.next := f.sampleList;
		f.sampleList := list
	END StoreSample;
	
	PROCEDURE (f: Factory) Title (OUT title: ARRAY OF CHAR);
	BEGIN
		title := "#Compare:Density strips"
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

	PROCEDURE GuardPointEstimate* (VAR par: Dialog.Par);
		VAR
			v: PlotsViews.View;
	BEGIN
		v := PlotsDialog.Singleton(); par.disabled := ~IsView(v);
		IF ~par.disabled THEN
			WITH v: View DO par.disabled := ~v.showPoint & ~ v.ranked END
		END
	END GuardPointEstimate;

	PROCEDURE Notifier* (op, from, to: INTEGER);
		VAR
			v: PlotsViews.View;
	BEGIN
		IF op = Dialog.changed THEN
			v := PlotsDialog.Singleton();
			Controllers.SetCurrentPath(Controllers.targetPath);
			Properties.EmitProp(NIL, densDialog);
			Controllers.ResetCurrentPath
		END
	END Notifier;

	(* Only update gamma correction property if valid value > 0 is typed *)
	(* Don't do this check in SetProperties, which will reset the text field
	while you are in the middle of typing a new value,  if the partially-typed value
	is invalid and you're too slow at typing *)

	PROCEDURE NotifierGamma* (op, from, to: INTEGER);
		VAR
			v: PlotsViews.View;
	BEGIN
		v := PlotsDialog.Singleton();
		Controllers.SetCurrentPath(Controllers.targetPath);
		WITH v: View DO
			IF densDialog.gamma > 0 THEN
				Properties.EmitProp(NIL, densDialog)
			END
		END;
		Controllers.ResetCurrentPath
	END NotifierGamma;

	PROCEDURE (s: SpecialDialog) Dialog (): Views.View;
		VAR
			loc: Files.Locator;
	BEGIN
		IF dialogView = NIL THEN
			loc := Files.dir.This("Compare/Rsrc");
			dialogView := Views.OldView(loc, "DensDialog");
		END;
		RETURN dialogView
	END Dialog;

	PROCEDURE (s: SpecialDialog) Set (prop: Properties.Property);
	BEGIN
		WITH prop: DensProp DO
			densDialog^ := prop^;
			Dialog.Update(densDialog)
		ELSE
		END
	END Set;

	PROCEDURE (s: SpecialApplyDialog) Apply ();
		VAR
			v: PlotsViews.View; i: INTEGER;
	BEGIN
		v := PlotsDialog.Singleton();
		Controllers.SetCurrentPath(Controllers.targetPath);
		i := 0;
		WITH v: View DO
			WHILE i < LEN(v.label) DO
				IF MathSmooth.IsDiscrete(v.sample[i], v.sampleSize[i]) THEN
					BuildHistogram(v.sample[i], v.sampleSize[i], v.smooth, v.densX[i], v.densY[i])
				ELSE BuildDensity(v.sample[i], v.sampleSize[i], v.smooth, v.densX[i], v.densY[i])
				END;
				INC(i)
			END
		END;
		Properties.EmitProp(NIL, densDialog);
		Controllers.ResetCurrentPath
	END Apply;

	PROCEDURE (s: SpecialApplyDialog) Guard (VAR par: Dialog.Par);
		VAR
			v: PlotsViews.View;
	BEGIN
		v := PlotsDialog.Singleton(); par.disabled := ~IsView(v);
		IF ~par.disabled THEN
			WITH v: View DO par.disabled := ~(v.smooth > 0) OR ~(v.smooth < 1) END
		END
	END Guard;

	PROCEDURE ViewInstall*;
	BEGIN
		PlotsDialog.SetSpecial(special);
		PlotsDialog.SetSpecialApply(specialApply)
	END ViewInstall;

	PROCEDURE Install*;
	BEGIN
		CompareViews.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "C.H.Jackson"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory;
			fractions: POINTER TO ARRAY OF REAL;
	BEGIN
		Maintainer;
		NEW(densDialog);
		densDialog.mode := showMeans;
		densDialog.orientation := stripVert;
		densDialog.stripColour := Ports.black;
		densDialog.smooth := 0.2;
		densDialog.baseline := 0;
		densDialog.gamma := 1;
		densDialog.showLine := TRUE;
		densDialog.showLabels := TRUE;
		densDialog.showPoint := TRUE;
		densDialog.show50 := FALSE;
		densDialog.show95 := FALSE;
		densDialog.ranked := FALSE;
		(* canLog ? *)
		densDialog.logScale := FALSE;
		densDialog.valid := propAll;
		NEW(f);
		NEW(fractions, numFract);
		fractions[0] := 2.5; fractions[1] := 25; fractions[2] := 50; fractions[3] := 75; fractions[4] := 97.5;
		f.SetFractions(fractions); f.SetArguments({CompareViews.node});
		fact := f;
		dialogView := NIL;
		NEW(special);
		NEW(specialApply)
	END Init;

BEGIN
	Init
END CompareDenstrip.
