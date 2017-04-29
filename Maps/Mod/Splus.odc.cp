(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		 *)

MODULE MapsSplus;


	

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
		BugsMsg.Lookup("MapsSplus" + numToString, errorMes);
		BugsMsg.Store(errorMes)
	END Error;

	PROCEDURE (imp: Importer) Load (VAR s: BugsMappers.Scanner): MapsMap.Map;
		CONST
			maxPoints = 10000;
		VAR
			i, id, numReg, pos: INTEGER;
			xScale, yScale: REAL;
			negative: BOOLEAN;
			name, string: ARRAY 80 OF CHAR;
			x, y: ARRAY maxPoints OF REAL;
			region: MapsMap.Region;
			polygon: MapsMap.Polygon;
			map: MapsMap.Map;
	BEGIN
		s.Scan;
		IF (s.type # BugsMappers.string) OR (s.string # "map") THEN
			Error(1); RETURN NIL
		END;
		s.Scan;
		IF (s.type # BugsMappers.char) OR (s.char # ":") THEN
			Error(2); RETURN NIL
		END;
		s.Scan;
		IF s.type # BugsMappers.int THEN
			Error(3); RETURN NIL
		END;
		numReg := s.int;
		map := MapsMap.NewMap(numReg);
		pos := s.Pos();
		s.Scan;
		IF (s.type = BugsMappers.string) & (s.string = "Xscale") THEN
			s.Scan;
			IF (s.type # BugsMappers.char) OR (s.char # ":") THEN
				Error(2); RETURN NIL
			END;
			s.Scan;
			IF s.type = BugsMappers.real THEN
				xScale := s.real
			ELSIF s.type = BugsMappers.int THEN
				xScale := s.int
			ELSE
				Error(15); RETURN NIL
			END
		ELSE
			xScale := 1.0;
			s.SetPos(pos)
		END;
		pos := s.Pos();
		s.Scan;
		IF (s.type = BugsMappers.string) & (s.string = "Yscale") THEN
			s.Scan;
			IF (s.type # BugsMappers.char) OR (s.char # ":") THEN
				Error(2); RETURN NIL
			END;
			s.Scan;
			IF s.type = BugsMappers.real THEN
				yScale := s.real
			ELSIF s.type = BugsMappers.int THEN
				yScale := s.int
			ELSE
				Error(15); RETURN NIL
			END
		ELSE
			yScale := 1.0;
			s.SetPos(pos)
		END;
		i := 0;
		WHILE i < numReg DO
			s.Scan;
			IF s.type # BugsMappers.int THEN
				Error(4); RETURN NIL
			END;
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
		WHILE (i < numReg) & (map.regions[i] # NIL) DO
			INC(i)
		END;
		IF i < numReg THEN
			Error(6); RETURN NIL
		END;
		s.Scan;
		IF s.type = BugsMappers.string THEN
			name := s.string$
		ELSIF s.type = BugsMappers.int THEN
			Strings.IntToString(s.int, name)
		ELSE
			Error(7); RETURN NIL
		END;
		REPEAT
			region := map.FindRegion(name);
			IF region = NIL THEN
				Error(8); RETURN NIL
			END;
			i := 0;
			REPEAT
				s.Scan;
				IF (s.type = BugsMappers.char) & (s.char = "-") THEN
					negative := TRUE;
					s.Scan
				ELSIF (s.type = BugsMappers.char) & (s.char = "-") THEN
					negative := FALSE;
					s.Scan
				ELSE
					negative := FALSE
				END;
				IF s.type = BugsMappers.real THEN
					x[i] := s.real * xScale
				ELSIF s.type = BugsMappers.int THEN
					x[i] := s.int * xScale
				ELSE
					Error(9); RETURN NIL
				END;
				IF negative THEN x[i] := -x[i] END;
				s.Scan;
				IF (s.type = BugsMappers.char) & (s.char = "-") THEN
					negative := TRUE;
					s.Scan
				ELSIF (s.type = BugsMappers.char) & (s.char = "-") THEN
					negative := FALSE;
					s.Scan
				ELSE
					negative := FALSE
				END;
				IF s.type = BugsMappers.real THEN
					y[i] := s.real * yScale
				ELSIF s.type = BugsMappers.int THEN
					y[i] := s.int * yScale
				ELSE
					Error(10); RETURN NIL
				END;
				IF negative THEN y[i] := -y[i] END;
				INC(i);
				s.Scan;
				IF s.type = BugsMappers.string THEN
					string := s.string$
				ELSIF s.type = BugsMappers.int THEN
					Strings.IntToString(s.int, string)
				ELSE
					Error(11); RETURN NIL
				END
			UNTIL string # name;
			IF string = "NA" THEN
				s.Scan; IF (s.type # BugsMappers.string) OR (s.string # "NA") THEN
					Error(12); RETURN NIL
				END;
				s.Scan;
				IF (s.type # BugsMappers.string) OR (s.string # "NA") THEN
					Error(13); RETURN NIL
				END;
				s.Scan;
				IF s.type = BugsMappers.string THEN
					string := s.string$
				ELSIF s.type = BugsMappers.int THEN
					Strings.IntToString(s.int, string)
				ELSE
					Error(14); RETURN NIL
				END
			END;
			name := string;
			polygon := MapsMap.NewPolygon(0, i, 0.0, 0.0, x, y);
			region.AddPolygon(polygon)
		UNTIL string = "END";
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
END MapsSplus.
