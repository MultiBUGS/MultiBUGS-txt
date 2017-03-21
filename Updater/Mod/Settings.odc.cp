(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterSettings;


	

	IMPORT
		Dialog, 
		StdTabViews,
		BugsDialog, BugsInterface, BugsMsg, BugsRegistry, BugsTexts,
		UpdaterMethods, UpdaterUpdaters;


	TYPE
		SettingsDialog* = POINTER TO RECORD(BugsDialog.DialogBox)
			adaptivePhase*, iterations*, overRelaxation*: INTEGER;
			methods*: Dialog.List;
			allMethods*: Dialog.List;
			activeMethods*: Dialog.List;
			use*: INTEGER;
			node*: Dialog.String
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		dialog*: SettingsDialog;
		compiled: BOOLEAN;

	CONST
		yes = 0;
		no = 1;

		(* Miscellaneous *)

	PROCEDURE PropertyValue (fact: UpdaterUpdaters.Factory; property: INTEGER): INTEGER;
		VAR
			value: INTEGER;
	BEGIN
		ASSERT(fact # NIL, 25);
		IF property IN fact.props THEN
			CASE property OF
			|UpdaterUpdaters.iterations:
				value := fact.iterations
			|UpdaterUpdaters.adaptivePhase:
				value := fact.adaptivePhase
			|UpdaterUpdaters.overRelaxation:
				value := fact.overRelaxation
			END
		ELSE
			value := 0
		END;
		RETURN value
	END PropertyValue;

	PROCEDURE ValidIterations (fact: UpdaterUpdaters.Factory): BOOLEAN;
	BEGIN
		RETURN ~(UpdaterUpdaters.iterations IN fact.props) OR (dialog.iterations > 0)
	END ValidIterations;

	PROCEDURE ValidAdaptivePhase (fact: UpdaterUpdaters.Factory): BOOLEAN;
		VAR
			valid: BOOLEAN;
	BEGIN
		IF ~(UpdaterUpdaters.adaptivePhase IN fact.props) THEN
			valid := TRUE
		ELSE
			IF dialog.adaptivePhase < 1 THEN
				valid := FALSE
			ELSE
				(*valid := dialog.adaptivePhase > UpdaterActions.iteration*)
				valid := TRUE
			END
		END;
		RETURN valid
	END ValidAdaptivePhase;

	PROCEDURE ValidOverRelaxation (fact: UpdaterUpdaters.Factory): BOOLEAN;
	BEGIN
		RETURN ~(UpdaterUpdaters.overRelaxation IN fact.props) OR (dialog.overRelaxation > 0)
	END ValidOverRelaxation;

	PROCEDURE HasParameters (fact: UpdaterUpdaters.Factory): BOOLEAN;
		CONST
			knownProperties = {UpdaterUpdaters.iterations, UpdaterUpdaters.adaptivePhase,
			UpdaterUpdaters.overRelaxation};
	BEGIN
		RETURN knownProperties * fact.props # {}
	END HasParameters;

	(* Commands *)

	PROCEDURE WriteRegistry (IN name: ARRAY OF CHAR; value, property: INTEGER);
		VAR
			key: ARRAY 256 OF CHAR;
	BEGIN
		key := SHORT(name);
		CASE property OF
		|UpdaterUpdaters.iterations:
			key := key + "_Iterations"
		|UpdaterUpdaters.adaptivePhase:
			key := key + "_AdaptivePhase"
		|UpdaterUpdaters.overRelaxation:
			key := key + "_OverRelaxation"
		END;
		BugsRegistry.WriteInt(key, value)
	END WriteRegistry;

	PROCEDURE FindFactory (): INTEGER;
		VAR
			name, name1: Dialog.String;
			i, index, len: INTEGER;
	BEGIN
		dialog.methods.GetItem(dialog.methods.index, name);
		i := 0;
		index := -1;
		IF UpdaterMethods.factories # NIL THEN
			len := LEN(UpdaterMethods.factories)
		ELSE
			len := 0
		END;
		WHILE (i < len) & (index = -1) DO
			UpdaterMethods.factories[i].Install(name1);
			BugsMsg.MapMsg(name1, name1);
			IF name = name1 THEN index := i END;
			INC(i)
		END;
		RETURN index
	END FindFactory;

	PROCEDURE FindAllFactory (): INTEGER;
		VAR
			name, name1: Dialog.String;
			i, index, len: INTEGER;
	BEGIN
		dialog.allMethods.GetItem(dialog.allMethods.index, name);
		i := 0;
		index := -1;
		IF UpdaterMethods.factories # NIL THEN
			len := LEN(UpdaterMethods.factories)
		ELSE
			len := 0
		END;
		WHILE (i < len) & (index = -1) DO
			UpdaterMethods.factories[i].Install(name1);
			BugsMsg.MapMsg(name1, name1);
			IF name = name1 THEN index := i END;
			INC(i)
		END;
		RETURN index
	END FindAllFactory;

	PROCEDURE FindActiveFactory (): INTEGER;
		VAR
			name, name1: Dialog.String;
			i, index,  len: INTEGER;
	BEGIN
		dialog.activeMethods.GetItem(dialog.activeMethods.index, name);
		i := 0;
		index := -1;
		IF UpdaterMethods.factories # NIL THEN
			len := LEN(UpdaterMethods.factories)
		ELSE
			len := 0
		END;
		WHILE (i < len) & (index = -1) DO
			UpdaterMethods.factories[i].Install(name1);
			BugsMsg.MapMsg(name1, name1);
			IF name = name1 THEN index := i END;
			INC(i)
		END;
		RETURN index
	END FindActiveFactory;

	PROCEDURE SetProperty (fact: UpdaterUpdaters.Factory; value, property: INTEGER);
	BEGIN
		IF property IN fact.props THEN
			fact.SetParameter(value, property)
		END
	END SetProperty;

	PROCEDURE SaveParameters*;
		VAR
			name: ARRAY 1024 OF CHAR;
			index: INTEGER;
			fact: UpdaterUpdaters.Factory;
	BEGIN
		index := FindActiveFactory();
		fact := UpdaterMethods.factories[index];
		fact.Install(name);
		SetProperty(fact, dialog.iterations, UpdaterUpdaters.iterations);
		WriteRegistry(name, dialog.iterations, UpdaterUpdaters.iterations);
		SetProperty(fact, dialog.adaptivePhase, UpdaterUpdaters.adaptivePhase);
		WriteRegistry(name, dialog.adaptivePhase, UpdaterUpdaters.adaptivePhase);
		SetProperty(fact, dialog.overRelaxation, UpdaterUpdaters.overRelaxation);
		WriteRegistry(name, dialog.overRelaxation, UpdaterUpdaters.overRelaxation)
	END SaveParameters;

	PROCEDURE SetParameters*;
		VAR
			index: INTEGER;
			fact: UpdaterUpdaters.Factory;
	BEGIN
		index := FindActiveFactory();
		fact := UpdaterMethods.factories[index];
		SetProperty(fact, dialog.iterations, UpdaterUpdaters.iterations);
		SetProperty(fact, dialog.adaptivePhase, UpdaterUpdaters.adaptivePhase);
		SetProperty(fact, dialog.overRelaxation, UpdaterUpdaters.overRelaxation)
	END SetParameters;

	PROCEDURE SetIterations* (iterations: INTEGER);
		VAR
			ok: BOOLEAN;
	BEGIN
		UpdaterMethods.IterationsGuard(ok);
		IF ok THEN
			dialog.iterations := iterations;
			UpdaterMethods.SetIterations(iterations);
		END
	END SetIterations;

	PROCEDURE SetAdaptivePhase* (adaptivePhase: INTEGER);
		VAR
			ok: BOOLEAN;
	BEGIN
		UpdaterMethods.AdaptivePhaseGuard(ok);
		IF ok THEN
			dialog.adaptivePhase := adaptivePhase;
			UpdaterMethods.SetAdaptivePhase(adaptivePhase);
		END
	END SetAdaptivePhase;

	PROCEDURE SetOverRelaxation* (overRelaxation: INTEGER);
		VAR
			ok: BOOLEAN;
	BEGIN
		UpdaterMethods.OverRelaxationGuard(ok);
		IF ok THEN
			dialog.overRelaxation := overRelaxation;
			UpdaterMethods.SetIterations(overRelaxation);
		END
	END SetOverRelaxation;

	PROCEDURE Clear*;
		VAR
			i, len: INTEGER;
			fact: UpdaterUpdaters.Factory;
	BEGIN
		i := 0;
		IF UpdaterMethods.factories # NIL THEN
			len := LEN(UpdaterMethods.factories)
		ELSE
			len := 0
		END;
		WHILE i < len DO
			fact := UpdaterMethods.factories[i];
			fact.SetProps(fact.props - {UpdaterUpdaters.active});
			INC(i)
		END;
		compiled := FALSE;
		dialog.node := ""
	END Clear;

	PROCEDURE FillDialog*;
		VAR
			i, j, k, len, index: INTEGER;
			name: Dialog.String;
			fact: UpdaterUpdaters.Factory;
	BEGIN
		dialog.activeMethods.SetLen(0);
		dialog.methods.SetLen(0);
		dialog.allMethods.SetLen(0);
		IF UpdaterMethods.factories # NIL THEN
			len := LEN(UpdaterMethods.factories)
		ELSE
			len := 0
		END;
		i := 0;
		j := 0;
		k := 0;
		WHILE i < len DO
			UpdaterMethods.factories[i].Install(name);
			BugsMsg.MapMsg(name, name);
			IF UpdaterUpdaters.active IN UpdaterMethods.factories[i].props THEN
				dialog.activeMethods.SetItem(j, name); INC(j)
			END;
			IF (UpdaterUpdaters.enabled IN UpdaterMethods.factories[i].props) & 
				~(UpdaterUpdaters.hidden IN UpdaterMethods.factories[i].props) THEN
				dialog.methods.SetItem(k, name); INC(k)
			END;
			dialog.allMethods.SetItem(i, name);
			INC(i)
		END;
		dialog.methods.SetLen(k);
		dialog.allMethods.SetLen(len);
		IF compiled THEN
			dialog.activeMethods.SetLen(j)
		END;
		dialog.methods.index := 0;
		dialog.allMethods.index := 0;
		dialog.activeMethods.index := 0;
		index := FindAllFactory();
		fact := UpdaterMethods.factories[index];
		IF UpdaterUpdaters.enabled IN fact.props THEN
			dialog.use := yes
		ELSE
			dialog.use := no
		END;
		index := FindActiveFactory();
		IF index # -1 THEN
			fact := UpdaterMethods.factories[index];
			dialog.iterations := PropertyValue(fact, UpdaterUpdaters.iterations);
			dialog.adaptivePhase := PropertyValue(fact, UpdaterUpdaters.adaptivePhase);
			dialog.overRelaxation := PropertyValue(fact, UpdaterUpdaters.overRelaxation)
		END
	END FillDialog;

	PROCEDURE ChangeSampler*;
		VAR
			ok: BOOLEAN;
			s: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		BugsInterface.ChangeSampler(dialog.node, dialog.methods.index, ok);
		IF ~ok THEN
			p[0] := dialog.node$;
			BugsMsg.MapParamMsg("BugsEmbed:couldNotChangeUpdater", p, s);
			BugsTexts.ShowMsg(s)
		END;
		FillDialog;
		Dialog.Update(dialog);
		Dialog.UpdateList(dialog.activeMethods)
	END ChangeSampler;

	PROCEDURE MarkCompiled*;
	BEGIN
		compiled := TRUE
	END MarkCompiled;

	PROCEDURE MarkActive* (index: INTEGER);
		VAR
			fact: UpdaterUpdaters.Factory;
	BEGIN
		fact := UpdaterMethods.factories[index];
		fact.SetProps(fact.props + {UpdaterUpdaters.active})
	END MarkActive;

	PROCEDURE Disable*;
		VAR
			index: INTEGER;
			fact: UpdaterUpdaters.Factory;
	BEGIN
		index := FindAllFactory();
		fact := UpdaterMethods.factories[index];
		IF ~(UpdaterUpdaters.hidden IN fact.props) THEN
			fact.SetProps(fact.props - {UpdaterUpdaters.enabled})
		END
	END Disable;

	PROCEDURE Enable*;
		VAR
			index: INTEGER;
			fact: UpdaterUpdaters.Factory;
	BEGIN
		index := FindAllFactory();
		fact := UpdaterMethods.factories[index];
		IF ~(UpdaterUpdaters.hidden IN fact.props) THEN
			fact.SetProps(fact.props + {UpdaterUpdaters.enabled})
		END
	END Enable;

	(* Guards *)

	PROCEDURE AdaptivePhaseGuard* (VAR par: Dialog.Par);
		VAR
			index: INTEGER;
			fact: UpdaterUpdaters.Factory;
	BEGIN
		index := FindActiveFactory();
		IF index = -1 THEN
			par.disabled := TRUE
		ELSE
			fact := UpdaterMethods.factories[index];
			par.disabled := ~(UpdaterUpdaters.adaptivePhase IN fact.props)
		END
	END AdaptivePhaseGuard;

	PROCEDURE EnableGuard* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := compiled;
	END EnableGuard;

	PROCEDURE IterationsGuard* (VAR par: Dialog.Par);
		VAR
			index: INTEGER;
			fact: UpdaterUpdaters.Factory;
	BEGIN
		index := FindActiveFactory();
		IF index = -1 THEN
			par.disabled := TRUE
		ELSE
			fact := UpdaterMethods.factories[index];
			par.disabled := ~(UpdaterUpdaters.iterations IN fact.props)
		END
	END IterationsGuard;

	PROCEDURE OverRelaxationGuard* (VAR par: Dialog.Par);
		VAR
			index: INTEGER;
			fact: UpdaterUpdaters.Factory;
	BEGIN
		index := FindActiveFactory();
		IF index = -1 THEN
			par.disabled := TRUE
		ELSE
			fact := UpdaterMethods.factories[index];
			par.disabled := ~(UpdaterUpdaters.overRelaxation IN fact.props)
		END
	END OverRelaxationGuard;

	PROCEDURE SetGuard* (VAR par: Dialog.Par);
		VAR
			index: INTEGER;
			fact: UpdaterUpdaters.Factory;
	BEGIN
		index := FindActiveFactory();
		par.disabled := index = -1;
		IF ~par.disabled THEN
			fact := UpdaterMethods.factories[index];
			par.disabled := ~HasParameters(fact) OR ~ValidIterations(fact)
			OR ~ValidAdaptivePhase(fact) OR ~ValidOverRelaxation(fact)
		END
	END SetGuard;

	(* Notifiers *)

	PROCEDURE SelectMethodNotifier* (op, from, to: INTEGER);
		VAR
			index: INTEGER;
			fact: UpdaterUpdaters.Factory;
	BEGIN
		IF op = Dialog.changed THEN
			index := FindAllFactory();
			fact := UpdaterMethods.factories[index];
			IF UpdaterUpdaters.enabled IN fact.props THEN
				dialog.use := yes
			ELSE
				dialog.use := no
			END;
			Dialog.Update(dialog);
			Dialog.UpdateList(dialog.allMethods)
		END
	END SelectMethodNotifier;

	PROCEDURE SelectActiveMethodNotifier* (op, from, to: INTEGER);
		VAR
			index: INTEGER;
			fact: UpdaterUpdaters.Factory;
	BEGIN
		IF op = Dialog.changed THEN
			index := FindActiveFactory();
			fact := UpdaterMethods.factories[index];
			dialog.iterations := PropertyValue(fact, UpdaterUpdaters.iterations);
			dialog.adaptivePhase := PropertyValue(fact, UpdaterUpdaters.adaptivePhase);
			dialog.overRelaxation := PropertyValue(fact, UpdaterUpdaters.overRelaxation);
			Dialog.Update(dialog);
			Dialog.UpdateList(dialog.activeMethods)
		END
	END SelectActiveMethodNotifier;

	PROCEDURE EnableNotifier* (op, from, to: INTEGER);
	BEGIN
		IF dialog.use = yes THEN
			Enable
		ELSE
			Disable
		END
	END EnableNotifier;

	PROCEDURE TabNotifier* (v: StdTabViews.View; from, to: INTEGER);
	BEGIN
		FillDialog;
		Dialog.Update(dialog);
		Dialog.UpdateList(dialog.methods);
		Dialog.UpdateList(dialog.allMethods);
		Dialog.UpdateList(dialog.activeMethods)
	END TabNotifier;

	PROCEDURE (d: SettingsDialog) Init-;
	BEGIN
		Clear;
		FillDialog;
		Dialog.Update(dialog);
		Dialog.UpdateList(dialog.methods);
		Dialog.UpdateList(dialog.activeMethods)
	END Init;

	PROCEDURE (d: SettingsDialog) Update-;
	BEGIN
		Dialog.Update(dialog);
		Dialog.UpdateList(dialog.methods);
		Dialog.UpdateList(dialog.activeMethods)
	END Update;

	(* Initialization *)

	PROCEDURE Maintainer;
	BEGIN
		version := 310;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		NEW(dialog);
		Clear;
		compiled := FALSE;
		FillDialog;
		dialog.methods.index := 0;
		dialog.activeMethods.index := 0;
		BugsDialog.AddDialog(dialog)
	END Init;

BEGIN
	Init
END UpdaterSettings.

