(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		 *)

MODULE MapsMap;


	

	IMPORT
		Files, Stores, Strings, 
		BugsMsg,
		TextMappers;

	TYPE

		Edge* = POINTER TO LIMITED RECORD
			x0-, y0-, x1-, y1-: REAL;
			next-: Edge
		END;

		Polygon* = POINTER TO LIMITED RECORD
			id-, len-: INTEGER;
			centreX-, centreY-: REAL;
			north-, east-, south-, west-: REAL;
			x-, y-: POINTER TO ARRAY OF REAL;
			hiddenEdges-: Edge;
			next-: Polygon
		END;


		Region* = POINTER TO LIMITED RECORD
			name-: ARRAY 80 OF CHAR;
			id-: INTEGER;
			north-, east-, south-, west-: REAL;
			outline-, inset-: Polygon
		END;

		Map* = POINTER TO LIMITED RECORD
			name-: ARRAY 80 OF CHAR;
			numReg-: INTEGER;
			north-, east-, south-, west-: REAL;
			regions-: POINTER TO ARRAY OF Region
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Error (errorNum: INTEGER);
		VAR
			numToString: ARRAY 8 OF CHAR;
			errorMes: ARRAY 1024 OF CHAR;
	BEGIN
		Strings.IntToString(errorNum, numToString);
		BugsMsg.Lookup("MapsMap" + numToString, errorMes);
		BugsMsg.Store(errorMes)
	END Error;

	PROCEDURE (polygon: Polygon) Set* (id, len: INTEGER; centreX, centreY: REAL; IN x, y: ARRAY OF REAL), NEW;
		VAR
			i: INTEGER;
	BEGIN
		polygon.id := id;
		polygon.len := len;
		polygon.centreX := centreX;
		polygon.centreY := centreY;
		polygon.hiddenEdges := NIL;
		IF len > 0 THEN
			NEW(polygon.x, len);
			NEW(polygon.y, len);
			i := 0;
			WHILE i < len DO
				polygon.x[i] := x[i];
				polygon.y[i] := y[i];
				INC(i)
			END
		ELSE
			polygon.x := NIL;
			polygon.y := NIL
		END
	END Set;

	PROCEDURE (polygon: Polygon) CalculateBounds, NEW;
		VAR
			i, len: INTEGER;
	BEGIN
		i := 0;
		len := polygon.len;
		polygon.north := polygon.y[0];
		polygon.east := polygon.x[0];
		polygon.south := polygon.y[0];
		polygon.west := polygon.x[0];
		WHILE i < len DO
			polygon.north := MAX(polygon.north, polygon.y[i]);
			polygon.east := MAX(polygon.east, polygon.x[i]);
			polygon.south := MIN(polygon.south, polygon.y[i]);
			polygon.west := MIN(polygon.west, polygon.x[i]);
			INC(i)
		END
	END CalculateBounds;

	PROCEDURE (polygon: Polygon) HiddenEdges (p: Polygon), NEW;
		CONST
			eps = 1.0;
		VAR
			i, j, lenI, lenJ, iP, jP, iM, jM: INTEGER;
			edge: Edge;
	BEGIN
		lenI := LEN(polygon.x);
		lenJ := LEN(p.x);
		i := 0;
		WHILE i < lenI DO
			j := 0;
			WHILE j < lenJ DO
				IF (ABS(polygon.x[i] - p.x[j]) < eps) & (ABS(polygon.y[i] - p.y[j]) < eps) THEN
					iP := (i + 1) MOD lenI; jP := (j + 1) MOD lenJ;
					iM := i - 1; IF iM < 0 THEN iM := lenI - 1 END;
					jM := j - 1; IF jM < 0 THEN jM := lenJ - 1 END;
					IF (ABS(polygon.x[iP] - p.x[jP]) < eps) & (ABS(polygon.y[iP] - p.y[jP]) < eps) THEN
						NEW(edge);
						edge.next := polygon.hiddenEdges;
						polygon.hiddenEdges := edge;
						edge.x0 := polygon.x[i];
						edge.y0 := polygon.y[i];
						edge.x1 := polygon.x[iP];
						edge.y1 := polygon.y[iP]
					ELSIF (ABS(polygon.x[iP] - p.x[jM]) < eps) & (ABS(polygon.y[iP] - p.y[jM]) < eps) THEN
						NEW(edge);
						edge.next := polygon.hiddenEdges;
						polygon.hiddenEdges := edge;
						edge.x0 := polygon.x[i];
						edge.y0 := polygon.y[i];
						edge.x1 := polygon.x[iP];
						edge.y1 := polygon.y[iP]
					END
				END;
				INC(j)
			END;
			INC(i)
		END
	END HiddenEdges;

	PROCEDURE (polygon: Polygon) Print (IN name: ARRAY OF CHAR; VAR f: TextMappers.Formatter), NEW;
		VAR
			i, len: INTEGER;
	BEGIN
		i := 0; len := LEN(polygon.x);
		WHILE i < len DO
			f.WriteString(name); f.WriteTab;
			f.WriteReal(polygon.x[i]);
			f.WriteTab;
			f.WriteReal(polygon.y[i]);
			f.WriteLn;
			INC(i)
		END;
		f.WriteString("NA");
		f.WriteTab;
		f.WriteString("NA");
		f.WriteTab;
		f.WriteString("NA");
		f.WriteLn
	END Print;

	PROCEDURE (polygon: Polygon) Read (VAR rd: Stores.Reader), NEW;
		VAR
			i, len: INTEGER;
			x: SHORTREAL;
	BEGIN
		rd.ReadInt(polygon.id);
		rd.ReadInt(len);
		polygon.len := len;
		rd.ReadReal(polygon.centreX);
		rd.ReadReal(polygon.centreY);
		rd.ReadReal(polygon.north);
		rd.ReadReal(polygon.east);
		rd.ReadReal(polygon.south);
		rd.ReadReal(polygon.west);
		IF len > 0 THEN
			NEW(polygon.x, len);
			NEW(polygon.y, len);
			i := 0;
			WHILE i < len DO
				rd.ReadSReal(x);
				polygon.x[i] := x;
				rd.ReadSReal(x);
				polygon.y[i] := x;
				INC(i)
			END
		ELSE
			polygon.x := NIL;
			polygon.y := NIL
		END
	END Read;

	PROCEDURE (polygon: Polygon) Write (VAR wr: Stores.Writer), NEW;
		VAR
			i, len: INTEGER;
	BEGIN
		wr.WriteInt(polygon.id);
		len := polygon.len;
		wr.WriteInt(len);
		wr.WriteReal(polygon.centreX);
		wr.WriteReal(polygon.centreY);
		wr.WriteReal(polygon.north);
		wr.WriteReal(polygon.east);
		wr.WriteReal(polygon.south);
		wr.WriteReal(polygon.west);
		i := 0;
		WHILE i < len DO
			wr.WriteSReal(SHORT(polygon.x[i]));
			wr.WriteSReal(SHORT(polygon.y[i]));
			INC(i)
		END
	END Write;

	PROCEDURE (region: Region) AddPolygon* (polygon: Polygon), NEW;
	BEGIN
		polygon.next := region.outline;
		region.outline := polygon
	END AddPolygon;

	PROCEDURE (region: Region) AddInternalPolygon* (polygon: Polygon), NEW;
	BEGIN
		polygon.next := region.inset;
		region.inset := polygon
	END AddInternalPolygon;

	PROCEDURE (region: Region) FindPolygon* (id: INTEGER): Polygon, NEW;
		VAR
			polygon: Polygon;
	BEGIN
		polygon := region.outline;
		WHILE (polygon # NIL) & (polygon.id # id) DO
			polygon := polygon.next
		END;
		RETURN polygon
	END FindPolygon;

	PROCEDURE (region: Region) CalculateBounds (OUT ok: BOOLEAN), NEW;
		VAR
			polygon: Polygon;
	BEGIN
		ok := TRUE;
		polygon := region.outline;
		IF polygon.len = 0 THEN ok := FALSE; Error(1); RETURN END;
		polygon.CalculateBounds;
		region.north := polygon.north;
		region.east := polygon.east;
		region.south := polygon.south;
		region.west := polygon.west;
		WHILE polygon # NIL DO
			IF polygon.len = 0 THEN ok := FALSE; Error(1); RETURN END;
			polygon.CalculateBounds;
			region.north := MAX(region.north, polygon.north);
			region.east := MAX(region.east, polygon.east);
			region.south := MIN(region.south, polygon.south);
			region.west := MIN(region.west, polygon.west);
			polygon := polygon.next
		END
	END CalculateBounds;

	PROCEDURE (region: Region) HiddenEdges, NEW;
		VAR
			polygon, poly: Polygon;
	BEGIN
		polygon := region.outline;
		WHILE polygon # NIL DO
			poly := polygon.next;
			WHILE poly # NIL DO
				polygon.HiddenEdges(poly);
				poly := poly.next
			END;
			polygon := polygon.next
		END
	END HiddenEdges;

	PROCEDURE (region: Region) Print (VAR f: TextMappers.Formatter), NEW;
		VAR
			cursor: Polygon;
	BEGIN
		cursor := region.outline;
		WHILE cursor # NIL DO
			cursor.Print(region.name, f);
			cursor := cursor.next
		END
	END Print;

	PROCEDURE (region: Region) Read (VAR rd: Stores.Reader), NEW;
		VAR
			polygon: Polygon;
			i, num: INTEGER;
	BEGIN
		rd.ReadString(region.name);
		rd.ReadInt(region.id);
		rd.ReadReal(region.north);
		rd.ReadReal(region.east);
		rd.ReadReal(region.south);
		rd.ReadReal(region.west);
		region.outline := NIL;
		i := 0;
		rd.ReadInt(num);
		WHILE i < num DO
			NEW(polygon);
			polygon.Read(rd);
			region.AddPolygon(polygon);
			INC(i)
		END;
		region.inset := NIL;
		i := 0;
		rd.ReadInt(num);
		WHILE
			i < num DO
			NEW(polygon);
			polygon.Read(rd);
			region.AddInternalPolygon(polygon);
			INC(i)
		END;
		region.HiddenEdges
	END Read;

	PROCEDURE (region: Region) Write (VAR wr: Stores.Writer), NEW;
		VAR
			polygon: Polygon;
			num: INTEGER;
	BEGIN
		wr.WriteString(region.name);
		wr.WriteInt(region.id);
		wr.WriteReal(region.north);
		wr.WriteReal(region.east);
		wr.WriteReal(region.south);
		wr.WriteReal(region.west);
		polygon := region.outline;
		num := 0;
		WHILE polygon # NIL DO
			INC(num);
			polygon := polygon.next
		END;
		wr.WriteInt(num);
		polygon := region.outline;
		WHILE polygon # NIL DO
			polygon.Write(wr);
			polygon := polygon.next
		END;
		polygon := region.inset;
		num := 0;
		WHILE polygon # NIL DO
			INC(num);
			polygon := polygon.next
		END;
		wr.WriteInt(num);
		polygon := region.inset;
		WHILE polygon # NIL DO
			polygon.Write(wr);
			polygon := polygon.next
		END
	END Write;

	PROCEDURE (map: Map) AddRegion* (region: Region), NEW;
		VAR
			index: INTEGER;
	BEGIN
		index := region.id - 1;
		map.regions[index] := region
	END AddRegion;

	PROCEDURE (map: Map) FindRegion* (IN name: ARRAY OF CHAR): Region, NEW;
		VAR
			i: INTEGER;
			region: Region;
	BEGIN
		i := 0;
		WHILE (i < map.numReg) & (map.regions[i].name # name) DO
			INC(i)
		END;
		IF i < map.numReg THEN
			region := map.regions[i]
		ELSE
			region := NIL
		END;
		RETURN region
	END FindRegion;

	PROCEDURE (map: Map) CalculateBounds* (OUT ok: BOOLEAN), NEW;
		VAR
			i: INTEGER;
			region: Region;
	BEGIN
		region := map.regions[0];
		region.CalculateBounds(ok);
		IF ~ok THEN
			RETURN
		END;
		map.north := region.north;
		map.east := region.east;
		map.south := region.south;
		map.west := region.west;
		i := 0;
		WHILE i < map.numReg DO
			region := map.regions[i];
			region.CalculateBounds(ok);
			IF ~ok THEN
				RETURN
			END;
			map.north := MAX(map.north, region.north);
			map.east := MAX(map.east, region.east);
			map.south := MIN(map.south, region.south);
			map.west := MIN(map.west, region.west);
			INC(i)
		END
	END CalculateBounds;

	PROCEDURE (map: Map) HiddenEdges*, NEW;
		VAR
			i, len: INTEGER;
	BEGIN
		i := 0;
		len := LEN(map.regions);
		WHILE i < len DO
			map.regions[i].HiddenEdges;
			INC(i)
		END
	END HiddenEdges;

	PROCEDURE IsInsidePolygon (IN x, y: ARRAY OF REAL; pX, pY: REAL): BOOLEAN;
		VAR
			inside: BOOLEAN;
			i, len, crossing: INTEGER;
			t, cY: REAL;
	BEGIN
		i := 0;
		crossing := 0;
		len := LEN(x);
		WHILE i < len - 1 DO
			IF ((x[i] < pX) & (pX < x[i + 1]) OR (x[i] > pX) & (pX > x[i + 1])) THEN
				t := (pX - x[i + 1]) / (x[i] - x[i + 1]);
				cY := t * y[i] + (1.0 - t) * y[i + 1];
				IF pY > cY THEN
					INC(crossing)
				END
			END;
			INC(i)
		END;
		inside := ODD(crossing);
		RETURN inside
	END IsInsidePolygon;

	PROCEDURE (map: Map) Print* (VAR f: TextMappers.Formatter), NEW;
		VAR
			i, len: INTEGER;
	BEGIN
		i := 0; len := LEN(map.regions);
		f.WriteString("map:");
		f.WriteInt(len);
		f.WriteLn;
		WHILE i < len DO
			f.WriteInt(i + 1); f.WriteTab;
			f.WriteString(map.regions[i].name);
			f.WriteLn;
			INC(i)
		END;
		i := 0;
		WHILE i < len DO
			map.regions[i].Print(f);
			INC(i)
		END;
		f.WriteString("END");
		f.WriteLn
	END Print;

	PROCEDURE (map: Map) RegionAt* (x, y: REAL): Region, NEW;
		CONST
			eps = 1.0E-1;
		VAR
			found: BOOLEAN;
			i: INTEGER;
			region: Region;
			poly: Polygon;
	BEGIN
		i := 0;
		LOOP
			region := map.regions[i];
			IF (x < region.west - eps) OR (x > region.east + eps)
				OR (y < region.south - eps) OR (y > region.north + eps) THEN
				INC(i);
				IF i = map.numReg THEN
					RETURN NIL
				END
			ELSE
				poly := region.outline;
				WHILE poly # NIL DO
					found := IsInsidePolygon(poly.x, poly.y, x, y) OR IsInsidePolygon(poly.y, poly.x, y, x);
					IF found THEN
						RETURN region
					END;
					poly := poly.next
				END;
				INC(i);
				IF i = map.numReg THEN
					RETURN NIL
				END
			END
		END
	END RegionAt;

	PROCEDURE (map: Map) Write* (VAR wr: Stores.Writer), NEW;
		VAR
			i: INTEGER;
	BEGIN
		wr.WriteInt(map.numReg);
		wr.WriteReal(map.north);
		wr.WriteReal(map.east);
		wr.WriteReal(map.south);
		wr.WriteReal(map.west);
		i := 0;
		WHILE i < map.numReg DO
			map.regions[i].Write(wr);
			INC(i)
		END
	END Write;

	PROCEDURE NewPolygon* (id, len: INTEGER; centreX, centreY: REAL;
	IN x, y: ARRAY OF REAL): Polygon;
		VAR
			polygon: Polygon;
	BEGIN
		NEW(polygon);
		polygon.Set(id, len, centreX, centreY, x, y);
		RETURN polygon
	END NewPolygon;

	PROCEDURE NewRegion* (IN name: ARRAY OF CHAR; id: INTEGER): Region;
		VAR
			region: Region;
	BEGIN
		NEW(region);
		region.name := name$;
		region.id := id;
		region.outline := NIL;
		region.inset := NIL;
		RETURN region
	END NewRegion;

	PROCEDURE NewMap* (numReg: INTEGER): Map;
		VAR
			i: INTEGER;
			map: Map;
	BEGIN
		NEW(map);
		map.name := "";
		map.numReg := numReg;
		NEW(map.regions, numReg);
		i := 0;
		WHILE i < numReg DO
			map.regions[i] := NIL;
			INC(i)
		END;
		map.north := 0.0;
		map.east := 0.0;
		map.south := 0.0;
		map.west := 0.0;
		RETURN map
	END NewMap;

	PROCEDURE New* (file: Files.File; IN name: ARRAY OF CHAR): Map;
		VAR
			i, numReg: INTEGER;
			region: Region;
			map: Map;
			rd: Stores.Reader;
	BEGIN
		rd.ConnectTo(file);
		rd.SetPos(0);
		rd.ReadInt(numReg);
		map := NewMap(numReg);
		map.name := name$;
		rd.ReadReal(map.north);
		rd.ReadReal(map.east);
		rd.ReadReal(map.south);
		rd.ReadReal(map.west);
		i := 0;
		WHILE i < map.numReg DO
			NEW(region);
			region.Read(rd);
			map.AddRegion(region);
			INC(i)
		END;
		file.Close;
		RETURN map
	END New;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END MapsMap.
