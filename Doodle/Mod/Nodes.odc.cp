(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	   *)

MODULE DoodleNodes;


	

	IMPORT
		Dialog, Stores;

	CONST
		constant* = 2;
		logical* = 1;
		stochastic* = 0;
		name* = 5;
		value* = 6;
		ldel = 08X;
		rdel = 07X;

	TYPE
		Parents* = POINTER TO ARRAY OF Node;

		Node* = POINTER TO LIMITED RECORD
			name-: Dialog.String;
			type-, density-, link-: INTEGER;
			defaults-: ARRAY 5 OF Dialog.String;
			value-: Dialog.String;
			label-, string-, index-: INTEGER;
			mask-: SET;
			parents-: Parents;
			x-, y-: INTEGER;
			selected-: BOOLEAN
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;


	PROCEDURE (node: Node) CopyFrom* (source: Node; nodes: Parents), NEW;
		VAR
			i, label, len: INTEGER;
			parents: Parents;
	BEGIN
		node.name := source.name;
		node.type := source.type;
		node.density := source.density;
		node.link := source.link;
		i := 0;
		WHILE i < LEN(node.defaults) DO
			node.defaults[i] := source.defaults[i];
			INC(i)
		END;
		node.value := source.value;
		node.label := source.label;
		node.string := source.string;
		node.index := source.index;
		node.mask := source.mask;
		node.x := source.x;
		node.y := source.y;
		node.selected := source.selected;
		parents := source.parents;
		IF parents # NIL THEN
			len := LEN(parents^);
			NEW(node.parents, len);
			i := 0;
			WHILE i < len DO
				IF parents[i] # NIL THEN
					label := parents[i].label;
					node.parents[i] := nodes[label]
				ELSE
					node.parents[i] := NIL
				END;
				INC(i)
			END
		END
	END CopyFrom;

	PROCEDURE (node: Node) ForceToGrid* (grid: INTEGER), NEW;
	BEGIN
		node.x := ((node.x + grid DIV 2) DIV grid) * grid;
		node.y := ((node.y + grid DIV 2) DIV grid) * grid
	END ForceToGrid;

	PROCEDURE (node: Node) Move* (deltaX, deltaY: INTEGER), NEW;
	BEGIN
		node.x := node.x + deltaX;
		node.y := node.y + deltaY
	END Move;

	PROCEDURE EditString (VAR s: ARRAY OF CHAR; ch: CHAR; VAR index: INTEGER);
		VAR
			i, len: INTEGER;
	BEGIN
		len := 0; WHILE s[len] # 0X DO INC(len) END;
		CASE ch OF
		|ldel:
			IF index > 0 THEN
				i := index;
				WHILE i < len DO s[i - 1] := s[i]; INC(i) END;
				DEC(index);
				DEC(len);
				s[len] := 0X
			END
		|rdel:
			i := index;
			WHILE i < len DO s[i] := s[i + 1]; INC(i) END
		ELSE
			IF len < LEN(s) - 1 THEN
				i := len;
				WHILE i >= index DO s[i + 1] := s[i]; DEC(i) END;
				s[index] := ch;
				INC(index)
			END
		END
	END EditString;

	PROCEDURE (node: Node) PasteChar* (ch: CHAR), NEW;
	BEGIN
		CASE node.string OF
		|name:
			EditString(node.name, ch, node.index)
		|value:
			EditString(node.value, ch, node.index)
		ELSE
			EditString(node.defaults[node.string], ch, node.index)
		END
	END PasteChar;

	PROCEDURE (node: Node) Read* (rd: Stores.Reader; parents: Parents), NEW;
		VAR
			i, label, numParents: INTEGER;
	BEGIN
		rd.ReadString(node.name);
		rd.ReadInt(node.type);
		rd.ReadInt(node.density);
		rd.ReadInt(node.link);
		i := 0;
		WHILE i < LEN(node.defaults) DO rd.ReadString(node.defaults[i]); INC(i) END;
		rd.ReadString(node.value);
		rd.ReadInt(node.string);
		rd.ReadInt(node.index);
		rd.ReadBool(node.selected);
		rd.ReadSet(node.mask);
		rd.ReadInt(node.x);
		rd.ReadInt(node.y);
		rd.ReadInt(numParents);
		IF numParents > 0 THEN
			NEW(node.parents, numParents)
		ELSE
			node.parents := NIL
		END;
		i := 0;
		WHILE i < numParents DO
			rd.ReadInt(label);
			IF label # - 1 THEN
				node.parents[i] := parents[label]
			ELSE
				node.parents[i] := NIL
			END;
			INC(i)
		END
	END Read;

	PROCEDURE (node: Node) RoundToGrid* (grid: INTEGER), NEW;
	BEGIN
		node.x := (node.x DIV grid) * grid;
		node.y := (node.y DIV grid) * grid
	END RoundToGrid;

	PROCEDURE (node: Node) Scale* (scale: REAL), NEW;
	BEGIN
		node.x := SHORT(ENTIER(node.x * scale));
		node.y := SHORT(ENTIER(node.y * scale))
	END Scale;

	PROCEDURE (node: Node) Select* (selected: BOOLEAN), NEW;
	BEGIN
		node.selected := selected
	END Select;

	PROCEDURE (node: Node) Set* (type, density, link, x, y: INTEGER; mask: SET;
	IN defaults: ARRAY OF Dialog.String), NEW;
		VAR
			i, len: INTEGER;
			parents: Parents;
	BEGIN
		len := 31;
		WHILE (len > 0) & ~(len IN mask) DO DEC(len) END;
		INC(len);
		IF len > 0 THEN
			NEW(parents, len)
		ELSE
			parents := NIL
		END;
		i := 0;
		WHILE i < len DO parents[i] := NIL; INC(i) END;
		IF node.parents = NIL THEN
			len := 0
		ELSIF len > LEN(node.parents) THEN
			len := LEN(node.parents)
		END;
		i := 0;
		WHILE i < len DO
			IF i IN mask THEN parents[i] := node.parents[i] END;
			INC(i)
		END;
		IF node.type # type THEN node.value := "" END;
		node.type := type;
		node.density := density;
		node.link := link;
		node.x := x;
		node.y := y;
		node.mask := mask;
		node.parents := parents;
		node.string := name;
		i := 0;
		WHILE i < LEN(defaults) DO node.defaults[i] := defaults[i]$; INC(i) END;
		node.index := LEN(node.name$)
	END Set;

	PROCEDURE (node: Node) SetCaret* (string, index: INTEGER), NEW;
		VAR
			len: INTEGER;
	BEGIN
		node.string := string;
		CASE string OF
		|name:
			len := 0; WHILE node.name[len] # 0X DO INC(len) END;
		|value:
			len := 0; WHILE node.value[len] # 0X DO INC(len) END;
		|0..4:
			len := 0; WHILE node.defaults[string, len] # 0X DO INC(len) END
		END;
		IF index > len THEN index := len END;
		node.index := index
	END SetCaret;

	PROCEDURE (node: Node) SetLabel* (label: INTEGER), NEW;
	BEGIN
		node.label := label
	END SetLabel;

	PROCEDURE (node: Node) SetEdge* (parent: Node), NEW;
		VAR
			i, len: INTEGER;
	BEGIN
		IF node.parents # NIL THEN
			len := LEN(node.parents^)
		ELSE
			len := 0
		END;
		i := 0;
		WHILE (i < len) & (node.parents[i] # parent) DO
			INC(i)
		END;
		IF i < len THEN
			node.parents[i] := NIL
		ELSE
			i := 0;
			WHILE (i < len) & (~(i IN node.mask) OR (node.parents[i] # NIL)) DO
				INC(i)
			END;
			IF i < len THEN
				node.parents[i] := parent;
				node.string := name
			END
		END
	END SetEdge;

	PROCEDURE (node: Node) SwapParents* (param0, param1: INTEGER), NEW;
		VAR
			temp: Node;
	BEGIN
		temp := node.parents[param0];
		node.parents[param0] := node.parents[param1];
		node.parents[param1] := temp
	END SwapParents;

	PROCEDURE (node: Node) Write* (wr: Stores.Writer), NEW;
		VAR
			i, numParents: INTEGER;
	BEGIN
		wr.WriteString(node.name);
		wr.WriteInt(node.type);
		wr.WriteInt(node.density);
		wr.WriteInt(node.link);
		i := 0;
		WHILE i < LEN(node.defaults) DO
			wr.WriteString(node.defaults[i]);
			INC(i)
		END;
		wr.WriteString(node.value);
		wr.WriteInt(node.string);
		wr.WriteInt(node.index);
		wr.WriteBool(node.selected);
		wr.WriteSet(node.mask);
		wr.WriteInt(node.x);
		wr.WriteInt(node.y);
		IF node.parents = NIL THEN
			numParents := 0
		ELSE
			numParents := LEN(node.parents^)
		END;
		wr.WriteInt(numParents);
		i := 0;
		WHILE i < numParents DO
			IF node.parents[i] # NIL THEN
				wr.WriteInt(node.parents[i].label)
			ELSE
				wr.WriteInt( - 1)
			END;
			INC(i)
		END
	END Write;

	PROCEDURE New* (): Node;
		VAR
			i: INTEGER;
			node: Node;
	BEGIN
		NEW(node);
		node.name := "";
		node.type := - 1;
		node.density := - 1;
		node.link := - 1;
		i := 0;
		WHILE i < LEN(node.defaults) DO
			node.defaults[i] := "";
			INC(i)
		END;
		node.label := - 1;
		node.string := - 1;
		node.index := - 1;
		node.parents := NIL;
		node.mask := {};
		node.x := - 1;
		node.y := - 1;
		node.selected := FALSE;
		RETURN node
	END New;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END DoodleNodes.
