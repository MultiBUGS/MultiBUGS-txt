(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsRandnum;

	

	IMPORT
		Stores,
		MathRandnum;

	VAR

		numberChains-: INTEGER;
		generators-: POINTER TO ARRAY OF MathRandnum.Generator;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Clear*;
	BEGIN
		numberChains := 1;
		generators := NIL
	END Clear;

	PROCEDURE CreateGenerators* (numChains: INTEGER);
		VAR
			i: INTEGER;
	BEGIN
		numberChains := numChains;
		NEW(generators, numChains);
		i := 0;
		WHILE i < numChains DO
			generators[i] := MathRandnum.NewGenerator(i);
			INC(i)
		END;
	END CreateGenerators;

	PROCEDURE ExternalizeRNGenerators* (VAR wr: Stores.Writer);
		VAR
			i: INTEGER;
			present: BOOLEAN;
	BEGIN
		present := generators # NIL;
		wr.WriteBool(present);
		IF present THEN
			wr.WriteInt(numberChains);
			i := 0;
			WHILE i < numberChains DO
				MathRandnum.Externalize(generators[i], wr);
				INC(i)
			END
		END
	END ExternalizeRNGenerators;

	PROCEDURE InternalizeRNGenerators* (VAR rd: Stores.Reader);
		VAR
			i: INTEGER;
			present: BOOLEAN;
	BEGIN
		rd.ReadBool(present);
		IF present THEN
			rd.ReadInt(numberChains);
			NEW(generators, numberChains);
			i := 0;
			WHILE i < numberChains DO
				generators[i] := MathRandnum.Internalize(rd);
				INC(i)
			END;
			MathRandnum.SetGenerator(generators[0])
		END
	END InternalizeRNGenerators;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		generators := NIL;
		numberChains := 1
	END Init;

BEGIN
	Init
END BugsRandnum.
