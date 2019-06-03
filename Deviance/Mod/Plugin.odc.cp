(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE DeviancePlugin;


	

	IMPORT
		Meta, Stores := Stores64,
		GraphNodes;

	TYPE
		Plugin* = POINTER TO ABSTRACT RECORD END;

		Factory* = POINTER TO ABSTRACT RECORD END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		fact-: Factory;

	PROCEDURE (plugin: Plugin) DevianceTerm* (termOffset: INTEGER): REAL, NEW, ABSTRACT;

	PROCEDURE (plugin: Plugin) IsValid* (): BOOLEAN, NEW, ABSTRACT;

	PROCEDURE (plugin: Plugin) NumParents* (): INTEGER, NEW, ABSTRACT;

	PROCEDURE (plugin: Plugin) Parents* (): GraphNodes.Vector, NEW, ABSTRACT;

	PROCEDURE (plugin: Plugin) SetValues* (IN values: ARRAY OF REAL), NEW, ABSTRACT;

	PROCEDURE (plugin: Plugin) Type* (): INTEGER, NEW, ABSTRACT;

	PROCEDURE (plugin: Plugin) Values* (): POINTER TO ARRAY OF REAL, NEW, ABSTRACT;

	PROCEDURE (f: Factory) Install* (OUT install: ARRAY OF CHAR), NEW, ABSTRACT;

	PROCEDURE (f: Factory) New* (): Plugin, NEW, ABSTRACT;

	PROCEDURE Clear*;
	BEGIN
		fact := NIL;
	END Clear;

	PROCEDURE Externalize* (VAR wr: Stores.Writer);
		VAR
			present: BOOLEAN;
			install: ARRAY 128 OF CHAR;
	BEGIN
		present := fact # NIL;
		wr.WriteBool(present);
		IF present THEN
			fact.Install(install);
			wr.WriteString(install)
		END
	END Externalize;

	PROCEDURE Internalize* (VAR rd: Stores.Reader);
		VAR
			present: BOOLEAN;
			install: ARRAY 128 OF CHAR;
			ok: BOOLEAN;
			item: Meta.Item;
			cmd: RECORD(Meta.Value) Do: PROCEDURE END;
	BEGIN
		fact := NIL;
		rd.ReadBool(present);
		IF present THEN
			rd.ReadString(install);
			Meta.LookupPath(install, item);
			IF item.obj = Meta.procObj THEN
				item.GetVal(cmd, ok);
				IF ok THEN
					cmd.Do
				END
			END
		END
	END Internalize;

	PROCEDURE SetFact* (f: Factory);
	BEGIN
		fact := f
	END SetFact;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		fact := NIL
	END Init;

BEGIN
	Init
END DeviancePlugin.
