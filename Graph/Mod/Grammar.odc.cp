(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"


The parse tree is first processed to a stack based representation of logical relations. This stack based representation can then be futher processed to produce Component Pascal source code that can be compiled on the fly to create new logical node classes. This is implemented in module BugsCPWrite.

The data type 'Internal' contains the integer field 'key' used by the stack based representation of logical
relations and the string field 'function' used by the Component Pascal representation.

The data type 'Exterrnal' is used to both represent stochastic realations and special logical relations. The field
'density' distinguishes the two cases. The field 'name' is the BUGS language name of a distribution or special logical function. The field 'install' is the procedure which creates a factory, field 'fact',  to create this type of stochastic or logical node. The factory is is only created if it is needed.

*)

MODULE GraphGrammar;


	

	IMPORT
		GraphNodes;

		(*	internal functions and operators	*)
	CONST
		ref* = 0; const* = 1; refStoch* = 2; add* = 3; sub* = 4; mult* = 5; div* = 6; uminus* = 7;
		cloglog* = 10; cos* = 11; equals* = 12; exp* = 13; log* = 14; logfact* = 15; logit* = 16;
		loggam* = 17; max* = 18; min* = 19; phi* = 20; pow* = 21; sin* = 22; sqrt* = 23;
		step* = 24; abs* = 25; round* = 26; trunc* = 27; tan* = 28; arcsin* = 29; arccos* = 30;
		arctan* = 31; sinh* = 32; cosh* = 33; tanh* = 34; arcsinh* = 35; arccosh* = 36;
		arctanh* = 37; ilogit* = 38; icloglog* = 39;

		modifier* = 40; leftConst* = modifier; rightConst* = 2 * modifier;

		(*	external functions and distributions	*)
	TYPE
		External* = POINTER TO RECORD
			name-, install: ARRAY 128 OF CHAR;
			fact-: GraphNodes.Factory;
			density: BOOLEAN;
			next: External
		END;

		Internal* = POINTER TO RECORD
			name-: ARRAY 60 OF CHAR;
			key-, numPar-: INTEGER;
			next: Internal
		END;

	VAR
		externalsList: External;
		internalsList: Internal;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Find (IN name: ARRAY OF CHAR; density: BOOLEAN): External;
		VAR
			descriptor: External;
	BEGIN
		descriptor := externalsList;
		WHILE (descriptor # NIL) & ((descriptor.name # name) OR (descriptor.density # density)) DO
			descriptor := descriptor.next
		END;
		IF (descriptor # NIL) & (descriptor.fact = NIL) THEN
			descriptor.fact := GraphNodes.InstallFactory(descriptor.install);
			GraphNodes.SetFactory(NIL)
		END;
		RETURN descriptor
	END Find;

	PROCEDURE FindDensity* (IN name: ARRAY OF CHAR): External;
		CONST
			density = TRUE;
	BEGIN
		RETURN Find(name, density)
	END FindDensity;

	PROCEDURE FindFunction* (IN name: ARRAY OF CHAR): External;
		CONST
			density = FALSE;
	BEGIN
		RETURN Find(name, density)
	END FindFunction;

	PROCEDURE FindInstalled* (IN install: ARRAY OF CHAR): External;
		VAR
			descriptor: External;
	BEGIN
		descriptor := externalsList;
		WHILE (descriptor # NIL) & (descriptor.install # install) DO
			descriptor := descriptor.next
		END;
		RETURN descriptor
	END FindInstalled;

	PROCEDURE FindInternal* (IN name: ARRAY OF CHAR): Internal;
		VAR
			descriptor: Internal;
	BEGIN
		descriptor := internalsList;
		WHILE (descriptor # NIL) & (descriptor.name # name) DO
			descriptor := descriptor.next
		END;
		RETURN descriptor
	END FindInternal;

	PROCEDURE RegisterDensity* (IN name, install: ARRAY OF CHAR);
		VAR
			external: External;
	BEGIN
		NEW(external);
		external.fact := NIL;
		external.name := name$;
		external.install := install$;
		external.density := TRUE;
		external.next := externalsList;
		externalsList := external
	END RegisterDensity;

	PROCEDURE RegisterFunction* (IN name, install: ARRAY OF CHAR);
		VAR
			external: External;
	BEGIN
		NEW(external);
		external.fact := NIL;
		external.name := name$;
		external.install := install$;
		external.density := FALSE;
		external.next := externalsList;
		externalsList := external
	END RegisterFunction;

	PROCEDURE Insert (IN name: ARRAY OF CHAR; numPar, key: INTEGER);
		VAR
			internal: Internal;
	BEGIN
		NEW(internal);
		internal.name := name$;
		internal.numPar := numPar;
		internal.key := key;
		internal.next := internalsList;
		internalsList := internal
	END Insert;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		externalsList := NIL;
		internalsList := NIL;
		Insert("cloglog", 1, cloglog);
		Insert("cos", 1, cos);
		Insert("equals", 2, equals);
		Insert("exp", 1, exp);
		Insert("log", 1, log);
		Insert("logfact", 1, logfact);
		Insert("logit", 1, logit);
		Insert("loggam", 1, loggam);
		Insert("max", 2, max);
		Insert("min", 2, min);
		Insert("phi", 1, phi);
		Insert("pow", 2, pow);
		Insert("sin", 1, sin);
		Insert("sqrt", 1, sqrt);
		Insert("step", 1, step);
		Insert("abs", 1, abs);
		Insert("round", 1, round);
		Insert("trunc", 1, trunc);
		Insert("tan", 1, tan);
		Insert("arcsin", 1, arcsin);
		Insert("arccos", 1, arccos);
		Insert("arctan", 1, arctan);
		Insert("sinh", 1, sinh);
		Insert("cosh", 1, cosh);
		Insert("tanh", 1, tanh);
		Insert("arcsinh", 1, arcsinh);
		Insert("arccosh", 1, arccosh);
		Insert("arctanh", 1, arctanh);
		Insert("ilogit", 1, ilogit);
		Insert("icloglog", 1, icloglog);
	END Init;

BEGIN
	Init
END GraphGrammar.

