(*		GNU General Public Licence	*)

(*
	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"
*)

MODULE BugsDocu;


	

	IMPORT
		Files, Fonts, Printing, Views,
		StdHeaders,
		TextMappers, TextModels, TextRulers, TextViews;

	CONST
		arab = FALSE;
		new = TRUE;
		old = FALSE;
		roman = TRUE;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Append* (head, tail: TextModels.Model);
		VAR
			lHead, lTail: INTEGER;
	BEGIN
		lHead := head.Length();
		lTail := tail.Length();
		head.InsertCopy(lHead, tail, 0, lTail - 1)
	END Append;

	PROCEDURE PageBreak* (m: TextModels.Model);
		VAR
			ruler: TextRulers.Ruler;
			mRuler: TextModels.Model;
			f: TextMappers.Formatter;
	BEGIN
		ruler := TextRulers.dir.New(NIL);
		TextRulers.SetPageBreak(ruler);
		mRuler := TextModels.dir.New();
		f.ConnectTo(mRuler);
		f.SetPos(0);
		f.WriteLn;
		f.WriteView(ruler);
		m.Append(mRuler)
	END PageBreak;

	PROCEDURE Header* (m: TextModels.Model; left, right: ARRAY OF CHAR;
	new, roman: BOOLEAN);
		VAR
			prop: StdHeaders.Prop;
			font: Fonts.Font;
			header: Views.View;
			mHeader: TextModels.Model;
			f: TextMappers.Formatter;
	BEGIN
		NEW(prop);
		prop.alternate := TRUE;
		prop.showFoot := TRUE;
		prop.head.left := left$;
		prop.head.right := right$;
		prop.number.new := new;
		IF new THEN prop.number.first := 1 END;
		IF roman THEN
			prop.foot.left := "-- &R --";
			prop.foot.right := "-- &R --"
		ELSE
			prop.foot.left := "[&p]";
			prop.foot.right := "[&p]"
		END;
		font := Fonts.dir.This("*", 12 * Fonts.point, {}, Fonts.normal);
		header := StdHeaders.New(prop, font);
		mHeader := TextModels.dir.New();
		f.ConnectTo(mHeader);
		f.SetPos(0);
		f.WriteView(header);
		m.Append(mHeader)
	END Header;

	PROCEDURE PrintChapter* (m: TextModels.Model; loc: Files.Locator;
	chapter, title, fileName: ARRAY OF CHAR; VAR first: BOOLEAN);
		VAR
			v: Views.View;
			m0: TextModels.Model;
			tV: TextViews.View;
	BEGIN
		v := Views.OldView(loc, fileName$);
		IF (v # NIL) & (v IS TextViews.View) THEN
			Header(m, title + "&; " + chapter$, chapter + "&; " + title$, first, arab);
			IF first THEN first := FALSE END;
			tV := v(TextViews.View);
			m0 := tV.ThisModel();
			Append(m, m0)
		ELSE
			HALT(0)
		END;
	END PrintChapter;

	PROCEDURE PrintExamplesVolI*;
		VAR
			v: Views.View;
			m0, m: TextModels.Model;
			tV: TextViews.View;
			loc, tempLoc: Files.Locator;
			par: Printing.Par;
			examples: ARRAY 20 OF ARRAY 64 OF CHAR;
			i, num: INTEGER;
			first: BOOLEAN;
	BEGIN
		par := Printing.NewDefaultPar("");
		loc := Files.dir.This("Examples");
		m := TextModels.dir.New();
		v := Views.OldView(loc, "VolumeI");
		IF (v # NIL) & (v IS TextViews.View) THEN
			Header(m, "Contents", "Contents", new, roman);
			tV := v(TextViews.View);
			m0 := tV.ThisModel();
			Append(m, m0);
			PageBreak(m)
		END;
		examples[0] := "Rats";
		examples[1] := "Pumps";
		examples[2] := "Dogs";
		examples[3] := "Seeds";
		examples[4] := "Surgical";
		examples[5] := "Magnesium";
		examples[6] := "Salm";
		examples[7] := "Equiv";
		examples[8] := "Dyes";
		examples[9] := "Stacks";
		examples[10] := "Epil";
		examples[11] := "Blockers";
		examples[12] := "Oxford";
		examples[13] := "LSAT";
		examples[14] := "Bones";
		examples[15] := "Inhalers";
		examples[16] := "Mice";
		examples[17] := "Kidney";
		examples[18] := "Leuk";
		examples[19] := "Leukfr";
		i := 0;
		first := TRUE;
		num := LEN(examples);
		WHILE i < num DO
			PrintChapter(m, loc, examples[i], "Examples Volume I", examples[i], first);
			IF i < num - 1 THEN PageBreak(m) END;
			INC(i)
		END;
		tV := TextViews.dir.New(m);
		tempLoc := Files.dir.This("");
		Views.RegisterView(tV, tempLoc, "$TEMP");
		v := Views.OldView(tempLoc, "$TEMP");
		Printing.PrintView(v, par);
		v := NIL;
		Files.dir.Delete(tempLoc, "$TEMP.odc")
	END PrintExamplesVolI;

	PROCEDURE PrintExamplesVolII*;
		VAR
			v: Views.View;
			m0, m: TextModels.Model;
			tV: TextViews.View;
			loc, tempLoc: Files.Locator;
			par: Printing.Par;
			examples: ARRAY 19 OF ARRAY 64 OF CHAR;
			i, num: INTEGER;
			first: BOOLEAN;
	BEGIN
		par := Printing.NewDefaultPar("");
		loc := Files.dir.This("Examples");
		m := TextModels.dir.New();
		v := Views.OldView(loc, "VolumeII");
		IF (v # NIL) & (v IS TextViews.View) THEN
			Header(m, "Contents", "Contents", new, roman);
			tV := v(TextViews.View);
			m0 := tV.ThisModel();
			Append(m, m0);
			PageBreak(m)
		END;
		examples[0] := "Dugongs";
		examples[1] := "Otrees";
		examples[2] := "OtreesMVN";
		examples[3] := "Biopsies";
		examples[4] := "Eyes";
		examples[5] := "Hearts";
		examples[6] := "Air";
		examples[7] := "Cervix";
		examples[8] := "Jaws";
		examples[9] := "Birats";
		examples[10] := "Schools";
		examples[11] := "Ice";
		examples[12] := "Beetles";
		examples[13] := "Alligators";
		examples[14] := "Endo";
		examples[15] := "Stagnant";
		examples[16] := "Asia";
		examples[17] := "Pigs";
		examples[18] := "t-df";
		i := 0;
		first := TRUE;
		num := LEN(examples);
		WHILE i < num DO
			PrintChapter(m, loc, examples[i], "Examples Volume II", examples[i], first);
			IF i < num - 1 THEN PageBreak(m) END;
			INC(i)
		END;
		tV := TextViews.dir.New(m);
		tempLoc := Files.dir.This("");
		Views.RegisterView(tV, tempLoc, "$TEMP");
		v := Views.OldView(tempLoc, "$TEMP");
		Printing.PrintView(v, par);
		v := NIL;
		Files.dir.Delete(tempLoc, "$TEMP.odc")
	END PrintExamplesVolII;

	PROCEDURE PrintExamplesVolIII*;
		VAR
			v: Views.View;
			m0, m: TextModels.Model;
			tV: TextViews.View;
			loc, tempLoc: Files.Locator;
			par: Printing.Par;
			examples: ARRAY 13 OF ARRAY 64 OF CHAR;
			i, num: INTEGER;
			first: BOOLEAN;
	BEGIN
		first := TRUE;
		par := Printing.NewDefaultPar("");
		loc := Files.dir.This("Examples");
		m := TextModels.dir.New();
		v := Views.OldView(loc, "VolumeIII");
		IF (v # NIL) & (v IS TextViews.View) THEN
			Header(m, "Contents", "Contents", new, roman);
			tV := v(TextViews.View);
			m0 := tV.ThisModel();
			Append(m, m0);
			PageBreak(m)
		END;
		examples[0] := "Camel";
		examples[1] := "Eyetracking";
		examples[2] := "Fire";
		examples[3] := "Funshapes";
		examples[4] := "Hepatitis";
		examples[5] := "Hips1";
		examples[6] := "Hips2";
		examples[7] := "Hips3";
		examples[8] := "Hips4";
		examples[9] := "Jama";
		examples[10] := "Pigs";
		examples[11] := "Pines";
		examples[12] := "StVeit";
		i := 0;
		num := LEN(examples);
		WHILE i < num DO
			PrintChapter(m, loc, examples[i], "Examples Volume III", examples[i], first);
			IF i < num - 1 THEN PageBreak(m) END;
			INC(i)
		END;
		tV := TextViews.dir.New(m);
		tempLoc := Files.dir.This("");
		Views.RegisterView(tV, tempLoc, "$TEMP");
		v := Views.OldView(tempLoc, "$TEMP");
		Printing.PrintView(v, par);
		v := NIL;
		Files.dir.Delete(tempLoc, "$TEMP.odc")
	END PrintExamplesVolIII;

	PROCEDURE PrintExamplesVolIV*;
		VAR
			v: Views.View;
			m0, m: TextModels.Model;
			tV: TextViews.View;
			loc, tempLoc: Files.Locator;
			par: Printing.Par;
			examples: ARRAY 12 OF ARRAY 64 OF CHAR;
			i, num: INTEGER;
			first: BOOLEAN;
	BEGIN
		first := TRUE;
		par := Printing.NewDefaultPar("");
		loc := Files.dir.This("Examples");
		m := TextModels.dir.New();
		v := Views.OldView(loc, "VolumeIII");
		IF (v # NIL) & (v IS TextViews.View) THEN
			Header(m, "Contents", "Contents", new, roman);
			tV := v(TextViews.View);
			m0 := tV.ThisModel();
			Append(m, m0);
			PageBreak(m)
		END;
		examples[0] := "SeedsDataCloning";
		examples[1] := "Coins";
		examples[2] := "SmartPhones";
		examples[3] := "Abbey";
		examples[4] := "BeetlesProbit";
		examples[5] := "Preeclampsia";
		examples[6] := "Lotka-Volterra";
		examples[7] := "Fivecompartment";
		examples[8] := "ChangePoints";
		examples[9] := "Pollution";
		examples[10] := "Methadone";
		examples[11] := "Functionals";
		i := 0;
		num := LEN(examples);
		WHILE i < num DO
			PrintChapter(m, loc, examples[i], "Examples Volume IV", examples[i], first);
			IF i < num - 1 THEN PageBreak(m) END;
			INC(i)
		END;
		tV := TextViews.dir.New(m);
		tempLoc := Files.dir.This("");
		Views.RegisterView(tV, tempLoc, "$TEMP");
		v := Views.OldView(tempLoc, "$TEMP");
		Printing.PrintView(v, par);
		v := NIL;
		Files.dir.Delete(tempLoc, "$TEMP.odc")
	END PrintExamplesVolIV;

	PROCEDURE PrintExamplesGeoBUGS*;
		VAR
			v: Views.View;
			m0, m: TextModels.Model;
			tV: TextViews.View;
			loc, tempLoc: Files.Locator;
			par: Printing.Par;
			examples: ARRAY 9 OF ARRAY 64 OF CHAR;
			i, num: INTEGER;
			first: BOOLEAN;
	BEGIN
		par := Printing.NewDefaultPar("");
		loc := Files.dir.This("GeoBUGS");
		m := TextModels.dir.New();
		v := Views.OldView(loc, "Examples");
		IF (v # NIL) & (v IS TextViews.View) THEN
			Header(m, "Contents", "Contents", new, roman);
			tV := v(TextViews.View);
			m0 := tV.ThisModel();
			Append(m, m0);
			PageBreak(m)
		END;
		examples[0] := "Scotland";
		examples[1] := "LHA";
		examples[2] := "Scotland1";
		examples[3] := "Elevation";
		examples[4] := "Forest";
		examples[5] := "Huddersfield";
		examples[6] := "MVCAR";
		examples[7] := "Shared";
		examples[8] := "Pollution";
		i := 0;
		first := TRUE;
		num := LEN(examples);
		WHILE i < num DO
			PrintChapter(m, loc, examples[i], "GeoBUGS Examples", examples[i], first);
			IF i < num - 1 THEN PageBreak(m) END;
			INC(i)
		END;
		tV := TextViews.dir.New(m);
		tempLoc := Files.dir.This("");
		Views.RegisterView(tV, tempLoc, "$TEMP");
		v := Views.OldView(tempLoc, "$TEMP");
		Printing.PrintView(v, par);
		v := NIL;
		Files.dir.Delete(tempLoc, "$TEMP.odc")
	END PrintExamplesGeoBUGS;

	PROCEDURE PrintExamplesEcology*;
		VAR
			v: Views.View;
			m0, m: TextModels.Model;
			tV: TextViews.View;
			loc, tempLoc: Files.Locator;
			par: Printing.Par;
			examples: ARRAY 6 OF ARRAY 64 OF CHAR;
			i, num: INTEGER;
			first: BOOLEAN;
	BEGIN
		par := Printing.NewDefaultPar("");
		loc := Files.dir.This("Examples");
		m := TextModels.dir.New();
		v := Views.OldView(loc, "VolumeEco");
		IF (v # NIL) & (v IS TextViews.View) THEN
			Header(m, "Contents", "Contents", new, roman);
			tV := v(TextViews.View);
			m0 := tV.ThisModel();
			Append(m, m0);
			PageBreak(m)
		END;
		examples[0] := "Gentians";
		examples[1] := "Sparrowhawk";
		examples[2] := "Birds";
		examples[3] := "Lizards";
		examples[4] := "Voles";
		examples[5] := "Impala";
		i := 0;
		first := TRUE;
		num := LEN(examples);
		WHILE i < num DO
			PrintChapter(m, loc, examples[i], "Ecology Examples", examples[i], first);
			IF i < num - 1 THEN PageBreak(m) END;
			INC(i)
		END;
		tV := TextViews.dir.New(m);
		tempLoc := Files.dir.This("");
		Views.RegisterView(tV, tempLoc, "$TEMP");
		v := Views.OldView(tempLoc, "$TEMP");
		Printing.PrintView(v, par);
		v := NIL;
		Files.dir.Delete(tempLoc, "$TEMP.odc")
	END PrintExamplesEcology;

	PROCEDURE PrintUserManual*;
		VAR
			v: Views.View;
			m0, m: TextModels.Model;
			tV: TextViews.View;
			loc, tempLoc: Files.Locator;
			par: Printing.Par;
			first: BOOLEAN;
	BEGIN
		par := Printing.NewDefaultPar("");
		loc := Files.dir.This("Manuals");
		m := TextModels.dir.New();
		v := Views.OldView(loc, "Manual");
		IF (v # NIL) & (v IS TextViews.View) THEN
			Header(m, "", "", new, roman);
			tV := v(TextViews.View);
			m0 := tV.ThisModel();
			Append(m, m0);
			PageBreak(m)
		END;
		v := Views.OldView(loc, "Contents");
		IF (v # NIL) & (v IS TextViews.View) THEN
			Header(m, "Contents", "Contents", old, roman);
			tV := v(TextViews.View);
			m0 := tV.ThisModel();
			Append(m, m0);
			PageBreak(m)
		END;
		first := TRUE;
		PrintChapter(m, loc, "Introduction", "User Manual", "Introduction", first);
		PageBreak(m);
		PrintChapter(m, loc, "Model Specification", "User Manual", "ModelSpecification", first);
		PageBreak(m);
		PrintChapter(m, loc, "Tutorial", "User Manual", "Tutorial", first);
		PageBreak(m);
		PrintChapter(m, loc, "Advanced Use of the BUGS language", "User Manual", "Tricks", first);
		PageBreak(m);
		PrintChapter(m, loc, "Troubleshooting", "User Manual", "TipsTroubleshooting", first);
		PageBreak(m);
		PrintChapter(m, loc, "DoodleBUGS", "User Manual", "DoodleBUGS", first);
		PageBreak(m);
		PrintChapter(m, loc, "Scripting and ClassicBUGS", "User Manual", "Scripts", first);
		PageBreak(m);
		PrintChapter(m, loc, "Compound Documents", "User Manual", "CompoundDocuments", first);
		PageBreak(m);
		PrintChapter(m, loc, "File Menu", "User Manual", "FileMenu", first);
		PageBreak(m);
		PrintChapter(m, loc, "Edit Menu", "User Manual", "EditMenu", first);
		PageBreak(m);
		PrintChapter(m, loc, "Attributes Menu", "User Manual", "AttributesMenu", first);
		PageBreak(m);
		PrintChapter(m, loc, "Tools Menu", "User Manual", "ToolsMenu", first);
		PageBreak(m);
		PrintChapter(m, loc, "Text Menu", "User Manual", "TextMenu", first);
		PageBreak(m);
		PrintChapter(m, loc, "Info Menu", "User Manual", "InfoMenu", first);
		PageBreak(m);
		PrintChapter(m, loc, "Model Menu", "User Manual", "ModelMenu", first);
		PageBreak(m);
		PrintChapter(m, loc, "Inference Menu", "User Manual", "InferenceMenu", first);
		PageBreak(m);
		PrintChapter(m, loc, "Examples Menu", "User Manual", "ExamplesMenu", first);
		PageBreak(m);
		PrintChapter(m, loc, "Manuals Menu", "User Manual", "ManualsMenu", first);
		PageBreak(m);
		PrintChapter(m, loc, "HelpM enu", "User Manual", "HelpMenu", first);
		PageBreak(m);
		PrintChapter(m, loc, "References", "User Manual", "References", first);
		PageBreak(m);
		PrintChapter(m, loc, "WinBUGS Graphics", "User Manual", "WinBUGSGraphics", first);
		tV := TextViews.dir.New(m);
		tempLoc := Files.dir.This("");
		Views.RegisterView(tV, tempLoc, "$TEMP");
		v := Views.OldView(tempLoc, "$TEMP");
		Printing.PrintView(v, par);
		v := NIL;
		Files.dir.Delete(tempLoc, "$TEMP.odc")
	END PrintUserManual;

	PROCEDURE PrintGeoBUGSManual*;
		VAR
			v: Views.View;
			m0, m: TextModels.Model;
			tV: TextViews.View;
			loc, tempLoc: Files.Locator;
			par: Printing.Par;
	BEGIN
		par := Printing.NewDefaultPar("");
		loc := Files.dir.This("GeoBUGS");
		m := TextModels.dir.New();
		v := Views.OldView(loc, "Manual");
		IF (v # NIL) & (v IS TextViews.View) THEN
			Header(m, "GeoBUGS Manual",
			"GeoBUGS Manual", old, arab);
			tV := v(TextViews.View);
			m0 := tV.ThisModel();
			Append(m, m0)
		END;
		tV := TextViews.dir.New(m);
		tempLoc := Files.dir.This("");
		Views.RegisterView(tV, tempLoc, "$TEMP");
		v := Views.OldView(tempLoc, "$TEMP");
		Printing.PrintView(v, par);
		v := NIL;
		Files.dir.Delete(tempLoc, "$TEMP.odc")
	END PrintGeoBUGSManual;

	PROCEDURE PrintDeveloperManual*;
		VAR
			v: Views.View;
			m0, m: TextModels.Model;
			tV: TextViews.View;
			loc, tempLoc: Files.Locator;
			par: Printing.Par;
			first: BOOLEAN;
	BEGIN
		par := Printing.NewDefaultPar("");
		loc := Files.dir.This("Developer");
		m := TextModels.dir.New();
		v := Views.OldView(loc, "Manual");
		IF (v # NIL) & (v IS TextViews.View) THEN
			Header(m, "", "", new, roman);
			tV := v(TextViews.View);
			m0 := tV.ThisModel();
			Append(m, m0);
			PageBreak(m)
		END;
		first := TRUE;
		PrintChapter(m, loc, "Setting up the development tools", "Developer Manual", "Tools", first);
		PageBreak(m);
		PrintChapter(m, loc, "Compiling the source code", "Developer Manual", "Compiling", first);
		PageBreak(m);
		PrintChapter(m, loc, "Compiling the source code", "Developer Manual", "Make", first);
		PageBreak(m);
		PrintChapter(m, loc, "Executable versions and linking", "Developer Manual", "Linking", first);
		PageBreak(m);
		PrintChapter(m, loc, "Initializing the software", "Developer Manual", "Initializing", first);
		PageBreak(m);
		PrintChapter(m, loc, "Distributing OpenBUGS", "Developer Manual", "Distributing", first);
		PageBreak(m);
		PrintChapter(m, loc, "Writing OpenBUGS extensions", "Developer Manual", "WritingBUGSExtensions", first);
		PageBreak(m);
		PrintChapter(m, loc, "Random number generators", "Developer Manual", "Randnum", first);
		PageBreak(m);
		PrintChapter(m, loc, "Sampling algorithms", "Developer Manual", "SamplingAlgorithms", first);
		PageBreak(m);
		PrintChapter(m, loc, "Blue diamonds", "Developer Manual", "BlueDiamonds", first);
		PageBreak(m);
		PrintChapter(m, loc, "The OpenBUGS library", "Developer Manual", "LibOpenBUGS", first);
		PageBreak(m);
		PrintChapter(m, loc, "Header for BRugs library", "Developer Manual", "BRugsH", first);
		PageBreak(m);
		PrintChapter(m, loc, "C code for ClassicBUGS", "Developer Manual", "CBugs", first);
		PageBreak(m);
		PrintChapter(m, loc, "Test script", "Developer Manual", "Test", first);
		PageBreak(m);
		PrintChapter(m, loc, "Documenting the OpenBUGS API", "Developer Manual", "Documenting", first);
		tV := TextViews.dir.New(m);
		tempLoc := Files.dir.This("");
		Views.RegisterView(tV, tempLoc, "$TEMP");
		v := Views.OldView(tempLoc, "$TEMP");
		Printing.PrintView(v, par);
		v := NIL;
		Files.dir.Delete(tempLoc, "$TEMP.odc")
	END PrintDeveloperManual;

	PROCEDURE Maintainer;
	BEGIN
		version := 310;
		maintainer := "A. Thomas"
	END Maintainer;

BEGIN
	Maintainer
END BugsDocu.

BugsDocu.PrintUserManual

BugsDocu.PrintGeoBUGSManual

BugsDocu.PrintDeveloperManual

BugsDocu.PrintExamplesGeoBUGS

BugsDocu.PrintExamplesVolI

BugsDocu.PrintExamplesVolII

BugsDocu.PrintExamplesVolIII

BugsDocu.PrintExamplesVolIV

BugsDocu.PrintExamplesEcology

