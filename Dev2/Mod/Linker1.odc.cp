MODULE Dev2Linker1;

	(* Alexander Shiryaev, 2016.11 *)

	IMPORT
		LB := Dev2LnkBase, Load := Dev2LnkLoad,
		WrPe := Dev2LnkWritePe,
		WrElf := Dev2LnkWriteElf, WrElfS := Dev2LnkWriteElfStatic,
		Kernel, Files, Log := StdLog, Dialog, DevCommanders, TextMappers;

	CONST
		tgtElfStatic = 0; tgtElfExe = 1; tgtElfDll = 2; tgtPeExe = 3;  tgtPeDll = 4; 

	VAR
		W: TextMappers.Formatter;

	PROCEDURE LinkIt0;
		VAR codeBase, dataBase, varsBase : INTEGER;
	BEGIN
		LB.BeginLinking;
		IF LB.outPe THEN	
			WrPe.Init;
			WrPe.GetBases(codeBase, dataBase, varsBase);
			LB.SetAddr(codeBase, dataBase, varsBase);
			LB.DoFixups;
			WrPe.WriteOut;
		ELSIF LB.outElf & LB.opt.elfStatic THEN
			WrElfS.Init;
			WrElfS.GetBases(codeBase, dataBase, varsBase);
			LB.SetAddr(codeBase, dataBase, varsBase);
			LB.DoFixups;
			WrElfS.WriteOut;
		ELSIF LB.outElf THEN
			WrElf.Init;
			WrElf.GetBases(codeBase, dataBase, varsBase);
			LB.SetAddr(codeBase, dataBase, varsBase);
			LB.DoFixups;
			WrElf.WriteOut;
		END;
		IF ~LB.error THEN	
			W.WriteString(LB.outputName$); W.WriteString(" written")
		END
	END LinkIt0;

	PROCEDURE LinkIt;
		VAR S: TextMappers.Scanner; name: Files.Name; end: INTEGER;
			modName: TextMappers.String;
			error, isFreeBSD, flag0: BOOLEAN;
	BEGIN
		Dialog.ShowStatus("linking");
		error := FALSE;
		IF DevCommanders.par = NIL THEN RETURN END;
		S.ConnectTo(DevCommanders.par.text);
		S.SetPos(DevCommanders.par.beg);
		end := DevCommanders.par.end;
		DevCommanders.par := NIL;
		isFreeBSD := FALSE;
		W.ConnectTo(Log.buf); S.Scan;
		IF S.type = TextMappers.string THEN
			IF LB.target = tgtElfExe THEN
				IF S.string = "Linux" THEN
					LB.opt.OSABI := WrElf.ELFOSABI_NONE;
					LB.opt.elfInterpreter := WrElf.linuxInterpreter
				ELSIF S.string = "FreeBSD" THEN
					LB.opt.OSABI := WrElf.ELFOSABI_FREEBSD;
					LB.opt.elfInterpreter := WrElf.freeBSDInterpreter;
					isFreeBSD := TRUE
				ELSIF S.string = "OpenBSD" THEN
					LB.opt.OSABI := WrElf.ELFOSABI_NONE;
					LB.opt.elfInterpreter := WrElf.openBSDInterpreter
				ELSE
					W.WriteString("unknown OS: "); W.WriteString(S.string); W.WriteLn;
					error := TRUE
				END;
				S.Scan
			END;
			name := S.string$; S.Scan;
			IF (S.type = TextMappers.char) & (S.char = ".") THEN S.Scan;
				IF S.type = TextMappers.string THEN
					Kernel.MakeFileName(name, S.string); S.Scan
				END
			END;
			IF (S.type = TextMappers.char) & (S.char = ":") THEN S.Scan;
				IF (S.type = TextMappers.char) & (S.char = "=") THEN S.Scan;
					LB.outputName := name$;
					WHILE (S.start < end) & (S.type = TextMappers.string) DO
						modName := S.string$; S.Scan;
						flag0 := FALSE;
						WHILE (S.start < end) & (S.type = TextMappers.char) &
							((S.char = "+") OR (S.char = "$")) DO
							IF S.char = "+" THEN LB.KernelName := SHORT(modName$);
								flag0 := TRUE
							ELSIF S.char = "$" THEN LB.mainName := SHORT(modName$);
								flag0 := TRUE
							ELSE
							END;
							S.Scan
						END;
						IF ~flag0 THEN
							IF LB.target = tgtElfStatic THEN
								Load.AddModule(modName$)
							ELSIF LB.target = tgtElfExe THEN
								Load.AddModule(modName$)
							ELSIF LB.target = tgtElfDll THEN
								Load.ExportModule(modName$)
							ELSIF LB.target = tgtPeExe THEN
								Load.AddModule(modName$)
							ELSIF LB.target = tgtPeDll THEN
								Load.ExportModule(modName$)
							ELSE HALT(102)
							END
						END
					END;
					IF LB.target = tgtElfStatic THEN
					ELSIF LB.target = tgtElfExe THEN
						IF isFreeBSD THEN
							Load.ExportVariable(LB.KernelName$, "__progname");
							Load.ExportVariable(LB.KernelName$, "environ")
						END;
						Load.ExportAll
					ELSIF LB.target = tgtElfDll THEN
					ELSIF LB.target = tgtPeExe THEN
						Load.ExportAll
					ELSIF LB.target = tgtPeDll THEN
					ELSE HALT(101)
					END;
					IF ~error THEN LinkIt0 END
				ELSE W.WriteString(" := missing")
				END
			ELSE W.WriteString(" := missing")
			END;
			W.WriteLn; Log.text.Append(Log.buf)
		END;
		IF error THEN Dialog.ShowStatus("failed") ELSE Dialog.ShowStatus("ok") END;
		W.ConnectTo(NIL); S.ConnectTo(NIL)
	END LinkIt;

	PROCEDURE LinkPeExe*;
	BEGIN
		LB.Init(tgtPeExe);
		LB.dynaInit := FALSE;
		LinkIt
	END LinkPeExe;

	PROCEDURE LinkPeDll*;
	BEGIN
		LB.Init(tgtPeDll);
		LinkIt
	END LinkPeDll;

	PROCEDURE LinkElfStatic*;
	BEGIN
		LB.Init(tgtElfStatic);
		LinkIt
	END LinkElfStatic;

	PROCEDURE LinkElfDll*;
	BEGIN
		LB.Init(tgtElfDll);
		LinkIt
	END LinkElfDll;

	PROCEDURE LinkElfExe*;
	BEGIN
		LB.Init(tgtElfExe);
		LB.dynaInit := FALSE;
		LinkIt
	END LinkElfExe;

END Dev2Linker1.

DevCompiler.CompileThis Dev2LnkBase Dev2LnkLoad Dev2LnkWritePe Dev2LnkWriteElf Dev2LnkWriteElfStatic Dev2LnkChmod Dev2Linker1

DevDebug.UnloadThis Dev2Linker1 Dev2LnkLoad Dev2LnkWritePe Dev2LnkWriteElf Dev2LnkWriteElfStatic Dev2LnkChmod Dev2LnkBase


Dev2Linker1.LinkPeExe BlackBox1 := Kernel$+ HostFiles StdLoader ~

Dev2Linker1.LinkElfExe Linux BlackBox1 := Kernel$+ HostFiles StdLoader ~

Dev2Linker1.LinkElfExe FreeBSD BlackBox1 := Kernel$+ HostFiles StdLoader ~

Dev2Linker1.LinkElfExe OpenBSD BlackBox1 := Kernel$+ HostFiles StdLoader ~

