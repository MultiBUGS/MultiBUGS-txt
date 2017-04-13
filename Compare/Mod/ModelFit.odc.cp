(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



 *)

MODULE CompareModelFit;


	

	IMPORT
		Controllers, Dialog, Files, Ports, Properties, Stores, Views, 
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
		propLogX = 0;
		propLogY = 1;

	TYPE
		View = POINTER TO RECORD (PlotsViews.View)
			x, y, fit, lower, upper: POINTER TO ARRAY OF REAL;
			missingY: POINTER TO ARRAY OF BOOLEAN;
			canLogX, canLogY, logX, logY: BOOLEAN
		END;

		Factory = POINTER TO RECORD (CompareViews.Factory) END;

		FitProp* = POINTER TO RECORD (Properties.Property)
			logX*, logY*: BOOLEAN
		END;

		SpecialDialog = POINTER TO RECORD (PlotsDialog.SpecialDialog) END;

	VAR
		fitDialog*: FitProp;
		fact: CompareViews.Factory;
		dialogView: Views.View;
		special: SpecialDialog;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (p: FitProp) IntersectWith* (q: Properties.Property; OUT equal: BOOLEAN);
		(* not implemented yet! *)
	END IntersectWith;


	PROCEDURE (v: View) InsertProperties (VAR list: Properties.Property);
		VAR
			prop: FitProp;
	BEGIN
		NEW(prop);
		prop.logX := v.logX; prop.logY := v.logY;
		prop.valid := {propLogX, propLogY};
		prop.known := prop.valid;
		Properties.Insert(list, prop)
	END InsertProperties;

	PROCEDURE (v: View) SetProperties (prop: Properties.Property);
		CONST
			log = "PlotsStdaxis.Log";
			cont = "PlotsStdaxis.Continuous";
		VAR
			type: ARRAY 80 OF CHAR;
			xAxis, yAxis: PlotsAxis.Axis;
	BEGIN
		WITH prop: FitProp DO
			IF (propLogX IN prop.valid) & (prop.logX # v.logX) THEN
				Views.BeginModification(Views.notUndoable, v);
				v.logX := prop.logX;
				IF v.logX THEN type := log ELSE type := cont END;
				xAxis := PlotsAxis.New(type);
				v.SetXAxis(xAxis);
				PlotsViews.IncEra;
				Views.EndModification(Views.notUndoable, v); Views.Update(v, Views.keepFrames)
			END;
			IF (propLogY IN prop.valid) & (prop.logY # v.logY) THEN
				Views.BeginModification(Views.notUndoable, v);
				v.logY := prop.logY;
				IF v.logY THEN type := log ELSE type := cont END;
				yAxis := PlotsAxis.New(type);
				v.SetYAxis(yAxis);
				PlotsViews.IncEra;
				Views.EndModification(Views.notUndoable, v); Views.Update(v, Views.keepFrames)
			END
		ELSE
		END
	END SetProperties;

	PROCEDURE (v: View) ExternalizeData (VAR wr: Stores.Writer);
		VAR
			len, i: INTEGER; writeData: BOOLEAN;
	BEGIN
		len := LEN(v.x); wr.WriteInt(len);
		writeData := v.y # NIL; wr.WriteBool(writeData);
		i := 0;
		WHILE i < len DO
			IF writeData THEN wr.WriteBool(v.missingY[i]) END;
			wr.WriteReal(v.x[i]); IF writeData & ~v.missingY[i] THEN wr.WriteReal(v.y[i]) END;
			wr.WriteReal(v.fit[i]);
			wr.WriteReal(v.lower[i]);
			wr.WriteReal(v.upper[i]);
			INC(i)
		END;
		wr.WriteBool(v.canLogX);
		wr.WriteBool(v.canLogY);
		wr.WriteBool(v.logX);
		wr.WriteBool(v.logY)
	END ExternalizeData;

	PROCEDURE (v: View) InternalizeData (VAR rd: Stores.Reader);
		VAR
			len, i: INTEGER; readData: BOOLEAN;
	BEGIN
		rd.ReadInt(len); rd.ReadBool(readData);
		NEW(v.x, len); IF readData THEN NEW(v.y, len) ELSE v.y := NIL END;
		NEW(v.fit, len); NEW(v.lower, len); NEW(v.upper, len);
		IF readData THEN NEW(v.missingY, len) ELSE v.missingY := NIL END;
		i := 0;
		WHILE i < len DO
			IF readData THEN rd.ReadBool(v.missingY[i]) END;
			rd.ReadReal(v.x[i]); IF readData & ~v.missingY[i] THEN rd.ReadReal(v.y[i]) END;
			rd.ReadReal(v.fit[i]); rd.ReadReal(v.lower[i]); rd.ReadReal(v.upper[i]);
			INC(i)
		END;
		rd.ReadBool(v.canLogX); rd.ReadBool(v.canLogY); rd.ReadBool(v.logX); rd.ReadBool(v.logY)
	END InternalizeData;

	PROCEDURE (v: View) CopyDataFrom (source: PlotsViews.View(*Stores.Store*));
		VAR
			len, i: INTEGER; copyData: BOOLEAN;
	BEGIN
		WITH source: View DO
			len := LEN(source.x); copyData := source.y # NIL;
			NEW(v.x, len); IF copyData THEN NEW(v.y, len) ELSE v.y := NIL END;
			NEW(v.fit, len); NEW(v.lower, len); NEW(v.upper, len);
			IF copyData THEN NEW(v.missingY, len) ELSE v.missingY := NIL END;
			i := 0;
			WHILE i < len DO
				IF copyData THEN v.missingY[i] := source.missingY[i] END;
				v.x[i] := source.x[i]; IF copyData & ~v.missingY[i] THEN v.y[i] := source.y[i] END;
				v.fit[i] := source.fit[i];
				v.lower[i] := source.lower[i];
				v.upper[i] := source.upper[i];
				INC(i)
			END;
			v.canLogX := source.canLogX;
			v.canLogY := source.canLogY;
			v.logX := source.logX;
			v.logY := source.logY
		END
	END CopyDataFrom;

	PROCEDURE (v: View) DataBounds (OUT minX, maxX, minY, maxY: REAL);
		VAR
			len, i: INTEGER; includeData: BOOLEAN;
	BEGIN
		len := LEN(v.x); includeData := v.y # NIL;
		minX := v.x[0]; maxX := v.x[len - 1];
		minY := MAX(REAL); maxY := MIN(REAL);
		i := 0;
		WHILE i < len DO
			IF includeData & ~v.missingY[i] THEN minY := MIN(minY, v.y[i]); maxY := MAX(maxY, v.y[i]) END;
			minY := MIN(minY, v.fit[i]);
			maxY := MAX(maxY, v.fit[i]);
			minY := MIN(minY, v.lower[i]);
			maxY := MAX(maxY, v.lower[i]);
			minY := MIN(minY, v.upper[i]);
			maxY := MAX(maxY, v.upper[i]);
			INC(i)
		END
	END DataBounds;

	PROCEDURE (v: View) RestoreData (f: Views.Frame; l, t, r, b: INTEGER);
		CONST
			pSize = Ports.mm DIV 2;
			thick = Ports.mm DIV 3;
		VAR
			drawData, foundStart: BOOLEAN;
			w, h, i, len: INTEGER;
			startX, startYF, startYL, startYU: REAL;
	BEGIN
		v.context.GetSize(w, h);
		len := LEN(v.x); drawData := v.y # NIL;
		i := 0;
		foundStart := FALSE;
		WHILE i < len DO
			IF drawData & ~v.missingY[i] THEN
				v.DrawPoint(f, v.x[i], v.y[i], Ports.black, pSize, Ports.fill, w, h)
			END;
			IF foundStart THEN
				v.DrawLine(f, startX, startYF, v.x[i], v.fit[i], Ports.red, thick, w, h);
				v.DrawLine(f, startX, startYL, v.x[i], v.lower[i], Ports.blue, thick, w, h);
				v.DrawLine(f, startX, startYU, v.x[i], v.upper[i], Ports.blue, thick, w, h);
				startX := v.x[i]; startYF := v.fit[i]; startYL := v.lower[i]; startYU := v.upper[i]
			ELSE
				foundStart := TRUE;
				startX := v.x[i];
				startYF := v.fit[i];
				startYL := v.lower[i];
				startYU := v.upper[i]
			END;
			INC(i)
		END
	END RestoreData;

	PROCEDURE (f: Factory) New (IN name: ARRAY OF CHAR;
	IN label: ARRAY OF ARRAY OF CHAR;
	IN mean: ARRAY OF REAL;
	IN percentiles: ARRAY OF ARRAY OF REAL;
	IN start, sampleSize: ARRAY OF INTEGER;
	other, axis: POINTER TO ARRAY OF REAL): PlotsViews.View;
		VAR
			includeData: BOOLEAN;
			i, len: INTEGER;
			minX, maxX, minY, maxY: REAL;
			map: POINTER TO ARRAY OF INTEGER;
			x: POINTER TO ARRAY OF REAL;
			v: View;
			xAxis, yAxis: PlotsAxis.Axis;
	BEGIN
		NEW(v);
		v.logX := FALSE;
		v.logY := FALSE;
		len := LEN(axis);
		NEW(x, len);
		NEW(map, len);
		includeData := other # NIL;
		i := 0;
		WHILE i < len DO
			x[i] := axis[i];
			INC(i)
		END;
		NEW(v.x, len);
		IF includeData THEN
			NEW(v.y, len)
		ELSE
			v.y := NIL
		END;
		NEW(v.fit, len);
		NEW(v.lower, len);
		NEW(v.upper, len);
		IF includeData THEN
			NEW(v.missingY, len)
		ELSE
			v.missingY := NIL
		END;
		MathSort.Rank(x, len, map);
		i := 0;
		WHILE i < len DO
			v.x[i] := axis[map[i]];
			IF includeData THEN
				v.y[i] := other[map[i]]
			END;
			v.fit[i] := percentiles[map[i], 1];
			v.lower[i] := percentiles[map[i], 0];
			v.upper[i] := percentiles[map[i], 2];
			INC(i)
		END;
		v.Init;
		v.DataBounds(minX, maxX, minY, maxY);
		v.canLogX := minX > 0;
		v.canLogY := minY > 0;
		v.SetSizes(minW, minH, left, top, right, bottom);
		xAxis := PlotsAxis.New("PlotsStdaxis.Continuous");
		v.SetXAxis(xAxis);
		yAxis := PlotsAxis.New("PlotsStdaxis.Continuous");
		yAxis.SetTitle(name);
		v.SetYAxis(yAxis);
		v.SetTitle("model fit: " + name);
		RETURN v
	END New;

	PROCEDURE (f: Factory) Title (OUT title: ARRAY OF CHAR);
	BEGIN
		title := "#Compare:Model fit"
	END Title;

	PROCEDURE IsView (v: PlotsViews.View): BOOLEAN;
	BEGIN
		IF v # NIL THEN
			RETURN v IS View
		ELSE
			RETURN FALSE
		END
	END IsView;

	PROCEDURE GuardLogX* (VAR par: Dialog.Par);
		VAR
			v: PlotsViews.View;
	BEGIN
		v := PlotsDialog.Singleton();
		par.disabled := ~IsView(v);
		IF ~par.disabled THEN
			WITH v: View DO
				par.disabled := ~v.canLogX
			END
		END
	END GuardLogX;

	PROCEDURE GuardLogY* (VAR par: Dialog.Par);
		VAR
			v: PlotsViews.View;
	BEGIN
		v := PlotsDialog.Singleton();
		par.disabled := ~IsView(v);
		IF ~par.disabled THEN
			WITH v: View DO
				par.disabled := ~v.canLogY
			END
		END
	END GuardLogY;

	PROCEDURE Notifier* (op, from, to: INTEGER);
		VAR
			v: PlotsViews.View;
	BEGIN
		IF op = Dialog.changed THEN
			v := PlotsDialog.Singleton();
			Controllers.SetCurrentPath(Controllers.targetPath);
			Properties.EmitProp(NIL, fitDialog);
			Controllers.ResetCurrentPath
		END
	END Notifier;

	PROCEDURE (s: SpecialDialog) Dialog (): Views.View;
		VAR
			loc: Files.Locator;
	BEGIN
		IF dialogView = NIL THEN
			loc := Files.dir.This("Compare/Rsrc");
			dialogView := Views.OldView(loc, "FitDialog")
		END;
		RETURN dialogView
	END Dialog;

	PROCEDURE (s: SpecialDialog) Set (prop: Properties.Property);
	BEGIN
		WITH prop: FitProp DO
			fitDialog^ := prop^;
			Dialog.Update(fitDialog)
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
		NEW(fitDialog);
		fitDialog.logX := FALSE;
		fitDialog.logY := FALSE;
		fitDialog.valid := {propLogX, propLogY};
		NEW(f);
		NEW(fractions, numFract);
		fractions[0] := 2.5;
		fractions[1] := 50;
		fractions[2] := 97.5;
		f.SetFractions(fractions);
		f.SetArguments({CompareViews.node, CompareViews.other, CompareViews.axis});
		fact := f;
		NEW(special)
	END Init;

BEGIN
	Init
END CompareModelFit.
