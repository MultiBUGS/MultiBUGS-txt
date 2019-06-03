(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterMethods;


	

	IMPORT
		Stores := Stores64, 
		BugsMsg,
		UpdaterUpdaters;

	VAR
		index-: INTEGER;
		factories-: POINTER TO ARRAY OF UpdaterUpdaters.Factory;

		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE LoadUpdater* (IN installProc: ARRAY OF CHAR);
		VAR
			fact: UpdaterUpdaters.Factory;
			newFactories: POINTER TO ARRAY OF UpdaterUpdaters.Factory;
			i, len: INTEGER;
	BEGIN
		fact := UpdaterUpdaters.InstallFactory(installProc);
		IF fact = NIL THEN RETURN END;
		IF factories = NIL THEN
			NEW(factories, 1);
			factories[0] := fact
		ELSE
			len := LEN(factories);
			NEW(newFactories, len + 1);
			i := 0;
			WHILE i < len DO
				newFactories[i] := factories[i];
				INC(i)
			END;
			newFactories[len] := fact;
			factories := newFactories;
		END
	END LoadUpdater;

	PROCEDURE DisableAll*;
		VAR
			i, len: INTEGER;
	BEGIN
		IF factories # NIL THEN len := LEN(factories) ELSE len := 0 END;
		i := 0;
		WHILE i < len DO
			factories[i].SetProps(factories[i].props - {UpdaterUpdaters.enabled});
			INC(i)
		END
	END DisableAll;

	PROCEDURE SetFactory* (updateMethod: ARRAY OF CHAR);
		VAR
			i, len: INTEGER;
			installProc, install: ARRAY 1024 OF CHAR;
			fact: UpdaterUpdaters.Factory;
	BEGIN
		i := 0;
		IF factories # NIL THEN len := LEN(factories) ELSE len := 0 END;
		BugsMsg.InverseMapMsg(updateMethod, install);
		IF install # "" THEN
			LOOP
				IF i = len THEN index :=  - 1; EXIT END;
				fact := factories[i];
				fact.Install(installProc);
				IF installProc = install THEN index := i; EXIT END;
				INC(i)
			END
		ELSE
			index :=  - 1
		END
	END SetFactory;

	PROCEDURE SetProperty (fact: UpdaterUpdaters.Factory; value, property: INTEGER);
	BEGIN
		IF property IN fact.props THEN
			fact.SetParameter(value, property)
		END
	END SetProperty;

	PROCEDURE SetIterations* (iterations: INTEGER);
		VAR
			fact: UpdaterUpdaters.Factory;
	BEGIN
		IF index #  - 1 THEN
			fact := factories[index];
			SetProperty(fact, iterations, UpdaterUpdaters.iterations)
		END
	END SetIterations;

	PROCEDURE SetAdaptivePhase* (adaptivePhase: INTEGER);
		VAR
			fact: UpdaterUpdaters.Factory;
	BEGIN
		IF index #  - 1 THEN
			fact := factories[index];
			SetProperty(fact, adaptivePhase, UpdaterUpdaters.adaptivePhase)
		END
	END SetAdaptivePhase;

	PROCEDURE SetOverRelaxation* (overRelaxation: INTEGER);
		VAR
			fact: UpdaterUpdaters.Factory;
	BEGIN
		IF index #  - 1 THEN
			fact := factories[index];
			SetProperty(fact, overRelaxation, UpdaterUpdaters.overRelaxation)
		END
	END SetOverRelaxation;

	PROCEDURE Disable*;
		VAR
			fact: UpdaterUpdaters.Factory;
	BEGIN
		IF index #  - 1 THEN
			fact := factories[index];
			fact.SetProps(fact.props - {UpdaterUpdaters.enabled})
		END
	END Disable;

	PROCEDURE Enable*;
		VAR
			fact: UpdaterUpdaters.Factory;
	BEGIN
		IF index #  - 1 THEN
			fact := factories[index];
			fact.SetProps(fact.props + {UpdaterUpdaters.enabled})
		END
	END Enable;

	(* Guards *)

	PROCEDURE FactoryGuard* (OUT ok: BOOLEAN);
	BEGIN
		ok := index #  - 1
	END FactoryGuard;

	PROCEDURE IterationsGuard* (OUT ok: BOOLEAN);
	BEGIN
		IF index =  - 1 THEN
			ok := FALSE
		ELSE
			ok := UpdaterUpdaters.iterations IN factories[index].props
		END
	END IterationsGuard;

	PROCEDURE AdaptivePhaseGuard* (OUT ok: BOOLEAN);
	BEGIN
		IF index =  - 1 THEN
			ok := FALSE
		ELSE
			ok := UpdaterUpdaters.adaptivePhase IN factories[index].props
		END
	END AdaptivePhaseGuard;

	PROCEDURE OverRelaxationGuard* (OUT ok: BOOLEAN);
	BEGIN
		IF index =  - 1 THEN
			ok := FALSE
		ELSE
			ok := UpdaterUpdaters.overRelaxation IN factories[index].props
		END
	END OverRelaxationGuard;

	PROCEDURE ExternalizeProperties* (VAR wr: Stores.Writer);
		VAR
			i, len: INTEGER;
	BEGIN
		IF factories # NIL THEN len := LEN(factories) ELSE len := 0 END;
		wr.WriteInt(len);
		i := 0;
		WHILE i < len DO
			factories[i].Externalize(wr);
			INC(i)
		END;
	END ExternalizeProperties;

	PROCEDURE InternalizeProperties* (VAR rd: Stores.Reader);
		VAR
			i, len: INTEGER;
	BEGIN
		ASSERT(factories # NIL, 21);
		rd.ReadInt(len);
		ASSERT(len = LEN(factories), 22);
		i := 0;
		WHILE i < len DO
			factories[i].Internalize(rd);
			INC(i)
		END;
	END InternalizeProperties;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		factories := NIL;
		index :=  - 1
	END Init;

BEGIN
	Init
END UpdaterMethods.

