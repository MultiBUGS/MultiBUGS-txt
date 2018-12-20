(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		 *)

MODULE MapsViews1;


	

	IMPORT
		Controllers, Dialog, Files, Ports, Properties, Stores, Strings, Views,
		TextMappers, TextModels, 
		BugsFiles, BugsMappers, 
		MapsAdjacency, MapsIndex, MapsMap, 
		PlotsAxis, PlotsViews;

	CONST
		minSize = 100 * Ports.mm;
		left = 0;
		top = 5 * Ports.mm;
		right = 0; bottom = 0;

	TYPE
		View = POINTER TO RECORD(PlotsViews.View)
			map: MapsMap.Map;
			adjacency: MapsAdjacency.Adjacency;
			colour: POINTER TO ARRAY OF Ports.Color;
			selection: INTEGER
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

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
		v.adjacency.Externalize(wr);
		i := 0; len := LEN(v.colour); wr.WriteInt(len);
		WHILE i < len DO wr.WriteInt(v.colour[i]); INC(i) END;
		wr.WriteInt(v.selection)
	END ExternalizeData;

	PROCEDURE (v: View) InternalizeData (VAR rd: Stores.Reader);
		VAR
			map: MapsMap.Map; name: ARRAY 80 OF CHAR; i, len: INTEGER;
			file: Files.File; loc: Files.Locator;
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
		v.adjacency := MapsAdjacency.New(map); v.adjacency.Internalize(rd);
		i := 0; rd.ReadInt(len); NEW(v.colour, len);
		WHILE i < len DO rd.ReadInt(v.colour[i]); INC(i) END;
		rd.ReadInt(v.selection)
	END InternalizeData;

	PROCEDURE (v: View) CopyDataFrom (source: PlotsViews.View(*Stores.Store*));
		VAR
			i, len: INTEGER;
	BEGIN
		WITH source: View DO
			v.map := source.map;
			v.adjacency := MapsAdjacency.New(v.map); v.adjacency.CopyFrom(source.adjacency);
			i := 0; len := LEN(source.colour); NEW(v.colour, len);
			WHILE i < len DO
				v.colour[i] := source.colour[i]; INC(i)
			END;
			v.selection := source.selection
		END
	END CopyDataFrom;

	PROCEDURE (v: View) DataBounds (OUT minX, maxX, minY, maxY: REAL);
		VAR
			map: MapsMap.Map; region: MapsMap.Region; polygon: MapsMap.Polygon;
			i, j, len: INTEGER;
	BEGIN
		map := v.map; region := map.regions[0]; polygon := region.outline;
		minX := polygon.x[0]; maxX := minX; minY := polygon.y[0]; maxY := minY;
		j := 0;
		WHILE j < map.numReg DO
			WHILE polygon # NIL DO
				i := 0; len := polygon.len;
				WHILE i < len DO
					minX := MIN(minX, polygon.x[i]); maxX := MAX(maxX, polygon.x[i]);
					minY := MIN(minY, polygon.y[i]); maxY := MAX(maxY, polygon.y[i]);
					INC(i)
				END;
				polygon := polygon.next
			END;
			INC(j);
			IF j # map.numReg THEN polygon := map.regions[j].outline END
		END
	END DataBounds;

	PROCEDURE (v: View) RestoreData (f: Views.Frame; l, t, r, b: INTEGER);
		CONST
			hairline = 0;
		VAR
			map: MapsMap.Map; region: MapsMap.Region; poly: MapsMap.Polygon;
			k, i, w, h: INTEGER; colour: Ports.Color; edge: MapsMap.Edge;
	BEGIN
		v.context.GetSize(w, h);
		map := v.map; i := 0;
		WHILE i < map.numReg DO
			region := map.regions[i];
			k := region.id - 1; colour := v.colour[k];
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
			k := region.id - 1; colour := v.colour[k];
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
		w := w - lM - rM;
		h := h - tM - bM;
		Properties.ProportionalConstraint(scaleW, scaleH, fixedW, fixedH, w, h);
		w := w + lM + rM;
		h := h + tM + bM
	END Constrain;

	PROCEDURE (v: View) HandleDataCtrlMsg (f: Views.Frame;
	VAR msg: Controllers.Message; VAR focus: Views.View);
		VAR
			x, x0, y, y0, w, h, i, len, lM, tM, rM, bM: INTEGER; isDown: BOOLEAN; buttons: SET;
			a, b, minX, maxX, minY, maxY, alphaX, alphaY, sX, sY, deltaX, deltaY: REAL;
			region: MapsMap.Region; cursor: MapsAdjacency.List; string: ARRAY 80 OF CHAR;
	BEGIN
		WITH msg: Controllers.TrackMsg DO
			Views.BeginModification(Views.notUndoable, v);
			x0 := msg.x; y0 := msg.y;
			REPEAT
				f.Input(x, y, buttons, isDown);
				IF (x # x0) OR (y # y0) THEN x0 := x; y0 := y END
			UNTIL ~isDown;
			v.context.GetSize(w, h); v.GetMargins(lM, tM, rM, bM);
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
				IF Controllers.modify IN buttons THEN
					IF (v.selection # - 1) & (v.selection # region.id) THEN
						i := 0; len := v.map.numReg;
						WHILE i < len DO v.colour[i] := Ports.grey6; INC(i) END;
						IF v.adjacency.AreNeighbours(v.selection, region.id) THEN
							v.adjacency.RemoveNeighbour(v.selection, region.id)
						ELSE
							v.adjacency.AddNeighbour(v.selection, region.id)
						END;
						v.colour[v.selection - 1] := Ports.RGBColor(255, 83, 83);
						cursor := v.adjacency.neighbours[v.selection - 1];
						WHILE cursor # NIL DO
							IF cursor.index > 0 THEN 
								v.colour[cursor.index - 1] := Ports.RGBColor(128, 255, 128)
							END;
							cursor := cursor.next
						END
					END
				ELSE
					v.selection := region.id;
					i := 0; len := v.map.numReg;
					WHILE i < len DO v.colour[i] := Ports.grey6; INC(i) END;
					v.colour[v.selection - 1] := Ports.RGBColor(255, 83, 83);
					cursor := v.adjacency.neighbours[v.selection - 1];
					WHILE cursor # NIL DO
						IF cursor.index > 0 THEN v.colour[cursor.index - 1] := Ports.RGBColor(128, 255, 128) END;
						cursor := cursor.next
					END
				END;
				Strings.IntToString(region.id, string);
				string := region.name	 + "[" + string + "]";
				Dialog.ShowStatus(string)
			ELSE
				i := 0; len := v.map.numReg;
				WHILE i < len DO v.colour[i] := Ports.grey6; INC(i) END;
				v.selection := - 1
			END;
			Views.EndModification(Views.notUndoable, v);
			Views.Update(v, Views.keepFrames)
		|msg: Controllers.PollCursorMsg DO
			msg.cursor := Ports.graphicsCursor
		ELSE
		END
	END HandleDataCtrlMsg;

	PROCEDURE (v: View) ShowData;
		VAR
			f: TextMappers.Formatter;
			t: TextModels.Model;
	BEGIN
		t := TextModels.dir.New();
		f.ConnectTo(t);
		f.SetPos(0);
		v.map.Print(f);
		BugsFiles.Open("Splus map", t)
	END ShowData;

	PROCEDURE New* (IN title: ARRAY OF CHAR; map: MapsMap.Map): PlotsViews.View;
		VAR
			i, len, w, h, scaleW, scaleH: INTEGER;
			minW, maxW, minH, maxH: REAL;
			v: View;
			xAxis, yAxis: PlotsAxis.Axis;
	BEGIN
		NEW(v);
		v.map := map; len := map.numReg; NEW(v.colour, len);
		i := 0; WHILE i < len DO v.colour[i] := Ports.grey6; INC(i) END;
		v.DataBounds(minW, maxW, minH, maxH);
		v.adjacency := MapsAdjacency.NewNN(map);
		v.selection := - 1;
		scaleW := SHORT(ENTIER(maxW - minW));
		scaleH := SHORT(ENTIER(maxH - minH));
		w := minSize; h := minSize;
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

	PROCEDURE ImportMapFile* (file: Files.File; OUT s: Stores.Store);
		VAR
			map: MapsMap.Map;
	BEGIN
		map := MapsMap.New(file, "");
		s := New("map", map)
	END ImportMapFile;

	PROCEDURE Focus* (): Views.View;
		VAR
			v: Views.View;
	BEGIN
		v := Controllers.FocusView();
		IF (v # NIL) & ~(v IS View) THEN v := NIL END;
		RETURN v
	END Focus;

	PROCEDURE FocusMap* (): MapsMap.Map;
		VAR
			v: Views.View; map: MapsMap.Map;
	BEGIN
		v := Controllers.FocusView();
		IF v # NIL THEN
			WITH v: View DO map := v.map ELSE map := NIL END
		ELSE
			map := NIL
		END;
		RETURN map
	END FocusMap;

	PROCEDURE FocusAdjacency* (): MapsAdjacency.Adjacency;
		VAR
			v: Views.View; adjacency: MapsAdjacency.Adjacency;
	BEGIN
		v := Controllers.FocusView();
		IF v # NIL THEN
			WITH v: View DO
				adjacency := v.adjacency
			ELSE
				adjacency := NIL
			END
		ELSE
			adjacency := NIL
		END;
		RETURN adjacency
	END FocusAdjacency;

	PROCEDURE FocusSetRegion* (id: INTEGER);
		VAR
			i, len: INTEGER; v: Views.View; cursor: MapsAdjacency.List;
	BEGIN
		v := Controllers.FocusView();
		WITH v: View DO
			len := v.map.numReg;
			IF (id > len) OR (len < 1) OR (id < 1) THEN RETURN END;
			Views.BeginModification(Views.notUndoable, v);
			v.selection := id;
			i := 0;
			WHILE i < len DO v.colour[i] := Ports.grey6; INC(i) END;
			v.colour[v.selection - 1] := Ports.RGBColor(255, 83, 83);
			cursor := v.adjacency.neighbours[v.selection - 1];
			WHILE cursor # NIL DO
				IF cursor.index > 0 THEN v.colour[cursor.index - 1] := Ports.RGBColor(128, 255, 128) END;
				cursor := cursor.next
			END;
			Views.EndModification(Views.notUndoable, v);
			Views.Update(v, Views.keepFrames)
		END
	END FocusSetRegion;

	PROCEDURE FocusColourRegion* (colour, id: INTEGER);
		VAR
			i, len: INTEGER; v: Views.View; cursor: MapsAdjacency.List;
	BEGIN
		v := Controllers.FocusView();
		WITH v: View DO
			len := v.map.numReg;
			IF (id > len) OR (len < 1) OR (id < 1) THEN RETURN END;
			Views.BeginModification(Views.notUndoable, v);
			v.selection := id;
			v.colour[v.selection - 1] := colour;
			Views.EndModification(Views.notUndoable, v);
			Views.Update(v, Views.keepFrames)
		END
	END FocusColourRegion;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "D.J.Lunn"
	END Maintainer;

BEGIN
	Maintainer
END MapsViews1.
