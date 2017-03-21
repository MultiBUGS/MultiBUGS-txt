(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		 *)

MODULE MapsIndex;


	

	IMPORT
		Dialog, Files,
		MapsMap;

	CONST
		mapDir = "Maps/Rsrc";

	TYPE
		List = POINTER TO RECORD
			map: MapsMap.Map;
			next: List
		END;

	VAR
		mapList: List;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Find* (IN name: ARRAY OF CHAR): MapsMap.Map;
		VAR
			cursor: List; map: MapsMap.Map;
	BEGIN
		cursor := mapList;
		map := NIL;
		WHILE (cursor # NIL) & (cursor.map.name # name) DO
			cursor := cursor.next
		END;
		IF cursor # NIL THEN
			map := cursor.map
		END;
		RETURN map
	END Find;

	PROCEDURE Store* (map: MapsMap.Map);
		VAR
			list: List;
	BEGIN
		ASSERT(map # NIL, 20);
		NEW(list);
		list.map := map;
		list.next := mapList;
		mapList := list
	END Store;

	PROCEDURE NumberOfMaps (): INTEGER;
		VAR
			loc: Files.Locator;
			fileList: Files.FileInfo;
			num: INTEGER;
	BEGIN
		loc := Files.dir.This(mapDir);
		fileList := Files.dir.FileList(loc);
		num := 0;
		WHILE fileList # NIL DO
			IF fileList.type = "map" THEN
				INC(num)
			END;
			fileList := fileList.next
		END;
		RETURN num
	END NumberOfMaps;

	PROCEDURE GetMaps* (): POINTER TO ARRAY OF Dialog.String;
		VAR
			i, j, num: INTEGER;
			loc: Files.Locator;
			fileList: Files.FileInfo;
			mapNames: POINTER TO ARRAY OF Dialog.String;
	BEGIN
		num := NumberOfMaps();
		IF num > 0 THEN
			NEW(mapNames, num)
		ELSE
			mapNames := NIL
		END;
		loc := Files.dir.This(mapDir);
		fileList := Files.dir.FileList(loc);
		i := 0;
		WHILE fileList # NIL DO
			IF fileList.type = "map" THEN
				mapNames[i] := fileList.name$;
				j := 0;
				WHILE mapNames[i, j] # 0X DO
					INC(j)
				END;
				mapNames[i, j - 4] := 0X;
				INC(i)
			END;
			fileList := fileList.next
		END;
		RETURN mapNames
	END GetMaps;

	PROCEDURE Exists* (IN name: ARRAY OF CHAR): BOOLEAN;
		VAR
			fileName: ARRAY 80 OF CHAR;
			loc: Files.Locator;
			fileList: Files.FileInfo;
	BEGIN
		loc := Files.dir.This(mapDir);
		fileList := Files.dir.FileList(loc);
		fileName := name + ".map";
		WHILE (fileList # NIL) & ((fileList.name # fileName) OR (fileList.type # "map")) DO
			fileList := fileList.next
		END;
		RETURN fileList # NIL
	END Exists;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		mapList := NIL
	END Init;

BEGIN
	Init
END MapsIndex.
