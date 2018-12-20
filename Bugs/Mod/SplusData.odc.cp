(*		GNU General Public Licence	   *)

(*
license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"
*)

MODULE BugsSplusData;


	

	IMPORT
		Strings, 
		BugsData, BugsIndex, BugsMappers, BugsMsg, BugsNames,
		GraphNodes, GraphStochastic;

	TYPE

		Action = POINTER TO ABSTRACT RECORD
			name: BugsNames.Name
		END;

		Loader = POINTER TO RECORD(BugsData.Loader)
			action: Action
		END;

		Allocator = POINTER TO RECORD(Action) END;

		DataChecker = POINTER TO RECORD(Action) END;

		DataLoader = POINTER TO RECORD(Action) END;

		InitsChecker = POINTER TO RECORD(Action) END;

		InitsLoader = POINTER TO RECORD(Action) END;

		Factory = POINTER TO RECORD(BugsData.Factory) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		fact-: BugsData.Factory;
		(* Node() is called three times but the warnings about variables missing in the model
		only need to be stored once.  This flag indicates when we need to store the warnings *)
		giveWarning: BOOLEAN;
		nmissing: INTEGER;
		missingVars: ARRAY 1024 OF CHAR;

	CONST
		number = 0;
		na = 1;
		error = 2;

	PROCEDURE (action: Action) Allocate, NEW, EMPTY;

	PROCEDURE (action: Action) Values (VAR s: BugsMappers.Scanner): INTEGER, NEW, ABSTRACT;

	PROCEDURE (action: Action) Slot (slot, slotSize: INTEGER; OUT ok: BOOLEAN), NEW, ABSTRACT;

	PROCEDURE Error (errorNum: INTEGER);
		VAR
			errorMsg: ARRAY 1024 OF CHAR;
			numToString: ARRAY 8 OF CHAR;
	BEGIN
		Strings.IntToString(errorNum, numToString);
		BugsMsg.Lookup("BugsSplusData" + numToString, errorMsg);
		BugsMsg.StoreError(errorMsg);
	END Error;

	PROCEDURE ErrorNode (errorNum: INTEGER; string: ARRAY OF CHAR);
		VAR
			errorMsg: ARRAY 1024 OF CHAR;
			numToString: ARRAY 8 OF CHAR;
	BEGIN
		Strings.IntToString(errorNum, numToString);
		BugsMsg.Lookup("BugsSplusData" + numToString, errorMsg);
		errorMsg := errorMsg + " " + string;
		BugsMsg.StoreError(errorMsg);
	END ErrorNode;

	PROCEDURE CheckValue (VAR s: BugsMappers.Scanner; OUT status: INTEGER);
		VAR
			signed: BOOLEAN;
	BEGIN
		signed := FALSE;
		s.Scan;
		IF s.type = BugsMappers.char THEN
			IF s.char = "-" THEN
				signed := TRUE;
				s.Scan
			ELSE (*	invalid or unexpected token scanned	*)
				status := error; Error(1); RETURN
			END
		END;
		IF (s.type = BugsMappers.string) & (s.string = "NA") THEN
			IF signed THEN (*	NA can not be signed	*)
				status := error; Error(2); RETURN
			END;
			status := na
		ELSIF (s.type # BugsMappers.int) & (s.type # BugsMappers.real) THEN
			(*	expected number or NA	*)
			status := error; Error(3); RETURN
		ELSE
			status := number
		END
	END CheckValue;

	PROCEDURE ReadValue (VAR s: BugsMappers.Scanner; OUT value: REAL; OUT nA: BOOLEAN);
		VAR
			signed: BOOLEAN;
			constant: GraphNodes.Node;
	BEGIN
		signed := FALSE;
		nA := FALSE;
		s.Scan;
		IF (s.type = BugsMappers.char) & (s.char = "-") THEN
			signed := TRUE;
			s.Scan
		END;
		IF (s.type = BugsMappers.string) & (s.string = "NA") THEN
			value := INF; nA := TRUE;
		ELSIF s.type = BugsMappers.int THEN
			value := s.int
		ELSIF s.type = BugsMappers.real THEN
			value := s.real
		END;
		IF signed THEN
			value := - value
		END;
	END ReadValue;

	PROCEDURE StoreInitValue (VAR s: BugsMappers.Scanner; node: GraphNodes.Node);
		VAR
			signed: BOOLEAN;
			value: REAL;
	BEGIN
		signed := FALSE;
		s.Scan; ;
		IF(s.type = BugsMappers.char) & (s.char = "-") THEN
			signed := TRUE;
			s.Scan
		END;
		IF (s.type = BugsMappers.string) & (s.string = "NA") THEN
			RETURN
		ELSIF s.type = BugsMappers.int THEN
			value := s.int
		ELSIF s.type = BugsMappers.real THEN
			value := s.real
		END;
		IF signed THEN
			value := - value
		END;
		node(GraphStochastic.Node).SetValue(value);
		node.SetProps(node.props + {GraphStochastic.initialized})
	END StoreInitValue;

	PROCEDURE (action: DataChecker) Values (VAR s: BugsMappers.Scanner): INTEGER;
		CONST
			undefined = 0;
		VAR
			name: BugsNames.Name;
			status: INTEGER;
			i, len: INTEGER;
	BEGIN
		i := 0;
		name := action.name;
		IF name.components # NIL THEN
			len := name.Size()
		ELSE
			len := undefined
		END;
		LOOP
			CheckValue(s, status);
			IF status = error THEN RETURN 0 END;
			IF name.IsDefined(i) & (status = number) THEN
				(*	data value already given for this component of node 	*)
				Error(4); RETURN 0
			END;
			INC(i);
			IF name.numSlots = 0 THEN EXIT END;
			s.Scan;
			IF s.type # BugsMappers.char THEN (*	expected a comma or right parenthesis	*)
				Error(5); RETURN 0
			ELSIF s.char = ")" THEN
				IF (len # undefined) & (i # len) THEN (*	number of  items not equal to length of vector	*)
					Error(6); RETURN 0
				END;
				EXIT
			ELSIF s.char # "," THEN (*	expected a comma or right parenthesis	*)
				Error(5); RETURN 0
			END
		END;
		RETURN i
	END Values;

	PROCEDURE (action: DataChecker) Slot (slot, slotSize: INTEGER; OUT ok: BOOLEAN);
		VAR
			name: BugsNames.Name;
	BEGIN
		ok := TRUE;
		name := action.name;
		IF (name.slotSizes[slot] # 0) & (name.slotSizes[slot] # slotSize) THEN
			(*	node dimension mis match	*)
			ok := FALSE; Error(7);
		END
	END Slot;

	PROCEDURE (action: Allocator) Allocate;
		VAR
			name: BugsNames.Name;
	BEGIN
		name := action.name;
		name.AllocateNodes
	END Allocate;

	PROCEDURE (action: Allocator) Values (VAR s: BugsMappers.Scanner): INTEGER;
		VAR
			c: DataChecker;
	BEGIN
		NEW(c);
		c.name := action.name;
		RETURN c.Values(s)
	END Values;

	PROCEDURE (action: Allocator) Slot (slot, slotSize: INTEGER; OUT ok: BOOLEAN);
		VAR
			name: BugsNames.Name;
	BEGIN
		ok := TRUE;
		name := action.name;
		name.SetRange(slot, slotSize)
	END Slot;

	PROCEDURE (action: InitsChecker) Values (VAR s: BugsMappers.Scanner): INTEGER;
		VAR
			name: BugsNames.Name;
			status: INTEGER;
			i, len: INTEGER;
			string: ARRAY 128 OF CHAR;
	BEGIN
		name := action.name;
		IF name.components = NIL THEN (*	no prior specified for this node	*)
			Error(8); RETURN 0
		END;
		i := 0;
		len := name.Size();
		LOOP
			CheckValue(s, status);
			IF status = error THEN RETURN 0 END;
			IF i = len THEN (*	number of  items not equal to length of vector	*)
				Error(6); RETURN 0
			END;
			IF status = number THEN
				name.Indices(i, string);
				string := name.string + string;
				IF name.components[i] = NIL THEN (*	no prior specified for this component of node	*)
					ErrorNode(9, string); RETURN 0
				ELSIF ~(name.components[i] IS GraphStochastic.Node) THEN (*	node is not stochastic	*)
					ErrorNode(10, string); RETURN 0
				ELSIF GraphNodes.data IN name.components[i].props THEN(*	node is data	*)
					ErrorNode(11, string); RETURN 0
				END
			END;
			INC(i);
			IF name.numSlots = 0 THEN EXIT END;
			s.Scan;
			IF s.type # BugsMappers.char THEN (*	expected a comma or right parenthesis	*)
				Error(12); RETURN 0
			ELSIF s.char = ")" THEN
				IF i # len THEN (*	number of  items not equal to size of node	*)
					Error(13); RETURN 0
				END;
				EXIT
			ELSIF s.char # "," THEN (*	expected a comma or right parenthesis	*)
				Error(14); RETURN 0
			END
		END;
		RETURN i
	END Values;

	PROCEDURE (action: InitsChecker) Slot (slot, slotSize: INTEGER; OUT ok: BOOLEAN);
		VAR
			name: BugsNames.Name;
	BEGIN
		name := action.name;
		ok := TRUE;
		IF name.slotSizes[slot] # slotSize THEN (*	node dimension does not match	*)
			ok := FALSE; Error(15); RETURN
		END
	END Slot;

	PROCEDURE (action: DataLoader) Values (VAR s: BugsMappers.Scanner): INTEGER;
		VAR
			i, len: INTEGER;
			name: BugsNames.Name;
			value: REAL;
			node: GraphNodes.Node;
			nA: BOOLEAN;
	BEGIN
		name := action.name;
		i := 0;
		len := name.Size(); 
		LOOP
			ReadValue(s, value, nA);
			IF name.isVariable THEN
				IF ~nA THEN
					name.StoreValue(i, value);
					node := name.components[i];
					node.SetProps(node.props + {GraphStochastic.data})
				END
			ELSE
				name.StoreValue(i, value)
			END;
			IF name.numSlots # 0 THEN s.Scan END;
			INC(i);
			IF i = len THEN EXIT END
		END;
		RETURN len
	END Values;

	(* Skip a vector of numbers for a variable in the data that isn't in the model *)

	PROCEDURE SkipList (VAR s: BugsMappers.Scanner): INTEGER;
		VAR
			status: INTEGER;
			signed: BOOLEAN;
	BEGIN
		LOOP
			CheckValue(s, status); (* read and check number or NA *)
			IF status = error THEN RETURN 0 END;
			s.Scan;
			IF (s.type # BugsMappers.char) THEN (* expected a comma or right parenthesis *)
				Error(22); RETURN 0
			ELSIF (s.char = ")") THEN
				EXIT (* stop when encounter right parenthesis *)
			ELSIF (s.char # ",") THEN
				Error(22); RETURN 0
			END;
		END;
		RETURN 1;
	END SkipList;

	PROCEDURE (action: DataLoader) Slot (slot, slotSize: INTEGER; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE
	END Slot;

	PROCEDURE (action: InitsLoader) Values (VAR s: BugsMappers.Scanner): INTEGER;
		VAR
			i, len: INTEGER;
			name: BugsNames.Name;
	BEGIN
		name := action .name;
		ASSERT(name.components # NIL, 21);
		i := 0;
		len := name.Size();
		LOOP
			StoreInitValue(s, name.components[i]);
			INC(i);
			IF name.numSlots # 0 THEN s.Scan END;
			IF i = len THEN EXIT END;
		END;
		RETURN len
	END Values;

	PROCEDURE (action: InitsLoader) Slot (slot, slotSize: INTEGER; OUT ok: BOOLEAN);
	BEGIN
		ok := TRUE
	END Slot;

	PROCEDURE (l: Loader) Dimen (VAR s: BugsMappers.Scanner): INTEGER, NEW;
		VAR
			ok: BOOLEAN;
			i, numSlots, size, slotSize: INTEGER;
			action: Action;
			name: BugsNames.Name;
	BEGIN
		action := l.action;
		name := action.name;
		s.Scan;
		IF (s.type # BugsMappers.char) OR (s.char # ".") THEN (*	expected a period	*)
			Error(16); RETURN 0
		END;
		s.Scan;
		IF (s.type # BugsMappers.string) OR (s.string # "Dim") THEN (*	expected key word Dim	*)
			Error(17); RETURN 0
		END;
		s.Scan;
		IF (s.type # BugsMappers.char) OR (s.char # "=") THEN (*	expected an equals sign	*)
			Error(18); RETURN 0
		END;
		s.Scan;
		IF (s.type # BugsMappers.function) OR (s.string # "c") THEN (*	expected collection operator c	*)
			Error(19); RETURN 0
		END;
		s.Scan;
		IF (s.type # BugsMappers.char) OR (s.char # "(") THEN (*	expected left parenthesis	*)
			Error(20); RETURN 0
		END;
		i := 0;
		size := 1;
		numSlots := name.numSlots;
		WHILE i < numSlots DO
			s.Scan;
			IF s.type # BugsMappers.int THEN (*	expected an integer	*)
				Error(21); RETURN 0
			END;
			slotSize := s.int;
			action.Slot(i, slotSize, ok);
			IF ~ok THEN RETURN 0 END;
			size := size * slotSize;
			s.Scan;
			IF s.type # BugsMappers.char THEN (*	expected a comma or right parenthesis	*)
				Error(22); RETURN 0
			ELSIF (i < numSlots - 1) & (s.char # ",") THEN (*	expected a commas	*)
				Error(23); RETURN 0
			END;
			INC(i)
		END;
		IF (s.type # BugsMappers.char) OR (s.char # ")") THEN (*	expected right parenthesis	*)
			Error(24); RETURN 0
		END;
		RETURN size
	END Dimen;

	(* Skip a dimension attribute for a variable in the data that isn't in the model *)

	PROCEDURE (l: Loader) SkipDimen (VAR s: BugsMappers.Scanner): INTEGER, NEW;
		VAR
			ok: BOOLEAN;
			i: INTEGER;
	BEGIN
		s.Scan;
		IF (s.type # BugsMappers.char) OR (s.char # ".") THEN (*	expected a period	*)
			Error(16); RETURN 0
		END;
		s.Scan;
		IF (s.type # BugsMappers.string) OR (s.string # "Dim") THEN (*	expected key word Dim	*)
			Error(17); RETURN 0
		END;
		s.Scan;
		IF (s.type # BugsMappers.char) OR (s.char # "=") THEN (*	expected an equals sign	*)
			Error(18); RETURN 0
		END;
		s.Scan;
		IF (s.type # BugsMappers.function) OR (s.string # "c") THEN (*	expected collection operator c	*)
			Error(19); RETURN 0
		END;
		s.Scan;
		IF (s.type # BugsMappers.char) OR (s.char # "(") THEN (*	expected left parenthesis	*)
			Error(20); RETURN 0
		END;
		i := 0;
		LOOP
			s.Scan;
			IF s.type # BugsMappers.int THEN (*	expected an integer	*)
				Error(21); RETURN 0
			END;
			s.Scan;
			IF s.type # BugsMappers.char THEN (*	expected a comma or right parenthesis	*)
				Error(22); RETURN 0
			ELSIF (s.char = ")") THEN EXIT
			ELSIF (s.char # ",") THEN (*	expected a comma	*)
				Error(23); RETURN 0
			END;
			INC(i)
		END;
		IF (s.type # BugsMappers.char) OR (s.char # ")") THEN (*	expected right parenthesis	*)
			Error(24); RETURN 0
		END;
		RETURN 1;
	END SkipDimen;

	PROCEDURE (l: Loader) Node (VAR s: BugsMappers.Scanner; OUT ok: BOOLEAN), NEW;
		VAR
			indices, len, size: INTEGER;
			action: Action;
			name: BugsNames.Name;
	BEGIN
		action := l.action;
		name := action.name;
		ok := TRUE;
		indices := name.numSlots;
		IF indices = 0 THEN
			len := action.Values(s);
			IF len = 0 THEN
				ok := FALSE; RETURN
			ELSIF len # 1 THEN (*	scalar node must have size of one	*)
				ok := FALSE; Error(25); RETURN
			END
		ELSIF indices = 1 THEN
			s.Scan;
			IF (s.type # BugsMappers.function) OR (s.string # "c") THEN
				(*	expected the collection operator c	*)
				ok := FALSE; Error(26); RETURN
			END;
			s.Scan;
			IF (s.type # BugsMappers.char) OR (s.char # "(") THEN
				(*	expected left parenthesis	*)
				ok := FALSE; Error(27); RETURN
			END;
			len := action.Values(s);
			IF len = 0 THEN
				ok := FALSE; RETURN
			ELSIF (name.components # NIL) & (len # name.Size()) THEN
				(*	node size # no of components	*)
				ok := FALSE; Error(28); RETURN
			END;
			action.Slot(0, len, ok)
		ELSE
			s.Scan;
			IF (s.type # BugsMappers.function) OR (s.string # "structure") THEN
				(*	expected key word structure	*)
				ok := FALSE; Error(29); RETURN
			END;
			s.Scan;
			IF (s.type # BugsMappers.char) OR (s.char # "(") THEN (*	expected left parenthesis	*)
				ok := FALSE; Error(30); RETURN
			END;
			s.Scan;
			IF (s.type # BugsMappers.char) OR (s.char # ".") THEN (*	expected a period	*)
				ok := FALSE; Error(31); RETURN
			END;
			s.Scan;
			IF (s.type # BugsMappers.string) OR (s.string # "Data") THEN (*	expected key word Data	*)
				ok := FALSE; Error(32); RETURN
			END;
			s.Scan;
			IF (s.type # BugsMappers.char) OR (s.char # "=") THEN (*	expected an equals sign	*)
				ok := FALSE; Error(33); RETURN
			END;
			s.Scan;
			IF (s.type # BugsMappers.function) OR (s.string # "c") THEN
				(*	expected the collection operator c	*)
				ok := FALSE; Error(34); RETURN
			END;
			s.Scan;
			IF (s.type # BugsMappers.char) OR (s.char # "(") THEN (*	expected left parenthesis	*)
				ok := FALSE; Error(35); RETURN
			END;
			len := action.Values(s);
			s.Scan;
			IF len = 0 THEN
				ok := FALSE; RETURN
			ELSIF (s.type # BugsMappers.char) OR (s.char # ",") THEN (*	expected comma	*)
				ok := FALSE; Error(36); RETURN
			END;
			size := l.Dimen(s);
			IF size = 0 THEN
				ok := FALSE;
				RETURN
			ELSIF size # len THEN (*	number of  items not equal to size of node	*)
				ok := FALSE; Error(37); RETURN
			END;
			s.Scan;
			IF (s.type # BugsMappers.char) & (s.char # ")") THEN (*	expected right parenthesis	*)
				ok := FALSE; Error(38); RETURN
			END
		END;
		action.Allocate
	END Node;


	(* Skip past a node in the datafile which is not in the model *)

	PROCEDURE (l: Loader) SkipNode (VAR s: BugsMappers.Scanner; OUT ok: BOOLEAN), NEW;
		VAR
			res: INTEGER;
			action: Action;
	BEGIN
		s.Scan;
		IF ((s.type = BugsMappers.char) & (s.char = "-")) THEN
			(* negative number *)
			s.Scan;
			IF (s.type # BugsMappers.int) & (s.type # BugsMappers.real) THEN
				IF (s.type = BugsMappers.string) & (s.string = "NA") THEN
					ok := FALSE; Error(2); RETURN (* NA cannot be signed *)
				ELSE
					ok := FALSE; Error(1); RETURN (* unexpected token *)
				END;
			END;
		ELSIF (s.type = BugsMappers.int) OR (s.type = BugsMappers.real) OR
			((s.type = BugsMappers.string) & (s.string = "NA")) THEN
			(* unsigned number or NA *)
			(* do nothing.  Next token should be name of next node. *)
		ELSIF (s.string = "c") THEN
			(* 1D vector node *)
			s.Scan;
			IF (s.type # BugsMappers.char) OR (s.char # "(") THEN (*	expected left parenthesis	*)
				ok := FALSE; Error(27); RETURN
			END;
			res := SkipList(s);
			IF res = 0 THEN ok := FALSE; RETURN END;
		ELSIF (s.string = "structure") THEN
			(* ugh - code duplicated from (Loader)Node *)
			s.Scan;
			IF (s.type # BugsMappers.char) OR (s.char # "(") THEN (*	expected left parenthesis	*)
				ok := FALSE; Error(30); RETURN
			END;
			s.Scan;
			IF (s.type # BugsMappers.char) OR (s.char # ".") THEN (*	expected a period	*)
				ok := FALSE; Error(31); RETURN
			END;
			s.Scan;
			IF (s.type # BugsMappers.string) OR (s.string # "Data") THEN (*	expected key word Data	*)
				ok := FALSE; Error(32); RETURN
			END;
			s.Scan;
			IF (s.type # BugsMappers.char) OR (s.char # "=") THEN (*	expected an equals sign	*)
				ok := FALSE; Error(33); RETURN
			END;
			s.Scan;
			IF (s.type # BugsMappers.function) OR (s.string # "c") THEN (*	expected the collection operator c	*)
				ok := FALSE; Error(34); RETURN
			END;
			s.Scan;
			IF (s.type # BugsMappers.char) OR (s.char # "(") THEN (*	expected left parenthesis	*)
				ok := FALSE; Error(35); RETURN
			END;
			res := SkipList(s);
			IF res = 0 THEN ok := FALSE; RETURN END;
			s.Scan;
			IF (s.type # BugsMappers.char) OR (s.char # ",") THEN (*	expected comma	*)
				ok := FALSE; Error(36); RETURN
			END;
			res := l.SkipDimen(s);
			IF res = 0 THEN ok := FALSE; RETURN END;
			s.Scan;
			IF (s.type # BugsMappers.char) & (s.char # ")") THEN (*	expected right parenthesis	*)
				ok := FALSE; Error(38); RETURN
			END;
		ELSE
			ok := FALSE; Error(48); RETURN (* expected a number, key word c(), or key word structure()  *)
		END;
	END SkipNode;

	PROCEDURE (l: Loader) Nodes (VAR s: BugsMappers.Scanner; OUT ok: BOOLEAN), NEW;
		VAR
			action: Action;
			name: BugsNames.Name;
			varname: ARRAY 64 OF CHAR;
			inModel: BOOLEAN;
	BEGIN
		ok := TRUE;
		action := l.action;
		LOOP
			s.Scan;
			IF s.type # BugsMappers.string THEN (*	expected variable name	*)
				ok := FALSE; Error(39); RETURN
			END;
			varname := s.string$;
			name := BugsIndex.Find(varname);
			inModel := name # NIL;
			(*	ok := FALSE; Error(39); RETURN *)
			action.name := name;
			s.Scan;
			IF (s.type # BugsMappers.char) OR (s.char # "=") THEN (*	expected an equals sign	*)
				ok := FALSE; Error(11); RETURN
			END;
			IF inModel THEN
				l.Node(s, ok);
			ELSE
				l.SkipNode(s, ok);
				IF (giveWarning) THEN
					INC(nmissing);
					IF (nmissing < 6) THEN
						IF (nmissing > 1) THEN
							missingVars := missingVars + ", ";
						END;
						missingVars := missingVars + varname;
					ELSIF (nmissing = 6) THEN
						missingVars := missingVars + " and others";
					END;
				END;
			END;
			(* 	l.Node(s, ok); *)
			IF ~ok THEN RETURN END;
			s.Scan;
			IF s.type # BugsMappers.char THEN (*	expected a comma or right parenthesis	*)
				ok := FALSE; Error(42); RETURN
			ELSIF s.char = ")" THEN
				EXIT
			ELSIF s.char # "," THEN (*		expected a comma	*)
				ok := FALSE; Error(43); RETURN
			END
		END
	END Nodes;

	PROCEDURE (l: Loader) Data* (VAR s: BugsMappers.Scanner; OUT ok: BOOLEAN);
		VAR
			beg: INTEGER;
			checker: DataChecker;
			allocator: Allocator;
			loader: DataLoader;
			warningMsg: ARRAY 1024 OF CHAR;
	BEGIN
		missingVars := "";
		nmissing := 0;
		beg := s.Pos();
		s.Scan;
		IF (s.type # BugsMappers.function) OR (s.string # "list") THEN (*	expected key word list	*)
			ok := FALSE; Error(44); RETURN
		END;
		s.Scan;
		IF (s.type # BugsMappers.char) OR (s.char # "(") THEN (*	expected left parenthesis	*)
			ok := FALSE; Error(41); RETURN
		END;
		NEW(checker);
		l.action := checker;
		giveWarning := TRUE;
		l.Nodes(s, ok);
		IF ~ok THEN RETURN END;
		giveWarning := FALSE;
		s.SetPos(beg);
		s.Scan;
		s.Scan;
		NEW(allocator);
		l.action := allocator;
		l.Nodes(s, ok);
		s.SetPos(beg);
		s.Scan;
		s.Scan;
		NEW(loader);
		l.action := loader;
		l.Nodes(s, ok);
		(* "variables not in the model" warning is concatenated with the list of such variable names *)
		IF (nmissing > 0) THEN
			BugsMsg.Lookup("BugsSplusData40", warningMsg);
			warningMsg := " (" + warningMsg + missingVars + ")";
			BugsMsg.StoreMsg(warningMsg);
			nmissing := 0;
			missingVars := "";
		END;
	END Data;

	PROCEDURE (l: Loader) Inits* (VAR s: BugsMappers.Scanner; OUT ok: BOOLEAN);
		VAR
			beg: INTEGER;
			checker: InitsChecker;
			loader: InitsLoader;
			warningMsg: ARRAY 1024 OF CHAR;
	BEGIN
		missingVars := "";
		nmissing := 0;
		beg := s.Pos();
		s.Scan;
		IF (s.type # BugsMappers.function) OR (s.string # "list") THEN (*	expected key word list	*)
			ok := FALSE; Error(46); RETURN
		END;
		s.Scan;
		IF (s.type # BugsMappers.char) OR (s.char # "(") THEN (*	expected left parenthesis	*)
			ok := FALSE; Error(47); RETURN
		END;
		NEW(checker);
		l.action := checker;
		giveWarning := TRUE;
		l.Nodes(s, ok);
		IF ~ok THEN RETURN END;
		giveWarning := FALSE;
		s.SetPos(beg);
		s.Scan;
		s.Scan;
		NEW(loader);
		l.action := loader;
		l.Nodes(s, ok);
		(* "variables not in the model" warning is concatenated with the list of such variable names *)
		IF (nmissing > 0) THEN
			BugsMsg.Lookup("BugsSplusData40", warningMsg);
			warningMsg := " (" + warningMsg + missingVars + ")";
			BugsMsg.StoreMsg(warningMsg);
			nmissing := 0;
			missingVars := "";
		END;
	END Inits;

	PROCEDURE (f: Factory) New (): Loader;
		VAR
			l: Loader;
	BEGIN
		NEW(l);
		RETURN l
	END New;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Install*;
	BEGIN
		BugsData.SetFactory(fact)
	END Install;

	PROCEDURE Init;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f;
		giveWarning := TRUE;
		missingVars := "";
		nmissing := 0;
	END Init;

BEGIN
	Init
END BugsSplusData.

