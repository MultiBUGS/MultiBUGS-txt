(*		GNU General Public Licence	   *)

(*
license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"
*)

MODULE BugsRectData;


	

	IMPORT
		Strings,
		BugsData, BugsEvaluate, BugsMappers, BugsMsg, BugsNames, BugsParser,
		GraphConstant, GraphNodes, GraphStochastic;

	TYPE
		List = POINTER TO RECORD
			inModel: BOOLEAN;
			name: BugsNames.Name;
			indices: POINTER TO ARRAY OF INTEGER;
			next: List
		END;

		Header = RECORD
			inModel: BOOLEAN;
			name: BugsNames.Name;
			indices: POINTER TO ARRAY OF INTEGER;
		END;

		Loader = POINTER TO RECORD(BugsData.Loader) END;

		Factory = POINTER TO RECORD(BugsData.Factory) END;

	CONST
		number = 0;
		na = 1;
		end = 2;
		error = 3;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		fact-: BugsData.Factory;
		(* ReadHeader() is called more than once but the warnings about variables missing in the model
		only need to be stored once.  This flag indicates when we need to store the warnings *)
		giveWarning: BOOLEAN;
		nmissing: INTEGER;
		missingVars: ARRAY 1024 OF CHAR;

	PROCEDURE Error (errorNum: INTEGER);
		VAR
			numToString: ARRAY 8 OF CHAR;
			errorMsg: ARRAY 1024 OF CHAR;
	BEGIN
		Strings.IntToString(errorNum, numToString);
		BugsMsg.Lookup("BugsRectData" + numToString, errorMsg);
		BugsMsg.Store(errorMsg);
	END Error;

	PROCEDURE ErrorNode (errorNum: INTEGER; string: ARRAY OF CHAR);
		VAR
			errorMsg: ARRAY 1024 OF CHAR;
			numToString: ARRAY 8 OF CHAR;
	BEGIN
		Strings.IntToString(errorNum, numToString);
		BugsMsg.Lookup("BugsRectData" + numToString, errorMsg);
		errorMsg := errorMsg + " " + string;
		BugsMsg.Store(errorMsg);
	END ErrorNode;

	PROCEDURE CheckItem (VAR s: BugsMappers.Scanner; OUT status: INTEGER);
		VAR
			signed: BOOLEAN;
	BEGIN
		signed := FALSE;
		IF s.type = BugsMappers.char THEN
			IF s.char = "-" THEN
				signed := TRUE;
				s.Scan
			ELSE
				status := error;
				Error(1); (*	invalid or unexpected token scanned	*)
				RETURN
			END
		END;
		IF (s.type = BugsMappers.string) & (s.string = "NA") THEN
			IF signed THEN
				status := error;
				Error(2); (*	NA can not be signed	*)
				RETURN
			END;
			status := na
		ELSIF (s.type = BugsMappers.string) & (s.string = "END") THEN
			status := end
		ELSIF (s.type # BugsMappers.int) & (s.type # BugsMappers.real) THEN
			status := error;
			Error(3); (*	expected number or NA or END	*)
			RETURN
		ELSE
			status := number
		END
	END CheckItem;

	PROCEDURE CreateDataNode (VAR s: BugsMappers.Scanner): GraphNodes.Node;
		VAR
			signed: BOOLEAN;
			value: REAL;
			constant: GraphNodes.Node;
	BEGIN
		signed := FALSE;
		IF (s.type = BugsMappers.char) & (s.char = "-") THEN
			signed := TRUE;
			s.Scan
		END;
		IF (s.type = BugsMappers.string) & (s.string = "NA") THEN
			RETURN NIL
		ELSIF s.type = BugsMappers.int THEN
			value := s.int
		ELSIF s.type = BugsMappers.real THEN
			value := s.real
		END;
		IF signed THEN
			value :=  - value
		END;
		constant := GraphConstant.New(value);
		RETURN constant
	END CreateDataNode;

	PROCEDURE StoreInitValue (VAR s: BugsMappers.Scanner; node: GraphNodes.Node);
		VAR
			signed: BOOLEAN;
			value: REAL;
	BEGIN
		signed := FALSE;
		IF (s.type = BugsMappers.char) & (s.char = "-") THEN
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
			value :=  - value
		END;
		node(GraphStochastic.Node).SetValue(value);
		node.SetProps(node.props + {GraphStochastic.initialized})
	END StoreInitValue;

	(* Skip past the name of a variable which is not in the model *)

	PROCEDURE SkipVariable (VAR s: BugsMappers.Scanner): INTEGER;
	BEGIN
		s.Scan;
		IF (s.type # BugsMappers.char) OR (s.char # "[") THEN
			Error(1); RETURN 0; (*	unexpected token, e.g. if supplying variable x with no brackets x[]	*)
		ELSE
			s.Scan;
			IF (s.type = BugsMappers.char) & (s.char = "]") THEN
				(* Do nothing. Found a 1D vector like x[] *)
			ELSIF (s.type = BugsMappers.char) & (s.char = ",") THEN
				LOOP (* found an array variable like x[,1,3,2] *)
					s.Scan;
					IF (s.type = BugsMappers.char) & (s.char = ",") THEN
						Error(7); RETURN 0 (*	second and later slots of column label must not be empty	*)
					ELSIF s.type # BugsMappers.int THEN (* expected an integer - unable to evaluate index *)
						Error(8); RETURN 0
					END;
					s.Scan;
					IF (s.type = BugsMappers.char) & (s.char = "]") THEN
						EXIT
					ELSIF (s.type = BugsMappers.char) & (s.char = ":") THEN
						Error(10); RETURN 0 (* range operator not allowed in index *)
					ELSIF (s.type # BugsMappers.char) OR (s.char # ",") THEN
						Error(18); RETURN 0; (* expected comma *)
					END;
				END
			ELSE
				Error(6); RETURN 0 (* first slot must be empty *)
			END;
		END;
		RETURN 1;
	END SkipVariable;

	PROCEDURE ReadHeader (VAR s: BugsMappers.Scanner): POINTER TO ARRAY OF Header;
		VAR
			i, res, numColums, pos: INTEGER;
			link, list: List;
			name: BugsNames.Name;
			varname: ARRAY 64 OF CHAR;
			variable: BugsParser.Variable;
			header: POINTER TO ARRAY OF Header;
	BEGIN
		list := NIL;
		numColums := 0;
		WHILE (s.type = BugsMappers.string) & (s.string # "NA") DO
			pos := s.Pos();
			variable := BugsParser.ParseVariable(NIL, s);
			varname := s.string$;
			IF variable = NIL THEN (* variable is not in model *)
				res := SkipVariable(s); 
				IF res = 0 THEN
					RETURN NIL
				ELSE
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
					END; (* warning if variable not found but carry on *)
				END;
				NEW(link);
				link.inModel := FALSE;
				INC(numColums);
			ELSE
				IF (s.type # BugsMappers.char) OR (s.char # "]") THEN
					Error(4); (*	expected a closing square bracket ]	*)
					s.SetPos(pos);
					RETURN NIL
				END;
				name := variable.name;
				IF name.numSlots < 1 THEN
					Error(5); (*	colum label can not be scalar node	*)
					s.SetPos(pos);
					RETURN NIL
				END;
				NEW(link);
				link.inModel := TRUE;
				link.name := name;
				NEW(link.indices, name.numSlots);
				INC(numColums);
				IF variable.lower[0] # NIL THEN
					Error(6); (*	first slot of colum label must be empty	*)
					s.SetPos(pos);
					RETURN NIL
				END;
				link.indices[0] := 1;
				i := 1;
				WHILE i < name.numSlots DO
					IF variable.lower[i] = NIL THEN
						Error(7); (*	second and later slots of colum label must not be empty	*)
						s.SetPos(pos);
						RETURN NIL
					END;
					link.indices[i] := BugsEvaluate.Index(variable.lower[i]);
					IF link.indices[i] = BugsEvaluate.error THEN
						s.SetPos(pos);
						Error(8); (*	unable to evaluate column label index	*)
						RETURN NIL
					END;
					IF (name.components # NIL) & (link.indices[i] > name.slotSizes[i]) THEN
						Error(9); (*	column label index is greater than array upper bound	*)
						s.SetPos(pos);
						RETURN NIL
					END;
					INC(i)
				END;
				i := 0;
				WHILE i < name.numSlots DO
					IF variable.upper[i] # NIL THEN
						Error(10); (*	range operator not allowed in column lables	*);
						RETURN NIL
					END;
					INC(i)
				END;
			END;
			link.next := list;
			list := link;
			s.Scan
		END;
		NEW(header, numColums);
		WHILE list # NIL DO
			DEC(numColums);
			header[numColums].inModel := list.inModel;
			IF list.inModel THEN
				header[numColums].name := list.name;
				header[numColums].indices := list.indices;
			END;
			list := list.next
		END;
		RETURN header
	END ReadHeader;

	PROCEDURE CheckData (VAR s: BugsMappers.Scanner; IN header: ARRAY OF Header; OUT ok: BOOLEAN);
		VAR
			status: INTEGER;
			col, i, numCols, offset, row: INTEGER;
			name: BugsNames.Name;
	BEGIN
		numCols := LEN(header);
		ok := TRUE;
		i := 0;
		LOOP
			CheckItem(s, status);
			IF status = error THEN
				ok := FALSE;
				RETURN
			ELSIF status = end THEN
				EXIT
			END;
			col := (i MOD numCols);
			row := (i DIV numCols);
			IF header[col].inModel THEN
				name := header[col].name;
				IF name.components # NIL THEN
					header[col].indices[0] := row + 1;
					offset := name.Offset(header[col].indices);
					IF (name.components[offset] # NIL) & (status = number) THEN
						ok := FALSE;
						Error(11); (*	data value already given for this component	*)
						RETURN
					END
				END;
			END;
			INC(i);
			s.Scan
		END;
		IF col # numCols - 1 THEN
			ok := FALSE;
			Error(12); (*	incomplete row of data	*)
			RETURN
		END;
		col := 0;
		WHILE col < numCols DO
			IF header[col].inModel THEN
				name := header[col].name;
				IF (name.components # NIL) & (name.slotSizes[0] # row) THEN
					ok := FALSE;
					Error(13); (*	wrong number of rows of data	*)
					RETURN
				END;
				name.slotSizes[0] := row + 1;
			END;
			INC(col)
		END;
	END CheckData;

	PROCEDURE AllocateNodes (IN header: ARRAY OF Header);
		VAR
			col, i, numCols: INTEGER;
			name: BugsNames.Name;
	BEGIN
		col := 0;
		numCols := LEN(header);
		WHILE col < numCols DO
			IF header[col].inModel THEN
				name := header[col].name;
				IF name.components = NIL THEN
					i := 1;
					WHILE i < name.numSlots DO
						name.slotSizes[i] := MAX(header[col].indices[i], name.slotSizes[i]);
						INC(i)
					END
				END;
			END;
			INC(col)
		END;
		col := 0;
		WHILE col < numCols DO
			IF header[col].inModel THEN
				name := header[col].name;
				IF name.components = NIL THEN
					name.AllocateNodes
				END;
			END;
			INC(col)
		END
	END AllocateNodes;

	PROCEDURE LoadNumericData (VAR s: BugsMappers.Scanner; IN header: ARRAY OF Header);
		VAR
			i, numCols, numRows, row, col, offset: INTEGER;
			constant: GraphNodes.Node;
	BEGIN
		i := 0;
		numCols := LEN(header);
		WHILE (~ header[i].inModel) & (i < numCols) DO INC(i); END;
		IF (header[i].inModel) THEN
			(* Only need to load data if there is at least one variable in the model *)
			numRows := header[i].name.slotSizes[0];
			i := 0;
			REPEAT
				col := i MOD numCols;
				row := (i DIV numCols);
				IF header[col].inModel THEN
					constant := CreateDataNode(s);
					header[col].indices[0] := row + 1;
					offset := header[col].name.Offset(header[col].indices);
					IF constant # NIL THEN
						header[col].name.components[offset] := constant
					END;
				END;
				s.Scan;
				INC(i);
			UNTIL i = numCols * numRows
		END;
	END LoadNumericData;

	PROCEDURE (l: Loader) Data (VAR s: BugsMappers.Scanner; OUT ok: BOOLEAN);
		VAR
			header: POINTER TO ARRAY OF Header; beg: INTEGER;
			errorMsg: ARRAY 1024 OF CHAR;
	BEGIN
		missingVars := "";
		nmissing := 0;
		beg := s.Pos();
		s.Scan;
		giveWarning := TRUE;
		header := ReadHeader(s);
		IF header = NIL THEN
			ok := FALSE; 
			RETURN
		END;
		giveWarning := FALSE;
		CheckData(s, header, ok);
		IF ~ok THEN
			RETURN
		END;
		s.SetPos(beg);
		s.Scan;
		header := ReadHeader(s);
		AllocateNodes(header);
		LoadNumericData(s, header);
		(* "variables not in the model" warning is concatenated with the list of such variable names *)
		IF (nmissing > 0) THEN
			BugsMsg.Lookup("BugsRectData19", errorMsg);
			errorMsg := " (" + errorMsg + missingVars + ")";
			BugsMsg.Store(errorMsg);
			nmissing := 0;
			missingVars := "";
		END;
	END Data;

	PROCEDURE CheckInits (VAR s: BugsMappers.Scanner; IN header: ARRAY OF Header; OUT ok: BOOLEAN);
		VAR
			status: INTEGER;
			col, i, numCols, offset, row: INTEGER;
			name: BugsNames.Name;
	BEGIN
		ok := TRUE;
		i := 0;
		numCols := LEN(header);
		LOOP
			CheckItem(s, status);
			IF status = error THEN
				ok := FALSE;
				RETURN
			ELSIF status = end THEN
				EXIT
			END;
			col := (i MOD numCols);
			row := (i DIV numCols);
			IF header[col].inModel THEN
				name := header[col].name;
				header[col].indices[0] := row + 1;
				offset := name.Offset(header[col].indices);
				IF name.components = NIL THEN
					ok := FALSE;
					Error(14); (*	no prior specified for this initial value	*)
					RETURN
				END;
				IF status = number THEN
					IF (name.components[offset] = NIL) THEN
						ok := FALSE;
						Error(14); (*	no prior specified for this initial value	*)
						RETURN
					ELSIF ~(name.components[offset] IS GraphStochastic.Node) THEN
						ok := FALSE;
						Error(15); (*	initial value given for non stochastic node	*)
						RETURN
					ELSIF GraphNodes.data IN name.components[offset].props THEN
						ok := FALSE;
						Error(16); (*	initial value given for data node	*)
						RETURN
					END
				END;
			END;
			INC(i);
			s.Scan
		END;
		IF col # numCols - 1 THEN
			ok := FALSE;
			Error(17); (*	incomplete row of initial values	*)
			RETURN
		END
	END CheckInits;

	PROCEDURE LoadNumericalInits (VAR s: BugsMappers.Scanner; IN header: ARRAY OF Header);
		VAR
			col, i, numCols, numRows, offset, row: INTEGER;
			node: GraphNodes.Node;
	BEGIN
		i := 0;
		numRows := header[0].name.slotSizes[0];
		numCols := LEN(header);
		col := 0;
		WHILE (~ header[i].inModel) & (i < numCols) DO INC(i); END;
		IF (header[i].inModel) THEN
			(* Only need to load data if there is at least one variable in the model *)
			REPEAT
				col := i MOD numCols;
				row := i DIV numCols;
				IF header[col].inModel THEN
					header[col].indices[0] := row + 1;
					offset := header[col].name.Offset(header[col].indices);
					node := header[col].name.components[offset];
					StoreInitValue(s, node);
				END;
				s.Scan;
				INC(i);
			UNTIL i = numCols * numRows;
		END
	END LoadNumericalInits;

	PROCEDURE (l: Loader) Inits (VAR s: BugsMappers.Scanner; OUT ok: BOOLEAN);
		VAR
			header: POINTER TO ARRAY OF Header; beg: INTEGER;
			errorMsg: ARRAY 1024 OF CHAR;
	BEGIN
		missingVars := "";
		nmissing := 0;
		beg := s.Pos();
		s.Scan;
		giveWarning := TRUE;
		header := ReadHeader(s);
		IF header = NIL THEN
			ok := FALSE;
			RETURN
		END;
		giveWarning := FALSE;
		CheckInits(s, header, ok);
		IF ~ok THEN
			RETURN
		END;
		s.SetPos(beg);
		s.Scan;
		header := ReadHeader(s);
		LoadNumericalInits(s, header);
		(* "variables not in the model" warning is concatenated with the list of such variable names *)
		IF (nmissing > 0) THEN
			BugsMsg.Lookup("BugsRectData19", errorMsg);
			errorMsg := " (" + errorMsg + missingVars + ")";
			BugsMsg.Store(errorMsg);
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
		nmissing := 0
	END Init;

BEGIN
	Init
END BugsRectData.

