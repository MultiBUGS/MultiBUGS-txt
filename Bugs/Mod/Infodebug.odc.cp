(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsInfodebug;

	

	IMPORT
		SYSTEM,
		Meta, Ports, Strings, Views,
		BugsCmds, BugsEvaluate, BugsIndex, BugsInfo, BugsMappers, BugsMsg, BugsNames,
		BugsParser, BugsTexts,
		GraphGrammar, GraphNodes, GraphStochastic,
		TextModels,
		UpdaterActions, UpdaterParallel, UpdaterUpdaters;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE MethodsState (IN variable: ARRAY OF CHAR; VAR f: BugsMappers.Formatter);
		VAR
			adr, chain, depth, i, index, len, numChains, size: INTEGER;
			offsets: POINTER TO ARRAY OF INTEGER;
			string: ARRAY 128 OF CHAR;
			name: BugsNames.Name;
			var: BugsParser.Variable;
			updater: UpdaterUpdaters.Updater;
			p: GraphNodes.Node;
			stoch: GraphStochastic.Node;
			item, item0: Meta.Item;
			v: Views.View;
			ok: BOOLEAN;
			heapRefView: RECORD(Meta.Value)
				Do: PROCEDURE (adr: INTEGER; name: ARRAY OF CHAR): Views.View
			END;
	BEGIN
		numChains := UpdaterActions.NumberChains();
		var := BugsParser.StringToVariable(variable);
		IF var = NIL THEN RETURN END;
		name := var.name;
		offsets := BugsEvaluate.Offsets(var);
		IF offsets = NIL THEN RETURN END;
		Meta.Lookup("DevDebug", item);
		ok := item.obj = Meta.modObj;
		IF ok THEN
			item.Lookup("HeapRefView", item0);
			IF item0.obj = Meta.procObj THEN
				item0.GetVal(heapRefView, ok)
			END
		END;
		f.WriteTab;
		f.WriteTab; f.WriteString("Type");
		f.WriteTab; f.WriteString("Size");
		f.WriteTab; f.WriteString("Depth");
		chain := 0;
		WHILE chain < numChains DO
			f.WriteTab;
			f.WriteString("chain[");
			f.WriteInt(chain + 1);
			f.WriteChar("]");
			INC(chain)
		END;
		f.WriteLn;
		len := LEN(offsets);
		i := 0;
		WHILE i < len DO
			index := offsets[i];
			p := name.components[index];
			IF (p # NIL) & (p IS GraphStochastic.Node) & (GraphStochastic.update IN p.props) THEN
				stoch := p(GraphStochastic.Node);
				updater := UpdaterActions.FindSampler(0, stoch);
				IF updater # NIL THEN
					size := updater.Size();
					depth := updater.Depth();
					name.Indices(index, string);
					string := name.string + string;
					f.WriteTab;
					f.WriteString(string);
					f.WriteTab;
					updater.Install(string);
					BugsMsg.MapMsg(string, string);
					f.WriteString(string);
					f.WriteTab;
					f.WriteInt(size);
					f.WriteTab;
					f.WriteInt(depth);
					chain := 0;
					WHILE chain < numChains DO
						updater := UpdaterActions.FindSampler(chain, stoch);
						adr := SYSTEM.VAL(INTEGER, updater);
						IF ok THEN
							v := heapRefView.Do(adr, string);
							f.WriteTab;
							f.WriteView(v, 0, 0)
						END;
						INC(chain)
					END;
					f.WriteLn;
					BugsInfo.WriteUpdaterNames(updater, f)
				END
			END;
			INC(i)
		END
	END MethodsState;

	PROCEDURE NodesState (IN variable: ARRAY OF CHAR; VAR f: BugsMappers.Formatter);
		VAR
			adr, i, index, size, pos: INTEGER;
			offsets: POINTER TO ARRAY OF INTEGER;
			string: ARRAY 128 OF CHAR;
			var: BugsParser.Variable;
			name: BugsNames.Name;
			p: GraphNodes.Node;
			v: Views.View;
			item, item0: Meta.Item;
			ok: BOOLEAN;
			heapRefView: RECORD(Meta.Value)
				Do: PROCEDURE (adr: INTEGER; name: ARRAY OF CHAR): Views.View
			END;
			descriptor: GraphGrammar.External;
	BEGIN
		var := BugsParser.StringToVariable(variable);
		IF var = NIL THEN RETURN END;
		name := var.name;
		offsets := BugsEvaluate.Offsets(var);
		IF offsets = NIL THEN RETURN END;
		Meta.Lookup("DevDebug", item);
		ok := item.obj = Meta.modObj;
		IF ok THEN
			item.Lookup("HeapRefView", item0);
			IF item0.obj = Meta.procObj THEN
				item0.GetVal(heapRefView, ok)
			END
		END;
		f.WriteTab;
		f.WriteTab; f.WriteString("Type");
		f.WriteLn;
		size := LEN(offsets);
		i := 0;
		WHILE i < size DO
			index := offsets[i];
			name.Indices(index, string);
			string := name.string + string;
			f.WriteTab; f.WriteString(string);
			f.WriteTab;
			p := name.components[index];
			IF p # NIL THEN
				p.Install(string);
				descriptor := GraphGrammar.FindInstalled(string);
				IF descriptor # NIL THEN
					 string := descriptor.name$ 
				ELSE
					Strings.Find(string, "_", 0, pos);
					IF pos # -1 THEN 
						string[pos] := 0X
					ELSE
						Strings.Find(string, "GraphConstant", 0, pos);
						IF pos # -1 THEN string := "const" END
					END
				END;
				f.WriteString(string);
				adr := SYSTEM.VAL(INTEGER, p);
				IF ok THEN
					v := heapRefView.Do(adr, string);
					f.WriteTab;
					f.WriteView(v, 0, 0)
				END
			ELSE
				f.WriteString("undefined")
			END;
			f.WriteLn;
			INC(i)
		END
	END NodesState;

	PROCEDURE Methods*;
		VAR
			tabs: POINTER TO ARRAY OF INTEGER;
			f: BugsMappers.Formatter;
			text: TextModels.Model;
			chains, numChains: INTEGER;
	BEGIN
		text := TextModels.dir.New();
		BugsTexts.ConnectFormatter(f, text);
		f.SetPos(0);
		numChains := UpdaterActions.NumberChains();
		NEW(tabs, 5 + numChains);
		tabs[0] := 0;
		tabs[1] := 25 * Ports.mm;
		tabs[2] := 75 * Ports.mm;
		tabs[3] := 85 * Ports.mm;
		tabs[4] := 95 * Ports.mm;
		chains := 0;
		WHILE chains < numChains DO
			tabs[5 + chains] := tabs[4] + (1 + chains) * 15 * Ports.mm;
			INC(chains)
		END;
		f.WriteRuler(tabs);
		MethodsState(BugsCmds.infoDialog.node.item, f);
		IF f.lines > 1 THEN
			f.Register("Updater types")
		END
	END Methods;

	PROCEDURE Types*;
		VAR
			tabs: ARRAY 4 OF INTEGER;
			f: BugsMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		text := TextModels.dir.New();
		BugsTexts.ConnectFormatter(f, text);
		f.SetPos(0);
		tabs[0] := 0;
		tabs[1] := 25 * Ports.mm;
		tabs[2] := 75 * Ports.mm;
		tabs[3] := 80 * Ports.mm;
		f.WriteRuler(tabs);
		NodesState(BugsCmds.infoDialog.node.item, f);
		IF f.lines > 1 THEN
			f.Register("Node types")
		END
	END Types;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END BugsInfodebug.

