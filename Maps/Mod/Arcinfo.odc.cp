(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		 *)

MODULE MapsArcinfo;


	

	IMPORT
		Strings, 
		BugsMappers, BugsMsg, 
		MapsImporter, MapsMap;

	TYPE
		Importer = POINTER TO RECORD(MapsImporter.Importer) END;

		Factory = POINTER TO RECORD(MapsImporter.Factory) END;

	VAR
		fact-: MapsImporter.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Error (errorNum: INTEGER);
		VAR
			numToString: ARRAY 8 OF CHAR;
			errorMes: ARRAY 1024 OF CHAR;
	BEGIN
		Strings.IntToString(errorNum, numToString);
		BugsMsg.MapMsg("MapsArcinfo" + numToString, errorMes);
		BugsMsg.StoreError(errorMes)
	END Error;

	PROCEDURE FindPolygon (map: MapsMap.Map; id: INTEGER;
	OUT region: MapsMap.Region; OUT polygon: MapsMap.Polygon);
		VAR
			i: INTEGER;
	BEGIN
		i := 0;
		LOOP
			region := map.regions[i];
			polygon := region.outline;
			WHILE (polygon # NIL) & (polygon.id # id) DO
				polygon := polygon.next
			END;
			IF polygon # NIL THEN
				EXIT
			ELSE
				INC(i);
				IF i = LEN(map.regions) THEN
					RETURN
				END
			END
		END
	END FindPolygon;

	PROCEDURE (imp: Importer) Load (VAR s: BugsMappers.Scanner): MapsMap.Map;
		CONST
			maxPoints = 10000;
		VAR
			i, numReg, id, pos: INTEGER;
			centreX, centreY, xScale, yScale: REAL;
			name: ARRAY 80 OF CHAR;
			x, y: ARRAY maxPoints OF REAL;
			region: MapsMap.Region;
			polygon: MapsMap.Polygon;
			map: MapsMap.Map;
	BEGIN
		s.Scan;
		IF (s.type # BugsMappers.string) OR (s.string # "map") THEN Error(1); RETURN NIL END;
		s.Scan;
		IF (s.type # BugsMappers.char) OR (s.char # ":") THEN Error(2); RETURN NIL END;
		s.Scan;
		IF s.type # BugsMappers.int THEN Error(3); RETURN NIL END;
		numReg := s.int;
		map := MapsMap.NewMap(numReg);
		pos := s.Pos();
		s.Scan;
		IF (s.type = BugsMappers.string) & (s.string = "Xscale") THEN
			s.Scan;
			IF (s.type # BugsMappers.char) OR (s.char # ":") THEN RETURN NIL END;
			s.Scan;
			IF s.type = BugsMappers.real THEN
				xScale := s.real
			ELSIF s.type = BugsMappers.int THEN
				xScale := s.int
			ELSE
				Error(21);
				RETURN NIL
			END
		ELSE
			xScale := 1.0; s.SetPos(pos)
		END;
		pos := s.Pos();
		s.Scan;
		IF (s.type = BugsMappers.string) & (s.string = "Yscale") THEN
			s.Scan;
			IF (s.type # BugsMappers.char) OR (s.char # ":") THEN Error(2); RETURN NIL END;
			s.Scan;
			IF s.type = BugsMappers.real THEN
				yScale := s.real
			ELSIF s.type = BugsMappers.int THEN
				yScale := s.int
			ELSE
				Error(21);
				RETURN NIL
			END
		ELSE
			yScale := 1.0;
			s.SetPos(pos)
		END;
		i := 0;
		WHILE i < numReg DO
			s.Scan;
			IF s.type # BugsMappers.int THEN Error(4); RETURN NIL END;
			id := s.int;
			s.Scan;
			IF s.type = BugsMappers.string THEN
				name := s.string$
			ELSIF s.type = BugsMappers.int THEN
				Strings.IntToString(s.int, name)
			ELSE
				Error(5); RETURN NIL
			END;
			region := MapsMap.NewRegion(name, id);
			map.AddRegion(region);
			INC(i)
		END;
		i := 0;
		WHILE (i < numReg) & (map.regions[i] # NIL) DO INC(i) END;
		IF i < numReg THEN Error(6); RETURN NIL END;
		s.Scan;
		IF (s.type # BugsMappers.string) OR (s.string # "regions") THEN Error(7); RETURN NIL END;
		s.Scan;
		WHILE s.type = BugsMappers.int DO
			id := s.int;
			s.Scan;
			IF s.type # BugsMappers.string THEN Error(8); RETURN NIL END;
			name := s.string$;
			i := 0;
			WHILE i < maxPoints DO
				x[i] := 0.0;
				y[i] := 0.0;
				INC(i)
			END;
			polygon := MapsMap.NewPolygon(id, 0, 0.0, 0.0, x, y);
			region := map.FindRegion(name);
			IF region = NIL THEN Error(9); RETURN NIL END;
			region.AddPolygon(polygon);
			s.Scan
		END;
		IF (s.type # BugsMappers.string) OR (s.string # "END") THEN Error(10); RETURN NIL END;
		s.Scan;
		WHILE s.type # BugsMappers.string DO
			IF s.type = BugsMappers.char THEN
				IF s.char # "-" THEN Error(11); RETURN NIL END;
				s.Scan; IF s.type # BugsMappers.int THEN Error(12); RETURN NIL END;
				id := - s.int; centreX := 0.0; centreY := 0.0
			ELSE
				IF s.type # BugsMappers.int THEN Error(13); RETURN NIL END;
				id := s.int;
				IF id > 0 THEN
					FindPolygon(map, id, region, polygon);
					IF polygon = NIL THEN Error(19); RETURN NIL END
				END;
				s.Scan;
				IF s.type = BugsMappers.real THEN centreX := s.real * xScale
				ELSIF s.type = BugsMappers.int THEN centreX := s.int * xScale
				ELSE Error(14); RETURN NIL
				END;
				s.Scan;
				IF s.type = BugsMappers.real THEN centreY := s.real * yScale
				ELSIF s.type = BugsMappers.int THEN centreY := s.int * yScale
				ELSE Error(15); RETURN NIL
				END
			END;
			s.Scan; i := 0;
			WHILE s.type # BugsMappers.string DO
				IF s.type = BugsMappers.real THEN x[i] := s.real * xScale
				ELSIF s.type = BugsMappers.int THEN x[i] := s.int * xScale
				ELSE Error(16); RETURN NIL
				END;
				s.Scan;
				IF s.type = BugsMappers.real THEN y[i] := s.real * yScale
				ELSIF s.type = BugsMappers.int THEN y[i] := s.int * yScale
				ELSE Error(17); RETURN NIL
				END;
				s.Scan;
				INC(i)
			END;
			IF s.string # "END" THEN Error(18); RETURN NIL END;
			IF id > 0 THEN
				polygon.Set(id, i, centreX, centreY, x, y)
			ELSIF id < 0 THEN
				polygon := MapsMap.NewPolygon(id, i, centreX, centreY, x, y);
				region.AddInternalPolygon(polygon)
			END;
			s.Scan
		END;
		IF s.string # "END" THEN Error(20); RETURN NIL END;
		RETURN map
	END Load;

	PROCEDURE (f: Factory) New (): MapsImporter.Importer;
		VAR
			importer: Importer;
	BEGIN
		NEW(importer);
		RETURN importer
	END New;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Install*;
	BEGIN
		MapsImporter.SetFactory(fact)
	END Install;

	PROCEDURE Init;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f
	END Init;

BEGIN
	Init
END MapsArcinfo.
