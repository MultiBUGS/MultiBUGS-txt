(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



  *)

MODULE PlotsDialog;


	

	IMPORT
		Containers, Controllers, Dialog, Models, Properties, Services, Views,
		FormModels, 
		StdApi, StdTabViews, 
		TextCmds, TextMappers, TextModels, 
		PlotsViews;

	CONST 
		specialTabNum = 5; 

	TYPE
		DialogHandler = POINTER TO RECORD (Services.Action)
			era: INTEGER;
			current: Views.View
		END;

		SpecialDialog* = POINTER TO ABSTRACT RECORD END;

		(* currently only used to deal with apply button in Compare.Denstrip *) 
		SpecialApplyDialog* = POINTER TO ABSTRACT RECORD END;

	VAR
		plotProp*: PlotsViews.Property;
		handler: DialogHandler;

		modify*: RECORD
			margins*, title*, xTitle*, yTitle*, xBounds*, yBounds*: BOOLEAN
		END;

		tabView: StdTabViews.View;
		special: SpecialDialog;
		specialApply: SpecialApplyDialog; 

		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (s: SpecialDialog) Dialog* (): Views.View, NEW, ABSTRACT;

	PROCEDURE (s: SpecialDialog) Set* (prop: Properties.Property), NEW, ABSTRACT;

	PROCEDURE (s: SpecialApplyDialog) Apply* (), NEW, ABSTRACT;

	PROCEDURE (s: SpecialApplyDialog) Guard* (VAR par: Dialog.Par), NEW, ABSTRACT;

	PROCEDURE SetSpecial* (s: SpecialDialog);
	BEGIN
		special := s;
	END SetSpecial;

	PROCEDURE SetSpecialApply* (s: SpecialApplyDialog);
	BEGIN
		specialApply := s;
	END SetSpecialApply;

	PROCEDURE InstallSpecialDialog (v: Views.View);
		VAR
			name: ARRAY 128 OF CHAR;
			res: INTEGER;
	BEGIN
		special := NIL;
		IF v # NIL THEN
			Services.GetTypeName(v, name);
			name := name + "Install";
			Dialog.Call(name, "", res);
		END
	END InstallSpecialDialog;

	PROCEDURE Singleton* (): PlotsViews.View;
		VAR
			v: Views.View;
	BEGIN
		Controllers.SetCurrentPath(Controllers.targetPath);
		v := Containers.FocusSingleton();
		Controllers.ResetCurrentPath;
		IF v = NIL THEN
			v := Controllers.FocusView()
		END;
		IF v # NIL THEN
			IF v IS PlotsViews.View THEN
				RETURN v(PlotsViews.View)
			END
		END;
		RETURN NIL
	END Singleton;

	PROCEDURE (dh: DialogHandler) Collect, NEW;
		VAR
			prop: Properties.Property;
	BEGIN
		Properties.CollectProp(prop);
		dh.era := PlotsViews.era;
		WHILE prop # NIL DO
			IF prop IS PlotsViews.Property THEN
				plotProp^ := prop(PlotsViews.Property)^;
				Dialog.Update(plotProp)
			ELSIF special # NIL THEN
				special.Set(prop)
			END;
			prop := prop.next
		END;
	END Collect;

	PROCEDURE (dh: DialogHandler) Reschedule, NEW;
	BEGIN
		Services.DoLater(dh, Services.Ticks() + Services.resolution)
	END Reschedule;

	PROCEDURE (dh: DialogHandler) Do;
		VAR
			v: PlotsViews.View;
	BEGIN
		v := Singleton();
		IF v # dh.current THEN
			IF tabView # NIL THEN
				InstallSpecialDialog(v);
				IF special # NIL THEN
					tabView.SetItem(specialTabNum, "Special", special.Dialog())
				ELSE
					tabView.SetNofTabs(specialTabNum)
				END
			END;
			dh.current := v;
			dh.Collect;
		ELSIF PlotsViews.era # dh.era THEN
			dh.Collect
		END;
		dh.Reschedule
	END Do;

	PROCEDURE Apply*;
		VAR
			v: PlotsViews.View;
			name: ARRAY 128 OF CHAR;
			res: INTEGER;
	BEGIN
		v := Singleton();
		Controllers.SetCurrentPath(Controllers.targetPath);
		Properties.EmitProp(NIL, plotProp);
		Controllers.ResetCurrentPath;
		IF (tabView # NIL) & (special # NIL) THEN
			(* Only apply special properties if focus is currently on "Special" tab *) 
			IF (tabView.Index() = specialTabNum) & (specialApply # NIL) THEN 
				specialApply.Apply();
			END; 					
		END;
	END Apply;

	PROCEDURE ResetChecks;
	BEGIN
		modify.margins := FALSE;
		modify.title := FALSE;
		modify.xTitle := FALSE;
		modify.yTitle := FALSE;
		modify.xBounds := FALSE;
		modify.yBounds := FALSE
	END ResetChecks;

	PROCEDURE ExcludeUnwanted (VAR valid: SET);
	BEGIN
		IF ~modify.margins THEN
			valid := valid - PlotsViews.allMargins
		END;
		IF ~modify.title THEN
			valid := valid - {PlotsViews.propTitle}
		END;
		IF ~modify.xTitle THEN
			valid := valid - {PlotsViews.propXTitle}
		END;
		IF ~modify.yTitle THEN
			valid := valid - {PlotsViews.propYTitle}
		END;
		IF ~modify.xBounds THEN
			valid := valid - PlotsViews.allXBounds
		END;
		IF ~modify.yBounds THEN
			valid := valid - PlotsViews.allYBounds
		END
	END ExcludeUnwanted;

	PROCEDURE ApplyAll*;
		VAR
			opts, old: SET; dl, dt, dr, db: INTEGER;
			prop: PlotsViews.Property;
			source: PlotsViews.View;
			s: TextMappers.Scanner;
			m: Models.Model;
			target: Views.View;
	BEGIN
		source := Singleton(); ASSERT(source # NIL, 25);
		NEW(prop);
		prop^ := plotProp^;
		m := Controllers.FocusModel();
		ASSERT(m # NIL, 25); ASSERT(m IS TextModels.Model, 25);
		WITH m: TextModels.Model DO
			s.ConnectTo(m);
			s.SetPos(0);
			opts := s.opts;
			INCL(opts, TextMappers.returnViews);
			s.SetOpts(opts);
			WHILE ~s.rider.eot DO
				s.Scan;
				IF s.type = TextMappers.view THEN
					target := s.view; ASSERT(target # NIL, 25);
					IF Services.SameType(target, source) THEN
						WITH target: PlotsViews.View DO
							Views.BeginModification(Views.notUndoable, target);
							old := prop.valid;
							ExcludeUnwanted(prop.valid);
							PlotsViews.SetProperties(target, prop, dl, dt, dr, db);
							prop.valid := old;
							target.ModifySize(dl + dr, dt + db);
							PlotsViews.IncEra;
							Views.EndModification(Views.notUndoable, target);
							Views.Update(target, Views.keepFrames)
						END
					END
				END
			END
		END;
		ResetChecks; Dialog.Update(modify)
	END ApplyAll;

	PROCEDURE Data*;
		VAR
			v: PlotsViews.View;
	BEGIN
		v := Singleton();
		IF v # NIL THEN v.ShowData END;
	END Data;

	PROCEDURE ShowDataGuard* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := Singleton() = NIL
	END ShowDataGuard;

	PROCEDURE FocusIsPlot (): BOOLEAN;
		VAR
			v: PlotsViews.View;
	BEGIN
		v := Singleton();
		RETURN v # NIL
	END FocusIsPlot;

	PROCEDURE NothingChecked (): BOOLEAN;
	BEGIN
		RETURN ~modify.margins & ~modify.title & ~modify.xTitle
		 & ~modify.yTitle & ~modify.xBounds & ~modify.yBounds
	END NothingChecked;

	PROCEDURE GuardFocus* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~FocusIsPlot()
	END GuardFocus;

	PROCEDURE GuardApply* (VAR par: Dialog.Par);
	BEGIN
		IF (special # NIL) & (tabView # NIL) THEN
			IF (tabView.Index() = specialTabNum) & (specialApply # NIL) THEN 
				specialApply.Guard(par);
			END;
		END;
		IF ~par.disabled THEN
			par.disabled := ~FocusIsPlot()
		END
	END GuardApply;

	PROCEDURE GuardApplyAll* (VAR par: Dialog.Par);
	BEGIN
		TextCmds.FocusGuard(par);
		IF ~par.disabled THEN
			par.disabled := ~FocusIsPlot() OR NothingChecked()
		END
	END GuardApplyAll;

	PROCEDURE GuardModify* (VAR par: Dialog.Par);
		VAR
			v: PlotsViews.View;
	BEGIN
		v := Singleton();
		par.disabled := v = NIL
	END GuardModify;

	PROCEDURE GuardProperty* (property: INTEGER; VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~(property IN plotProp.valid) OR ~FocusIsPlot()
	END GuardProperty;

	PROCEDURE OpenDialog*;
		VAR
			v: Views.View;
			m: Models.Model;
			r: FormModels.Reader;
	BEGIN
		tabView := NIL;
		v := Singleton();
		InstallSpecialDialog(v);
		StdApi.OpenToolDialog('Plots/Rsrc/PropDialog', 'Plot Properties', v);
		IF v # NIL THEN
			m := v.ThisModel();
			IF m IS FormModels.Model THEN
				r := m(FormModels.Model).NewReader(NIL);
				LOOP
					r.ReadView(v);
					IF v = NIL THEN
						EXIT
					ELSIF v IS StdTabViews.View THEN
						tabView := v(StdTabViews.View);
						IF special # NIL THEN
							tabView.SetItem(5, "Special", special.Dialog())
						END;
						EXIT
					END
				END
			END
		END
	END OpenDialog;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "D.J.Lunn"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		tabView := NIL;
		NEW(plotProp);
		special := NIL;
		specialApply := NIL;
		NEW(handler);
		handler.Reschedule;
		ResetChecks
	END Init;

BEGIN
	Init
CLOSE
	Services.RemoveAction(handler)
END PlotsDialog.
