
(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsRobjects;

	

	IMPORT
		SYSTEM,
		Math, Strings,
		BugsIndex, BugsMsg, BugsNames, BugsRandnum, BugsVariables,
		GraphConstant, GraphLogical, GraphNodes, GraphStochastic,
		UpdaterActions;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 20 OF CHAR;
		errorNum-: INTEGER;
		errorPos-: INTEGER;
		chain-: INTEGER;
		node: BugsNames.Name;

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

	PROCEDURE AddNode* (name: ARRAY OF CHAR);
	BEGIN
		BugsVariables.StoreName(name);
	END AddNode;
	
	PROCEDURE Allocate*;
	BEGIN
		IF (node # NIL) & (node.components # NIL) THEN node.AllocateNodes END
	END Allocate;

	PROCEDURE CheckData* (IN values: ARRAY OF REAL);
		VAR
			i, size: INTEGER;
			p: GraphNodes.Node;
			x: REAL;
	BEGIN
		errorNum := 0;
		size := LEN(values);
		IF node.components = NIL THEN RETURN END;
		ASSERT(size = node.Size(), 20);
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

	PROCEDURE CheckInits* (IN values: ARRAY OF REAL);
		VAR
			i, size: INTEGER;
			p: GraphNodes.Node;
			x: REAL;
	BEGIN
		errorNum := 0;
		size := LEN(values);
		ASSERT(node.components # NIL, 21);
		ASSERT(size = node.Size(), 20);
		UpdaterActions.LoadSamples(chain);
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
		END;
		UpdaterActions.LoadSamples(0)
	END CheckInits;

	PROCEDURE GetDimensions* (OUT dim: ARRAY OF INTEGER);
		VAR
			i, numSlots: INTEGER;
	BEGIN
		numSlots := node.numSlots;
		ASSERT(numSlots = LEN(dim), 20);
		i := 0; WHILE i < numSlots DO dim[i] := node.slotSizes[i]; INC(i) END
	END GetDimensions;

	PROCEDURE GetNumChains* (): INTEGER;
	BEGIN
		RETURN BugsRandnum.numberChains
	END GetNumChains;
	
	PROCEDURE GetNumDimensions* (): INTEGER;
		VAR
			numSlots: INTEGER;
	BEGIN
		numSlots := - 1;
		IF node # NIL THEN
			numSlots := node.numSlots
		END;
		RETURN numSlots
	END GetNumDimensions;

	PROCEDURE GetNumNames* (): INTEGER;
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
	END GetNumNames;

	PROCEDURE GetSize* (): INTEGER;
		VAR
			size: INTEGER;
	BEGIN
		size := 0;
		IF (node # NIL) & (node.components # NIL) THEN
			size := LEN(node.components)
		END;
		RETURN size
	END GetSize;

	PROCEDURE GetStringLength* (): INTEGER;
	BEGIN
		RETURN LEN(node.string$)
	END GetStringLength;

	PROCEDURE GetValues* (OUT values: ARRAY OF REAL);
		VAR
			i, j, size, numChains: INTEGER;
			p: GraphNodes.Node;
	BEGIN
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
		UpdaterActions.LoadSamples(0)
	END GetValues;

	PROCEDURE GetVariable* (OUT var: ARRAY OF CHAR);
	BEGIN
		var := node.string$
	END GetVariable;

	PROCEDURE IsAllocated* (): BOOLEAN;
	BEGIN
		RETURN (node # NIL) & (node.components # NIL)
	END IsAllocated;

	PROCEDURE IsNode* (): BOOLEAN;
	BEGIN
		RETURN node # NIL
	END IsNode;

	PROCEDURE LoadData* (IN values: ARRAY OF REAL);
		VAR
			i, size: INTEGER;
			p: GraphNodes.Node;
			x: REAL;
	BEGIN
		size := LEN(values);
		ASSERT(node.components # NIL, 21);
		ASSERT(size = node.Size(), 20);
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

	PROCEDURE LoadInits* (IN values: ARRAY OF REAL);
		VAR
			i, size: INTEGER;
			p: GraphNodes.Node;
			x: REAL;
	BEGIN
		size := LEN(values);
		ASSERT(node.components # NIL, 21);
		ASSERT(size = node.Size(), 20);
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

	PROCEDURE SetChain* (c: INTEGER);
	BEGIN
		chain := c
	END SetChain;

	PROCEDURE SetDimensions* (IN dim: ARRAY OF INTEGER);
		VAR
			i, numSlots: INTEGER;
	BEGIN
		numSlots := LEN(dim);
		ASSERT(numSlots = node.numSlots, 20);
		i := 0; WHILE i < numSlots DO node.slotSizes[i] := dim[i]; INC(i) END
	END SetDimensions;

	PROCEDURE SetValues* (IN values: ARRAY OF REAL);
		VAR
			i, j, size, numChains: INTEGER;
			p: GraphNodes.Node;
			x: REAL;
	BEGIN
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
		UpdaterActions.LoadSamples(0)
	END SetValues;
	
	PROCEDURE SetVariableByName* (IN var: ARRAY OF CHAR);
	BEGIN
		node := BugsIndex.Find(var)
	END SetVariableByName;

	PROCEDURE SetVariableByIndex* (index: INTEGER);
		VAR
			i, j, pos: INTEGER;
	BEGIN
		i := 0;
		j := 0;
		REPEAT
			node := BugsIndex.FindByNumber(i);
			IF node # NIL THEN
				Strings.Find(node.string, "(", 0, pos);
				IF pos = - 1 THEN
					(*	only consider proper node names and not F() or D(,)	*)
					INC(j);
				END
			END;
			INC(i)
		UNTIL (node = NIL) OR (j > index);
	END SetVariableByIndex;

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

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		node := NIL;
		chain := 0;
		Maintainer
	END Init;

BEGIN
	Init
END BugsRobjects.

