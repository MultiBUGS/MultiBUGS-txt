(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsCodegen;


	

	IMPORT
		Strings,
		BugsEvaluate, BugsMsg, BugsParser,
		GraphGrammar, GraphLogical, GraphNodes, GraphStochastic;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	CONST
		tooManyOperators = 1;
		tooManyConstants = 2;
		tooManyScalars = 3;

	PROCEDURE Error (errorNum: INTEGER);
		VAR
			errorMsg: ARRAY 1024 OF CHAR;
			numToString: ARRAY 8 OF CHAR;
	BEGIN
		Strings.IntToString(errorNum, numToString);
		BugsMsg.Lookup("BugsCodegen" + numToString, errorMsg);
		BugsMsg.StoreError(errorMsg) 
	END Error;

	PROCEDURE NodeError (error: SET; node: GraphNodes.Node);
		VAR
			i, pos: INTEGER;
			numToString: ARRAY 8 OF CHAR;
			errorMes, param, install: ARRAY 1024 OF CHAR;
			external: GraphGrammar.External;
	BEGIN
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
		errorMes := "error for node of type " + errorMes;
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

	PROCEDURE Vector (t: BugsParser.Node): GraphNodes.SubVector;
		VAR
			var: BugsParser.Variable;
			vector: GraphNodes.SubVector;
	BEGIN
		var := t(BugsParser.Variable);
		vector := BugsEvaluate.RHVector(var);
		IF vector = NIL THEN RETURN NIL END;
		RETURN vector
	END Vector;

	PROCEDURE WriteVectorToFunc (t: BugsParser.Node; VAR args: GraphStochastic.ArgsLogical);
		VAR
			vector: GraphNodes.SubVector;
			i, nElem, start, step: INTEGER;
	BEGIN
		vector := Vector(t);
		IF vector = NIL THEN args.valid := FALSE; RETURN END;
		i := 0;
		start := vector.start;
		step := vector.step;
		nElem := vector.nElem;
		WHILE i < nElem DO
			IF (vector.components # NIL) & (vector.components[start + i * step] = NIL) THEN 
				args.valid := FALSE; RETURN 
			ELSIF (vector.values # NIL) & (vector.values[start + i * step] = INF) THEN
				args.valid := FALSE; RETURN 
			END;
			INC(i)
		END;
		args.vectors[args.numVectors] := vector;
		INC(args.numVectors)
	END WriteVectorToFunc;

	PROCEDURE WriteScalarToFunc (tree: BugsParser.Node; VAR args: GraphStochastic.ArgsLogical);
		VAR
			scalar: GraphNodes.Node;
	BEGIN
		scalar := BugsEvaluate.RHScalar(tree);
		IF scalar = NIL THEN args.valid := FALSE; RETURN END;
		args.scalars[args.numScalars] := scalar;
		INC(args.numScalars)
	END WriteScalarToFunc;

	PROCEDURE WriteScalarToStack (tree: BugsParser.Node; VAR args: GraphStochastic.ArgsLogical);
		VAR
			scalar: GraphNodes.Node;
			i: INTEGER;
	BEGIN
		scalar := BugsEvaluate.RHScalarOpt(tree);
		IF scalar = NIL THEN args.valid := FALSE; RETURN END;
		IF args.numOps = LEN(args.ops) THEN
			args.valid := FALSE; Error(tooManyOperators); RETURN
		END;
		IF GraphNodes.data IN scalar.props THEN
			args.ops[args.numOps] := GraphGrammar.const;
			INC(args.numOps);
			IF args.numConsts = LEN(args.consts) THEN
				args.valid := FALSE; Error(tooManyConstants); RETURN
			END;
			args.consts[args.numConsts] := scalar.Value();
			INC(args.numConsts);
		ELSE
			IF scalar IS GraphStochastic.Node THEN
				args.ops[args.numOps] := GraphGrammar.refStoch;
				INC(args.numOps);
				i := 0;
				WHILE (args.stochastics[i] # NIL) & (args.stochastics[i] # scalar) DO INC(i) END;
				IF args.stochastics[i] = NIL THEN
					args.stochastics[args.numStochs] := scalar(GraphStochastic.Node);
					INC(args.numStochs);
					IF args.numStochs = LEN(args.stochastics) THEN
						args.valid := FALSE; Error(tooManyScalars); RETURN
					END
				END;
				args.ops[args.numOps] := i;
				INC(args.numOps)
			ELSIF scalar IS GraphLogical.Node THEN
				args.ops[args.numOps] := GraphGrammar.ref;
				INC(args.numOps);
				i := 0;
				WHILE (args.logicals[i] # NIL) & (args.logicals[i] # scalar) DO INC(i) END;
				IF args.logicals[i] = NIL THEN
					args.logicals[args.numLogicals] := scalar(GraphLogical.Node);
					INC(args.numLogicals);
					IF args.numLogicals = LEN(args.logicals) THEN
						args.valid := FALSE; Error(tooManyScalars); RETURN
					END
				END;
				args.ops[args.numOps] := i;
				INC(args.numOps)
			ELSE
				HALT(0)
			END
		END
	END WriteScalarToStack;

	PROCEDURE WriteFunctionArgs* (function: BugsParser.Function;
	VAR args: GraphStochastic.ArgsLogical);
		VAR
			i, j, numPar: INTEGER;
			parents: POINTER TO ARRAY OF BugsParser.Node;
			descriptor: GraphGrammar.External;
			fact: GraphNodes.Factory;
			signature: ARRAY 64 OF CHAR;
	BEGIN
		i := 0;
		j := 0;
		descriptor := function.descriptor;
		fact := descriptor.fact;
		fact.Signature(signature);
		numPar := fact.NumParam();
		parents := function.parents;
		WHILE i < numPar DO
			IF signature[i] = "D" THEN
				WriteVectorToFunc(parents[j], args);
				IF ~args.valid THEN RETURN END;
				INC(j);
				WriteVectorToFunc(parents[j], args);
				IF ~args.valid THEN RETURN END;
				INC(j);
				args.scalars[args.numScalars] := BugsEvaluate.RHScalar(parents[j]);
				INC(args.numScalars)
			ELSIF signature[i] = "F" THEN
				args.scalars[args.numScalars] := BugsEvaluate.RHScalar(parents[j]);
				IF ~args.valid THEN RETURN END;
				INC(args.numScalars);
				INC(j);
				args.scalars[args.numScalars] := BugsEvaluate.RHScalar(parents[j]);
				INC(args.numScalars)
			ELSIF signature[i] = "v" THEN
				WriteVectorToFunc(parents[j], args);
				IF ~args.valid THEN RETURN END
			ELSIF signature[i] = "s" THEN
				WriteScalarToFunc(parents[j], args);
				IF ~args.valid THEN RETURN END
			ELSE (*	unknown type of arguement	*)
				HALT(77)
			END;
			INC(i);
			INC(j)
		END
	END WriteFunctionArgs;

	PROCEDURE WriteTreeArgs* (tree: BugsParser.Node; VAR args: GraphStochastic.ArgsLogical);

		PROCEDURE Write (t: BugsParser.Node);
			CONST
				eps = 1.0E-20;
			VAR
				i, modifier: INTEGER;
				res: SET;
				bin: BugsParser.Binary;
				func: BugsParser.Function;
				operator: BugsParser.Internal;
				funcNode: GraphNodes.Node;
				funcArgs: GraphStochastic.ArgsLogical;
		BEGIN
			IF~args.valid THEN RETURN END;
			IF t.evaluated THEN
				IF args.numConsts = LEN(args.consts) THEN
					args.valid := FALSE; Error(tooManyConstants); RETURN
				END;
				args.consts[args.numConsts] := t.value;
				INC(args.numConsts);
				IF args.numOps = LEN(args.ops) THEN
					args.valid := FALSE; Error(tooManyOperators); RETURN
				END;
				args.ops[args.numOps] := GraphGrammar.const;
				INC(args.numOps)
			ELSIF t IS BugsParser.Binary THEN
				bin := t(BugsParser.Binary);
				IF bin.op = GraphGrammar.uminus THEN
					Write(bin.left);
					IF ~args.valid THEN RETURN END;
					IF args.numOps = LEN(args.ops) THEN
						args.valid := FALSE; Error(tooManyOperators); RETURN
					END;
					args.ops[args.numOps] := bin.op;
					INC(args.numOps)
				ELSIF bin.op = GraphGrammar.mult THEN
					IF bin.left.evaluated & (ABS(bin.left.value - 1) < eps) THEN
						Write(bin.right);
						IF ~args.valid THEN RETURN END
					ELSIF bin.right.evaluated & (ABS(bin.right.value - 1) < eps) THEN
						Write(bin.left);
						IF ~args.valid THEN RETURN END
					ELSE
						Write(bin.left);
						IF ~args.valid THEN RETURN END;
						Write(bin.right);
						IF ~args.valid THEN RETURN END;
						IF args.numOps = LEN(args.ops) THEN
							args.valid := FALSE; Error(tooManyOperators); RETURN
						END;
						IF bin.left.evaluated THEN
							args.ops[args.numOps] := bin.op + GraphGrammar.leftConst
						ELSIF bin.right.evaluated THEN
							args.ops[args.numOps] := bin.op + GraphGrammar.rightConst
						ELSE
							args.ops[args.numOps] := bin.op
						END;
						INC(args.numOps)
					END
				ELSIF bin.op = GraphGrammar.add THEN
					IF bin.left.evaluated & (ABS(bin.left.value) < eps) THEN
						Write(bin.right);
						IF ~args.valid THEN RETURN END
					ELSIF bin.right.evaluated & (ABS(bin.right.value) < eps) THEN
						Write(bin.left);
						IF ~args.valid THEN RETURN END
					ELSE
						Write(bin.left);
						IF ~args.valid THEN RETURN END;
						Write(bin.right);
						IF ~args.valid THEN RETURN END;
						IF args.numOps = LEN(args.ops) THEN
							args.valid := FALSE; Error(tooManyOperators); RETURN
						END;
						IF bin.left.evaluated THEN
							args.ops[args.numOps] := bin.op + GraphGrammar.leftConst
						ELSIF bin.right.evaluated THEN
							args.ops[args.numOps] := bin.op + GraphGrammar.rightConst
						ELSE
							args.ops[args.numOps] := bin.op
						END;
						INC(args.numOps)
					END
				ELSE	(*	div and sub	*)
					Write(bin.left);
					IF ~args.valid THEN RETURN END;
					Write(bin.right);
					IF ~args.valid THEN RETURN END;
					IF args.numOps = LEN(args.ops) THEN
						args.valid := FALSE; Error(tooManyOperators); RETURN
					END;
					IF bin.left.evaluated THEN
						args.ops[args.numOps] := bin.op + GraphGrammar.leftConst
					ELSIF bin.right.evaluated THEN
						args.ops[args.numOps] := bin.op + GraphGrammar.rightConst
					ELSE
						args.ops[args.numOps] := bin.op
					END;
					INC(args.numOps)
				END
			ELSIF t IS BugsParser.Internal THEN
				operator := t(BugsParser.Internal);
				i := 0;
				WHILE i < operator.descriptor.numPar DO
					Write(operator.parents[i]);
					IF ~args.valid THEN RETURN END;
					INC(i)
				END;
				IF args.numOps = LEN(args.ops) THEN
					args.valid := FALSE; Error(tooManyOperators); RETURN
				END;
				IF operator.descriptor.key = GraphGrammar.pow THEN 
					IF operator.parents[0].evaluated THEN 
						args.ops[args.numOps] := GraphGrammar.pow + GraphGrammar.leftConst
					ELSIF operator.parents[1].evaluated THEN 
						args.ops[args.numOps] := GraphGrammar.pow + GraphGrammar.rightConst
					ELSE
						args.ops[args.numOps] := GraphGrammar.pow
					END
				ELSE
				args.ops[args.numOps] := operator.descriptor.key
				END;
				INC(args.numOps)
			ELSIF t IS BugsParser.Function THEN
				func := t(BugsParser.Function);
				funcNode := func.descriptor.fact.New();
				funcArgs.Init;
				WriteFunctionArgs(func, funcArgs);
				IF ~funcArgs.valid THEN args.valid := FALSE; RETURN END;
				funcNode.Set(funcArgs, res);
				IF res # {} THEN
					args.valid := FALSE; NodeError(res, funcNode); RETURN
				END;
				IF args.numOps = LEN(args.ops) THEN
					args.valid := FALSE; Error(tooManyOperators); RETURN
				END;
				IF args.numLogicals = LEN(args.logicals) THEN
					args.valid := FALSE; Error(tooManyScalars); RETURN
				END;
				args.logicals[args.numLogicals] := funcNode(GraphLogical.Node);
				args.ops[args.numOps] := GraphGrammar.ref;
				INC(args.numOps);
				args.ops[args.numOps] := args.numLogicals;
				INC(args.numOps);
				INC(args.numLogicals)
			ELSE
				WriteScalarToStack(t, args);
				IF ~args.valid THEN RETURN END
			END
		END Write;

	BEGIN
		Write(tree)
	END WriteTreeArgs;

	PROCEDURE WriteDensityArgs* (density: BugsParser.Density; VAR args: GraphStochastic.Args);
		VAR
			i, numPar: INTEGER;
			descriptor: GraphGrammar.External;
			variable: BugsParser.Variable;
			parents: POINTER TO ARRAY OF BugsParser.Node;
			scalar: GraphNodes.Node;
			vector: GraphNodes.SubVector;
			fact: GraphNodes.Factory;
			signature: ARRAY 64 OF CHAR;
	BEGIN
		descriptor := density.descriptor;
		fact := descriptor.fact;
		fact.Signature(signature);
		numPar := fact.NumParam();
		i := 0;
		parents := density.parents;
		WHILE i < numPar DO
			IF parents[i] IS BugsParser.Variable THEN
				variable := parents[i](BugsParser.Variable);
				IF signature[i] = "v" THEN
					vector := Vector(variable);
					IF vector = NIL THEN args.valid := FALSE; RETURN END;
					args.vectors[args.numVectors] := vector;
					INC(args.numVectors)
				ELSE
					scalar := BugsEvaluate.RHScalar(variable);
					IF scalar = NIL THEN args.valid := FALSE; RETURN END;
					args.scalars[args.numScalars] := scalar;
					INC(args.numScalars)
				END
			ELSE
				scalar := BugsEvaluate.RHScalar(parents[i]);
				IF scalar = NIL THEN args.valid := FALSE; RETURN END;
				args.scalars[args.numScalars] := scalar;
				INC(args.numScalars)
			END;
			INC(i)
		END;
		IF density.leftCen # NIL THEN
			args.leftCen := BugsEvaluate.RHScalar(density.leftCen);
			IF args.leftCen = NIL THEN args.valid := FALSE; RETURN END
		END;
		IF density.rightCen # NIL THEN
			args.rightCen := BugsEvaluate.RHScalar(density.rightCen);
			IF args.rightCen = NIL THEN args.valid := FALSE; RETURN END
		END;
		IF density.leftTrunc # NIL THEN
			args.leftTrunc := BugsEvaluate.RHScalar(density.leftTrunc);
			IF args.leftTrunc = NIL THEN args.valid := FALSE; RETURN END
		END;
		IF density.rightTrunc # NIL THEN
			args.rightTrunc := BugsEvaluate.RHScalar(density.rightTrunc);
			IF args.rightTrunc = NIL THEN args.valid := FALSE; RETURN END
		END
	END WriteDensityArgs;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END BugsCodegen.
