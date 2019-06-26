(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"


*)

MODULE ParallelDebug;

	

	IMPORT
		SYSTEM,
		Dialog, Files := Files64, Ports, Stores := Stores64, Strings, Views,
		TextMappers, TextModels,
		BugsComponents, BugsDialog, BugsFiles, BugsIndex, BugsInterface, BugsParallel,
		BugsRandnum,
		DevDebug,
		GraphStochastic,
		ParallelActions;

	TYPE
		DialogBox* = POINTER TO RECORD(BugsDialog.DialogBox)
			workersPerChain*, rank*: INTEGER
		END;

	VAR
		dialog*: DialogBox;
		version-: INTEGER; 	(*	version number	*)
		maintainer-: ARRAY 40 OF CHAR; 	(*	person maintaining module	*)

	PROCEDURE (dialog: DialogBox) Init-;
	BEGIN
		dialog.workersPerChain := 2; dialog.rank := 0
	END Init;

	PROCEDURE (dialog: DialogBox) Update-;
	BEGIN
		Dialog.Update(dialog)
	END Update;

	PROCEDURE MapGraphAddress (adr: INTEGER; OUT label: ARRAY OF CHAR);
		VAR
			a, row, col: INTEGER;
			p: GraphStochastic.Node;
			label0: ARRAY 128 OF CHAR;
	BEGIN
		ParallelActions.FindStochasticPointer(adr, row, col);
		IF row >= 0 THEN
			p := BugsParallel.Stochastic(col, row);
			a := SYSTEM.VAL(INTEGER, p);
			BugsIndex.MapGraphAddress(a, label0);
			IF label0[0] = "[" THEN (*	maybe auxillary variable	*)
				IF p.children # NIL THEN (*	try and get some more information	*)
					a := SYSTEM.VAL(INTEGER, p.children[0]);
					BugsIndex.MapGraphAddress(a, label0);
					label0[0] := "_"; label0[LEN(label0$) - 1] := ">";
					label0 := "<aux" + label0
				END
			END
		ELSE
			a := ParallelActions.MapNamedPointer(adr);
			BugsIndex.MapGraphAddress(a, label0)
		END;
		label := label0$
	END MapGraphAddress;

	PROCEDURE Distribute* (rank, workersPerChain: INTEGER);
		VAR
			f: Files.File;
			restartLoc: Files.Locator;
			adr, i, j, numRows: INTEGER;
			devianceExists, seperable: BOOLEAN;
			rd: Stores.Reader;
			pos: POINTER TO ARRAY OF LONGINT;
			p: GraphStochastic.Node;
			v: Views.View;
			label: ARRAY 64 OF CHAR;
			form: TextMappers.Formatter;
			text: TextModels.Model;
			tabs: POINTER TO ARRAY OF INTEGER;
			newAttr, oldAttr: TextModels.Attributes;
		CONST
			space = 35 * Ports.mm;
			numChains = 1;
			chain = 0;
	BEGIN
		restartLoc := Files.dir.This("Restart");
		f := Files.dir.New(restartLoc, Files.dontAsk);
		(*	set the debug flag so that pointer name info is written in the .bug file	*)
		BugsParallel.debug := TRUE;
		BugsComponents.WriteModel(f, workersPerChain, numChains);
		BugsParallel.debug := FALSE;
		rd.ConnectTo(f);
		rd.SetPos(0);
		rd.ReadBool(devianceExists);
		BugsRandnum.InternalizeRNGenerators(rd);
		rd.ReadBool(seperable);
		NEW(pos, workersPerChain);
		i := 0; WHILE i < workersPerChain DO rd.ReadLong(pos[i]); INC(i) END;
		rd.SetPos(pos[rank]);
		ParallelActions.Read(chain, rd);
		rd.ConnectTo(NIL);
		f.Close;
		f := NIL;
		NEW(tabs, 2 + workersPerChain);
		tabs[0] := 0;
		tabs[1] := 5 * Ports.mm;
		i := 2; WHILE i < LEN(tabs) DO tabs[i] := tabs[i - 1] + space; INC(i) END;
		IF ParallelActions.globalStochs # NIL THEN
			numRows := LEN(ParallelActions.globalStochs[0]);
			text := TextModels.dir.New();
			form.ConnectTo(text);
			form.SetPos(0);
			BugsFiles.WriteRuler(tabs, form);
			i := 0;
			form.WriteString("Number of workers per chain: ");
			form.WriteInt(workersPerChain);
			form.WriteLn;
			form.WriteString("Shards are ");
			IF ~seperable THEN form.WriteString("not ") END; form.WriteString("seperable");
			form.WriteLn;
			form.WriteLn;
			form.WriteTab;
			form.WriteTab;
			WHILE i < workersPerChain DO
				form.WriteString("#");
				form.WriteInt(i + 1);
				form.WriteTab;
				INC(i)
			END;
			form.WriteLn;
			oldAttr := form.rider.attr;
			newAttr := TextModels.NewColor(oldAttr, Ports.grey50);
			i := 0;
			WHILE i < numRows DO
				j := 0;
				form.WriteTab;
				WHILE j < workersPerChain DO
					p := ParallelActions.globalStochs[j, i];
					adr := SYSTEM.VAL(INTEGER, p);
					MapGraphAddress(adr, label);
					v := DevDebug.HeapRefView(adr, label);
					IF label[0] = "[" THEN
						label := "dummy"
					ELSE
						label[0] := " "; label[LEN(label$) - 1] := 0X
					END;
					form.WriteTab; form.WriteView(v);
					IF (j # rank) & ~(GraphStochastic.distributed IN p.props) THEN form.rider.SetAttr(newAttr) END;
					form.WriteString(label);
					IF (j # rank) & ~(GraphStochastic.distributed IN p.props) THEN form.rider.SetAttr(oldAttr) END;
					INC(j)
				END;
				form.WriteLn;
				INC(i)
			END;
			Strings.IntToString(rank, label);
			BugsFiles.Open("Distribution info for worker " + label, text)
		END;
	END Distribute;

	PROCEDURE ShowDistribution*;
		VAR
			rank, workersPerChain: INTEGER;
	BEGIN
		workersPerChain := dialog.workersPerChain;
		rank := dialog.rank;
		Distribute(rank, workersPerChain)
	END ShowDistribution;

	PROCEDURE Update* (iterations: INTEGER);
		VAR
			res: SET;
	BEGIN
		REPEAT
			ParallelActions.Update(FALSE, FALSE, res); DEC(iterations)
		UNTIL iterations = 0
	END Update;

	PROCEDURE Guard* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~BugsInterface.IsInitialized()
	END Guard;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		DevDebug.MapHex := MapGraphAddress;
		NEW(dialog);
		dialog.workersPerChain := 2;
		dialog.rank := 0;
		BugsDialog.AddDialog(dialog);
		BugsDialog.UpdateDialogs
	END Init;

BEGIN
	Init
END ParallelDebug.

"ParallelDebug.Update(1)"

