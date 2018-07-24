(*	

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsDistribute;

	

	IMPORT
		Converters, Dialog, Files, Strings, Views,
		StdLog,
		TextViews;

	VAR
		nDir, nTran, nCopy, nSkip: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		version-: INTEGER;
		dest*: ARRAY 80 OF CHAR;
		copyBinary*, copyDocu*, copySource*, copyExamples*,
		produceHTML-: BOOLEAN;

	PROCEDURE Proof (VAR name: ARRAY OF CHAR);
		VAR i: INTEGER;
	BEGIN
		FOR i := 0 TO LEN(name$) - 1 DO
			IF name[i] = "\" THEN name[i] := "/" END
		END;
		IF name = "/" THEN name := "" END
	END Proof;

	PROCEDURE ExportDoc (srcLoc, dstLoc: Files.Locator; name: Files.Name);
		VAR
			pos, res: INTEGER;
			v: Views.View;
			conv: Converters.Converter;
			f: Files.File;
	BEGIN
		conv := NIL;
		v := Views.Old(Views.dontAsk, srcLoc, name, conv);
		IF (v # NIL) & (v IS TextViews.View) THEN
			Strings.Find(name, ".", 0, pos);
			IF pos > 0 THEN Strings.Extract(name, 0, pos, name) END; 	(* filename without extension *)
			f := Files.dir.New(dstLoc, Files.dontAsk);
			(*HtmlExporter.ExportNamedText(v(TextViews.View), f, dstLoc, name$);*)
			f.Register(name, "html", Files.dontAsk, res);
			ASSERT(res = 0, 100)
		ELSE
			StdLog.String(" file open error");
			StdLog.Ln
		END
	END ExportDoc;

	PROCEDURE CopyFileLoc (srcLoc, dstLoc: Files.Locator; name: Files.Name);
		VAR
			i, len, res: INTEGER;
			byte: BYTE;
			new, old: Files.File;
			type: Files.Type;
			rd: Files.Reader;
			wr: Files.Writer;
	BEGIN
		old := Files.dir.Old(srcLoc, name, Files.shared);
		IF (old # NIL) & (old.type # "bmp") THEN
			new := Files.dir.New(dstLoc, Files.dontAsk);
			len := old.Length();
			i := 0;
			rd := old.NewReader(NIL);
			wr := new.NewWriter(NIL);
			WHILE i < len DO
				rd.ReadByte(byte); wr.WriteByte(byte); INC(i)
			END;
			old.Close;
			Strings.ToLower(old.type, type);
			new.Register(name, type, Files.dontAsk, res)
		END
	END CopyFileLoc;

	PROCEDURE CopyFileLocToAscii (srcLoc, dstLoc: Files.Locator; srcName, destName: Files.Name;
	ext: ARRAY OF CHAR);
		VAR
			v: Views.View;
			conv: Converters.Converter;
			pos: INTEGER;
	BEGIN
		v := Views.OldView(srcLoc, srcName);
		IF (v # NIL) & (v IS TextViews.View) THEN
			conv := Converters.list;
			WHILE (conv # NIL) & (conv.fileType # ext) DO
				conv := conv.next
			END;
			IF conv # NIL THEN
				Strings.Find(destName, ".odc", 0, pos);
				IF pos # - 1 THEN
					destName[pos + 1] := 0X;
					destName := destName + ext
				END;
				Converters.Export(dstLoc, destName, conv, v)
			END
		END
	END CopyFileLocToAscii;

	PROCEDURE PathToLoc (IN path: ARRAY OF CHAR; OUT loc: Files.Locator);
		VAR
			i, j: INTEGER;
			ch: CHAR;
			name: ARRAY 256 OF CHAR;
	BEGIN
		loc := Files.dir.This("");
		IF path # "" THEN
			i := 0; j := 0;
			REPEAT
				ch := path[i]; INC(i);
				IF (ch = "/") OR (ch = 0X) THEN name[j] := 0X; j := 0; loc := loc.This(name)
				ELSE name[j] := ch; INC(j)
				END
			UNTIL (ch = 0X) OR (loc.res # 0)
		END
	END PathToLoc;

	PROCEDURE CopyFile* (name, destRoot: ARRAY OF CHAR);
		VAR
			len, pos, start: INTEGER;
			dstLoc, srcLoc: Files.Locator;
			fName, path: Files.Name;
	BEGIN
		Proof(name);
		Proof(destRoot);
		path := name$;
		fName := name$;
		start := 0;
		Strings.Find(name, "/", start, pos);
		WHILE pos # - 1 DO
			start := pos + 1;
			Strings.Find(name, "/", start, pos)
		END;
		path[start] := 0X;
		PathToLoc(path, srcLoc);
		PathToLoc(destRoot + "/" + path, dstLoc);
		len := LEN(name$);
		Strings.Extract(name, start, len - start, fName);
		StdLog.String("copying ");
		StdLog.String(name);
		StdLog.Ln;
		INC(nCopy);
		CopyFileLoc(srcLoc, dstLoc, fName)
	END CopyFile;

	PROCEDURE CopyFileToAscii (srcPath, destPath, ext: ARRAY OF CHAR);
		VAR
			len, pos, start: INTEGER;
			destLoc, srcLoc: Files.Locator;
			srcName, destName: Files.Name;
	BEGIN
		Proof(srcPath);
		Proof(destPath);
		srcName := srcPath$;
		start := 0;
		Strings.Find(srcPath, "/", start, pos);
		WHILE pos # - 1 DO
			start := pos + 1;
			Strings.Find(srcPath, "/", start, pos)
		END;
		srcPath[start] := 0X;
		PathToLoc(srcPath, srcLoc);
		len := LEN(srcName$);
		Strings.Extract(srcName, start, len - start, srcName);
		destName := destPath$;
		start := 0;
		Strings.Find(destPath, "/", start, pos);
		WHILE pos # - 1 DO
			start := pos + 1;
			Strings.Find(destPath, "/", start, pos)
		END;
		destPath[start] := 0X;
		PathToLoc(destPath, destLoc);
		len := LEN(destName$);
		Strings.Extract(destName, start, len - start, destName);
		StdLog.String("copying ");
		StdLog.String(srcPath);
		StdLog.Ln;
		INC(nCopy);
		CopyFileLocToAscii(srcLoc, destLoc, srcName, destName, ext)
	END CopyFileToAscii;

	PROCEDURE CopyLoc* (srcPath, destRoot: ARRAY OF CHAR);
		VAR
			len, pos: INTEGER;
			srcLoc, dstLoc: Files.Locator;
			files: Files.FileInfo;
			name, extension: Files.Name;
			message: ARRAY 256 OF CHAR;
	BEGIN
		Proof(srcPath);
		Proof(destRoot);
		INC(nDir);
		PathToLoc(srcPath, srcLoc);
		PathToLoc(destRoot + "/" + srcPath, dstLoc);
		files := Files.dir.FileList(srcLoc);
		WHILE files # NIL DO
			name := files.name$;
			len := LEN(name$);
			Strings.Extract(name, len - 4, len, extension);
			Strings.ToLower(extension, extension);
			name[len - 4] := 0X;
			Strings.Find(name, "odc", 0, pos);
			IF (pos # - 1) & (extension = "odc") THEN
				(*	skip temp odc*.odc	files	*)
			ELSE
				name := name + extension;
				message := srcPath$ + name$;
				Dialog.ShowStatus(message);
				StdLog.String("copying ");
				StdLog.String(srcPath$ + "/" + name$);
				StdLog.Ln;
				INC(nCopy);
				CopyFileLoc(srcLoc, dstLoc, name)
			END;
			files := files.next
		END
	END CopyLoc;

	PROCEDURE DeleteFile* (fileName, srcPath, destRoot: ARRAY OF CHAR);
		VAR
			dstLoc: Files.Locator;
	BEGIN
		Proof(srcPath);
		Proof(destRoot);
		PathToLoc(destRoot + "/" + srcPath, dstLoc);
		Files.dir.Delete(dstLoc, fileName$)
	END DeleteFile;

	PROCEDURE CopyExamplesLocToAscii (srcPath, destRoot: ARRAY OF CHAR);
		VAR
			len, pos: INTEGER;
			srcLoc, dstLoc: Files.Locator;
			files: Files.FileInfo;
			name, extension: Files.Name;
			message: ARRAY 256 OF CHAR;
	BEGIN
		Proof(srcPath);
		Proof(destRoot);
		INC(nDir);
		PathToLoc(srcPath, srcLoc);
		PathToLoc(destRoot + "/" + srcPath, dstLoc);
		files := Files.dir.FileList(srcLoc);
		WHILE files # NIL DO
			name := files.name$;
			len := LEN(name$);
			Strings.Extract(name, len - 4, len, extension);
			Strings.ToLower(extension, extension);
			name[len - 4] := 0X;
			Strings.Find(name, "odc", 0, pos);
			IF (pos # - 1) & (extension = "odc") THEN
				(*	skip temp odc*.odc	files	*)
			ELSE
				Strings.Find(name, "model", 0, pos);
				IF pos = - 1 THEN Strings.Find(name, "data", 0, pos) END;
				IF pos = - 1 THEN Strings.Find(name, "inits", 0, pos) END;
				IF pos # - 1 THEN
					name := name + extension;
					message := srcPath$ + name$;
					Dialog.ShowStatus(message);
					StdLog.String("copying ");
					StdLog.String(srcPath$ + "/" + name$);
					StdLog.Ln;
					INC(nCopy);
					CopyFileLocToAscii(srcLoc, dstLoc, name, name, "txt")
				END
			END;
			files := files.next
		END
	END CopyExamplesLocToAscii;

	PROCEDURE CopyLocToAscii* (srcPath, destRoot, ext: ARRAY OF CHAR);
		VAR
			len, pos: INTEGER;
			srcLoc, dstLoc: Files.Locator;
			files: Files.FileInfo;
			name, extension: Files.Name;
			message: ARRAY 256 OF CHAR;
	BEGIN
		Proof(srcPath);
		Proof(destRoot);
		INC(nDir);
		PathToLoc(srcPath, srcLoc);
		PathToLoc(destRoot + "/" + srcPath, dstLoc);
		files := Files.dir.FileList(srcLoc);
		WHILE files # NIL DO
			name := files.name$;
			len := LEN(name$);
			Strings.Extract(name, len - 4, len, extension);
			Strings.ToLower(extension, extension);
			name[len - 4] := 0X;
			Strings.Find(name, "odc", 0, pos);
			IF (pos # - 1) & (extension = "odc") THEN
				(*	skip temp odc*.odc	files	*)
			ELSE
				name := name + extension;
				message := srcPath$ + name$;
				Dialog.ShowStatus(message);
				StdLog.String("copying ");
				StdLog.String(srcPath$ + "/" + name$);
				StdLog.Ln;
				INC(nCopy);
				CopyFileLocToAscii(srcLoc, dstLoc, name, name, ext)
			END;
			files := files.next
		END
	END CopyLocToAscii;

	PROCEDURE ExportFile (name, destRoot: ARRAY OF CHAR);
		VAR
			len, pos, start: INTEGER;
			dstLoc, srcLoc: Files.Locator;
			extension, fName, path: Files.Name;
	BEGIN
		Proof(name);
		Proof(destRoot);
		path := name$;
		fName := name$;
		start := 0;
		Strings.Find(name, "/", start, pos);
		WHILE pos # - 1 DO
			start := pos + 1;
			Strings.Find(name, "/", start, pos)
		END;
		path[start] := 0X;
		PathToLoc(path, srcLoc);
		PathToLoc(destRoot + "/" + path, dstLoc);
		len := LEN(name$);
		Strings.Extract(name, start, len - start, fName);
		len := LEN(fName$);
		Strings.Extract(fName, len - 4, len, extension);
		IF extension = ".odc" THEN
			ExportDoc(srcLoc, dstLoc, fName);
			StdLog.String("translating ");
			StdLog.String(name);
			StdLog.Ln;
			INC(nTran)
		ELSE
			INC(nSkip);
			StdLog.String("skip ");
			StdLog.String(name);
			StdLog.Ln
		END
	END ExportFile;

	PROCEDURE CopyLocToHtml* (srcPath, destRoot: ARRAY OF CHAR);
		VAR
			len: INTEGER;
			srcLoc, dstLoc: Files.Locator;
			files: Files.FileInfo;
			name, extension: Files.Name;
			message: ARRAY 256 OF CHAR;
	BEGIN
		Proof(srcPath);
		Proof(destRoot);
		INC(nDir);
		PathToLoc(srcPath, srcLoc);
		PathToLoc(destRoot + "/" + srcPath, dstLoc);
		files := Files.dir.FileList(srcLoc);
		WHILE files # NIL DO
			name := files.name$;
			len := LEN(name$);
			Strings.Extract(name, len - 4, len, extension);
			Strings.ToLower(extension, extension);
			name[len - 4] := 0X;
			name := name + extension;
			IF extension = ".odc" THEN
				message := srcPath$ + name$;
				Dialog.ShowStatus(message);
				StdLog.String("translating ");
				StdLog.String(srcPath$ + "/" + name$);
				StdLog.Ln;
				INC(nTran);
				ExportDoc(srcLoc, dstLoc, name)
			ELSE
				INC(nSkip);
				StdLog.String("skip ");
				StdLog.String(srcPath$ + "/" + name$);
				StdLog.Ln
			END;
			files := files.next
		END
	END CopyLocToHtml;

	PROCEDURE CopyBugs*;
		VAR
			destLoc, loc, sourceLoc: Files.Locator;
	BEGIN
		StdLog.String("Copying started ...");
		StdLog.Ln;
		nDir := 0;
		nTran := 0;
		nSkip := 0;
		nCopy := 0;

		(*	copy gpl licecnce	*)
		CopyFile("gpl-3.0.txt", dest);

		IF copyBinary THEN
			(*	Windows specific  files	*)
			CopyFile("OpenBUGS.exe", dest);
			CopyFile("MultiBUGS.exe", dest);
			CopyFile("MultiBUGS.exe.manifest", dest);
			CopyFile("WorkerBUGS.exe", dest);
			CopyFile("libMultiBUGS.dll", dest);

			CopyFile("libtaucs.dll", dest);
			CopyFile("BackBUGS.lnk", dest);
			CopyFile("Script.odc", dest);
			CopyFile("Win/Code/Console.ocf", dest);
			CopyFile("Code/Console.ocf", dest);
			CopyFile("Code/MPI.ocf", dest);
			CopyFile("Code/MPImsimp.ocf", dest);
			CopyFile("Lin/Code/Console.ocf", dest);

			
			CopyLoc("Bugs/Code", dest);
			CopyLoc("Compare/Code", dest);
			CopyLoc("Correl/Code", dest);
			CopyLoc("Dev/Code", dest);
			CopyLoc("Deviance/Code", dest);
			CopyLoc("Diff/Code", dest);
			CopyLoc("Doodle/Code", dest);
			CopyLoc("Dynamic/Code", dest);
			CopyLoc("Graph/Code", dest);
			CopyLoc("Html/Code", dest);
			CopyLoc("Maps/Code", dest);
			CopyLoc("Math/Code", dest);
			CopyLoc("Models/Code", dest);
			CopyLoc("Monitor/Code", dest);
			CopyLoc("Parallel/Code", dest);
			CopyLoc("Pharmaco/Code", dest);
			CopyLoc("PKBugs/Code", dest);
			CopyLoc("Plots/Code", dest);
			CopyLoc("Ranks/Code", dest);
			CopyLoc("Reliability/Code", dest);
			CopyLoc("Samples/Code", dest);
			CopyLoc("Spatial/Code", dest);
			CopyLoc("Summary/Code", dest);
			CopyLoc("Test/Code", dest);
			CopyLoc("Updater/Code", dest);

			CopyLoc("Form/Code", dest);
			CopyLoc("Host/Code", dest);
			CopyLoc("Lin/Code", dest);
			CopyLoc("Ole/Code", dest);
			CopyLoc("Std/Code", dest);
			CopyLoc("System/Code", dest);
			CopyLoc("Text/Code", dest);
			CopyLoc("Win/Code", dest);
			CopyLoc("Xhtml/Code", dest);
			
			CopyFile("Math/Sym/Func.osf", dest);
			CopyFile("Graph/Sym/Logical.osf", dest);
			CopyFile("Graph/Sym/Nodes.osf", dest);
			CopyFile("Graph/Sym/Rules.osf", dest);
			CopyFile("Graph/Sym/Scalar.osf", dest);
			CopyFile("Graph/Sym/Stack.osf", dest);
			CopyFile("Graph/Sym/Stochastic.osf", dest);
			CopyFile("System/Sym/Math.osf", dest);
			CopyFile("System/Sym/Stores.osf", dest);

			(*	if config and startup files exist overwrite standard BB files	*)
			CopyFile("Code/Config.ocf", dest + "/System");
			CopyFile("Code/Startup.ocf", dest + "/System");
			CopyFile("Code/MPI.ocf", dest + "/System");
			CopyFile("Code/MPImsmpi.ocf", dest + "/System");
			CopyFile("Code/MPIworker.ocf", dest + "/System");
		END;

		IF copyDocu THEN
			CopyLoc("Manuals", dest);
			CopyLoc("GeoBUGS", dest);
			CopyLoc("GeoBUGS/Manuals", dest);
			CopyLoc("PKBugs/Manuals", dest);
			CopyLoc("Reliability", dest);
			CopyLoc("Reliability/Manuals", dest);
			(*	script file that can be used for validating installation	*)
			CopyFile("Script.txt", dest);
		END;
		
		IF copyExamples THEN
			CopyLoc("GeoBUGS/Examples", dest);
			CopyExamplesLocToAscii("GeoBUGS/Examples", dest);
			CopyLoc("PKBugs/Examples", dest);
			CopyExamplesLocToAscii("PKBugs/Examples", dest);
			CopyLoc("Reliability/Examples", dest);
			CopyExamplesLocToAscii("Reliability/Examples", dest);
			CopyLoc("Examples", dest);
			CopyExamplesLocToAscii("Examples", dest);
		END;
		
		IF copySource OR copyBinary THEN
			CopyLoc("Developer", dest);
			CopyLoc("Bugs/Rsrc", dest);
			CopyLoc("Compare/Rsrc", dest);
			CopyLoc("Correl/Rsrc", dest);
			CopyLoc("Deviance/Rsrc", dest);
			CopyLoc("Doodle/Rsrc", dest);
			CopyLoc("Graph/Rsrc", dest);
			CopyLoc("Html/Rsrc", dest);
			CopyLoc("Maps/Rsrc", dest);
			CopyLoc("Math/Rsrc", dest);
			CopyLoc("Models/Rsrc", dest);
			CopyLoc("Monitor/Rsrc", dest);
			CopyLoc("Parallel/Rsrc", dest);
			CopyLoc("PKBugs/Rsrc", dest);
			CopyLoc("Plots/Rsrc", dest);
			CopyLoc("Ranks/Rsrc", dest);
			CopyLoc("Reliability/Rsrc", dest);
			CopyLoc("Samples/Rsrc", dest);
			CopyLoc("Spatial/Rsrc", dest);
			CopyLoc("Summary/Rsrc", dest);
			CopyLoc("Test/Rsrc", dest);
			CopyLoc("Updater/Rsrc", dest);

			CopyLoc("Form/Rsrc", dest);
			CopyLoc("Host/Rsrc", dest);
			CopyLoc("Ole/Rsrc", dest);
			CopyLoc("Std/Rsrc", dest);
			CopyLoc("System/Rsrc", dest);
			CopyLoc("Text/Rsrc", dest);
			CopyLoc("Win/Rsrc", dest);
			CopyLoc("Xhtml/Rsrc", dest);

			CopyFile("Dev/Mod/Browser.odc", dest);
			CopyFile("Dev/Mod/Debug.odc", dest);
			CopyFile("Dev/Mod/ElfLinker.odc", dest);
			CopyFile("Dev/Mod/Search.odc", dest);
			CopyFile("System/Mod/Console.odc", dest);
			CopyFile("System/Mod/MPI.odc", dest);
			CopyFile("System/Mod/MPImslib.odc", dest);
			CopyFile("System/Mod/MPImsimp.odc", dest);

			CopyFile("Dev/Rsrc/Strings.odc", dest);

			PathToLoc("Rsrc", loc);
			IF (loc # NIL) & (loc.res = 0) THEN
				CopyLoc("Rsrc", dest + "/System")
			END
		END;

		(*	BlackBox files	*)
		CopyFile("Docu/BB-Open-Source-License.odc", dest);
		CopyFile("Docu/BB-Licensing-Policy.odc", dest);
		CopyFile("Docu/BB-License.odc", dest);
		CopyFile("Docu/MultiBUGS-License.odc", dest);

		(*	Required for source code license compliance	*)
		CopyFile("Rsrc/About.odc", dest);
		CopyFile("Rsrc/AboutMulti.odc", dest);

		(*	copy source code files	*)
		IF copySource THEN
			CopyLoc("Lin/Mod", dest);
			CopyLoc("Bugs/Mod", dest);
			CopyLoc("Compare/Mod", dest);
			CopyLoc("Correl/Mod", dest);
			CopyLoc("Deviance/Mod", dest);
			CopyLoc("Diff/Mod", dest);
			CopyLoc("Doodle/Mod", dest);
			CopyLoc("Graph/Mod", dest);
			CopyLoc("Html/Mod", dest);
			CopyLoc("Lin/Mod", dest);
			CopyLoc("Maps/Mod", dest);
			CopyLoc("Math/Mod", dest);
			CopyLoc("Models/Mod", dest);
			CopyLoc("Monitor/Mod", dest);
			CopyLoc("Parallel/Mod", dest);
			CopyLoc("Pharmaco/Mod", dest);
			CopyLoc("PKBugs/Mod", dest);
			CopyLoc("Plots/Mod", dest);
			CopyLoc("Ranks/Mod", dest);
			CopyLoc("Reliability/Mod", dest);
			CopyLoc("Samples/Mod", dest);
			CopyLoc("Spatial/Mod", dest);
			CopyLoc("Summary/Mod", dest);
			CopyLoc("Test/Mod", dest);
			CopyLoc("Updater/Mod", dest);
			
			CopyFile("Randnumseeds.odc", dest);
			CopyFile("Test/TestAllModels.odc", dest);
			CopyFile("Dev/Mod/Browser.odc", dest);
			CopyFile("Dev/Mod/Debug.odc", dest);
			CopyFile("Host/Mod/Cmds.odc", dest);
			CopyFile("Host/Mod/Menus.odc", dest);
			CopyFile("Std/Mod/Cmds1.odc", dest);
			CopyFile("Win/Mod/Console.odc", dest);
		END;

		IF produceHTML THEN
			CopyLocToHtml("Manuals", dest);
			CopyLocToHtml("GeoBUGS", dest);
			CopyLocToHtml("GeoBUGS/Manuals", dest);
			CopyLocToHtml("GeoBUGS/Examples", dest);
			CopyLocToHtml("PKBugs", dest);
			CopyLocToHtml("PKBugs/Manuals", dest);
			CopyLocToHtml("PKBugs/Examples", dest);
			CopyLocToHtml("Reliability", dest);
			CopyLocToHtml("Reliability/Manuals", dest);
			CopyLocToHtml("Reliability/Examples", dest);
			CopyLocToHtml("Examples", dest);
			CopyLocToHtml("Developer", dest);
		END;

		Dialog.ShowStatus("");
		StdLog.Int(nCopy);
		StdLog.String(" files copied");
		StdLog.Ln;
		StdLog.Int(nTran);
		StdLog.String(" odc files translated to html");
		StdLog.Ln;
		StdLog.Int(nSkip);
		StdLog.String(" files skipped");
		StdLog.Ln;
		StdLog.Int(nDir);
		StdLog.String(" directories processed");
		StdLog.Ln;
		StdLog.String("Copying finished");
		StdLog.Ln

	END CopyBugs;

	PROCEDURE Maintainer;
	BEGIN
		maintainer := "A.Thomas";
		version := 500
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		dest := "C:/MultiBUGS";
		copyBinary := TRUE;
		copyDocu := TRUE;
		copySource := FALSE;
		copyExamples := TRUE;
		produceHTML := FALSE
	END Init;

BEGIN
	Init
END BugsDistribute.

