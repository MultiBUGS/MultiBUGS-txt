(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsNodes;


	

	IMPORT
		Strings,
		BugsCPCompiler, BugsCodegen, BugsEvaluate, BugsIndex, BugsMsg, BugsNames,
		BugsOptimize, BugsParser,
		GraphConstant, GraphGrammar, GraphLogical, GraphMultivariate, GraphNodes,
		GraphSentinel, GraphStochastic, GraphVector;

	TYPE

		Allocator = POINTER TO RECORD(BugsNames.Visitor)
			ok: BOOLEAN
		END;

		Dimensions = POINTER TO RECORD(BugsEvaluate.Visitor) END;

		CreateLogical = POINTER TO RECORD(BugsEvaluate.Visitor) END;

		CreateSentinel = POINTER TO RECORD(BugsEvaluate.Visitor) END;

		CreateStochastic = POINTER TO RECORD(BugsEvaluate.Visitor) END;

		CreateConstant = POINTER TO RECORD(BugsEvaluate.Visitor)
			repeat: BOOLEAN
		END;

		WriteLogical = POINTER TO RECORD(BugsEvaluate.Visitor) END;

		WriteStochastic = POINTER TO RECORD(BugsEvaluate.Visitor) END;

		Check = POINTER TO RECORD(BugsNames.ElementVisitor)
			ok: BOOLEAN
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Error (errorNum: INTEGER; name: ARRAY OF CHAR);
		VAR
			errorMsg: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
			numToString: ARRAY 8 OF CHAR;
	BEGIN
		Strings.IntToString(errorNum, numToString);
		p[0] := name$;
		BugsMsg.LookupParam("BugsNodes" + numToString, p, errorMsg); 
		BugsMsg.StoreError(errorMsg)
	END Error;

	PROCEDURE NodeError (error: SET; node: GraphNodes.Node);
		VAR
			i, pos: INTEGER;
			numToString: ARRAY 8 OF CHAR;
			errorMes, install, name, param: ARRAY 1024 OF CHAR;
			external: GraphGrammar.External;
	BEGIN
		BugsIndex.FindGraphNode(node, name);
		pos := LEN(name$);
		Strings.Extract(name, 1, pos - 2, name);
		node.Install(install);
		external := GraphGrammar.FindInstalled(install);
		IF external # NIL THEN
			errorMes := external.name$
		ELSE
			Strings.Find(install, "DynamicNode", 0, pos);
			IF pos # - 1 THEN
				errorMes := "logical"
			ELSE
				errorMes := "unknown type"
			END;
		END;
		errorMes := "error for node " + name + " of type " + errorMes + ", ";
		i := 0;
		WHILE i <= MAX(SET) DO
			IF i IN error THEN
				Strings.IntToString(i, numToString);
				BugsMsg.Lookup("Graph" + numToString, param);
				errorMes := errorMes + " " + param;
			END;
			INC(i)
		END;
		BugsMsg.StoreError(errorMes)
	END NodeError;

	PROCEDURE (visitor: Dimensions) Node (statement: BugsParser.Statement; OUT ok: BOOLEAN);
		VAR
			d, i, numSlots: INTEGER;
			name: BugsNames.Name;
			variable: BugsParser.Variable;
	BEGIN
		ok := TRUE;
		variable := statement.variable;
		name := variable.name;
		IF variable.lower # NIL THEN
			i := 0;
			numSlots := name.numSlots;
			WHILE i < numSlots DO
				IF variable.lower[i] # NIL THEN
					d := BugsEvaluate.Index(variable.lower[i]);
					IF d = BugsEvaluate.error THEN
						ok := FALSE; RETURN
					END;
					IF d > name.slotSizes[i] THEN
						IF name.components = NIL THEN
							name.slotSizes[i] := d
						ELSE (*	array index greater than array bounds	*)
							ok := FALSE; Error(1, name.string); RETURN
						END
					END
				END;
				IF variable.upper[i] # NIL THEN
					d := BugsEvaluate.Index(variable.upper[i]);
					IF d = BugsEvaluate.error THEN
						ok := FALSE; RETURN
					END;
					IF d > name.slotSizes[i] THEN
						IF name.components = NIL THEN
							name.slotSizes[i] := d
						ELSE (*	array index greater than array bounds	*)
							ok := FALSE; Error(2, name.string); RETURN
						END
					END
				END;
				INC(i)
			END
		END
	END Node;

	PROCEDURE (v: Allocator) Do (name: BugsNames.Name);
		VAR
			nElem: INTEGER;
	BEGIN
		IF name.components = NIL THEN
			nElem := name.Size();
			IF nElem > 0 THEN
				name.AllocateNodes
			ELSE (*	variable not defined or not data	*)
				v.ok := FALSE; Error(3, name.string); RETURN
			END
		END
	END Do;

	PROCEDURE Allocate* (source: BugsParser.Statement; OUT ok: BOOLEAN);
		VAR
			visitor: Dimensions;
			v: Allocator;
	BEGIN
		NEW(visitor);
		source.Accept(visitor, ok);
		IF ok THEN
			NEW(v);
			v.ok := TRUE;
			BugsIndex.Accept(v);
			ok := v.ok
		END
	END Allocate;

	PROCEDURE Logical (expression: BugsParser.Node): GraphNodes.Node;
		VAR
			function: BugsParser.Function;
			node: GraphNodes.Node;
			args: GraphStochastic.ArgsLogical;
	BEGIN
		IF expression IS BugsParser.Function THEN
			function := expression(BugsParser.Function);
			node := function.descriptor.fact.New()
		ELSE
			args.Init;
			BugsOptimize.FoldConstants(expression);
			BugsCodegen.WriteTreeArgs(expression, args); 
			IF ~args.valid THEN RETURN NIL END;
			BugsOptimize.UnMark(expression);
			node := BugsCPCompiler.CreateLogical(args);
			ASSERT(node # NIL, 77);
		END;
		RETURN node
	END Logical;

	PROCEDURE (visitor: CreateLogical) Node (statement: BugsParser.Statement; OUT ok: BOOLEAN);
		VAR
			i, end, index: INTEGER;
			expression: BugsParser.Node;
			variable: BugsParser.Variable;
			vector: GraphNodes.SubVector;
			multivariate: GraphVector.Node;
			components: GraphLogical.Vector;
	BEGIN
		ok := TRUE;
		expression := statement.expression;
		IF expression # NIL THEN
			variable := statement.variable;
			BugsEvaluate.LHVariable(variable, vector);
			i := vector.start;
			end := vector.start + vector.nElem;
			WHILE i < end DO
				IF ~(GraphNodes.data IN vector.components[i].props) THEN
					vector.components[i] := Logical(expression);
					IF vector.components[i] = NIL THEN
						ok := FALSE; RETURN
					END
				END;
				INC(i)
			END;
			IF vector.components[vector.start] IS GraphVector.Node THEN
				NEW(components, vector.nElem);
				index := 0;
				i := vector.start;
				WHILE i < end DO
					multivariate := vector.components[i](GraphVector.Node);
					components[index] := multivariate;
					multivariate.SetComponent(components, index);
					INC(index);
					INC(i)
				END
			ELSIF vector.nElem > 1 THEN
				ok := FALSE; Error(8, variable.name.string)
			END
		END
	END Node;

	PROCEDURE CreateLogicals* (source: BugsParser.Statement; OUT ok: BOOLEAN);
		VAR
			visitor: CreateLogical;
	BEGIN
		NEW(visitor);
		source.Accept(visitor, ok)
	END CreateLogicals;

	PROCEDURE (visitor: CreateSentinel) Node (statement: BugsParser.Statement; OUT ok: BOOLEAN);
		VAR
			i, end: INTEGER;
			indices: ARRAY 120 OF CHAR;
			variable: BugsParser.Variable;
			vector: GraphNodes.SubVector;
	BEGIN
		ok := TRUE;
		IF statement.expression # NIL THEN
			variable := statement.variable;
			BugsEvaluate.LHVariable(variable, vector);
			IF vector.components = NIL THEN ok := FALSE; RETURN END;
			IF vector.step # 1 THEN 	(* non consecative elements *)
				ok := FALSE; Error(4, variable.name.string); RETURN
			END;
			i := vector.start;
			end := vector.start + vector.nElem;
			WHILE i < end DO
				IF vector.components[i] # NIL THEN (* multiple definitions of node *)
					variable.name.Indices(i, indices);
					ok := FALSE; Error(5, variable.name.string + indices); RETURN
				END;
				vector.components[i] := GraphSentinel.New();
				INC(i)
			END;
		END
	END Node;

	PROCEDURE CreateSentinels* (source: BugsParser.Statement; OUT ok: BOOLEAN);
		VAR
			visitor: CreateSentinel;
	BEGIN
		NEW(visitor);
		source.Accept(visitor, ok)
	END CreateSentinels;

	PROCEDURE Stochastic (density: BugsParser.Density): GraphStochastic.Node;
		VAR
			node: GraphNodes.Node;
			descriptor: GraphGrammar.External;
			fact: GraphNodes.Factory;
	BEGIN
		descriptor := density.descriptor;
		fact := descriptor.fact;
		node := fact.New();
		IF node # NIL THEN
			RETURN node(GraphStochastic.Node)
		ELSE
			RETURN NIL
		END
	END Stochastic;

	PROCEDURE (visitor: CreateStochastic) Node (statement: BugsParser.Statement; OUT ok: BOOLEAN);
		VAR
			i, end, index: INTEGER;
			indices: ARRAY 120 OF CHAR;
			density: BugsParser.Density;
			variable: BugsParser.Variable;
			stochastic: GraphStochastic.Node;
			multivariate: GraphMultivariate.Node;
			vector: GraphNodes.SubVector;
			components: GraphStochastic.Vector;
	BEGIN
		ok := TRUE;
		IF statement.density # NIL THEN
			variable := statement.variable;
			BugsEvaluate.LHVariable(variable, vector);
			IF vector.components = NIL THEN ok := FALSE; RETURN END;
			IF vector.step # 1 THEN (*	non consecative elements	*)
				ok := FALSE; Error(6, variable.name.string); RETURN
			END;
			density := statement.density;
			i := vector.start;
			end := vector.start + vector.nElem;
			WHILE i < end DO
				IF vector.components[i] # NIL THEN
					IF GraphNodes.data IN vector.components[i].props THEN
						stochastic := Stochastic(density);
						IF stochastic = NIL THEN (*	unable to create stochastic node	*)
							ok := FALSE; Error(9, variable.name.string + indices); RETURN
						END;
						stochastic.SetValue(vector.components[i].Value());
						stochastic.SetProps(stochastic.props + {GraphNodes.data});
						IF GraphStochastic.logical IN vector.components[i].props THEN
							stochastic.SetProps(stochastic.props + {GraphStochastic.logical})
						END;
						vector.components[i] := stochastic
					ELSE (*	multiple definitions of node	*)
						variable.name.Indices(i, indices);
						ok := FALSE; Error(7, variable.name.string + indices); RETURN
					END
				ELSE
					stochastic := Stochastic(density);
					IF stochastic = NIL THEN (*	unable to create stochastic node	*)
						ok := FALSE; Error(9, variable.name.string + indices); RETURN
					END;
					vector.components[i] := stochastic
				END;
				INC(i)
			END;
			IF vector.components[vector.start] IS GraphMultivariate.Node THEN
				NEW(components, vector.nElem);
				index := 0;
				i := vector.start;
				WHILE i < end DO
					multivariate := vector.components[i](GraphMultivariate.Node);
					components[index] := multivariate;
					multivariate.SetComponent(components, index);
					INC(index);
					INC(i)
				END
			ELSIF vector.nElem > 1 THEN
				ok := FALSE; Error(8, variable.name.string)
			END
		END
	END Node;

	PROCEDURE CreateStochastics* (source: BugsParser.Statement; OUT ok: BOOLEAN);
		VAR
			visitor: CreateStochastic;
	BEGIN
		NEW(visitor);
		source.Accept(visitor, ok)
	END CreateStochastics;

	PROCEDURE (visitor: CreateConstant) Node (statement: BugsParser.Statement; OUT ok: BOOLEAN);
		VAR
			i: INTEGER;
			expression: BugsParser.Node;
			variable: BugsParser.Variable;
			vector: GraphNodes.SubVector;
			node: GraphNodes.Node;
	BEGIN
		ok := TRUE;
		expression := statement.expression;
		IF expression # NIL THEN
			variable := statement.variable;
			BugsEvaluate.LHVariable(variable, vector);
			i := vector.start;
			IF vector.nElem # 1 THEN RETURN END; (*	can not fold vector valued constants	*)
			IF (vector.components[i] # NIL) & (GraphNodes.data IN vector.components[i].props) THEN
				RETURN
			END;
			BugsOptimize.FoldConstants(expression);
			IF expression.evaluated THEN
				node := GraphConstant.New(expression.value);
				node.SetProps(node.props + {GraphStochastic.logical});
				vector.components[i] := node;
				visitor.repeat := TRUE
			END;
			BugsOptimize.UnMark(expression)
		END
	END Node;

	PROCEDURE CreateConstants* (source: BugsParser.Statement; OUT ok: BOOLEAN);
		VAR
			visitor: CreateConstant;
	BEGIN
		NEW(visitor);
		REPEAT
			visitor.repeat := FALSE;
			source.Accept(visitor, ok)
		UNTIL ~visitor.repeat
	END CreateConstants;

	PROCEDURE (visitor: WriteLogical) Node (statement: BugsParser.Statement; OUT ok: BOOLEAN);
		VAR
			vectorValued: BOOLEAN;
			i, end: INTEGER;
			res: SET;
			varName: ARRAY 120 OF CHAR;
			descriptor: GraphGrammar.External;
			expression: BugsParser.Node;
			function: BugsParser.Function;
			variable: BugsParser.Variable;
			args: GraphStochastic.ArgsLogical;
			node, ref: GraphNodes.Node;
			vector: GraphNodes.SubVector;
			fact: GraphNodes.Factory;
			sig: ARRAY 64 OF CHAR;
		CONST
			undefined = {16};
	BEGIN
		ok := TRUE;
		expression := statement.expression;
		IF expression = NIL THEN RETURN END;
		variable := statement.variable;
		BugsEvaluate.LHVariable(variable, vector);
		IF vector.components = NIL THEN ok := FALSE; RETURN END;
		node := vector.components[vector.start];
		IF GraphNodes.data IN node.props THEN RETURN END;
		variable.name.Indices(vector.start, varName);
		varName := variable.name.string + varName;
		args.Init;
		BugsOptimize.UnMark(expression);
		BugsOptimize.FoldConstants(expression);
		IF expression IS BugsParser.Function THEN	(*	external function	*)
			function := expression(BugsParser.Function);
			descriptor := function.descriptor;
			fact := descriptor.fact;
			fact.Signature(sig);
			vectorValued := fact IS GraphVector.Factory;
			IF vector.nElem = 1 THEN
				IF vectorValued THEN ok := FALSE; Error(14, varName); RETURN END
			ELSE
				IF ~vectorValued THEN ok := FALSE; Error(15, varName); RETURN END
			END;
			IF sig = "eL" THEN
				BugsCodegen.WriteTreeArgs(function.parents[0], args);
				ref := BugsCPCompiler.CreateLogical(args);
				ref.Set(args, res); ASSERT(res = {}, 77);
				args.Init;
				args.numScalars := 1;
				args.scalars[0] := ref;
				node.Set(args, res); ASSERT(res = {}, 88)
			ELSE
				BugsCodegen.WriteFunctionArgs(function, args);
				IF ~args.valid THEN ok := FALSE; Error(10, varName); RETURN END;
				i := vector.start;
				end := vector.start + vector.nElem;
				WHILE i < end DO
					node := vector.components[i];
					node.Set(args, res);
					IF res # {} THEN ok := FALSE; NodeError(res, node); RETURN END;
					INC(i)
				END
			END
		ELSE	(*	tree	*)
			ASSERT(vector.nElem = 1, 66);
			IF node IS GraphLogical.Node THEN
				BugsCodegen.WriteTreeArgs(expression, args);
				IF~args.valid THEN ok := FALSE; NodeError(undefined, node); RETURN END;
				node.Set(args, res);
				IF res # {} THEN ok := FALSE; NodeError(res, node); RETURN END;
			END
		END
	END Node;

	PROCEDURE WriteLogicals* (source: BugsParser.Statement; OUT ok: BOOLEAN);
		VAR
			visitor: WriteLogical;
	BEGIN
		ok := TRUE;
		NEW(visitor);
		source.Accept(visitor, ok)
	END WriteLogicals;

	PROCEDURE (visitor: WriteStochastic) Node (statement: BugsParser.Statement; OUT ok: BOOLEAN);
		VAR
			multivariate: BOOLEAN;
			i, end: INTEGER;
			res: SET;
			varName: ARRAY 120 OF CHAR;
			density: BugsParser.Density;
			variable: BugsParser.Variable;
			name: BugsNames.Name;
			node: GraphNodes.Node;
			vector: GraphNodes.SubVector;
			args: GraphStochastic.Args;
			descriptor: GraphGrammar.External;
			fact: GraphNodes.Factory;
	BEGIN
		ok := TRUE;
		density := statement.density;
		IF density = NIL THEN RETURN END;
		variable := statement.variable;
		descriptor := density.descriptor;
		fact := descriptor.fact;
		multivariate := fact IS GraphMultivariate.Factory;
		BugsEvaluate.LHVariable(variable, vector);
		IF vector.components = NIL THEN ok := FALSE; RETURN END;
		name := variable.name;
		name.Indices(vector.start, varName);
		varName := name.string + varName;
		IF vector.nElem = 1 THEN
			IF multivariate THEN ok := FALSE; Error(14, varName); RETURN END
		ELSE
			IF ~multivariate THEN ok := FALSE; Error(15, varName); RETURN END
		END;
		args.Init;
		BugsCodegen.WriteDensityArgs(density, args);
		IF ~args.valid THEN ok := FALSE; RETURN END;
		i := vector.start;
		end := vector.start + vector.nElem;
		WHILE i < end DO
			node := vector.components[i];
			node.Set(args, res);
			IF res # {} THEN ok := FALSE; NodeError(res, node); RETURN END;
			INC(i)
		END
	END Node;

	PROCEDURE WriteStochastics* (source: BugsParser.Statement; OUT ok: BOOLEAN);
		VAR
			visitor: WriteStochastic;
	BEGIN
		NEW(visitor);
		source.Accept(visitor, ok)
	END WriteStochastics;

	PROCEDURE (v: Check) Do (name: BugsNames.Name);
		VAR
			node: GraphNodes.Node;
			string: ARRAY 1024 OF CHAR;
			res: SET;
	BEGIN
		IF name.passByreference THEN
			node := name.components[v.index];
			IF (node # NIL) & v.ok THEN
				res := node.Check();
				IF res # {} THEN
					v.ok := FALSE;
					name.Indices(v.index, string);
					string := name.string + string;
					NodeError(res, node)
				END
			END
		END
	END Do;

	PROCEDURE Checks* (OUT ok: BOOLEAN);
		VAR
			v: Check;
	BEGIN
		NEW(v);
		v.ok := TRUE;
		BugsIndex.Accept(v);
		ok := v.ok
	END Checks;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END BugsNodes.
