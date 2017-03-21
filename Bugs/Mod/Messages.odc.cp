(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		 *)

MODULE BugsMessages;

	

	IMPORT
		BugsMsg;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		Map: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		Map("BugsScript1", "non-matching brackets in command");
		Map("BugsScript2", "non-closed quote in command");
		Map("BugsScript3", "non-closed quote in command");
		Map("BugsScript4", "non-matching square brackets in command");
		Map("BugsScript5", "both single and double quotes in command");
		Map("BugsScript6", "unknown script command");

		Map("BugsCheck1", "array index less than one");
		Map("BugsCheck2", "array index greater than array upper bound for ^0");

		Map("BugsCodegen1", "logical expression contains too many constants");
		Map("BugsCodegen2", "logical expression contains too many operators");
		Map("BugsCodegen3", "logical expression contains too many operators");
		Map("BugsCodegen4", "logical expression contains too many operators");
		Map("BugsCodegen5", "logical expression contains too many operators");
		Map("BugsCodegen6", "logical expression contains too many operators");
		Map("BugsCodegen7", "logical expression contains too many operands");
		Map("BugsCodegen8", "logical expression contains too many operators");
		Map("BugsCodegen9", "logical expression contains too many operators");
		Map("BugsCodegen10", "logical expression contains too many operands");
		Map("BugsCodegen11", "logical expression contains too many operands");
		Map("BugsCodegen12", "vector-valued logical expression must have more than one component");
		Map("BugsCodegen13", "scalar-valued logical expression cannot have more than one component");
		Map("BugsCodegen14", "multivariate distribution must have more than one component");
		Map("BugsCodegen15", "univariate distribution can not have more than one component");

		Map("BugsEvaluate1", "integer divide by zero");
		Map("BugsEvaluate2", "variable ^0 is not defined");
		Map("BugsEvaluate3", "missing index");
		Map("BugsEvaluate4", "invalid use of range construct in ^0");
		Map("BugsEvaluate5", "array index greater than array upper bound for ^0");
		Map("BugsEvaluate6", "variable ^0 not data");
		Map("BugsEvaluate7", "variable ^0 not defined in index expression");
		Map("BugsEvaluate8", "array index is not an integer ^0");
		Map("BugsEvaluate9", "array index is greater than array upper bound for ^0");
		Map("BugsEvaluate10", "invalid range specified for  ^0");
		Map("BugsEvaluate11", "array index is less than one for ^0");
		Map("BugsEvaluate12", "array index is less than one for ^0");
		Map("BugsEvaluate13", "array index is greater than array upper bound for ^0");
		Map("BugsEvaluate14", "made use of undefined node ^0");
		Map("BugsEvaluate15", "array index is greater than array upper bound for ^0");
		Map("BugsEvaluate16", "invalid range specified for  ^0");
		Map("BugsEvaluate17", "array index is less than one for ^0");
		Map("BugsEvaluate18", "the vector quantity ^0 is not defined");
		Map("BugsEvaluate19", "the vector quantity ^0 has an undefined component");

		Map("BugsMask1", "array index of ^0 is less than one");
		Map("BugsMask2", "array index of ^0 is greater than array upper bound");
		Map("BugsMask3", "invalid index range specified for ^0");
		Map("BugsMask4", "array index of ^0 greater than array upper bound");
		Map("BugsMask5", "variable name ^0 must begin with a character");
		Map("BugsMask6", "name ^0 does not occur in model");

		Map("BugsGraph1", "unable to create updater for ^0");
		Map("BugsGraph2", "invalid use of weight function for node ^0");
		
		Map("BugsInterface1", "unable to generate initial values for node ^0");

		Map("BugsNodes1", "array index is greater than array upper bound for ^0");
		Map("BugsNodes2", "array index is greater than array upper bound for ^0");
		Map("BugsNodes3", "variable ^0 is not defined in model or in data set");
		Map("BugsNodes4", "vector-valued relation ^0 must involve consecutive elements of variable");
		Map("BugsNodes5", "multiple definitions of node ^0");
		Map("BugsNodes6", "vector-valued relation ^0 must involve consecutive elements of variable");
		Map("BugsNodes7", "multiple definitions of node ^0");
		Map("BugsNodes8", "expected multivariate node");
		Map("BugsNodes9", "unable to create stochastic node ^0 with these options");
		
		Map("BugsParallel1", "unable to create updater for ^0");

		Map("BugsParser1", "unknown type of logical function");
		Map("BugsParser2", "link function cannot be used on right hand side");
		Map("BugsParser3", "expected left parenthesis");
		Map("BugsParser4", "unknown type of argument for logical function");
		Map("BugsParser5", "expected comma");
		Map("BugsParser6", "expected right parenthesis");
		Map("BugsParser7", "expected left parenthesis");
		Map("BugsParser8", "expected comma");
		Map("BugsParser9", "expected right parenthesis");
		Map("BugsParser10", "unexpected token in factor");
		Map("BugsParser11", "logical function not allowed in integer expression");
		Map("BugsParser12", "expected right parenthesis");
		Map("BugsParser13", "expected variable name");
		Map("BugsParser14", "expected comma");
		Map("BugsParser15", "expected right square bracket");
		Map("BugsParser16", "expected an integer");
		Map("BugsParser17", "expected a number");
		Map("BugsParser18", "invalid or unexpected token scanned");
		Map("BugsParser19", "unknown type of probability density");
		Map("BugsParser20", "expected left parenthesis");
		Map("BugsParser21", "expected variable name");
		Map("BugsParser22", "unknown type of argument in probability density");
		Map("BugsParser23", "expected a comma");
		Map("BugsParser24", "expected right parenthesis");
		Map("BugsParser25", "this density cannot be censored");
		Map("BugsParser26", "expected left parenthesis");
		Map("BugsParser27", "expected comma");
		Map("BugsParser28", "expected right parenthesis");
		Map("BugsParser29", "loop index must be a name");
		Map("BugsParser30", "loop index cannot be a name of a variable in the model");
		Map("BugsParser31", "loop name already used in outer loop");
		Map("BugsParser32", "expected left parenthesis");
		Map("BugsParser33", "expected the key word in");
		Map("BugsParser34", "expected a colon");
		Map("BugsParser35", "expected right parenthesis");
		Map("BugsParser36", "expected an open brace {");
		Map("BugsParser37", "empty slot not allowed in variable name");
		Map("BugsParser38", "expected left pointing arrow <  - ");
		Map("BugsParser39", "expected left pointing arrow <  - or twiddles ~");
		Map("BugsParser40", "unknown type of logical function");
		Map("BugsParser41", "function is not a link function");
		Map("BugsParser42", "expected right parenthesis");
		Map("BugsParser43", "expected left pointing arrow <  - ");
		Map("BugsParser44", "expected left pointing arrow <  - ");
		Map("BugsParser45", "invalid or unexpected token scanned");
		Map("BugsParser46", "this density cannot be truncated");
		Map("BugsParser47", "this density already censored");
		Map("BugsParser48", "this density already truncated");

		Map("BugsRectData1", "invalid or unexpected token scanned");
		Map("BugsRectData2", "NA cannot be given a sign");
		Map("BugsRectData3", "expected a number or an NA or END");
		Map("BugsRectData4", "expected a closing square bracket]");
		Map("BugsRectData5", "column label cannot be a scalar");
		Map("BugsRectData6", "first slot of column label must be empty");
		Map("BugsRectData7", "second and later slots of column label must not be empty");
		Map("BugsRectData8", "unable to evaluate column label index");
		Map("BugsRectData9", "column label index is greater than array upper bound");
		Map("BugsRectData10", "range operator not allowed in column labels");
		Map("BugsRectData11", "data value already given for this component");
		Map("BugsRectData12", "incomplete row of data");
		Map("BugsRectData13", "wrong number of rows of data");
		Map("BugsRectData14", "no prior specified for this initial value");
		Map("BugsRectData15", "initial value given for non stochastic node");
		Map("BugsRectData16", "initial value given for data node");
		Map("BugsRectData17", "incomplete row of initial values");
		Map("BugsRectData18", "expected a comma");
		Map("BugsRectData19", "variables not in the model: ");

		Map("BugsSplusData1", "invalid or unexpected token scanned");
		Map("BugsSplusData2", "NA cannot be given a sign");
		Map("BugsSplusData3", "expected a number or an NA");
		Map("BugsSplusData4", "data value already given for this component of node");
		Map("BugsSplusData5", "expected a comma or right parenthesis");
		Map("BugsSplusData6", "number of items not equal to size of node");
		Map("BugsSplusData7", "node dimension does not match");
		Map("BugsSplusData8", "no prior specified for this node");
		Map("BugsSplusData9", "no prior specified for this component of node");
		Map("BugsSplusData10", "this component of node is not stochastic");
		Map("BugsSplusData11", "this component of node is data");
		Map("BugsSplusData12", "expected a comma or right parenthesis");
		Map("BugsSplusData13", "number of items not equal to size of node");
		Map("BugsSplusData14", "expected a comma or right parenthesis");
		Map("BugsSplusData15", "node dimension does not match");
		Map("BugsSplusData16", "expected a period");
		Map("BugsSplusData17", "expected key word Dim");
		Map("BugsSplusData18", "expected an equals sign");
		Map("BugsSplusData19", "expected collection operator c");
		Map("BugsSplusData20", "expected left parenthesis");
		Map("BugsSplusData21", "expected an integer");
		Map("BugsSplusData22", "expected a comma or right parenthesis");
		Map("BugsSplusData23", "expected a comma");
		Map("BugsSplusData24", "expected right parenthesis");
		Map("BugsSplusData25", "scalar node must have size of one");
		Map("BugsSplusData26", "expected the collection operator c");
		Map("BugsSplusData27", "expected left parenthesis");
		Map("BugsSplusData28", "size of node not equal to number of components");
		Map("BugsSplusData29", "expected key word structure");
		Map("BugsSplusData30", "expected left parenthesis");
		Map("BugsSplusData31", "expected a period");
		Map("BugsSplusData32", "expected key word Data");
		Map("BugsSplusData33", "expected an equals sign");
		Map("BugsSplusData34", "expected the collection operator c");
		Map("BugsSplusData35", "expected left parenthesis");
		Map("BugsSplusData36", "expected a comma");
		Map("BugsSplusData37", "number of  items not equal to size of node");
		Map("BugsSplusData38", "expected right parenthesis");
		Map("BugsSplusData39", "expected variable name");
		Map("BugsSplusData40", "variables not in the model: ");
		Map("BugsSplusSata41", "expected an equals sign");
		Map("BugsSplusData42", "expected a comma or right parenthesis");
		Map("BugsSplusData43", "expected a comma");
		Map("BugsSplusData44", "expected key word list");
		Map("BugsSplusData45", "expected left parenthesis");
		Map("BugsSplusData46", "expected key word list");
		Map("BugsSplusData47", "expected left parenthesis");
		Map("BugsSplusData48", "expected number, NA, collection operator c or key word structure");

		Map("BugsVariables1", "loop index must be a name");
		Map("BugsVariables2", "missing closing square bracket ]");

		Map("BugsCmds:OkSyntax", "model is syntactically correct");
		Map("BugsCmds:OkData", "data loaded");
		Map("BugsCmds:OkCompile", "model compiled in ^0 s");
		Map("BugsCmds:UninitOther", "initial values loaded and chain initialized but another chain contains uninitialized variables");
		Map("BugsCmds:NotInit", "initial values loaded but chain contains uninitialized variables");
		Map("BugsCmds:OkInits", "model is initialized");
		Map("BugsCmds:OkGenInits", "initial values generated, model initialized");
		Map("BugsCmds:UpdatesTook", "^0 updates took ^1 s");
		Map("BugsCmds:Updating", "model is updating");

		Map("BugsCmds:NoFile", "file ^0 does not exist");
		Map("BugsCmds:NoCheckData", "model must be checked before data is loaded");
		Map("BugsCmds:NoCheckCompile", "model must be checked before compiling");
		Map("BugsCmdsNoCompileInits", "model must be compiled before initial values loaded");
		Map("BugsCmds:NoCompileGenInits", "model must be compiled before generating initial values");
		Map("BugsCmds:AlreadyInits", "model is already initialized");
		Map("BugsCmds:NotInits", "model must be initialized before updating");
		Map("BugsCmds:SeedZero", "seed of random number generator must not be zero");
		Map("BugsCmds.AlreadyCompiled", "this option can not be changed once the model is compiled");
		Map("BugsCmds:ChainsWrittenOut", "current values for chain(s) written");
		Map("BugsCmds:DataWritten", "data values written");
		Map("BugsCmds:UninitializedNdes", "unitialized nodes ");
		Map("BugsCmds:couldNotChangeUpdater", "could not change (all) updaters for ^0");
		Map("BugsCmds:UninitializedNodes", "all nodes in model are initialized");
		Map("BugsCmds:DataOut", "model data");
		
		Map("BugsEmbed:OkSyntax", "model is syntactically correct");
		Map("BugsEmbed:OkData", "data loaded");
		Map("BugsEmbed:OkCompile", "model compiled");
		Map("BugsEmbed:UninitOther", "initial values loaded and chain initialized but another chain contain uninitialized variables");
		Map("BugsEmbed:NotInit", "initial values loaded but chain contain uninitialized variables");
		Map("BugsEmbed:OkInits", "model is initialized");
		Map("BugsEmbed:OkGenInits", "initial values generated, model initialized");
		Map("BugsEmbed:UpdatesTook", "^0 updates took ^1 s");
		Map("BugsEmbed:Updating", "model is updating");

		Map("BugsEmbed:NoFile", "file ^0 does not exist");
		Map("BugsEmbed:NoCheckData", "model must be checked before data is loaded");
		Map("BugsEmbed:NoCheckCompile", "model must be checked before compiling");
		Map("BugsEmbed:NoCompileInits", "model must be compiled before initial values loaded");
		Map("BugsEmbed:NoCompileGenInits", "model must be compiled before generating initial values");
		Map("BugsEmbed:AlreadyInits", "model is already initialized");
		Map("BugsEmbed:NotInits", "model must be initialized before updating");
		Map("BugsEmbed:SeedZero", "seed of random number generator must not be zero");
		Map("BugsEmbed.AlreadyCompiled", "this option can not be changed once the model is compiled");
		Map("BugsEmbed:chainsWrittenOut", "chains written out");
		Map("BugsEmbed:couldNotChangeUpdater", "could not change (all) updaters for  ^0 ")
		
	END Load;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		Map := BugsMsg.Map
	END Init;

BEGIN
	Init
END BugsMessages.
