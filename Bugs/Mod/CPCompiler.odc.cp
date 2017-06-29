(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

(*	Un-comment the blue code to see what the module is doing	*)

(* There is a problem when we want to internalize a BUGS model that uses dynamic nodes. We do not have the code for the dynamic modules. This is fixed by having the externalize procedure copy the key of the compiled dynamic module out to disc file and the internalize procedure read the key it in from disc file, regenerate the source code. compile it and load it	*)

MODULE BugsCPCompiler;

	

	IMPORT
		Files, Kernel, Meta, Services, Stores, Strings, Views, 
		BugsCPWrite, 
		DevCPB, DevCPM, DevCPP, DevCPT, DevCPV486, 
		GraphLogical, GraphNodes, GraphStochastic, 
		TextMappers, TextModels, TextViews;

	TYPE
		FactoryList = POINTER TO RECORD
			ops: POINTER TO ARRAY OF INTEGER;
			numConsts, numLogicals, numStochs, numOps: INTEGER;
			factory: GraphNodes.Factory;
			next: FactoryList
		END;

	VAR
		numDynamic: INTEGER;
		factoryList: FactoryList;
		debug*: BOOLEAN;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;


	PROCEDURE Compile (source: TextModels.Reader; OUT ok: BOOLEAN);
		CONST
			checks = 0; assert = 2; obj = 3; ref = 4; allref = 5; srcpos = 6;
			defOpts = {checks, assert, obj, ref, allref, srcpos};
		VAR
			ext, new: BOOLEAN;
			opts: SET;
			p: DevCPT.Node;
	BEGIN
		opts := defOpts;
		DevCPM.Init(source, NIL);
		DevCPT.Init(opts);
		DevCPB.typSize := DevCPV486.TypeSize;
		DevCPT.processor := DevCPV486.processor;
		DevCPP.Module(p);
		IF ~DevCPM.noerr THEN
			DevCPM.InsertMarks(source.Base());
			Views.OpenAux(TextViews.dir.New(source.Base()), "Error in compiling");
			HALT(0)
		END;		
		IF DevCPM.noerr THEN
			IF DevCPT.libName # "" THEN EXCL(opts, obj) END;
			DevCPV486.Init(opts); DevCPV486.Allocate; DevCPT.Export(ext, new);
			IF DevCPM.noerr & (obj IN opts) THEN
				DevCPV486.Module(p)
			END;
			DevCPV486.Close
		END;
		DevCPM.DeleteNewSym;
		DevCPT.Close;
		ok := DevCPM.noerr;
		DevCPM.Close;
		p := NIL;
		Services.Collect
	END Compile;

	(*	Writes new Component Pascal module to represent stack node. Compiles this module and
	then loads module so that the factory object to create new nodes of this type can be accessed. 	*)

	PROCEDURE CreateFactory (IN args: GraphStochastic.ArgsLogical): GraphNodes.Factory;
		VAR
			rd: TextModels.Reader;
			f: TextMappers.Formatter;
			ok: BOOLEAN;
			modName, timeStamp: ARRAY 128 OF CHAR;
			factory: GraphNodes.Factory;
			text: TextModels.Model;
			item: Meta.Item;
	BEGIN
		Strings.IntToString(GraphNodes.timeStamp, timeStamp);
		Strings.IntToString(numDynamic, modName);
		modName := "DynamicNode" + modName + "_" + timeStamp;
		text := TextModels.dir.New();
		f.ConnectTo(text);
		BugsCPWrite.WriteModule(args, numDynamic, f);
		IF debug THEN 
			Views.OpenAux(TextViews.dir.New(text), modName$); 
			Meta.LookupPath("CpcBeautifier.Beautify", item);
			 item.Call(ok)
		END;
		rd := text.NewReader(NIL);
		rd.SetPos(0);
		Compile(rd, ok);
		ASSERT(ok, 54);
		factory := GraphNodes.InstallFactory(modName + ".Install");
		ASSERT(factory # NIL, 55);
		RETURN factory
	END CreateFactory;

	PROCEDURE StoreFactory (fact: GraphNodes.Factory; IN args: GraphStochastic.ArgsLogical);
		VAR
			elem: FactoryList;
			i, numOps: INTEGER;
	BEGIN
		numOps := args.numOps;
		NEW(elem);
		elem.numStochs := args.numStochs;
		elem.numLogicals := args.numLogicals;
		elem.numConsts := args.numConsts;
		elem.numOps := args.numOps;
		NEW(elem.ops, numOps);
		i := 0; WHILE i < numOps DO elem.ops[i] := args.ops[i]; INC(i) END;
		elem.factory := fact;
		elem.next := factoryList;
		factoryList := elem;
		INC(numDynamic)
	END StoreFactory;

	PROCEDURE FindFactory (IN args: GraphStochastic.ArgsLogical): GraphNodes.Factory;
		VAR
			i, numOps: INTEGER;
			list: FactoryList;
			factory: GraphNodes.Factory;
	BEGIN
		factory := NIL;
		list := factoryList;
		numOps := args.numOps;
		LOOP
			IF list = NIL THEN EXIT END;
			IF numOps = LEN(list.ops) THEN
				i := 0; WHILE (i < numOps) & (args.ops[i] = list.ops[i]) DO INC(i) END;
				IF i = numOps THEN EXIT END
			END;
			list := list.next
		END;
		IF list # NIL THEN factory := list.factory END;
		RETURN factory
	END FindFactory;

	PROCEDURE CreateLogical* (IN args: GraphStochastic.ArgsLogical): GraphLogical.Node;
		VAR
			fact: GraphNodes.Factory;
			p: GraphLogical.Node;
	BEGIN
		fact := FindFactory(args);
		IF fact = NIL THEN
			fact := CreateFactory(args);
			StoreFactory(fact, args)
		END;
		p := fact.New()(GraphLogical.Node);
		RETURN p
	END CreateLogical;

	(*	Unloades modules created by on the fly compilation	*)
	PROCEDURE Clear*;
		VAR
			i: INTEGER;
			cursor: FactoryList;
			mod: Kernel.Module;
			modName, timeStamp, label: ARRAY 128 OF CHAR;
			loc, locSub: Files.Locator;
			fileInfo: Files.FileInfo;
			pos: INTEGER;
	BEGIN
		cursor := factoryList;
		WHILE cursor # NIL DO cursor.factory := NIL; cursor := cursor.next END;
		(*	collect garbage, graph will no longer exist, so no nodes or factories of dynamic type	*)
		Services.Collect;
		i := 0;
		loc := Files.dir.This("Dynamic");
		locSub := loc.This("Code");
		Strings.IntToString(GraphNodes.timeStamp, timeStamp);
		cursor := factoryList;
		WHILE cursor # NIL DO
			Strings.IntToString(i, label);
			label := label + "_" + timeStamp;
			modName := "DynamicNode" + label;
			mod := Kernel.ThisLoadedMod(SHORT(modName));
			IF mod # NIL THEN Kernel.UnloadMod(mod); END;
			INC(i);
			cursor := cursor.next
		END;
		(*	delete code files with current time stamp	*)
		fileInfo := Files.dir.FileList(locSub);
		WHILE fileInfo # NIL DO
			Strings.Find(fileInfo.name, "_" + timeStamp, 0, pos);
			IF pos #  - 1 THEN
				Files.dir.Delete(locSub, fileInfo.name)
			END;
			fileInfo := fileInfo.next
		END;
		factoryList := NIL;
		numDynamic := 0
	END Clear;

	(*	Write out array of operators.	*)
	PROCEDURE Externalize* (VAR wr: Stores.Writer);
		VAR
			cursor: FactoryList;
			i, len: INTEGER;
	BEGIN
		wr.WriteLong(GraphNodes.timeStamp);
		wr.WriteInt(numDynamic);
		cursor := factoryList;
		WHILE cursor # NIL DO
			wr.WriteInt(cursor.numConsts);
			wr.WriteInt(cursor.numLogicals);
			wr.WriteInt(cursor.numStochs);
			wr.WriteInt(cursor.numOps);
			len := cursor.numOps;
			i := 0; WHILE i < len DO wr.WriteInt(cursor.ops[i]); INC(i) END;
			cursor := cursor.next
		END
	END Externalize;

	(*	Read in array of operators	*)
	PROCEDURE Internalize* (VAR rd: Stores.Reader);
		VAR
			i, j, len, op, totalNumDynamic: INTEGER;
			cursor: FactoryList;
			args: GraphStochastic.ArgsLogical;
			timeStamp: LONGINT;
	BEGIN
		rd.ReadLong(timeStamp);
		GraphNodes.SetTimeStamp(timeStamp);
		rd.ReadInt(totalNumDynamic);
		i := 0;
		factoryList := NIL;
		numDynamic := 0;
		WHILE i < totalNumDynamic DO
			NEW(cursor);
			rd.ReadInt(cursor.numConsts);
			rd.ReadInt(cursor.numLogicals);
			rd.ReadInt(cursor.numStochs);
			rd.ReadInt(cursor.numOps);
			len := cursor.numOps;
			NEW(cursor.ops, len);
			j := 0;
			WHILE j < len DO
				rd.ReadInt(op); cursor.ops[j] := op; INC(j)
			END;
			cursor.factory := NIL;
			cursor.next := factoryList;
			factoryList := cursor;
			INC(i)
		END;
		(*	create new factories by generating and compiling CP code	*)
		cursor := factoryList;
		factoryList := NIL;
		WHILE cursor # NIL DO
			(*	set up the args data structure	*)
			args.Init;
			args.numConsts := cursor.numConsts;
			args.numLogicals := cursor.numLogicals;
			args.numStochs := cursor.numStochs;
			args.numOps := cursor.numOps;
			j := 0;
			WHILE j < cursor.numOps DO
				args.ops[j] := cursor.ops[j]; INC(j)
			END;
			cursor.factory := CreateFactory(args);
			StoreFactory(cursor.factory, args);
			cursor := cursor.next
		END
	END Internalize;

	PROCEDURE CreateTimeStamp*;
		VAR
			text: TextModels.Model;
			ok: BOOLEAN;
			rd: TextModels.Reader;
			f: TextMappers.Formatter;
			timeStamp: ARRAY 64 OF CHAR;
	BEGIN
		Strings.IntToString(GraphNodes.timeStamp, timeStamp);
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.WriteString("MODULE DynamicTime_" + timeStamp + ";");
		f.WriteString("IMPORT GraphNodes;");
		f.WriteString("BEGIN ");
		f.WriteString("GraphNodes.SetTimeStamp(" + timeStamp + ") ");
		f.WriteString("END DynamicTime_" + timeStamp + ".");
		rd := text.NewReader(NIL);
		rd.SetPos(0);
		Compile(rd, ok); ASSERT(ok, 55);
	END CreateTimeStamp;

	PROCEDURE NumberDynamic* (): INTEGER;
		VAR
			num: INTEGER;
			cursor: FactoryList;
	BEGIN
		num := 0;
		cursor := factoryList;
		WHILE cursor # NIL DO
			INC(num);
			cursor := cursor.next
		END;
		RETURN num
	END NumberDynamic;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		debug := FALSE;
		numDynamic := 0;
		factoryList := NIL
	END Init;

BEGIN
	Init
CLOSE
	Clear
END BugsCPCompiler.

