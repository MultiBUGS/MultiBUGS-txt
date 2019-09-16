(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsComponents;

	

	IMPORT
		Dialog, Files := Files64, Kernel, Meta, Services, Stores := Stores64, Strings,
		BugsGraph, BugsIndex, BugsMsg, BugsParallel, BugsRandnum,
		GraphGrammar, GraphNodes, GraphStochastic,
		UpdaterActions, UpdaterMethods, UpdaterUpdaters;

	TYPE
		List = POINTER TO RECORD
			name: Files.Name;
			next: List
		END;

	VAR
		graphModules, updaterModules: List;
		platform: Dialog.String;
		allThis-: BOOLEAN;
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
			mpiImp: Dialog.String;
	BEGIN
		NEW(entry);
		entry.name := name;
		entry.next := modList;
		modList := entry;
		IF name = "Files" THEN
			AddModule("HostFiles", modList)
		ELSIF name = "Files64" THEN
			AddModule("HostFiles64", modList)
		ELSIF name = "MathSparsematrix" THEN
			(*	add in sparse matrix library if found on this computer	*)
			IF Kernel.ThisLoadedMod("MathTaucsImp") # NIL THEN
				AddModule("MathTaucsImp", modList)
			END
		ELSIF name = "MPI" THEN
			(*	add implementation of MPI map name in string resource file	*)
			Dialog.MapString("#Bugs:MPI" + platform, mpiImp);
			AddModule(mpiImp$, modList);
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

	PROCEDURE AddModules;
		VAR
			name, name0: Files.Name;
			index, dot, open, close, plus: INTEGER;
			external: GraphGrammar.External;
	BEGIN
		(*	get used Graph modules	*)
		index := 0;
		GraphNodes.GetInstallProc(index, name);
		WHILE name # "" DO
			Strings.Find(name, ")", 0, close);
			IF close # - 1 THEN name[close] := 0X END;
			Strings.Find(name, "+", 0, plus);
			IF plus # - 1 THEN
				Strings.Extract(name, plus + 1, LEN(name), name0);
				external := GraphGrammar.FindDensity(name0);
				name0 := external.name$;
				Strings.Find(name0, ".", 0, dot);
				name0[dot] := 0X;
				IF IsNew(name0, graphModules) THEN
					AddModule(name0, graphModules)
				END;
				name[plus] := 0X
			END;
			Strings.Find(name, "(", 0, open);
			IF open # - 1 THEN
				Strings.Extract(name, open + 1, LEN(name), name0);
				external := GraphGrammar.FindDensity(name0);
				name0 := external.name$;
				Strings.Find(name0, ".", 0, dot);
				name0[dot] := 0X;
				IF IsNew(name0, graphModules) THEN
					AddModule(name0, graphModules)
				END;
			END;
			Strings.Find(name, ".", 0, dot);
			name[dot] := 0X;
			IF IsNew(name, graphModules) THEN
				AddModule(name, graphModules)
			END;
			INC(index);
			GraphNodes.GetInstallProc(index, name);
		END;
		(*	get used Updater modules	*)
		index := 0;
		UpdaterUpdaters.GetInstallProc(index, name);
		WHILE name # "" DO
			Strings.Find(name, ".", 0, dot);
			name[dot] := 0X;
			IF IsNew(name, updaterModules) THEN
				AddModule(name, updaterModules)
			END;
			INC(index);
			UpdaterUpdaters.GetInstallProc(index, name);
		END
	END AddModules;

	PROCEDURE Clear;
	BEGIN
		graphModules := NIL;
		updaterModules := NIL;
		allThis := TRUE
	END Clear;

	PROCEDURE Modules* (IN plat: ARRAY OF CHAR): POINTER TO ARRAY OF Files.Name;
		VAR
			i, numMod: INTEGER;
			modList: List;
			modules: POINTER TO ARRAY OF Files.Name;
			file: Files.File;
			imports: POINTER TO ARRAY OF Files.Name;
			loc: Files.Locator;
			mod: Meta.Item;
	BEGIN
		platform := plat$;
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

		WHILE graphModules # NIL DO
			Imports(graphModules.name, modList);
			graphModules := graphModules.next
		END;

		WHILE updaterModules # NIL DO
			Imports(updaterModules.name, modList);
			updaterModules := updaterModules.next
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
		BugsMsg.LookupParam("BugsComponents" + numToString, p, errorMsg);
		BugsMsg.StoreError(errorMsg)
	END Error;

	PROCEDURE WriteModel* (f: Files.File; workersPerChain, numberChains: INTEGER);
		VAR
			pos0: LONGINT;
			chain, rank: INTEGER;
			pos: POINTER TO ARRAY OF LONGINT;
			wr: Stores.Writer;
			this: BOOLEAN;
	BEGIN
		allThis := TRUE;
		wr.ConnectTo(f);
		wr.SetPos(0);
		wr.WriteBool(BugsGraph.devianceExists);
		BugsRandnum.ExternalizeRNGenerators(wr);
		UpdaterActions.MarkDistributed(workersPerChain);
		BugsParallel.Distribute(workersPerChain); 
		pos0 := wr.Pos();
		wr.WriteBool(allThis);
		NEW(pos, workersPerChain);
		rank := 0;
		WHILE rank < workersPerChain DO
			pos[rank] := - 1; wr.WriteLong(pos[rank]); INC(rank)
		END;
		rank := 0;
		WHILE rank < workersPerChain DO
			pos[rank] := wr.Pos();
			BugsParallel.Write(rank, numberChains, this, wr); 
			allThis := allThis & this;
			AddModules;
			INC(rank)
		END;
		chain := 0;
		WHILE chain < numberChains DO
			BugsParallel.WriteValues(chain, wr);
			INC(chain);
		END;
		wr.SetPos(pos0);
		wr.WriteBool(allThis);
		rank := 0; WHILE rank < workersPerChain DO wr.WriteLong(pos[rank]); INC(rank) END;
		f.Flush;
		wr.ConnectTo(NIL);
		UpdaterActions.UnMarkDistributed
	END WriteModel;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		Clear
	END Init;

BEGIN
	Init
END BugsComponents.

