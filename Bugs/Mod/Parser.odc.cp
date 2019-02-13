(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsParser;


	

	IMPORT
		Stores, Strings,
		BugsIndex, BugsMappers, BugsMsg, BugsNames,
		GraphGrammar, GraphLogical, GraphNodes, GraphStochastic;

	CONST
		binary = 0; index = 1; variable = 2; function = 3; density = 4;
		internal = 5; real = 6; integer = 7;

	TYPE
		Node* = POINTER TO ABSTRACT RECORD
			pos-, label: INTEGER;
			value*: REAL;
			evaluated*: BOOLEAN
		END;

		Binary* = POINTER TO LIMITED RECORD(Node)
			op-: INTEGER;
			left-, right-: Node
		END;

		Index* = POINTER TO LIMITED RECORD(Node)
			name-: ARRAY 32 OF CHAR;
			intVal*: INTEGER;
			lower-, upper-: Node
		END;

		Variable* = POINTER TO LIMITED RECORD(Node)
			name-: BugsNames.Name;
			lower-, upper-: POINTER TO ARRAY OF Node
		END;

		Function* = POINTER TO LIMITED RECORD(Node)
			parents-: POINTER TO ARRAY OF Node;
			descriptor-: GraphGrammar.External
		END;

		Density* = POINTER TO LIMITED RECORD(Node)
			parents-: POINTER TO ARRAY OF Node;
			descriptor-: GraphGrammar.External;
			leftCen-, rightCen-, leftTrunc-, rightTrunc-: Node
		END;

		Internal* = POINTER TO LIMITED RECORD(Node)
			parents-: POINTER TO ARRAY OF Node;
			descriptor-: GraphGrammar.Internal
		END;

		Real* = POINTER TO LIMITED RECORD(Node)
			real-: REAL
		END;

		Integer* = POINTER TO LIMITED RECORD(Node)
			integer-: INTEGER
		END;

		Statement* = POINTER TO LIMITED RECORD
			variable-: Variable;
			expression-: Node;
			density-: Density;
			index-: Index;
			next-: Statement
		END;

		Visitor* = POINTER TO ABSTRACT RECORD END;

		List = POINTER TO RECORD
			node: Node;
			next: List
		END;

	VAR
		model-: Statement;
		error-: BOOLEAN;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		nodeLabel: INTEGER;
		nodes: POINTER TO ARRAY OF Node;
		nodeList: List;
		startOfGraph, numIfs: INTEGER;

	PROCEDURE (node: Node) Externalize- (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (node: Node) Internalize- (VAR rd: Stores.Reader), NEW, ABSTRACT;

	PROCEDURE ExternalizeNode (node: Node; VAR wr: Stores.Writer);
		VAR
			element: List;
	BEGIN
		IF node = NIL THEN
			wr.WriteInt(0)
		ELSIF node.label > 0 THEN
			wr.WriteInt(node.label)
		ELSE
			(*	first time seen node	*)
			NEW(element);
			element.node := node;
			element.next := nodeList;
			nodeList := element;
			INC(nodeLabel);
			node.label := nodeLabel;
			wr.WriteInt( - nodeLabel);
			WITH node: Binary DO wr.WriteInt(binary)
			|node: Index DO wr.WriteInt(index)
			|node: Variable DO wr.WriteInt(variable)
			|node: Function DO wr.WriteInt(function)
			|node: Density DO wr.WriteInt(density)
			|node: Internal DO wr.WriteInt(internal)
			|node: Real DO wr.WriteInt(real)
			|node: Integer DO wr.WriteInt(integer)
			END;
			node.Externalize(wr)
		END
	END ExternalizeNode;

	PROCEDURE InternalizeNode (OUT node: Node; VAR rd: Stores.Reader);
		VAR
			label, type: INTEGER;
			binaryNode: Binary;
			indexNode: Index;
			variableNode: Variable;
			functionNode: Function;
			densityNode: Density;
			internalNode: Internal;
			realNode: Real;
			integerNode: Integer;
	BEGIN
		rd.ReadInt(label);
		IF label = 0 THEN
			node := NIL
		ELSIF label > 0 THEN
			node := nodes[label]
		ELSE
			label := - label;
			rd.ReadInt(type);
			CASE type OF
			|binary: NEW(binaryNode); node := binaryNode
			|index: NEW(indexNode); node := indexNode
			|variable: NEW(variableNode); node := variableNode
			|function: NEW(functionNode); node := functionNode
			|density: NEW(densityNode); node := densityNode
			|internal: NEW(internalNode); node := internalNode
			|real: NEW(realNode); node := realNode
			|integer: NEW(integerNode); node := integerNode
			END;
			nodes[label] := node;
			node.label := 0;
			node.Internalize(rd)
		END
	END InternalizeNode;

	PROCEDURE ExternalizeBase (node: Node; VAR wr: Stores.Writer);
	BEGIN
		wr.WriteInt(node.pos);
		wr.WriteReal(node.value);
		wr.WriteBool(node.evaluated)
	END ExternalizeBase;

	PROCEDURE InternalizeBase (node: Node; VAR rd: Stores.Reader);
	BEGIN
		rd.ReadInt(node.pos);
		rd.ReadReal(node.value);
		rd.ReadBool(node.evaluated)
	END InternalizeBase;

	PROCEDURE (statement: Statement) CopyToList* (VAR list: Statement), NEW;
		VAR
			temp: Statement;
	BEGIN
		NEW(temp);
		temp.variable := statement.variable;
		temp.density := statement.density;
		temp.expression := statement.expression;
		temp.index := statement.index;
		temp.next := list;
		list := temp
	END CopyToList;

	PROCEDURE (visitor: Visitor) Do* (statement: Statement; OUT ok: BOOLEAN), NEW, ABSTRACT;

	PROCEDURE (list0: Statement) MergeLists* (VAR list1: Statement), NEW;
		VAR
			cursor: Statement;
	BEGIN
		cursor := list0;
		WHILE cursor # NIL DO
			cursor.CopyToList(list1);
			cursor := cursor.next
		END
	END MergeLists;

	PROCEDURE (statement: Statement) Accept* (visitor: Visitor; OUT ok: BOOLEAN), NEW;
	BEGIN
		WHILE statement # NIL DO
			visitor.Do(statement, ok);
			IF ~ok THEN RETURN END;
			WHILE (statement # NIL) & (statement.index # NIL) DO
				statement := statement.next
			END;
			statement := statement.next
		END
	END Accept;

	PROCEDURE Error (errorNum: INTEGER);
		VAR
			numToString: ARRAY 8 OF CHAR;
			errorMes: ARRAY 1024 OF CHAR;
	BEGIN
		Strings.IntToString(errorNum, numToString);
		BugsMsg.Lookup("BugsParser" + numToString, errorMes);
		BugsMsg.StoreError(errorMes);
		error := TRUE
	END Error;

	PROCEDURE NewBinary (pos, op: INTEGER): Binary;
		VAR
			binary: Binary;
	BEGIN
		NEW(binary);
		binary.label := 0;
		binary.pos := pos;
		binary.value := 0.0;
		binary.evaluated := FALSE;
		binary.op := op;
		binary.left := NIL;
		binary.right := NIL;
		RETURN binary
	END NewBinary;

	PROCEDURE (binary: Binary) Externalize- (VAR wr: Stores.Writer);
	BEGIN
		ExternalizeBase(binary, wr);
		wr.WriteInt(binary.op);
		ExternalizeNode(binary.left, wr);
		ExternalizeNode(binary.right, wr)
	END Externalize;

	PROCEDURE (binary: Binary) Internalize- (VAR rd: Stores.Reader);
	BEGIN
		InternalizeBase(binary, rd);
		rd.ReadInt(binary.op);
		InternalizeNode(binary.left, rd);
		InternalizeNode(binary.right, rd)
	END Internalize;

	PROCEDURE NewIndex (pos: INTEGER; IN name: ARRAY OF CHAR): Index;
		VAR
			index: Index;
	BEGIN
		NEW(index);
		index.label := 0;
		index.pos := pos;
		index.value := 0.0;
		index.evaluated := FALSE;
		index.name := name$;
		index.intVal := - 1;
		index.lower := NIL;
		index.upper := NIL;
		RETURN index
	END NewIndex;

	PROCEDURE (index: Index) Externalize- (VAR wr: Stores.Writer);
	BEGIN
		ExternalizeBase(index, wr);
		wr.WriteString(index.name);
		wr.WriteInt(index.intVal);
		ExternalizeNode(index.lower, wr);
		ExternalizeNode(index.upper, wr)
	END Externalize;

	PROCEDURE (index: Index) Internalize- (VAR rd: Stores.Reader);
	BEGIN
		InternalizeBase(index, rd);
		rd.ReadString(index.name);
		rd.ReadInt(index.intVal);
		InternalizeNode(index.lower, rd);
		InternalizeNode(index.upper, rd)
	END Internalize;

	PROCEDURE NewVariable (pos: INTEGER; name: BugsNames.Name): Variable;
		VAR
			i, numSlots: INTEGER;
			variable: Variable;
	BEGIN
		NEW(variable);
		variable.label := 0;
		variable.pos := pos;
		variable.value := 0.0;
		variable.evaluated := FALSE;
		variable.name := name;
		variable.lower := NIL;
		variable.upper := NIL;
		IF name.numSlots # 0 THEN
			numSlots := name.numSlots;
			NEW(variable.lower, numSlots);
			NEW(variable.upper, numSlots);
			i := 0;
			WHILE i < numSlots DO
				variable.lower[i] := NIL;
				variable.upper[i] := NIL;
				INC(i)
			END
		END;
		RETURN variable
	END NewVariable;

	PROCEDURE (variable: Variable) Externalize- (VAR wr: Stores.Writer);
		VAR
			i, len: INTEGER;
	BEGIN
		ExternalizeBase(variable, wr);
		wr.WriteString(variable.name.string);
		i := 0;
		len := variable.name.numSlots;
		WHILE i < len DO
			ExternalizeNode(variable.lower[i], wr);
			ExternalizeNode(variable.upper[i], wr);
			INC(i)
		END
	END Externalize;

	PROCEDURE (variable: Variable) Internalize- (VAR rd: Stores.Reader);
		VAR
			i, len: INTEGER;
			string: ARRAY 124 OF CHAR;
	BEGIN
		InternalizeBase(variable, rd);
		rd.ReadString(string);
		variable.name := BugsIndex.Find(string);
		i := 0;
		len := variable.name.numSlots;
		IF len > 0 THEN NEW(variable.lower, len); NEW(variable.upper, len) END;
		WHILE i < len DO
			InternalizeNode(variable.lower[i], rd);
			InternalizeNode(variable.upper[i], rd);
			INC(i)
		END
	END Internalize;

	PROCEDURE NewFunction (pos: INTEGER; descriptor: GraphGrammar.External): Function;
		VAR
			i, numPar: INTEGER;
			function: Function;
			fact: GraphNodes.Factory;
			signature: ARRAY 64 OF CHAR;
	BEGIN
		NEW(function);
		function.label := 0;
		function.pos := pos;
		function.value := 0.0;
		function.evaluated := FALSE;
		function.descriptor := descriptor;
		fact := descriptor.fact;
		numPar := fact.NumParam();
		fact.Signature(signature);
		i := 0;
		WHILE i < numPar DO
			IF signature[i] = "D" THEN	(*	differential equation	*)
				INC(numPar, 2)
			ELSIF signature[i] = "F" THEN	(*	functional	*)
				INC(numPar)
			END;
			INC(i)
		END;
		IF numPar > 0 THEN
			NEW(function.parents, numPar);
			i := 0;
			WHILE i < numPar DO
				function.parents[i] := NIL;
				INC(i)
			END
		ELSE
			function.parents := NIL
		END;
		RETURN function
	END NewFunction;

	PROCEDURE (function: Function) Externalize- (VAR wr: Stores.Writer);
		VAR
			i, len: INTEGER;
	BEGIN
		ExternalizeBase(function, wr);
		wr.WriteString(function.descriptor.name);
		i := 0;
		IF function.parents # NIL THEN len := LEN(function.parents) ELSE len := 0 END;
		wr.WriteInt(len);
		WHILE i < len DO
			ExternalizeNode(function.parents[i], wr);
			INC(i)
		END
	END Externalize;

	PROCEDURE (function: Function) Internalize- (VAR rd: Stores.Reader);
		VAR
			i, len: INTEGER;
			string: ARRAY 124 OF CHAR;
	BEGIN
		InternalizeBase(function, rd);
		rd.ReadString(string);
		function.descriptor := GraphGrammar.FindFunction(string);
		i := 0;
		rd.ReadInt(len);
		IF len > 0 THEN NEW(function.parents, len) END;
		WHILE i < len DO
			InternalizeNode(function.parents[i], rd);
			INC(i)
		END
	END Internalize;

	PROCEDURE NewDensity (pos: INTEGER; descriptor: GraphGrammar.External): Density;
		VAR
			i, numPar: INTEGER;
			density: Density;
			fact: GraphNodes.Factory;
			string: ARRAY 124 OF CHAR;
			signature: ARRAY 64 OF CHAR;
	BEGIN
		NEW(density);
		density.label := 0;
		density.pos := pos;
		density.value := 0.0;
		density.evaluated := FALSE;
		density.parents := NIL;
		density.descriptor := descriptor;
		fact := descriptor.fact;
		fact.Signature(signature);
		numPar := fact.NumParam();
		i := 0;
		WHILE i < numPar DO
			IF signature[i] = "F" THEN	(*	functional	*)
				INC(numPar) 
			END;
			INC(i)
		END;
		IF numPar > 0 THEN
			NEW(density.parents, numPar);
			i := 0;
			WHILE i < numPar DO
				density.parents[i] := NIL;
				INC(i)
			END
		ELSE
			density.parents := NIL
		END;
		density.leftCen := NIL;
		density.rightCen := NIL;
		density.leftTrunc := NIL;
		density.rightTrunc := NIL;
		RETURN density
	END NewDensity;

	PROCEDURE (density: Density) Externalize- (VAR wr: Stores.Writer);
		VAR
			i, len: INTEGER;
	BEGIN
		ExternalizeBase(density, wr);
		wr.WriteString(density.descriptor.name);
		i := 0;
		IF density.parents # NIL THEN len := LEN(density.parents ) ELSE len := 0 END;
		wr.WriteInt(len);
		WHILE i < len DO
			ExternalizeNode(density.parents[i], wr);
			INC(i)
		END;
		ExternalizeNode(density.leftCen, wr);
		ExternalizeNode(density.rightCen, wr);
		ExternalizeNode(density.leftTrunc, wr);
		ExternalizeNode(density.rightTrunc, wr)
	END Externalize;

	PROCEDURE (density: Density) Internalize- (VAR rd: Stores.Reader);
		VAR
			i, len: INTEGER;
			string: ARRAY 124 OF CHAR;
	BEGIN
		InternalizeBase(density, rd);
		rd.ReadString(string);
		density.descriptor := GraphGrammar.FindDensity(string);
		i := 0;
		rd.ReadInt(len);
		IF len > 0 THEN NEW(density.parents, len) END;
		WHILE i < len DO
			InternalizeNode(density.parents[i], rd);
			INC(i)
		END;
		InternalizeNode(density.leftCen, rd);
		InternalizeNode(density.rightCen, rd);
		InternalizeNode(density.leftTrunc, rd);
		InternalizeNode(density.rightTrunc, rd)
	END Internalize;

	PROCEDURE NewInternal (pos: INTEGER; descriptor: GraphGrammar.Internal): Internal;
		VAR
			internal: Internal;
	BEGIN
		NEW(internal);
		internal.label := 0;
		internal.pos := pos;
		internal.value := 0.0;
		internal.evaluated := FALSE;
		internal.descriptor := descriptor;
		NEW(internal.parents, descriptor.numPar);
		RETURN internal
	END NewInternal;

	PROCEDURE (internal: Internal) Externalize- (VAR wr: Stores.Writer);
		VAR
			i, len: INTEGER;
	BEGIN
		ExternalizeBase(internal, wr);
		wr.WriteString(internal.descriptor.name);
		i := 0;
		len := internal.descriptor.numPar;
		WHILE i < len DO
			ExternalizeNode(internal.parents[i], wr);
			INC(i)
		END
	END Externalize;

	PROCEDURE (internal: Internal) Internalize- (VAR rd: Stores.Reader);
		VAR
			i, len: INTEGER;
			string: ARRAY 124 OF CHAR;
	BEGIN
		InternalizeBase(internal, rd);
		rd.ReadString(string);
		internal.descriptor := GraphGrammar.FindInternal(string);
		i := 0;
		len := internal.descriptor.numPar;
		IF len > 0 THEN NEW(internal.parents, len) END;
		WHILE i < len DO
			InternalizeNode(internal.parents[i], rd);
			INC(i)
		END
	END Internalize;

	PROCEDURE NewReal (pos: INTEGER; value: REAL): Real;
		VAR
			real: Real;
	BEGIN
		NEW(real);
		real.label := 0;
		real.pos := pos;
		real.value := 0.0;
		real.evaluated := FALSE;
		real.real := value;
		RETURN real
	END NewReal;

	PROCEDURE (real: Real) Externalize- (VAR wr: Stores.Writer);
	BEGIN
		ExternalizeBase(real, wr);
		wr.WriteReal(real.real)
	END Externalize;

	PROCEDURE (real: Real) Internalize- (VAR rd: Stores.Reader);
	BEGIN
		InternalizeBase(real, rd);
		rd.ReadReal(real.real)
	END Internalize;

	PROCEDURE NewInteger (pos: INTEGER; value: INTEGER): Integer;
		VAR
			integer: Integer;
	BEGIN
		NEW(integer);
		integer.label := 0;
		integer.pos := pos;
		integer.value := 0.0;
		integer.evaluated := FALSE;
		integer.integer := value;
		RETURN integer
	END NewInteger;

	PROCEDURE (integer: Integer) Externalize- (VAR wr: Stores.Writer);
	BEGIN
		ExternalizeBase(integer, wr);
		wr.WriteInt(integer.integer)
	END Externalize;

	PROCEDURE (integer: Integer) Internalize- (VAR rd: Stores.Reader);
	BEGIN
		InternalizeBase(integer, rd);
		rd.ReadInt(integer.integer)
	END Internalize;

	PROCEDURE ^Param (loops: Statement; VAR s: BugsMappers.Scanner; integer: BOOLEAN): Node;

	PROCEDURE ^ParseVariable* (loops: Statement; VAR s: BugsMappers.Scanner): Variable;

	PROCEDURE ^Expression (loops: Statement; VAR s: BugsMappers.Scanner; integer: BOOLEAN): Node;

	PROCEDURE ^ParseTerm (loops: Statement; VAR s: BugsMappers.Scanner; integer: BOOLEAN): Node;

	PROCEDURE ^ParseFactor (loops: Statement;
	VAR s: BugsMappers.Scanner; integer: BOOLEAN): Node;

	PROCEDURE ParseDnotation (loops: Statement;
	VAR s: BugsMappers.Scanner; OUT derivative, dependent, independent: Variable);
		VAR
			i, numSlots: INTEGER;
			derivName: ARRAY 120 OF CHAR;
	BEGIN
		s.Scan;
		IF (s.type # BugsMappers.char) OR (s.char # "(") THEN
			Error(3);
			independent := NIL;
			RETURN
		END;
		s.Scan;
		dependent := ParseVariable(loops, s);
		IF dependent = NIL THEN
			(*	error handling ???	*)
			independent := NIL;
			RETURN
		END;
		s.Scan;
		IF (s.type # BugsMappers.char) OR (s.char # ",") THEN
			Error(5);
			independent := NIL;
			RETURN
		END;
		s.Scan;
		independent := ParseVariable(loops, s);
		IF independent = NIL THEN
			(*	error handling ???	*)
			RETURN
		END;
		s.Scan;
		IF (s.type # BugsMappers.char) OR (s.char # ")") THEN
			Error(6);
			independent := NIL;
			RETURN
		END;
		derivName := dependent.name.string$;
		derivName := "D(" + derivName + "," + independent.name.string + ")";
		NEW(derivative);
		derivative.pos := s.Pos();
		derivative.value := 0.0;
		derivative.evaluated := FALSE;
		derivative.name := BugsIndex.Find(derivName);
		numSlots := dependent.name.numSlots;
		IF derivative.name = NIL THEN
			derivative.name := BugsNames.New(derivName, numSlots);
			BugsIndex.Store(derivative.name)
		END;
		NEW(derivative.lower, numSlots);
		NEW(derivative.upper, numSlots);
		i := 0;
		WHILE i < numSlots DO
			derivative.lower[i] := dependent.lower[i];
			derivative.upper[i] := dependent.upper[i]; ;
			INC(i)
		END;
		s.Scan
	END ParseDnotation;

	PROCEDURE ParseFnotation (loops: Statement; VAR s: BugsMappers.Scanner;
	OUT functionVar, independent: Variable);
		VAR
			numSlots: INTEGER;
			functionName: ARRAY 120 OF CHAR;
	BEGIN
		s.Scan;
		IF (s.type # BugsMappers.char) OR (s.char # "(") THEN
			Error(3);
			independent := NIL;
			RETURN
		END;
		s.Scan;
		independent := ParseVariable(loops, s);
		IF independent = NIL THEN
			RETURN
		END;
		numSlots := independent.name.numSlots;
		s.Scan;
		IF (s.type # BugsMappers.char) OR (s.char # ")") THEN
			Error(6);
			independent := NIL;
			RETURN
		END;
		functionName := independent.name.string$;
		functionName := "F(" + functionName + ")";
		NEW(functionVar);
		functionVar.pos := s.Pos();
		functionVar.value := 0.0;
		functionVar.evaluated := FALSE;
		functionVar.name := BugsIndex.Find(functionName);
		functionVar.lower := independent.lower;
		functionVar.upper := independent.upper;
		IF functionVar.name = NIL THEN
			functionVar.name := BugsNames.New(functionName, numSlots);
			BugsIndex.Store(functionVar.name)
		END;
		s.Scan
	END ParseFnotation;

	PROCEDURE ParseFunction (loops: Statement; VAR s: BugsMappers.Scanner): Node;
		VAR
			i, j, numPar: INTEGER;
			function: Function;
			internal: Internal;
			dependent, independent, derivative, functionVar: Variable;
			opDesc: GraphGrammar.Internal;
			funcDesc: GraphGrammar.External;
			fact: GraphNodes.Factory;
			signature: ARRAY 64 OF CHAR;
	BEGIN
		opDesc := GraphGrammar.FindInternal(s.string);
		IF opDesc = NIL THEN
			funcDesc := GraphGrammar.FindFunction(s.string);
			IF funcDesc = NIL THEN (*	unknown type of logical function	*)
				Error(1); RETURN NIL
			END;
			fact := funcDesc.fact;
			IF fact = NIL THEN
				(*	unable to load logical function	*)
				Error(1); RETURN NIL
			END;
			fact.Signature(signature);
			numPar := fact.NumParam();
			IF signature[numPar] = "L" THEN (*	link function can not be used on right hand side	*)
				Error(2); RETURN NIL
			END;
			function := NewFunction(s.Pos(), funcDesc);
			s.Scan;
			IF (s.type # BugsMappers.char) OR (s.char # "(") THEN (*	expected left parenthesis	*)
				Error(3); RETURN NIL
			END;
			s.Scan;
			i := 0;
			j := 0;
			WHILE i < numPar DO
				CASE signature[i] OF
				|"s":
					function.parents[j] := Param(loops, s, FALSE);
					IF function.parents[j] = NIL THEN RETURN NIL END;
					s.Scan
				|"v":
					function.parents[j] := ParseVariable(loops, s);
					IF function.parents[j] = NIL THEN RETURN NIL END;
					s.Scan
				|"e":
					function.parents[j] := Expression(loops, s, FALSE);
					IF function.parents[j] = NIL THEN RETURN NIL END;
				|"D":
					ParseDnotation(loops, s, derivative, dependent, independent);
					IF derivative = NIL THEN RETURN NIL END;
					function.parents[j] := derivative;
					function.parents[j + 1] := dependent;
					function.parents[j + 2] := independent;
					INC(j, 2)
				|"F":
					ParseFnotation(loops, s, functionVar, independent);
					IF functionVar = NIL THEN RETURN NIL END;
					function.parents[j] := functionVar;
					function.parents[j + 1] := independent;
					INC(j)
				ELSE	(*	unknown type of argument for logical  function	*)
					Error(4); RETURN NIL
				END;
				function.parents[j].pos := s.Pos();
				INC(i);
				INC(j);
				IF i # numPar THEN
					IF (s.type # BugsMappers.char) OR (s.char # ",") THEN (*	expected comma	*)
						Error(5); RETURN NIL
					END;
					s.Scan
				END
			END;
			IF (s.type # BugsMappers.char) OR (s.char # ")") THEN (*	expected right parenthesis	*)
				Error(6); RETURN NIL
			END;
			RETURN function
		ELSE
			internal := NewInternal(s.Pos(), opDesc);
			numPar := opDesc.numPar;
			s.Scan;
			IF (s.type # BugsMappers.char) OR (s.char # "(") THEN (*	expected left parenthesis	*)
				Error(7); RETURN NIL
			END;
			s.Scan;
			i := 0;
			WHILE i < numPar DO
				internal.parents[i] := Expression(loops, s, FALSE);
				IF internal.parents[i] = NIL THEN RETURN NIL END;
				internal.parents[i].pos := s.Pos();
				INC(i);
				IF i # numPar THEN
					IF (s.type # BugsMappers.char) OR (s.char # ",") THEN
						Error(8); (*	expected comma	*)
						RETURN NIL
					ELSE
						s.Scan
					END
				END
			END;
			IF (s.type # BugsMappers.char) OR (s.char # ")") THEN
				Error(9); (*	expected right parenthesis	*)
				RETURN NIL
			END;
			RETURN internal
		END
	END ParseFunction;

	PROCEDURE ParseTerm (loops: Statement; VAR s: BugsMappers.Scanner; integer: BOOLEAN): Node;
		VAR
			op: INTEGER;
			binary: Binary;
			node: Node;
	BEGIN
		node := ParseFactor(loops, s, integer);
		IF node = NIL THEN RETURN NIL END;
		s.Scan;
		WHILE (s.type = BugsMappers.char) & ((s.char = "*") OR (s.char = "/")) DO
			IF s.char = "*" THEN
				op := GraphGrammar.mult
			ELSE
				op := GraphGrammar.div
			END;
			binary := NewBinary(s.Pos(), op);
			s.Scan;
			binary.left := node;
			binary.right := ParseFactor(loops, s, integer);
			IF binary.right = NIL THEN RETURN NIL END;
			node := binary;
			s.Scan
		END;
		RETURN node
	END ParseTerm;

	PROCEDURE Expression (loops: Statement; VAR s: BugsMappers.Scanner; integer: BOOLEAN): Node;
		VAR
			op: INTEGER;
			binary: Binary;
			node: Node;
	BEGIN
		node := ParseTerm(loops, s, integer);
		IF node = NIL THEN RETURN NIL END;
		WHILE (s.type = BugsMappers.char) & ((s.char = "+") OR (s.char = "-")) DO
			IF s.char = "+" THEN
				op := GraphGrammar.add
			ELSE
				op := GraphGrammar.sub
			END;
			binary := NewBinary(s.Pos(), op);
			binary.left := node;
			s.Scan;
			binary.right := ParseTerm(loops, s, integer);
			IF binary.right = NIL THEN RETURN NIL END;
			node := binary
		END;
		RETURN node
	END Expression;

	PROCEDURE ParseFactor (loops: Statement; VAR s: BugsMappers.Scanner; integer: BOOLEAN): Node;
		VAR
			pos: INTEGER;
			binary: Binary;
			node: Node;
			internal: GraphGrammar.Internal;
	BEGIN
		IF (s.type = BugsMappers.char) & (s.char = "+") THEN (*	unexpected token in factor	*)
			Error(10); RETURN NIL
		END;
		IF s.type = BugsMappers.function THEN
			IF integer THEN (*	logical function not allowed in integer expression	*)
				internal := GraphGrammar.FindInternal(s.string); 
				IF internal # NIL THEN
					CASE internal.key OF
					|GraphGrammar.equals, GraphGrammar.max, GraphGrammar.min, GraphGrammar.step:
					ELSE
						Error(11); RETURN NIL
					END
				END
			END;
			node := ParseFunction(loops, s);
			IF node = NIL THEN RETURN NIL END
		ELSIF (s.type = BugsMappers.char) & (s.char = "-") THEN
			binary := NewBinary(s.Pos(), GraphGrammar.uminus);
			s.Scan;
			pos := s.Pos();
			binary.left := ParseFactor(loops, s, integer);
			IF binary.left = NIL THEN RETURN NIL END;
			binary.left.pos := pos;
			node := binary
		ELSIF (s.type = BugsMappers.char) & (s.char = "(") THEN
			s.Scan; pos := s.Pos();
			node := Expression(loops, s, integer);
			IF node = NIL THEN RETURN NIL END;
			IF (s.type # BugsMappers.char) OR (s.char # ")") THEN
				Error(12); (*	expected right parenthesis	*)
				RETURN NIL
			END
		ELSE
			node := Param(loops, s, integer)
		END;
		RETURN node
	END ParseFactor;

	PROCEDURE ParseVariable* (loops: Statement; VAR s: BugsMappers.Scanner): Variable;
		VAR
			i, numSlots: INTEGER;
			variable: Variable;
			name: BugsNames.Name;
	BEGIN
		IF s.type # BugsMappers.string THEN (*	expected variable name	*)
			Error(13); RETURN NIL
		END;
		name := BugsIndex.Find(s.string);
		IF name = NIL THEN (*	expected variable name	*)
			Error(13); RETURN NIL
		END;
		variable := NewVariable(s.Pos(), name);
		numSlots := name.numSlots;
		IF numSlots > 0 THEN
			s.Scan;
			IF (s.type # BugsMappers.char) OR (s.char # "[") THEN
				RETURN variable
			END
		END;
		i := 0;
		WHILE i < numSlots DO
			s.Scan;
			IF (s.type # BugsMappers.char) OR ((s.char # ",") & (s.char # "]")) THEN
				variable.lower[i] := Expression(loops, s, TRUE);
				IF variable.lower[i] = NIL THEN RETURN NIL END;
				IF (s.type = BugsMappers.char) & (s.char = ":") THEN
					s.Scan;
					variable.upper[i] := Expression(loops, s, TRUE);
					IF variable.upper[i] = NIL THEN RETURN NIL END
				END
			END;
			INC(i);
			IF(i # numSlots) & ((s.type # BugsMappers.char) OR (s.char # ",")) THEN
				(*	expected comma	*)
				Error(14); RETURN NIL
			END
		END;
		IF (numSlots > 0) & ((s.type # BugsMappers.char) OR (s.char # "]")) THEN
			(* right square braket	*)
			Error(15); RETURN NIL
		END;
		RETURN variable
	END ParseVariable;

	PROCEDURE Param (loops: Statement; VAR s: BugsMappers.Scanner; integer: BOOLEAN): Node;
		VAR
			signed: BOOLEAN;
			intVal: INTEGER;
			realVal: REAL;
			node: Node;
			cursor: Statement;
	BEGIN
		IF s.type = BugsMappers.string THEN
			cursor := loops;
			WHILE (cursor # NIL) & (cursor.index.name # s.string) DO
				cursor := cursor.next
			END;
			IF cursor # NIL THEN
				node := cursor.index
			ELSE
				node := ParseVariable(loops, s);
				IF node = NIL THEN RETURN NIL END
			END
		ELSIF (s.type = BugsMappers.real) OR (s.type = BugsMappers.int) OR
			((s.type = BugsMappers.char) & (s.char = "-")) THEN
			signed := s.type = BugsMappers.char;
			IF signed THEN
				s.Scan
			END;
			IF s.type = BugsMappers.int THEN
				IF signed THEN
					intVal := - s.int
				ELSE
					intVal := s.int
				END;
				node := NewInteger(s.Pos(), intVal)
			ELSIF s.type = BugsMappers.real THEN
				IF integer THEN
					Error(16); (*	expected an integer	*)
					RETURN NIL
				END;
				IF signed THEN
					realVal := - s.real
				ELSE
					realVal := s.real
				END;
				node := NewReal(s.Pos(), realVal)
			ELSE
				Error(17); (*	expected a number	*)
				RETURN NIL
			END
		ELSE
			Error(18); (*	invalid or unexpected token scanned	*)
			RETURN NIL
		END;
		RETURN node
	END Param;

	PROCEDURE ParseParameter* (loops: Statement; VAR s: BugsMappers.Scanner): Node;
	BEGIN
		RETURN Param(loops, s, FALSE)
	END ParseParameter;

	PROCEDURE ParseExpression* (loops: Statement; VAR s: BugsMappers.Scanner): Node;
	BEGIN
		RETURN Expression(loops, s, FALSE)
	END ParseExpression;

	PROCEDURE ParseLimit* (loops: Statement; VAR s: BugsMappers.Scanner): Node;
	BEGIN
		RETURN Expression(loops, s, TRUE)
	END ParseLimit;

	PROCEDURE ParseCen* (loops: Statement; VAR s: BugsMappers.Scanner;
	VAR density: Density);
		VAR
			fact: GraphNodes.Factory;
			signature: ARRAY 64 OF CHAR;
			len, numPar: INTEGER;
	BEGIN
		fact := density.descriptor.fact;
		fact.Signature(signature);
		numPar := fact.NumParam();
		len := LEN(signature$);
		IF (s.type = BugsMappers.function) & ((s.string = "I") OR (s.string = "C")) THEN
			IF (len = 0) OR (signature[numPar] # "C") & (signature[len - 1] # "C") THEN
				Error(25); density := NIL; RETURN	(*	can not be censored	*)
			END;
			IF (density.leftCen # NIL) OR (density.rightCen # NIL) THEN
				(*	this density already censored	*)
				Error(47); density := NIL; RETURN
			END;
			s.Scan;
			IF (s.type # BugsMappers.char) OR (s.char # "(") THEN (*	expected left parenthesis	*)
				Error(26); density := NIL; RETURN
			END;
			s.Scan;
			IF (s.type = BugsMappers.char) & (s.char = ",") THEN s.Scan
			ELSE
				density.leftCen := Param(loops, s, FALSE);
				IF density.leftCen = NIL THEN density := NIL; RETURN END;
				s.Scan;
				IF (s.type # BugsMappers.char) OR (s.char # ",") THEN (*	expected comma	*)
					Error(27); density := NIL; RETURN
				END;
				s.Scan
			END;
			IF (s.type # BugsMappers.char) OR (s.char # ")") THEN
				density.rightCen := Param(loops, s, FALSE);
				IF density.rightCen = NIL THEN density := NIL; RETURN END;
				s.Scan;
				IF (s.type # BugsMappers.char) OR (s.char # ")") THEN (*	expected right parenthesis	*)
					Error(28); density := NIL; RETURN
				END
			END;
			s.Scan
		END
	END ParseCen;

	PROCEDURE ParseTrunc* (loops: Statement; VAR s: BugsMappers.Scanner;
	VAR density: Density);
		VAR
			fact: GraphNodes.Factory;
			signature: ARRAY 64 OF CHAR;
			len, numPar: INTEGER;
	BEGIN
		fact := density.descriptor.fact;
		fact.Signature(signature);
		numPar := fact.NumParam();
		len := LEN(signature$);
		IF (s.type = BugsMappers.function) & (s.string = "T") THEN
			IF (len = 0) OR (signature[numPar] # "T") & (signature[len - 1] # "T") THEN
				Error(46); density := NIL; RETURN (*	can not be truncated	*)
			END;
			IF (density.leftTrunc # NIL) OR (density.rightTrunc # NIL) THEN (*	density alreay truncated	*)
				Error(48); density := NIL; RETURN
			END;
			s.Scan;
			IF (s.type # BugsMappers.char) OR (s.char # "(") THEN (*	expected left parenthesis	*)
				Error(26); density := NIL; RETURN
			END;
			s.Scan;
			IF (s.type = BugsMappers.char) & (s.char = ",") THEN s.Scan
			ELSE
				density.leftTrunc := Param(loops, s, FALSE);
				IF density.leftTrunc = NIL THEN density := NIL; RETURN END;
				s.Scan;
				IF (s.type # BugsMappers.char) OR (s.char # ",") THEN (*	expected comma	*)
					Error(27); density := NIL; RETURN
				END;
				s.Scan
			END;
			IF (s.type # BugsMappers.char) OR (s.char # ")") THEN
				density.rightTrunc := Param(loops, s, FALSE);
				IF density.rightTrunc = NIL THEN density := NIL; RETURN END;
				s.Scan;
				IF (s.type # BugsMappers.char) OR (s.char # ")") THEN (*	expected right parenthesis	*)
					Error(28); density := NIL; RETURN
				END
			END;
			s.Scan
		END;
	END ParseTrunc;

	PROCEDURE ParseDensity* (loops: Statement; VAR s: BugsMappers.Scanner): Density;
		VAR
			i, j, numPar: INTEGER;
			descriptor: GraphGrammar.External;
			density: Density;
			fact: GraphNodes.Factory;
			signature: ARRAY 64 OF CHAR;
			independent, functionVar: Variable;
	BEGIN
		descriptor := GraphGrammar.FindDensity(s.string);
		IF descriptor = NIL THEN (*	unknown type of probability density	*)
			Error(19); RETURN NIL
		END;
		fact := descriptor.fact;
		IF (fact = NIL) OR ~(fact IS GraphStochastic.Factory) THEN
			(*	unknown type of probability density	*)
			Error(19); RETURN NIL
		END;
		density := NewDensity(s.Pos(), descriptor);
		s.Scan;
		IF (s.type # BugsMappers.char) OR (s.char # "(") THEN (*	expected left parenthesis	*)
			Error(20); RETURN NIL
		END;
		fact.Signature(signature);
		numPar := fact.NumParam();
		s.Scan;
		i := 0;
		j := 0;
		WHILE i < numPar DO
			CASE signature[i] OF
			|"s":
				density.parents[j] := Param(loops, s, FALSE);
				IF density.parents[j] = NIL THEN RETURN NIL END;
				s.Scan
			|"v":
				IF s.type # BugsMappers.string THEN (*	expected variable name	*)
					Error(21); RETURN NIL
				END;
				density.parents[j] := ParseVariable(loops, s);
				IF density.parents[j] = NIL THEN RETURN NIL END;
				s.Scan
			|"F":
				ParseFnotation(loops, s, functionVar, independent);
				IF functionVar = NIL THEN RETURN NIL END;
				density.parents[j] := functionVar;
				density.parents[j + 1] := independent;
				INC(j)
			ELSE
				Error(22); (*	unknown type of argument for probability density	*)
				RETURN NIL
			END;
			density.parents[i].pos := s.Pos();
			INC(i);
			INC(j);
			IF i # numPar THEN
				IF (s.type # BugsMappers.char) OR (s.char # ",") THEN (*	expected comma	*)
					Error(23); RETURN NIL
				ELSE
					s.Scan
				END
			END
		END;
		IF (s.type # BugsMappers.char) OR (s.char # ")") THEN (*	expected right parenthesis	*)
			Error(24); RETURN NIL
		END;
		s.Scan;
		ParseCen(loops, s, density);
		IF density = NIL THEN RETURN NIL END;
		ParseTrunc(loops, s, density);
		IF density = NIL THEN RETURN NIL END;
		ParseCen(loops, s, density);
		IF density = NIL THEN RETURN NIL END;
		ParseTrunc(loops, s, density);
		RETURN density
	END ParseDensity;

	PROCEDURE BuildFunction* (parents: POINTER TO ARRAY OF Node; descriptor: GraphGrammar.External): Function;
		VAR
			i, numPar: INTEGER;
			function: Function;
	BEGIN
		function := NewFunction(0, descriptor);
		IF parents # NIL THEN
			numPar := LEN(parents)
		ELSE
			numPar := 0
		END;
		i := 0;
		WHILE i < numPar DO
			function.parents[i] := parents[i];
			INC(i)
		END;
		RETURN function
	END BuildFunction;

	PROCEDURE BuildDensity* (IN parents: ARRAY OF Node; descriptor: GraphGrammar.External): Density;
		VAR
			i, numPar: INTEGER;
			density: Density;
			fact: GraphNodes.Factory;
	BEGIN
		density := NewDensity(0, descriptor);
		fact := descriptor.fact;
		numPar := fact.NumParam(); ;
		i := 0;
		WHILE i < numPar DO
			ASSERT(parents[i] # NIL, 33);
			density.parents[i] := parents[i];
			INC(i)
		END;
		IF LEN(parents) > numPar + 1 THEN
			IF parents[numPar] # NIL THEN
				density.leftCen := parents[numPar]
			END;
			IF parents[numPar + 1] # NIL THEN
				density.rightCen := parents[numPar + 1]
			END
		END;
		IF LEN(parents) > numPar + 3 THEN
			IF parents[numPar + 2] # NIL THEN
				density.leftTrunc := parents[numPar + 2]
			END;
			IF parents[numPar + 3] # NIL THEN
				density.rightTrunc := parents[numPar + 3]
			END
		END;
		RETURN density
	END BuildDensity;

	PROCEDURE BuildIndex* (index: Index; lower, upper: Node);
	BEGIN
		index.lower := lower;
		index.upper := upper;
	END BuildIndex;

	PROCEDURE BuildForLoop* (index: Index): Statement;
		VAR
			statement: Statement;
	BEGIN
		NEW(statement);
		statement.variable := NIL;
		statement.expression := NIL;
		statement.density := NIL;
		statement.index := index;
		RETURN statement
	END BuildForLoop;

	PROCEDURE BuildLogical* (variable: Variable; expression: Node): Statement;
		VAR
			statement: Statement;
			descriptor: GraphGrammar.External;
			descIntern: GraphGrammar.Internal;
			parents: POINTER TO ARRAY OF Node;
			name: ARRAY 64 OF CHAR;
	BEGIN
		NEW(statement);
		statement.variable := variable;
		statement.expression := expression;
		statement.density := NIL;
		statement.index := NIL;
		statement.next := NIL;
		WITH expression: Function DO
			descriptor := GraphGrammar.FindFunction(expression.descriptor.name);
			IF descriptor # NIL THEN
				statement.expression := BuildFunction(expression.parents, descriptor)
			END
		|expression: Internal DO
			descIntern := expression.descriptor;
			IF descIntern.numPar = 1 THEN
				NEW(parents, 1);
				parents[0] := expression.parents[0];
				IF descIntern.name = "exp" THEN
					name := "log"
				ELSIF descIntern.name = "ilogit" THEN
					name := "logit"
				ELSIF descIntern.name = "phi" THEN
					name := "probit"
				ELSIF descIntern.name = "icloglog" THEN
					name := "cloglog"
				ELSE
					name := ""
				END;
				descriptor := GraphGrammar.FindFunction(name);
				IF descriptor # NIL THEN
					statement.expression := BuildFunction(parents, descriptor);
					statement.expression.pos := - 1
				END
			END
		ELSE
		END;
		RETURN statement
	END BuildLogical;

	PROCEDURE BuildStochastic* (variable: Variable; density: Density): Statement;
		VAR
			statement: Statement;
	BEGIN
		NEW(statement);
		statement.variable := variable;
		statement.expression := NIL;
		statement.density := density;
		statement.index := NIL;
		statement.next := NIL;
		RETURN statement
	END BuildStochastic;

	PROCEDURE ParseIndexName* (loops: Statement; VAR s: BugsMappers.Scanner): Index;
		VAR
			for: Statement;
			index: Index;
	BEGIN
		IF s.type # BugsMappers.string THEN (*	loop index must be a name	*)
			Error(29); RETURN NIL
		END;
		IF BugsIndex.Find(s.string) # NIL THEN (*	loop index can not be a variable name	*)
			Error(30); RETURN NIL
		END;
		for := loops;
		WHILE (for # NIL) & (for.index.name # s.string) DO for := for.next END;
		IF for # NIL THEN (*	already used as loop index	*)
			Error(31); RETURN NIL
		END;
		index := NewIndex(s.Pos(), s.string);
		RETURN index
	END ParseIndexName;

	PROCEDURE ParseForLoop* (loops: Statement; VAR s: BugsMappers.Scanner): Statement;
		VAR
			lower, upper: Node;
			index: Index;
			statement: Statement;
	BEGIN
		IF (s.type # BugsMappers.char) OR (s.char # "(") THEN (*	expected left parenthesis	*)
			Error(32); RETURN NIL
		END;
		s.Scan;
		index := ParseIndexName(loops, s);
		IF index = NIL THEN RETURN NIL END;
		s.Scan;
		IF (s.type # BugsMappers.string) OR (s.string # "in") THEN (*	expected in	*)
			Error(33); RETURN NIL
		END;
		s.Scan;
		lower := ParseLimit(loops, s);
		IF lower = NIL THEN RETURN NIL END;
		IF (s.type # BugsMappers.char) OR (s.char # ":") THEN (*	expected colon	*)
			Error(34); RETURN NIL
		END;
		index.lower := lower;
		s.Scan;
		upper := ParseLimit(loops, s);
		IF upper = NIL THEN RETURN NIL END;
		IF (s.type # BugsMappers.char) OR (s.char # ")") THEN (*	expected right parenthesis	*)
			Error(35); RETURN NIL
		END;
		s.Scan;
		IF (s.type # BugsMappers.char) OR (s.char # "{") THEN (*	expected open brace	*)
			Error(36); RETURN NIL
		END;
		index.upper := upper;
		s.Scan;
		NEW(statement);
		statement.variable := NIL;
		statement.expression := NIL;
		statement.density := NIL;
		statement.index := index;
		RETURN statement
	END ParseForLoop;

	PROCEDURE ParseIf* (loops: Statement; VAR s: BugsMappers.Scanner): Statement;
		VAR
			condition: Node;
			if: Index;
			statement: Statement;
			one: Integer;
			min: Internal;
			descriptor: GraphGrammar.Internal;
			ifName: ARRAY 32 OF CHAR;
	BEGIN
		IF (s.type # BugsMappers.char) OR (s.char # "(") THEN (*	expected left parenthesis	*)
			Error(32); RETURN NIL
		END;
		s.Scan;
		condition := ParseLimit(loops, s);
		IF condition = NIL THEN RETURN NIL END;
		IF (s.type # BugsMappers.char) OR (s.char # ")") THEN (*	expected right parenthesis	*)
			Error(35); RETURN NIL
		END;
		s.Scan;
		IF (s.type # BugsMappers.char) OR (s.char # "{") THEN (*	expected open brace	*)
			Error(36); RETURN NIL
		END;
		Strings.IntToString(numIfs, ifName);
		ifName := "$" + ifName;	(*	$ prevents clash with any for loop variable name	*)
		INC(numIfs);
		if := NewIndex(0, ifName);
		one := NewInteger(0, 1);
		if.lower := one;
		one := NewInteger(0, 1);
		descriptor := GraphGrammar.FindInternal("min");
		min := NewInternal(0, descriptor);
		min.parents[0] := one;
		min.parents[1] := condition;
		if.upper := min;
		s.Scan;
		NEW(statement);
		statement.variable := NIL;
		statement.expression := NIL;
		statement.density := NIL;
		statement.index := if;
		RETURN statement
	END ParseIf;

	PROCEDURE ReverseList* (model: Statement): Statement;
		VAR
			cursor, loops, statement: Statement;
	BEGIN
		cursor := model;
		model := NIL;
		WHILE cursor # NIL DO
			loops := NIL;
			WHILE cursor.index # NIL DO
				statement := BuildForLoop(cursor.index);
				statement.CopyToList(loops); 	(*	push loop stack	*)
				cursor := cursor.next
			END;
			NEW(statement);
			statement.variable := cursor.variable;
			statement.expression := cursor.expression;
			statement.density := cursor.density;
			statement.index := cursor.index;
			statement.next := model;
			model := statement;
			IF loops # NIL THEN loops.MergeLists(model) END; 	(*	put loops before statement	*)
			cursor := cursor.next
		END;
		RETURN model
	END ReverseList;

	PROCEDURE ParseModel* (VAR s: BugsMappers.Scanner);
		VAR
			i, numPar, numSlots: INTEGER;
			function, expression: Node;
			dependent, derivative, independent, variable, functionVar: Variable;
			density: Density;
			parents: POINTER TO ARRAY OF Node;
			loops, statement: Statement;
			descriptor: GraphGrammar.External;
			fact: GraphNodes.Factory;
			signature: ARRAY 64 OF CHAR;
	BEGIN
		model := NIL;
		loops := NIL;
		error := FALSE;
		NEW(parents, 1);
		s.Scan;
		LOOP
			IF ((s.type = BugsMappers.string) OR (s.type = BugsMappers.function)) & (s.string = "for") THEN
				s.Scan;
				statement := ParseForLoop(loops, s);
				IF statement = NIL THEN RETURN END;
				statement.CopyToList(loops)	(*	push loop stack	*)
			ELSIF ((s.type = BugsMappers.string) OR (s.type = BugsMappers.function)) & (s.string = "if") THEN
				s.Scan;
				statement := ParseIf(loops, s);
				IF statement = NIL THEN RETURN END;
				statement.CopyToList(loops)	(*	push loop stack	*)
			ELSIF s.type = BugsMappers.string THEN
				variable := ParseVariable(loops, s);
				IF variable = NIL THEN RETURN END;
				numSlots := variable.name.numSlots;
				i := 0;
				WHILE i < numSlots DO
					IF variable.lower[i] = NIL THEN
						Error(37); RETURN (*	empty slot not allowed in variable name	*)
					END;
					INC(i)
				END;
				s.Scan;
				IF (s.type = BugsMappers.char) & (s.char = "~") THEN
					s.Scan;
					density := ParseDensity(loops, s);
					IF density = NIL THEN RETURN END;
					statement := BuildStochastic(variable, density);
					statement.CopyToList(model);
					IF loops # NIL THEN loops.MergeLists(model) END	(*	put loops before statement	*)
				ELSIF (s.type = BugsMappers.char) & (s.char = "<") THEN
					s.Scan;
					IF (s.type # BugsMappers.char) OR (s.char # "-") THEN
						Error(38); RETURN (*	expected arrow <-	*)
					END;
					s.Scan;
					expression := ParseExpression(loops, s);
					IF expression = NIL THEN RETURN END;
					statement := BuildLogical(variable, expression);
					statement.CopyToList(model);
					IF loops # NIL THEN loops.MergeLists(model) END	(*	put loops before statement	*)
				ELSE
					Error(39); RETURN (*	expected arrow or twidles	*)
				END;
				IF (s.type = BugsMappers.char) & (s.char = ";") THEN s.Scan END
			ELSIF s.type = BugsMappers.function THEN
				descriptor := GraphGrammar.FindFunction(s.string);
				IF descriptor = NIL THEN
					IF s.string = "D" THEN
						ParseDnotation(loops, s, derivative, dependent, independent);
						derivative.name.isVariable := TRUE; 
						dependent.name.isVariable := TRUE; 
						independent.name.isVariable := TRUE;
						IF s.char # "<" THEN Error(38); RETURN END;
						s.Scan;
						IF s.char # "-" THEN Error(38); RETURN END;
						s.Scan;
						expression := ParseExpression(loops, s);
						IF expression = NIL THEN RETURN END;
						statement := BuildLogical(derivative, expression);
						statement.CopyToList(model); (*	put loops before statement	*)
						IF loops # NIL THEN loops.MergeLists(model) END;
						(*	set up dummy relationship for dependent variables	*)
						NEW(density);
						density.descriptor := GraphGrammar.FindDensity("_dummy_");
						density.parents := NIL;
						density.leftCen := NIL;
						density.rightCen := NIL;
						statement := BuildStochastic(dependent, density);
						statement.CopyToList(model);
						IF loops # NIL THEN loops.MergeLists(model) END;
						(*	put loops before statement	*)
						IF (((independent.name.numSlots = 0) & (independent.name.components = NIL)
							OR ((independent.name.numSlots > 0) & (independent.name.slotSizes[0] = 0))))
							THEN
							(*	only do this stuff once	*)
							IF independent.name.numSlots = 0 THEN
								independent.name.AllocateNodes
							ELSE
								independent.name.slotSizes[0] := 1
							END;
							(*	set up dummy relationship for independent variable	*)
							NEW(density);
							density.descriptor := GraphGrammar.FindDensity("_dummy_");
							density.parents := NIL;
							density.leftCen := NIL;
							density.rightCen := NIL;
							statement := BuildStochastic(independent, density);
							statement.CopyToList(model);
							(*	put loops before statement	*)
							IF loops # NIL THEN loops.MergeLists(model) END
						END
					ELSIF s.string = "F" THEN
						ParseFnotation(loops, s, functionVar, independent);
						functionVar.name.isVariable := TRUE; 
						independent.name.isVariable := TRUE; 
						IF s.char # "<" THEN Error(38); RETURN END;
						s.Scan;
						IF s.char # "-" THEN Error(38); RETURN END;
						s.Scan;
						expression := ParseExpression(loops, s);
						IF expression = NIL THEN RETURN END;
						statement := BuildLogical(functionVar, expression);
						statement.CopyToList(model);
						(*	put loops before statement	*)
						IF loops # NIL THEN loops.MergeLists(model) END;
						(*	set up dummy relationship for independent variable	*)
						NEW(density);
						density.descriptor := GraphGrammar.FindDensity("_dummy_");
						density.parents := NIL;
						density.leftCen := NIL;
						density.rightCen := NIL;
						statement := BuildStochastic(independent, density);
						statement.CopyToList(model);
						(*	put loops before statement	*)
						IF loops # NIL THEN loops.MergeLists(model) END
					ELSE
						Error(40); RETURN (*	unknown type of fogical function	*)
					END
				ELSE
					fact := descriptor.fact;
					fact.Signature(signature);
					numPar := fact.NumParam();
					IF signature[numPar] = "L" THEN
						s.Scan;
						s.Scan;
						variable := ParseVariable(loops, s);
						IF variable = NIL THEN RETURN END;
						s.Scan;
						IF (s.type # BugsMappers.char) OR (s.char # ")") THEN
							Error(42); RETURN (*	expected right parenthesis	*)
						END;
						s.Scan;
						IF (s.type # BugsMappers.char) OR (s.char # "<") THEN
							Error(43); RETURN (*	expected <- 	*)
						END;
						s.Scan;
						IF (s.type # BugsMappers.char) OR (s.char # "-") THEN
							Error(44); RETURN (*	expected <- 	*)
						END;
						s.Scan;
						expression := ParseExpression(loops, s);
						IF expression = NIL THEN RETURN END;
						parents[0] := expression;
						function := BuildFunction(parents, descriptor);
						statement := BuildLogical(variable, function);
						statement.CopyToList(model);
						IF (s.type = BugsMappers.char) & (s.char = ";") THEN s.Scan END;
						(*	put loops before statement	*)
						IF loops # NIL THEN loops.MergeLists(model) END
					ELSE
						Error(41); RETURN (*	function is not a link function	*)
					END
				END
			ELSIF (s.type = BugsMappers.char) & (s.char = "}") THEN
				IF loops = NIL THEN model := ReverseList(model); RETURN END;
				loops := loops.next; 	(*	pop loop stack	*)
				s.Scan
			ELSE
				Error(45); RETURN (*	invalid or unexpected token scanned	*)
			END
		END;
	END ParseModel;

	PROCEDURE StringToVariable* (IN string: ARRAY OF CHAR): Variable;
		VAR
			s: BugsMappers.Scanner;
			name: BugsNames.Name;
			variable: Variable;
	BEGIN
		s.ConnectToString(string);
		s.SetPos(0);
		s.Scan;
		IF s.type # BugsMappers.string THEN
			Error(13);
			RETURN NIL
		END;
		name := BugsIndex.Find(s.string);
		IF name = NIL THEN
			Error(13);
			RETURN NIL
		END;
		variable := ParseVariable(NIL, s);
		RETURN variable
	END StringToVariable;

	PROCEDURE CountStatements* (OUT logical, stochastic: INTEGER);
		VAR
			list: Statement;
	BEGIN
		list := model;
		logical := 0;
		stochastic := 0;
		WHILE list # NIL DO
			IF list.expression # NIL THEN INC(logical)
			ELSIF list.density # NIL THEN INC(stochastic)
			END;
			list := list.next
		END
	END CountStatements;

	PROCEDURE BeginExternalize (VAR wr: Stores.Writer);
	BEGIN
		startOfGraph := wr.Pos();
		wr.WriteInt(MIN(INTEGER));
		nodes := NIL;
		nodeList := NIL;
		nodeLabel := 0;
	END BeginExternalize;

	PROCEDURE EndExternalize (VAR wr: Stores.Writer);
		VAR
			endPos: INTEGER;
	BEGIN
		endPos := wr.Pos();
		wr.SetPos(startOfGraph);
		wr.WriteInt(nodeLabel);
		(*	clear label field	*)
		WHILE nodeList # NIL DO
			nodeList.node.label := 0;
			nodeList := nodeList.next
		END;
		wr.SetPos(endPos)
	END EndExternalize;

	PROCEDURE Externalize* (statement: Statement; VAR wr: Stores.Writer);
		VAR
			num: INTEGER;
			state: Statement;
	BEGIN
		BeginExternalize(wr);
		state := statement;
		num := 0;
		WHILE state # NIL DO INC(num); state := state.next END;
		wr.WriteInt(num);
		state := statement;
		WHILE state # NIL DO
			ExternalizeNode(state.variable, wr);
			ExternalizeNode(state.expression, wr);
			ExternalizeNode(state.density, wr);
			ExternalizeNode(state.index, wr);
			state := state.next
		END;
		(*	clear node labels	*)

		EndExternalize(wr)
	END Externalize;

	PROCEDURE BeginInternalize (VAR rd: Stores.Reader);
		VAR
			numNodes: INTEGER;
	BEGIN
		rd.ReadInt(numNodes);
		NEW(nodes, numNodes + 1);
		nodes[0] := NIL;
	END BeginInternalize;

	PROCEDURE EndInternalize;
	BEGIN
		nodes := NIL;
		nodeLabel := 0
	END EndInternalize;

	PROCEDURE Internalize* (VAR rd: Stores.Reader): Statement;
		VAR
			num: INTEGER;
			elem, state: Statement;
			node: Node;
			statement: Statement;
	BEGIN
		BeginInternalize(rd);
		statement := NIL;
		rd.ReadInt(num);
		WHILE num > 0 DO
			NEW(state);
			InternalizeNode(node, rd);
			IF node # NIL THEN
				state.variable := node(Variable)
			ELSE
				state.variable := NIL
			END;
			InternalizeNode(state.expression, rd);
			InternalizeNode(node, rd);
			IF node # NIL THEN
				state.density := node(Density)
			ELSE
				state.density := NIL
			END;
			InternalizeNode(node, rd);
			IF node # NIL THEN
				state.index := node(Index)
			ELSE
				state.index := NIL
			END;
			state.next := statement;
			statement := state;
			DEC(num)
		END;
		(*	reverse list to get model in correct order	*)
		state := statement;
		statement := NIL;
		WHILE state # NIL DO
			NEW(elem);
			elem.variable := state.variable;
			elem.expression := state.expression;
			elem.density := state.density;
			elem.index := state.index;
			elem.next := statement;
			statement := elem;
			state := state.next
		END;
		EndInternalize;
		RETURN statement
	END Internalize;

	PROCEDURE IsFunctional (IN signiture: ARRAY OF CHAR): BOOLEAN;
		VAR
			i, len: INTEGER;
	BEGIN
		i := 0;
		len := LEN(signiture$);
		WHILE (i < len) & (signiture[i] # "F") & (signiture[i] # "D") DO INC(i) END;
		RETURN i < len
	END IsFunctional;
	
	PROCEDURE MarkExpression (expression: Node);
		VAR
			parents: POINTER TO ARRAY OF Node;
			index, parent: Node;
			name: BugsNames.Name;
			i, numParents, numSlots, numVarIndex: INTEGER;
			signiture: ARRAY 64 OF CHAR;
			isFunctional: BOOLEAN;
	BEGIN
		WITH expression: Function DO
			parents := expression.parents;
			IF parents # NIL THEN numParents := LEN(parents) ELSE numParents := 0 END;
			expression.descriptor.fact.Signature(signiture);
			isFunctional := IsFunctional(signiture);
			i := 0;
			WHILE i < numParents DO
				parent := parents[i];
				WITH parent: Variable DO
					IF isFunctional OR (signiture[i] # "v") THEN parent.name.isVariable := TRUE END;
				ELSE
				END;
				INC(i)
			END
		|expression: Binary DO
			MarkExpression(expression.left); 
			IF expression.right # NIL THEN MarkExpression(expression.right) END
		|expression: Internal DO
			parents := expression.parents;
			IF parents # NIL THEN numParents := LEN(parents) ELSE numParents := 0 END;
			i := 0;
			WHILE i < numParents DO
				parent := parents[i];
				MarkExpression(parent);
				INC(i)
			END
		|expression: Variable DO
			name := expression.name;
			i := 0;
			numVarIndex := 0;
			numSlots := name.numSlots;
			WHILE i < numSlots DO
				index := expression.lower[i];
				IF index # NIL THEN
					WITH index: Index DO
					|index: Integer DO
					|index: Real DO
					ELSE
						name.isVariable := TRUE	(*	possible mixture model	*)
					END
				END;
				INC(i)
			END
		ELSE
		END
	END MarkExpression;

	(*	mark names which must be passed by reference	*)
	PROCEDURE MarkVariables*;
		VAR
			list: Statement;
			variable: Variable;
			density: Density;
			expression: Node;
			parents: POINTER TO ARRAY OF Node;
			parent: Node;
			i, numParents: INTEGER;
	BEGIN
		list := model;
		WHILE list # NIL DO
			IF list.expression # NIL THEN
				variable := list.variable;
				variable.name.isVariable := TRUE;
				expression := list.expression;
				MarkExpression(expression)
			ELSIF list.density # NIL THEN
				variable := list.variable;
				variable.name.isVariable := TRUE;
				density := list.density;
				parents := density.parents;
				IF parents # NIL THEN numParents := LEN(parents) ELSE numParents := 0 END;
				i := 0;
				WHILE i < numParents DO
					parent := parents[i];
					WITH parent: Variable DO
						parent.name.isVariable := TRUE
					ELSE
					END;
					INC(i)
				END
			END;
			list := list.next
		END
	END MarkVariables;

	PROCEDURE SetModel* (m: Statement);
	BEGIN
		model := m;
		error := FALSE
	END SetModel;

	PROCEDURE Clear*;
	BEGIN
		nodes := NIL;
		nodeLabel := 0;
		error := FALSE;
		model := NIL;
		numIfs := 0
	END Clear;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Clear;
		Maintainer
	END Init;

BEGIN
	Init
END BugsParser.

	
