(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsEvaluate;


	

	IMPORT
		Strings,
		BugsMsg, BugsNames, BugsParser,
		GraphConstant, GraphGrammar, GraphLogical, GraphMixture, GraphNodes, GraphStochastic;

	TYPE
		Visitor* = POINTER TO ABSTRACT RECORD(BugsParser.Visitor) END;

	CONST
		error* = MIN(INTEGER);

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (visitor: Visitor) Node* (statement: BugsParser.Statement; OUT ok: BOOLEAN), NEW, ABSTRACT;

	PROCEDURE Error (errorNum: INTEGER; IN name: ARRAY OF CHAR);
		VAR
			errorMsg: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
			numToString: ARRAY 8 OF CHAR;
	BEGIN
		Strings.IntToString(errorNum, numToString);
		p[0] := name$;
		BugsMsg.LookupParam("BugsEvaluate" + numToString, p, errorMsg);
		BugsMsg.StoreError(errorMsg)
	END Error;

	PROCEDURE Index* (t: BugsParser.Node): INTEGER;
		CONST
			eps = 1.0E-5;
		VAR
			i, intValue, left, numSlots, offset, right, key: INTEGER;
			abs, value: REAL;
			indices: ARRAY 20 OF INTEGER;
			string: ARRAY 80 OF CHAR;
			bin: BugsParser.Binary;
			int: BugsParser.Integer;
			index: BugsParser.Index;
			name: BugsNames.Name;
			variable: BugsParser.Variable;
			node: GraphNodes.Node;
			internal: BugsParser.Internal;
	BEGIN
		IF t IS BugsParser.Binary THEN
			bin := t(BugsParser.Binary);
			left := Index(bin.left);
			IF left = error THEN RETURN error END;
			IF bin.right # NIL THEN
				right := Index(bin.right);
				IF right = error THEN RETURN error END
			END;
			IF bin.op = GraphGrammar.add THEN
				RETURN left + right
			ELSIF bin.op = GraphGrammar.sub THEN
				IF bin.right # NIL THEN
					RETURN left - right
				ELSE
					RETURN - left
				END
			ELSIF bin.op = GraphGrammar.mult THEN
				RETURN left * right
			ELSIF bin.op = GraphGrammar.div THEN
				IF right = 0 THEN
					Error(1, ""); (*	integer divide by zero	*)
					RETURN error
				END;
				RETURN left DIV right
			END
		ELSIF t IS BugsParser.Internal THEN
			internal := t(BugsParser.Internal);
			left := Index(internal.parents[0]);
			key := internal.descriptor.key;
			IF left = error THEN RETURN error END;
			IF LEN(internal.parents) = 2 THEN
				right := Index(internal.parents[1]);
				IF right = error THEN RETURN error END;
				CASE key  OF
				|GraphGrammar.equals:
					IF left = right THEN RETURN 1 ELSE RETURN 0 END
				|GraphGrammar.max:
					RETURN MAX(left, right)
				|GraphGrammar.min:
					RETURN MIN(left, right)
				END
			ELSE
				ASSERT(key = GraphGrammar.step, 21);
				IF left >= right THEN RETURN 1 ELSE RETURN 0 END
			END;
		ELSIF t IS BugsParser.Variable THEN
			variable := t(BugsParser.Variable);
			name := variable.name;
			IF name.passByReference & (name.components = NIL) THEN
				Error(2, name.string); (*	variable not defined	*)
				RETURN error
			END;
			i := 0;
			numSlots := name.numSlots;
			WHILE i < numSlots DO
				IF variable.lower[i] = NIL THEN
					Error(3, name.string); (*	missing index	*)
					RETURN error
				ELSIF variable.upper[i] # NIL THEN
					Error(4, name.string); (*	invalid use of range	*)
					RETURN error
				END;
				indices[i] := Index(variable.lower[i]);
				IF indices[i] = error THEN
					RETURN error
				END;
				INC(i)
			END;
			offset := name.Offset(indices);
			IF offset >= name.Size() THEN
				Error(5, name.string); (*	index out of range	*)
				RETURN error
			END;
			IF ~name.IsDefined(offset) THEN
				Error(2, name.string); (*	variable not defined	*)
				RETURN error
			END;
			IF name.passByReference THEN
				node := name.components[offset];
				IF ~(GraphNodes.data IN node.props) THEN
					name.Indices(offset, string);
					Error(6, name.string + string); (*	variable not data	*)
					RETURN error
				END
			END;
			value := name.Value(offset);
			abs := ABS(value);
			intValue := SHORT(ENTIER(abs + eps));
			IF (abs - intValue) > eps THEN
				Error(8, name.string); (*	index is not an integer	*)
				RETURN error
			END;
			IF value < 0 THEN
				RETURN - intValue
			ELSE
				RETURN intValue
			END
		ELSIF t IS BugsParser.Integer THEN
			int := t(BugsParser.Integer);
			RETURN int.integer
		ELSIF t IS BugsParser.Index THEN
			index := t(BugsParser.Index);
			RETURN index.intVal
		END
	END Index;

	PROCEDURE Mask (var: BugsParser.Variable): POINTER TO ARRAY OF BOOLEAN;
		VAR
			i, numSlots, nElem: INTEGER;
			mask: POINTER TO ARRAY OF BOOLEAN;
			indices, lower, upper: ARRAY 20 OF INTEGER;
			name: BugsNames.Name;

		PROCEDURE Loop (slot: INTEGER; VAR indices, lower, upper: ARRAY OF INTEGER);
			VAR
				j, offset, slot1: INTEGER;
		BEGIN
			slot1 := slot + 1;
			j := lower[slot];
			WHILE j <= upper[slot] DO
				indices[slot] := j;
				numSlots := name.numSlots;
				IF slot1 = numSlots THEN
					offset := name.Offset(indices);
					mask[offset] := TRUE
				ELSE
					Loop(slot1, indices, lower, upper)
				END;
				INC(j)
			END
		END Loop;

	BEGIN
		name := var.name;
		i := 0;
		numSlots := name.numSlots;
		WHILE i < numSlots DO
			IF var.lower[i] = NIL THEN
				lower[i] := 1
			ELSE
				lower[i] := Index(var.lower[i]);
				IF lower[i] = error THEN
					RETURN NIL
				END;
				IF lower[i] < 1 THEN
					Error(11, name.string);
					RETURN NIL
				END;
				IF lower[i] > name.slotSizes[i] THEN
					Error(13, name.string);
					RETURN NIL
				END
			END;
			IF var.upper[i] = NIL THEN
				IF var.lower[i] = NIL THEN
					upper[i] := name.slotSizes[i]
				ELSE
					upper[i] := lower[i]
				END
			ELSE
				upper[i] := Index(var.upper[i]);
				IF upper[i] = error THEN
					RETURN NIL
				END;
				IF upper[i] < lower[i] THEN
					Error(16, name.string); (*	invalid range specified	*)
					RETURN NIL
				END;
				IF upper[i] > name.slotSizes[i] THEN
					Error(13, name.string); (*	array index greater than array bounds	*)
					RETURN NIL
				END
			END;
			INC(i)
		END;
		nElem := name.Size();
		NEW(mask, nElem);
		i := 0;
		WHILE i < nElem DO
			mask[i] := FALSE;
			INC(i)
		END;
		IF name.numSlots = 0 THEN
			mask[0] := TRUE
		ELSE
			Loop(0, indices, lower, upper)
		END;
		RETURN mask
	END Mask;

	PROCEDURE Len (IN mask: ARRAY OF BOOLEAN): INTEGER;
		VAR
			i, len, nElem: INTEGER;
	BEGIN
		nElem := LEN(mask);
		len := 0;
		i := 0;
		WHILE i < nElem DO
			IF mask[i] THEN INC(len) END;
			INC(i)
		END;
		RETURN len
	END Len;

	PROCEDURE Offsets* (var: BugsParser.Variable): POINTER TO ARRAY OF INTEGER;
		VAR
			i, len, num, nElem: INTEGER;
			mask: POINTER TO ARRAY OF BOOLEAN;
			offsets: POINTER TO ARRAY OF INTEGER;
	BEGIN
		mask := Mask(var);
		IF mask = NIL THEN RETURN NIL END;
		len := Len(mask);
		IF len = 0 THEN RETURN NIL END;
		NEW(offsets, len);
		i := 0;
		num := 0;
		nElem := LEN(mask);
		WHILE i < nElem DO
			IF mask[i] THEN
				offsets[num] := i;
				INC(num)
			END;
			INC(i)
		END;
		RETURN offsets
	END Offsets;

	PROCEDURE (visitor: Visitor) Do* (statement: BugsParser.Statement; OUT ok: BOOLEAN);
		VAR
			i, lower, upper: INTEGER;
			index: BugsParser.Index;
	BEGIN
		ok := TRUE;
		IF statement.index # NIL THEN
			index := statement.index;
			lower := Index(index.lower);
			IF lower = error THEN
				ok := FALSE; RETURN
			END;
			upper := Index(index.upper);
			IF upper = error THEN
				ok := FALSE; RETURN
			END;
			i := lower;
			statement := statement.next;
			WHILE (i <= upper) & ok DO
				index.intVal := i;
				visitor.Do(statement, ok);
				INC(i)
			END
		ELSE
			visitor.Node(statement, ok)
		END
	END Do;

	PROCEDURE LHVariable* (variable: BugsParser.Variable): GraphNodes.SubVector;
		VAR
			i, numSlots, upper: INTEGER;
			indices: ARRAY 20 OF INTEGER;
			name: BugsNames.Name;
			vector: GraphNodes.SubVector;
	BEGIN
		vector := GraphNodes.NewVector();
		name := variable.name;
		IF name.components = NIL THEN Error(2, name.string); RETURN NIL END;
		i := 0;
		vector.nElem := 1;
		vector.step := 1;
		numSlots := name.numSlots;
		WHILE i < numSlots DO
			IF variable.lower[i] = NIL THEN
				indices[i] := 1;
				vector.nElem := vector.nElem * name.slotSizes[i];
				vector.step := name.Step(i)
			ELSIF variable.upper[i] # NIL THEN
				indices[i] := Index(variable.lower[i]);
				IF indices[i] = error THEN RETURN NIL END;
				upper := Index(variable.upper[i]);
				IF upper = error THEN RETURN NIL END;
				IF upper > name.slotSizes[i] THEN (*	array index greater than array bounds	*)
					Error(9, name.string); RETURN NIL
				END;
				IF upper < indices[i] THEN (*	invalid range specified	*)
					Error(10, name.string); RETURN NIL
				END;
				vector.nElem := vector.nElem * (upper + 1 - indices[i]);
				vector.step := name.Step(i)
			ELSE
				indices[i] := Index(variable.lower[i]);
				IF indices[i] = error THEN RETURN NIL
				END
			END;
			IF indices[i] < 1 THEN (*	array index is less than one	*)
				Error(11, name.string); RETURN NIL
			END;
			INC(i)
		END;
		vector.components := name.components;
		vector.start := name.Offset(indices);
		RETURN vector
	END LHVariable;

	PROCEDURE RHRef (tree: BugsParser.Node; optimizeData: BOOLEAN): GraphNodes.Node;
		VAR
			i, k, numSlots, numVarIndex, offset: INTEGER;
			value: REAL;
			res: SET;
			indices: ARRAY 20 OF INTEGER;
			argL: GraphStochastic.ArgsLogical;
			node: GraphNodes.Node;
			integer: BugsParser.Integer;
			real: BugsParser.Real;
			variable: BugsParser.Variable;
			index: BugsParser.Index;
			name: BugsNames.Name;
	BEGIN
		IF tree IS BugsParser.Variable THEN
			variable := tree(BugsParser.Variable);
			name := variable.name;
			i := 0;
			numVarIndex := 0;
			argL.Init;
			numSlots := name.numSlots;
			WHILE i < numSlots DO
				IF variable.lower[i] = NIL THEN RETURN NIL END;
				IF variable.lower[i] IS BugsParser.Variable THEN
					node := RHRef(variable.lower[i], FALSE);
					IF node = NIL THEN RETURN NIL END;
					IF ~(node IS GraphLogical.Node) & (GraphNodes.data IN node.props) THEN
						indices[i] := Index(variable.lower[i]);
						IF indices[i] = error THEN RETURN NIL END
					ELSE
						indices[i] := 1;
						argL.scalars[numVarIndex] := node;
						k := 2 * numVarIndex;
						argL.ops[k] := name.slotSizes[i];
						argL.ops[k + 1] := name.Step(i);
						INC(numVarIndex)
					END
				ELSE
					indices[i] := Index(variable.lower[i]);
					IF indices[i] = error THEN RETURN NIL END
				END;
				IF name.slotSizes[i] < 1 THEN
					Error(18, name.string); RETURN NIL
				ELSIF indices[i] < 1 THEN (*	array index is less than one	*)
					Error(12, name.string); RETURN NIL
				ELSIF indices[i] > name.slotSizes[i] THEN (* 	array index greater than array bounds	*)
					Error(13, name.string); RETURN NIL
				END;
				INC(i)
			END;
			IF numVarIndex = 0 THEN
				offset := name.Offset(indices);
				IF name.passByReference THEN
					node := variable.name.components[offset];
					IF node = NIL THEN Error(14, name.string); RETURN NIL END
				ELSE
					IF ~variable.name.IsDefined(offset) THEN Error(14, name.string); RETURN NIL END;
					value := variable.name.Value(offset);
					IF optimizeData THEN
						node := GraphConstant.Old(value);
					ELSE
						node := GraphConstant.New(value)
					END;
				END
			ELSE
				argL.vectors[0] := GraphNodes.NewVector();
				argL.vectors[0].components := name.components;
				argL.vectors[0].start := name.Offset(indices);
				argL.numScalars := numVarIndex;
				node := GraphMixture.fact.New();
				node.Set(argL, res);
				IF res # {} THEN Error(14, name.string); RETURN NIL END
			END
		ELSIF tree IS BugsParser.Integer THEN
			integer := tree(BugsParser.Integer);
			IF optimizeData THEN
				node := GraphConstant.Old(integer.integer);
			ELSE
				node := GraphConstant.New(integer.integer)
			END;
		ELSIF tree IS BugsParser.Real THEN
			real := tree(BugsParser.Real);
			IF optimizeData THEN
				node := GraphConstant.Old(real.real);
			ELSE
				node := GraphConstant.New(real.real)
			END;
		ELSIF tree IS BugsParser.Index THEN
			index := tree(BugsParser.Index);
			IF optimizeData THEN
				node := GraphConstant.Old(index.intVal);
			ELSE
				node := GraphConstant.New(index.intVal)
			END
		ELSE
			RETURN NIL
		END;
		RETURN node
	END RHRef;

	PROCEDURE RHScalar* (tree: BugsParser.Node): GraphNodes.Node;
	BEGIN
		RETURN RHRef(tree, FALSE)
	END RHScalar;

	PROCEDURE RHScalarOpt* (tree: BugsParser.Node): GraphNodes.Node;
	BEGIN
		RETURN RHRef(tree, TRUE)
	END RHScalarOpt;

	PROCEDURE RHVector* (variable: BugsParser.Variable): GraphNodes.SubVector;
		VAR
			i, k, finish, numSlots, numVarIndex: INTEGER;
			res: SET;
			lower, upper: ARRAY 20 OF INTEGER;
			name: BugsNames.Name;
			node: GraphNodes.Node;
			argL: GraphStochastic.ArgsLogical;
			vector: GraphNodes.SubVector;
	BEGIN
		vector := GraphNodes.NewVector();
		name := variable.name;
		IF (name.components = NIL) & (name.values = NIL) THEN 
			Error(2, name.string); RETURN NIL 
		END;
		i := 0;
		vector.nElem := 1;
		vector.step := 1;
		numVarIndex := 0;
		numSlots := name.numSlots;
		WHILE i < numSlots DO
			IF variable.lower[i] = NIL THEN	(*	empty slot	*)
				lower[i] := 1;
				upper[i] := name.slotSizes[i];
				vector.nElem := vector.nElem * upper[i];
				vector.step := name.Step(i)
			ELSIF variable.upper[i] # NIL THEN	(*	range specified	*)
				lower[i] := Index(variable.lower[i]);
				IF lower[i] = error THEN RETURN NIL END;
				IF lower[i] > name.slotSizes[i] THEN Error(15, name.string); RETURN NIL END;
				upper[i] := Index(variable.upper[i]);
				IF upper[i] = error THEN RETURN NIL END;
				IF upper[i] > name.slotSizes[i] THEN Error(15, name.string); RETURN NIL END;
				IF upper[i] < lower[i] THEN Error(16, name.string); RETURN NIL END;
				vector.nElem := vector.nElem * (upper[i] - lower[i] + 1);
				IF upper[i] # lower[i] THEN vector.step := name.Step(i) END
			ELSIF variable.lower[i] IS BugsParser.Variable THEN
				node := RHScalar(variable.lower[i]);
				IF node = NIL THEN RETURN NIL END;
				IF GraphNodes.data IN node.props THEN
					lower[i] := Index(variable.lower[i]);
					IF lower[i] = error THEN RETURN NIL END;
					IF lower[i] > name.slotSizes[i] THEN Error(15, name.string); RETURN NIL END;
					upper[i] := lower[i]
				ELSE
					lower[i] := 1;
					upper[i] := 1;
					argL.scalars[numVarIndex] := node;
					k := 2 * numVarIndex;
					argL.ops[k] := name.slotSizes[i];
					argL.ops[k + 1] := name.Step(i);
					INC(numVarIndex)
				END
			ELSE
				lower[i] := Index(variable.lower[i]);
				IF lower[i] = error THEN RETURN NIL END;
				IF lower[i] > name.slotSizes[i] THEN Error(15, name.string); RETURN NIL END;
				upper[i] := lower[i]
			END;
			IF lower[i] < 1 THEN Error(17, name.string); RETURN NIL END;
			INC(i)
		END;
		IF numVarIndex = 0 THEN
			IF name.passByReference THEN
				vector.components := name.components
			ELSE
				vector.values := name.values
			END;
			vector.start := name.Offset(lower);
			finish := name.Offset(upper);
			IF (finish - vector.start + vector.step) DIV vector.step # vector.nElem THEN
				Error(16, name.string); RETURN NIL
			END
		ELSE
			NEW(vector.components, vector.nElem);
			vector.start := 0;
			argL.vectors[0] := GraphNodes.NewVector();
			argL.vectors[0].components := name.components;
			argL.vectors[0].start := name.Offset(lower);
			argL.numScalars := numVarIndex;
			i := 0;
			WHILE i < vector.nElem DO
				node := GraphMixture.fact.New();
				node.Set(argL, res);
				IF res # {} THEN RETURN NIL END;
				vector.components[i] := node;
				INC(argL.vectors[0].start, vector.step);
				INC(i)
			END
		END;
		RETURN vector
	END RHVector;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END BugsEvaluate.

