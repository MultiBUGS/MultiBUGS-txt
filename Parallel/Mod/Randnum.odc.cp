(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"


*)

MODULE ParallelRandnum;

	

	IMPORT
		Stores,
		BugsRandnum,
		MathRandnum, MathTT800;

	VAR
		sameStream, privateStream: MathRandnum.Generator;

		version-: INTEGER; 	(*	version number	*)
		maintainer-: ARRAY 40 OF CHAR; 	(*	person maintaining module	*)

	PROCEDURE ExeternalizeGenerators* (VAR wr: Stores.Writer);
	BEGIN
		MathRandnum.Externalize(privateStream, wr);
		MathRandnum.Externalize(sameStream, wr)
	END ExeternalizeGenerators;

	PROCEDURE InternalizeGenerators* (VAR rd: Stores.Reader);
	BEGIN
		privateStream := MathRandnum.Internalize(rd);
		sameStream := MathRandnum.Internalize(rd)
	END InternalizeGenerators;

	PROCEDURE UseSameStream*;
	BEGIN
		MathRandnum.SetGenerator(sameStream)
	END UseSameStream;

	PROCEDURE UsePrivateStream*;
	BEGIN
		MathRandnum.SetGenerator(privateStream)
	END UsePrivateStream;

	PROCEDURE SetUp* (chain, worldRank, numChains, worldSize: INTEGER);
		VAR
			offset: INTEGER;
	BEGIN
		(*	set up the random number streams for each chain and each process	*)
		MathTT800.Install;
		offset := BugsRandnum.offset;
		privateStream := MathRandnum.NewGenerator(numChains + offset + worldRank);
		sameStream := BugsRandnum.generators[chain];
		IF numChains = worldSize THEN
			MathRandnum.SetGenerator(sameStream)
		ELSE
			MathRandnum.SetGenerator(privateStream)
		END
	END SetUp;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		privateStream := NIL;
		sameStream := NIL;
	END Init;

BEGIN
	Init
END ParallelRandnum.

