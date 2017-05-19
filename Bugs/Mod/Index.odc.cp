(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsIndex;


	

	IMPORT SYSTEM,
		Stores, Strings, 
		BugsNames,
		GraphNodes;

	TYPE
		Index = POINTER TO LIMITED RECORD
			name: BugsNames.Name;
			next: Index
		END;

	VAR
		index: Index;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Accept* (v: BugsNames.Visitor);
		VAR
			cursor: Index;
			name: BugsNames.Name;
	BEGIN
		cursor := index;
		WHILE cursor # NIL DO
			name := cursor.name;
			name.Accept(v);
			cursor := cursor.next
		END
	END Accept;

	PROCEDURE Store* (name: BugsNames.Name);
		VAR
			cursor, link: Index;
	BEGIN
		NEW(link);
		link.name := name;
		IF (index = NIL) OR (name.string < index.name.string) THEN
			link.next := index;
			index := link
		ELSE
			cursor := index;
			LOOP
				IF cursor.next = NIL THEN
					link.next := NIL;
					cursor.next := link;
					EXIT
				ELSIF name.string < cursor.next.name.string THEN
					link.next := cursor.next;
					cursor.next := link;
					EXIT
				END;
				cursor := cursor.next
			END
		END
	END Store;

	PROCEDURE NumberNodes* (): INTEGER;
		VAR
			num: INTEGER;
			cursor: Index;
			name: BugsNames.Name;
	BEGIN
		num := 0;
		cursor := index;
		WHILE cursor # NIL DO
			name := cursor.name;
			IF name.components # NIL THEN
				IF name.string # "deviance" THEN
					INC(num, LEN(name.components))
				END
			END;
			cursor := cursor.next
		END;
		RETURN num
	END NumberNodes;

	PROCEDURE ExternalizeNames* (VAR wr: Stores.Writer);
		VAR
			numNames: INTEGER;
			cursor: Index;
			name: BugsNames.Name;
	BEGIN
		cursor := index;
		numNames := 0;
		WHILE cursor # NIL DO
			name := cursor.name;
			IF name.string # "deviance" THEN
				INC(numNames)
			END;
			cursor := cursor.next
		END;
		wr.WriteInt(numNames);
		(*	externalize name info	*)
		cursor := index;
		WHILE cursor # NIL DO
			name := cursor.name;
			IF name.string # "deviance" THEN
				name.ExternalizeName(wr)
			END;
			cursor := cursor.next
		END;
	END ExternalizeNames;

	PROCEDURE ExternalizePointers* (VAR wr: Stores.Writer);
		VAR
			cursor: Index;
			name: BugsNames.Name;
			num: INTEGER;
	BEGIN
		num := NumberNodes();
		wr.WriteInt(num);
		cursor := index;
		WHILE cursor # NIL DO
			name := cursor.name;
			IF name.string # "deviance" THEN
				name.ExternalizePointers(wr)
			END;
			cursor := cursor.next
		END
	END ExternalizePointers;


	PROCEDURE ExternalizeData* (VAR wr: Stores.Writer);
		VAR
			cursor: Index;
			name: BugsNames.Name;
	BEGIN
		cursor := index;
		WHILE cursor # NIL DO
			name := cursor.name;
			IF name.string # "deviance" THEN
				name.ExternalizeData(wr)
			END;
			cursor := cursor.next
		END
	END ExternalizeData;

	PROCEDURE InternalizeNames* (VAR rd: Stores.Reader);
		VAR
			i, numNames: INTEGER;
			name: BugsNames.Name;
	BEGIN
		rd.ReadInt(numNames);
		index := NIL;
		i := 0;
		WHILE i < numNames DO
			name := BugsNames.New("", 0);
			name.InternalizeName(rd);
			Store(name);
			INC(i)
		END;
	END InternalizeNames;

	PROCEDURE InternalizePointers* (VAR rd: Stores.Reader);
		VAR
			cursor: Index;
			name: BugsNames.Name;
			p: GraphNodes.Node;
			num: INTEGER;
	BEGIN
		rd.ReadInt(num);
		cursor := index;
		WHILE cursor # NIL DO
			name := cursor.name;
			name.InternalizePointers(rd);
			cursor := cursor.next
		END
	END InternalizePointers;

	PROCEDURE Find* (IN string: ARRAY OF CHAR): BugsNames.Name;
		VAR
			cursor: Index;
			name: BugsNames.Name;
	BEGIN
		cursor := index;
		WHILE (cursor # NIL) & (cursor.name.string # string) DO
			cursor := cursor.next
		END;
		IF cursor # NIL THEN
			name := cursor.name
		ELSE
			name := NIL
		END;
		RETURN name
	END Find;

	PROCEDURE FindByNumber* (number: INTEGER): BugsNames.Name;
		VAR
			i: INTEGER;
			cursor: Index;
			name: BugsNames.Name;
	BEGIN
		cursor := index;
		i := 0;
		WHILE (cursor # NIL) & (i < number) DO
			cursor := cursor.next;
			INC(i)
		END;
		IF (cursor # NIL) & (i <= number) THEN
			name := cursor.name
		ELSE
			name := NIL
		END;
		RETURN name
	END FindByNumber;

	PROCEDURE MapGraphAddress* (adr: INTEGER; OUT label: ARRAY OF CHAR);
		VAR
			cursor: Index;
			name: BugsNames.Name;
			i, len: INTEGER;
	BEGIN
		cursor := index;
		LOOP
			IF cursor = NIL THEN
				Strings.IntToStringForm(adr, Strings.hexadecimal, 9, "0", Strings.showBase, label);
				label := "[" + label + "]";
				EXIT
			END;
			name := cursor.name;
			IF name.components # NIL THEN
				len := name.Size();
				i := 0;
				WHILE (i < len) & 
					((name.components[i] = NIL) OR (SYSTEM.VAL(INTEGER, name.components[i]) # adr)) DO
					INC(i)
				END;
				IF i < len THEN
					name.Indices(i, label);
					label := "<" + name.string + label + ">";
					EXIT
				END
			END;
			cursor := cursor.next
		END;
	END MapGraphAddress;

	PROCEDURE FindGraphNode* (p: GraphNodes.Node; OUT label: ARRAY OF CHAR);
	BEGIN
		MapGraphAddress(SYSTEM.VAL(INTEGER, p), label)
	END FindGraphNode;

	PROCEDURE NumberOfNames (): INTEGER;
		VAR
			num: INTEGER;
			cursor: Index;
	BEGIN
		cursor := index;
		num := 0;
		WHILE cursor # NIL DO
			INC(num);
			cursor := cursor.next
		END;
		RETURN num
	END NumberOfNames;

	PROCEDURE GetNames* (): POINTER TO ARRAY OF BugsNames.Name;
		VAR
			i, num: INTEGER;
			cursor: Index;
			names: POINTER TO ARRAY OF BugsNames.Name;
	BEGIN
		num := NumberOfNames();
		IF num > 0 THEN
			NEW(names, num)
		ELSE
			names := NIL
		END;
		cursor := index;
		i := 0;
		WHILE cursor # NIL DO
			names[i] := cursor.name;
			INC(i);
			cursor := cursor.next
		END;
		RETURN names
	END GetNames;

	PROCEDURE MakeLabel* (IN scalar: ARRAY OF CHAR; offset: INTEGER; OUT label: ARRAY OF CHAR);
		VAR
			string: ARRAY 128 OF CHAR;
			i: INTEGER;
			name: BugsNames.Name;
	BEGIN
		i := 0; WHILE (scalar[i] # 0X) & (scalar[i] # "[") DO string[i] := scalar[i]; INC(i) END;
		string[i] := 0X;
		label := string$;
		name := Find(string);
		IF name # NIL THEN name.Indices(offset, string); label := label + string END
	END MakeLabel;

	PROCEDURE Clear*;
	BEGIN
		index := NIL
	END Clear;

	PROCEDURE Exists* (): BOOLEAN;
	BEGIN
		RETURN index # NIL
	END Exists;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		index := NIL
	END Init;

BEGIN
	Init
END BugsIndex.
