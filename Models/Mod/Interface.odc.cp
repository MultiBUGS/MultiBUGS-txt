(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE ModelsInterface;


	

	IMPORT
		BugsIndex, BugsNames,
		ModelsIndex, ModelsMonitors,
		MonitorModel;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE IsStar* (IN string: ARRAY OF CHAR): BOOLEAN;
		VAR
			i: INTEGER;
	BEGIN
		i := 0;
		WHILE
			string[i] = " " DO
			INC(i)
		END;
		RETURN string[i] = "*"
	END IsStar;

	PROCEDURE Clear* (IN name: ARRAY OF CHAR; OUT ok: BOOLEAN);
		VAR
			node: BugsNames.Name;
			monitor: ModelsMonitors.Monitor;
	BEGIN
		IF IsStar(name) THEN
			ok := TRUE; ModelsIndex.Clear
		ELSE
			ok := TRUE;
			node := BugsIndex.Find(name);
			IF node = NIL THEN
				ok := FALSE;
				RETURN
			END;
			monitor := ModelsIndex.Find(name);
			IF monitor = NIL THEN
				ok := FALSE;
				RETURN
			END;
			ModelsIndex.DeRegister(monitor)
		END
	END Clear;

	PROCEDURE ClearNI* (IN name: ARRAY OF CHAR);
		VAR
			ok: BOOLEAN;
	BEGIN
		Clear(name, ok)
	END ClearNI;

	PROCEDURE SampleSize* (IN name: ARRAY OF CHAR): INTEGER;
		VAR
			sampleSize: INTEGER;
			node: BugsNames.Name;
			monitor: ModelsMonitors.Monitor;
	BEGIN
		node := BugsIndex.Find(name);
		IF node = NIL THEN RETURN 0 END;
		monitor := ModelsIndex.Find(name);
		IF monitor = NIL THEN RETURN 0 END;
		sampleSize := monitor.SampleSize();
		RETURN sampleSize
	END SampleSize;

	PROCEDURE ComponentProbs* (IN name: ARRAY OF CHAR; OUT comProbs: POINTER TO ARRAY OF REAL);
		VAR
			sampleSize, numComponents: INTEGER;
			monitor: ModelsMonitors.Monitor;
	BEGIN
		sampleSize := SampleSize(name);
		IF sampleSize = 0 THEN RETURN END;
		monitor := ModelsIndex.Find(name);
		numComponents := monitor.Name().Size();
		NEW(comProbs, numComponents);
		monitor.ComponentProbs(comProbs)
	END ComponentProbs;

	PROCEDURE ModelProbs* (IN name: ARRAY OF CHAR;
	OUT models: POINTER TO ARRAY OF MonitorModel.Model;
	OUT modelProbs: POINTER TO ARRAY OF REAL);
		VAR
			sampleSize: INTEGER;
			monitor: ModelsMonitors.Monitor;
	BEGIN
		sampleSize := SampleSize(name);
		models := NIL;
		modelProbs := NIL;
		IF sampleSize = 0 THEN RETURN END;
		monitor := ModelsIndex.Find(name);
		monitor.ModelProbs(models, modelProbs)
	END ModelProbs;

	PROCEDURE NumberComponents* (IN name: ARRAY OF CHAR): INTEGER;
		VAR
			numberComponents: INTEGER;
			node: BugsNames.Name;
			monitor: ModelsMonitors.Monitor;
	BEGIN
		node := BugsIndex.Find(name);
		IF node = NIL THEN RETURN 0 END;
		monitor := ModelsIndex.Find(name);
		IF monitor = NIL THEN RETURN 0 END;
		numberComponents := monitor.NumberComponents();
		RETURN numberComponents
	END NumberComponents;

	PROCEDURE Set* (IN name: ARRAY OF CHAR; OUT ok: BOOLEAN);
		VAR
			node: BugsNames.Name;
			monitor: ModelsMonitors.Monitor;
	BEGIN
		ok := TRUE;
		node := BugsIndex.Find(name);
		IF node = NIL THEN ok := FALSE; RETURN END;
		IF node.numSlots = 0 THEN ok := FALSE; RETURN END;
		monitor := ModelsMonitors.fact.New(node);
		ModelsIndex.Register(monitor);
	END Set;

	PROCEDURE SetNI* (IN name: ARRAY OF CHAR);
		VAR
			ok: BOOLEAN;
	BEGIN
		Set(name, ok)
	END SetNI;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END ModelsInterface.
