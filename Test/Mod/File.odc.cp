MODULE TestFile;

	

	IMPORT Files, Stores, HostFiles, MPIworker, StdLog, UpdaterParallel;

	PROCEDURE Do*;
		VAR
			loc: Files.Locator;
	BEGIN
		loc := Files.dir.This("Restart");
		StdLog.String(loc(HostFiles.Locator).path); StdLog.Ln
	END Do;

	PROCEDURE Read*;
		VAR
			rd: Stores.Reader;
			f: Files.File;
			loc: Files.Locator;
			fileName: Files.Name;
			numChains: INTEGER;
	BEGIN
		fileName := "Bugs_3394784";
		loc := Files.dir.This("Restart");
		f := Files.dir.Old(loc, fileName + ".bug", Files.shared);
		rd.ConnectTo(f);
		rd.SetPos(0);
		rd.ReadInt(numChains);
		MPIworker.ReadPort(rd);
		UpdaterParallel.ReadGraph(0, rd)
	END Read;

END TestFile.


TestFile.Do

TestFile.Read
