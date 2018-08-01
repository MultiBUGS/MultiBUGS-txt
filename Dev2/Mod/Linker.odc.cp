MODULE Dev2Linker;

	(*
		Igor Dehtyarenko (Trurl) , 2016.11
	*)
	
	IMPORT Log, 
		LB := Dev2LnkBase, Load := Dev2LnkLoad, 
		WrPe := Dev2LnkWritePe,
		WrElf := Dev2LnkWriteElf, WrElfS := Dev2LnkWriteElfStatic;
	CONST
		tgtElfStatic = 0; tgtElfExe = 1; tgtElfDll = 2; tgtPeExe = 3;  tgtPeDll = 4; 

	PROCEDURE LinkIt;
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
			Log.String(LB.outputName$); Log.String(" written"); Log.Ln;
		END
	END LinkIt;	
	
(**********************************)
	PROCEDURE Do1Pe*(modname: ARRAY OF CHAR);
	BEGIN
		LB.Init(tgtPeExe);
		Load.AddModule(modname);
		LinkIt
	END Do1Pe;	

	PROCEDURE Do1Dll*(modname: ARRAY OF CHAR);
	BEGIN
		LB.Init(tgtPeDll);;
		Load.ExportModule(modname);
		LinkIt
	END Do1Dll;	

	PROCEDURE Do1ElfLinux* (modname: ARRAY OF CHAR);
	BEGIN
		LB.Init(tgtElfExe);
		LB.opt.OSABI := WrElf.ELFOSABI_NONE;
		LB.opt.elfInterpreter := WrElf.linuxInterpreter;
		LB.opt.rpath := ".";
		Load.AddModule(modname);
		LinkIt
	END Do1ElfLinux;

	PROCEDURE Do1ElfOpenBSD* (modname: ARRAY OF CHAR);
	BEGIN
		LB.Init(tgtElfExe);
		LB.opt.OSABI := WrElf.ELFOSABI_NONE;
		LB.opt.elfInterpreter := WrElf.openBSDInterpreter;
		LB.opt.rpath := ".";
		Load.AddModule(modname);
		LinkIt
	END Do1ElfOpenBSD;

	PROCEDURE Do1ElfFreeBSD* (modname: ARRAY OF CHAR);
	BEGIN
		LB.Init(tgtElfExe);
		LB.opt.OSABI := WrElf.ELFOSABI_FREEBSD;
		LB.opt.elfInterpreter := WrElf.freeBSDInterpreter;
		LB.opt.rpath := ".";
		Load.AddModule(modname);
		Load.ExportVariable('Dumb', '__progname');
		Load.ExportVariable('Dumb', 'environ');
		LinkIt
	END Do1ElfFreeBSD;

	PROCEDURE Do1ElfSt*(modname: ARRAY OF CHAR);
	BEGIN
		LB.Init(tgtElfStatic);
		Load.AddModule(modname);
		LinkIt
	END Do1ElfSt;	

 	PROCEDURE Do1So*(modname: ARRAY OF CHAR);
	BEGIN
		LB.Init(tgtElfDll);
		Load.ExportModule(modname);
		LinkIt
	END Do1So;	

	PROCEDURE BlackBoxPe*;
	BEGIN
		LB.Init(tgtPeExe);;
		LB.dynaInit := ##=>TRUE
FALSE##<=;
		LB.outputName := "BlackBox1";
		LB.mainName := "Kernel";
		Load.AddModule('HostFiles');
		Load.AddModule('StdLoader');
		Load.ExportAll;
		LinkIt
	END BlackBoxPe;	
	
	PROCEDURE BlackBoxElfLinux*;
	BEGIN
		LB.Init(tgtElfExe);
		LB.dynaInit := ##=>TRUE
FALSE##<=;
		LB.opt.OSABI := WrElf.ELFOSABI_NONE;
		LB.opt.elfInterpreter := WrElf.linuxInterpreter;
		LB.outputName := "BlackBox1";
		LB.mainName := "Kernel";
		Load.AddModule('HostFiles');
		Load.AddModule('StdLoader');
		Load.ExportAll;
		LinkIt
	END BlackBoxElfLinux;

	PROCEDURE BlackBoxElfOpenBSD*;
	BEGIN
		LB.Init(tgtElfExe);
		LB.dynaInit := ##=>TRUE
FALSE##<=;
		LB.opt.OSABI := WrElf.ELFOSABI_NONE;
		LB.opt.elfInterpreter := WrElf.openBSDInterpreter;
		LB.outputName := "BlackBox1";
		LB.mainName := "Kernel";
		Load.AddModule('HostFiles');
		Load.AddModule('StdLoader');
		Load.ExportAll;
		LinkIt
	END BlackBoxElfOpenBSD;

	PROCEDURE BlackBoxElfFreeBSD*;
	BEGIN
		LB.Init(tgtElfExe);
		LB.dynaInit := ##=>TRUE
FALSE##<=;
		LB.opt.OSABI := WrElf.ELFOSABI_FREEBSD;
		LB.opt.elfInterpreter := WrElf.freeBSDInterpreter;
		LB.outputName := "BlackBox1";
		LB.mainName := "Kernel";
		Load.AddModule('HostFiles');
		Load.AddModule('StdLoader');
		Load.ExportVariable('Kernel', '__progname');
		Load.ExportVariable('Kernel', 'environ');
		Load.ExportAll;
		LinkIt
	END BlackBoxElfFreeBSD;
	
END Dev2Linker.

DevCompiler.CompileThis Dev2LnkBase Dev2LnkLoad Dev2LnkWritePe Dev2LnkWriteElf Dev2LnkWriteElfStatic Dev2Linker
DevDebug.UnloadThis Dev2Linker Dev2LnkLoad
Dev2LnkWritePe Dev2LnkWriteElf Dev2LnkWriteElfStatic
 Dev2LnkBase

"Dev2Linker.Do1Pe('Simple')"   "DevDecExe.Decode('','Simple.exe')"
"Dev2Linker.Do1Dll('Mydll')"
 Dev2Linker.BlackBoxPe


DevCompiler.CompileThis al ali test0 testa Libc testc

"Dev2Linker.Do1So('al')"
"Dev2Linker.Do1Elf('testa')"
"Dev2Linker.Do1Elf('testc')"
"Dev2Linker.Do1ElfSt('test0')"


 Dev2Linker.BlackBoxElfLinux
 Dev2Linker.BlackBoxElfOpenBSD
 Dev2Linker.BlackBoxElfFreeBSD

