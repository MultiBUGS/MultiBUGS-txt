(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsComponents;

	

	IMPORT
		Files, Kernel, Meta, Services, Stores, Strings,
		BugsIndex, BugsMsg, BugsRandnum, BugsSerialize,
		GraphNodes, GraphStochastic,
		UpdaterActions, UpdaterMethods, UpdaterUpdaters;

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

	PROCEDURE Modules* (mpiImplementation: ARRAY OF CHAR): POINTER TO ARRAY OF Files.Name;
		VAR
			name, timeStamp: ARRAY 128 OF CHAR;
			index, pos, numMod: INTEGER;
			list, cursor, cursor1, modList: List;
			modItem: Meta.Item;
			modules: POINTER TO ARRAY OF Files.Name;

		PROCEDURE Imports (name: ARRAY OF CHAR);
			VAR
				mod: Kernel.Module;
				i: INTEGER;
				element: List;
		BEGIN
			mod := Kernel.modList;
			WHILE (mod # NIL) & (mod.name # name) DO mod := mod.next END;
			IF mod # NIL THEN
				NEW(element);
				element.name := name$;
				element.next := list;
				list := element;
				i := 0;
				WHILE i < mod.nofimps DO
					IF (mod.imports[i] # NIL) & (mod.imports[i].name # "Kernel") THEN
						Imports(LONG(mod.imports[i].name))
					END;
					INC(i)
				END
			END
		END Imports;

		PROCEDURE AddModule (name: Files.Name; VAR modList: List);
			VAR
				entry: List;
		BEGIN
			NEW(entry);
			entry.name := name;
			entry.next := modList;
			modList := entry
		END AddModule;

	BEGIN
		list := NIL;
		modList := NIL;

		(*	get used Updater modules	*)
		index := 0;
		UpdaterUpdaters.GetInstallProc(index, name);
		WHILE name # "" DO
			Strings.Find(name, ".", 0, pos);
			name[pos] := 0X;
			Imports(name);
			INC(index);
			UpdaterUpdaters.GetInstallProc(index, name);
		END;

		(*	get used Graph modules	*)
		index := 0;
		GraphNodes.GetInstallProc(index, name);
		WHILE name # "" DO
			Strings.Find(name, ".", 0, pos);
			name[pos] := 0X;
			Imports(name);
			INC(index);
			GraphNodes.GetInstallProc(index, name);
		END;

		Meta.Lookup("MPIworker", modItem);
		Imports("MPIworker");
		Imports("ParallelUpdaters");
		Meta.Lookup("ParallelActions", modItem);
		Imports("ParallelActions");

		AddModule("Kernel+", modList);

		(*	remove duplicate module names from linker script	*)
		cursor := list;
		WHILE cursor # NIL DO
			cursor1 := modList;
			WHILE (cursor1 # NIL) & (cursor1.name # cursor.name) DO
				cursor1 := cursor1.next
			END;
			IF cursor1 = NIL THEN
				AddModule(cursor.name, modList);
				IF cursor.name = "Files" THEN
					AddModule("HostFiles", modList)
				END;
			END;
			cursor := cursor.next
		END;

		AddModule(mpiImplementation$, modList);
		Strings.IntToString(GraphNodes.timeStamp, timeStamp);
		AddModule("DynamicTime_" + timeStamp, modList);
		AddModule("GraphCenTrunc", modList);
		AddModule("ParallelTraphandler", modList);
		AddModule("MonitorMonitors", modList);
		AddModule("Services", modList);
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
			updater := UpdaterActions.GetUpdater(0, i);
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
							updater := UpdaterActions.GetUpdater(j, i);
							updater.LoadSample;
							updater := UpdaterUpdaters.CopyFrom(newUpdater);
							updater.StoreSample;
							UpdaterActions.SetUpdater(j, i, updater);
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

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END BugsComponents.

