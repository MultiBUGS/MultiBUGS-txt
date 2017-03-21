(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	   *)

MODULE BugsNames;

	

(*	data structure to represent names in BUGS model	*)

	IMPORT
		Stores, Strings, 
		GraphNodes, GraphStochastic;

	TYPE
		(*	strucure representing names in OpenBUGS model	*)
		Name* = POINTER TO LIMITED RECORD
			numSlots-: INTEGER; (*	number of indices	*)
			slotSizes-: POINTER TO ARRAY OF INTEGER; (*	range of each index	*)
			string-:  ARRAY 128 OF CHAR; (*	name as a string	*)
			components-: GraphNodes.Vector	(*	nodes in graph	*)
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

	(*	what the visitor does when it visits name	*)
	PROCEDURE (v: Visitor) Do* (name: Name), NEW, ABSTRACT;

	(*	name accepts a visit from v, depending on the type of v v can visit each element of name	*)
	PROCEDURE (name: Name) Accept* (v: Visitor), NEW;
		VAR
			i, size: INTEGER;
	BEGIN
		WITH v: ElementVisitor DO
			i := 0;
			size := LEN(name.components);
			WHILE i < size DO
				v.index := i;
				v.Do(name);
				INC(i)
			END
		ELSE
			v.Do(name)
		END
	END Accept;

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
		ASSERT(name.components = NIL, 21);
		size := name.Size();
		NEW(name.components, size);
		i := 0;
		WHILE i < size DO
			name.components[i] := NIL;
			INC(i)
		END
	END AllocateNodes;

	(*	externalizes name and slot data	*)
	PROCEDURE (name: Name) ExternalizeName* (VAR wr: Stores.Writer), NEW;
		VAR
			i, len: INTEGER;
	BEGIN
		wr.WriteString(name.string);
		wr.WriteInt(name.numSlots);
		i := 0;
		len := name.numSlots;
		WHILE i < len DO wr.WriteInt(name.slotSizes[i]); INC(i) END;
	END ExternalizeName;

	(*	externalizes data contained in name, only pointers to nodes in graph are externalized	*)
	PROCEDURE (name: Name) ExternalizePointers* (VAR wr: Stores.Writer), NEW;
		VAR
			i, len: INTEGER;
	BEGIN
		IF name.components # NIL THEN len := LEN(name.components) ELSE len := 0 END;
		i := 0;
		WHILE i < len DO GraphNodes.ExternalizePointer(name.components[i], wr); INC(i) END
	END ExternalizePointers;

	(*	externalizes the internal fields of each node of the graph associated with name	
		   if node is being sampled from do not externalize its data as this is done in 
		   UpdaterActions.ExternalizeData	
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
			IF (p # NIL) & ~(GraphStochastic.update IN p.props) THEN p.Externalize(wr) END;
			INC(i)
		END
	END ExternalizeData;

	PROCEDURE (name: Name) InternalizeName* (VAR rd: Stores.Reader), NEW;
		VAR
			i, len: INTEGER;
			string: ARRAY 64 OF CHAR;
	BEGIN
		rd.ReadString(string);
		len := LEN(string$);
		name.string := string$;
		rd.ReadInt(len);
		IF len > 0 THEN NEW(name.slotSizes, len) ELSE name.slotSizes := NIL END;
		name.numSlots := len;
		i := 0;
		WHILE i < len DO rd.ReadInt(name.slotSizes[i]); INC(i) END;
	END InternalizeName;
	
	(*	internalizes pointers to nodes in graph	*)
	PROCEDURE (name: Name) InternalizePointers* (VAR rd: Stores.Reader), NEW;
		VAR
			i, len: INTEGER;
	BEGIN
		(*rd.ReadInt(len);*)
		len := name.Size();
		IF len > 0 THEN NEW(name.components, len) ELSE name.components := NIL END;
		i := 0;
		WHILE i < len DO name.components[i] := GraphNodes.InternalizePointer(rd); INC(i) END
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
			init = {GraphNodes.data, GraphStochastic.initialized, GraphStochastic.nR};
		VAR
			initialized: BOOLEAN;
			i, size: INTEGER;
			node: GraphNodes.Node;
	BEGIN
		i := 0;
		size := name.Size();
		initialized := TRUE;
		WHILE (i < size) & initialized DO
			node := name.components[i];
			IF node # NIL THEN
				WITH node: GraphStochastic.Node DO
					initialized := init * node.props # {}
				ELSE
				END
			END;
			INC(i)
		END;
		RETURN initialized
	END Initialized;

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
		RETURN name
	END New;

	(*	resizes the name and copies nodes in the graphical model to new offsets	*)
	PROCEDURE (name: Name) Resize* (IN indices: ARRAY OF INTEGER), NEW;
		VAR
			i, index, newOffset, numSlots, offset, size: INTEGER;
			inRange: BOOLEAN;
			dim: POINTER TO ARRAY OF INTEGER;
			temp: Name;
	BEGIN
		numSlots := name.numSlots;
		IF name.numSlots # 0 THEN
			temp := New(name.string, numSlots);
			i := 0;
			WHILE i < numSlots DO
				temp.SetRange(i, indices[i]);
				INC(i)
			END;
			temp.AllocateNodes();
			offset := 0;
			size := name.Size();
			NEW(dim, numSlots);
			WHILE (offset < size) DO
				i := numSlots;
				newOffset := offset;
				inRange := TRUE;
				WHILE inRange & (i > 0) DO
					DEC(i);
					index := (newOffset MOD name.slotSizes[i]) + 1;
					newOffset := newOffset DIV name.slotSizes[i];
					dim[i] := index;
					inRange := index <= indices[i]
				END;
				IF inRange THEN
					newOffset := temp.Offset(dim);
					temp.components[newOffset] := name.components[offset]
				END;
				INC(offset)
			END;
			name.components := temp.components;
			i := 0;
			WHILE i < numSlots DO
				name.slotSizes[i] := temp.slotSizes[i];
				INC(i)
			END
		END
	END Resize;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END BugsNames.

