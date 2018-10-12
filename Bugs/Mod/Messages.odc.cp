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

		RegisterKey: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		RegisterKey("BugsCheck1", "array index less than one");
		RegisterKey("BugsCheck2", "array index greater than array upper bound for ^0");

		RegisterKey("BugsCmds:OkSyntax", "model is syntactically correct");
		RegisterKey("BugsCmds:OkData", "data loaded");
		RegisterKey("BugsCmds:OkCompile", "model compiled in ^0 s");
		RegisterKey("BugsCmds:UninitOther", "initial values loaded and chain initialized but another chain contains uninitialized variables");
		RegisterKey("BugsCmds:NotInit", "initial values loaded but chain contains uninitialized variables");
		RegisterKey("BugsCmds:OkInits", "model is initialized");
		RegisterKey("BugsCmds:OkGenInits", "initial values generated, model initialized");
		RegisterKey("BugsCmds:UpdatesTook", "^0 updates took ^1 s");
		RegisterKey("BugsCmds:Updating", "model is updating");
		RegisterKey("BugsCmds:ScriptFailed", "script failed in interpretation");
		RegisterKey("BugsCmds.CommandError", "script command ");
		RegisterKey("BugsCmds:NoFile", "file ^0 does not exist");
		RegisterKey("BugsCmds:NoCheckData", "model must be checked before data is loaded");
		RegisterKey("BugsCmds:NoCheckCompile", "model must be checked before compiling");
		RegisterKey("BugsCmdsNoCompileInits", "model must be compiled before initial values loaded");
		RegisterKey("BugsCmds:NoCompileGenInits", "model must be compiled before generating initial values");
		RegisterKey("BugsCmds:AlreadyInits", "model is already initialized");
		RegisterKey("BugsCmds:NotInits", "model must be initialized before updating");
		RegisterKey("BugsCmds:SeedZero", "seed of random number generator must not be zero");
		RegisterKey("BugsCmds.AlreadyCompiled", "this option can not be changed once the model is compiled");
		RegisterKey("BugsCmds:ChainsWrittenOut", "current values for chain(s) written");
		RegisterKey("BugsCmds:DataWritten", "data values written");
		RegisterKey("BugsCmds:UninitializedNdes", "unitialized nodes ");
		RegisterKey("BugsCmds:couldNotChangeUpdater", "could not change (all) updaters for ^0");
		RegisterKey("BugsCmds:UninitializedNodes", "all nodes in model are initialized");
		RegisterKey("BugsCmds:DataOut", "model data");
		RegisterKey("BugsCmds:NoDeviance", "no deviance for model");
		RegisterKey("BugsCmds:ModelDistibuted", "model distributed");
		RegisterKey("BugsCmds:ScripIsLog", "script must not be in the log window");

		RegisterKey("BugsCodegen1", "logical expression contains too many operators");
		RegisterKey("BugsCodegen2", "logical expression contains too many constants");
		RegisterKey("BugsCodegen3", "logical expression contains too many scalars");
		
		RegisterKey("BugsComponents1", "unable to distribute updater for ^0");

		RegisterKey("BugsEvaluate1", "integer divide by zero");
		RegisterKey("BugsEvaluate2", "variable ^0 is not defined");
		RegisterKey("BugsEvaluate3", "missing index");
		RegisterKey("BugsEvaluate4", "invalid use of range construct in ^0");
		RegisterKey("BugsEvaluate5", "array index greater than array upper bound for ^0");
		RegisterKey("BugsEvaluate6", "variable ^0 not data");
		RegisterKey("BugsEvaluate7", "variable ^0 not defined in index expression");
		RegisterKey("BugsEvaluate8", "array index is not an integer ^0");
		RegisterKey("BugsEvaluate9", "array index is greater than array upper bound for ^0");
		RegisterKey("BugsEvaluate10", "invalid range specified for  ^0");
		RegisterKey("BugsEvaluate11", "array index is less than one for ^0");
		RegisterKey("BugsEvaluate12", "array index is less than one for ^0");
		RegisterKey("BugsEvaluate13", "array index is greater than array upper bound for ^0");
		RegisterKey("BugsEvaluate14", "made use of undefined node ^0");
		RegisterKey("BugsEvaluate15", "array index is greater than array upper bound for ^0");
		RegisterKey("BugsEvaluate16", "invalid range specified for  ^0");
		RegisterKey("BugsEvaluate17", "array index is less than one for ^0");
		RegisterKey("BugsEvaluate18", "the vector quantity ^0 is not defined");
		RegisterKey("BugsEvaluate19", "the vector quantity ^0 has an undefined component");

		RegisterKey("BugsGraph1", "unable to create updater for ^0");
		
		RegisterKey("BugsInterface1", "unable to generate initial values for node ^0");

		RegisterKey("BugsMask1", "array index of ^0 is less than one");
		RegisterKey("BugsMask2", "array index of ^0 is greater than array upper bound");
		RegisterKey("BugsMask3", "invalid index range specified for ^0");
		RegisterKey("BugsMask4", "array index of ^0 greater than array upper bound");
		RegisterKey("BugsMask5", "variable name ^0 must begin with a character");
		RegisterKey("BugsMask6", "name ^0 does not occur in model");
		
		RegisterKey("BugsMaster.NoGraphFile", "unable to write graph file for worker");
		RegisterKey("BugsMaster.LinkingFailure", "unable to link BugsWorker executable");

		RegisterKey("BugsNodes1", "array index is greater than array upper bound for ^0");
		RegisterKey("BugsNodes2", "array index is greater than array upper bound for ^0");
		RegisterKey("BugsNodes3", "variable ^0 is not defined in model or in data set");
		RegisterKey("BugsNodes4", "vector-valued relation ^0 must involve consecutive elements of variable");
		RegisterKey("BugsNodes5", "multiple definitions of node ^0");
		RegisterKey("BugsNodes6", "vector-valued relation ^0 must involve consecutive elements of variable");
		RegisterKey("BugsNodes7", "multiple definitions of node ^0");
		RegisterKey("BugsNodes8", "expected multivariate node");
		RegisterKey("BugsNodes9", "unable to create stochastic node ^0 with these options");
		RegisterKey("BugsNodes10", "invalid arguments for  node ^0");
		RegisterKey("BugsNodes12", "vector-valued logical expression must have more than one component");
		RegisterKey("BugsNodes13", "scalar-valued logical expression cannot have more than one component");
		RegisterKey("BugsNodes14", "node ^0 with multivariate distribution must have more than one component");
		RegisterKey("BugsNodes15", "node ^0 with univariate distribution can not have more than one component");
		RegisterKey("BugsNodes16", "node has undefined argument");
		
		RegisterKey("BugsDistribute1", "unable to create updater for ^0");

		RegisterKey("BugsParser1", "unknown type of logical function");
		RegisterKey("BugsParser2", "link function cannot be used on right hand side");
		RegisterKey("BugsParser3", "expected left parenthesis");
		RegisterKey("BugsParser4", "unknown type of argument for logical function");
		RegisterKey("BugsParser5", "expected comma");
		RegisterKey("BugsParser6", "expected right parenthesis");
		RegisterKey("BugsParser7", "expected left parenthesis");
		RegisterKey("BugsParser8", "expected comma");
		RegisterKey("BugsParser9", "expected right parenthesis");
		RegisterKey("BugsParser10", "unexpected token in factor");
		RegisterKey("BugsParser11", "logical function not allowed in integer expression");
		RegisterKey("BugsParser12", "expected right parenthesis");
		RegisterKey("BugsParser13", "expected variable name");
		RegisterKey("BugsParser14", "expected comma");
		RegisterKey("BugsParser15", "expected right square bracket");
		RegisterKey("BugsParser16", "expected an integer");
		RegisterKey("BugsParser17", "expected a number");
		RegisterKey("BugsParser18", "invalid or unexpected token scanned");
		RegisterKey("BugsParser19", "unknown type of probability density");
		RegisterKey("BugsParser20", "expected left parenthesis");
		RegisterKey("BugsParser21", "expected variable name");
		RegisterKey("BugsParser22", "unknown type of argument in probability density");
		RegisterKey("BugsParser23", "expected a comma");
		RegisterKey("BugsParser24", "expected right parenthesis");
		RegisterKey("BugsParser25", "this density cannot be censored");
		RegisterKey("BugsParser26", "expected left parenthesis");
		RegisterKey("BugsParser27", "expected comma");
		RegisterKey("BugsParser28", "expected right parenthesis");
		RegisterKey("BugsParser29", "loop index must be a name");
		RegisterKey("BugsParser30", "loop index cannot be a name of a variable in the model");
		RegisterKey("BugsParser31", "loop name already used in outer loop");
		RegisterKey("BugsParser32", "expected left parenthesis");
		RegisterKey("BugsParser33", "expected the key word in");
		RegisterKey("BugsParser34", "expected a colon");
		RegisterKey("BugsParser35", "expected right parenthesis");
		RegisterKey("BugsParser36", "expected an open brace {");
		RegisterKey("BugsParser37", "empty slot not allowed in variable name");
		RegisterKey("BugsParser38", "expected left pointing arrow <  - ");
		RegisterKey("BugsParser39", "expected left pointing arrow <  - or twiddles ~");
		RegisterKey("BugsParser40", "unknown type of logical function");
		RegisterKey("BugsParser41", "function is not a link function");
		RegisterKey("BugsParser42", "expected right parenthesis");
		RegisterKey("BugsParser43", "expected left pointing arrow <  - ");
		RegisterKey("BugsParser44", "expected left pointing arrow <  - ");
		RegisterKey("BugsParser45", "invalid or unexpected token scanned");
		RegisterKey("BugsParser46", "this density cannot be truncated");
		RegisterKey("BugsParser47", "this density already censored");
		RegisterKey("BugsParser48", "this density already truncated");

		RegisterKey("BugsRectData1", "invalid or unexpected token scanned");
		RegisterKey("BugsRectData2", "NA cannot be given a sign");
		RegisterKey("BugsRectData3", "expected a number or an NA or END");
		RegisterKey("BugsRectData4", "expected a closing square bracket]");
		RegisterKey("BugsRectData5", "column label cannot be a scalar");
		RegisterKey("BugsRectData6", "first slot of column label must be empty");
		RegisterKey("BugsRectData7", "second and later slots of column label must not be empty");
		RegisterKey("BugsRectData8", "unable to evaluate column label index");
		RegisterKey("BugsRectData9", "column label index is greater than array upper bound");
		RegisterKey("BugsRectData10", "range operator not allowed in column labels");
		RegisterKey("BugsRectData11", "data value already given for this component");
		RegisterKey("BugsRectData12", "incomplete row of data");
		RegisterKey("BugsRectData13", "wrong number of rows of data");
		RegisterKey("BugsRectData14", "no prior specified for this initial value");
		RegisterKey("BugsRectData15", "initial value given for non stochastic node");
		RegisterKey("BugsRectData16", "initial value given for data node");
		RegisterKey("BugsRectData17", "incomplete row of initial values");
		RegisterKey("BugsRectData18", "expected a comma");
		RegisterKey("BugsRectData19", "variables not in the model: ");
		
		RegisterKey("BugsScript1", "non-matching brackets in command");
		RegisterKey("BugsScript2", "non-closed quote in command");
		RegisterKey("BugsScript3", "non-closed quote in command");
		RegisterKey("BugsScript4", "non-matching square brackets in command");
		RegisterKey("BugsScript5", "both single and double quotes in command");
		RegisterKey("BugsScript6", "unknown script command");

		RegisterKey("BugsSplusData1", "invalid or unexpected token scanned");
		RegisterKey("BugsSplusData2", "NA cannot be given a sign");
		RegisterKey("BugsSplusData3", "expected a number or an NA");
		RegisterKey("BugsSplusData4", "data value already given for this component of node");
		RegisterKey("BugsSplusData5", "expected a comma or right parenthesis");
		RegisterKey("BugsSplusData6", "number of items not equal to size of node");
		RegisterKey("BugsSplusData7", "node dimension does not match");
		RegisterKey("BugsSplusData8", "no prior specified for this node");
		RegisterKey("BugsSplusData9", "no prior specified for this component of node");
		RegisterKey("BugsSplusData10", "this component of node is not stochastic");
		RegisterKey("BugsSplusData11", "this component of node is data");
		RegisterKey("BugsSplusData12", "expected a comma or right parenthesis");
		RegisterKey("BugsSplusData13", "number of items not equal to size of node");
		RegisterKey("BugsSplusData14", "expected a comma or right parenthesis");
		RegisterKey("BugsSplusData15", "node dimension does not match");
		RegisterKey("BugsSplusData16", "expected a period");
		RegisterKey("BugsSplusData17", "expected key word Dim");
		RegisterKey("BugsSplusData18", "expected an equals sign");
		RegisterKey("BugsSplusData19", "expected collection operator c");
		RegisterKey("BugsSplusData20", "expected left parenthesis");
		RegisterKey("BugsSplusData21", "expected an integer");
		RegisterKey("BugsSplusData22", "expected a comma or right parenthesis");
		RegisterKey("BugsSplusData23", "expected a comma");
		RegisterKey("BugsSplusData24", "expected right parenthesis");
		RegisterKey("BugsSplusData25", "scalar node must have size of one");
		RegisterKey("BugsSplusData26", "expected the collection operator c");
		RegisterKey("BugsSplusData27", "expected left parenthesis");
		RegisterKey("BugsSplusData28", "size of node not equal to number of components");
		RegisterKey("BugsSplusData29", "expected key word structure");
		RegisterKey("BugsSplusData30", "expected left parenthesis");
		RegisterKey("BugsSplusData31", "expected a period");
		RegisterKey("BugsSplusData32", "expected key word Data");
		RegisterKey("BugsSplusData33", "expected an equals sign");
		RegisterKey("BugsSplusData34", "expected the collection operator c");
		RegisterKey("BugsSplusData35", "expected left parenthesis");
		RegisterKey("BugsSplusData36", "expected a comma");
		RegisterKey("BugsSplusData37", "number of  items not equal to size of node");
		RegisterKey("BugsSplusData38", "expected right parenthesis");
		RegisterKey("BugsSplusData39", "expected variable name");
		RegisterKey("BugsSplusData40", "variables not in the model: ");
		RegisterKey("BugsSplusSata41", "expected an equals sign");
		RegisterKey("BugsSplusData42", "expected a comma or right parenthesis");
		RegisterKey("BugsSplusData43", "expected a comma");
		RegisterKey("BugsSplusData44", "expected key word list");
		RegisterKey("BugsSplusData45", "expected left parenthesis");
		RegisterKey("BugsSplusData46", "expected key word list");
		RegisterKey("BugsSplusData47", "expected left parenthesis");
		RegisterKey("BugsSplusData48", "expected number, NA, collection operator c or key word structure");

		RegisterKey("BugsVariables1", "loop index must be a name");
		RegisterKey("BugsVariables2", "missing closing square bracket ]");
		
	END Load;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		RegisterKey := BugsMsg.RegisterKey
	END Init;

BEGIN
	Init
END BugsMessages.
