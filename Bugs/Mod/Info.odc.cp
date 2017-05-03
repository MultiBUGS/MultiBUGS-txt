(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsInfo;

	

	IMPORT
		Fonts, Strings, Views,
		BugsBlueDiamonds, BugsEvaluate, BugsFiles, BugsIndex, BugsInterface, BugsMsg, BugsNames, 
		BugsParser,
		GraphLogical, GraphNodes, GraphStochastic,
		UpdaterActions, UpdaterAuxillary, UpdaterMultivariate, UpdaterParallel, UpdaterUpdaters,
		TextMappers, TextModels;

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
		
	PROCEDURE WriteReal (x: REAL; VAR f: TextMappers.Formatter);
	BEGIN
		f.WriteRealForm(x, BugsFiles.prec, 0, 0, TextModels.digitspace)
	END WriteReal;
		
	PROCEDURE FindGraphNode (p: GraphStochastic.Node; OUT string: ARRAY OF CHAR);
		VAR
			len: INTEGER;
			children: GraphStochastic.Vector;
	BEGIN
		BugsIndex.FindGraphNode(p, string);
		len := LEN(string$);
		Strings.Extract(string, 1, len - 2, string)
	END FindGraphNode;

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

	PROCEDURE IsConstant* (IN variable: ARRAY OF CHAR): BOOLEAN; 
		VAR
		i, index, size: INTEGER;
		offsets: POINTER TO ARRAY OF INTEGER;
		components: GraphNodes.Vector;
		var: BugsParser.Variable;
		name: BugsNames.Name;
		node: GraphNodes.Node;
		isConstant: BOOLEAN;
	BEGIN
		var := BugsParser.StringToVariable(variable);
		IF var = NIL THEN RETURN TRUE END;
		name := var.name;
		components := name.components;
		offsets := BugsEvaluate.Offsets(var);
		IF offsets = NIL THEN RETURN TRUE END;
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

	PROCEDURE Values* (IN variable: ARRAY OF CHAR; numChains: INTEGER;
	VAR f: TextMappers.Formatter);
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
			init, isConstant: BOOLEAN;
	BEGIN
		var := BugsParser.StringToVariable(variable);
		IF var = NIL THEN RETURN END;
		name := var.name;
		components := name.components;
		offsets := BugsEvaluate.Offsets(var);
		IF offsets = NIL THEN RETURN END;
		size := LEN(offsets);
		isConstant := IsConstant(variable);
		IF isConstant THEN numChains := 1 END;
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
		IF~ isConstant  & (numChains > 1)THEN
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
	END Values;

	PROCEDURE WriteRealInt (VAR f: TextMappers.Formatter; value: REAL);
		VAR
			int: INTEGER;
			absVal: REAL;
		CONST
			eps = 1.0E-6;
	BEGIN
		absVal := ABS(value);
		int := SHORT(ENTIER(value + eps));
		IF ABS(absVal - int) > eps THEN
			WriteReal(value, f)
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
		newAttr := TextModels.NewWeight(oldAttr, Fonts.bold);
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
		WHILE i < size DO
			IF counter = 0 THEN f.WriteTab END;
			node := name.components[i];
			IF node = NIL THEN
				f.WriteString("NA")
			ELSIF data THEN
				IF (GraphNodes.data IN node.props) & ~(GraphLogical.logical IN node.props) THEN
					value := node.Value();
					IF (node IS GraphStochastic.Node) & ~(GraphStochastic.integer IN node.props) THEN
						WriteReal(value, f)
					ELSE
						WriteRealInt(f, value)
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
			medianNumChild, numLogical, numParam: INTEGER;
	BEGIN
		numObs := CountObservations();
		numData := CountDatum();
		numLogical := CountLogicals();
		numParam := UpdaterActions.NumParameters();
		numUpdater := UpdaterActions.NumberUpdaters();
		meanNumChild := UpdaterActions.MeanNumChildren();
		medianNumChild := UpdaterActions.MedianNumChildren();
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
		f.WriteString("Mean number of children per updater: ");
		f.WriteInt(meanNumChild);
		f.WriteLn;
		f.WriteTab;
		f.WriteString("Median number of children per updater: ");
		f.WriteInt(medianNumChild);
		f.WriteLn
	END ModelMetrics;

	PROCEDURE UpdaterType (updater: UpdaterUpdaters.Updater; OUT type: ARRAY OF CHAR);
		VAR
			pos, len: INTEGER;
	BEGIN
		updater.Install(type);
		BugsMsg.Lookup(type, type);
		Strings.Find(type, "Install", 0, pos);
		IF pos #  - 1 THEN
			len := LEN("Install");
			Strings.Replace(type, pos, len, "Updater")
		END
	END UpdaterType;

	PROCEDURE UpdatersByName* (VAR f: TextMappers.Formatter);
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
						BugsMsg.Lookup(string, string);
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

	PROCEDURE UpdatersByDepth* (VAR f: TextMappers.Formatter);
		VAR
			depth, i, factors, size, numUpdaters: INTEGER;
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
			updater := UpdaterActions.GetUpdater(chain, i);
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

	PROCEDURE Distribute* (numProc: INTEGER; VAR f: TextMappers.Formatter);
		VAR
			i, j, index, numUpdaters, size, space, start, thin: INTEGER;
			updaters: POINTER TO ARRAY OF UpdaterUpdaters.Vector;
			u: UpdaterUpdaters.Updater;
			fact: UpdaterUpdaters.Factory;
			p: GraphStochastic.Node;
			install, string, sizeString: ARRAY 128 OF CHAR;
			id: POINTER TO ARRAY OF INTEGER;
			isAuxillary: BOOLEAN;
			children: GraphStochastic.Vector;
			blueDiamond: Views.View;
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
			isAuxillary := (u IS UpdaterAuxillary.UpdaterUV) OR (u IS UpdaterAuxillary.UpdaterMV);
			size := u.Size();
			WHILE index < size DO
				i := 0;
				WHILE i < numProc DO
					u := updaters[i, j];
					p := u.Prior(index);
					IF (p # NIL) & (GraphStochastic.distributed IN p.props) THEN
						start := i; thin := numProc
					ELSE
						start := -1; thin := 1
					END;
					children := u.Children();
					IF (index # 0) & ~isAuxillary THEN f.WriteTab; f.WriteChar("(") END;
					IF p # NIL THEN
						FindGraphNode(p, string);
						IF isAuxillary THEN
							IF index = 0  THEN
								IF size > 1 THEN 
									Strings.IntToString(size, sizeString);
									string := "aux" + "[1:" + sizeString + "]_" + string
								ELSE
									string := "aux_" + string
								END;
								f.WriteTab; 
								(*blueDiamond := BugsBlueDiamonds.New(children, string, start, thin);
								f.WriteView(blueDiamond);*)
								f.WriteString(string);
							END
						ELSE
							IF index = 0 THEN f.WriteTab END; 
							blueDiamond := BugsBlueDiamonds.New(children, string, start, thin);
							f.WriteView(blueDiamond);
							f.WriteString(string);
						END
					ELSE
						IF (index = 0) OR ~isAuxillary THEN
							f.WriteTab; f.WriteString("dummy")
						END
					END;
					IF (index # 0) & ~isAuxillary THEN f.WriteChar(")") END;
					INC(i)
				END;
				IF index = 0 THEN
					f.WriteTab;
					IF id[j] # 0 THEN f.WriteInt(ABS(id[j])) END;
					IF id[j] < 0 THEN f.WriteChar("*") END;
					IF id[j] = 0 THEN f.WriteChar("-") END;
				END;
				IF isAuxillary THEN
					IF index = 0 THEN f.WriteLn  END
				ELSE
					f.WriteLn;
				END;
				INC(index)
			END;
			INC(j)
		END;
		UpdaterActions.UnMarkDistributed
	END Distribute;

	PROCEDURE DistributeDeviance* (numProc: INTEGER; VAR f: TextMappers.Formatter);
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

	PROCEDURE DistributeInfo* (VAR f: TextMappers.Formatter);
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
						IF pos #  - 1 THEN string[pos] := 0X END
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
