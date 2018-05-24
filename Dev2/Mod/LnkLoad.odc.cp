MODULE Dev2LnkLoad;
	
	(*
		Igor Dehtyarenko (Trurl) , 2016.11
	*)
	
	IMPORT LB := Dev2LnkBase, SYSTEM, Files;
	
	CONST
		newRecName = "NewRec"; NewRecFP = 4E27A847H;
		newArrName = "NewArr"; NewArrFP = 76068C78H;
		
		(* meta interface consts *)
		mConst = 1; mTyp = 2; mVar = 3; mProc = 4; 
		mInternal = 1;  mExported = 4;

		(* Module descriptor fields *)
			mdf_next = 0; mdf_opts = 4; mdf_refcnt = 8; mdf_term = 40; 
			mdf_names = 84;  mdf_imports = 92; mdf_export= 96;

	TYPE 
		Module =  LB.Module;
		Section = LB.Section;
		STRING = LB.STRING;
		StrPtr = POINTER TO ARRAY [untagged] OF SHORTCHAR;
	
	PROCEDURE^ LoadModule(IN modName: STRING):Module;
	
	PROCEDURE ThisMod*(IN name: STRING): Module;
		VAR mod: Module; 
	BEGIN
		IF name[0] = "$" THEN
			mod := LB.ThisDll(name)
		ELSE
			mod := LB.modList;
			WHILE (mod # NIL) & (mod.name$ # name) DO mod := mod.next END;
			IF mod = NIL THEN
				mod := LoadModule(name);
				IF mod # NIL THEN LB.RegMod(mod) END;
			END;
		END;
		RETURN mod
	END ThisMod;

	PROCEDURE Read4(inp: Files.Reader; VAR x: INTEGER);
		VAR b: BYTE; y: INTEGER;
	BEGIN
		inp.ReadByte(b); y := b MOD 256;
		inp.ReadByte(b); y := y + 100H * (b MOD 256);
		inp.ReadByte(b); y := y + 10000H * (b MOD 256);
		inp.ReadByte(b); x := y + 1000000H * b
	END Read4;

	PROCEDURE ReadNum(inp: Files.Reader; VAR x: INTEGER);
		VAR b: BYTE; s, y: INTEGER;
	BEGIN
		s := 0; y := 0; 
		inp.ReadByte(b);
		WHILE b < 0 DO 
			INC(y, ASH(b + 128, s)); 
			INC(s, 7); 
			inp.ReadByte(b) 
		END;
		x := ASH((b + 64) MOD 128 - 64, s) + y
	END ReadNum;
	
	PROCEDURE ReadName (inp: Files.Reader; VAR name: STRING);
		VAR i: INTEGER; b: BYTE;
	BEGIN 
		i := 0;
		REPEAT
			inp.ReadByte(b); name[i] := SHORT(CHR(b)); INC(i)
		UNTIL b = 0
	END ReadName;

	PROCEDURE Get3(VAR d:ARRAY OF BYTE; a: INTEGER): INTEGER;
	BEGIN
		RETURN d[a] MOD 256 + d[a+1] MOD 256 * 256 + d[a+2] * 10000H;
	END Get3;

	PROCEDURE Get4(VAR d:ARRAY OF BYTE; a: INTEGER): INTEGER;
	BEGIN
		RETURN ((d[a+3] * 256 + d[a+2] MOD 256) * 256 + d[a+1] MOD 256) * 256 + d[a] MOD 256
	END Get4;

	PROCEDURE Put4(VAR d:ARRAY OF BYTE; a: INTEGER; x: INTEGER);
	BEGIN
		d[a] := SHORT(SHORT(x));
		d[a+1] := SHORT(SHORT(x DIV 100H));
		d[a+2] := SHORT(SHORT(x DIV 10000H));
		d[a+3] := SHORT(SHORT(x DIV 1000000H))
	END Put4;
 
	PROCEDURE DWord(mod: Module; a: INTEGER): INTEGER;
	BEGIN
		RETURN Get4(mod.data.bytes, a);
	END DWord;

	PROCEDURE MWord(mod: Module; offs: INTEGER): INTEGER;
	BEGIN
		RETURN Get4(mod.data.bytes, mod.mDesc + offs);
	END MWord;

	PROCEDURE PutMWord(mod: Module; offs: INTEGER; x: INTEGER);
	BEGIN
		Put4(mod.data.bytes, mod.mDesc + offs, x);
	END PutMWord;

	PROCEDURE GetPName(mod: Module; idx: INTEGER): StrPtr;
	VAR pNames: INTEGER; 
	BEGIN
		pNames := MWord(mod, mdf_names);	
		RETURN SYSTEM.VAL(StrPtr, SYSTEM.ADR(mod.data.bytes[pNames + idx]));
	END GetPName;

	PROCEDURE GetDir(mod: Module; VAR pDir, numobj: INTEGER);
	BEGIN
		pDir := MWord(mod, mdf_export);	(* numobj:4 {Object}	*)
		numobj := DWord(mod,  pDir); 
		pDir := pDir + 4; 
	END GetDir;


	PROCEDURE SearchFprint(mod: Module; fprint: INTEGER): INTEGER;
	VAR p, numobj, fp: INTEGER; 
	BEGIN
		GetDir(mod, p, numobj);
		fp := DWord(mod,  p + 4);
		WHILE (fp # fprint) & (numobj > 0) DO
			INC(p, 16); DEC(numobj);
			fp := DWord(mod,  p + 4);
		END;
		IF fp = fprint THEN RETURN p ELSE RETURN -1 END;
	END SearchFprint;

	PROCEDURE SearchName(mod: Module; IN name: STRING): INTEGER;
	VAR  l, r, m, p, id: INTEGER; 
			p0, numobj: INTEGER; 
			s:StrPtr;
	BEGIN
		GetDir(mod, p0, numobj);
		l := 0; r := numobj-1; 
		WHILE (l <= r) DO	(* binary search *)
			m := (l + r) DIV 2; 
			p := p0 + m * 16;
			id := DWord(mod, p + 8);
			s := GetPName(mod,  id DIV 256);
			IF name = s$ THEN
				RETURN p 
			ELSIF name > s$ THEN 
				l := m + 1 
			ELSE 
				r := m - 1
			END
		END;
		RETURN -1
	END SearchName;

	
	PROCEDURE SearchObj(mod: Module; IN name: STRING; mode, fprint, opt: INTEGER; 
			OUT sec: Section; OUT adr: INTEGER);
	VAR pObj, id, md,  fp: INTEGER; 
	BEGIN
		sec:= NIL; adr := -1;
		IF mod.isDll THEN
			IF mode = mProc THEN
				sec := mod.code;
				LB.ImportProc(mod, name, adr);
			ELSIF mode = mTyp THEN
				LB.Error(name$ + " import type from DLL"); 
			ELSIF mode = mVar THEN
				LB.Error(name$ + " import var from DLL"); 
			END;
		
		ELSIF name = "" THEN 
			pObj := SearchFprint(mod, fprint);
			IF pObj < 0 THEN
				LB.Error("anonymous type not found"); 
			ELSE
				id := DWord(mod,  pObj + 8);
				IF (id MOD 16 = mTyp) & (id DIV 256 = 0) THEN
					adr := DWord(mod,  pObj + 12); 
					sec := mod.data;
				ELSE
					LB.Error("anonymous type not found"); 
				END;
			END
			
		ELSE 
			pObj := SearchName(mod, name);
			IF pObj < 0 THEN
				LB.Error(mod.name$+ "."+ name$ + " not found "); 
			ELSE
				fp := DWord(mod, pObj);
				id := DWord(mod, pObj + 8);
				md := id MOD 16;
				IF ODD(opt) & (mode = mTyp) THEN fp := DWord(mod, pObj + 4) END;
				IF md # mode THEN 
					LB.Error(mod.name$+ "."+ name$ + " has wrong class");
				ELSIF fp # fprint THEN
					LB.Error(mod.name$+ "."+ name$ + " has wrong fprint");
				ELSIF mode = mTyp THEN
					IF (opt > 1) & (id DIV 16 MOD 16 # mExported) THEN
						LB.Error(mod.name$+ "."+ name$ + " has wrong visibility");
					END;
					sec := mod.data; 
					adr := DWord(mod,  pObj + 12)
				ELSIF mode = mVar THEN
					sec := mod.vars; 
					adr := DWord(mod,  pObj + 4);
				ELSIF mode = mProc THEN
					sec := mod.code; 
					adr := DWord(mod,  pObj + 4)
				END;
			END;
		END;
	END SearchObj;


	PROCEDURE LoadModule(IN modName: STRING):Module;
	VAR mod: Module;
			file: Files.File;
			inp: Files.Reader; 
			nofimp: INTEGER;
			import: ARRAY 128 OF Module;

		PROCEDURE ReadHeader;
		CONST	OFtag = 6F4F4346H; (*  "oOCF" *)
		VAR mtag, processor, i : INTEGER;
				iname: LB.Name;
				headSize, descSize, metaSize, codeSize, varsSize: INTEGER;
				dataSize: INTEGER;
		BEGIN
			Read4(inp, mtag);         (* mtag = OFtag *);
			Read4(inp, processor);		(* processor = proc386 *)
			Read4(inp, headSize); 
			Read4(inp, metaSize); 
			Read4(inp, descSize); 
			Read4(inp, codeSize); 
			Read4(inp, varsSize); 
			dataSize := metaSize + descSize;
			mod.mDesc := metaSize;
			mod.data := LB.NewSection(dataSize);	
			mod.code := LB.NewSection(codeSize);	
			mod.vars := LB.NewSection(varsSize);	

			ReadNum(inp, nofimp);
			ReadName(inp, iname); 
			mod.name := modName$;
			FOR i := 1 TO nofimp  DO 
				ReadName(inp, iname);
				IF iname = "$$" THEN iname := LB.KernelName END;
				import[i] := ThisMod(iname);
			END;
			inp.SetPos(headSize);
		END ReadHeader;

		PROCEDURE ReadSection(sec: Section);
		BEGIN
			NEW(sec.bytes, sec.size);
			inp.ReadBytes(sec.bytes, 0, sec.size); 
		END ReadSection;
		
		PROCEDURE WrongLink (fixt, link : INTEGER);
		BEGIN
			LB.Error(" Wrong link in " + modName$); 
		END WrongLink;

		PROCEDURE FixChain(link, offset: INTEGER; sec: Section; val: INTEGER);
			CONST absolute = 100; relative = 101; copy = 102;  table = 103; tableend = 104;
			VAR fixtype, fixdat: INTEGER;
		BEGIN
			WHILE link # 0 DO
				IF link > 0 THEN (* code *)
					fixdat := Get3(mod.code.bytes, link);
					fixtype := mod.code.bytes[link+3]; 
					IF fixtype = absolute THEN
						Put4(mod.code.bytes, link, offset + val); 
						LB.FixCode(mod, link, sec);
						link := fixdat;
					ELSIF fixtype = relative THEN
						Put4(mod.code.bytes, link, offset + val - link - 4);
						LB.FixRel(mod, link, sec); 
						link := fixdat;
					ELSIF fixtype = table THEN 
						Put4(mod.code.bytes, link, fixdat);
						LB.FixCode(mod, link, sec); 
						link := link + 4;
					ELSIF fixtype = tableend THEN
						Put4(mod.code.bytes, link, fixdat);
						LB.FixCode(mod, link, sec);
						link := 0;
					ELSE
						WrongLink(fixtype, link);
						RETURN;
					END;
					
				ELSE  (* data *)
					link := -link;
					fixdat := Get3(mod.data.bytes, link);
					fixtype := mod.data.bytes[link+3]; 
					IF fixtype = absolute THEN 
						Put4(mod.data.bytes, link, offset + val); 
						LB.FixData(mod, link,  sec);
						link := fixdat;
					ELSIF fixtype = copy THEN
						Put4(mod.data.bytes, link, offset + val);
						LB.FixCopy(mod, link, sec);
						link := fixdat;
					ELSE
						WrongLink(fixtype, link);
						RETURN;
					END;
				END;
			END;
		END FixChain;

		
		PROCEDURE FixLocal(sec: Section; val: INTEGER);
		VAR offset, link: INTEGER;
		BEGIN
			ReadNum(inp, link);
			WHILE link # 0 DO
				ReadNum(inp, offset);
				FixChain(link, offset, sec, val);
				ReadNum(inp, link)
			END
		END FixLocal;

		PROCEDURE FixExt(imod:Module; IN name: STRING; mode, fp, opt: INTEGER);
		VAR offset, link: INTEGER; sec: Section; adr: INTEGER; 
		BEGIN
			SearchObj(imod, name, mode, fp, opt, sec, adr);
			ReadNum(inp, link);
			WHILE link # 0 DO
				ReadNum(inp, offset);
				FixChain(link, offset, sec, adr);
				ReadNum(inp,link)
			END
		END FixExt;

		PROCEDURE FixKernel(IN name: STRING; fp: INTEGER);
		VAR offset, link: INTEGER; sec: Section; oadr: INTEGER; 
		BEGIN
			ReadNum(inp, link);
			IF link # 0 THEN
				IF (LB.kernelMod # NIL) & (mod # LB.kernelMod)  THEN
					SearchObj(LB.kernelMod, name, mProc, fp, -1, sec, oadr);
					WHILE link # 0 DO
						ReadNum(inp, offset);
						FixChain(link, offset, sec, oadr);
						ReadNum(inp, link)
					END
				ELSE
					LB.Error("no kernel"); RETURN
				END
			END;
		END FixKernel;

		PROCEDURE ReadFixups;
		VAR mno : INTEGER; otag, fprint, opt: INTEGER; 
				name: LB.Name;
		BEGIN
			(* FixBlk *)
			FixKernel(newRecName, NewRecFP); 
			FixKernel(newArrName, NewArrFP);
			FixLocal(mod.data, 0);	(* Const, Meta *); 
			FixLocal(mod.data, mod.mDesc);	(* Descriptors  *)
			FixLocal(mod.code, 0);	(* proc, CaseLinks, Code.links *)
			FixLocal(mod.vars, 0); 
			(* UseBlk: dlls, modules *)
			FOR mno := 1 TO nofimp DO
				ReadNum(inp, otag);
				WHILE otag # 0 DO
					ReadName(inp, name); 
					ReadNum(inp, fprint); 
					IF otag = mTyp THEN ReadNum(inp, opt) ELSE opt := 0 END;
					IF otag # mConst THEN 
						FixExt(import[mno], name, otag, fprint, opt);
						IF LB.error THEN RETURN END;  
					END;
					ReadNum(inp, otag)
				END;
			END;	
		END ReadFixups;
		
		PROCEDURE FixImpRef;
		VAR imod: Module; i, imptab, refcnt: INTEGER; 
		BEGIN
			imptab := MWord(mod, mdf_imports);	
			FOR i := 1 TO nofimp DO
				imod :=	import[i];
				IF ~imod.isDll THEN
					(* import ref *)
					Put4(mod.data.bytes, imptab, imod.mDesc);	(* ->DescBlk*)
					LB.FixData(mod, imptab, imod.data);
					(* inc ref count of imported module *)
					refcnt := MWord(imod, mdf_refcnt);	
					PutMWord(imod, mdf_refcnt, refcnt + 1);	
				END;
				INC(imptab, 4);
			END;
		END FixImpRef;

		PROCEDURE SetOpts;
		VAR opts: SET; 
		BEGIN
			opts := BITS(MWord(mod, mdf_opts));
			IF ~LB.dynaInit THEN INCL(opts, 16) END;	(* init bit (16) *)
			IF LB.outDll THEN INCL(opts, 24) END;	(* dll bit (24) *)
			PutMWord(mod, mdf_opts, ORD(opts)); 
		END SetOpts;

	BEGIN
		file := LB.ThisObjFile(modName$);
		IF file = NIL THEN
			LB.Error(modName$ + " not found"); 
			RETURN NIL;
		END;
		inp := file.NewReader(NIL); 
		inp.SetPos(0);
		NEW(mod);
		mod.isDll := FALSE;

		ReadHeader;
		ReadSection(mod.data); 
		ReadSection(mod.code); 
		LB.InitFixTable(mod.dataFix, mod.data); 
		LB.InitFixTable(mod.codeFix, mod.code);
		ReadFixups;
		mod.termadr := MWord(mod, mdf_term);
		FixImpRef;
		SetOpts;
		IF LB.lastMod # NIL THEN 
			PutMWord(mod, mdf_next, LB.lastMod.mDesc); 
			LB.FixData(mod, mod.mDesc, LB.lastMod.data);
		END;
		file.Close;
		RETURN mod;
	END LoadModule;

	PROCEDURE CollectExports(mod: Module);
	VAR p, numobj: INTEGER; 
			id, vis, mode, addr: INTEGER; 
			name: StrPtr;
	BEGIN
		IF ~mod.exported THEN
			GetDir(mod, p, numobj);
			WHILE numobj > 0 DO
				id := DWord(mod,  p + 8);	(* id = nameIdx*256 + vis*16 + mode *)
				mode := id MOD 16;
				vis := id DIV 16 MOD 16;
				IF (vis # mInternal) & (mode = mProc) THEN
					name := GetPName(mod, id DIV 256);
					addr := DWord(mod,  p + 4);
					LB.ExportProc(mod, name$, addr);
				END;
				DEC(numobj); INC(p, 16)
			END;
			mod.exported := TRUE;
		END;
	END CollectExports;

	PROCEDURE ExportVariable*(IN modname: ARRAY OF CHAR; varname: ARRAY OF SHORTCHAR);
	VAR p, numobj: INTEGER; 
			id, vis, mode, addr: INTEGER; 
			name: StrPtr;
			mod: Module;
	BEGIN
		mod := LB.modList;
		WHILE mod # NIL DO
			(* Log.String(mod.name$);Log.Ln; *)
			mod := mod.next 
		END;
		(* Log.String("HIA: varname = " + varname$); Log.Ln; *)

		mod := ThisMod(SHORT(modname$));
				
		GetDir(mod, p, numobj);
		WHILE numobj > 0 DO
			id := DWord(mod,  p + 8);	(* id = nameIdx*256 + vis*16 + mode *)
			mode := id MOD 16;
			vis := id DIV 16 MOD 16;
			IF (vis # mInternal) & (mode = mVar) THEN
				name := GetPName(mod, id DIV 256);
				
				(* Log.String("HIA: name = " + name$); Log.Ln; *)
				addr := DWord(mod,  p + 4);
				IF varname$ = name$ THEN
					(* Log.String("Export var " + varname);Log.Ln; *)
					LB.ExportVar(mod, name$, addr);
				END;
			END;
			DEC(numobj); INC(p, 16)
		END;
	END ExportVariable;

	PROCEDURE AddModule*(IN modname: ARRAY OF CHAR);
	VAR mod: Module;
	BEGIN
		mod := ThisMod(SHORT(modname$));
	END AddModule;

	PROCEDURE MainModule*(IN modname: ARRAY OF CHAR);
	VAR mod: Module;
	BEGIN
		mod := ThisMod(SHORT(modname$));
		IF mod # NIL THEN LB.mainMod := mod END;
	END MainModule;
	
	PROCEDURE ExportModule*(IN modname: ARRAY OF CHAR);
	VAR mod: Module;
	BEGIN
		mod := ThisMod(SHORT(modname$));
		IF mod # NIL THEN CollectExports(mod) END;
	END ExportModule;

	PROCEDURE ExportAll*;
	VAR mod: Module;
	BEGIN
		mod := LB.modList;
		WHILE (mod # NIL) DO
			CollectExports(mod); 
			mod := mod.next 
		END;
	END ExportAll;

END Dev2LnkLoad.

