(*		GNU General Public Licence	   *)

(*
license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"
*)

MODULE BugsRectData;


	

	IMPORT
		Strings, 
		BugsData, BugsEvaluate, BugsMappers, BugsMsg, BugsNames, BugsParser,
		GraphNodes, GraphStochastic;

	TYPE
		ColumnLabel = RECORD
			name: BugsNames.Name;
			indices: POINTER TO ARRAY OF INTEGER;
		END;

		ColumnLabels = POINTER TO ARRAY OF ColumnLabel;

		List = POINTER TO RECORD
			label: ColumnLabel;
			next: List
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
		numMissing: INTEGER;
		missingVars: ARRAY 1024 OF CHAR;

	PROCEDURE Error (errorNum: INTEGER);
		VAR
			numToString: ARRAY 8 OF CHAR;
			errorMsg: ARRAY 1024 OF CHAR;
	BEGIN
		Strings.IntToString(errorNum, numToString);
		BugsMsg.Lookup("BugsRectData" + numToString, errorMsg);
		BugsMsg.StoreError(errorMsg);
	END Error;

	PROCEDURE ReadItem (VAR s: BugsMappers.Scanner; OUT status: INTEGER);
		VAR
			signed: BOOLEAN;
	BEGIN
		signed := FALSE;
		IF s.type = BugsMappers.char THEN
			IF s.char # "-" THEN (*	invalid or unexpected token scanned	*)
				status := error; Error(1); RETURN
			END;
			signed := TRUE;
			s.Scan
		END;
		IF (s.type = BugsMappers.string) & (s.string = "NA") THEN
			IF signed THEN (*	NA can not be signed	*)
				status := error; Error(2); RETURN
			END;
			status := na
		ELSIF (s.type = BugsMappers.string) & (s.string = "END") THEN
			status := end
		ELSIF (s.type # BugsMappers.int) & (s.type # BugsMappers.real) THEN
			status := error; Error(3); RETURN
		ELSE
			status := number
		END;
		s.Scan
	END ReadItem;

	PROCEDURE ReadValue (VAR s: BugsMappers.Scanner; OUT value: REAL; OUT nA: BOOLEAN);
		VAR
			signed: BOOLEAN;
	BEGIN
		signed := FALSE;
		nA := FALSE;
		IF (s.type = BugsMappers.char) & (s.char = "-") THEN
			signed := TRUE; s.Scan
		END;
		IF (s.type = BugsMappers.string) & (s.string = "NA") THEN
			value := INF; nA := TRUE;
		ELSIF s.type = BugsMappers.int THEN
			value := s.int
		ELSIF s.type = BugsMappers.real THEN
			value := s.real
		END;
		IF signed THEN value := - value END;
		s.Scan
	END ReadValue;

	(* Skip past the name of a variable which is not in the model *)
	PROCEDURE SkipLabel (VAR s: BugsMappers.Scanner; OUT ok: BOOLEAN);
	BEGIN
		ok := FALSE;
		s.Scan;
		IF (s.type # BugsMappers.char) OR (s.char # "[") THEN
			Error(1); RETURN; (*	unexpected token, e.g. if supplying variable x with no brackets x[]	*)
		ELSE
			s.Scan;
			IF (s.type = BugsMappers.char) & (s.char = "]") THEN
				(* Do nothing. Found a 1D vector like x[] *)
			ELSIF (s.type = BugsMappers.char) & (s.char = ",") THEN
				LOOP (* found an array variable like x[,1,3,2] *)
					s.Scan;
					IF (s.type = BugsMappers.char) & (s.char = ",") THEN
						Error(7); RETURN (*	second and later slots of column label must not be empty	*)
					ELSIF s.type # BugsMappers.int THEN (* expected an integer - unable to evaluate index *)
						Error(8); RETURN
					END;
					s.Scan;
					IF (s.type = BugsMappers.char) & (s.char = "]") THEN
						EXIT
					ELSIF (s.type = BugsMappers.char) & (s.char = ":") THEN
						Error(10); RETURN (* range operator not allowed in index *)
					ELSIF (s.type # BugsMappers.char) OR (s.char # ",") THEN
						Error(18); RETURN (* expected comma *)
					END;
				END
			ELSE
				Error(6); RETURN (* first slot must be empty *)
			END;
		END;
		ok := TRUE
	END SkipLabel;

	PROCEDURE ReadLabels (VAR s: BugsMappers.Scanner; warning: BOOLEAN; 
	OUT labels: ColumnLabels);
		VAR
			i, numColums, pos: INTEGER;
			ok: BOOLEAN;
			link, list: List;
			name: BugsNames.Name;
			varname: ARRAY 64 OF CHAR;
			variable: BugsParser.Variable;
	BEGIN
		list := NIL;
		labels := NIL;
		numColums := 0;
		WHILE (s.type = BugsMappers.string) & (s.string # "NA") DO
			pos := s.Pos();
			variable := BugsParser.ParseVariable(NIL, s);
			varname := s.string$;
			IF variable = NIL THEN (* variable is not in model *)
				SkipLabel(s, ok);
				IF ~ok THEN RETURN END;
				IF warning THEN
					INC(numMissing);
					IF numMissing < 6 THEN
						IF numMissing > 1 THEN
							missingVars := missingVars + ", ";
						END;
						missingVars := missingVars + varname;
					ELSIF numMissing = 6 THEN
						missingVars := missingVars + " and others";
					END;
				END; (* warning if variable not found but carry on *)
				NEW(link);
				link.label.name := NIL;
				link.label.indices := NIL;
				INC(numColums);
			ELSE
				IF (s.type # BugsMappers.char) OR (s.char # "]") THEN
					Error(4); (*	expected a closing square bracket ]	*)
					s.SetPos(pos);
					RETURN
				END;
				name := variable.name;
				IF name.numSlots < 1 THEN
					Error(5); (*	colum label can not be scalar node	*)
					s.SetPos(pos);
					RETURN
				END;
				NEW(link);
				link.label.name := name;
				NEW(link.label.indices, name.numSlots);
				INC(numColums);
				IF variable.lower[0] # NIL THEN
					Error(6); (*	first slot of colum label must be empty	*)
					s.SetPos(pos);
					RETURN
				END;
				link.label.indices[0] := 1;
				i := 1;
				WHILE i < name.numSlots DO
					IF variable.lower[i] = NIL THEN
						Error(7); (*	second and later slots of colum label must not be empty	*)
						s.SetPos(pos);
						RETURN
					END;
					link.label.indices[i] := BugsEvaluate.Index(variable.lower[i]);
					IF link.label.indices[i] = BugsEvaluate.error THEN
						s.SetPos(pos);
						Error(8); (*	unable to evaluate column label index	*)
						RETURN
					END;
					IF (name.components # NIL) & (link.label.indices[i] > name.slotSizes[i]) THEN
						Error(9); (*	column label index is greater than array upper bound	*)
						s.SetPos(pos);
						RETURN
					END;
					INC(i)
				END;
				i := 0;
				WHILE i < name.numSlots DO
					IF variable.upper[i] # NIL THEN
						Error(10); (*	range operator not allowed in column lables	*);
						RETURN
					END;
					INC(i)
				END;
			END;
			link.next := list;
			list := link;
			s.Scan
		END;
		IF list # NIL THEN NEW(labels, numColums) END;
		WHILE list # NIL DO
			DEC(numColums);
			labels[numColums] := list.label;
			list := list.next
		END
	END ReadLabels;

	PROCEDURE CheckData (VAR s: BugsMappers.Scanner; IN labels: ARRAY OF ColumnLabel;
	OUT ok: BOOLEAN);
		VAR
			status: INTEGER;
			col, i, numCols, offset, row: INTEGER;
			name: BugsNames.Name;
	BEGIN
		numCols := LEN(labels);
		ok := TRUE;
		i := 0;
		LOOP
			ReadItem(s, status);
			IF status = error THEN
				ok := FALSE;
				RETURN
			ELSIF status = end THEN
				EXIT
			END;
			col := i MOD numCols;
			row := i DIV numCols;
			IF labels[col].name # NIL THEN
				name := labels[col].name;
				labels[col].indices[0] := row + 1;
				offset := name.Offset(labels[col].indices);
				IF (name.IsDefined(offset)) & (status = number) THEN(*	data value already given	*)
					ok := FALSE; Error(11); RETURN
				END
			END;
			INC(i)
		END;
		IF col # numCols - 1 THEN (*	incomplete row of data	*)
			ok := FALSE; Error(12); RETURN
		END;
		col := 0;
		WHILE col < numCols DO
			IF labels[col].name # NIL THEN
				name := labels[col].name;
				IF (name.slotSizes[0] # row) & (name.slotSizes[0] # 0) THEN (*	wrong number of rows	*)
					ok := FALSE; Error(13); RETURN
				END;
				name.slotSizes[0] := row + 1; (*		????	*)
			END;
			INC(col)
		END;
	END CheckData;

	PROCEDURE AllocateNodes (labels: ColumnLabels);
		VAR
			col, i, numCols: INTEGER;
			name: BugsNames.Name;
	BEGIN
		col := 0;
		numCols := LEN(labels);
		WHILE col < numCols DO
			IF labels[col].name # NIL THEN
				name := labels[col].name;
				i := 1;
				WHILE i < name.numSlots DO
					name.slotSizes[i] := MAX(labels[col].indices[i], name.slotSizes[i]);
					INC(i)
				END
			END;
			INC(col)
		END;
		col := 0;
		WHILE col < numCols DO
			IF labels[col].name # NIL THEN
				name := labels[col].name;
				name.AllocateNodes
			END;
			INC(col)
		END
	END AllocateNodes;

	PROCEDURE LoadData (VAR s: BugsMappers.Scanner; labels: ColumnLabels);
		VAR
			col, count, i, numCols, numRows, row, offset, totalNumVals: INTEGER;
			value: REAL;
			name: BugsNames.Name;
			node: GraphNodes.Node;
			nA: BOOLEAN;
	BEGIN
		i := 0;
		numCols := LEN(labels);
		WHILE (i < numCols) & (labels[i].name = NIL) DO INC(i); END;
		IF i = numCols THEN RETURN END;
		IF labels[i].name # NIL THEN
			(* Only need to load data if there is at least one variable in the model *)
			numRows := labels[i].name.slotSizes[0];
			totalNumVals := numCols * numRows;
			count := 0;
			REPEAT
				col := count MOD numCols;
				row := count DIV numCols;
				ReadValue(s, value, nA);
				IF labels[col].name # NIL THEN
					labels[col].indices[0] := row + 1;
					name := labels[col].name;
					offset := name.Offset(labels[col].indices);
					IF name.isVariable THEN
						IF ~nA THEN
							name.StoreValue(offset, value);
							node := name.components[offset];
							node.SetProps(node.props + {GraphStochastic.data})
						END
					ELSE
						name.StoreValue(offset, value)
					END
				END;
				INC(count);
			UNTIL count = totalNumVals
		END
	END LoadData;

	PROCEDURE (l: Loader) Data (VAR s: BugsMappers.Scanner; OUT ok: BOOLEAN);
		VAR
			labels: ColumnLabels;
			beg: INTEGER;
			warningMsg: ARRAY 1024 OF CHAR;
	BEGIN
		missingVars := "";
		numMissing := 0;
		beg := s.Pos();
		s.Scan;
		ReadLabels(s, TRUE, labels);
		IF labels = NIL THEN ok := FALSE; RETURN END;
		CheckData(s, labels, ok);
		IF ~ok THEN RETURN END;
		s.SetPos(beg);
		s.Scan;
		ReadLabels(s, FALSE, labels);
		AllocateNodes(labels);
		LoadData(s, labels);
		(* "variables not in the model" warning is concatenated with the list of such variable names *)
		IF numMissing > 0 THEN
			BugsMsg.Lookup("BugsRectData19", warningMsg);
			warningMsg := " (" + warningMsg + missingVars + ")";
			BugsMsg.StoreMsg(warningMsg);
			numMissing := 0;
			missingVars := "";
		END;
	END Data;

	PROCEDURE CheckInits (VAR s: BugsMappers.Scanner; labels: ColumnLabels; OUT ok: BOOLEAN);
		VAR
			status: INTEGER;
			col, i, numCols, offset, row: INTEGER;
			name: BugsNames.Name;
	BEGIN
		ok := TRUE;
		i := 0;
		numCols := LEN(labels);
		LOOP
			ReadItem(s, status);
			IF status = error THEN
				ok := FALSE;
				RETURN
			ELSIF status = end THEN
				EXIT
			END;
			col := i MOD numCols;
			row := i DIV numCols;
			IF labels[col].name # NIL THEN
				name := labels[col].name;
				IF ~name.isVariable THEN (*	initial value given for data node	*)
					ok := FALSE; Error(16); RETURN
				END;
				labels[col].indices[0] := row + 1;
				offset := name.Offset(labels[col].indices);
				IF name.components = NIL THEN (*	no prior specified for this initial value	*)
					ok := FALSE; Error(14); RETURN
				END;
				IF status = number THEN
					IF name.components[offset] = NIL THEN
						ok := FALSE; Error(14); RETURN
					END;
					IF ~(name.components[offset] IS GraphStochastic.Node) THEN
						ok := FALSE; Error(15); RETURN
					END;
					IF GraphNodes.data IN name.components[offset].props THEN
						ok := FALSE; Error(16); RETURN
					END
				END
			END;
			INC(i);
			s.Scan
		END;
		IF col # numCols - 1 THEN (*	incomplete row of initial values	*)
			ok := FALSE; Error(17); RETURN
		END
	END CheckInits;

	PROCEDURE LoadInits (VAR s: BugsMappers.Scanner; labels: ColumnLabels);
		VAR
			col, count, i, numCols, numRows, offset, row, totalNumVals: INTEGER;
			value: REAL;
			name: BugsNames.Name;
			node: GraphNodes.Node;
			nA: BOOLEAN;
	BEGIN
		i := 0;
		numCols := LEN(labels);
		WHILE (labels[i].name = NIL) & (i < numCols) DO INC(i); END;
		IF labels[i].name # NIL THEN
			(* Only need to load data if there is at least one variable in the model *)
			numRows := labels[i].name.slotSizes[0];
			totalNumVals := numCols * numRows;
			count := 0;
			REPEAT
				col := count MOD numCols;
				row := count DIV numCols;
				ReadValue(s, value, nA);
				IF labels[col].name # NIL THEN
					labels[col].indices[0] := row + 1;
					IF ~nA THEN 
						offset := labels[col].name.Offset(labels[col].indices);
						name := labels[col].name;
						node := name.components[offset];
						name.StoreValue(offset, value);
						node.SetProps(node.props + {GraphStochastic.initialized})
					END
				END;
				INC(count);
			UNTIL count = totalNumVals
		END
	END LoadInits;

	PROCEDURE (l: Loader) Inits (VAR s: BugsMappers.Scanner; OUT ok: BOOLEAN);
		VAR
			labels: ColumnLabels;
			beg: INTEGER;
			warningMsg: ARRAY 1024 OF CHAR;
	BEGIN
		missingVars := "";
		numMissing := 0;
		beg := s.Pos();
		s.Scan;
		ReadLabels(s, TRUE, labels);
		IF labels = NIL THEN ok := FALSE; RETURN END;
		CheckInits(s, labels, ok);
		IF ~ok THEN RETURN END;
		s.SetPos(beg);
		s.Scan;
		ReadLabels(s, FALSE, labels);
		LoadInits(s, labels);
		(* "variables not in the model" warning is concatenated with the list of such variable names *)
		IF numMissing > 0 THEN
			BugsMsg.Lookup("BugsRectData19", warningMsg);
			warningMsg := " (" + warningMsg + missingVars + ")";
			BugsMsg.StoreMsg(warningMsg);
			numMissing := 0;
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
		missingVars := "";
		numMissing := 0
	END Init;

BEGIN
	Init
END BugsRectData.

