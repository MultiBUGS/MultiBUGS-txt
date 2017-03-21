(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE DeviancePluginS;


	

	IMPORT
		BugsIndex, BugsNames,
		DeviancePlugin,
		GraphChain, GraphDeviance, GraphLogical, GraphMultivariate, GraphNodes, GraphStochastic;

	TYPE
		Plugin = POINTER TO RECORD(DeviancePlugin.Plugin)
			terms: GraphStochastic.Vector;
			parents: GraphNodes.Vector
		END;

		Factory = POINTER TO RECORD(DeviancePlugin.Factory) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		fact-: DeviancePlugin.Factory;

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
			stoch: GraphStochastic.Node;
	BEGIN
		numPar := LEN(plugin.parents);
		i := 0;
		WHILE i < numPar DO
			stoch := plugin.parents[i](GraphStochastic.Node);
			stoch.SetValue(values[i]);
			INC(i)
		END
	END SetValues;

	PROCEDURE (plugin: Plugin) Type (): INTEGER;
	BEGIN
		RETURN 1
	END Type;

	PROCEDURE (plugin: Plugin) Values (): POINTER TO ARRAY OF REAL;
		VAR
			i, numPar: INTEGER;
			values: POINTER TO ARRAY OF REAL;
			stoch: GraphStochastic.Node;
	BEGIN
		numPar := LEN(plugin.parents);
		NEW(values, numPar);
		i := 0;
		WHILE i < numPar DO
			stoch := plugin.parents[i](GraphStochastic.Node);
			values[i] := stoch.value;
			INC(i)
		END;
		RETURN values
	END Values;

	PROCEDURE (f: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "DeviancePluginS.Install"
	END Install;

	PROCEDURE (f: Factory) New (): Plugin;
		VAR
			i, j, len, size: INTEGER;
			deviance: GraphNodes.Node;
			node: GraphStochastic.Node;
			list, cursor: GraphNodes.List;
			name: BugsNames.Name;
			all: BOOLEAN;
			plugin: Plugin;
	BEGIN
		NEW(plugin);
		name := BugsIndex.Find("deviance");
		ASSERT(name # NIL, 21);
		deviance := name.components[0](GraphLogical.Node);
		ASSERT(deviance # NIL, 22);
		all := TRUE;
		list := deviance.Parents(all);
		len := 0;
		cursor := list;
		WHILE cursor # NIL DO
			node := cursor.node(GraphStochastic.Node);
			IF node IS GraphChain.Node THEN
				size := 1
			ELSE
				size := node.Size()
			END;
			INC(len, size);
			cursor := cursor.next
		END;
		IF len > 0 THEN NEW(plugin.parents, len) END;
		i := 0;
		cursor := list;
		WHILE cursor # NIL DO
			node := cursor.node(GraphStochastic.Node);
			IF node IS GraphChain.Node THEN
				size := 1
			ELSE
				size := node.Size()
			END;
			WITH node: GraphMultivariate.Node DO
				j := 0;
				WHILE j < size DO
					plugin.parents[i] := node.components[j];
					INC(j);
					INC(i)
				END
			ELSE
				plugin.parents[i] := node;
				INC(i)
			END;
			cursor := cursor.next
		END;
		plugin.terms := GraphDeviance.DevianceTerms(deviance);
		RETURN plugin
	END New;

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
		fact := f
	END Init;

BEGIN
	Init
END DeviancePluginS.
