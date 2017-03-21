(* 	

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	*)

MODULE MapsMessages;

	

	IMPORT
		BugsMsg;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		Map: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		Map("MapsCmds1", "map has different number of regions to number of components of plotted quantity");
		Map("MapsCmds2", "not a quantity in the statistical model");
		Map("MapsCmds3", "plotted quantity has no defined components");
		Map("MapsCmds4", "all components of plotted quantity are the same");
		Map("MapsCmds5", "summary monitor not set for quantity");
		Map("MapsCmds6", "samples monitor not set for quantity");
		
		Map("MapsArcinfo1", "expected key word map");
		Map("MapsArcinfo2", "expected a colon");
		Map("MapsArcinfo3", "expected the number of regions (an integer)");
		Map("MapsArcinfo4", "expected GeoBUGS region index (an integer)");
		Map("MapsArcinfo5", "expected ArcInfo region name (string or integer)");
		Map("MapsArcinfo6", "too few regions specified / duplicate region");
		Map("MapsArcinfo7", "expected key word regions");
		Map("MapsArcinfo8", "expected ArcInfo region name");
		Map("MapsArcinfo9", "no region of this name in map");
		Map("MapsArcinfo10", "expected key word END after list of polygon region pairs");
		Map("MapsArcinfo11", "expected an integer polygon label");
		Map("MapsArcinfo12", "expected an integer polygon label");
		Map("MapsArcinfo13", "expected an integer polygon label");
		Map("MapsArcinfo14", "expected polygon centroid x coordinate (a number)");
		Map("MapsArcinfo15", "expected polygon centroid y coordinate (a number)");
		Map("MapsArcinfo16", "expected polygon vertex x coordinate (a number)");
		Map("MapsArcinfo17", "expected polygon vertex y coordinate (a number)");
		Map("MapsArcinfo18", "expected key word END after set of polygon data");
		Map("MapsArcinfo19", "no polygon of this number in map data");
		Map("MapsArcinfo20", "expected key word END after all the polygon data");
		Map("MapsArcinfo21", "expected scale factor (a number)");
		
		Map("MapsEpimap1", "expected key word map");
		Map("MapsEpimap2", "expected a colon");
		Map("MapsEpimap3", "expected the number of regions (an integer)");
		Map("MapsEpimap4", "expected GeoBUGS region index (an integer)");
		Map("MapsEpimap5", "expected EpiMap region name (string or integer)");
		Map("MapsEpimap6", "too few regions specified / duplicate region");
		Map("MapsEpimap7", "no region of this name in map");
		Map("MapsEpimap8", "expected a comma");
		Map("MapsEpimap9", "expected number of vertices in polygon (an integer)");
		Map("MapsEpimap10", "expected x coordinate of vertex (a number)");
		Map("MapsEpimap11", "expected a comma");
		Map("MapsEpimap12", "expected y coordinate of vertex (a number)");
		Map("MapsEpimap13", "expected key word END after all the polygon data");
		Map("MapsEpimap14", "expected scale factor (a number)");
		
		Map("MapsSplus1", "expected key word map");
		Map("MapsSplus2", "expected a colon");
		Map("MapsSplus3", "expected the number of regions (an integer)");
		Map("MapsSplus4", "expected GeoBUGS region index (an integer)");
		Map("MapsSplus5", "expected MapSplus region name (string or integer)");
		Map("MapsSplus6", "too few regions specified / duplicate region");
		Map("MapsSplus7", "expected MapSplus region name (string or integer)");
		Map("MapsSplus8", "no region of this name in map");
		Map("MapsSplus9", "expected x coordinate of vertex (a number)");
		Map("MapsSplus10", "expected y coordinate of vertex (a number)");
		Map("MapsSplus11", "expected MapSplus region name (string or integer)");
		Map("MapsSplus12", "expected NA NA NA to end polygon data");
		Map("MapsSplus13", "expected NA NA NA to end polygon data");
		Map("MapsSplus14", "expected Splus region name (string or integer)");
		Map("Splus15", "expected scale factor (a number)");
		
		Map("MapsMap1", "cannot calculate bounding box of map");
		
	END Load;
	
	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		Map := BugsMsg.Map
	END Init;

BEGIN
	Init
END MapsMessages.
