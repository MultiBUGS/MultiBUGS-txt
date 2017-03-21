(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		 *)

MODULE MapsViews;


	

	IMPORT
		Controllers, Dialog, Files, Fonts, Ports, Properties, Stores, Strings, Views, 
		TextModels, 
		BugsMappers, BugsTexts, 
		MapsIndex, MapsMap, MathSort, 
		PlotsAxis, PlotsViews;

	CONST
		minSize = 100 * Ports.mm;
		boxSize = 6 * Ports.mm;
		gap = 2 * Ports.mm;
		left = 0;
		top = 5 * Ports.mm;
		right = 40 * Ports.mm;
		bottom = 0;

		absCut* = 0;
		percentileCut* = 1;

	TYPE
		String = ARRAY 80 OF CHAR;

		View = POINTER TO RECORD(PlotsViews.View)
			map: MapsMap.Map;
			numCuts, typeCuts: INTEGER;
			cuts, percentiles, data: POINTER TO ARRAY OF REAL;
			mask: POINTER TO ARRAY OF BOOLEAN;
			palette: POINTER TO ARRAY OF Ports.Color;
			histogram: ARRAY 7 OF INTEGER
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE AbsoluteLegend (IN cuts: ARRAY OF REAL): POINTER TO ARRAY OF String;
		VAR
			i, len: INTEGER;
			string0, string1: ARRAY 80 OF CHAR;
			legend: POINTER TO ARRAY OF String;
	BEGIN
		i := 0;
		len := LEN(cuts) + 1;
		NEW(legend, len);
		WHILE i < len DO
			IF i = 0 THEN
				string0 := "<";
				Strings.RealToStringForm(cuts[i], 6, 7, 0, " ", string1)
			ELSIF i = len - 1 THEN
				string0 := ">=";
				Strings.RealToStringForm(cuts[i - 1], 6, 7, 0, " ", string1)
			ELSE
				Strings.RealToStringForm(cuts[i - 1], 6, 7, 0, " ", string0);
				string0 := string0 + " - ";
				Strings.RealToStringForm(cuts[i], 6, 7, 0, " ", string1)
			END;
			legend[i] := string0 + string1;
			INC(i)
		END;
		RETURN legend
	END AbsoluteLegend;

	PROCEDURE PercentileLegend (IN percentiles: ARRAY OF REAL): POINTER TO ARRAY OF String;
		VAR
			i, len: INTEGER;
			string0, string1: ARRAY 80 OF CHAR;
			legend: POINTER TO ARRAY OF String;
	BEGIN
		i := 0;
		len := LEN(percentiles) + 1;
		NEW(legend, len);
		WHILE i < len DO
			IF i = 0 THEN
				string0 := "<";
				Strings.RealToStringForm(percentiles[i], 6, 7, 0, " ", string1)
			ELSIF i = len - 1 THEN
				string0 := ">=";
				Strings.RealToStringForm(percentiles[i - 1], 6, 7, 0, " ", string1)
			ELSE
				Strings.RealToStringForm(percentiles[i - 1], 6, 7, 0, " ", string0);
				string0 := string0 + "% - ";
				Strings.RealToStringForm(percentiles[i], 6, 7, 0, " ", string1)
			END;
			legend[i] := string0 + string1 + "%";
			INC(i)
		END;
		RETURN legend
	END PercentileLegend;

	PROCEDURE (v: View) ModifySize (dw, dh: INTEGER);
		VAR
			w, h: INTEGER;
	BEGIN
		v.context.GetSize(w, h);
		v.context.SetSize(w + dw, h + dh)
	END ModifySize;

	PROCEDURE (v: View) ExcludeInvalidProps (VAR valid: SET);
	BEGIN
		valid := valid - PlotsViews.allXBounds - PlotsViews.allYBounds
	END ExcludeInvalidProps;

	PROCEDURE (v: View) ExternalizeData (VAR wr: Stores.Writer);
		VAR
			i, len: INTEGER;
	BEGIN
		wr.WriteString(v.map.name);
		wr.WriteInt(v.numCuts);
		wr.WriteInt(v.typeCuts);
		i := 0;
		WHILE i < v.numCuts DO
			wr.WriteReal(v.cuts[i]);
			INC(i)
		END;
		i := 0;
		WHILE i < v.numCuts DO
			wr.WriteReal(v.percentiles[i]);
			INC(i)
		END;
		len := LEN(v.data);
		wr.WriteInt(len);
		i := 0;
		WHILE i < len DO
			wr.WriteReal(v.data[i]);
			INC(i)
		END;
		i := 0;
		WHILE i < len DO
			wr.WriteBool(v.mask[i]);
			INC(i)
		END;
		len := LEN(v.palette);
		wr.WriteInt(len);
		i := 0;
		WHILE i < len DO
			wr.WriteInt(v.palette[i]);
			INC(i)
		END
	END ExternalizeData;

	PROCEDURE (v: View) InternalizeData (VAR rd: Stores.Reader);
		VAR
			i, len: INTEGER;
			map: MapsMap.Map;
			name: ARRAY 80 OF CHAR;
			file: Files.File;
			loc: Files.Locator;
	BEGIN
		rd.ReadString(name);
		map := MapsIndex.Find(name);
		IF map = NIL THEN
			loc := Files.dir.This("Maps/Rsrc");
			file := Files.dir.Old(loc, name + ".map", Files.shared);
			map := MapsMap.New(file, name);
			MapsIndex.Store(map)
		END;
		v.map := map;
		rd.ReadInt(v.numCuts);
		rd.ReadInt(v.typeCuts);
		NEW(v.cuts, v.numCuts);
		i := 0;
		WHILE i < v.numCuts DO
			rd.ReadReal(v.cuts[i]);
			INC(i)
		END;
		NEW(v.percentiles, v.numCuts);
		i := 0;
		WHILE i < v.numCuts DO
			rd.ReadReal(v.percentiles[i]);
			INC(i)
		END;
		rd.ReadInt(len);
		NEW(v.data, len);
		NEW(v.mask, len);
		i := 0;
		WHILE i < len DO
			rd.ReadReal(v.data[i]);
			INC(i)
		END;
		i := 0;
		WHILE i < len DO
			rd.ReadBool(v.mask[i]);
			INC(i)
		END;
		rd.ReadInt(len);
		NEW(v.palette, len);
		i := 0;
		WHILE i < len DO
			rd.ReadInt(v.palette[i]);
			INC(i)
		END
	END InternalizeData;

	PROCEDURE (v: View) CopyDataFrom (source: PlotsViews.View(*Stores.Store*));
		VAR
			i, len: INTEGER;
	BEGIN
		WITH source: View DO
			v.map := source.map;
			v.numCuts := source.numCuts;
			v.typeCuts := source.typeCuts;
			len := LEN(source.cuts);
			NEW(v.cuts, len);
			i := 0;
			WHILE i < len DO
				v.cuts[i] := source.cuts[i];
				INC(i)
			END;
			len := LEN(source.percentiles);
			NEW(v.percentiles, len);
			i := 0;
			WHILE i < len DO
				v.percentiles[i] := source.percentiles[i];
				INC(i)
			END;
			v.data := source.data;
			v.mask := source.mask;
			len := LEN(source.palette);
			NEW(v.palette, len);
			i := 0;
			WHILE i < len DO
				v.palette[i] := source.palette[i];
				INC(i)
			END
		END
	END CopyDataFrom;

	PROCEDURE (v: View) DataBounds (OUT minX, maxX, minY, maxY: REAL);
		VAR
			i, j, len: INTEGER;
			map: MapsMap.Map;
			region: MapsMap.Region;
			polygon: MapsMap.Polygon;
	BEGIN
		map := v.map;
		region := map.regions[0];
		polygon := region.outline;
		minX := polygon.x[0];
		maxX := minX;
		minY := polygon.y[0];
		maxY := minY;
		j := 0;
		WHILE j < map.numReg DO
			WHILE polygon # NIL DO
				i := 0;
				len := polygon.len;
				WHILE i < len DO
					minX := MIN(minX, polygon.x[i]);
					maxX := MAX(maxX, polygon.x[i]);
					minY := MIN(minY, polygon.y[i]);
					maxY := MAX(maxY, polygon.y[i]);
					INC(i)
				END;
				polygon := polygon.next
			END;
			INC(j);
			IF j # map.numReg THEN
				polygon := map.regions[j].outline
			END
		END
	END DataBounds;

	PROCEDURE (v: View) DrawKey (f: Views.Frame; font: Fonts.Font; col: INTEGER);
		CONST
			thick = Ports.mm DIV 3;
		VAR
			i, lenKey, boxHeight, boxSpace, xPos, yPos, asc, dsc,
			ww, space, textPos, w, h, lM, tM, rM, bM: INTEGER;
			string: String;
			legend: POINTER TO ARRAY OF String;
	BEGIN
		v.context.GetSize(w, h);
		v.GetMargins(lM, tM, rM, bM);
		CASE v.typeCuts OF
		|absCut:
			legend := AbsoluteLegend(v.cuts)
		|percentileCut:
			legend := PercentileLegend(v.percentiles)
		END;
		font.GetBounds(asc, dsc, ww);
		lenKey := LEN(legend);
		IF lenKey * boxSize + (lenKey - 1) * gap > h - bM THEN
			boxSpace := (h - bM) DIV 4 * (lenKey - 1); boxHeight := 3 * boxSpace
		ELSE
			boxSpace := gap; boxHeight := boxSize
		END;
		i := 0;
		xPos := w - rM + gap;
		yPos := 0;
		WHILE i < lenKey DO
			f.DrawRect(xPos, yPos, xPos + boxSize, yPos + boxHeight, Ports.fill, v.palette[i]);
			f.DrawRect(xPos, yPos, xPos + boxSize, yPos + boxHeight, thick, Ports.black);
			Strings.IntToString(v.histogram[i], string); string := "(" + string + ") " + legend[i];
			space := font.StringWidth(legend[i]); textPos := xPos + boxSize + gap;
			IF textPos + space < w THEN
				f.DrawString(textPos, yPos + boxHeight DIV 2 + dsc, col, string, font)
			END;
			yPos := yPos + boxHeight + boxSpace;
			INC(i)
		END
	END DrawKey;

	PROCEDURE KeyColour (v: View; y, h, lenKey: INTEGER): INTEGER;
		VAR
			boxSpace, boxHeight, i, yPos, lM, tM, rM, bM: INTEGER;
	BEGIN
		v.GetMargins(lM, tM, rM, bM);
		IF lenKey * boxSize + (lenKey - 1) * gap > h - bM THEN
			boxSpace := (h - bM) DIV 4 * (lenKey - 1);
			boxHeight := 3 * boxSpace
		ELSE
			boxSpace := gap;
			boxHeight := boxSize
		END;
		i := 0;
		yPos := 0;
		WHILE (i < lenKey) & ((y < yPos) OR (y > yPos + boxHeight)) DO
			INC(i);
			yPos := yPos + boxHeight + boxSpace
		END;
		RETURN i
	END KeyColour;

	PROCEDURE CutPoint (value: REAL; IN cuts: ARRAY OF REAL): INTEGER;
		VAR
			i, numCuts: INTEGER;
	BEGIN
		numCuts := LEN(cuts);
		i := 0;
		WHILE (i < numCuts) & (value > cuts[i]) DO
			INC(i)
		END;
		RETURN i
	END CutPoint;

	PROCEDURE (v: View) RestoreData (f: Views.Frame; l, t, r, b: INTEGER);
		CONST
			hairline = 0;
		VAR
			k, i, w, h, cutPoint, numCutpoints: INTEGER;
			map: MapsMap.Map;
			region: MapsMap.Region;
			poly: MapsMap.Polygon;
			colour: Ports.Color;
			edge: MapsMap.Edge;
	BEGIN
		v.context.GetSize(w, h);
		numCutpoints := LEN(v.cuts);
		i := 0;
		WHILE i <= numCutpoints DO
			v.histogram[i] := 0;
			INC(i)
		END;
		map := v.map;
		i := 0;
		WHILE i < map.numReg DO
			region := map.regions[i];
			k := region.id - 1;
			IF v.mask[k] THEN
				cutPoint := CutPoint(v.data[k], v.cuts);
				colour := v.palette[cutPoint];
				INC(v.histogram[cutPoint])
			ELSE
				colour := Ports.white
			END;
			IF region.inset # NIL THEN
				poly := region.outline;
				WHILE poly # NIL DO
					IF poly.len > 0 THEN
						v.DrawPath(f, poly.x, poly.y, colour, poly.len, Ports.closedPoly, Ports.fill, w, h);
						v.DrawPath(f, poly.x, poly.y, Ports.black, poly.len, Ports.closedPoly, hairline, w, h)
					END;
					poly := poly.next
				END;
				poly := region.inset;
				WHILE poly # NIL DO
					v.DrawPath(f, poly.x, poly.y, Ports.white, poly.len, Ports.closedPoly, Ports.fill, w, h);
					v.DrawPath(f, poly.x, poly.y, Ports.black, poly.len, Ports.closedPoly, hairline, w, h);
					poly := poly.next
				END
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < map.numReg DO
			region := map.regions[i];
			k := region.id - 1;
			IF v.mask[k] THEN
				cutPoint := CutPoint(v.data[k], v.cuts); colour := v.palette[cutPoint]
			ELSE
				colour := Ports.white
			END;
			IF region.inset = NIL THEN
				poly := region.outline;
				WHILE poly # NIL DO
					v.DrawPath(f, poly.x, poly.y, colour, poly.len, Ports.closedPoly, Ports.fill, w, h);
					v.DrawPath(f, poly.x, poly.y, Ports.black, poly.len, Ports.closedPoly, hairline, w, h);
					poly := poly.next
				END
			END;
			poly := region.outline;
			WHILE poly # NIL DO
				edge := poly.hiddenEdges;
				WHILE edge # NIL DO
					v.DrawLine(f, edge.x0, edge.y0, edge.x1, edge.y1, colour, hairline, w, h);
					edge := edge.next
				END;
				poly := poly.next
			END;
			INC(i)
		END
	END RestoreData;

	PROCEDURE (v: View) Constrain (fixedW, fixedH: BOOLEAN; VAR w, h: INTEGER);
		VAR
			minW, minH, scaleW, scaleH, lM, tM, rM, bM: INTEGER;
	BEGIN
		v.MinSize(minW, minH);
		scaleW := minW - left - right;
		scaleH := minH - top - bottom;
		v.GetMargins(lM, tM, rM, bM);
		w := w - lM - rM; h := h - tM - bM;
		Properties.ProportionalConstraint(scaleW, scaleH, fixedW, fixedH, w, h);
		w := w + lM + rM; h := h + tM + bM
	END Constrain;

	PROCEDURE (v: View) HandleDataCtrlMsg (f: Views.Frame;
	VAR msg: Controllers.Message; VAR focus: Views.View);
		VAR
			isDown, set: BOOLEAN;
			x, x0, y, y0, w, h, i, lenKey, lM, tM, rM, bM: INTEGER;
			m: SET;
			a, b, minX, maxX, minY, maxY, alphaX, alphaY, sX, sY, deltaX, deltaY: REAL;
			region: MapsMap.Region; string, string1: ARRAY 80 OF CHAR;
			colour: Ports.Color;
	BEGIN
		WITH msg: Controllers.TrackMsg DO
			x0 := msg.x;
			y0 := msg.y;
			REPEAT
				f.Input(x, y, m, isDown);
				IF (x # x0) OR (y # y0) THEN
					x0 := x;
					y0 := y
				END
			UNTIL ~isDown;
			v.context.GetSize(w, h);
			v.GetMargins(lM, tM, rM, bM);
			IF x0 < w - rM THEN
				v.Bounds(minX, maxX, minY, maxY);
				sX := 1.2; sY := 1.2;
				alphaX := 0.50 * (w - lM - rM) * (1.0 - 1.0 / sX);
				alphaY := 0.50 * (h - bM - tM) * (1.0 - 1.0 / sY);
				deltaX := (w - lM - rM) / (sX * (maxX - minX));
				deltaY := (h - bM - tM) / (sY * (maxY - minY));
				a := minX + (x0 - lM - alphaX) / deltaX;
				b := minY + (h - bM - alphaY - y0) / deltaY;
				region := v.map.RegionAt(a, b);
				IF region # NIL THEN
					IF Controllers.modify IN m THEN
						Views.BeginModification(Views.notUndoable, v);
						v.mask[region.id - 1] := ~v.mask[region.id - 1];
						Views.EndModification(Views.notUndoable, v);
						Views.Update(v, Views.keepFrames)
					END;
					Strings.IntToString(region.id, string1);
					IF v.mask[region.id - 1] THEN
						Strings.RealToStringForm(v.data[region.id - 1], 6, 7, 0, " ", string);
						string := "value for region " + region.name + "[" + string1 + "] is " + string
					ELSE
						string := "region " + region.name
					END
				ELSE
					string := ""
				END;
				Dialog.ShowStatus(string)
			ELSE
				IF (x0 > w - rM + gap) & (x < w - rM + gap + boxSize) THEN
					lenKey := v.numCuts + 1;
					i := KeyColour(v, y0, h, lenKey);
					IF i < lenKey THEN
						Dialog.GetColor(v.palette[i], colour, set);
						IF set THEN
							Views.BeginModification(Views.notUndoable, v);
							v.palette[i] := colour;
							Views.EndModification(Views.notUndoable, v);
							Views.Update(v, Views.keepFrames)
						END
					END
				END
			END;
			Views.Update(v, Views.keepFrames)
		|msg: Controllers.PollCursorMsg DO
			f.Input(x, y, m, isDown);
			v.context.GetSize(w, h);
			v.GetMargins(lM, tM, rM, bM);
			IF x < w - rM THEN
				msg.cursor := Ports.graphicsCursor
			ELSE
				msg.cursor := Ports.arrowCursor
			END
		ELSE
		END
	END HandleDataCtrlMsg;

	PROCEDURE (v: View) ShowData;
		VAR
			f: BugsMappers.Formatter;
			t: TextModels.Model;
	BEGIN
		t := TextModels.dir.New();
		BugsTexts.ConnectFormatter(f, t);
		f.SetPos(0);
		v.map.Print(f);
		f.Register("Splus map")
	END ShowData;

	PROCEDURE New* (IN title: ARRAY OF CHAR;
	map: MapsMap.Map;
	numCuts, typeCuts: INTEGER;
	IN cuts, data: ARRAY OF REAL;
	IN mask: ARRAY OF BOOLEAN;
	IN palette: ARRAY OF Ports.Color): PlotsViews.View;
		VAR
			i, j, len, size, w, h, scaleW, scaleH: INTEGER;
			minW, maxW, minH, maxH: REAL;
			buffer: POINTER TO ARRAY OF REAL;
			v: View;
			xAxis, yAxis: PlotsAxis.Axis;
	BEGIN
		NEW(v);
		v.map := map;
		v.numCuts := numCuts;
		v.typeCuts := typeCuts;
		len := LEN(data);
		NEW(v.data, len);
		NEW(v.mask, len);
		i := 0; size := 0;
		WHILE i < len DO
			v.mask[i] := mask[i];
			IF mask[i] THEN
				v.data[i] := data[i]; INC(size)
			ELSE
				v.data[i] := 0.0
			END;
			INC(i)
		END;
		CASE typeCuts OF
		|absCut:
			NEW(v.cuts, numCuts);
			NEW(v.percentiles, numCuts);
			i := 0;
			WHILE i < numCuts DO
				v.cuts[i] := cuts[i];
				INC(i)
			END;
			i := 0;
			WHILE i < numCuts DO
				v.percentiles[i] := cuts[i];
				INC(i)
			END;
		|percentileCut:
			NEW(v.cuts, numCuts);
			NEW(v.percentiles, numCuts);
			i := 0;
			WHILE i < numCuts DO
				v.percentiles[i] := cuts[i];
				INC(i)
			END;
			NEW(buffer, size);
			i := 0;
			j := 0;
			WHILE i < len DO
				IF mask[i] THEN
					buffer[j] := data[i];
					INC(j)
				END;
				INC(i)
			END;
			MathSort.HeapSort(buffer, size);
			i := 0;
			WHILE i < numCuts DO
				v.cuts[i] := buffer[SHORT(ENTIER(len * v.percentiles[i] / 100))];
				INC(i)
			END
		END;
		len := LEN(palette);
		NEW(v.palette, len);
		i := 0;
		WHILE i < len DO
			v.palette[i] := palette[i];
			INC(i)
		END;
		v.DataBounds(minW, maxW, minH, maxH);
		scaleW := SHORT(ENTIER(maxW - minW));
		scaleH := SHORT(ENTIER(maxH - minH));
		w := minSize;
		h := minSize;
		Properties.ProportionalConstraint(scaleW, scaleH, FALSE, FALSE, w, h);
		v.Init;
		w := w + left + right;
		h := h + top + bottom;
		v.SetSizes(w, h, left, top, right, bottom);
		xAxis := PlotsAxis.New("PlotsEmptyaxis.Map");
		v.SetXAxis(xAxis);
		yAxis := PlotsAxis.New("PlotsEmptyaxis.Map");
		v.SetYAxis(yAxis);
		v.SetTitle(title);
		RETURN v
	END New;

	PROCEDURE FocusMap* (): MapsMap.Map;
		VAR
			v: Views.View;
			map: MapsMap.Map;
	BEGIN
		v := Controllers.FocusView();
		IF v # NIL THEN
			WITH v: View DO
				map := v.map
			ELSE map := NIL
			END
		ELSE
			map := NIL
		END;
		RETURN map
	END FocusMap;

	PROCEDURE Focus* (): Views.View;
		VAR
			v: Views.View;
	BEGIN
		v := Controllers.FocusView();
		IF (v # NIL) & ~(v IS View) THEN
			v := NIL
		END;
		RETURN v
	END Focus;

	PROCEDURE GetCuts* (cutType: INTEGER; OUT cuts: ARRAY OF REAL;
	OUT numCuts, typeCuts: INTEGER);
		VAR
			v: Views.View; i: INTEGER;
	BEGIN
		v := Controllers.FocusView();
		WITH v: View DO
			IF cutType # v.typeCuts THEN
				typeCuts := absCut
			ELSE
				typeCuts := v.typeCuts
			END;
			CASE typeCuts OF
			|absCut:
				i := 0;
				numCuts := LEN(v.cuts);
				WHILE i < numCuts DO
					cuts[i] := v.cuts[i];
					INC(i)
				END
			|percentileCut:
				i := 0;
				numCuts := LEN(v.percentiles);
				WHILE i < numCuts DO
					cuts[i] := v.percentiles[i];
					INC(i)
				END
			END
		END
	END GetCuts;

	PROCEDURE SetCuts* (IN cuts: ARRAY OF REAL; numCuts, typeCuts: INTEGER;
	IN palette: ARRAY OF Ports.Color);
		VAR
			i, j, len: INTEGER;
			v: Views.View;
			buffer: POINTER TO ARRAY OF REAL;
	BEGIN
		v := Controllers.FocusView();
		WITH v: View DO
			Views.BeginModification(Views.notUndoable, v);
			v.numCuts := numCuts;
			v.typeCuts := typeCuts;
			len := LEN(palette);
			NEW(v.palette, len);
			i := 0;
			WHILE i < len DO
				v.palette[i] := palette[i];
				INC(i)
			END;
			CASE typeCuts OF
			|absCut:
				NEW(v.cuts, numCuts);
				i := 0;
				WHILE i < numCuts DO
					v.cuts[i] := cuts[i];
					INC(i)
				END;
				NEW(v.percentiles, numCuts);
				i := 0;
				WHILE i < numCuts DO
					v.percentiles[i] := cuts[i];
					INC(i)
				END;
			|percentileCut:
				NEW(v.cuts, numCuts);
				NEW(v.percentiles, numCuts);
				i := 0;
				WHILE i < numCuts DO
					v.percentiles[i] := cuts[i];
					INC(i)
				END;
				IF cuts[numCuts - 1] > 99.9999 THEN
					RETURN
				END;
				i := 0;
				j := 0;
				len := LEN(v.data);
				NEW(buffer, len);
				WHILE i < len DO
					IF v.mask[i] THEN
						buffer[j] := v.data[i];
						INC(j)
					END;
					INC(i)
				END;
				MathSort.HeapSort(buffer, j);
				i := 0;
				WHILE i < numCuts DO
					v.cuts[i] := buffer[SHORT(ENTIER(len * cuts[i] / 100))];
					INC(i)
				END
			END;
			Views.EndModification(Views.notUndoable, v);
			Views.Update(v, Views.keepFrames)
		END
	END SetCuts;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "D.J.Lunn"
	END Maintainer;

BEGIN
	Maintainer
END MapsViews.
