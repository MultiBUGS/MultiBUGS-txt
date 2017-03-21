(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	   *)

MODULE DoodleMenus;


	

	IMPORT
		Dialog, Fonts, Ports, Views, 
		GraphGrammar, GraphNodes;

	VAR
		densities, param0, param1, param2, default0, default1, default2, links, types: Dialog.List;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE GetSize (IN menu: Dialog.List; font: Fonts.Font; OUT w, h: INTEGER);
		VAR
			asc, dsc, i, len, maxW, width: INTEGER;
			string: Dialog.String;
	BEGIN
		font.GetBounds(asc, dsc, width);
		i := 0;
		maxW := 0;
		len := menu.len;
		WHILE i < len DO
			menu.GetItem(i, string);
			w := font.StringWidth(string);
			IF w > maxW THEN
				maxW := w
			END;
			INC(i)
		END;
		w := maxW + 2 * width;
		IF w < 5 * width THEN
			w := 5 * width
		END;
		h := len * (asc + dsc)
	END GetSize;

	PROCEDURE DrawMenu (f: Views.Frame; IN menu: Dialog.List; l, t, w, h, opt: INTEGER;
	font: Fonts.Font);
		VAR
			asc, b, delta, dsc, i, len, r, width: INTEGER;
			string: Dialog.String;
	BEGIN
		font.GetBounds(asc, dsc, width);
		delta := asc + dsc;
		r := l + w; b := t + h;
		f.DrawRect(l, t, r, b, Ports.fill, Ports.background);
		f.DrawRect(l, t, r, b, 0, Ports.black);
		i := 0;
		len := menu.len;
		WHILE i < len DO
			menu.GetItem(i, string);
			IF i = opt THEN
				f.DrawRect(l, t + i * delta, r, t + (i + 1) * delta, Ports.fill, Ports.black);
				f.DrawString(l + width, t + asc, Ports.background, string, font)
			ELSE
				f.DrawString(l + width, t + i * delta + asc, Ports.black, string, font)
			END;
			INC(i)
		END
	END DrawMenu;

	PROCEDURE Option* (f: Views.Frame; IN menu: Dialog.List; l, t: INTEGER;
	font: Fonts.Font): INTEGER;
		VAR
			isDown: BOOLEAN;
			asc, b, delta, dsc, h, newOp, oldOp, option, r, x, y, w, width: INTEGER;
			modifiers: SET;
			string: Dialog.String;
	BEGIN
		font.GetBounds(asc, dsc, width);
		delta := asc + dsc;
		GetSize(menu, font, w, h);
		oldOp := 0; newOp := 0;
		DrawMenu(f, menu, l, t, w, h, oldOp, font);
		f.Input(x, y, modifiers, isDown);
		r := l + w; b := t + h;
		WHILE isDown & (l < x) & (r > x) & (t < y) & (b > y) DO
			oldOp := (y - t) DIV delta;
			IF oldOp # newOp THEN
				menu.GetItem(newOp, string);
				f.DrawRect(l, t + newOp * delta, r, t + (newOp + 1) * delta, Ports.fill, Ports.background);
				f.DrawString(l + width, t + newOp * delta + asc, Ports.black, string, font);
				f.DrawRect(l, t + oldOp * delta, r, t + (oldOp + 1) * delta, Ports.fill, Ports.black);
				menu.GetItem(oldOp, string);
				f.DrawString(l + width, t + oldOp * delta + asc, Ports.background, string, font);
				f.DrawRect(l, t, r, b, 0, Ports.black);
				newOp := oldOp
			END;
			f.Input(x, y, modifiers, isDown)
		END;
		IF (l > x) OR (r < x) OR (t > y) OR (b < y) THEN
			option := - 1
		ELSE
			option := oldOp
		END;
		RETURN option
	END Option;

	PROCEDURE Density* (density: INTEGER; OUT densityString: Dialog.String);
	BEGIN
		densities.GetItem(density, densityString)
	END Density;

	PROCEDURE DensityOption* (f: Views.Frame; l, t: INTEGER; font: Fonts.Font): INTEGER;
	BEGIN
		RETURN Option(f, densities, l, t, font)
	END DensityOption;

	PROCEDURE NumParam* (density: INTEGER): INTEGER;
		VAR
			densityString: Dialog.String;
			descriptor: GraphGrammar.External;
	BEGIN
		Density(density, densityString);
		descriptor := GraphGrammar.FindDensity(densityString);
		RETURN descriptor.fact.NumParam()
	END NumParam;

	PROCEDURE TotalNumParam* (density: INTEGER): INTEGER;
		VAR
			len, numPar: INTEGER;
			densityString: Dialog.String;
			descriptor: GraphGrammar.External;
			signature: ARRAY 64 OF CHAR;
			fact: GraphNodes.Factory;
	BEGIN
		Density(density, densityString);
		descriptor := GraphGrammar.FindDensity(densityString);
		fact := descriptor.fact;
		fact.Signature(signature);
		numPar := fact.NumParam();
		len := LEN(signature$);
		IF (signature[numPar] = "C") OR (signature[len - 1] = "C") THEN
			INC(numPar, 2)
		END;
		RETURN numPar
	END TotalNumParam;

	PROCEDURE Param* (density, slot: INTEGER; OUT paramString: Dialog.String);
		VAR
			numPar: INTEGER;
	BEGIN
		numPar := NumParam(density);
		IF slot < numPar THEN
			CASE slot OF
			|0: param0.GetItem(density, paramString)
			|1: param1.GetItem(density, paramString)
			|2: param2.GetItem(density, paramString)
			END
		ELSE
			IF slot = numPar THEN
				paramString := "lower bound"
			ELSE
				paramString := "upper bound"
			END
		END
	END Param;

	PROCEDURE Default* (density, slot: INTEGER; OUT defaultString: Dialog.String);
		VAR
			numPar: INTEGER;
	BEGIN
		numPar := NumParam(density);
		IF slot < numPar THEN
			CASE slot OF
			|0: default0.GetItem(density, defaultString)
			|1: default1.GetItem(density, defaultString)
			|2: default2.GetItem(density, defaultString)
			END;
			IF defaultString = "NA" THEN
				defaultString := ""
			END
		ELSE
			defaultString := ""
		END
	END Default;

	PROCEDURE Link* (link: INTEGER; OUT linkName: Dialog.String);
	BEGIN
		links.GetItem(link, linkName)
	END Link;

	PROCEDURE LinkOption* (f: Views.Frame; l, t: INTEGER; font: Fonts.Font): INTEGER;
	BEGIN
		RETURN Option(f, links, l, t, font)
	END LinkOption;

	PROCEDURE Type* (type: INTEGER; OUT typeName: Dialog.String);
	BEGIN
		types.GetItem(type, typeName)
	END Type;

	PROCEDURE TypeOption* (f: Views.Frame; l, t: INTEGER; font: Fonts.Font): INTEGER;
	BEGIN
		RETURN Option(f, types, l, t, font)
	END TypeOption;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		densities.SetResources("#Doodle:densities");
		param0.SetResources("#Doodle:param0");
		param1.SetResources("#Doodle:param1");
		param2.SetResources("#Doodle:param2");
		default0.SetResources("#Doodle:default0");
		default1.SetResources("#Doodle:default1");
		default2.SetResources("#Doodle:default2");
		links.SetResources("#Doodle:links");
		types.SetResources("#Doodle:types")
	END Init;

BEGIN
	Init
END DoodleMenus.
