(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsNames;

	

	(*	data structure to represent names in BUGS model	*)

	IMPORT
		Stores := Stores64, Strings,
		GraphConstant, GraphNodes, GraphStochastic;

	TYPE
		(*	strucure representing names in OpenBUGS model	*)
		Name* = POINTER TO LIMITED RECORD
			passByreference*: BOOLEAN;
			numSlots-: INTEGER; (*	number of indices	*)
			slotSizes-: POINTER TO ARRAY OF INTEGER; (*	range of each index	*)
			string-: ARRAY 128 OF CHAR; (*	name as a string	*)
			components-: GraphNodes.Vector; 	(*	nodes in graph	*)
			values-: POINTER TO ARRAY OF SHORTREAL
		END;

		(*	visitor for doing things to the Name structure	*)
		Visitor* = POINTER TO ABSTRACT RECORD END;

		(*	specialization of Visitor which does something to each element of components in Name	*)
		ElementVisitor* = POINTER TO ABSTRACT RECORD (Visitor)
			index-: INTEGER	(*	which element of component array is being visited	*)
		END;

	VAR
		version-: INTEGER; (*	version number	*)
		maintainer-: ARRAY 40 OF CHAR; (*	person maintaining module	*)

	(*	number of components	*)
	PROCEDURE (name: Name) Size* (): INTEGER, NEW;
		VAR
			i, numSlots, size: INTEGER;
	BEGIN
		size := 1;
		i := 0;
		numSlots := name.numSlots;
		WHILE i < numSlots DO
			size := size * name.slotSizes[i];
			INC(i)
		END;
		RETURN size
	END Size;

	(*	allocates storage for nodes in graph	*)
	PROCEDURE (name: Name) AllocateNodes*, NEW;
		VAR
			i, size: INTEGER;
	BEGIN
		size := name.Size();
		IF name.passByreference THEN
			IF name.components # NIL THEN RETURN END;
			NEW(name.components, size);
			i := 0;
			WHILE i < size DO
				name.components[i] := NIL;
				INC(i)
			END
		ELSE
			IF name.values # NIL THEN RETURN END;
			NEW(name.values, size);
			i := 0;
			WHILE i < size DO
				name.values[i] := INF;
				INC(i)
			END
		END
	END AllocateNodes;

	(*	Externalizes name, slot data and any constants*)
	PROCEDURE (name: Name) ExternalizeName* (VAR wr: Stores.Writer), NEW;
		VAR
			i, len: INTEGER;
	BEGIN
		wr.WriteBool(name.passByreference);
		wr.WriteString(name.string);
		wr.WriteInt(name.numSlots);
		i := 0;
		len := name.numSlots;
		WHILE i < len DO wr.WriteInt(name.slotSizes[i]); INC(i) END;
		IF name.values # NIL THEN len := LEN(name.values) ELSE len := 0 END;
		i := 0;
		WHILE i < len DO wr.WriteSReal(name.values[i]); INC(i) END
	END ExternalizeName;

	(*	Externalizes data contained in name, only pointers to nodes in graph are externalized	*)
	PROCEDURE (name: Name) ExternalizePointers* (VAR wr: Stores.Writer), NEW;
		VAR
			i, len: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		IF name.components # NIL THEN len := LEN(name.components) ELSE len := 0 END;
		i := 0;
		WHILE i < len DO
			p := name.components[i];
			GraphNodes.ExternalizePointer(p, wr);
			INC(i)
		END
	END ExternalizePointers;

	(*	Externalizes the internal fields of each node of the graph associated with name
	but if node is being sampled from do not externalize its data as this is done in
	UpdaterActions.ExternalizeData.
	*)
	PROCEDURE (name: Name) ExternalizeData* (VAR wr: Stores.Writer), NEW;
		VAR
			i, len: INTEGER;
			p: GraphNodes.Node;
	BEGIN
		IF name.components # NIL THEN len := LEN(name.components) ELSE len := 0 END;
		i := 0;
		WHILE i < len DO
			p := name.components[i];
			IF (p # NIL)
				 & (~(GraphStochastic.update IN p.props) OR (GraphStochastic.hidden IN p.props)) THEN
				p.Externalize(wr)
			END;
			INC(i)
		END
	END ExternalizeData;

	(*	Internalize name, slot data and any constants	*)
	PROCEDURE (name: Name) InternalizeName* (VAR rd: Stores.Reader), NEW;
		VAR
			i, len: INTEGER;
			string: ARRAY 64 OF CHAR;
	BEGIN
		rd.ReadBool(name.passByreference);
		rd.ReadString(string);
		name.string := string$;
		rd.ReadInt(len);
		IF len > 0 THEN NEW(name.slotSizes, len) ELSE name.slotSizes := NIL END;
		name.numSlots := len;
		i := 0;
		WHILE i < len DO rd.ReadInt(name.slotSizes[i]); INC(i) END;
		IF ~name.passByreference THEN
			len := name.Size();
			IF len > 0 THEN NEW(name.values, len) ELSE name.values := NIL END;
			i := 0;
			WHILE i < len DO rd.ReadSReal(name.values[i]); INC(i) END
		END
	END InternalizeName;

	(*	internalizes pointers to nodes in graph	*)
	PROCEDURE (name: Name) InternalizePointers* (VAR rd: Stores.Reader), NEW;
		VAR
			i, len: INTEGER;
	BEGIN
		len := name.Size();
		IF name.passByreference THEN
			IF len > 0 THEN NEW(name.components, len) ELSE name.components := NIL END;
			i := 0;
			WHILE i < len DO name.components[i] := GraphNodes.InternalizePointer(rd); INC(i) END
		END
	END InternalizePointers;

	(*	finds the indices of node in graph at offset from begining of component array	*)
	PROCEDURE (name: Name) Indices* (offset: INTEGER; OUT indices: ARRAY OF CHAR), NEW;
		VAR
			i, index: INTEGER;
			string: ARRAY 80 OF CHAR;
	BEGIN
		indices := "";
		IF name.numSlots # 0 THEN
			i := name.numSlots;
			WHILE i > 0 DO
				DEC(i);
				index := (offset MOD name.slotSizes[i]) + 1;
				offset := offset DIV name.slotSizes[i];
				Strings.IntToString(index, string);
				indices := string + indices;
				indices := "," + indices
			END;
			indices[0] := "[";
			indices := indices + "]"
		END
	END Indices;

	(*	are stochastic sampling type nodes in graph associated with name initialized	*)
	PROCEDURE (name: Name) Initialized* (): BOOLEAN, NEW;
		CONST
			init = {GraphNodes.data, GraphStochastic.initialized, GraphStochastic.hidden};
		VAR
			initialized: BOOLEAN;
			i, size: INTEGER;
			node: GraphNodes.Node;
	BEGIN
		i := 0;
		size := name.Size();
		initialized := TRUE;
		IF name.passByreference THEN
			WHILE (i < size) & initialized DO
				node := name.components[i];
				IF node # NIL THEN
					WITH node: GraphStochastic.Node DO
						initialized := init * node.props # {}
					ELSE
					END
				END;
				INC(i)
			END
		END;
		RETURN initialized
	END Initialized;

	PROCEDURE (name: Name) IsDefined* (offset: INTEGER): BOOLEAN, NEW;
	BEGIN
		IF name.passByreference THEN
			RETURN (name.components # NIL) & (name.components[offset] # NIL)
		ELSE
			RETURN (name.values # NIL) & (name.values[offset] # INF)
		END
	END IsDefined;

	(*	finds offset from start of component array corresponding to node in graph with indices	*)
	PROCEDURE (name: Name) Offset* (IN indices: ARRAY OF INTEGER): INTEGER, NEW;
		VAR
			i, offset, step: INTEGER;
	BEGIN
		offset := 0;
		IF name.numSlots # 0 THEN
			i := name.numSlots;
			step := 1;
			WHILE i > 0 DO
				DEC(i);
				offset := offset + step * (indices[i] - 1);
				step := step * name.slotSizes[i]
			END
		END;
		RETURN offset
	END Offset;

	(*	sets the range of slot	*)
	PROCEDURE (name: Name) SetRange* (slot, range: INTEGER), NEW;
	BEGIN
		name.slotSizes[slot] := range
	END SetRange;

	(*	how far apart in the component array are two node with have a difference of one in slot index	*)
	PROCEDURE (name: Name) Step* (slot: INTEGER): INTEGER, NEW;
		VAR
			i, step: INTEGER;
	BEGIN
		i := name.numSlots - 1;
		step := 1;
		WHILE i > slot DO
			step := step * name.slotSizes[i];
			DEC(i)
		END;
		RETURN step
	END Step;

	PROCEDURE (name: Name) StoreValue* (offset: INTEGER; value: REAL), NEW;
		VAR
			cons: GraphNodes.Node;
	BEGIN
		IF name.passByreference THEN
			name.components[offset] := GraphConstant.New(value)
		ELSE
			name.values[offset] := SHORT(value)
		END
	END StoreValue;

	PROCEDURE (name: Name) Value* (offset: INTEGER): REAL, NEW;
	BEGIN
		IF name.passByreference THEN
			RETURN name.components[offset].Value()
		ELSE
			RETURN name.values[offset]
		END
	END Value;

	(*	what the visitor does when it visits name	*)
	PROCEDURE (v: Visitor) Do* (name: Name), NEW, ABSTRACT;

		(*	name accepts a visit from v, depending on the type of v v can visit each element of name	*)
	PROCEDURE (name: Name) Accept* (v: Visitor), NEW;
		VAR
			i, size: INTEGER;
	BEGIN
		WITH v: ElementVisitor DO
			i := 0;
			size := name.Size();
			WHILE i < size DO
				v.index := i;
				v.Do(name);
				INC(i)
			END
		ELSE
			v.Do(name)
		END
	END Accept;

	(*	factory procedure for creating new name	*)
	PROCEDURE New* (IN string: ARRAY OF CHAR; numSlots: INTEGER): Name;
		VAR
			i: INTEGER;
			name: Name;
	BEGIN
		NEW(name);
		name.numSlots := numSlots;
		name.string := string$;
		name.components := NIL;
		name.values := NIL;
		IF numSlots > 0 THEN
			NEW(name.slotSizes, numSlots);
			i := 0;
			WHILE i < numSlots DO
				name.slotSizes[i] := 0;
				INC(i)
			END
		ELSE
			name.slotSizes := NIL
		END;
		name.passByreference := FALSE;
		RETURN name
	END New;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END BugsNames.

