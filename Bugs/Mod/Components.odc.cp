(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsComponents;

	

	IMPORT
		Files, Kernel, Meta, Services, Stores, Strings,
		BugsGraph, BugsIndex, BugsMsg, BugsRandnum, BugsSerialize,
		GraphNodes, GraphStochastic,
		UpdaterActions, UpdaterMethods, UpdaterParallel, UpdaterUpdaters;

	TYPE
		List = POINTER TO RECORD
			name: Files.Name;
			next: List
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE ListToVector (list: List): POINTER TO ARRAY OF Files.Name;
		VAR
			cursor: List;
			len: INTEGER;
			vector: POINTER TO ARRAY OF Files.Name;
	BEGIN
		vector := NIL;
		cursor := list;
		len := 0;
		WHILE cursor # NIL DO
			INC(len); cursor := cursor.next
		END;
		IF len > 0 THEN NEW(vector, len) END;
		cursor := list;
		WHILE cursor # NIL DO
			DEC(len);
			vector[len] := cursor.name;
			cursor := cursor.next
		END;
		RETURN vector;
	END ListToVector;
		
	PROCEDURE AddModule (name: Files.Name; VAR modList: List);
		VAR
			entry: List;
	BEGIN
		NEW(entry);
		entry.name := name;
		entry.next := modList;
		modList := entry;
		IF name = "Files" THEN
			AddModule("HostFiles", modList)
		END
	END AddModule;

	PROCEDURE IsNew (name: Files.Name; modList: List): BOOLEAN;
		VAR
			cursor: List;
	BEGIN
		cursor := modList;
		WHILE (cursor # NIL) & (cursor.name # name) DO cursor := cursor.next END;
		RETURN cursor = NIL
	END IsNew;

	PROCEDURE Imports (name: Files.Name; VAR modList: List);
		VAR
			mod: Kernel.Module;
			i: INTEGER;
			ok: BOOLEAN;
	BEGIN
		IF name = "Kernel" THEN RETURN END;
		mod := Kernel.modList;
		WHILE (mod # NIL) & (mod.name # name) DO mod := mod.next END;
		IF mod # NIL THEN
			ok := IsNew(name, modList);
			IF ok THEN
				i := 0;
				WHILE i < mod.nofimps DO
					IF (mod.imports[i] # NIL) & (mod.imports[i].name # "Kernel") THEN
						Imports(LONG(mod.imports[i].name), modList)
					END;
					INC(i)
				END;
				AddModule(name, modList)
			END
		END
	END Imports;
	
	PROCEDURE ImportList (file: Files.File): POINTER TO ARRAY OF Files.Name;
		VAR
			i, num, p: INTEGER;
			imports: POINTER TO ARRAY OF Files.Name;
			rd: Files.Reader;
			name: Files.Name;
					
		PROCEDURE RWord (VAR x: INTEGER);
			VAR b: BYTE; y: INTEGER;
		BEGIN
			rd.ReadByte(b); y := b MOD 256;
			rd.ReadByte(b); y := y + 100H * (b MOD 256);
			rd.ReadByte(b); y := y + 10000H * (b MOD 256);
			rd.ReadByte(b); x := y + 1000000H * b
		END RWord;

		PROCEDURE RNum (VAR x: INTEGER);
			VAR b: BYTE; s, y: INTEGER;
		BEGIN
			s := 0; y := 0; rd.ReadByte(b);
			WHILE b < 0 DO INC(y, ASH(b + 128, s)); INC(s, 7); rd.ReadByte(b) END;
			x := ASH((b + 64) MOD 128 - 64, s) + y
		END RNum;

		PROCEDURE RName (VAR name: ARRAY OF CHAR);
			VAR b: BYTE; i, n: INTEGER;
		BEGIN
			i := 0; n := LEN(name) - 1; rd.ReadByte(b);
			WHILE (i < n) & (b # 0) DO name[i] := CHR(b); INC(i); rd.ReadByte(b) END;
			WHILE b # 0 DO rd.ReadByte(b) END;
			name[i] := 0X
		END RName;

	BEGIN
		imports := NIL;
		rd := file.NewReader(NIL);
		rd.SetPos(0); RWord(p);
		IF p = 6F4F4346H THEN
			RWord(p); RWord(p); RWord(p); RWord(p); RWord(p); RWord(p);
			RNum(num); 
			NEW(imports, num);
			RName(name); 
			i := 0;
			WHILE i < num DO RName(imports[i]); INC(i) END;
		END;
		file.Close;
		RETURN imports
	END ImportList;
	
	PROCEDURE Modules* (mpiImplementation: ARRAY OF CHAR): POINTER TO ARRAY OF Files.Name;
		VAR
			name, timeStamp: Files.Name;
			i, index, pos, numMod: INTEGER;
			modList: List;
			modules: POINTER TO ARRAY OF Files.Name;
			file: Files.File;
			imports: POINTER TO ARRAY OF Files.Name;
			loc: Files.Locator;
			mod: Meta.Item;
			
	BEGIN
		modList := NIL;
		
		(*	add Kernel to modList	*)
		AddModule("Kernel+", modList);
		
		(*	load some modules used by ParallelWorker	*)
		loc := Files.dir.This("Parallel");
		loc := loc.This("Code");
		file := Files.dir.Old(loc, "Worker.ocf", Files.shared);
		imports := ImportList(file);
		numMod := LEN(imports);
		i := 0;
		WHILE i < numMod DO 
			Meta.Lookup(imports[i], mod);
			Imports(imports[i], modList); 
			INC(i) 
		END; 

		(*	add time stamp to linker list but do not load	*)
		Strings.IntToString(GraphNodes.timeStamp, timeStamp);
		AddModule("DynamicTime_" + timeStamp, modList);
				
		(*	add mpi implementation to linker script but do not load	*)
		AddModule(mpiImplementation$, modList);

		(*	get used Updater modules	*)
		index := 0;
		UpdaterUpdaters.GetInstallProc(index, name);
		WHILE name # "" DO
			Strings.Find(name, ".", 0, pos);
			name[pos] := 0X;
			Imports(name, modList);
			INC(index);
			UpdaterUpdaters.GetInstallProc(index, name);
		END;

		(*	get used Graph modules	*)
		index := 0;
		GraphNodes.GetInstallProc(index, name);
		WHILE name # "" DO
			Strings.Find(name, ".", 0, pos);
			name[pos] := 0X;
			Imports(name, modList);
			INC(index);
			GraphNodes.GetInstallProc(index, name);
		END;

		(*	top level module for BUGS worker program but do not load	*)
		AddModule("ParallelWorker", modList);

		modules := ListToVector(modList); 
		RETURN modules
	END Modules;

	PROCEDURE Error (errorNum: INTEGER; name: ARRAY OF CHAR);
		VAR
			errorMsg: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
			numToString: ARRAY 8 OF CHAR;
	BEGIN
		Strings.IntToString(errorNum, numToString);
		p[0] := name$;
		BugsMsg.LookupParam("BugsParallel" + numToString, p, errorMsg);
		BugsMsg.Store(errorMsg)
	END Error;

	(*	check that for distributed updaters algorithm can be distributed and if not replace it with one that
	can	*)
	PROCEDURE ModifyUpdaterMethods (OUT ok: BOOLEAN);
		VAR
			newUpdater, updater: UpdaterUpdaters.Updater;
			prior, node: GraphStochastic.Node;
			i, j, numChains, numFact, numUpdaters, size: INTEGER;
			factory: UpdaterUpdaters.Factory;
			label: ARRAY 128 OF CHAR;
	BEGIN
		ok := TRUE;
		numChains := UpdaterActions.NumberChains();
		numUpdaters := UpdaterActions.NumberUpdaters();
		numFact := LEN(UpdaterMethods.factories);
		i := 0;
		WHILE i < numUpdaters DO
			updater := UpdaterActions.updaters[0, i];
			size := updater.Size();
			prior := updater.Prior(0);
			IF prior # NIL THEN
				IF GraphStochastic.distributed IN prior.props THEN
					(*	clear update mark from updaters priors	*)
					j := 0;
					WHILE j < size DO
						node := updater.Prior(j);
						node.SetProps(node.props - {GraphStochastic.update});
						INC(j)
					END;
					j := 0;
					newUpdater := NIL;
					WHILE (newUpdater = NIL) & (j < numFact) DO
						factory := UpdaterMethods.factories[j];
						IF (UpdaterUpdaters.enabled IN factory.props) & factory.CanUpdate(prior) THEN
							newUpdater := factory.New(prior);
						END;
						INC(j)
					END;
					IF newUpdater = NIL THEN
						ok := FALSE;
						BugsIndex.FindGraphNode(prior, label);
						Error(1, label);
						RETURN
					ELSIF ~Services.SameType(updater, newUpdater) THEN
						ASSERT(updater.Size() = size, 66);
						j := 0;
						WHILE j < numChains DO
							updater := UpdaterActions.updaters[j, i];
							updater.LoadSample;
							updater := UpdaterUpdaters.CopyFrom(newUpdater);
							updater.StoreSample;
							UpdaterActions.updaters[j, i] :=  updater;
							INC(j)
						END
					ELSE
						(*	the old updater is good so put update mark back on prior	*)
						j := 0;
						WHILE j < size DO
							node := updater.Prior(j);
							node.SetProps(node.props + {GraphStochastic.update});
							INC(j)
						END
					END
				END
			END; 
			INC(i)
		END
	END ModifyUpdaterMethods;

	PROCEDURE WriteModel* (f: Files.File; numChains: INTEGER; portName: ARRAY OF SHORTCHAR;
	OUT ok: BOOLEAN);
		VAR
			i, len: INTEGER;
			wr: Stores.Writer;
	BEGIN
		wr.ConnectTo(f);
		wr.SetPos(0);
		(*	copy port name into bugs files from where the workers can read it and establish connection
		with master	*)
		i := 0;
		len := LEN(portName);
		WHILE i < len DO
			wr.WriteSChar(portName[i]); INC(i)
		END;
		wr.WriteBool(BugsGraph.devianceExists);
		BugsRandnum.ExternalizeRNGenerators(wr);
		wr.WriteInt(numChains);
		UpdaterActions.MarkDistributed;
		ModifyUpdaterMethods(ok);
		IF ~ok THEN RETURN END;
		UpdaterActions.UnMarkDistributed;
		BugsSerialize.ExternalizeGraph(wr);
		UpdaterActions.ExternalizeUpdaterData(wr);
		f.Flush;
		wr.ConnectTo(NIL)
	END WriteModel;

	PROCEDURE Debug* (numChains: INTEGER);
		VAR
			f: Files.File;
			loc: Files.Locator;
			wr: Stores.Writer;
			rd: Stores.Reader;
			ok: BOOLEAN;
			numberChains: INTEGER;
	BEGIN
		loc := Files.dir.This("");
		f := Files.dir.New(loc, Files.exclusive);
		wr.ConnectTo(f);
		wr.SetPos(0);
		wr.WriteInt(numChains);
		UpdaterActions.MarkDistributed;
		ModifyUpdaterMethods(ok);
		IF ~ok THEN HALT(0) END;
		UpdaterActions.UnMarkDistributed;
		BugsSerialize.ExternalizeGraph(wr);
		UpdaterActions.ExternalizeUpdaterData(wr);
		rd.ConnectTo(f);
		rd.SetPos(0);
		rd.ReadInt(numberChains);
		UpdaterParallel.ReadGraph(0, rd);
		HALT(0)
	END Debug;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END BugsComponents.

"BugsComponents.Debug(1)"

