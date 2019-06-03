(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsInfo;

	

	IMPORT
		Strings,
		TextMappers, TextModels,
		BugsComponents, BugsEvaluate, BugsFiles, BugsIndex, BugsInterface, BugsMsg,
		BugsNames, BugsParser, BugsPartition,
		GraphLogical, GraphNodes, GraphStochastic,
		UpdaterActions, UpdaterAuxillary, UpdaterMultivariate, UpdaterUpdaters;

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
			f: TextMappers.Formatter;
			first: BOOLEAN
		END;

		WriterData = POINTER TO RECORD(BugsNames.Visitor)
			f: TextMappers.Formatter;
			first: BOOLEAN
		END;

		WriterUninit = POINTER TO RECORD(BugsNames.Visitor)
			f: TextMappers.Formatter;
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	CONST
		bold = 700;
		undefined = MAX(INTEGER);

	PROCEDURE WriteReal (x: REAL; VAR f: TextMappers.Formatter);
	BEGIN
		f.WriteRealForm(x, BugsFiles.prec, 0, 0, TextModels.digitspace)
	END WriteReal;

	PROCEDURE WriteRealInt (x: REAL; VAR f: TextMappers.Formatter);
		VAR
			int: INTEGER;
			absVal: REAL;
		CONST
			eps = 1.0E-6;
	BEGIN
		absVal := ABS(x);
		int := SHORT(ENTIER(x + eps));
		IF ABS(absVal - int) > eps THEN
			WriteReal(x, f)
		ELSE
			IF x < 0.0 THEN
				int := - int
			END;
			f.WriteInt(int)
		END
	END WriteRealInt;

	PROCEDURE FindGraphNode (p: GraphStochastic.Node; OUT string: ARRAY OF CHAR);
		VAR
			len: INTEGER;
	BEGIN
		BugsIndex.FindGraphNode(p, string);
		len := LEN(string$);
		Strings.Extract(string, 1, len - 2, string)
	END FindGraphNode;

	PROCEDURE MapValue (updater: UpdaterUpdaters.Updater; VAR f: TextMappers.Formatter);
		VAR
			i, size: INTEGER;
			mapValues, values: POINTER TO ARRAY OF REAL;
			p: GraphStochastic.Node;
			name: ARRAY 128 OF CHAR;
	BEGIN
		size := updater.Size();
		NEW(values, size);
		NEW(mapValues, size);
		i := 0;
		WHILE i < size DO
			p := updater.Prior(i);
			values[i] := p.value;
			IF ~(GraphStochastic.integer IN p.props) THEN
				mapValues[i] := p.Map()
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < size DO
			p := updater.Prior(i);
			IF ~(GraphStochastic.integer IN p.props) THEN
				p.InvMap(mapValues[i])
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < size DO
			p := updater.Prior(i);
			FindGraphNode(p, name);
			f.WriteTab; f.WriteString(name); f.WriteTab;
			WriteRealInt(values[i], f); f.WriteTab;
			IF ~(GraphStochastic.integer IN p.props) THEN
				WriteReal(mapValues[i], f); f.WriteTab;
				WriteRealInt(p.value, f); f.WriteTab
			ELSE
				f.WriteString("-"); f.WriteTab;
				f.WriteString("-"); f.WriteTab
			END;
			f.WriteLn;
			INC(i)
		END
	END MapValue;

	PROCEDURE WriteUpdaterNames* (updater: UpdaterUpdaters.Updater; VAR f: TextMappers.Formatter);
		VAR
			k, size: INTEGER;
			p: GraphStochastic.Node;
			multiUpdater: UpdaterMultivariate.Updater;
			string: ARRAY 128 OF CHAR;
	BEGIN
		IF (updater IS UpdaterMultivariate.Updater) & ~(updater IS UpdaterAuxillary.UpdaterMV) THEN
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

	PROCEDURE IsConstant* (name: BugsNames.Name;offsets: POINTER TO ARRAY OF INTEGER): BOOLEAN;
		VAR
			i, index, size: INTEGER;
			components: GraphNodes.Vector;
			node: GraphNodes.Node;
			isConstant: BOOLEAN;
	BEGIN
		IF ~name.passByreference THEN RETURN TRUE END;
		components := name.components;
		size := LEN(offsets);
		i := 0;
		isConstant := TRUE;
		WHILE isConstant & (i < size) DO
			index := offsets[i];
			node := components[index];
			isConstant := (node # NIL) & (GraphNodes.data IN node.props);
			INC(i)
		END;
		RETURN isConstant
	END IsConstant;

	PROCEDURE VariableValues (name: BugsNames.Name; offsets: POINTER TO ARRAY OF INTEGER; numChains: INTEGER;
	VAR f: TextMappers.Formatter);
		CONST
			all = TRUE;
		VAR
			i, index, j, max, size: INTEGER;
			string: ARRAY 80 OF CHAR;
			values: POINTER TO ARRAY OF ARRAY OF REAL;
			initialized: POINTER TO ARRAY OF ARRAY OF BOOLEAN;
			components: GraphNodes.Vector;
			node: GraphNodes.Node;
			parents: GraphStochastic.List;
			init, isConstant: BOOLEAN;
	BEGIN
		components := name.components;
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
		IF~ isConstant & (numChains > 1)THEN
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
		END;
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
								WriteReal(values[i, j], f)
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
	END VariableValues;

	PROCEDURE Values* (IN variable: ARRAY OF CHAR; VAR numChains: INTEGER;
	VAR f: TextMappers.Formatter);
		VAR
			var: BugsParser.Variable;
			name: BugsNames.Name;
			offsets: POINTER TO ARRAY OF INTEGER;
			value: REAL;
			i, index, size: INTEGER;
			string: ARRAY 80 OF CHAR;
	BEGIN
		var := BugsParser.StringToVariable(variable);
		IF var = NIL THEN RETURN END;
		offsets := BugsEvaluate.Offsets(var);
		IF offsets = NIL THEN RETURN END;
		size := LEN(offsets);
		name := var.name;
		IF name.passByreference THEN
			IF offsets # NIL THEN
				IF IsConstant(name, offsets) THEN numChains := 1 END;
				VariableValues(name, offsets, numChains, f)
			END
		ELSE
			i := 0;
			numChains := 1;
			WHILE i < size DO
				index := offsets[i];
				f.WriteString(name.string);
				name.Indices(index, string);
				f.WriteString(string);
				f.WriteTab; f.WriteTab;
				IF name.IsDefined(index) THEN
					value := name.Value(index);
					WriteReal(value, f)
				ELSE
					f.WriteString("NA")
				END;
				f.WriteLn;
				INC(i)
			END
		END
	END Values;

	PROCEDURE CountDataNode (name: BugsNames.Name): INTEGER;
		VAR
			count, i, size: INTEGER;
			components: GraphNodes.Vector;
			p: GraphNodes.Node;
	BEGIN
		count := 0;
		size := name.Size();
		IF name.passByreference THEN
			i := 0;
			components := name.components;
			WHILE i < size DO
				p := components[i];
				IF (p # NIL) & (GraphNodes.data IN p.props) & ~(p IS GraphStochastic.Node) THEN
					INC(count)
				END;
				INC(i)
			END
		ELSE
			INC(count, size)
		END;
		RETURN count
	END CountDataNode;

	PROCEDURE CountLogicalNode (name: BugsNames.Name): INTEGER;
		VAR
			count, i, size: INTEGER;
			components: GraphNodes.Vector;
			p: GraphNodes.Node;
	BEGIN
		count := 0;
		IF name.passByreference THEN
			i := 0;
			size := name.Size();
			components := name.components;
			WHILE i < size DO
				p := components[i];
				IF (p # NIL) & (p IS GraphLogical.Node) & ~(GraphNodes.data IN p.props) THEN
					INC(count)
				END;
				INC(i)
			END
		END;
		RETURN count
	END CountLogicalNode;

	PROCEDURE CountObservedNode (name: BugsNames.Name): INTEGER;
		VAR
			count, i, size: INTEGER;
			components: GraphNodes.Vector;
			p: GraphNodes.Node;
	BEGIN
		count := 0;
		IF name.passByreference THEN
			i := 0;
			components := name.components;
			size := LEN(components);
			WHILE i < size DO
				p := components[i];
				IF (p # NIL) & (p IS GraphStochastic.Node) & (GraphNodes.data IN p.props) THEN
					INC(count)
				END;
				INC(i)
			END
		END;
		RETURN count
	END CountObservedNode;

	PROCEDURE WriteNode (VAR f: TextMappers.Formatter; name: BugsNames.Name; data: BOOLEAN);
		CONST
			maximumCols = 6;
		VAR
			counter, i, maxCols, numSlots, size, slot: INTEGER;
			value: REAL;
			node: GraphNodes.Node;
			newAttr, oldAttr: TextModels.Attributes;
	BEGIN
		size := name.Size();
		numSlots := name.numSlots;
		IF (numSlots > 1) & (name.slotSizes[numSlots - 1] <= 2 * maximumCols) THEN
			maxCols := name.slotSizes[numSlots - 1]
		ELSE
			maxCols := maximumCols
		END;
		oldAttr := f.rider.attr;
		newAttr := TextModels.NewWeight(oldAttr, bold);
		f.rider.SetAttr(newAttr);
		f.WriteString(name.string);
		f.rider.SetAttr(oldAttr);
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
		IF name.passByreference THEN
			WHILE i < size DO
				IF counter = 0 THEN f.WriteTab END;
				node := name.components[i];
				IF node = NIL THEN
					f.WriteString("NA")
				ELSIF data THEN
					IF (GraphNodes.data IN node.props) & ~(GraphStochastic.logical IN node.props) THEN
						value := node.Value();
						IF (node IS GraphStochastic.Node) & ~(GraphStochastic.integer IN node.props) THEN
							WriteReal(value, f)
						ELSE
							WriteRealInt(value, f)
						END
					ELSE
						f.WriteString("NA")
					END
				ELSIF node IS GraphStochastic.Node THEN
					IF ~(GraphNodes.data IN node.props) & (GraphStochastic.initialized IN node.props) THEN
						value := node.Value();
						WriteReal(value, f)
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
			END
		ELSE
			WHILE i < size DO
				IF name.IsDefined(i) THEN
					value := name.Value(i);
					WriteReal(value, f)
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
			END
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
		IF ~name.passByreference THEN RETURN FALSE END;
		size := name.Size();
		i := 0;
		isStochastic := FALSE;
		WHILE (i < size) & ~isStochastic DO
			node := name.components[i];
			IF (node # NIL) & (node IS GraphStochastic.Node) THEN
				isStochastic := ({GraphNodes.data, GraphStochastic.hidden} * node.props = {})
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

	PROCEDURE WriteChain* (chain: INTEGER; VAR f: TextMappers.Formatter);
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
		IF ~name.passByreference THEN RETURN TRUE END;
		size := name.Size();
		i := 0;
		isData := FALSE;
		WHILE (i < size) & ~isData DO
			node := name.components[i];
			IF node # NIL THEN
				isData := (GraphNodes.data IN node.props) & ~(GraphStochastic.logical IN node.props)
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

	PROCEDURE WriteData* (VAR f: TextMappers.Formatter);
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
		IF name.passByreference THEN
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
		END
	END Do;

	PROCEDURE WriteUninitNodes* (chain: INTEGER; VAR f: TextMappers.Formatter);
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

	PROCEDURE ModelMetrics* (VAR f: TextMappers.Formatter);
		VAR
			numUpdater, numData, numObs, meanNumChild,
			medianNumChild, numLogical, numParam, minDepth, maxDepth,
			numDet, numStoch, numNames: INTEGER;
	BEGIN
		numObs := CountObservations();
		numData := CountDatum();
		numLogical := CountLogicals();
		numParam := UpdaterActions.NumParameters();
		numUpdater := UpdaterActions.NumberUpdaters();
		meanNumChild := UpdaterActions.MeanNumChildren();
		medianNumChild := UpdaterActions.MedianNumChildren();
		UpdaterActions.MinMaxDepth(minDepth, maxDepth);
		maxDepth := MAX(ABS(minDepth), ABS(maxDepth));
		BugsParser.CountStatements(numDet, numStoch);
		numNames := BugsIndex.NumberNames();
		f.WriteTab;
		f.WriteString("Number of names: ");
		f.WriteInt(numNames);
		f.WriteLn;
		f.WriteTab;
		f.WriteString("Number of logical relations: ");
		f.WriteInt(numDet);
		f.WriteLn;
		f.WriteTab;
		f.WriteString("Number of stochastic relations: ");
		f.WriteInt(numStoch);
		f.WriteLn;
		f.WriteTab;
		f.WriteString("Number of constants: ");
		f.WriteInt(numData);
		f.WriteLn;
		f.WriteTab;
		f.WriteString("Number of observations: ");
		f.WriteInt(numObs);
		f.WriteLn;
		f.WriteTab;
		f.WriteString("Number of expressions: ");
		f.WriteInt(numLogical);
		f.WriteLn;
		f.WriteTab;
		f.WriteString("Number of parameters: ");
		f.WriteInt(numParam);
		f.WriteLn;
		f.WriteTab;
		f.WriteString("Number of updaters: ");
		f.WriteInt(numUpdater);
		f.WriteLn;
		f.WriteTab;
		f.WriteString("Depth of model graph: ");
		f.WriteInt(maxDepth);
		f.WriteLn;
		f.WriteTab;
		f.WriteString("Mean number of children per updater: ");
		f.WriteInt(meanNumChild);
		f.WriteLn;
		f.WriteTab;
		f.WriteString("Median number of children per updater: ");
		f.WriteInt(medianNumChild);
		f.WriteLn;
	END ModelMetrics;

	PROCEDURE UpdaterType (updater: UpdaterUpdaters.Updater; OUT type: ARRAY OF CHAR);
		VAR
			pos, len: INTEGER;
			string: ARRAY 10 OF CHAR;
	BEGIN
		updater.Install(type);
		BugsMsg.Lookup(type, type);
		Strings.Find(type, "Install", 0, pos);
		IF pos # - 1 THEN
			string := "Install";
			len := LEN(string$);
			Strings.Replace(type, pos, len, "Updater")
		END
	END UpdaterType;

	PROCEDURE UpdatersByName* (VAR f: TextMappers.Formatter);
		VAR
			depth, factors, i, j, len, size: INTEGER;
			name: BugsNames.Name;
			p: GraphNodes.Node;
			stoch: GraphStochastic.Node;
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
		i := 0;
		LOOP
			name := BugsIndex.FindByNumber(i);
			IF name = NIL THEN EXIT END;
			IF name.passByreference THEN
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
							BugsMsg.Lookup(string, string);
							f.WriteString(string);
							f.WriteTab;
							f.WriteInt(size);
							f.WriteTab;
							f.WriteInt(ABS(depth));
							f.WriteTab;
							f.WriteInt(factors);
							f.WriteLn;
							WriteUpdaterNames(updater, f)
						END
					END;
					INC(j)
				END
			END;
			INC(i);
		END
	END UpdatersByName;

	PROCEDURE UpdatersByDepth* (VAR f: TextMappers.Formatter);
		VAR
			depth, i, factors, size, numUpdaters, pos: INTEGER;
			p: GraphStochastic.Node;
			children: GraphStochastic.Vector;
			updater: UpdaterUpdaters.Updater;
			string, sizeString: ARRAY 128 OF CHAR;
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
			updater := UpdaterActions.updaters[chain, i];
			IF updater # NIL THEN
				size := updater.Size();
				depth := updater.Depth();
				children := updater.Children();
				IF children # NIL THEN factors := LEN(children) ELSE factors := 0 END;
				p := updater.Prior(0);
				FindGraphNode(p, string);
				WITH updater: UpdaterAuxillary.UpdaterUV DO
					string := "aux_" + string
				|updater: UpdaterAuxillary.UpdaterMV DO
					Strings.IntToString(size, sizeString);
					string := "aux[1:" + sizeString + "]_" + string
				ELSE
				END;
				IF size = 0 THEN (*	is a constraint	*)
					Strings.Find(string, ",", 0, pos); 
					IF pos # -1 THEN
						string[pos] := "]"; string[pos + 1] := 0X
					ELSE
						Strings.Find(string, "[", 0, pos); 
						IF pos # - 1 THEN string[pos] := 0X END;
						string := string + "[]"
					END
				END;
				f.WriteTab;
				f.WriteString(string);
				f.WriteTab;
				UpdaterType(updater, string);
				f.WriteString(string);
				f.WriteTab;
				f.WriteInt(size);
				f.WriteTab;
				f.WriteInt(ABS(depth));
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

	PROCEDURE MapValues* (VAR f: TextMappers.Formatter);
		VAR
			i, numUpdaters: INTEGER;
			updater: UpdaterUpdaters.Updater;
		CONST
			chain = 0;
	BEGIN
		f.WriteTab;
		f.WriteTab;
		f.WriteString("Value");
		f.WriteTab;
		f.WriteString("Mapped value");
		f.WriteTab;
		f.WriteString("Value");
		f.WriteLn;
		numUpdaters := UpdaterActions.NumberUpdaters();
		i := 0;
		WHILE i < numUpdaters DO
			updater := UpdaterActions.updaters[chain, i];
			IF updater # NIL THEN
				MapValue(updater, f)
			END;
			INC(i)
		END
	END MapValues;

	PROCEDURE Distribute* (workersPerChain: INTEGER; VAR f: TextMappers.Formatter);
		VAR
			i, j, index, label, numUpdaters, pos, size: INTEGER;
			updaters: POINTER TO ARRAY OF ARRAY OF INTEGER;
			u: UpdaterUpdaters.Updater;
			p: GraphStochastic.Node;
			string, sizeString: ARRAY 128 OF CHAR;
			id: POINTER TO ARRAY OF INTEGER;
			isAuxillary: BOOLEAN;
			observations: POINTER TO ARRAY OF GraphStochastic.Vector;
	BEGIN
		IF workersPerChain = 1 THEN
			numUpdaters := UpdaterActions.NumberUpdaters();
			IF numUpdaters > 0 THEN NEW(updaters, 1, numUpdaters) ELSE updaters := NIL END;
			i := 0;
			WHILE i < numUpdaters DO updaters[0, i] := i; INC(i) END;
			id := NIL
		ELSE
			BugsPartition.DistributeUpdaters(workersPerChain, updaters, id);
			observations := BugsPartition.DistributeObservations(updaters); 
			BugsPartition.DistributeCensored(observations, updaters, id)
		END;
		f.WriteLn;
		f.WriteTab;
		f.WriteString("Number of workers per chain: ");
		f.WriteInt(workersPerChain);
		f.WriteLn;
		f.WriteLn;
		f.WriteTab;
		i := 0;
		WHILE i < workersPerChain DO
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
			label := updaters[0, j];
			IF label # undefined THEN
				u := UpdaterActions.updaters[0, label];
				isAuxillary := (u IS UpdaterAuxillary.UpdaterUV) OR (u IS UpdaterAuxillary.UpdaterMV);
				size := u.Size();
			ELSE
				isAuxillary := FALSE;
				size := 1
			END;
			IF size = 0 THEN (*	write constraint	*)
				i := 0;
				WHILE i < workersPerChain DO
					label := updaters[i, j];
					u := UpdaterActions.updaters[0, label];
					p := u.Prior(0);
					FindGraphNode(p, string);
					Strings.Find(string, ",", 0, pos);
					IF pos # -1 THEN
						string[pos] := "}"; string[pos + 1] := 0X
					ELSE
						Strings.Find(string, "[", 0, pos);
						string[pos] := 0X
					END;
					string := "cons(" + string + "[])";
					f.WriteTab;
					f.WriteString(string);
					INC(i)
				END;
				f.WriteTab;
				f.WriteString("-");
				f.WriteLn
			END;
			WHILE index < size DO
				i := 0;
				WHILE i < workersPerChain DO
					label := updaters[i, j];
					IF label # undefined THEN
						u := UpdaterActions.updaters[0, label];
						p := u.Prior(index);
						IF (index # 0) & ~isAuxillary THEN f.WriteTab; f.WriteChar("(") END;
						IF p # NIL THEN
							FindGraphNode(p, string);
							IF isAuxillary THEN
								IF index = 0 THEN
									IF size > 1 THEN
										Strings.IntToString(size, sizeString);
										string := "aux" + "[1:" + sizeString + "]_" + string
									ELSE
										string := "aux_" + string
									END;
									f.WriteTab;
									f.WriteString(string);
								END
							ELSE
								IF index = 0 THEN
									f.WriteTab;
								END;
								f.WriteString(string);
							END
						ELSE
							f.WriteTab; f.WriteString("dummy")
						END
					ELSE
						IF (index = 0) OR ~isAuxillary THEN
							f.WriteTab;
							IF (index # 0) & ~isAuxillary THEN f.WriteChar("(") END;
							f.WriteString("dummy")
						END
					END;
					IF (index # 0) & ~isAuxillary THEN f.WriteChar(")") END;
					INC(i)
				END;
				IF (id # NIL) & (index = 0) THEN
					f.WriteTab;
					IF id[j] # 0 THEN f.WriteInt(ABS(id[j])) END;
					IF id[j] < 0 THEN f.WriteChar("*") END;
					IF id[j] = 0 THEN f.WriteChar("-") END;
				END;
				IF isAuxillary THEN
					IF index = 0 THEN f.WriteLn END
				ELSE
					f.WriteLn;
				END;
				INC(index)
			END;
			INC(j)
		END;
		UpdaterActions.UnMarkDistributed
	END Distribute;

	PROCEDURE DistributeDeviance* (workersPerChain: INTEGER; VAR f: TextMappers.Formatter);
		VAR
			observations: POINTER TO ARRAY OF GraphStochastic.Vector;
			updaters: POINTER TO ARRAY OF ARRAY OF INTEGER;
			id: POINTER TO ARRAY OF INTEGER;
			i, j, maxNum, num, numUpdaters: INTEGER;
			p: GraphStochastic.Node;
			string: ARRAY 128 OF CHAR;
	BEGIN
		IF workersPerChain = 1 THEN
			numUpdaters := UpdaterActions.NumberUpdaters();
			IF numUpdaters > 0 THEN NEW(updaters, 1, numUpdaters) ELSE updaters := NIL END;
			i := 0;
			WHILE i < numUpdaters DO updaters[0, i] := i; INC(i) END;
			id := NIL
		ELSE
			BugsPartition.DistributeUpdaters(workersPerChain, updaters, id)
		END;
		observations := BugsPartition.DistributeObservations(updaters);
		f.WriteTab;
		f.WriteString("Number of workers per chain: ");
		f.WriteInt(workersPerChain);
		f.WriteLn;
		f.WriteLn;
		f.WriteTab;
		i := 0;
		maxNum := 0;
		WHILE i < workersPerChain DO
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
			WHILE i < workersPerChain DO
				IF observations[i] # NIL THEN num := LEN(observations[i]) ELSE num := 0 END;
				IF j < num THEN
					p := observations[i, j];
					FindGraphNode(p, string);
					f.WriteString(string);
					IF ~(GraphNodes.data IN p.props) THEN
						f.WriteString("*")
					END
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

	PROCEDURE DistributeInfo* (VAR f: TextMappers.Formatter);
		VAR
			worker, workersPerChain, numChains, numWorkers, memory, time, i, num, pos: INTEGER;
			size: LONGINT;
			hook: BugsInterface.DistributeHook;
			string: ARRAY 128 OF CHAR;
	BEGIN
		IF BugsInterface.IsDistributed() THEN
			f.WriteTab;
			IF BugsComponents.allThis THEN
				f.WriteString("Graph seperates over cores")
			ELSE
				f.WriteString("Graph does not seperate over cores")
			END;
			f.WriteLn;
			f.WriteLn;
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
			workersPerChain := hook.workersPerChain;
			numChains := hook.numChains;
			numWorkers := workersPerChain * numChains;
			WHILE worker < numWorkers DO
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
