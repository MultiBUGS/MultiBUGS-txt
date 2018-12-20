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

		RegisterKey: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		RegisterKey("MapsCmds1", "map has different number of regions to number of components of plotted quantity");
		RegisterKey("MapsCmds2", "not a quantity in the statistical model");
		RegisterKey("MapsCmds3", "plotted quantity has no defined components");
		RegisterKey("MapsCmds4", "all components of plotted quantity are the same");
		RegisterKey("MapsCmds5", "summary monitor not set for quantity");
		RegisterKey("MapsCmds6", "samples monitor not set for quantity");
		
		RegisterKey("MapsArcinfo1", "expected key word map");
		RegisterKey("MapsArcinfo2", "expected a colon");
		RegisterKey("MapsArcinfo3", "expected the number of regions (an integer)");
		RegisterKey("MapsArcinfo4", "expected GeoBUGS region index (an integer)");
		RegisterKey("MapsArcinfo5", "expected ArcInfo region name (string or integer)");
		RegisterKey("MapsArcinfo6", "too few regions specified / duplicate region");
		RegisterKey("MapsArcinfo7", "expected key word regions");
		RegisterKey("MapsArcinfo8", "expected ArcInfo region name");
		RegisterKey("MapsArcinfo9", "no region of this name in map");
		RegisterKey("MapsArcinfo10", "expected key word END after list of polygon region pairs");
		RegisterKey("MapsArcinfo11", "expected an integer polygon label");
		RegisterKey("MapsArcinfo12", "expected an integer polygon label");
		RegisterKey("MapsArcinfo13", "expected an integer polygon label");
		RegisterKey("MapsArcinfo14", "expected polygon centroid x coordinate (a number)");
		RegisterKey("MapsArcinfo15", "expected polygon centroid y coordinate (a number)");
		RegisterKey("MapsArcinfo16", "expected polygon vertex x coordinate (a number)");
		RegisterKey("MapsArcinfo17", "expected polygon vertex y coordinate (a number)");
		RegisterKey("MapsArcinfo18", "expected key word END after set of polygon data");
		RegisterKey("MapsArcinfo19", "no polygon of this number in map data");
		RegisterKey("MapsArcinfo20", "expected key word END after all the polygon data");
		RegisterKey("MapsArcinfo21", "expected scale factor (a number)");
		
		RegisterKey("MapsEpimap1", "expected key word map");
		RegisterKey("MapsEpimap2", "expected a colon");
		RegisterKey("MapsEpimap3", "expected the number of regions (an integer)");
		RegisterKey("MapsEpimap4", "expected GeoBUGS region index (an integer)");
		RegisterKey("MapsEpimap5", "expected EpiRegisterKey region name (string or integer)");
		RegisterKey("MapsEpimap6", "too few regions specified / duplicate region");
		RegisterKey("MapsEpimap7", "no region of this name in map");
		RegisterKey("MapsEpimap8", "expected a comma");
		RegisterKey("MapsEpimap9", "expected number of vertices in polygon (an integer)");
		RegisterKey("MapsEpimap10", "expected x coordinate of vertex (a number)");
		RegisterKey("MapsEpimap11", "expected a comma");
		RegisterKey("MapsEpimap12", "expected y coordinate of vertex (a number)");
		RegisterKey("MapsEpimap13", "expected key word END after all the polygon data");
		RegisterKey("MapsEpimap14", "expected scale factor (a number)");
		
		RegisterKey("MapsSplus1", "expected key word map");
		RegisterKey("MapsSplus2", "expected a colon");
		RegisterKey("MapsSplus3", "expected the number of regions (an integer)");
		RegisterKey("MapsSplus4", "expected GeoBUGS region index (an integer)");
		RegisterKey("MapsSplus5", "expected RegisterKeySplus region name (string or integer)");
		RegisterKey("MapsSplus6", "too few regions specified / duplicate region");
		RegisterKey("MapsSplus7", "expected RegisterKeySplus region name (string or integer)");
		RegisterKey("MapsSplus8", "no region of this name in map");
		RegisterKey("MapsSplus9", "expected x coordinate of vertex (a number)");
		RegisterKey("MapsSplus10", "expected y coordinate of vertex (a number)");
		RegisterKey("MapsSplus11", "expected RegisterKeySplus region name (string or integer)");
		RegisterKey("MapsSplus12", "expected NA NA NA to end polygon data");
		RegisterKey("MapsSplus13", "expected NA NA NA to end polygon data");
		RegisterKey("MapsSplus14", "expected Splus region name (string or integer)");
		RegisterKey("Splus15", "expected scale factor (a number)");
		RegisterKey("Splus16", "too many points in map polygon");
		
		RegisterKey("MapsMap1", "cannot calculate bounding box of map");
		
	END Load;
	
	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		RegisterKey := BugsMsg.RegisterKey
	END Init;

BEGIN
	Init
END MapsMessages.
