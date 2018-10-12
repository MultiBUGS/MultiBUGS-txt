(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE DoodleCmds;


	

	IMPORT
		Controllers, Dialog, Models, Ports, Views,
		BugsCmds, BugsDialog, BugsFiles, BugsInterface, BugsMsg, BugsParser, DoodleModels,
		DoodleNodes, DoodleParser, DoodlePlates, DoodleViews, TextViews;

	TYPE
		Sizes* = RECORD
			n0*, n1*, d*: INTEGER
		END;

		Scale* = RECORD
			scaleFactor*: INTEGER
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		sizes*: Sizes;
		scale*: Scale;

	PROCEDURE Deposit*;
		VAR
			semiAxis: INTEGER;
			m: DoodleModels.Model;
			v: Views.View;
	BEGIN
		semiAxis := (sizes.d + 3) DIV 4;
		IF semiAxis < 1 THEN semiAxis := 1 END;
		m := DoodleModels.New();
		v := DoodleViews.New(sizes.n0, sizes.n1, semiAxis * Ports.mm, Ports.mm, m);
		Views.Deposit(v)
	END Deposit;

	PROCEDURE SetGrid* (grid: INTEGER);
		VAR
			v: Views.View;
			m: DoodleModels.Model;
			nodeList: DoodleModels.NodeList;
			plateList: DoodleModels.PlateList;
			node: DoodleNodes.Node;
			plate: DoodlePlates.Plate;
			umsg: Models.UpdateMsg;
	BEGIN
		v := Controllers.FocusView();
		IF v # NIL THEN
			WITH v: DoodleViews.View DO
				m := v.ThisModel();
				nodeList := m.nodeList;
				plateList := m.plateList;
				IF grid < 1 THEN
					v.SetGrid(1)
				ELSE
					v.SetGrid(grid * Ports.mm)
				END;
				Models.BeginModification(Views.notUndoable, m);
				WHILE nodeList # NIL DO
					node := nodeList.node;
					node.ForceToGrid(v.grid);
					nodeList := nodeList.next
				END;
				WHILE plateList # NIL DO
					plate := plateList.plate;
					plate.ForceToGrid(v.grid);
					plateList := plateList.next
				END;
				Models.EndModification(Views.notUndoable, m);
				Models.Broadcast(m, umsg)
			ELSE
			END
		END
	END SetGrid;

	PROCEDURE NewModel (OUT ok: BOOLEAN);
		VAR
			res: INTEGER;
	BEGIN
		IF BugsInterface.IsParsed() THEN
			Dialog.GetOK("#Bugs:CmdsNewModel", "", "", "", {Dialog.ok, Dialog.cancel}, res);
			ok := res = Dialog.ok
		ELSE
			ok := TRUE
		END
	END NewModel;

	PROCEDURE Parse*;
		VAR
			ok: BOOLEAN;
			model: BugsParser.Statement;
			doodle: DoodleModels.Model;
			v: Views.View;
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		v := Controllers.FocusView();
		IF v = NIL THEN RETURN END;
		IF ~(v IS DoodleViews.View) THEN RETURN END;
		NewModel(ok);
		IF ~ok THEN RETURN END;
		BugsCmds.Clear;
		BugsDialog.UpdateDialogs;
		doodle := v.ThisModel()(DoodleModels.Model);
		model := DoodleParser.ParseModel(doodle);
		BugsParser.SetModel(model);
		BugsParser.MarkVariables;
		IF model = NIL THEN
			DoodleParser.PrintError(doodle)
		ELSE
			BugsMsg.Lookup("BugsCmds:OkSyntax", msg);
			BugsFiles.ShowStatus(msg)
		END
	END Parse;

	PROCEDURE FocusGuard* (VAR par: Dialog.Par);
		VAR
			v: Views.View;
	BEGIN
		v := Controllers.FocusView();
		IF v # NIL THEN
			par.disabled := ~(v IS DoodleViews.View)
		ELSE
			par.disabled := TRUE
		END
	END FocusGuard;

	PROCEDURE GridGuard* (grid: INTEGER; VAR par: Dialog.Par);
		VAR
			v: Views.View;
	BEGIN
		v := Controllers.FocusView();
		IF v # NIL THEN
			par.disabled := ~(v IS DoodleViews.View);
			WITH v: DoodleViews.View DO
				par.checked := v.grid DIV Ports.mm = grid
			ELSE
			END
		ELSE
			par.disabled := TRUE
		END
	END GridGuard;

	PROCEDURE ModelGuard* (VAR par: Dialog.Par);
		VAR
			v: Views.View;
	BEGIN
		v := Controllers.FocusView();
		IF v # NIL THEN
			par.disabled := ~((v IS DoodleViews.View) OR (v IS TextViews.View))
		ELSE
			par.disabled := TRUE
		END
	END ModelGuard;

	PROCEDURE ScaleModel*;
		VAR
			scale0, scale1: INTEGER;
			m: DoodleModels.Model;
			v: Views.View;
			umsg: Models.UpdateMsg;
	BEGIN
		v := Controllers.FocusView();
		WITH v: DoodleViews.View DO
			m := v.ThisModel();
			scale0 := v.scale;
			scale1 := scale0 * scale.scaleFactor DIV 100;
			v.SetScale(scale1);
			Models.BeginModification(Views.notUndoable, m);
			m.Scale(scale0, scale1);
			m.RoundToGrid(v.grid);
			Models.EndModification(Views.notUndoable, m);
			Models.Broadcast(m, umsg)
		ELSE
		END
	END ScaleModel;

	PROCEDURE RemoveSelection*;
		VAR
			m: DoodleModels.Model;
			v: Views.View;
			umsg: Models.UpdateMsg;
	BEGIN
		v := Controllers.FocusView();
		WITH v: DoodleViews.View DO
			m := v.ThisModel();
			Models.BeginModification(Views.notUndoable, m);
			v.RemoveSelection;
			Models.EndModification(Views.notUndoable, m);
			Models.Broadcast(m, umsg)
		ELSE
		END
	END RemoveSelection;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		sizes.n0 := 160;
		sizes.n1 := 100;
		sizes.d := 20;
		scale.scaleFactor := 100
	END Init;

BEGIN
	Init
END DoodleCmds.

