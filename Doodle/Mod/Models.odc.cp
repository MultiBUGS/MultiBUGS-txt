(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	   *)

MODULE DoodleModels;


	

	IMPORT
		Models, Stores,
		DoodleNodes, DoodlePlates;

	CONST
		minVersion = 0;
		maxVersion = 0;

	TYPE
		NodeList* = POINTER TO LIMITED RECORD
			node-: DoodleNodes.Node;
			next-: NodeList
		END;

		PlateList* = POINTER TO LIMITED RECORD
			plate-: DoodlePlates.Plate;
			next-: PlateList
		END;

		Model* = POINTER TO RECORD (Models.Model)
			nodeList-: NodeList;
			plateList-: PlateList
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (m: Model) CopyFrom- (source: Stores.Store);
		VAR
			i, len: INTEGER;
			nCursor, nList: NodeList;
			pCursor, pList: PlateList;
			node: DoodleNodes.Node;
			plate: DoodlePlates.Plate;
			nodes: DoodleNodes.Parents;
	BEGIN
		nCursor := source(Model).nodeList;
		m.nodeList := NIL;
		i := 0;
		WHILE nCursor # NIL DO
			node := nCursor.node;
			node.SetLabel(i);
			NEW(nList);
			nList.next := m.nodeList;
			m.nodeList := nList;
			INC(i);
			nCursor := nCursor.next
		END;
		len := i;
		IF len > 0 THEN
			NEW(nodes, len)
		END;
		i := 0;
		WHILE i < len DO
			nodes[i] := DoodleNodes.New();
			INC(i)
		END;
		nCursor := source(Model).nodeList;
		i := 0;
		WHILE nCursor # NIL DO
			node := nCursor.node;
			nodes[i].CopyFrom(node, nodes);
			INC(i);
			nCursor := nCursor.next
		END;
		nList := m.nodeList; i := 0;
		WHILE nList # NIL DO
			nList.node := nodes[i];
			INC(i);
			nList := nList.next
		END;
		pCursor := source(Model).plateList;
		m.plateList := NIL;
		WHILE pCursor # NIL DO
			NEW(pList);
			pList.next := m.plateList;
			m.plateList := pList;
			pCursor := pCursor.next
		END;
		pCursor := source(Model).plateList;
		pList := m.plateList;
		WHILE pCursor # NIL DO
			plate := DoodlePlates.New();
			plate.CopyFrom(pCursor.plate);
			pList.plate := plate;
			pCursor := pCursor.next;
			pList := pList.next
		END
	END CopyFrom;

	PROCEDURE (m: Model) DeleteNode* (node: DoodleNodes.Node), NEW;
		VAR
			i, len: INTEGER;
			nodeList: NodeList;
			p: DoodleNodes.Node;
	BEGIN
		nodeList := m.nodeList;
		WHILE nodeList # NIL DO
			p := nodeList.node;
			IF p.parents = NIL THEN
				len := 0
			ELSE
				len := LEN(p.parents)
			END;
			i := 0;
			WHILE i < len DO
				IF p.parents[i] = node THEN
					p.parents[i] := NIL
				END;
				INC(i)
			END;
			nodeList := nodeList.next
		END;
		IF m.nodeList.node = node THEN
			m.nodeList := m.nodeList.next
		ELSE
			nodeList := m.nodeList;
			WHILE nodeList.next.node # node DO
				nodeList := nodeList.next
			END;
			nodeList.next := nodeList.next.next
		END
	END DeleteNode;

	PROCEDURE (m: Model) DeletePlate* (plate: DoodlePlates.Plate), NEW;
		VAR
			plateList: PlateList;
	BEGIN
		IF m.plateList.plate = plate THEN
			m.plateList := m.plateList.next
		ELSE
			plateList := m.plateList;
			WHILE plateList.next.plate # plate DO
				plateList := plateList.next
			END;
			plateList.next := plateList.next.next
		END
	END DeletePlate;

	PROCEDURE (m: Model) Externalize- (VAR wr: Stores.Writer);
		VAR
			i, numNodes, numPlates: INTEGER;
			nodeList: NodeList;
			plateList: PlateList;
	BEGIN
		wr.WriteVersion(maxVersion);
		numPlates := 0;
		plateList := m.plateList;
		WHILE plateList # NIL DO
			INC(numPlates);
			plateList := plateList.next
		END;
		wr.WriteInt(numPlates);
		plateList := m.plateList;
		WHILE plateList # NIL DO
			plateList.plate.Write(wr);
			plateList := plateList.next
		END;
		numNodes := 0;
		nodeList := m.nodeList;
		WHILE nodeList # NIL DO
			INC(numNodes);
			nodeList := nodeList.next
		END;
		wr.WriteInt(numNodes);
		i := numNodes;
		nodeList := m.nodeList;
		WHILE nodeList # NIL DO
			DEC(i);
			nodeList.node.SetLabel(i);
			nodeList := nodeList.next
		END;
		nodeList := m.nodeList;
		WHILE nodeList # NIL DO
			nodeList.node.Write(wr);
			nodeList := nodeList.next
		END
	END Externalize;

	PROCEDURE (m: Model) Internalize- (VAR rd: Stores.Reader);
		VAR
			i, numNodes, numPlates, thisVersion: INTEGER;
			plateList: PlateList;
			nodeList, nodeLink: NodeList;
			parents: DoodleNodes.Parents;
	BEGIN
		rd.ReadVersion(minVersion, maxVersion, thisVersion);
		IF rd.cancelled THEN RETURN END;
		rd.ReadInt(numPlates);
		i := 0;
		m.plateList := NIL;
		WHILE i < numPlates DO
			NEW(plateList);
			plateList.plate := DoodlePlates.New();
			plateList.plate.Read(rd);
			plateList.next := m.plateList;
			m.plateList := plateList;
			INC(i)
		END;
		rd.ReadInt(numNodes);
		IF numNodes > 0 THEN NEW(parents, numNodes) END;
		i := 0; WHILE i < numNodes DO parents[i] := DoodleNodes.New(); INC(i) END;
		m.nodeList := NIL;
		i := numNodes;
		WHILE i > 0 DO
			NEW(nodeList);
			DEC(i);
			parents[i].Read(rd, parents);
			nodeList.node := parents[i];
			nodeList.next := m.nodeList;
			m.nodeList := nodeList
		END;
		nodeList := m.nodeList;
		m.nodeList := NIL;
		WHILE nodeList # NIL DO
			NEW(nodeLink);
			nodeLink.node := nodeList.node;
			nodeList := nodeList.next;
			nodeLink.next := m.nodeList;
			m.nodeList := nodeLink
		END
	END Internalize;

	PROCEDURE (m: Model) InsertNode* (node: DoodleNodes.Node), NEW;
		VAR
			nodeList: NodeList;
	BEGIN
		NEW(nodeList);
		nodeList.node := node;
		nodeList.next := m.nodeList;
		m.nodeList := nodeList
	END InsertNode;

	PROCEDURE (m: Model) InsertPlate* (plate: DoodlePlates.Plate), NEW;
		VAR
			plateList: PlateList;
	BEGIN
		NEW(plateList);
		plateList.plate := plate;
		plateList.next := m.plateList;
		m.plateList := plateList
	END InsertPlate;

	PROCEDURE (m: Model) MoveRegion* (l, t, r, b, deltaX, deltaY: INTEGER), NEW;
		VAR
			l1, t1, r1, b1: REAL;
			nodeList: NodeList;
			plateList: PlateList;
			node: DoodleNodes.Node;
			plate: DoodlePlates.Plate;
	BEGIN
		plateList := m.plateList;
		plateList := m.plateList;
		WHILE plateList # NIL DO
			plate := plateList.plate;
			l1 := plate.l;
			t1 := plate.t;
			r1 := plate.r;
			b1 := plate.b;
			IF (l <= l1) & (r >= r1) & (t <= t1) & (b >= b1) THEN
				plate.Move(deltaX, deltaY)
			END;
			plateList := plateList.next
		END;
		nodeList := m.nodeList;
		WHILE nodeList # NIL DO
			node := nodeList.node;
			l1 := node.x - 2;
			t1 := node.y - 1;
			r1 := node.x + 2;
			b1 := node.y + 1;
			IF (l <= l1) & (r >= r1) & (t <= t1) & (b >= b1) THEN
				node.Move(deltaX, deltaY)
			END;
			nodeList := nodeList.next
		END
	END MoveRegion;

	PROCEDURE (m: Model) NodeAt* (x, y, d: INTEGER): DoodleNodes.Node, NEW;
		VAR
			nodeList: NodeList;
			node: DoodleNodes.Node;
	BEGIN
		nodeList := m.nodeList;
		LOOP
			IF nodeList = NIL THEN
				node := NIL;
				EXIT
			END;
			node := nodeList.node;
			IF (ABS(x - node.x) < 2 * d) & (ABS(y - node.y) < d) THEN
				EXIT
			END;
			nodeList := nodeList.next
		END;
		RETURN node
	END NodeAt;

	PROCEDURE (m: Model) PlateAt* (x, y, d: INTEGER): DoodlePlates.Plate, NEW;
		VAR
			l, t, r, b: INTEGER;
			plateList: PlateList;
			plate: DoodlePlates.Plate;
	BEGIN
		plateList := m.plateList;
		LOOP
			IF plateList = NIL THEN
				RETURN NIL
			END;
			plate := plateList.plate;
			l := plate.l;
			t := plate.t;
			r := plate.r;
			b := plate.b;
			IF(x < l) OR (x > r) OR (y < t) OR (y > b) THEN
				plateList := plateList.next
			ELSIF (x > l + d DIV 2) & (x < r - d DIV 2) & (y > t + d DIV 2) & (y < b - d DIV 2) THEN
				plateList := plateList.next
			ELSE
				RETURN plate
			END
		END
	END PlateAt;

	PROCEDURE (m: Model) RemoveSelection*, NEW;
		VAR
			nodeList: NodeList;
			plateList: PlateList;
	BEGIN
		nodeList := m.nodeList;
		WHILE nodeList # NIL DO
			nodeList.node.Select(FALSE);
			nodeList := nodeList.next
		END;
		plateList := m.plateList;
		WHILE plateList # NIL DO
			plateList.plate.Select(FALSE);
			plateList := plateList.next
		END
	END RemoveSelection;

	PROCEDURE (m: Model) RoundToGrid* (grid: INTEGER), NEW;
		VAR
			nodeList: NodeList;
			plateList: PlateList;
	BEGIN
		nodeList := m.nodeList;
		WHILE nodeList # NIL DO
			nodeList.node.RoundToGrid(grid);
			nodeList := nodeList.next
		END;
		plateList := m.plateList;
		WHILE plateList # NIL DO
			plateList.plate.RoundToGrid(grid);
			plateList := plateList.next
		END
	END RoundToGrid;

	PROCEDURE (m: Model) Scale* (scale0, scale1: INTEGER), NEW;
		VAR
			scaleFactor: REAL;
			nodeList: NodeList;
			plateList: PlateList;
	BEGIN
		scaleFactor := 1.0 * scale1 / scale0;
		nodeList := m.nodeList;
		WHILE nodeList # NIL DO
			nodeList.node.Scale(scaleFactor);
			nodeList := nodeList.next
		END;
		plateList := m.plateList;
		WHILE plateList # NIL DO
			plateList.plate.Scale(scaleFactor);
			plateList := plateList.next
		END
	END Scale;

	PROCEDURE (m: Model) SelectedNode* (): DoodleNodes.Node, NEW;
		VAR
			node: DoodleNodes.Node;
			nodeList: NodeList;
	BEGIN
		nodeList := m.nodeList;
		LOOP
			IF nodeList = NIL THEN
				node := NIL;
				EXIT
			END;
			node := nodeList.node;
			IF node.selected THEN
				EXIT
			END;
			nodeList := nodeList.next
		END;
		RETURN node
	END SelectedNode;

	PROCEDURE (m: Model) SelectedPlate* (): DoodlePlates.Plate, NEW;
		VAR
			plate: DoodlePlates.Plate;
			plateList: PlateList;
	BEGIN
		plateList := m.plateList;
		LOOP
			IF plateList = NIL THEN
				plate := NIL;
				EXIT
			END;
			plate := plateList.plate;
			IF plate.selected THEN
				EXIT
			END;
			plateList := plateList.next
		END;
		RETURN plate
	END SelectedPlate;
	
	PROCEDURE New* (): Model;
		VAR
			model: Model;
	BEGIN
		NEW(model);
		model.nodeList := NIL;
		model.plateList := NIL;
		RETURN model
	END New;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END DoodleModels.

