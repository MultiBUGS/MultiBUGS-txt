(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



	   *)

(*

Models are stored in a AVL tree. The model configuration is represented as an array of BOOLEAN
"key" and how often the configuration occurs as an INTEGER "count".

A key already in the AVL tree can be found in O(n) operations  where n is the number of leafs of the tree. A new key can also be inserted into the tree in O(n) operations.

*)

MODULE MonitorModel;

	

	IMPORT
		Stores,
		GraphNodes;

	TYPE

		Key = ARRAY OF BOOLEAN;

		Model* = POINTER TO LIMITED RECORD
			key: POINTER TO Key;
			bal, count: INTEGER;
			left, right: Model
		END;

		Monitor* = POINTER TO ABSTRACT RECORD END;

		StdMonitor = POINTER TO RECORD(Monitor)
			nodes: GraphNodes.Vector;
			numModels, sampleSize: INTEGER;
			models: Model;
			key: POINTER TO Key;
		END;

		Factory* = POINTER TO ABSTRACT RECORD END;

		StdFactory = POINTER TO RECORD(Factory) END;

	CONST
		less = -1;
		equal = 0;
		greater = 1;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 20 OF CHAR;
		fact-: Factory;

	PROCEDURE (monitor: Monitor) ComponentProbs* (OUT probs: ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Copy* (): Monitor, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Externalize* (VAR wr: Stores.Writer), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Internalize* (VAR rd: Stores.Reader), NEW, ABSTRACT;

		PROCEDURE (monitor: Monitor) ModelProbs* (OUT models: POINTER TO ARRAY OF Model;
	OUT probs: POINTER TO ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) NumberComponents* (): INTEGER, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) SampleSize* (): INTEGER, NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Store* (IN key: ARRAY OF BOOLEAN; count: INTEGER), NEW, ABSTRACT;

	PROCEDURE (monitor: Monitor) Update*, NEW, ABSTRACT;

	PROCEDURE (f: Factory) New* (nodes: GraphNodes.Vector): Monitor, NEW, ABSTRACT;

	PROCEDURE Compare (IN key1, key2: Key): INTEGER;
		VAR
			i, len: INTEGER;
	BEGIN
		i := 0;
		len := LEN(key1);
		WHILE (i < len) & (key1[i] = key2[i])DO INC(i) END;
		IF i = len THEN RETURN equal
		ELSIF key1[i] THEN RETURN greater
		ELSE RETURN less
		END
	END Compare;

	PROCEDURE Store (VAR t: Model; VAR h: BOOLEAN; VAR count: INTEGER; IN key: Key);
		VAR
			compare, i, len: INTEGER;
			t1, t2: Model;
	BEGIN
		IF t = NIL THEN
			len := LEN(key);
			NEW(t);
			NEW(t.key, len);
			i := 0; WHILE i < len DO t.key[i] := key[i]; INC(i) END;
			t.count := count; t.bal := equal; t.left := NIL; t.right := NIL;
		ELSE
			compare := Compare(t.key, key);
			CASE compare OF
			|greater:
				Store(t.left, h, count, key);
				IF h THEN
					CASE t.bal OF
					|greater:
						t.bal := equal; h := FALSE
					|equal:
						t.bal := less
					|less:
						t1 := t.left;
						IF t1.bal = less THEN
							t.left := t1.right; t1.right := t; t.bal := equal; t := t1
						ELSE
							t2 := t1.right;
							t1.right := t2.left; t2.left := t1;
							t.left := t2.right; t2.right := t;
							IF t2.bal = less THEN t.bal := greater ELSE t.bal := equal END;
							IF t2.bal = greater THEN t1.bal := less ELSE t1.bal := equal END;
							t := t2
						END
					END
				END
			|equal:
				t.count := t.count + count;
				count := t.count
			|less:
				Store(t.right, h, count, key);
				IF h THEN
					CASE t.bal OF
					|greater:
						t1 := t.right;
						IF t1.bal = greater THEN
							t.right := t1.left; t1.left := t; t.bal := equal; t := t1
						ELSE
							t2 := t1.left;
							t1.left := t2.right; t2.right := t1;
							t.right := t2.left; t2.left := t;
							IF t2.bal = greater THEN t.bal := less ELSE t.bal := equal END;
							IF t2.bal = less THEN t1.bal := greater ELSE t1.bal := equal END;
							t := t2
						END;
						t.bal := equal; h := FALSE
					|equal:
						t.bal := greater
					|less:
						t.bal := equal; h := FALSE
					END
				END
			END
		END
	END Store;

	PROCEDURE (monitor: StdMonitor) ComponentProbs (OUT probs: ARRAY OF REAL);
		VAR
			i, len, sampleSize: INTEGER;

		PROCEDURE Count (t: Model);
			VAR
				j: INTEGER;
		BEGIN
			j := 0;
			WHILE j < len DO
				IF t.key[j] THEN probs[j] := probs[j] + t.count END;
				INC(j)
			END;
			IF t.left # NIL THEN
				Count(t.left)
			END;
			IF t.right # NIL THEN
				Count(t.right)
			END
		END Count;

	BEGIN
		i := 0;
		len := LEN(monitor.key);
		WHILE i < len DO
			probs[i] := 0.0; INC(i)
		END;
		sampleSize := monitor.sampleSize;
		Count(monitor.models);
		i := 0;
		WHILE i < len DO
			probs[i] := probs[i] / sampleSize; INC(i)
		END;
	END ComponentProbs;

	PROCEDURE (monitor: StdMonitor) Copy (): Monitor;
		VAR
			m: StdMonitor;
			model: Model;
			h: BOOLEAN;

		PROCEDURE Write (t: Model);
		BEGIN
			IF t # NIL THEN
				Store(model, h, t.count, t.key);
				IF t.left # NIL THEN
					Write(t.left)
				END;
				IF t.right # NIL THEN
					Write(t.right)
				END
			END
		END Write;

	BEGIN
		NEW(m);
		m.nodes := monitor.nodes;
		m.numModels := monitor.numModels;
		m.sampleSize := monitor.sampleSize;
		m.key := monitor.key;
		Write(monitor.models);
		m.models := model;
		RETURN m
	END Copy;

	PROCEDURE (monitor: StdMonitor) Externalize (VAR wr: Stores.Writer);
		VAR
			len: INTEGER;

		PROCEDURE Write (t: Model);
			VAR
				j: INTEGER;
		BEGIN
			IF t # NIL THEN
				j := 0;
				WHILE j < len DO wr.WriteBool(t.key[j]); INC(j) END;
				wr.WriteInt(t.count);
				IF t.left # NIL THEN
					Write(t.left)
				END;
				IF t.right # NIL THEN
					Write(t.right)
				END
			END;
		END Write;

	BEGIN
		len := LEN(monitor.key);
		wr.WriteInt(len);
		wr.WriteInt(monitor.numModels);
		Write(monitor.models)
	END Externalize;

	PROCEDURE (monitor: StdMonitor) Internalize (VAR rd: Stores.Reader);
		VAR
			count, i, j, len, numModels, oldCount: INTEGER;
			h: BOOLEAN;
	BEGIN
		rd.ReadInt(len);
		rd.ReadInt(numModels);
		NEW(monitor.key, len);
		i := 0;
		WHILE i < numModels DO
			j := 0;
			WHILE j < len DO rd.ReadBool(monitor.key[j]); INC(j) END;
			rd.ReadInt(count);
			INC(monitor.sampleSize, count);
			oldCount := count;
			h := FALSE;
			Store(monitor.models, h, count, monitor.key);
			IF oldCount = count THEN INC(monitor.numModels) END;
			INC(i)
		END
	END Internalize;

	PROCEDURE (monitor: StdMonitor) ModelProbs (OUT models: POINTER TO ARRAY OF Model;
	OUT probs: POINTER TO ARRAY OF REAL);
		VAR
			i, numModels, sampleSize: INTEGER;

		PROCEDURE Fill (t: Model; VAR index: INTEGER);
		BEGIN
			models[index] := t;
			INC(index);
			IF t.left # NIL THEN
				Fill(t.left, index)
			END;
			IF t.right # NIL THEN
				Fill(t.right, index)
			END
		END Fill;

	BEGIN
		i := 0;
		sampleSize := monitor.sampleSize;
		numModels := monitor.numModels;
		NEW(models, numModels);
		NEW(probs, numModels);
		Fill(monitor.models, i);
		i := 0;
		WHILE i < numModels DO
			probs[i] := models[i].count / sampleSize; INC(i)
		END;
	END ModelProbs;

	PROCEDURE (monitor: StdMonitor) NumberComponents (): INTEGER;
	BEGIN
		RETURN LEN(monitor.key)
	END NumberComponents;

	PROCEDURE (monitor: StdMonitor) SampleSize (): INTEGER;
	BEGIN
		RETURN monitor.sampleSize
	END SampleSize;

	PROCEDURE (monitor: StdMonitor) Store (IN key: ARRAY OF BOOLEAN; count: INTEGER);
		VAR
			h: BOOLEAN;
			oldCount: INTEGER;
	BEGIN
		INC(monitor.sampleSize, count);
		oldCount := count;
		h := FALSE;
		Store(monitor.models, h, count, key);
		IF oldCount = count THEN INC(monitor.numModels) END;
	END Store;

	PROCEDURE (monitor: StdMonitor) Update;
		VAR
			count, i, len: INTEGER;
			h: BOOLEAN;
	BEGIN
		i := 0;
		len := LEN(monitor.key);
		h := FALSE;
		WHILE i < len DO
			monitor.key[i] := monitor.nodes[i].Value() > 0.5;
			INC(i)
		END;
		count := 1;
		Store(monitor.models, h, count, monitor.key);
		IF count = 1 THEN INC(monitor.numModels) END;
		INC(monitor.sampleSize);
	END Update;

	PROCEDURE (f: StdFactory) New (nodes: GraphNodes.Vector): Monitor;
		VAR
			monitor: StdMonitor;
			i, len: INTEGER;
	BEGIN
		ASSERT(nodes # NIL, 20);
		len := LEN(nodes);
		i := 0;
		WHILE i < len DO ASSERT(nodes[i] # NIL, 21); INC(i) END;
		NEW(monitor);
		monitor.nodes := nodes;
		NEW(monitor.key, len);
		monitor.sampleSize := 0;
		monitor.numModels := 0;
		monitor.models := NIL;
		RETURN monitor
	END New;

	PROCEDURE (model: Model) Display* (OUT state: ARRAY OF BOOLEAN), NEW;
		VAR
			i, len: INTEGER;
	BEGIN
		i := 0;
		len := LEN(model.key);
		WHILE i < len DO state[i] := model.key[i]; INC(i) END
	END Display;

	PROCEDURE SetFactory* (f: Factory);
	BEGIN
		fact := f
	END SetFactory;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: StdFactory;
	BEGIN
		Maintainer;
		NEW(f);
		fact := f
	END Init;

BEGIN
	Init
END MonitorModel.
