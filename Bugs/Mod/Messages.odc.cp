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

		StoreKey: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		StoreKey("BugsScript1", "non-matching brackets in command");
		StoreKey("BugsScript2", "non-closed quote in command");
		StoreKey("BugsScript3", "non-closed quote in command");
		StoreKey("BugsScript4", "non-matching square brackets in command");
		StoreKey("BugsScript5", "both single and double quotes in command");
		StoreKey("BugsScript6", "unknown script command");

		StoreKey("BugsCheck1", "array index less than one");
		StoreKey("BugsCheck2", "array index greater than array upper bound for ^0");

		StoreKey("BugsCodegen1", "logical expression contains too many constants");
		StoreKey("BugsCodegen2", "logical expression contains too many operators");
		StoreKey("BugsCodegen3", "logical expression contains too many operators");
		StoreKey("BugsCodegen4", "logical expression contains too many operators");
		StoreKey("BugsCodegen5", "logical expression contains too many operators");
		StoreKey("BugsCodegen6", "logical expression contains too many operators");
		StoreKey("BugsCodegen7", "logical expression contains too many operands");
		StoreKey("BugsCodegen8", "logical expression contains too many operators");
		StoreKey("BugsCodegen9", "logical expression contains too many operators");
		StoreKey("BugsCodegen10", "logical expression contains too many operands");
		StoreKey("BugsCodegen11", "logical expression contains too many operands");
		StoreKey("BugsCodegen12", "vector-valued logical expression must have more than one component");
		StoreKey("BugsCodegen13", "scalar-valued logical expression cannot have more than one component");
		StoreKey("BugsCodegen14", "multivariate distribution must have more than one component");
		StoreKey("BugsCodegen15", "univariate distribution can not have more than one component");

		StoreKey("BugsEvaluate1", "integer divide by zero");
		StoreKey("BugsEvaluate2", "variable ^0 is not defined");
		StoreKey("BugsEvaluate3", "missing index");
		StoreKey("BugsEvaluate4", "invalid use of range construct in ^0");
		StoreKey("BugsEvaluate5", "array index greater than array upper bound for ^0");
		StoreKey("BugsEvaluate6", "variable ^0 not data");
		StoreKey("BugsEvaluate7", "variable ^0 not defined in index expression");
		StoreKey("BugsEvaluate8", "array index is not an integer ^0");
		StoreKey("BugsEvaluate9", "array index is greater than array upper bound for ^0");
		StoreKey("BugsEvaluate10", "invalid range specified for  ^0");
		StoreKey("BugsEvaluate11", "array index is less than one for ^0");
		StoreKey("BugsEvaluate12", "array index is less than one for ^0");
		StoreKey("BugsEvaluate13", "array index is greater than array upper bound for ^0");
		StoreKey("BugsEvaluate14", "made use of undefined node ^0");
		StoreKey("BugsEvaluate15", "array index is greater than array upper bound for ^0");
		StoreKey("BugsEvaluate16", "invalid range specified for  ^0");
		StoreKey("BugsEvaluate17", "array index is less than one for ^0");
		StoreKey("BugsEvaluate18", "the vector quantity ^0 is not defined");
		StoreKey("BugsEvaluate19", "the vector quantity ^0 has an undefined component");

		StoreKey("BugsMask1", "array index of ^0 is less than one");
		StoreKey("BugsMask2", "array index of ^0 is greater than array upper bound");
		StoreKey("BugsMask3", "invalid index range specified for ^0");
		StoreKey("BugsMask4", "array index of ^0 greater than array upper bound");
		StoreKey("BugsMask5", "variable name ^0 must begin with a character");
		StoreKey("BugsMask6", "name ^0 does not occur in model");

		StoreKey("BugsGraph1", "unable to create updater for ^0");
		StoreKey("BugsGraph2", "invalid use of weight function for node ^0");
		
		StoreKey("BugsInterface1", "unable to generate initial values for node ^0");

		StoreKey("BugsNodes1", "array index is greater than array upper bound for ^0");
		StoreKey("BugsNodes2", "array index is greater than array upper bound for ^0");
		StoreKey("BugsNodes3", "variable ^0 is not defined in model or in data set");
		StoreKey("BugsNodes4", "vector-valued relation ^0 must involve consecutive elements of variable");
		StoreKey("BugsNodes5", "multiple definitions of node ^0");
		StoreKey("BugsNodes6", "vector-valued relation ^0 must involve consecutive elements of variable");
		StoreKey("BugsNodes7", "multiple definitions of node ^0");
		StoreKey("BugsNodes8", "expected multivariate node");
		StoreKey("BugsNodes9", "unable to create stochastic node ^0 with these options");
		StoreKey("BugsNodes10", "invalid arguments for  node ^0");
		
		StoreKey("BugsParallel1", "unable to create updater for ^0");

		StoreKey("BugsParser1", "unknown type of logical function");
		StoreKey("BugsParser2", "link function cannot be used on right hand side");
		StoreKey("BugsParser3", "expected left parenthesis");
		StoreKey("BugsParser4", "unknown type of argument for logical function");
		StoreKey("BugsParser5", "expected comma");
		StoreKey("BugsParser6", "expected right parenthesis");
		StoreKey("BugsParser7", "expected left parenthesis");
		StoreKey("BugsParser8", "expected comma");
		StoreKey("BugsParser9", "expected right parenthesis");
		StoreKey("BugsParser10", "unexpected token in factor");
		StoreKey("BugsParser11", "logical function not allowed in integer expression");
		StoreKey("BugsParser12", "expected right parenthesis");
		StoreKey("BugsParser13", "expected variable name");
		StoreKey("BugsParser14", "expected comma");
		StoreKey("BugsParser15", "expected right square bracket");
		StoreKey("BugsParser16", "expected an integer");
		StoreKey("BugsParser17", "expected a number");
		StoreKey("BugsParser18", "invalid or unexpected token scanned");
		StoreKey("BugsParser19", "unknown type of probability density");
		StoreKey("BugsParser20", "expected left parenthesis");
		StoreKey("BugsParser21", "expected variable name");
		StoreKey("BugsParser22", "unknown type of argument in probability density");
		StoreKey("BugsParser23", "expected a comma");
		StoreKey("BugsParser24", "expected right parenthesis");
		StoreKey("BugsParser25", "this density cannot be censored");
		StoreKey("BugsParser26", "expected left parenthesis");
		StoreKey("BugsParser27", "expected comma");
		StoreKey("BugsParser28", "expected right parenthesis");
		StoreKey("BugsParser29", "loop index must be a name");
		StoreKey("BugsParser30", "loop index cannot be a name of a variable in the model");
		StoreKey("BugsParser31", "loop name already used in outer loop");
		StoreKey("BugsParser32", "expected left parenthesis");
		StoreKey("BugsParser33", "expected the key word in");
		StoreKey("BugsParser34", "expected a colon");
		StoreKey("BugsParser35", "expected right parenthesis");
		StoreKey("BugsParser36", "expected an open brace {");
		StoreKey("BugsParser37", "empty slot not allowed in variable name");
		StoreKey("BugsParser38", "expected left pointing arrow <  - ");
		StoreKey("BugsParser39", "expected left pointing arrow <  - or twiddles ~");
		StoreKey("BugsParser40", "unknown type of logical function");
		StoreKey("BugsParser41", "function is not a link function");
		StoreKey("BugsParser42", "expected right parenthesis");
		StoreKey("BugsParser43", "expected left pointing arrow <  - ");
		StoreKey("BugsParser44", "expected left pointing arrow <  - ");
		StoreKey("BugsParser45", "invalid or unexpected token scanned");
		StoreKey("BugsParser46", "this density cannot be truncated");
		StoreKey("BugsParser47", "this density already censored");
		StoreKey("BugsParser48", "this density already truncated");

		StoreKey("BugsRectData1", "invalid or unexpected token scanned");
		StoreKey("BugsRectData2", "NA cannot be given a sign");
		StoreKey("BugsRectData3", "expected a number or an NA or END");
		StoreKey("BugsRectData4", "expected a closing square bracket]");
		StoreKey("BugsRectData5", "column label cannot be a scalar");
		StoreKey("BugsRectData6", "first slot of column label must be empty");
		StoreKey("BugsRectData7", "second and later slots of column label must not be empty");
		StoreKey("BugsRectData8", "unable to evaluate column label index");
		StoreKey("BugsRectData9", "column label index is greater than array upper bound");
		StoreKey("BugsRectData10", "range operator not allowed in column labels");
		StoreKey("BugsRectData11", "data value already given for this component");
		StoreKey("BugsRectData12", "incomplete row of data");
		StoreKey("BugsRectData13", "wrong number of rows of data");
		StoreKey("BugsRectData14", "no prior specified for this initial value");
		StoreKey("BugsRectData15", "initial value given for non stochastic node");
		StoreKey("BugsRectData16", "initial value given for data node");
		StoreKey("BugsRectData17", "incomplete row of initial values");
		StoreKey("BugsRectData18", "expected a comma");
		StoreKey("BugsRectData19", "variables not in the model: ");

		StoreKey("BugsSplusData1", "invalid or unexpected token scanned");
		StoreKey("BugsSplusData2", "NA cannot be given a sign");
		StoreKey("BugsSplusData3", "expected a number or an NA");
		StoreKey("BugsSplusData4", "data value already given for this component of node");
		StoreKey("BugsSplusData5", "expected a comma or right parenthesis");
		StoreKey("BugsSplusData6", "number of items not equal to size of node");
		StoreKey("BugsSplusData7", "node dimension does not match");
		StoreKey("BugsSplusData8", "no prior specified for this node");
		StoreKey("BugsSplusData9", "no prior specified for this component of node");
		StoreKey("BugsSplusData10", "this component of node is not stochastic");
		StoreKey("BugsSplusData11", "this component of node is data");
		StoreKey("BugsSplusData12", "expected a comma or right parenthesis");
		StoreKey("BugsSplusData13", "number of items not equal to size of node");
		StoreKey("BugsSplusData14", "expected a comma or right parenthesis");
		StoreKey("BugsSplusData15", "node dimension does not match");
		StoreKey("BugsSplusData16", "expected a period");
		StoreKey("BugsSplusData17", "expected key word Dim");
		StoreKey("BugsSplusData18", "expected an equals sign");
		StoreKey("BugsSplusData19", "expected collection operator c");
		StoreKey("BugsSplusData20", "expected left parenthesis");
		StoreKey("BugsSplusData21", "expected an integer");
		StoreKey("BugsSplusData22", "expected a comma or right parenthesis");
		StoreKey("BugsSplusData23", "expected a comma");
		StoreKey("BugsSplusData24", "expected right parenthesis");
		StoreKey("BugsSplusData25", "scalar node must have size of one");
		StoreKey("BugsSplusData26", "expected the collection operator c");
		StoreKey("BugsSplusData27", "expected left parenthesis");
		StoreKey("BugsSplusData28", "size of node not equal to number of components");
		StoreKey("BugsSplusData29", "expected key word structure");
		StoreKey("BugsSplusData30", "expected left parenthesis");
		StoreKey("BugsSplusData31", "expected a period");
		StoreKey("BugsSplusData32", "expected key word Data");
		StoreKey("BugsSplusData33", "expected an equals sign");
		StoreKey("BugsSplusData34", "expected the collection operator c");
		StoreKey("BugsSplusData35", "expected left parenthesis");
		StoreKey("BugsSplusData36", "expected a comma");
		StoreKey("BugsSplusData37", "number of  items not equal to size of node");
		StoreKey("BugsSplusData38", "expected right parenthesis");
		StoreKey("BugsSplusData39", "expected variable name");
		StoreKey("BugsSplusData40", "variables not in the model: ");
		StoreKey("BugsSplusSata41", "expected an equals sign");
		StoreKey("BugsSplusData42", "expected a comma or right parenthesis");
		StoreKey("BugsSplusData43", "expected a comma");
		StoreKey("BugsSplusData44", "expected key word list");
		StoreKey("BugsSplusData45", "expected left parenthesis");
		StoreKey("BugsSplusData46", "expected key word list");
		StoreKey("BugsSplusData47", "expected left parenthesis");
		StoreKey("BugsSplusData48", "expected number, NA, collection operator c or key word structure");

		StoreKey("BugsVariables1", "loop index must be a name");
		StoreKey("BugsVariables2", "missing closing square bracket ]");

		StoreKey("BugsCmds:OkSyntax", "model is syntactically correct");
		StoreKey("BugsCmds:OkData", "data loaded");
		StoreKey("BugsCmds:OkCompile", "model compiled in ^0 s");
		StoreKey("BugsCmds:UninitOther", "initial values loaded and chain initialized but another chain contains uninitialized variables");
		StoreKey("BugsCmds:NotInit", "initial values loaded but chain contains uninitialized variables");
		StoreKey("BugsCmds:OkInits", "model is initialized");
		StoreKey("BugsCmds:OkGenInits", "initial values generated, model initialized");
		StoreKey("BugsCmds:UpdatesTook", "^0 updates took ^1 s");
		StoreKey("BugsCmds:Updating", "model is updating");

		StoreKey("BugsCmds:NoFile", "file ^0 does not exist");
		StoreKey("BugsCmds:NoCheckData", "model must be checked before data is loaded");
		StoreKey("BugsCmds:NoCheckCompile", "model must be checked before compiling");
		StoreKey("BugsCmdsNoCompileInits", "model must be compiled before initial values loaded");
		StoreKey("BugsCmds:NoCompileGenInits", "model must be compiled before generating initial values");
		StoreKey("BugsCmds:AlreadyInits", "model is already initialized");
		StoreKey("BugsCmds:NotInits", "model must be initialized before updating");
		StoreKey("BugsCmds:SeedZero", "seed of random number generator must not be zero");
		StoreKey("BugsCmds.AlreadyCompiled", "this option can not be changed once the model is compiled");
		StoreKey("BugsCmds:ChainsWrittenOut", "current values for chain(s) written");
		StoreKey("BugsCmds:DataWritten", "data values written");
		StoreKey("BugsCmds:UninitializedNdes", "unitialized nodes ");
		StoreKey("BugsCmds:couldNotChangeUpdater", "could not change (all) updaters for ^0");
		StoreKey("BugsCmds:UninitializedNodes", "all nodes in model are initialized");
		StoreKey("BugsCmds:DataOut", "model data");
		StoreKey("BugsCmds:NoDeviance", "no deviance for model");
		
	END Load;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		StoreKey := BugsMsg.StoreKey
	END Init;

BEGIN
	Init
END BugsMessages.
