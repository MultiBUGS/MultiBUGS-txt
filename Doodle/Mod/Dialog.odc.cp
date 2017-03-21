
(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE DoodleDialog;


	

	IMPORT
		Dialog, Fonts, Views,
		DoodleMenus, DoodleNodes, 
		GraphGrammar, GraphNodes;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		menu: Dialog.List;

	PROCEDURE Mask* (IN density: Dialog.String; OUT mask: SET);
		VAR
			i, len, numPar: INTEGER;
			descriptor: GraphGrammar.External;
			fact: GraphNodes.Factory;
			signature: ARRAY 64 OF CHAR;
	BEGIN
		descriptor := GraphGrammar.FindDensity(density);
		fact := descriptor.fact;
		numPar := fact.NumParam();
		fact.Signature(signature);
		len := LEN(signature$);
		mask := {};
		i := 0;
		WHILE i < numPar DO
			INCL(mask, i);
			INC(i)
		END;
		IF (signature[numPar] = "C") OR (signature[len - 1] = "C") THEN
			INCL(mask, i);
			INCL(mask, i + 1);
		END
	END Mask;

	PROCEDURE Type* (f: Views.Frame; node: DoodleNodes.Node; l, t: INTEGER; font: Fonts.Font);
		VAR
			i, type: INTEGER;
			mask: SET;
			densityName: Dialog.String;
			defaults: ARRAY 5 OF Dialog.String;
	BEGIN
		i := 0;
		WHILE i < 5 DO
			defaults[i] := "";
			INC(i)
		END;
		type := DoodleMenus.TypeOption(f, l, t, font);
		IF (type # - 1) & (type # node.type) THEN
			IF type = DoodleNodes.stochastic THEN
				DoodleMenus.Density(0, densityName);
				Mask(densityName, mask);
				DoodleMenus.Default(0, 0, defaults[0]);
				DoodleMenus.Default(0, 1, defaults[1]);
				DoodleMenus.Default(0, 2, defaults[2]);
				defaults[3] := "";
				defaults[4] := "";
				node.Set(type, node.density, node.link, node.x, node.y, mask, defaults)
			ELSIF type = DoodleNodes.logical THEN
				node.Set(type, node.density, node.link, node.x, node.y, {0..31}, defaults)
			ELSIF type = DoodleNodes.constant THEN
				node.Set(type, node.density, node.link, node.x, node.y, {}, defaults)
			END
		END
	END Type;

	PROCEDURE Density* (f: Views.Frame; node: DoodleNodes.Node; l, t: INTEGER; font: Fonts.Font);
		VAR
			density: INTEGER;
			mask: SET;
			densityName: Dialog.String;
			defaults: ARRAY 5 OF Dialog.String;
	BEGIN
		density := DoodleMenus.DensityOption(f, l, t, font);
		IF (density # - 1) & (density # node.density) THEN
			DoodleMenus.Density(density, densityName);
			Mask(densityName, mask);
			DoodleMenus.Default(density, 0, defaults[0]);
			DoodleMenus.Default(density, 1, defaults[1]);
			DoodleMenus.Default(density, 2, defaults[2]);
			defaults[3] := "";
			defaults[4] := "";
			node.Set(node.type, density, node.link, node.x, node.y, mask, defaults)
		END
	END Density;

	PROCEDURE Link* (f: Views.Frame; node: DoodleNodes.Node; l, t: INTEGER; font: Fonts.Font);
		VAR
			i, link: INTEGER;
			defaults: ARRAY 5 OF Dialog.String;
	BEGIN
		link := DoodleMenus.LinkOption(f, l, t, font);
		IF (link # - 1) & (link # node.link) THEN
			i := 0;
			WHILE i < 5 DO defaults[i] := "";
				INC(i)
			END;
			node.Set(node.type, node.density, link, node.x, node.y, {0..31}, defaults)
		END
	END Link;

	PROCEDURE GetParents (node: DoodleNodes.Node; OUT names: ARRAY OF Dialog.String;
	OUT num: INTEGER);
		VAR
			i, len: INTEGER;
	BEGIN
		i := 0;
		num := 0;
		len := LEN(node.parents);
		WHILE i < len DO
			IF (i IN node.mask) & (node.parents[i] # NIL) THEN
				names[num] := node.parents[i].name$;
				INC(num)
			END;
			INC(i)
		END
	END GetParents;

	PROCEDURE SwapParents (node: DoodleNodes.Node; option, param: INTEGER);
		VAR
			i, j: INTEGER;
	BEGIN
		i := 0;
		j := 0;
		LOOP
			IF node.parents[i] # NIL THEN
				INC(j)
			END;
			IF j > option THEN
				EXIT
			END;
			INC(i)
		END;
		node.SwapParents(i, param)
	END SwapParents;

	PROCEDURE Parents* (f: Views.Frame; node: DoodleNodes.Node; l, t, param: INTEGER;
	font: Fonts.Font);
		VAR
			i, len, num, option: INTEGER;
			densityName: Dialog.String;
			names: ARRAY 10 OF Dialog.String;
			parent: DoodleNodes.Node;
			descriptor: GraphGrammar.External;
			fact: GraphNodes.Factory;
			signature: ARRAY 64 OF CHAR;
	BEGIN
		GetParents(node, names, num);
		parent := node.parents[param];
		DoodleMenus.Density(node.density, densityName);
		descriptor := GraphGrammar.FindDensity(densityName);
		fact := descriptor.fact;
		fact.Signature(signature);
		IF (param IN {0, 1}) & (signature[param] = "v") THEN
			IF num = 0 THEN RETURN END;
			menu.SetLen(num + 1)
		ELSIF num = 0 THEN
			len := LEN(node.defaults[param]$);
			node.SetCaret(param, len);
			RETURN
		ELSE
			menu.SetLen(num + 1);
			menu.SetItem(num, "edit value")
		END;
		i := 0;
		WHILE i < num DO
			menu.SetItem(i, names[i]);
			INC(i)
		END;
		option := DoodleMenus.Option(f, menu, l, t, font);
		IF option = num THEN
			len := LEN(node.defaults[param]$);
			node.SetCaret(param, len);
			node.SetEdge(parent)
		ELSIF option # - 1 THEN
			SwapParents(node, option, param)
		END
	END Parents;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END DoodleDialog.
