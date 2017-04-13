(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		 *)

MODULE SamplesDensity;


	

	IMPORT
		Controllers, Dialog, Files, Models, Ports, Properties, Stores, Strings, Views, 
		TextCmds, TextMappers, TextModels, 
		MathSmooth, 
		PlotsAxis, PlotsDialog, PlotsViews, 
		SamplesViews;

	CONST
		minW = 70 * Ports.mm; minH = 35 * Ports.mm;
		denSmooth = 0; histBinSize = 0;

	TYPE
		View = POINTER TO ABSTRACT RECORD (PlotsViews.View) END;

		Density = POINTER TO RECORD (View)
			sample, density: POINTER TO ARRAY OF REAL;
			sampleSize: INTEGER;
			xMin, delta, smooth: REAL
		END;

		Histogram = POINTER TO RECORD (View)
			data, density: POINTER TO ARRAY OF REAL;
			start, xMin, binSize: INTEGER
		END;

		SpecialDen = POINTER TO RECORD (PlotsDialog.SpecialDialog) END;

		SpecialHist = POINTER TO RECORD (PlotsDialog.SpecialDialog) END;

		Factory = POINTER TO RECORD (SamplesViews.Factory) END;

		DenProp* = POINTER TO RECORD (Properties.Property)
			smooth*: REAL
		END;

		HistProp* = POINTER TO RECORD (Properties.Property)
			binSize*: INTEGER
		END;

	VAR
		denDialog*: DenProp;
		histDialog*: HistProp;
		dialogDen, dialogHist: Views.View;
		specialDen: SpecialDen;
		specialHist: SpecialHist;
		fact: SamplesViews.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (v: View) BuildDensity (IN sample: ARRAY OF REAL; sampleSize: INTEGER), NEW, ABSTRACT;

	PROCEDURE IsDensity (v: PlotsViews.View): BOOLEAN;
	BEGIN
		IF v # NIL THEN
			RETURN v IS Density
		ELSE
			RETURN FALSE
		END
	END IsDensity;

	PROCEDURE (p: DenProp) IntersectWith* (q: Properties.Property; OUT equal: BOOLEAN);
		(* not implemented yet! *)
	END IntersectWith;

	PROCEDURE ApplyToDen*;
	BEGIN
		Controllers.SetCurrentPath(Controllers.targetPath);
		Properties.EmitProp(NIL, denDialog);
		Controllers.ResetCurrentPath
	END ApplyToDen;

	PROCEDURE ApplyToAllDen*;
		VAR
			source: PlotsViews.View;
			target: Views.View;
			m: Models.Model;
			prop: DenProp;
			s: TextMappers.Scanner;
			opts: SET;
	BEGIN
		source := PlotsDialog.Singleton(); ASSERT(IsDensity(source), 25);
		NEW(prop);
		prop^ := denDialog^;
		m := Controllers.FocusModel();
		ASSERT(m # NIL, 25); ASSERT(m IS TextModels.Model, 25);
		WITH m: TextModels.Model DO
			s.ConnectTo(m);
			s.SetPos(0);
			opts := s.opts;
			INCL(opts, TextMappers.returnViews);
			s.SetOpts(opts);
			WHILE ~s.rider.eot DO
				s.Scan;
				IF s.type = TextMappers.view THEN
					target := s.view; ASSERT(target # NIL, 25);
					WITH target: Density DO
						target.SetProperties(prop);
					ELSE
					END
				END
			END
		END
	END ApplyToAllDen;

	PROCEDURE (v: Density) BuildDensity (IN sample: ARRAY OF REAL; sampleSize: INTEGER);
		VAR
			numBins, m: INTEGER;
			bandWidth, delta, minX: REAL;
			weights: ARRAY 10 OF REAL;
			density: POINTER TO ARRAY OF REAL;
	BEGIN
		bandWidth := MathSmooth.BandWidth(sample, sampleSize, v.smooth);
		MathSmooth.TriWeight(weights);
		m := LEN(weights);
		MathSmooth.BinInfo(sample, bandWidth, sampleSize, m, minX, delta, numBins);
		NEW(density, numBins);
		MathSmooth.Smooth(sample, weights, sampleSize, numBins, m, bandWidth, minX, delta, density);
		v.density := density;
		v.xMin := minX;
		v.delta := delta
	END BuildDensity;

	PROCEDURE (v: Density) CopyDataFrom (source: PlotsViews.View(*Stores.Store*));
		VAR
			lenD, lenS, i: INTEGER;
	BEGIN
		WITH source: Density DO
			lenD := LEN(source.density);
			lenS := LEN(source.sample);
			NEW(v.density, lenD);
			NEW(v.sample, lenS);
			i := 0;
			WHILE i < lenD DO
				v.density[i] := source.density[i];
				INC(i)
			END;
			i := 0;
			WHILE i < lenS DO
				v.sample[i] := source.sample[i];
				INC(i)
			END;
			v.sampleSize := source.sampleSize;
			v.xMin := source.xMin;
			v.delta := source.delta;
			v.smooth := source.smooth
		END
	END CopyDataFrom;

	PROCEDURE (v: Density) DataBounds (OUT minX, maxX, minY, maxY: REAL);
		VAR
			i, len: INTEGER;
	BEGIN
		len := LEN(v.density);
		minX := v.xMin; maxX := minX + v.delta * len;
		i := 0; minY := 0; maxY := v.density[0];
		WHILE i < len DO maxY := MAX(maxY, v.density[i]); INC(i) END
	END DataBounds;

	PROCEDURE (v: Density) ExternalizeData (VAR wr: Stores.Writer);
		VAR
			lenD, lenS, i: INTEGER;
	BEGIN
		lenD := LEN(v.density);
		lenS := LEN(v.sample);
		wr.WriteInt(lenD);
		wr.WriteInt(lenS);
		i := 0;
		WHILE i < lenD DO
			wr.WriteSReal(SHORT(v.density[i]));
			INC(i)
		END;
		i := 0;
		WHILE i < lenS DO
			wr.WriteSReal(SHORT(v.sample[i]));
			INC(i)
		END;
		wr.WriteInt(v.sampleSize);
		wr.WriteReal(v.xMin);
		wr.WriteReal(v.delta);
		wr.WriteReal(v.smooth)
	END ExternalizeData;

	PROCEDURE (v: Density) InsertProperties (VAR list: Properties.Property);
		VAR
			prop: DenProp;
	BEGIN
		NEW(prop);
		prop.smooth := v.smooth;
		prop.valid := {denSmooth};
		prop.known := prop.valid;
		Properties.Insert(list, prop)
	END InsertProperties;

	PROCEDURE (v: Density) InternalizeData (VAR rd: Stores.Reader);
		VAR
			lenD, lenS, i: INTEGER;
			x: SHORTREAL;
	BEGIN
		rd.ReadInt(lenD);
		rd.ReadInt(lenS);
		NEW(v.density, lenD);
		NEW(v.sample, lenS);
		i := 0;
		WHILE i < lenD DO
			rd.ReadSReal(x);
			v.density[i] := x;
			INC(i)
		END;
		i := 0;
		WHILE i < lenS DO
			rd.ReadSReal(x);
			v.sample[i] := x;
			INC(i)
		END;
		rd.ReadInt(v.sampleSize);
		rd.ReadReal(v.xMin);
		rd.ReadReal(v.delta);
		rd.ReadReal(v.smooth)
	END InternalizeData;

	PROCEDURE (v: Density) RestoreData (f: Views.Frame; l, t, r, b: INTEGER);
		VAR
			w, h, i, len: INTEGER;
			x0, x1: REAL;
	BEGIN
		v.context.GetSize(w, h);
		len := LEN(v.density);
		x0 := v.xMin;
		x1 := x0 + v.delta;
		i := 1;
		WHILE i < len DO
			v.DrawLine(f, x0, v.density[i - 1], x1, v.density[i], Ports.red, Ports.mm DIV 3, w, h);
			x0 := x1;
			x1 := x1 + v.delta;
			INC(i)
		END
	END RestoreData;

	PROCEDURE (v: Density) SetProperties (prop: Properties.Property);
		VAR
			minX, maxX, minY, maxY: REAL;
	BEGIN
		IF denSmooth IN prop.valid THEN
			WITH prop: DenProp DO
				Views.BeginModification(Views.notUndoable, v);
				v.smooth := prop.smooth;
				v.BuildDensity(v.sample, v.sampleSize);
				v.DataBounds(minX, maxX, minY, maxY);
				v.SetBounds(minX, maxX, minY, maxY);
				PlotsViews.IncEra;
				Views.EndModification(Views.notUndoable, v);
				Views.Update(v, Views.keepFrames)
			ELSE
			END
		END
	END SetProperties;

	PROCEDURE IsHistogram (v: PlotsViews.View): BOOLEAN;
	BEGIN
		IF v # NIL THEN
			RETURN v IS Histogram
		ELSE
			RETURN FALSE
		END
	END IsHistogram;

	PROCEDURE (p: HistProp) IntersectWith* (q: Properties.Property; OUT equal: BOOLEAN);
		(* not implemented yet! *)
	END IntersectWith;

	PROCEDURE ApplyToHist*;
	BEGIN
		Controllers.SetCurrentPath(Controllers.targetPath);
		Properties.EmitProp(NIL, histDialog);
		Controllers.ResetCurrentPath
	END ApplyToHist;

	PROCEDURE ApplyToAllHist*;
		VAR
			source: PlotsViews.View;
			target: Views.View;
			m: Models.Model;
			prop: HistProp;
			s: TextMappers.Scanner;
			opts: SET;
	BEGIN
		source := PlotsDialog.Singleton(); ASSERT(IsHistogram(source), 25);
		NEW(prop);
		prop^ := histDialog^;
		m := Controllers.FocusModel();
		ASSERT(m # NIL, 25); ASSERT(m IS TextModels.Model, 25);
		WITH m: TextModels.Model DO
			s.ConnectTo(m);
			s.SetPos(0);
			opts := s.opts;
			INCL(opts, TextMappers.returnViews);
			s.SetOpts(opts);
			WHILE ~s.rider.eot DO
				s.Scan;
				IF s.type = TextMappers.view THEN
					target := s.view; ASSERT(target # NIL, 25);
					WITH target: Histogram DO
						target.SetProperties(prop);
					ELSE
					END
				END
			END
		END
	END ApplyToAllHist;

	PROCEDURE (v: Histogram) BuildData, NEW;
		VAR
			lenDen, rem, numBins, extra, i, j, outer, inner, bin: INTEGER;
			data: POINTER TO ARRAY OF REAL;
	BEGIN
		lenDen := LEN(v.density);
		rem := lenDen MOD v.binSize;
		IF rem = 0 THEN
			numBins := lenDen DIV v.binSize; extra := 0
		ELSE
			numBins := lenDen DIV v.binSize + 1;
			extra := v.binSize - rem
		END;
		NEW(data, numBins);
		v.start := v.xMin - extra DIV 2;
		i := 0;
		WHILE i < numBins DO
			data[i] := 0;
			INC(i)
		END;
		i := 0;
		outer := v.start;
		inner := v.xMin;
		bin := 0;
		WHILE i < numBins DO
			j := 0;
			WHILE j < v.binSize DO
				IF outer = inner THEN
					IF bin < lenDen THEN
						data[i] := data[i] + v.density[bin];
						INC(inner); INC(bin)
					END
				END;
				INC(outer);
				INC(j)
			END;
			data[i] := data[i] / v.binSize;
			INC(i)
		END;
		v.data := data
	END BuildData;

	PROCEDURE (v: Histogram) BuildDensity (IN sample: ARRAY OF REAL; sampleSize: INTEGER);
		CONST
			eps = 1.0E-10;
		VAR
			bin, i, numBins: INTEGER;
			minX, maxX: REAL;
			density: POINTER TO ARRAY OF REAL;
	BEGIN
		minX := MathSmooth.Min(sample, sampleSize);
		maxX := MathSmooth.Max(sample, sampleSize);
		numBins := SHORT(ENTIER(maxX - minX + eps)) + 1;
		NEW(density, numBins);
		i := 0;
		WHILE i < numBins DO
			density[i] := 0;
			INC(i)
		END;
		i := 0;
		WHILE i < sampleSize DO
			bin := SHORT(ENTIER(sample[i] - minX + eps));
			density[bin] := density[bin] + 1;
			INC(i)
		END;
		i := 0;
		WHILE i < numBins DO
			density[i] := density[i] / sampleSize;
			INC(i)
		END;
		v.density := density;
		v.xMin := SHORT(ENTIER(minX + eps));
		v.BuildData
	END BuildDensity;

	PROCEDURE (v: Histogram) CopyDataFrom (source: PlotsViews.View(*Stores.Store*));
		VAR
			lenData, lenDen, i: INTEGER;
	BEGIN
		WITH source: Histogram DO
			lenDen := LEN(source.density);
			lenData := LEN(source.data);
			NEW(v.density, lenDen);
			NEW(v.data, lenData);
			i := 0;
			WHILE i < lenDen DO
				v.density[i] := source.density[i];
				INC(i)
			END;
			i := 0;
			WHILE i < lenData DO
				v.data[i] := source.data[i];
				INC(i)
			END;
			v.start := source.start;
			v.xMin := source.xMin;
			v.binSize := source.binSize
		END
	END CopyDataFrom;

	PROCEDURE (v: Histogram) DataBounds (OUT minX, maxX, minY, maxY: REAL);
		VAR
			i, numBins: INTEGER;
	BEGIN
		numBins := LEN(v.data);
		minX := v.start - 1;
		maxX := v.start + numBins * v.binSize;
		i := 0; minY := 0;
		maxY := v.data[0];
		WHILE i < numBins DO
			maxY := MAX(maxY, v.data[i]);
			INC(i)
		END
	END DataBounds;

	PROCEDURE (v: Histogram) ExternalizeData (VAR wr: Stores.Writer);
		VAR
			lenData, lenDen, i: INTEGER;
	BEGIN
		lenDen := LEN(v.density);
		lenData := LEN(v.data);
		wr.WriteInt(lenDen);
		wr.WriteInt(lenData);
		i := 0;
		WHILE i < lenDen DO
			wr.WriteSReal(SHORT(v.density[i]));
			INC(i)
		END;
		i := 0;
		WHILE i < lenData DO
			wr.WriteSReal(SHORT(v.data[i]));
			INC(i)
		END;
		wr.WriteInt(v.start);
		wr.WriteInt(v.xMin);
		wr.WriteInt(v.binSize)
	END ExternalizeData;

	PROCEDURE (v: Histogram) InsertProperties (VAR list: Properties.Property);
		VAR
			prop: HistProp;
	BEGIN
		NEW(prop);
		prop.binSize := v.binSize;
		prop.valid := {histBinSize};
		prop.known := prop.valid;
		Properties.Insert(list, prop)
	END InsertProperties;

	PROCEDURE (v: Histogram) InternalizeData (VAR rd: Stores.Reader);
		VAR
			lenData, lenDen, i: INTEGER;
			x: SHORTREAL;
	BEGIN
		rd.ReadInt(lenDen);
		rd.ReadInt(lenData);
		NEW(v.density, lenDen);
		NEW(v.data, lenData);
		i := 0;
		WHILE i < lenDen DO
			rd.ReadSReal(x);
			v.density[i] := x;
			INC(i)
		END;
		i := 0;
		WHILE i < lenData DO
			rd.ReadSReal(x);
			v.data[i] := x;
			INC(i)
		END;
		rd.ReadInt(v.start);
		rd.ReadInt(v.xMin);
		rd.ReadInt(v.binSize)
	END InternalizeData;

	PROCEDURE (v: Histogram) RestoreData (f: Views.Frame; l, t, r, b: INTEGER);
		CONST
			margin = 0.1;
			bottom = 0;
		VAR
			w, h, i, numBins: INTEGER;
			left, top, right: REAL;
	BEGIN
		v.context.GetSize(w, h);
		numBins := LEN(v.data);
		i := 0;
		left := v.start - 0.5;
		right := left + v.binSize;
		WHILE i < numBins DO
			top := v.data[i];
			v.DrawRectangle(f, left + margin, top, right - margin, bottom, Ports.red, Ports.fill, w, h);
			left := left + v.binSize;
			right := right + v.binSize;
			INC(i)
		END
	END RestoreData;

	PROCEDURE (v: Histogram) SetProperties (prop: Properties.Property);
		VAR
			minX, maxX, minY, maxY: REAL;
	BEGIN
		IF histBinSize IN prop.valid THEN
			WITH prop: HistProp DO
				Views.BeginModification(Views.notUndoable, v);
				v.binSize := prop.binSize; v.BuildData;
				v.DataBounds(minX, maxX, minY, maxY);
				v.SetBounds(minX, maxX, minY, maxY);
				PlotsViews.IncEra;
				Views.EndModification(Views.notUndoable, v);
				Views.Update(v, Views.keepFrames)
			ELSE
			END
		END
	END SetProperties;

	PROCEDURE (f: Factory) New (IN name: ARRAY OF CHAR; IN sample: ARRAY OF ARRAY OF REAL;
	start, step, firstChain, numChains: INTEGER): Views.View;
		CONST
			left = 20 * Ports.mm;
			top = 5 * Ports.mm;
			right = 0 * Ports.mm;
			bottom = 12 * Ports.mm;
		VAR
			i, j, lenChain, noChains, sampleSize: INTEGER;
			string, xType: ARRAY 80 OF CHAR;
			buffer: POINTER TO ARRAY OF REAL;
			v: View;
			den: Density;
			hist: Histogram;
			xAxis, yAxis: PlotsAxis.Axis;
	BEGIN
		noChains := LEN(sample, 0);
		lenChain := LEN(sample, 1);
		IF lenChain = 1 THEN RETURN NIL END;
		sampleSize := lenChain * noChains;
		Strings.IntToString(sampleSize, string);
		NEW(buffer, sampleSize);
		i := 0;
		WHILE i < noChains DO
			j := 0;
			WHILE j < lenChain DO
				buffer[i * lenChain + j] := sample[i, j];
				INC(j)
			END;
			INC(i)
		END;
		NEW(hist);
		hist.binSize := 1;
		NEW(den);
		den.smooth := 0.2;
		IF MathSmooth.IsDiscrete(buffer, sampleSize) THEN
			v := hist;
			xType := "PlotsStdaxis.Integer"
		ELSE
			NEW(den.sample, sampleSize);
			den.sampleSize := sampleSize;
			i := 0;
			WHILE i < sampleSize DO
				den.sample[i] := buffer[i];
				INC(i)
			END;
			v := den;
			xType := "PlotsStdaxis.Continuous"
		END;
		v.Init;
		v.SetSizes(minW, minH, left, top, right, bottom);
		v.BuildDensity(buffer, sampleSize);
		xAxis := PlotsAxis.New(xType);
		xAxis.SetTitle(name);
		v.SetXAxis(xAxis);
		yAxis := PlotsAxis.New("PlotsStdaxis.Continuous");
		yAxis.SetTitle("P(" + name + ")");
		v.SetYAxis(yAxis);
		v.SetTitle(name + " sample: " + string);
		RETURN v
	END New;

	PROCEDURE (f: Factory) Title (OUT title: ARRAY OF CHAR);
	BEGIN
		title := "#Samples:Posterior density"
	END Title;

	PROCEDURE Install*;
	BEGIN
		SamplesViews.SetFactory(fact)
	END Install;

	PROCEDURE GuardSmooth* (VAR par: Dialog.Par);
		VAR
			v: PlotsViews.View;
	BEGIN
		v := PlotsDialog.Singleton();
		par.disabled := ~IsDensity(v)
	END GuardSmooth;

	PROCEDURE GuardApplyToDen* (VAR par: Dialog.Par);
		VAR
			v: PlotsViews.View;
	BEGIN
		v := PlotsDialog.Singleton();
		par.disabled := ~IsDensity(v) OR ~(denDialog.smooth > 0) OR ~(denDialog.smooth < 1)
	END GuardApplyToDen;

	PROCEDURE GuardApplyToAllDen* (VAR par: Dialog.Par);
		VAR
			v: PlotsViews.View;
	BEGIN
		TextCmds.FocusGuard(par);
		IF ~par.disabled THEN
			v := PlotsDialog.Singleton();
			par.disabled := ~IsDensity(v) OR ~(denDialog.smooth > 0) OR ~(denDialog.smooth < 1)
		END
	END GuardApplyToAllDen;

	PROCEDURE GuardBinSize* (VAR par: Dialog.Par);
		VAR
			v: PlotsViews.View;
	BEGIN
		v := PlotsDialog.Singleton();
		par.disabled := ~IsHistogram(v)
	END GuardBinSize;

	PROCEDURE GuardApplyToAllHist* (VAR par: Dialog.Par);
		VAR
			v: PlotsViews.View;
	BEGIN
		TextCmds.FocusGuard(par);
		IF ~par.disabled THEN
			v := PlotsDialog.Singleton();
			par.disabled := ~IsHistogram(v) OR ~(histDialog.binSize > 0)
		END
	END GuardApplyToAllHist;

	PROCEDURE GuardApplyToHist* (VAR par: Dialog.Par);
		VAR
			v: PlotsViews.View;
	BEGIN
		v := PlotsDialog.Singleton();
		par.disabled := ~IsHistogram(v) OR ~(histDialog.binSize > 0)
	END GuardApplyToHist;

	PROCEDURE (s: SpecialDen) Dialog (): Views.View;
		VAR
			loc: Files.Locator;
	BEGIN
		IF dialogDen = NIL THEN
			loc := Files.dir.This("Samples/Rsrc");
			dialogDen := Views.OldView(loc, "DensityDialog")
		END;
		RETURN dialogDen
	END Dialog;

	PROCEDURE (s: SpecialDen) Set (prop: Properties.Property);
	BEGIN
		WITH prop: DenProp DO
			denDialog.smooth := prop.smooth;
			Dialog.Update(denDialog)
		ELSE
		END
	END Set;

	PROCEDURE (s: SpecialHist) Dialog (): Views.View;
		VAR
			loc: Files.Locator;
	BEGIN
		IF dialogHist = NIL THEN
			loc := Files.dir.This("Samples/Rsrc");
			dialogHist := Views.OldView(loc, "HistogramDialog")
		END;
		RETURN dialogHist
	END Dialog;

	PROCEDURE (s: SpecialHist) Set (prop: Properties.Property);
	BEGIN
		WITH prop: HistProp DO
			histDialog.binSize := prop.binSize;
			Dialog.Update(histDialog)
		ELSE
		END
	END Set;

	PROCEDURE DensityInstall*;
	BEGIN
		PlotsDialog.SetSpecial(specialDen)
	END DensityInstall;

	PROCEDURE HistogramInstall*;
	BEGIN
		PlotsDialog.SetSpecial(specialHist)
	END HistogramInstall;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "D.J.Lunn"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(denDialog);
		denDialog.smooth := 0.1;
		INCL(denDialog.valid, denSmooth);
		NEW(histDialog);
		INCL(histDialog.valid, histBinSize);
		histDialog.binSize := 1;
		NEW(f);
		fact := f;
		dialogDen := NIL;
		dialogHist := NIL;
		NEW(specialDen);
		NEW(specialHist)
	END Init;

BEGIN
	Init
END SamplesDensity.

