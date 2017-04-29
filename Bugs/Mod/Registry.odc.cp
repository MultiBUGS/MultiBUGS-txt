(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsRegistry;


	

	IMPORT
		Files, Stores;

	TYPE
		Item = POINTER TO ABSTRACT RECORD
			key: POINTER TO ARRAY OF CHAR
		END;

		Bool = POINTER TO RECORD(Item)
			x: BOOLEAN
		END;

		Int = POINTER TO RECORD(Item)
			x: INTEGER
		END;

		Set = POINTER TO RECORD(Item)
			x: SET
		END;

		Str = POINTER TO RECORD(Item)
			x: POINTER TO ARRAY OF CHAR
		END;

		ItemList = POINTER TO RECORD
			item: Item;
			next: ItemList
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		registry: ItemList;
		res-: INTEGER;
		
	PROCEDURE ReadBool* (IN key: ARRAY OF CHAR; OUT x: BOOLEAN; OUT res: INTEGER);
		VAR
			cursor: ItemList;
	BEGIN
		cursor := registry;
		WHILE (cursor # NIL) & (cursor.item.key^ # key) DO
			cursor := cursor.next
		END;
		IF (cursor # NIL) & (cursor.item IS Bool) THEN
			res := 0;
			x := cursor.item(Bool).x
		ELSE
			res := 1
		END
	END ReadBool;

	PROCEDURE ReadInt* (IN key: ARRAY OF CHAR; OUT x, res: INTEGER);
		VAR
			cursor: ItemList;
	BEGIN
		cursor := registry;
		WHILE (cursor # NIL) & (cursor.item.key^ # key) DO
			cursor := cursor.next
		END;
		IF (cursor # NIL) & (cursor.item IS Int) THEN
			res := 0;
			x := cursor.item(Int).x
		ELSE
			res := 1
		END
	END ReadInt;

	PROCEDURE ReadSet* (IN key: ARRAY OF CHAR; OUT x: SET; OUT res: INTEGER);
		VAR
			cursor: ItemList;
	BEGIN
		cursor := registry;
		WHILE (cursor # NIL) & (cursor.item.key^ # key) DO
			cursor := cursor.next
		END;
		IF (cursor # NIL) & (cursor.item IS Set) THEN
			res := 0;
			x := cursor.item(Set).x
		ELSE
			res := 1
		END
	END ReadSet;

	PROCEDURE ReadString* (IN key: ARRAY OF CHAR; OUT str: ARRAY OF CHAR; OUT res: INTEGER);
		VAR
			i, len0, len1: INTEGER;
			cursor: ItemList;
	BEGIN
		cursor := registry;
		WHILE (cursor # NIL) & (cursor.item.key^ # key) DO
			cursor := cursor.next
		END;
		IF (cursor # NIL) & (cursor.item IS Str) THEN
			res := 0;
			len0 := LEN(cursor.item(Str).x);
			len1 := LEN(str);
			IF len1 > len0 THEN
				i := 0; WHILE i < len0 DO str[i] := cursor.item(Str).x[i]; INC(i) END;
				str[i] := 0X
			ELSE
				res := 1
			END
		ELSE
			res := 1
		END
	END ReadString;

	PROCEDURE FindItem (IN key: ARRAY OF CHAR): ItemList;
		VAR
			cursor: ItemList;
	BEGIN
		cursor := registry;
		WHILE (cursor # NIL) & (cursor.item.key^ # key) DO
			cursor := cursor.next
		END;
		RETURN cursor
	END FindItem;

	PROCEDURE WriteBool* (IN key: ARRAY OF CHAR; x: BOOLEAN);
		VAR
			i, len: INTEGER;
			bool: Bool;
			itemList: ItemList;
	BEGIN
		itemList := FindItem(key);
		IF itemList = NIL THEN
			NEW(bool);
			bool.x := x;
			i := 0; WHILE key[i] # 0X DO INC(i) END;
			len := i + 1;
			NEW(bool.key, len);
			i := 0; WHILE i < len DO bool.key[i] := key[i]; INC(i) END;
			NEW(itemList);
			itemList.item := bool;
			itemList.next := registry;
			registry := itemList
		ELSE
			itemList.item(Bool).x := x
		END
	END WriteBool;

	PROCEDURE WriteInt* (IN key: ARRAY OF CHAR; x: INTEGER);
		VAR
			i, len: INTEGER;
			int: Int;
			itemList: ItemList;
	BEGIN
		itemList := FindItem(key);
		IF itemList = NIL THEN
			NEW(int);
			int.x := x;
			i := 0; WHILE key[i] # 0X DO INC(i) END;
			len := i + 1;
			NEW(int.key, len);
			i := 0; WHILE i < len DO int.key[i] := key[i]; INC(i) END;
			NEW(itemList);
			itemList.item := int;
			itemList.next := registry;
			registry := itemList
		ELSE
			itemList.item(Int).x := x
		END
	END WriteInt;

	PROCEDURE WriteSet* (IN key: ARRAY OF CHAR; x: SET);
		VAR
			i, len: INTEGER;
			set: Set;
			itemList: ItemList;
	BEGIN
		itemList := FindItem(key);
		IF itemList = NIL THEN
			NEW(set);
			set.x := x;
			i := 0; WHILE key[i] # 0X DO INC(i) END;
			len := i + 1;
			NEW(set.key, len);
			i := 0; WHILE i < len DO set.key[i] := key[i]; INC(i) END;
			NEW(itemList);
			itemList.item := set;
			itemList.next := registry;
			registry := itemList
		ELSE
			itemList.item(Set).x := x
		END
	END WriteSet;

	PROCEDURE WriteString* (IN key: ARRAY OF CHAR; IN x: ARRAY OF CHAR);
		VAR
			i, len: INTEGER;
			str: Str;
			itemList: ItemList;
	BEGIN
		itemList := FindItem(key);
		IF itemList = NIL THEN
			NEW(str);
			i := 0; WHILE key[i] # 0X DO INC(i) END;
			len := i + 1;
			NEW(str.key, len);
			i := 0; WHILE i < len DO str.key[i] := key[i]; INC(i) END;
			NEW(itemList);
			itemList.item := str;
			itemList.next := registry;
			registry := itemList
		ELSE
			str := itemList.item(Str)
		END;
		i := 0; WHILE x[i] # 0X DO INC(i) END;
		len := i + 1;
		NEW(str.x, len);
		WHILE i < len DO str.x[i] := x[i]; INC(i) END
	END WriteString;

	PROCEDURE Load*;
		VAR
			key: ARRAY 80 OF CHAR;
			loc: Files.Locator;
			f: Files.File;
			rd: Stores.Reader;
			name: Files.Name;
			string: ARRAY 128 OF CHAR;
			bool: BOOLEAN;
			int: INTEGER;
			set: SET;
	BEGIN
		loc := Files.dir.This("Bugs/Rsrc");
		IF loc = NIL THEN res := 2; RETURN END;
		name := "Registry";
		res := loc.res; IF res # 0 THEN RETURN END;
		f := Files.dir.Old(loc, name, Files.shared);
		IF f = NIL THEN res := 2; RETURN END;
		rd.ConnectTo(f);
		rd.SetPos(0);
		rd.ReadString(string);
		WHILE string # "END" DO
			key := string$;
			rd.ReadString(string);
			IF string = "Bool" THEN
				rd.ReadBool(bool);
				WriteBool(key, bool)
			ELSIF string = "Int" THEN
				rd.ReadInt(int);
				WriteInt(key, int)
			ELSIF string = "Set" THEN
				rd.ReadSet(set);
				WriteSet(key, set)
			ELSIF string = "String" THEN
				rd.ReadString(string);
				WriteString(key, string)
			ELSE
				HALT(100)
			END;
			rd.ReadString(string)
		END;
		ASSERT(string = "END", 66);
		f.Close
	END Load;

	PROCEDURE Store;
		VAR
			cursor: ItemList;
			item: Item;
			loc: Files.Locator;
			name: Files.Name;
			f: Files.File;
			wr: Stores.Writer;
			res: INTEGER;
	BEGIN
		loc := Files.dir.This("Bugs/Rsrc");
		name := "Registry";
		f := Files.dir.Old(loc, name, Files.shared);
		IF f # NIL THEN
			f.Close;
			f := NIL;
			Files.dir.Delete(loc, name);
			res := loc.res;
			IF res # 0 THEN RETURN END
		END;
		f := Files.dir.New(loc, Files.dontAsk);
		IF f = NIL THEN res := 2; RETURN END;
		wr.ConnectTo(f);
		wr.SetPos(0);
		cursor := registry;
		WHILE cursor # NIL DO
			item := cursor.item;
			wr.WriteString(item.key);
			WITH item: Bool DO
				wr.WriteString("Bool");
				wr.WriteBool(item.x);
			|item: Int DO
				wr.WriteString("Int");
				wr.WriteInt(item.x)
			|item: Set DO
				wr.WriteString("Set");
				wr.WriteSet(item.x)
			|item: Str DO
				wr.WriteString("Str");
				wr.WriteString(item.x)
			END;
			cursor := cursor.next
		END;
		wr.WriteString("END");
		f.Register(name, "txt", Files.dontAsk, res)
	END Store;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		registry := NIL;
		res := 0
	END Init;

BEGIN
	Init
CLOSE
	Store
END BugsRegistry.

