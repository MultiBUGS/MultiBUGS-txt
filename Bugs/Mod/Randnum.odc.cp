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
		seperateGenerators-: BOOLEAN;
		generators-: POINTER TO ARRAY OF MathRandnum.Generator;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Clear*;
	BEGIN
		numberChains := 1;
		generators := NIL
	END Clear;

	PROCEDURE CreateGenerators* (numChains: INTEGER; sepGens: BOOLEAN);
		VAR
			i: INTEGER;
	BEGIN
		numberChains := numChains;
		seperateGenerators := sepGens;
		NEW(generators, numChains);
		generators[0] := MathRandnum.NewGenerator(0);
		MathRandnum.SetGenerator(generators[0]);
		i := 0;
		WHILE i < numChains DO
			IF sepGens THEN
				generators[i] := MathRandnum.NewGenerator(i);
			ELSE
				generators[i] := generators[0];
			END;
			INC(i)
		END;
	END CreateGenerators;

	PROCEDURE ExternalizeRNGenerators* (VAR wr: Stores.Writer);
		VAR
			i: INTEGER;
			present: BOOLEAN;
	BEGIN
		wr.WriteBool(seperateGenerators);
		present := generators # NIL;
		wr.WriteBool(present);
		IF present THEN
			wr.WriteInt(numberChains);
			IF seperateGenerators THEN
				i := 0;
				WHILE i < numberChains DO
					MathRandnum.Externalize(generators[i], wr);
					INC(i)
				END
			ELSE
				MathRandnum.Externalize(generators[0], wr)
			END
		END;
	END ExternalizeRNGenerators;

	PROCEDURE InternalizeRNGenerators* (VAR rd: Stores.Reader);
		VAR
			i, numberChains: INTEGER;
			present, seperateGenerators: BOOLEAN;
	BEGIN
		rd.ReadBool(seperateGenerators);
		rd.ReadBool(present);
		IF present THEN
			rd.ReadInt(numberChains);
			CreateGenerators(numberChains, seperateGenerators);
			IF seperateGenerators THEN
				i := 0;
				WHILE i < numberChains DO
					generators[i] := MathRandnum.Internalize(rd);
					INC(i)
				END
			ELSE
				generators[0] := MathRandnum.Internalize(rd);
				i := 0;
				WHILE i < numberChains DO
					generators[i] := generators[0];
					INC(i)
				END
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
		seperateGenerators := TRUE;
		numberChains := 1
	END Init;

BEGIN
	Init
END BugsRandnum.
