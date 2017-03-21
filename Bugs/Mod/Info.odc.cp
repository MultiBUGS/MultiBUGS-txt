(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsInfo;

	

	IMPORT
		Strings,
		BugsEvaluate, BugsIndex, BugsInterface, BugsMappers, BugsMsg, BugsNames, BugsParser,
		GraphLogical, GraphNodes, GraphStochastic,
		UpdaterActions, UpdaterMultivariate, UpdaterParallel, UpdaterUpdaters;

	TYPE
		CountData = POINTER TO RECORD(BugsNames.Visitor)
			count: INTEGER
		END;
		
		CountLogical = POINTER TO RECORD(BugsNames.Visitor)
			count: INTEGER
		END;

		CountObservation = POINTER TO RECORD(BugsNames.Visitor)
			count: INTEGER
		END;

		WriterStochastic = POINTER TO RECORD(BugsNames.Visitor)
			f: BugsMappers.Formatter;
			first: BOOLEAN
		END;

		WriterData = POINTER TO RECORD(BugsNames.Visitor)
			f: BugsMappers.Formatter;
			first: BOOLEAN
		END;

		WriterUninit = POINTER TO RECORD(BugsNames.Visitor)
			f: BugsMappers.Formatter;
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Types* (IN variable: ARRAY OF CHAR; VAR f: BugsMappers.Formatter);
		VAR
			i, index, pos, size: INTEGER;
			offsets: POINTER TO ARRAY OF INTEGER;
			string: ARRAY 128 OF CHAR;
			var: BugsParser.Variable;
			name: BugsNames.Name;
	BEGIN
		var := BugsParser.StringToVariable(variable);
		IF var = NIL THEN RETURN END;
		name := var.name;
		offsets := BugsEvaluate.Offsets(var);
		IF offsets = NIL THEN RETURN END;
		f.WriteTab;
		f.WriteTab;
		f.WriteString("Type");
		f.WriteLn;
		size := LEN(offsets);
		i := 0;
		WHILE i < size DO
			index := offsets[i];
			name.Indices(index, string);
			string := name.string + string;
			f.WriteTab; f.WriteString(string);
			f.WriteTab;
			IF name.components[index] # NIL THEN
				name.components[index].Install(string);
				Strings.Find(string, "Install", 0, pos);
				IF pos #  - 1 THEN
					Strings.Replace(string, pos, pos + 6, "Node")
				END;
				f.WriteString(string)
			ELSE
				f.WriteString("undefined")
			END;
			f.WriteLn;
			INC(i)
		END
	END Types;

	PROCEDURE FindGraphNode (p: GraphStochastic.Node; OUT string: ARRAY OF CHAR);
		VAR
			len: INTEGER;
			children: GraphStochastic.Vector;
	BEGIN
		BugsIndex.FindGraphNode(p, string);
		IF string[0] = "<" THEN
			len := LEN(string$);
			Strings.Extract(string, 1, len - 2, string)
		ELSIF p.likelihood # NIL THEN
			children := p.Children();
			p := children[0];
			BugsIndex.FindGraphNode(p, string);
			IF string[0] = "<" THEN
				len := LEN(string$);
				Strings.Extract(string, 1, len - 2, string)
			END;
			IF string[0] = "[" THEN
				string := "aux" + string
			ELSE
				string := "aux." + string
			END
		END
	END FindGraphNode;

	PROCEDURE WriteUpdaterNames* (updater: UpdaterUpdaters.Updater; VAR f: BugsMappers.Formatter);
		VAR
			k, size: INTEGER;
			p: GraphStochastic.Node;
			multiUpdater: UpdaterMultivariate.Updater;
			string: ARRAY 128 OF CHAR;
	BEGIN
		IF updater IS UpdaterMultivariate.Updater THEN
			multiUpdater := updater(UpdaterMultivariate.Updater);
			size := multiUpdater.Size();
			k := 1;
			WHILE k < size DO
				p := multiUpdater.Prior(k);
				FindGraphNode(p, string);
				f.WriteTab; f.WriteChar("("); f.WriteString(string); f.WriteChar(")"); f.WriteLn;
				INC(k)
			END
		END
	END WriteUpdaterNames;

	PROCEDURE Methods* (IN variable: ARRAY OF CHAR; VAR f: BugsMappers.Formatter);
		VAR
			depth, i, index, size: INTEGER;
			offsets: POINTER TO ARRAY OF INTEGER;
			string: ARRAY 80 OF CHAR;
			name: BugsNames.Name;
			var: BugsParser.Variable;
			updater: UpdaterUpdaters.Updater;
			p: GraphNodes.Node;
			stoch: GraphStochastic.Node;
		CONST
			chain = 0;
	BEGIN
		var := BugsParser.StringToVariable(variable);
		IF var = NIL THEN RETURN END;
		name := var.name;
		offsets := BugsEvaluate.Offsets(var);
		IF offsets = NIL THEN RETURN END;
		f.WriteTab;
		f.WriteTab;
		f.WriteString("Type");
		f.WriteLn;
		size := LEN(offsets);
		i := 0;
		WHILE i < size DO
			index := offsets[i];
			p := name.components[index];
			IF (p # NIL) & (p IS GraphStochastic.Node) & (GraphStochastic.update IN p.props) THEN
				stoch := p(GraphStochastic.Node);
				updater := UpdaterActions.FindSampler(chain, stoch);
				IF updater # NIL THEN
					size := updater.Size();
					depth := updater.Depth();
					name.Indices(i, string);
					string := name.string + string;
					f.WriteTab; f.WriteString(string);
					updater.Install(string);
					f.WriteString(string);
					f.WriteTab; f.WriteInt(size);
					f.WriteTab; f.WriteInt(depth);
					f.WriteLn;
					WriteUpdaterNames(updater, f)
				END
			END;
			INC(i)
		END
	END Methods;

	PROCEDURE Values* (IN variable: ARRAY OF CHAR; numChains: INTEGER;
	VAR f: BugsMappers.Formatter);
		CONST
			all = TRUE;
		VAR
			i, index, j, max, size: INTEGER;
			offsets: POINTER TO ARRAY OF INTEGER;
			string: ARRAY 80 OF CHAR;
			values: POINTER TO ARRAY OF ARRAY OF REAL;
			initialized: POINTER TO ARRAY OF ARRAY OF BOOLEAN;
			name: BugsNames.Name;
			components: GraphNodes.Vector;
			var: BugsParser.Variable;
			node: GraphNodes.Node;
			parents: GraphStochastic.List;
			init: BOOLEAN;
	BEGIN
		var := BugsParser.StringToVariable(variable);
		IF var = NIL THEN RETURN END;
		name := var.name;
		components := name.components;
		offsets := BugsEvaluate.Offsets(var);
		IF offsets = NIL THEN RETURN END;
		size := LEN(offsets);
		NEW(values, size, numChains);
		NEW(initialized, size, numChains);
		j := 0;
		WHILE j < numChains DO
			UpdaterActions.LoadSamples(j);
			BugsInterface.LoadDeviance(j);
			i := 0;
			WHILE i < size DO
				index := offsets[i];
				node := components[index];
				IF node # NIL THEN
					IF node IS GraphStochastic.Node THEN
						initialized[i, j] := {GraphStochastic.initialized, GraphNodes.data} * node.props # {};
					ELSIF node IS GraphLogical.Node THEN
						parents := GraphStochastic.Parents(node, all);
						init := TRUE;
						WHILE (parents # NIL) & init DO
							init := GraphStochastic.initialized IN parents.node.props;
							parents := parents.next
						END;
						initialized[i, j] := init
					ELSE
						initialized[i, j] := TRUE
					END;
					IF initialized[i, j] THEN values[i, j] := node.Value() END
				END;
				INC(i)
			END;
			INC(j)
		END;
		UpdaterActions.LoadSamples(0);
		BugsInterface.LoadDeviance(0);
		max := 2 + MIN(numChains, 10);
		f.WriteTab;
		i := 0;
		WHILE i < max - 2 DO
			f.WriteTab;
			f.WriteString("chain[");
			f.WriteInt(i + 1);
			f.WriteChar("]");
			INC(i)
		END;
		f.WriteLn;
		i := 0;
		WHILE i < size DO
			index := offsets[i];
			IF components[index] # NIL THEN
				f.WriteString(name.string);
				name.Indices(index, string);
				f.WriteString(string);
				f.WriteTab;
				j := 0;
				WHILE j < numChains DO
					f.WriteTab;
					IF components[index] # NIL THEN
						IF initialized[i, j] THEN
							IF ~(GraphNodes.data IN components[index].props) OR (j = 0) THEN
								f.WriteReal(values[i, j])
							END
						ELSE
							f.WriteString("NA")
						END;
					ELSE
						f.WriteString("NA")
					END;
					INC(j)
				END;
				f.WriteLn
			END;
			INC(i)
		END
	END Values;

	PROCEDURE WriteRealInt (VAR f: BugsMappers.Formatter; value: REAL);
		VAR
			int: INTEGER;
			absVal: REAL;
		CONST
			eps = 1.0E-6;
	BEGIN
		absVal := ABS(value);
		int := SHORT(ENTIER(value + eps));
		IF ABS(absVal - int) > eps THEN
			f.WriteReal(value)
		ELSE
			IF value < 0.0 THEN
				int :=  - int
			END;
			f.WriteInt(int)
		END
	END WriteRealInt;

	PROCEDURE CountDataNode (name: BugsNames.Name): INTEGER;
		VAR
			count, i, size: INTEGER;
			components: GraphNodes.Vector;
			p: GraphNodes.Node;
	BEGIN
		components := name.components;
		count := 0;
		i := 0;
		size := LEN(components);
		WHILE i < size DO
			p := components[i];
			IF (p # NIL) & (GraphNodes.data IN p.props) & ~(p IS GraphStochastic.Node) THEN
				INC(count)
			END;
			INC(i)
		END;
		RETURN count
	END CountDataNode;

	PROCEDURE CountLogicalNode (name: BugsNames.Name): INTEGER;
		VAR
			count, i, size: INTEGER;
			components: GraphNodes.Vector;
			p: GraphNodes.Node;
	BEGIN
		components := name.components;
		count := 0;
		i := 0;
		size := LEN(components);
		WHILE i < size DO
			p := components[i];
			IF (p # NIL) & (p IS GraphLogical.Node) & ~(GraphNodes.data IN p.props) THEN
				INC(count)
			END;
			INC(i)
		END;
		RETURN count
	END CountLogicalNode;

	PROCEDURE CountObservedNode (name: BugsNames.Name): INTEGER;
		VAR
			count, i, size: INTEGER;
			components: GraphNodes.Vector;
			p: GraphNodes.Node;
	BEGIN
		components := name.components;
		count := 0;
		i := 0;
		size := LEN(components);
		WHILE i < size DO
			p := components[i];
			IF (p # NIL) & (p IS GraphStochastic.Node) & (GraphNodes.data IN p.props) THEN
				INC(count)
			END;
			INC(i)
		END;
		RETURN count
	END CountObservedNode;

	PROCEDURE WriteNode (VAR f: BugsMappers.Formatter; name: BugsNames.Name; data: BOOLEAN);
		CONST
			maximumCols = 6;
		VAR
			counter, i, maxCols, numSlots, size, slot: INTEGER;
			value: REAL;
			node: GraphNodes.Node;
	BEGIN
		size := name.Size();
		numSlots := name.numSlots;
		IF (numSlots > 1) & (name.slotSizes[numSlots - 1] <= 2 * maximumCols) THEN
			maxCols := name.slotSizes[numSlots - 1]
		ELSE
			maxCols := maximumCols
		END;
		f.Bold;
		f.WriteString(name.string);
		f.Bold;
		CASE numSlots OF
		|0:
			f.WriteString(" = ")
		|1:
			f.WriteString(" = c("); f.WriteLn
		ELSE
			f.WriteString(" = structure(.Data = c(");
			f.WriteLn
		END;
		counter := 0;
		i := 0;
		WHILE i < size DO
			IF counter = 0 THEN f.WriteTab END;
			node := name.components[i];
			IF node = NIL THEN
				f.WriteString("NA")
			ELSIF data THEN
				IF (GraphNodes.data IN node.props) & ~(GraphLogical.logical IN node.props) THEN
					value := node.Value();
					IF (node IS GraphStochastic.Node) & ~(GraphStochastic.integer IN node.props) THEN
						f.WriteReal(value)
					ELSE
						WriteRealInt(f, value)
					END
				ELSE
					f.WriteString("NA")
				END
			ELSIF node IS GraphStochastic.Node THEN
				IF ~(GraphNodes.data IN node.props) & (GraphStochastic.initialized IN node.props) THEN
					value := node.Value();
					f.WriteReal(value)
				ELSE
					f.WriteString("NA")
				END
			ELSE
				f.WriteString("NA")
			END;
			INC(counter);
			IF i # size - 1 THEN
				f.WriteChar(",");
				IF counter # maxCols THEN f.WriteTab END
			END;
			IF (counter = maxCols) & (i # size - 1) THEN
				f.WriteLn;
				counter := 0
			END;
			INC(i)
		END;
		IF numSlots = 1 THEN
			f.WriteChar(")")
		ELSIF numSlots > 1 THEN
			f.WriteString("),");
			f.WriteLn;
			f.WriteString(".Dim = c(");
			slot := 0;
			WHILE slot < numSlots DO
				f.WriteInt(name.slotSizes[slot]);
				IF slot # numSlots - 1 THEN
					f.WriteChar(",")
				END;
				INC(slot)
			END;
			f.WriteString("))")
		END
	END WriteNode;

	PROCEDURE IsStochastic (name: BugsNames.Name): BOOLEAN;
		VAR
			isStochastic: BOOLEAN;
			i, size: INTEGER;
			node: GraphNodes.Node;
	BEGIN
		size := name.Size();
		i := 0;
		isStochastic := FALSE;
		WHILE (i < size) & ~isStochastic DO
			node := name.components[i];
			IF (node # NIL) & (node IS GraphStochastic.Node) THEN
				isStochastic := ({GraphNodes.data, GraphNodes.hidden} * node.props = {})
				 & (GraphStochastic.initialized IN node.props)
			END;
			INC(i)
		END;
		RETURN isStochastic
	END IsStochastic;

	PROCEDURE (v: WriterStochastic) Do (name: BugsNames.Name);
		VAR
			data: BOOLEAN;
	BEGIN
		IF IsStochastic(name) THEN
			IF ~v.first THEN v.f.WriteChar(",") END;
			v.first := FALSE;
			v.f.WriteLn;
			data := FALSE;
			WriteNode(v.f, name, data)
		END
	END Do;

	PROCEDURE (v: CountData) Do (name: BugsNames.Name);
		VAR
			count: INTEGER;
	BEGIN
		count := CountDataNode(name);
		INC(v.count, count)
	END Do;

	PROCEDURE (v: CountLogical) Do (name: BugsNames.Name);
		VAR
			count: INTEGER;
	BEGIN
		count := CountLogicalNode(name);
		INC(v.count, count)
	END Do;

	PROCEDURE (v: CountObservation) Do (name: BugsNames.Name);
		VAR
			count: INTEGER;
	BEGIN
		count := CountObservedNode(name);
		INC(v.count, count)
	END Do;

	PROCEDURE WriteChain* (chain: INTEGER; VAR f: BugsMappers.Formatter);
		VAR
			visitor: WriterStochastic;
	BEGIN
		UpdaterActions.LoadSamples(chain);
		NEW(visitor);
		visitor.f := f;
		visitor.first := TRUE;
		visitor.f.WriteString("list(");
		BugsIndex.Accept(visitor);
		visitor.f.WriteChar(")");
		visitor.f.WriteLn;
		f := visitor.f;
		UpdaterActions.LoadSamples(0)
	END WriteChain;

	PROCEDURE IsData (name: BugsNames.Name): BOOLEAN;
		VAR
			isData: BOOLEAN;
			i, size: INTEGER;
			node: GraphNodes.Node;
	BEGIN
		size := name.Size();
		i := 0;
		isData := FALSE;
		WHILE (i < size) & ~isData DO
			node := name.components[i];
			IF node # NIL THEN
				isData := (GraphNodes.data IN node.props) & ~(GraphLogical.logical IN node.props)
			END;
			INC(i)
		END;
		RETURN isData
	END IsData;

	PROCEDURE (v: WriterData) Do (name: BugsNames.Name);
		VAR
			data: BOOLEAN;
	BEGIN
		IF IsData(name) THEN
			IF ~v.first THEN v.f.WriteChar(",") END;
			v.first := FALSE;
			v.f.WriteLn;
			data := TRUE;
			WriteNode(v.f, name, data)
		END
	END Do;

	PROCEDURE CountDatum* (): INTEGER;
		VAR
			visitor: CountData;
	BEGIN
		NEW(visitor);
		visitor.count := 0;
		BugsIndex.Accept(visitor);
		RETURN visitor.count
	END CountDatum;

	PROCEDURE CountLogicals* (): INTEGER;
		VAR
			visitor: CountLogical;
	BEGIN
		NEW(visitor);
		visitor.count := 0;
		BugsIndex.Accept(visitor);
		RETURN visitor.count
	END CountLogicals;

	PROCEDURE CountObservations* (): INTEGER;
		VAR
			visitor: CountObservation;
	BEGIN
		NEW(visitor);
		visitor.count := 0;
		BugsIndex.Accept(visitor);
		RETURN visitor.count
	END CountObservations;

	PROCEDURE WriteData* (VAR f: BugsMappers.Formatter);
		VAR
			visitor: WriterData;
	BEGIN
		NEW(visitor);
		visitor.f := f;
		visitor.first := TRUE;
		visitor.f.WriteString("list(");
		BugsIndex.Accept(visitor);
		visitor.f.WriteChar(")");
		visitor.f.WriteLn;
		f := visitor.f;
	END WriteData;

	PROCEDURE (v: WriterUninit) Do (name: BugsNames.Name);
		VAR
			i, size: INTEGER;
			p: GraphNodes.Node;
			string: ARRAY 128 OF CHAR;
	BEGIN
		i := 0;
		size := name.Size();
		WHILE i < size DO
			p := name.components[i];
			IF (p # NIL) & (p IS GraphStochastic.Node) & 
				(GraphStochastic.update IN p.props) & ~(GraphStochastic.initialized IN p.props) THEN
				name.Indices(i, string);
				string := name.string + string;
				v.f.WriteString(string); v.f.WriteLn;
			END;
			INC(i)
		END
	END Do;

	PROCEDURE WriteUninitNodes* (chain: INTEGER; VAR f: BugsMappers.Formatter);
		VAR
			visitor: WriterUninit;
	BEGIN
		UpdaterActions.LoadSamples(chain);
		NEW(visitor);
		visitor.f := f;
		BugsIndex.Accept(visitor);
		f := visitor.f;
		UpdaterActions.LoadSamples(0)
	END WriteUninitNodes;

	PROCEDURE ModelMetrics* (VAR f: BugsMappers.Formatter);
		VAR
			numUpdater, numData, numObs, numChild, numLogical, numParam: INTEGER;
	BEGIN
		numObs := CountObservations();
		numData := CountDatum();
		numLogical := CountLogicals();
		numParam := UpdaterActions.NumParameters();
		numUpdater := UpdaterActions.NumberUpdaters();
		numChild := UpdaterActions.AverageNumChildren();
		f.WriteTab;
		f.WriteString("Number of constants: ");
		f.WriteInt(numData);
		f.WriteLn;
		f.WriteTab;
		f.WriteString("Number of observations: ");
		f.WriteInt(numObs);
		f.WriteLn;
		f.WriteTab;
		f.WriteString("Number of deterministic relations: ");
		f.WriteInt(numLogical);
		f.WriteLn;
		f.WriteTab;
		f.WriteString("Number of parameters: ");
		f.WriteInt(numParam);
		f.WriteLn;
		f.WriteTab;
		f.WriteString("Number of updater: ");
		f.WriteInt(numUpdater);
		f.WriteLn;
		f.WriteTab;
		f.WriteString("Average number of children per updater: ");
		f.WriteInt(numChild);
		f.WriteLn;
	END ModelMetrics;

	PROCEDURE UpdaterType (updater: UpdaterUpdaters.Updater; OUT type: ARRAY OF CHAR);
		VAR
			pos, len: INTEGER;
	BEGIN
		updater.Install(type);
		BugsMsg.MapMsg(type, type);
		Strings.Find(type, "Install", 0, pos);
		IF pos #  - 1 THEN
			Strings.Replace(type, pos, LEN("Install"), "Updater")
		END
	END UpdaterType;

	PROCEDURE UpdatersByName* (VAR f: BugsMappers.Formatter);
		VAR
			depth, factors, i, j, len, pos, size: INTEGER;
			name: BugsNames.Name;
			p: GraphNodes.Node;
			stoch: GraphStochastic.Node;
			children: GraphStochastic.Vector;
			updater: UpdaterUpdaters.Updater;
			string: ARRAY 128 OF CHAR;
		CONST
			chain = 0;
	BEGIN
		f.WriteLn;
		f.WriteTab;
		f.WriteTab;
		f.WriteString("Updater type");
		f.WriteTab;
		f.WriteString("Size");
		f.WriteTab;
		f.WriteString("Depth");
		f.WriteTab;
		f.WriteString("Childs");
		f.WriteLn;
		i := 0;
		LOOP
			name := BugsIndex.FindByNumber(i);
			IF name = NIL THEN EXIT END;
			j := 0;
			len := name.Size();
			WHILE j < len DO
				p := name.components[j];
				IF (p # NIL) & (p IS GraphStochastic.Node) & (GraphStochastic.update IN p.props) THEN
					stoch := p(GraphStochastic.Node);
					updater := UpdaterActions.FindSampler(chain, stoch);
					IF updater # NIL THEN
						size := updater.Size();
						depth := updater.Depth();
						children := updater.Children();
						IF children # NIL THEN factors := LEN(children) ELSE factors := 0 END;
						name.Indices(j, string);
						string := name.string + string;
						f.WriteTab; f.WriteString(string);
						UpdaterType(updater, string);
						f.WriteTab;
						BugsMsg.MapMsg(string, string);
						f.WriteString(string);
						f.WriteTab;
						f.WriteInt(size);
						f.WriteTab;
						f.WriteInt(depth);
						f.WriteTab;
						f.WriteInt(factors);
						f.WriteLn;
						WriteUpdaterNames(updater, f)
					END
				END;
				INC(j)
			END;
			INC(i);
		END
	END UpdatersByName;

	PROCEDURE UpdatersByDepth* (VAR f: BugsMappers.Formatter);
		VAR
			depth, i, factors, size, numUpdaters: INTEGER;
			p: GraphStochastic.Node;
			children: GraphStochastic.Vector;
			updater: UpdaterUpdaters.Updater;
			string: ARRAY 128 OF CHAR;
		CONST
			chain = 0;
	BEGIN
		f.WriteTab;
		f.WriteTab;
		f.WriteString("Updater type");
		f.WriteTab;
		f.WriteString("Size");
		f.WriteTab;
		f.WriteString("Depth");
		f.WriteTab;
		f.WriteString("Childs");
		f.WriteLn;
		numUpdaters := UpdaterActions.NumberUpdaters();
		i := 0;
		WHILE i < numUpdaters DO
			updater := UpdaterActions.GetUpdater(chain, i);
			IF updater # NIL THEN
				size := updater.Size();
				depth := updater.Depth();
				children := updater.Children();
				IF children # NIL THEN factors := LEN(children) ELSE factors := 0 END;
				p := updater.Prior(0);
				FindGraphNode(p, string);
				f.WriteTab;
				f.WriteString(string);
				f.WriteTab;
				UpdaterType(updater, string);
				f.WriteString(string);
				f.WriteTab;
				f.WriteInt(size);
				f.WriteTab;
				f.WriteInt(depth);
				f.WriteTab;
				f.WriteInt(factors);
				f.WriteLn;
				WriteUpdaterNames(updater, f)
			ELSE
				f.WriteString(" dummy updater");
				f.WriteLn
			END;
			INC(i)
		END
	END UpdatersByDepth;

	PROCEDURE Distribute* (numProc: INTEGER; VAR f: BugsMappers.Formatter);
		VAR
			i, j, index, numUpdaters, size, space: INTEGER;
			updaters: POINTER TO ARRAY OF UpdaterUpdaters.Vector;
			u: UpdaterUpdaters.Updater;
			fact: UpdaterUpdaters.Factory;
			p: GraphStochastic.Node;
			install, string: ARRAY 128 OF CHAR;
			id: POINTER TO ARRAY OF INTEGER;
		CONST
			chain = 0;
	BEGIN
		UpdaterParallel.DistributeUpdaters(numProc, chain, updaters, id);
		f.WriteTab;
		f.WriteString("Number of processor: ");
		f.WriteInt(numProc);
		f.WriteLn;
		f.WriteLn;
		f.WriteTab;
		i := 0;
		WHILE i < numProc DO
			f.WriteString("#");
			f.WriteInt(i + 1);
			f.WriteTab;
			INC(i)
		END;
		f.WriteString("row");
		f.WriteTab;
		f.WriteLn;
		numUpdaters := LEN(updaters[0]);
		j := 0;
		WHILE j < numUpdaters DO
			index := 0;
			u := updaters[0, j];
			size := u.Size();
			WHILE index < size DO
				i := 0;
				WHILE i < numProc DO
					f.WriteTab;
					u := updaters[i, j];
					p := u.Prior(index);
					IF index # 0 THEN f.WriteChar("(") END;
					IF p # NIL THEN
						FindGraphNode(p, string);
						f.WriteString(string)
					ELSE
						f.WriteString("dummy")
					END;
					IF index # 0 THEN f.WriteChar(")") END;
					INC(i)
				END;
				IF index = 0 THEN
					f.WriteTab;
					IF id[j] # 0 THEN f.WriteInt(ABS(id[j])) END;
					IF id[j] < 0 THEN f.WriteChar("*") END;
					IF id[j] = 0 THEN f.WriteChar("-") END;
					f.WriteTab;
				END;
				f.WriteLn;
				INC(index)
			END;
			INC(j)
		END;
		UpdaterActions.UnMarkDistributed
	END Distribute;

	PROCEDURE DistributeDeviance* (numProc: INTEGER; VAR f: BugsMappers.Formatter);
		VAR
			observations: POINTER TO ARRAY OF GraphStochastic.Vector;
			updaters: POINTER TO ARRAY OF UpdaterUpdaters.Vector;
			id: POINTER TO ARRAY OF INTEGER;
			i, j, maxNum, num: INTEGER;
			p: GraphStochastic.Node;
			string: ARRAY 128 OF CHAR;
		CONST
			chain = 0;
	BEGIN
		UpdaterParallel.DistributeUpdaters(numProc, chain, updaters, id);
		UpdaterParallel.DistributeObservations(updaters, id, observations); 
		f.WriteTab;
		f.WriteString("Number of processor: ");
		f.WriteInt(numProc);
		f.WriteLn;
		f.WriteLn;
		f.WriteTab;
		i := 0;
		maxNum := 0;
		WHILE i < numProc DO
			IF observations[i] # NIL THEN maxNum := MAX(maxNum, LEN(observations[i])) END;
			f.WriteString("#");
			f.WriteInt(i + 1);
			f.WriteTab;
			INC(i)
		END;
		f.WriteLn;
		j := 0;
		WHILE j < maxNum DO
			i := 0;
			f.WriteTab;
			WHILE i < numProc DO
				IF observations[i] # NIL THEN num := LEN(observations[i]) ELSE num := 0 END;
				IF j < num THEN
					p := observations[i, j];
					FindGraphNode(p, string);
					f.WriteString(string);
				ELSE
					f.WriteString("-")
				END;
				f.WriteTab;
				INC(i)
			END;
			f.WriteLn;
			INC(j)
		END;
		UpdaterActions.UnMarkDistributed
	END DistributeDeviance;

	PROCEDURE DistributeInfo* (VAR f: BugsMappers.Formatter);
		VAR
			worker, numWorker, memory, size, time, i, num, pos: INTEGER;
			hook: BugsInterface.DistributeHook;
			string: ARRAY 128 OF CHAR;
	BEGIN
		IF BugsInterface.IsDistributed() THEN
			hook := BugsInterface.hook;
			f.WriteTab; f.WriteString("Time taken to write model file ");
			time := hook.writeTime;
			f.WriteInt(time DIV 1000); f.WriteString("."); f.WriteInt(time MOD 1000); f.WriteString("s");
			f.WriteLn;
			f.WriteTab; f.WriteString("Size of  model file ");
			size := hook.fileSize;
			IF size DIV 1000000 > 0 THEN
				f.WriteInt(size DIV 1000000); f.WriteString(" ")
			END;
			size := size MOD 1000000;
			IF size DIV 1000 < 10 THEN f.WriteString("0") END;
			IF size DIV 1000 < 100 THEN f.WriteString("0") END;
			f.WriteInt(size DIV 1000); f.WriteString(" ");
			IF size MOD 1000 < 10 THEN f.WriteString("0") END;
			IF size MOD 1000 < 100 THEN f.WriteString("0") END;
			f.WriteInt(size MOD 1000); f.WriteString(" bytes");
			f.WriteLn;
			f.WriteTab; f.WriteString("Time taken to link worker program ");
			time := hook.linkTime;
			f.WriteInt(time DIV 1000); f.WriteString("."); f.WriteInt(time MOD 1000); f.WriteString("s");
			f.WriteLn;
			f.WriteTab; f.WriteString("Memory allocated in OpenBUGS ");
			memory := hook.masterMemory;
			IF memory DIV 1000000 > 0 THEN
				f.WriteInt(memory DIV 1000000); f.WriteString(" ")
			END;
			memory := memory MOD 1000000;
			IF memory DIV 1000 < 10 THEN f.WriteString("0") END;
			IF memory DIV 1000 < 100 THEN f.WriteString("0") END;
			f.WriteInt(memory DIV 1000); f.WriteString(" ");
			IF memory MOD 1000 < 10 THEN f.WriteString("0") END;
			IF memory MOD 1000 < 100 THEN f.WriteString("0") END;
			f.WriteInt(memory MOD 1000); f.WriteString(" bytes");
			f.WriteLn;
			f.WriteLn;
			f.WriteString("Workers");
			f.WriteLn;
			f.WriteLn;
			f.WriteTab;
			f.WriteTab; f.WriteString("Rank");
			f.WriteTab; f.WriteString("Set up time");
			f.WriteTab; f.WriteString("Memory");
			f.WriteLn;
			worker := 0;
			numWorker := hook.numWorker;
			WHILE worker < numWorker DO
				f.WriteTab;
				f.WriteTab; f.WriteInt(hook.rank[worker]);
				f.WriteTab; f.WriteInt(hook.setupTime[worker] DIV 1000);
				f.WriteString("."); f.WriteInt(hook.setupTime[worker] MOD 1000); f.WriteString("s");
				memory := hook.memory[worker];
				f.WriteTab;
				IF memory DIV 1000000 > 0 THEN
					f.WriteInt(memory DIV 1000000); f.WriteString(" ")
				END;
				memory := memory MOD 1000000;
				IF memory DIV 1000 < 10 THEN f.WriteString("0") END;
				IF memory DIV 1000 < 100 THEN f.WriteString("0") END;
				f.WriteInt(memory DIV 1000); f.WriteString(" ");
				IF memory MOD 1000 < 10 THEN f.WriteString("0") END;
				IF memory MOD 1000 < 100 THEN f.WriteString("0") END;
				f.WriteInt(memory MOD 1000); f.WriteString(" bytes");
				f.WriteLn;
				INC(worker)
			END;
			IF hook.modules # NIL THEN
				f.WriteLn;
				f.WriteString("Linked modules in BUGS worker:");
				f.WriteLn;
				i := 0;
				num := LEN(hook.modules);
				WHILE i < num DO
					string := hook.modules[i]$;
					Strings.Find(string, "Dynamic", 0, pos);
					IF pos = 0 THEN
						Strings.Find(string, "_", 0, pos);
						IF pos # - 1 THEN string[pos] := 0X END
					END;
					IF i MOD 4 = 0 THEN f.WriteTab END;
					f.WriteTab; f.WriteString(string);
					INC(i);
					IF i MOD 4 = 0 THEN f.WriteLn END
				END;
				IF num DIV 4 # 0 THEN f.WriteLn END
			END
		END
	END DistributeInfo;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END BugsInfo.
