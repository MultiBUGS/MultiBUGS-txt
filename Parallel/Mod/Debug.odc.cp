(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"


*)

MODULE ParallelDebug;

	

	IMPORT
		SYSTEM, Dialog,
		Meta, Ports, Views, 
		TextMappers, TextModels, 
		BugsDialog, BugsFiles, BugsIndex, BugsMsg, 
		GraphNodes,
		GraphStochastic, ParallelActions,
		UpdaterParallel, UpdaterUpdaters;

	TYPE
		DialogBox* = POINTER TO RECORD(BugsDialog.DialogBox)
			numProc*, rank*: INTEGER;
		END;

	VAR
		dialog*: DialogBox;

		version-: INTEGER; 	(*	version number	*)
		maintainer-: ARRAY 40 OF CHAR; 	(*	person maintaining module	*)

	PROCEDURE InitDialog;
	BEGIN
		dialog.numProc := 2;
		dialog.rank := 0
	END InitDialog;

	PROCEDURE (dialogBox: DialogBox) Init-;
	BEGIN
		InitDialog
	END Init;

	PROCEDURE (dialogBox: DialogBox) Update-;
	BEGIN
		Dialog.Update(dialog)
	END Update;

	PROCEDURE ModifyModel (numProc, rank: INTEGER);
		VAR
			updaters: POINTER TO ARRAY OF UpdaterUpdaters.Vector;
			deviances: POINTER TO ARRAY OF GraphStochastic.Vector;
			id: POINTER TO ARRAY OF INTEGER;
		CONST
			chain = 0;
	BEGIN
		UpdaterParallel.ModifyUpdaters(numProc, rank, chain, updaters, id, deviances);
		ParallelActions.ConfigureModel(updaters, id, deviances, rank)
	END ModifyModel;

	PROCEDURE MethodsState (numProc, rank: INTEGER; VAR f: TextMappers.Formatter);
		VAR
			adr, i, j, len, numUpdaters, size: INTEGER;
			label, string: ARRAY 128 OF CHAR;
			p: GraphNodes.Node;
			block: GraphStochastic.Vector;
			v: Views.View;
			item, item0: Meta.Item;
			ok: BOOLEAN;
			heapRefView: RECORD(Meta.Value)
				Do: PROCEDURE (adr: INTEGER; name: ARRAY OF CHAR): Views.View
			END;
			updater: UpdaterUpdaters.Updater;
	BEGIN
		Meta.Lookup("DevDebug", item);
		ok := item.obj = Meta.modObj;
		IF ok THEN
			item.Lookup("HeapRefView", item0);
			IF item0.obj = Meta.procObj THEN
				item0.GetVal(heapRefView, ok)
			END
		END;
		IF ~ok THEN RETURN END;
		ModifyModel(numProc, rank);
		numUpdaters := ParallelActions.NumUpdaters();
		i := 0;
		WHILE i < numUpdaters DO
			f.WriteTab;
			updater := ParallelActions.GetUpdater(i);
			block := ParallelActions.GetBlock(i);
			p := updater.Prior(0);
			IF p # NIL THEN
				BugsIndex.FindGraphNode(p, label);
				len := LEN(label$);
				IF label[len - 1] = ">" THEN label[len - 1] := 0X END;
				IF label[0] = "<" THEN label[0] := " " END;
				IF GraphStochastic.distributed IN p.props THEN
					label := label + "$"
				END;
			ELSE
				label := " -"
			END;
			f.WriteString(label);
			f.WriteTab;
			updater := ParallelActions.GetUpdater(i);
			updater.Install(string);
			BugsMsg.Lookup(string, string);
			f.WriteString(string);
			adr := SYSTEM.VAL(INTEGER, updater);
			v := heapRefView.Do(adr, label);
			f.WriteTab;
			f.WriteView(v);
			IF block # NIL THEN
				adr := SYSTEM.VAL(INTEGER, block);
				v := heapRefView.Do(adr, label + " block");
				f.WriteTab;
				f.WriteView(v);
			END;
			f.WriteLn;
			size := updater.Size();
			j := 1;
			WHILE j < size DO
				p := updater.Prior(j);
				BugsIndex.FindGraphNode(p, label);
				len := LEN(label$);
				IF label[len - 1] = ">" THEN label[len - 1] := 0X END;
				IF label[0] = "<" THEN label[0] := " " END;
				IF GraphStochastic.distributed IN p.props THEN
					label := label + "$"
				END;
				f.WriteTab; f.WriteString(" " + label); f.WriteLn;
				INC(j)
			END;
			INC(i);
		END
	END MethodsState;

	PROCEDURE Methods*;
		VAR
			tabs: ARRAY 5 OF INTEGER;
			f: TextMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		tabs[0] := 5 * Ports.mm;
		tabs[1] := 35 * Ports.mm;
		tabs[2] := 85 * Ports.mm;
		tabs[3] := 95 * Ports.mm;
		tabs[4] := 100 * Ports.mm;
		BugsFiles.WriteRuler(tabs, f);
		f.WriteTab;
		f.WriteString("number processors: ");
		f.WriteInt(dialog.numProc);
		f.WriteString(" rank: ");
		f.WriteInt(dialog.rank);
		f.WriteLn;
		f.WriteLn;
		f.WriteTab;
		f.WriteString(" node");
		f.WriteTab;
		f.WriteString("updater");
		f.WriteLn;
		MethodsState(dialog.numProc, dialog.rank, f);
		BugsFiles.Open("Methods", text)
	END Methods;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		NEW(dialog);
		InitDialog;
		BugsDialog.AddDialog(dialog)
	END Init;

BEGIN
	Init
END ParallelDebug.
