(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		 *)

MODULE MapsAdjacency;


	

	IMPORT
		Stores,
		MapsMap;

	TYPE
		List* = POINTER TO LIMITED RECORD
			index-: INTEGER;
			next-: List
		END;

		Adjacency* = POINTER TO LIMITED RECORD
			radius-: REAL;
			neighbours-: POINTER TO ARRAY OF List
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE IsOverlaping (region, neighbour: MapsMap.Region): BOOLEAN;
		CONST
			eps = 1.0E-1;
		VAR
			nsOverlap, ewOverlap: BOOLEAN;
	BEGIN
		nsOverlap := (neighbour.north < region.north + eps) & (neighbour.north > region.south - eps);
		nsOverlap := nsOverlap
		OR ((neighbour.south < region.north + eps) & (neighbour.south > region.south - eps));
		nsOverlap := nsOverlap
		OR ((region.south < neighbour.north + eps) & (region.south > neighbour.south - eps));
		nsOverlap := nsOverlap
		OR ((region.south < neighbour.north + eps) & (region.south > neighbour.south - eps));
		ewOverlap := (neighbour.east < region.east + eps) & (neighbour.east > region.west - eps);
		ewOverlap := ewOverlap
		OR ((neighbour.west < region.east + eps) & (neighbour.west > region.west - eps));
		ewOverlap := ewOverlap
		OR ((region.west < neighbour.east + eps) & (region.west > neighbour.west - eps));
		ewOverlap := ewOverlap
		OR ((region.west < neighbour.east + eps) & (region.west > neighbour.west - eps));
		RETURN nsOverlap & ewOverlap
	END IsOverlaping;

	PROCEDURE IsTouching (region, neighbour: MapsMap.Region): BOOLEAN;
		CONST
			eps = 1.0E-1;
		VAR
			isList: BOOLEAN; rPoly, nPoly, iPoly, inPoly: MapsMap.Polygon; i, j, rLen, nLen: INTEGER;
	BEGIN
		isList := FALSE; rPoly := region.outline; iPoly := region.inset;
		WHILE ~isList & (rPoly # NIL) DO
			i := 0; IF rPoly.x # NIL THEN rLen := LEN(rPoly.x) ELSE rLen := 0 END;
			WHILE ~isList & (i < rLen) DO
				IF (rPoly.x[i] > neighbour.west - eps) & (rPoly.x[i] < neighbour.east + eps) & 
					(rPoly.y[i] > neighbour.south - eps) & (rPoly.y[i] < neighbour.north + eps) THEN
					nPoly := neighbour.outline; inPoly := neighbour.inset;
					WHILE ~isList & (nPoly # NIL) DO
						j := 0; IF nPoly.x # NIL THEN nLen := LEN(nPoly.x) ELSE nLen := 0 END;
						WHILE ~isList & (j < nLen) DO
							isList := (ABS(rPoly.x[i] - nPoly.x[j]) < eps) & 
							(ABS(rPoly.y[i] - nPoly.y[j]) < eps);
							INC(j)
						END;
						nPoly := nPoly.next;
						IF nPoly = NIL THEN nPoly := inPoly; inPoly := NIL END
					END
				END;
				INC(i)
			END;
			rPoly := rPoly.next;
			IF rPoly = NIL THEN rPoly := iPoly; iPoly := NIL END
		END;
		RETURN isList
	END IsTouching;

	PROCEDURE AddNeighbour (VAR list: List; index: INTEGER);
		VAR
			cursor: List;
	BEGIN
		NEW(cursor); cursor.index := index; cursor.next := list;
		list := cursor
	END AddNeighbour;

	PROCEDURE RemoveNeighbour (VAR list: List; index: INTEGER);
		VAR
			cursor: List;
	BEGIN
		cursor := list;
		WHILE (cursor # NIL) & (cursor.index # index) DO cursor := cursor.next END;
		IF cursor # NIL THEN cursor.index := - index END
	END RemoveNeighbour;

	PROCEDURE (adjacency: Adjacency) AreNeighbours* (index0, index1: INTEGER): BOOLEAN, NEW;
		VAR
			cursor: List;
	BEGIN
		cursor := adjacency.neighbours[index0 - 1];
		WHILE (cursor # NIL) & (cursor.index # index1) DO cursor := cursor.next END;
		RETURN cursor # NIL
	END AreNeighbours;

	PROCEDURE (adjacency: Adjacency) AddNeighbour* (index0, index1: INTEGER), NEW;
	BEGIN
		IF ~adjacency.AreNeighbours(index0, index1) THEN
			AddNeighbour(adjacency.neighbours[index0 - 1], index1)
		END;
		IF ~adjacency.AreNeighbours(index1, index0) THEN
			AddNeighbour(adjacency.neighbours[index1 - 1], index0)
		END
	END AddNeighbour;

	PROCEDURE (adjacency: Adjacency) RemoveNeighbour* (index0, index1: INTEGER), NEW;
	BEGIN
		RemoveNeighbour(adjacency.neighbours[index0 - 1], index1);
		RemoveNeighbour(adjacency.neighbours[index1 - 1], index0)
	END RemoveNeighbour;

	PROCEDURE (adjacency: Adjacency) NumNeighbours* (index: INTEGER): INTEGER, NEW;
		VAR
			cursor: List; num: INTEGER;
	BEGIN
		num := 0; cursor := adjacency.neighbours[index - 1];
		WHILE cursor # NIL DO
			IF cursor.index > 0 THEN INC(num) END;
			cursor := cursor.next
		END;
		RETURN num
	END NumNeighbours;

	PROCEDURE (adjacency: Adjacency) Externalize* (VAR wr: Stores.Writer), NEW;
		VAR
			i, len: INTEGER; cursor: List;
	BEGIN
		i := 0; len := LEN(adjacency.neighbours);
		WHILE i < len DO
			cursor := adjacency.neighbours[i];
			WHILE cursor # NIL DO
				IF cursor.index > 0 THEN wr.WriteInt(cursor.index) END;
				cursor := cursor.next
			END;
			wr.WriteInt( - 1);
			INC(i)
		END
	END Externalize;

	PROCEDURE (adjacency: Adjacency) Internalize* (VAR rd: Stores.Reader), NEW;
		VAR
			i, len, index: INTEGER; cursor: List;
	BEGIN
		i := 0; len := LEN(adjacency.neighbours);
		WHILE i < len DO
			cursor := NIL;
			rd.ReadInt(index);
			WHILE index > 0 DO
				AddNeighbour(cursor, index); rd.ReadInt(index)
			END;
			adjacency.neighbours[i] := cursor;
			INC(i)
		END
	END Internalize;

	PROCEDURE (adjacency: Adjacency) CopyFrom* (source: Adjacency), NEW;
		VAR
			i, len: INTEGER; cursor, cursorS: List;
	BEGIN
		i := 0; len := LEN(adjacency.neighbours);
		WHILE i < len DO
			cursor := NIL; cursorS := source.neighbours[i];
			WHILE cursorS # NIL DO
				IF cursorS.index > 0 THEN AddNeighbour(cursor, cursorS.index) END;
				cursorS := cursorS.next
			END;
			adjacency.neighbours[i] := cursor;
			INC(i)
		END
	END CopyFrom;

	PROCEDURE New* (map: MapsMap.Map): Adjacency;
		VAR
			i, numRegions: INTEGER; adjacency: Adjacency;
	BEGIN
		NEW(adjacency);
		i := 0; numRegions := map.numReg;
		NEW(adjacency.neighbours, numRegions);
		WHILE i < numRegions DO adjacency.neighbours[i] := NIL; INC(i) END;
		RETURN adjacency
	END New;

	PROCEDURE NewNN* (map: MapsMap.Map): Adjacency;
		VAR
			i, j, numRegions: INTEGER; adjacency: Adjacency;
	BEGIN
		NEW(adjacency); adjacency.radius := 0.0;
		i := 0; numRegions := map.numReg; NEW(adjacency.neighbours, numRegions);
		WHILE i < numRegions DO
			adjacency.neighbours[i] := NIL; INC(i)
		END;
		i := 0;
		WHILE i < numRegions DO
			j := 0;
			WHILE j < numRegions DO
				IF (i # j) & IsOverlaping(map.regions[i], map.regions[j]) & IsTouching(map.regions[i], map.regions[j]) THEN
					(*AddNeighbour(adjacency.neighbours[i], j + 1)*)
					adjacency.AddNeighbour(i + 1, j + 1)
				END;
				INC(j)
			END;
			INC(i)
		END;
		RETURN adjacency
	END NewNN;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END MapsAdjacency.
