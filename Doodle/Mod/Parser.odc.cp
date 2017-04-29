(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE DoodleParser;


	

	IMPORT
		Controllers, Dialog, Models, Strings, Views,
		BugsMappers, BugsMsg, BugsParser, BugsVariables, 
		DoodleMenus, DoodleModels, DoodleNodes, DoodlePlates, DoodleViews, 
		GraphGrammar, GraphNodes;

	TYPE
		NodeList = POINTER TO RECORD
			node: DoodleNodes.Node;
			next: NodeList
		END;

		PlateList = POINTER TO RECORD
			plate: DoodlePlates.Plate;
			next: PlateList
		END;

		ParsedPlateList = POINTER TO RECORD
			plate: DoodlePlates.Plate;
			index: BugsParser.Index;
			next: ParsedPlateList
		END;

	VAR
		errorNode: DoodleNodes.Node;
		errorPlate: DoodlePlates.Plate;
		parsedPlateList: ParsedPlateList;
		errorCursor, errorIndex: INTEGER;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;


	PROCEDURE Error (errorNum: INTEGER);
		VAR
			numToString: ARRAY 8 OF CHAR;
			errorMes: ARRAY 1024 OF CHAR;
	BEGIN
		Strings.IntToString(errorNum, numToString);
		BugsMsg.Lookup("DoodleParser" + numToString, errorMes);
		BugsMsg.Store(errorMes)
	END Error;

	PROCEDURE Nested (m: DoodleModels.Model; inner, outer: DoodlePlates.Plate): BOOLEAN;
		VAR
			nested: BOOLEAN;
			innerL, innerT, innerR, innerB, outerL, outerT, outerR, outerB, x, y: INTEGER;
			nodeList: DoodleModels.NodeList;
			node: DoodleNodes.Node;
	BEGIN
		nested := (inner.l > outer.l) & (inner.t > outer.t) & (inner.r < outer.r) & (inner.b < outer.b);
		IF ~nested THEN
			nodeList := m.nodeList;
			innerL := MAX(INTEGER); innerT := MAX(INTEGER); innerR := 0; innerB := 0;
			outerL := MAX(INTEGER); outerT := MAX(INTEGER); outerR := 0; outerB := 0;
			WHILE nodeList # NIL DO
				node := nodeList.node;
				IF node.type # DoodleNodes.constant THEN
					x := node.x; y := node.y;
					IF (x > inner.l) & (y > inner.t) & (x < inner.r) & (y < inner.b) THEN
						innerL := MIN(innerL, x); innerT := MIN(innerT, y);
						innerR := MAX(innerR, x); innerB := MAX(innerB, y)
					END;
					IF (x > outer.l) & (y > outer.t) & (x < outer.r) & (y < outer.b) THEN
						outerL := MIN(outerL, x); outerT := MIN(outerT, y);
						outerR := MAX(outerR, x); outerB := MAX(outerB, y)
					END
				END;
				nodeList := nodeList.next
			END;
			nested := (innerL >= outerL) & (innerT >= outerT) & (innerR <= outerR) & (innerB <= outerB)
		END;
		RETURN nested
	END Nested;

	PROCEDURE MaxPlateDepth (m: DoodleModels.Model): INTEGER;
		VAR
			depth: INTEGER;
			plateList: DoodleModels.PlateList;
	BEGIN
		plateList := m.plateList;
		depth := 0;
		WHILE plateList # NIL DO INC(depth); plateList := plateList.next END;
		RETURN depth
	END MaxPlateDepth;

	PROCEDURE NodeDepth (m: DoodleModels.Model; node: DoodleNodes.Node): INTEGER;
		VAR
			depth, l, t, b, r, x, y: INTEGER;
			plateList: DoodleModels.PlateList;
			plate: DoodlePlates.Plate;
	BEGIN
		plateList := m.plateList;
		depth := 0;
		x := node.x;
		y := node.y;
		WHILE plateList # NIL DO
			plate := plateList.plate;
			l := plate.l;
			t := plate.t;
			r := plate.r;
			b := plate.b;
			IF (l < x) & (r > x) & (t < y) & (b > y) THEN
				INC(depth)
			END;
			plateList := plateList.next
		END;
		RETURN depth
	END NodeDepth;

	PROCEDURE SortedNodes (nodeList: DoodleModels.NodeList): NodeList;
		VAR
			list, link, cursor: NodeList;
			node: DoodleNodes.Node;
	BEGIN
		list := NIL;
		WHILE nodeList # NIL DO
			node := nodeList.node;
			NEW(link);
			link.node := node;
			IF (list = NIL) OR (node.name > list.node.name) THEN
				link.next := list;
				list := link
			ELSE
				cursor := list;
				LOOP
					IF cursor.next = NIL THEN
						link.next := NIL;
						cursor.next := link;
						EXIT
					ELSIF node.name > cursor.next.node.name THEN
						link.next := cursor.next;
						cursor.next := link;
						EXIT
					END;
					cursor := cursor.next
				END
			END;
			nodeList := nodeList.next
		END;
		RETURN list
	END SortedNodes;

	PROCEDURE GetIndicesNames (m: DoodleModels.Model; OUT ok: BOOLEAN);
		VAR
			plateList: DoodleModels.PlateList;
			plate: DoodlePlates.Plate;
			s: BugsMappers.Scanner;
	BEGIN
		ok := TRUE;
		plateList := m.plateList;
		WHILE plateList # NIL DO
			plate := plateList.plate;
			s.ConnectToString(plate.variable);
			s.SetPos(0);
			s.Scan;
			IF s.type = BugsMappers.string THEN
				BugsVariables.StoreLoop(s.string)
			ELSE
				ok := FALSE;
				Error(1);
				errorPlate := plate;
				errorCursor := DoodlePlates.index;
				errorIndex := s.Pos();
				RETURN
			END;
			plateList := plateList.next
		END
	END GetIndicesNames;

	PROCEDURE GetVariableNames (m: DoodleModels.Model; OUT ok: BOOLEAN);
		VAR
			plateList: DoodleModels.PlateList;
			nodeList: DoodleModels.NodeList;
			node: DoodleNodes.Node;
			plate: DoodlePlates.Plate;
			s: BugsMappers.Scanner;
	BEGIN
		ok := TRUE;
		plateList := m.plateList;
		WHILE plateList # NIL DO
			plate := plateList.plate;
			s.ConnectToString(plate.lower);
			s.SetPos(0);
			BugsVariables.StoreNames(s, ok);
			IF ~ok THEN
				errorPlate := plate;
				errorCursor := DoodlePlates.lower;
				errorIndex := s.Pos();
				RETURN
			END;
			plateList := plateList.next
		END;
		plateList := m.plateList;
		WHILE plateList # NIL DO
			plate := plateList.plate;
			s.ConnectToString(plate.upper);
			s.SetPos(0);
			BugsVariables.StoreNames(s, ok);
			IF ~ok THEN
				errorPlate := plate;
				errorCursor := DoodlePlates.upper;
				errorIndex := s.Pos();
				RETURN
			END;
			plateList := plateList.next
		END;
		nodeList := m.nodeList;
		WHILE nodeList # NIL DO
			node := nodeList .node;
			s.ConnectToString(node.name);
			s.SetPos(0);
			BugsVariables.StoreNames(s, ok);
			IF ~ok THEN
				errorNode := node;
				errorCursor := DoodleNodes.name;
				errorIndex := s.Pos();
				RETURN
			END;
			nodeList := nodeList.next
		END;
		nodeList := m.nodeList;
		WHILE nodeList # NIL DO
			node := nodeList.node;
			IF node.type = DoodleNodes.logical THEN
				s.ConnectToString(node.value);
				s.SetPos(0);
				BugsVariables.StoreNames(s, ok);
				IF ~ok THEN
					errorNode := node;
					errorCursor := DoodleNodes.value;
					errorIndex := s.Pos();
					RETURN
				END
			END;
			nodeList := nodeList.next
		END
	END GetVariableNames;

	PROCEDURE GetPlates (m: DoodleModels.Model; node: DoodleNodes.Node): PlateList;
		VAR
			b, l, r, t, x, y: INTEGER;
			plateList: DoodleModels.PlateList;
			plate: DoodlePlates.Plate;
			cursor, list, newList, element: PlateList;
	BEGIN
		x := node.x;
		y := node.y;
		list := NIL;
		plateList := m.plateList;
		WHILE plateList # NIL DO
			plate := plateList.plate;
			l := plate.l;
			t := plate.t;
			r := plate.r;
			b := plate.b;
			IF (l < x) & (r > x) & (t < y) & (b > y) THEN
				NEW(element);
				element.plate := plate;
				element.next := list;
				list := element
			END;
			plateList := plateList.next
		END;
		(*	sort plates by nesting	*)
		newList := NIL;
		WHILE list # NIL DO
			plate := list.plate;
			NEW(element);
			element.plate := plate;
			IF (newList = NIL) OR Nested(m, newList.plate, plate) THEN
				element.next := newList;
				newList := element
			ELSE
				cursor := newList;
				LOOP
					IF cursor.next = NIL THEN
						element.next := NIL;
						cursor.next := element;
						EXIT
					ELSIF Nested(m, cursor.next.plate, plate) THEN
						element.next := cursor.next;
						cursor.next := element;
						EXIT
					END;
					cursor := cursor.next
				END
			END;
			list := list.next
		END;
		RETURN newList
	END GetPlates;

	PROCEDURE IsEmptyString (s: ARRAY OF CHAR): BOOLEAN;
		VAR
			i: INTEGER;
	BEGIN
		i := 0;
		WHILE (s[i] # 0X) & (s[i] = " ") DO
			INC(i)
		END;
		RETURN s[i] = 0X
	END IsEmptyString;

	PROCEDURE ParseStochastic (loops: BugsParser.Statement;
	node: DoodleNodes.Node): BugsParser.Statement;
		VAR
			i, numPar: INTEGER;
			densityName: Dialog.String;
			s: BugsMappers.Scanner;
			density: BugsParser.Density;
			left: BugsParser.Variable;
			param: BugsParser.Node;
			parents: ARRAY 5 OF BugsParser.Node;
			descriptor: GraphGrammar.External;
			fact: GraphNodes.Factory;
	BEGIN
		DoodleMenus.Density(node.density, densityName);
		descriptor := GraphGrammar.FindDensity(densityName);
		IF descriptor = NIL THEN
			Error(2);
			errorNode := node;
			errorCursor := DoodleNodes.name;
			errorIndex := 0;
			RETURN NIL
		END;
		fact := descriptor.fact;
		numPar := fact.NumParam();
		s.ConnectToString(node.name);
		s.SetPos(0);
		s.Scan;
		left := BugsParser.ParseVariable(loops, s);
		IF left = NIL THEN
			errorNode := node;
			errorCursor := DoodleNodes.name;
			errorIndex := s.Pos();
			RETURN NIL
		END;
		i := 0;
		WHILE i < 5 DO
			parents[i] := NIL;
			IF i IN node.mask THEN
				IF node.parents[i] # NIL THEN
					s.ConnectToString(node.parents[i].name);
					s.SetPos(0);
					s.Scan;
					param := BugsParser.ParseParameter(loops, s);
					IF param = NIL THEN
						errorNode := node;
						errorCursor := DoodleNodes.name;
						errorIndex := s.Pos();
						RETURN NIL
					END;
					parents[i] := param;
				ELSIF ~IsEmptyString(node.defaults[i]) THEN
					s.ConnectToString(node.defaults[i]);
					s.SetPos(0);
					s.Scan;
					parents[i] := BugsParser.ParseParameter(loops, s);
					IF parents[i] = NIL THEN
						errorNode := node;
						errorCursor := i;
						errorIndex := s.Pos();
						RETURN NIL
					END
				ELSIF i < numPar THEN
					Error(4);
					errorNode := node;
					errorCursor := DoodleNodes.name;
					errorIndex := 0;
					RETURN NIL
				END
			END;
			INC(i)
		END;
		density := BugsParser.BuildDensity(parents, descriptor);
		RETURN BugsParser.BuildStochastic(left, density)
	END ParseStochastic;

	PROCEDURE ParseLogical (loops: BugsParser.Statement; node: DoodleNodes.Node
	): BugsParser.Statement;
		VAR
			linkName: Dialog.String;
			s: BugsMappers.Scanner;
			variable: BugsParser.Variable;
			function, expression: BugsParser.Node;
			descriptor: GraphGrammar.External;
			statement: BugsParser.Statement;
			parents: POINTER TO ARRAY OF BugsParser.Node;
			fact: GraphNodes.Factory;
			signature: ARRAY 64 OF CHAR;
			len: INTEGER;
	BEGIN
		NEW(parents, 1);
		s.ConnectToString(node.name);
		s.SetPos(0);
		s.Scan;
		variable := BugsParser.ParseVariable(loops, s);
		IF variable = NIL THEN
			errorNode := node;
			errorCursor := DoodleNodes.name;
			errorIndex := s.Pos();
			RETURN NIL
		END;
		s.ConnectToString(node.value);
		s.SetPos(0);
		s.Scan;
		expression := BugsParser.ParseExpression(loops, s);
		IF expression = NIL THEN
			errorNode := node;
			errorCursor := DoodleNodes.value;
			errorIndex := s.Pos(); RETURN NIL
		END;
		IF node.link = 0 THEN
			statement := BugsParser.BuildLogical(variable, expression)
		ELSE
			DoodleMenus.Link(node.link, linkName);
			descriptor := GraphGrammar.FindFunction(linkName);
			IF descriptor = NIL THEN
				Error(3);
				errorNode := node;
				errorCursor := DoodleNodes.name;
				errorIndex := 0;
				RETURN NIL
			END;
			fact := descriptor.fact;
			fact.Signature(signature);
			len := LEN(signature$);
			IF signature[len - 1] # "L" THEN
				Error(3);
				errorNode := node;
				errorCursor := DoodleNodes.name;
				errorIndex := 0;
				RETURN NIL
			END;
			parents[0] := expression;
			function := BugsParser.BuildFunction(parents, descriptor);
			statement := BugsParser.BuildLogical(variable, function)
		END;
		RETURN statement
	END ParseLogical;

	PROCEDURE FindParsedPlate (plate: DoodlePlates.Plate): ParsedPlateList;
		VAR
			cursor: ParsedPlateList;
	BEGIN
		cursor := parsedPlateList;
		WHILE (cursor # NIL) & (cursor.plate # plate) DO cursor := cursor.next END;
		RETURN cursor
	END FindParsedPlate;

	PROCEDURE AddParsedPlate (plate: DoodlePlates.Plate; index: BugsParser.Index);
		VAR
			element: ParsedPlateList;
	BEGIN
		NEW(element);
		element.plate := plate;
		element.index := index;
		element.next := parsedPlateList;
		parsedPlateList := element
	END AddParsedPlate;

	PROCEDURE ParseLoops (plates: PlateList): BugsParser.Statement;
		VAR
			plateDoodle: DoodlePlates.Plate;
			s: BugsMappers.Scanner;
			index: BugsParser.Index;
			lower, upper: BugsParser.Node;
			loops, statement: BugsParser.Statement;
			parsedPlateList: ParsedPlateList;
	BEGIN
		loops := NIL;
		WHILE plates # NIL DO
			plateDoodle := plates.plate;
			s.ConnectToString(plateDoodle.variable);
			s.SetPos(0);
			s.Scan;
			index := BugsParser.ParseIndexName(loops, s);
			IF index = NIL THEN
				errorPlate := plateDoodle;
				errorCursor := DoodlePlates.index;
				errorIndex := s.Pos();
				RETURN NIL
			END;
			s.ConnectToString(plateDoodle.lower);
			s.SetPos(0);
			s.Scan;
			lower := BugsParser.ParseExpression(loops, s);
			IF lower = NIL THEN
				errorPlate := plateDoodle;
				errorCursor := DoodlePlates.lower;
				errorIndex := s.Pos();
				RETURN NIL
			END;
			s.ConnectToString(plateDoodle.upper);
			s.SetPos(0);
			s.Scan;
			upper := BugsParser.ParseExpression(loops, s);
			IF upper = NIL THEN
				errorPlate := plateDoodle;
				errorCursor := DoodlePlates.upper;
				errorIndex := s.Pos();
				RETURN NIL
			END;
			BugsParser.BuildIndex(index, lower, upper);
			parsedPlateList := FindParsedPlate(plateDoodle);
			IF parsedPlateList = NIL THEN
				AddParsedPlate(plateDoodle, index);
			ELSE
				index := parsedPlateList.index;
			END;
			statement := BugsParser.BuildForLoop(index);
			statement.CopyToList(loops);
			plates := plates.next
		END;
		RETURN loops
	END ParseLoops;

	PROCEDURE ParseGraph (m: DoodleModels.Model): BugsParser.Statement;
		VAR
			nodeList, sortedNodeList: NodeList;
			node: DoodleNodes.Node;
			plates: PlateList;
			loops, model, statement: BugsParser.Statement;
			depth, i, maxDepth, type: INTEGER;
	BEGIN
		model := NIL;
		errorNode := NIL;
		errorPlate := NIL;
		maxDepth := MaxPlateDepth(m);
		sortedNodeList := SortedNodes(m.nodeList);
		i := 0;
		WHILE i <= maxDepth DO
			type := DoodleNodes.logical;
			WHILE type >= DoodleNodes.stochastic DO
				nodeList := sortedNodeList;
				WHILE nodeList # NIL DO
					node := nodeList.node;
					depth := NodeDepth(m, node);
					IF i = depth THEN
						statement := NIL;
						IF (node.type = DoodleNodes.stochastic) OR (node.type = DoodleNodes.logical) THEN
							loops := NIL;
							plates := GetPlates(m, node);
							IF plates # NIL THEN
								loops := ParseLoops(plates);
								IF loops = NIL THEN RETURN NIL END
							END;
							IF (node.type = DoodleNodes.stochastic) & (node.type = type) THEN
								IF node.name[0] # 0X THEN
									statement := ParseStochastic(loops, node);
									IF statement = NIL THEN RETURN NIL END
								END
							ELSIF (node.type = DoodleNodes.logical) & (node.type = type) THEN
								statement := ParseLogical(loops, node);
								IF statement = NIL THEN RETURN NIL END
							END;
							IF statement # NIL THEN
								statement.CopyToList(model);
								IF loops # NIL THEN loops.MergeLists(model) END
							END
						END
					END;
					nodeList := nodeList.next
				END;
				DEC(type)
			END;
			INC(i)
		END;
		RETURN model
	END ParseGraph;

	PROCEDURE ParseModel* (m: DoodleModels.Model): BugsParser.Statement;
		VAR
			ok: BOOLEAN;
			model: BugsParser.Statement;
	BEGIN
		errorNode := NIL;
		errorPlate := NIL;
		BugsVariables.Clear;
		GetIndicesNames(m, ok);
		IF ~ok THEN RETURN NIL END;
		GetVariableNames(m, ok);
		IF ~ok THEN RETURN NIL END;
		model := ParseGraph(m);
		parsedPlateList := NIL;
		RETURN model
	END ParseModel;

	PROCEDURE PrintError* (m: DoodleModels.Model);
		VAR
			errorMes: ARRAY 240 OF CHAR;
			v: Views.View;
			umsg: Models.UpdateMsg;
	BEGIN
		Models.BeginModification(Views.notUndoable, m);
		m.RemoveSelection;
		v := Controllers.FocusView();
		WITH v: DoodleViews.View DO
			v.ShowCaret
		ELSE
		END;
		IF errorNode # NIL THEN
			errorNode.Select(TRUE);
			errorNode.SetCaret(errorCursor, errorIndex)
		ELSIF errorPlate # NIL THEN
			errorPlate.Select(TRUE);
			errorPlate.SetCaret(errorCursor, errorIndex)
		END;
		BugsMsg.Show(BugsMsg.message);
		Models.EndModification(Views.notUndoable, m);
		Models.Broadcast(m, umsg)
	END PrintError;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas";
		parsedPlateList := NIL
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		parsedPlateList := NIL;
		errorNode := NIL;
		errorPlate := NIL
	END Init;

BEGIN
	Init
END DoodleParser.
