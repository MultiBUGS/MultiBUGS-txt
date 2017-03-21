
(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsRobjects;

	

	IMPORT
		SYSTEM,
		Math, Strings,
		BugsFiles, BugsIndex, BugsMsg, BugsNames, BugsRandnum,
		GraphConstant, GraphLogical, GraphNodes, GraphStochastic,
		UpdaterActions;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 20 OF CHAR;
		errorNum-: INTEGER;
		errorPos-: INTEGER;
		variable: ARRAY 1024 OF CHAR;

	PROCEDURE GetStringLength* (): INTEGER;
	BEGIN
		RETURN LEN(variable$)
	END GetStringLength;

	PROCEDURE IsNA* (x: REAL): BOOLEAN;
		TYPE
			Real = RECORD[union]
				value: REAL;
				words: ARRAY[untagged] 2 OF INTEGER
			END;
		VAR
			real: Real;
			isNA: BOOLEAN;
	BEGIN
		real.value := x;
		isNA := real.words[0] = 1954;
		IF ~isNA THEN isNA := Math.Exponent(x) = MAX(INTEGER) END;
		RETURN isNA
	END IsNA;

	PROCEDURE IsNode* (): BOOLEAN;
	BEGIN
		RETURN BugsIndex.Find(variable) # NIL
	END IsNode;

	PROCEDURE SetVariable* (IN var: ARRAY OF CHAR);
	BEGIN
		variable := var$
	END SetVariable;

	PROCEDURE SetGuard* (OUT ok: BOOLEAN);
		VAR
			msg: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		ok := BugsIndex.Find(variable) # NIL;
		IF ~ok THEN
			p[0] := variable$;
			BugsMsg.MapParamMsg("BugsRobjects:NotVariable", p, msg);
			BugsFiles.ShowMsg(msg)
		END
	END SetGuard;

	PROCEDURE Allocate*;
		VAR
			node: BugsNames.Name;
	BEGIN
		node := BugsIndex.Find(variable);
		IF node # NIL THEN node.AllocateNodes END
	END Allocate;

	PROCEDURE IsAllocated* (): BOOLEAN;
		VAR
			node: BugsNames.Name;
	BEGIN
		node := BugsIndex.Find(variable);
		RETURN (node # NIL) & (node.components # NIL)
	END IsAllocated;

	PROCEDURE GetSize* (): INTEGER;
		VAR
			size: INTEGER;
			node: BugsNames.Name;
	BEGIN
		node := BugsIndex.Find(variable);
		size := - 1;
		IF (node # NIL) & (node.components # NIL) THEN
			size := LEN(node.components)
		END;
		RETURN size
	END GetSize;

	PROCEDURE GetNumDimensions* (): INTEGER;
		VAR
			numSlots: INTEGER;
			node: BugsNames.Name;
	BEGIN
		node := BugsIndex.Find(variable);
		numSlots := - 1;
		IF node # NIL THEN
			numSlots := node.numSlots
		END;
		RETURN numSlots
	END GetNumDimensions;

	PROCEDURE GetDimensions* (OUT dim: ARRAY OF INTEGER);
		VAR
			i, numSlots: INTEGER;
			node: BugsNames.Name;
	BEGIN
		node := BugsIndex.Find(variable);
		numSlots := GetNumDimensions();
		ASSERT(numSlots = LEN(dim), 20);
		i := 0; WHILE i < numSlots DO dim[i] := node.slotSizes[i]; INC(i) END
	END GetDimensions;

	PROCEDURE SetDimensions* (IN dim: ARRAY OF INTEGER);
		VAR
			i, numSlots: INTEGER;
			node: BugsNames.Name;
	BEGIN
		node := BugsIndex.Find(variable);
		numSlots := LEN(dim);
		ASSERT(numSlots = GetNumDimensions(), 20);
		i := 0; WHILE i < numSlots DO node.slotSizes[i] := dim[i]; INC(i) END
	END SetDimensions;

	PROCEDURE TransposeDimensions* (VAR dim: ARRAY OF INTEGER);
		VAR
			temp: POINTER TO ARRAY OF INTEGER;
			i, numSlots: INTEGER;
	BEGIN
		numSlots := LEN(dim);
		NEW(temp, numSlots);
		i := 0; WHILE i < numSlots DO temp[i] := dim[i]; INC(i) END;
		i := 0; WHILE i < numSlots DO dim[i] := temp[numSlots - 1 - i]; INC(i) END
	END TransposeDimensions;

	PROCEDURE GetValues* (OUT values: ARRAY OF REAL);
		VAR
			i, j, size, numChains: INTEGER;
			p: GraphNodes.Node;
			node: BugsNames.Name;
	BEGIN
		node := BugsIndex.Find(variable);
		numChains := BugsRandnum.numberChains;
		size := LEN(values) DIV numChains;
		j := 0;
		WHILE j < numChains DO
			UpdaterActions.LoadSamples(j);
			i := 0;
			WHILE i < size DO
				p := node.components[i];
				IF p # NIL THEN
					values[j * size + i] := p.Value();
				END;
				INC(i)
			END;
			INC(j)
		END;
	END GetValues;

	PROCEDURE SetValues* (IN values: ARRAY OF REAL);
		VAR
			i, j, size, numChains: INTEGER;
			p: GraphNodes.Node;
			node: BugsNames.Name;
			x: REAL;
	BEGIN
		node := BugsIndex.Find(variable);
		numChains := BugsRandnum.numberChains;
		size := LEN(values) DIV numChains;
		j := 0;
		WHILE j < numChains DO
			UpdaterActions.LoadSamples(j);
			i := 0;
			WHILE i < size DO
				x := values[j * size + i];
				IF ~IsNA(x) THEN
					p := node.components[i];
					IF p # NIL THEN
						WITH p: GraphStochastic.Node DO
							IF GraphStochastic.update IN p.props THEN p.SetValue(x) END
						ELSE
						END
					END
				END;
				INC(i)
			END;
			UpdaterActions.StoreSamples(j);
			INC(j)
		END;
	END SetValues;

	PROCEDURE CheckData* (IN values: ARRAY OF REAL);
		VAR
			i, size: INTEGER;
			p: GraphNodes.Node;
			node: BugsNames.Name;
			x: REAL;
	BEGIN
		node := BugsIndex.Find(variable);
		IF node.components = NIL THEN RETURN END;
		size := LEN(values);
		ASSERT(size = GetSize(), 20);
		i := 0;
		errorNum := 0;
		WHILE i < size DO
			p := node.components[i];
			IF p # NIL THEN
				x := values[i];
				IF ~IsNA(x) THEN
					IF p IS GraphLogical.Node THEN
						errorNum := 1; (*	node is logical	*)
						errorPos := i;
						RETURN
					ELSIF GraphNodes.data IN p.props THEN
						errorNum := 2; (*	data value already given for node	*)
						errorPos := i;
						RETURN
					END
				END
			END;
			INC(i)
		END
	END CheckData;

	PROCEDURE LoadData* (IN values: ARRAY OF REAL);
		VAR
			i, size: INTEGER;
			p: GraphNodes.Node;
			node: BugsNames.Name;
			x: REAL;
	BEGIN
		node := BugsIndex.Find(variable);
		ASSERT(node.components # NIL, 21);
		size := LEN(values);
		ASSERT(size = GetSize(), 20);
		i := 0;
		WHILE i < size DO
			p := node.components[i];
			x := values[i];
			IF ~IsNA(x) THEN
				IF p # NIL THEN
					IF p IS GraphStochastic.Node THEN
						p(GraphStochastic.Node).SetValue(values[i]);
						p.SetProps(p.props + {GraphNodes.data})
					END
				ELSE
					node.components[i] := GraphConstant.New(x)
				END
			END;
			INC(i)
		END
	END LoadData;

	PROCEDURE CheckInits* (IN values: ARRAY OF REAL);
		VAR
			i, size: INTEGER;
			p: GraphNodes.Node;
			node: BugsNames.Name;
			x: REAL;
	BEGIN
		node := BugsIndex.Find(variable);
		size := LEN(values);
		ASSERT(size = GetSize(), 20);
		ASSERT(node.components # NIL, 21);
		i := 0;
		errorNum := 0;
		WHILE i < size DO
			x := values[i];
			IF ~IsNA(x) THEN
				p := node.components[i];
				IF p = NIL THEN
					errorNum := 3; (*	no prior for node	*)
					errorPos := i;
					RETURN
				ELSE
					IF ~(p IS GraphStochastic.Node) THEN
						errorNum := 4; (*	node is not stochastic	*)
						errorPos := i;
						RETURN
					ELSIF GraphNodes.data IN p.props THEN
						errorNum := 5; (*	node is data	*)
						errorPos := i;
						RETURN
					END
				END
			END;
			INC(i)
		END
	END CheckInits;

	PROCEDURE LoadInits* (IN values: ARRAY OF REAL);
		VAR
			i, size: INTEGER;
			p: GraphNodes.Node;
			node: BugsNames.Name;
			x: REAL;
	BEGIN
		node := BugsIndex.Find(variable);
		size := LEN(values);
		ASSERT(size = GetSize(), 20);
		i := 0;
		WHILE i < size DO
			x := values[i];
			IF ~IsNA(x) THEN
				p := node.components[i];
				p(GraphStochastic.Node).SetValue(x);
				p.SetProps(p.props + {GraphStochastic.initialized})
			END;
			INC(i)
		END
	END LoadInits;

	PROCEDURE SetIndex* (index: INTEGER);
		VAR
			name: BugsNames.Name;
			i, j, pos: INTEGER;
	BEGIN
		i := 0;
		j := 0;
		REPEAT
			name := BugsIndex.FindByNumber(i);
			IF name # NIL THEN
				Strings.Find(name.string, "(", 0, pos);
				IF pos = - 1 THEN
					(*	only consider proper node names and not F() or D(,)	*)
					INC(j);
				END
			END;
			INC(i)
		UNTIL (name = NIL) OR (j > index);
		IF name # NIL THEN variable := name.string$ ELSE variable := "" END;
	END SetIndex;

	PROCEDURE GetNumberNames* (): INTEGER;
		VAR
			i, numberNames, pos: INTEGER;
			name: BugsNames.Name;
	BEGIN
		i := 0;
		numberNames := 0;
		REPEAT
			name := BugsIndex.FindByNumber(i);
			IF name # NIL THEN
				Strings.Find(name.string, "(", 0, pos);
				IF pos = - 1 THEN
					(*	only consider proper node names and not F() or D(,)	*)
					INC(numberNames);
				END
			END;
			INC(i)
		UNTIL name = NIL;
		RETURN numberNames
	END GetNumberNames;

	PROCEDURE GetVariable* (OUT var: ARRAY OF CHAR);
	BEGIN
		var := variable$
	END GetVariable;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		variable := "";
		Maintainer
	END Init;

BEGIN
	Init
END BugsRobjects.

