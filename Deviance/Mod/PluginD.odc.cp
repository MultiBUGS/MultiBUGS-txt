(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)


MODULE DeviancePluginD;

	

	IMPORT
		BugsIndex, BugsNames,
		DeviancePlugin,
		GraphDeviance, GraphLogical, GraphNodes, GraphParamtrans, GraphStochastic, GraphVector;

	TYPE
		Plugin = POINTER TO RECORD(DeviancePlugin.Plugin)
			terms: GraphStochastic.Vector;
			parents: GraphNodes.Vector
		END;

		Factory = POINTER TO RECORD(DeviancePlugin.Factory) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		fact-: Factory;

	PROCEDURE (plugin: Plugin) DevianceTerm (termOffset: INTEGER): REAL;
	BEGIN
		RETURN plugin.terms[termOffset].Deviance()
	END DevianceTerm;

	PROCEDURE (plugin: Plugin) IsValid (): BOOLEAN;
		VAR
			isValid: BOOLEAN;
			i, len: INTEGER;
	BEGIN
		IF plugin.parents # NIL THEN len := LEN(plugin.parents) ELSE len := 0 END;
		isValid := TRUE;
		WHILE (i < len) & isValid DO
			isValid := ~(GraphStochastic.integer IN plugin.parents[i].props);
			INC(i)
		END;
		RETURN isValid
	END IsValid;

	PROCEDURE (plugin: Plugin) NumParents (): INTEGER;
		VAR
			num: INTEGER;
	BEGIN
		IF plugin.parents # NIL THEN num := LEN(plugin.parents) ELSE num := 0 END;
		RETURN num
	END NumParents;

	PROCEDURE (plugin: Plugin) Parents (): GraphNodes.Vector;
	BEGIN
		RETURN plugin.parents
	END Parents;

	PROCEDURE (plugin: Plugin) SetValues (IN values: ARRAY OF REAL);
		VAR
			i, numPar: INTEGER;
			trans: GraphParamtrans.Transform;
			vectorTransform: GraphParamtrans.VectorTransform;
	BEGIN
		numPar := LEN(plugin.parents);
		i := 0;
		WHILE i < numPar DO
			IF plugin.parents[i] IS GraphParamtrans.Transform THEN
				trans := plugin.parents[i](GraphParamtrans.Transform);
				trans.SetAvgValue(values[i])
			ELSIF plugin.parents[i] IS GraphParamtrans.VectorTransform THEN
				vectorTransform := plugin.parents[i](GraphParamtrans.VectorTransform);
				vectorTransform.SetAvgValue(values[i])
			END;
			INC(i)
		END
	END SetValues;

	PROCEDURE (plugin: Plugin) Type (): INTEGER;
	BEGIN
		RETURN 0
	END Type;

	PROCEDURE (plugin: Plugin) Values (): POINTER TO ARRAY OF REAL;
		VAR
			i, numPar: INTEGER;
			values: POINTER TO ARRAY OF REAL;
	BEGIN
		numPar := LEN(plugin.parents);
		NEW(values, numPar);
		i := 0;
		WHILE i < numPar DO
			values[i] := 0.0;
			INC(i)
		END;
		RETURN values
	END Values;

	PROCEDURE (f: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "DeviancePluginD.Install"
	END Install;

	PROCEDURE (f: Factory) New (): Plugin;
		VAR
			i, j, len, size: INTEGER;
			deviance, modifiedDeviance, node: GraphNodes.Node;
			list, cursor: GraphNodes.List;
			name: BugsNames.Name;
			all: BOOLEAN;
			plugin: Plugin;
			vector: GraphVector.Node;
	BEGIN
		NEW(plugin);
		name := BugsIndex.Find("deviance");
		ASSERT(name # NIL, 21);
		deviance := name.components[0];
		ASSERT(deviance # NIL, 22);
		modifiedDeviance := GraphDeviance.ModifiedDeviance(deviance);
		all := TRUE;
		list := modifiedDeviance.Parents(all);
		len := 0;
		cursor := list;
		WHILE cursor # NIL DO
			node := cursor.node;
			size := node.Size();
			INC(len, size);
			cursor := cursor.next
		END;
		IF len > 0 THEN NEW(plugin.parents, len) END;
		i := 0;
		cursor := list;
		WHILE cursor # NIL DO
			node := cursor.node;
			IF node IS GraphVector.Node THEN
				vector := node(GraphVector.Node);
				size := vector.Size();
				j := 0;
				WHILE j < size DO
					plugin.parents[i + j] := vector.components[j];
					INC(j)
				END;
				INC(i, size)
			ELSE
				plugin.parents[i] := node;
				INC(i)
			END;
			cursor := cursor.next
		END;
		plugin.terms := GraphDeviance.DevianceTerms(modifiedDeviance);
		RETURN plugin
	END New;

	PROCEDURE SetFact* (f: Factory);
	BEGIN
		fact := f
	END SetFact;

	PROCEDURE Install*;
	BEGIN
		DeviancePlugin.SetFact(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f;
	END Init;

BEGIN
	Init
END DeviancePluginD.
