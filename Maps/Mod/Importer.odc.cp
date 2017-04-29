(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



		 *)

MODULE MapsImporter;


	

	IMPORT
		Controllers, Dialog, Files, Meta, Models, Stores,
		TextControllers, TextModels, TextViews,
		BugsMappers, BugsMsg, 
		MapsMap;

	TYPE
		Importer* = POINTER TO ABSTRACT RECORD END;

		Factory* = POINTER TO ABSTRACT RECORD END;

	VAR
		fact: Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (I: Importer) Load- (VAR s: BugsMappers.Scanner): MapsMap.Map, NEW, ABSTRACT;

	PROCEDURE (f: Factory) New- (): Importer, NEW, ABSTRACT;

	PROCEDURE Import* (moduleName: ARRAY OF CHAR);
		CONST
			type = "map";
		VAR
			ok: BOOLEAN;
			pos, res: INTEGER;
			errorMes: ARRAY 240 OF CHAR;
			importer: Importer;
			m: Models.Model;
			s: BugsMappers.Scanner;
			text: TextModels.Model;
			map: MapsMap.Map;
			f: Files.File;
			fileName: Files.Name;
			loc: Files.Locator;
			wr: Stores.Writer;
			item0, item1: Meta.Item;
	BEGIN
		m := Controllers.FocusModel();
		IF m # NIL THEN
			IF m IS TextModels.Model THEN
				text := m(TextModels.Model);
				s.ConnectToText(text);
				s.SetPos(0);
				Meta.Lookup(moduleName, item0);
				ASSERT(item0.obj = Meta.modObj, 66);
				item0.Lookup("Install", item1);
				ASSERT(item1.obj = Meta.procObj, 67);
				item1.Call(ok);
				importer := fact.New();
				fact := NIL;
				map := importer.Load(s);
				IF map = NIL THEN
					BugsMsg.Show(BugsMsg.message);
					pos := s.Pos();
					TextViews.ShowRange(text, pos, pos, FALSE);
					TextControllers.SetCaret(text, pos);
					Dialog.Beep;
					RETURN
				END;
				map.CalculateBounds(ok);
				IF ~ok THEN
					BugsMsg.Show(BugsMsg.message);
					RETURN
				END;
				map.HiddenEdges;
				loc := Files.dir.This("");
				loc := loc.This("Maps");
				loc := loc.This("Rsrc");
				Dialog.GetExtSpec("", type, loc, fileName);
				IF (loc # NIL) & (fileName # "") THEN
					f := Files.dir.New(loc, Files.dontAsk);
					wr.ConnectTo(f);
					map.Write(wr);
					f.Register(fileName, type, Files.dontAsk, res)
				END
			END
		END
	END Import;

	PROCEDURE SetFactory* (f: Factory);
	BEGIN
		fact := f
	END SetFactory;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		fact := NIL;
		Maintainer
	END Init;

BEGIN
	Init
END MapsImporter.
