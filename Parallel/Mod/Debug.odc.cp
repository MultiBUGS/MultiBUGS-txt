(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"


*)

MODULE ParallelDebug;

	

	IMPORT
		SYSTEM, StdLog,
		Dialog, Files, Meta, Ports, Stores, Views,
		BugsDialog, BugsIndex, BugsMappers, BugsMsg, BugsSerialize, 
		GraphNodes, GraphStochastic,
		ParallelActions,
		TextMappers, TextModels,
		UpdaterActions, UpdaterParallel, UpdaterUpdaters;

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

	PROCEDURE WriteGraph (numChains: INTEGER; VAR wr: Stores.Writer);
	BEGIN
		wr.WriteInt(numChains);
		UpdaterActions.MarkDistributed;
		UpdaterActions.UnMarkDistributed;
		BugsSerialize.ExternalizeGraph(wr);
		UpdaterActions.ExternalizeUpdaterData(wr)
	END WriteGraph;

	PROCEDURE ModifyModel (numProc, rank: INTEGER);
		VAR
			updaters: POINTER TO ARRAY OF UpdaterUpdaters.Vector;
			deviances: POINTER TO ARRAY OF GraphStochastic.Vector;
			id: POINTER TO ARRAY OF INTEGER;
			f: Files.File;
			loc: Files.Locator;
			wr: Stores.Writer;
			rd: Stores.Reader;
			numChains: INTEGER;
		CONST
			chain = 0;
	BEGIN
		(*numChains := UpdaterActions.NumChains();
		(*	test externalize / internalize	*)
		loc := Files.dir.This("");
		f := Files.dir.New(loc, Files.dontAsk);
		wr.ConnectTo(f);
		wr.SetPos(0);
		WriteGraph(numChains, wr);
		StdLog.String("File length is "); StdLog.Int(f.Length()); StdLog.Ln;
		rd.ConnectTo(f);
		rd.SetPos(0);
		rd.ReadInt(numChains);
		StdLog.String("Number of chains is "); StdLog.Int(numChains); StdLog.Ln;
		UpdaterParallel.ReadGraph(0, rd);*)
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
		(*f.WriteRuler(tabs);*)
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
		(*IF f.lines > 1 THEN
			f.Register("Updater types")
		END*)
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
