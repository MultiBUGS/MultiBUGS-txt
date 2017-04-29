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

		StoreKey: PROCEDURE (IN key, mes: ARRAY OF CHAR);

	PROCEDURE Load*;
	BEGIN
		StoreKey("MapsCmds1", "map has different number of regions to number of components of plotted quantity");
		StoreKey("MapsCmds2", "not a quantity in the statistical model");
		StoreKey("MapsCmds3", "plotted quantity has no defined components");
		StoreKey("MapsCmds4", "all components of plotted quantity are the same");
		StoreKey("MapsCmds5", "summary monitor not set for quantity");
		StoreKey("MapsCmds6", "samples monitor not set for quantity");
		
		StoreKey("MapsArcinfo1", "expected key word map");
		StoreKey("MapsArcinfo2", "expected a colon");
		StoreKey("MapsArcinfo3", "expected the number of regions (an integer)");
		StoreKey("MapsArcinfo4", "expected GeoBUGS region index (an integer)");
		StoreKey("MapsArcinfo5", "expected ArcInfo region name (string or integer)");
		StoreKey("MapsArcinfo6", "too few regions specified / duplicate region");
		StoreKey("MapsArcinfo7", "expected key word regions");
		StoreKey("MapsArcinfo8", "expected ArcInfo region name");
		StoreKey("MapsArcinfo9", "no region of this name in map");
		StoreKey("MapsArcinfo10", "expected key word END after list of polygon region pairs");
		StoreKey("MapsArcinfo11", "expected an integer polygon label");
		StoreKey("MapsArcinfo12", "expected an integer polygon label");
		StoreKey("MapsArcinfo13", "expected an integer polygon label");
		StoreKey("MapsArcinfo14", "expected polygon centroid x coordinate (a number)");
		StoreKey("MapsArcinfo15", "expected polygon centroid y coordinate (a number)");
		StoreKey("MapsArcinfo16", "expected polygon vertex x coordinate (a number)");
		StoreKey("MapsArcinfo17", "expected polygon vertex y coordinate (a number)");
		StoreKey("MapsArcinfo18", "expected key word END after set of polygon data");
		StoreKey("MapsArcinfo19", "no polygon of this number in map data");
		StoreKey("MapsArcinfo20", "expected key word END after all the polygon data");
		StoreKey("MapsArcinfo21", "expected scale factor (a number)");
		
		StoreKey("MapsEpimap1", "expected key word map");
		StoreKey("MapsEpimap2", "expected a colon");
		StoreKey("MapsEpimap3", "expected the number of regions (an integer)");
		StoreKey("MapsEpimap4", "expected GeoBUGS region index (an integer)");
		StoreKey("MapsEpimap5", "expected EpiStoreKey region name (string or integer)");
		StoreKey("MapsEpimap6", "too few regions specified / duplicate region");
		StoreKey("MapsEpimap7", "no region of this name in map");
		StoreKey("MapsEpimap8", "expected a comma");
		StoreKey("MapsEpimap9", "expected number of vertices in polygon (an integer)");
		StoreKey("MapsEpimap10", "expected x coordinate of vertex (a number)");
		StoreKey("MapsEpimap11", "expected a comma");
		StoreKey("MapsEpimap12", "expected y coordinate of vertex (a number)");
		StoreKey("MapsEpimap13", "expected key word END after all the polygon data");
		StoreKey("MapsEpimap14", "expected scale factor (a number)");
		
		StoreKey("MapsSplus1", "expected key word map");
		StoreKey("MapsSplus2", "expected a colon");
		StoreKey("MapsSplus3", "expected the number of regions (an integer)");
		StoreKey("MapsSplus4", "expected GeoBUGS region index (an integer)");
		StoreKey("MapsSplus5", "expected StoreKeySplus region name (string or integer)");
		StoreKey("MapsSplus6", "too few regions specified / duplicate region");
		StoreKey("MapsSplus7", "expected StoreKeySplus region name (string or integer)");
		StoreKey("MapsSplus8", "no region of this name in map");
		StoreKey("MapsSplus9", "expected x coordinate of vertex (a number)");
		StoreKey("MapsSplus10", "expected y coordinate of vertex (a number)");
		StoreKey("MapsSplus11", "expected StoreKeySplus region name (string or integer)");
		StoreKey("MapsSplus12", "expected NA NA NA to end polygon data");
		StoreKey("MapsSplus13", "expected NA NA NA to end polygon data");
		StoreKey("MapsSplus14", "expected Splus region name (string or integer)");
		StoreKey("Splus15", "expected scale factor (a number)");
		
		StoreKey("MapsMap1", "cannot calculate bounding box of map");
		
	END Load;
	
	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		StoreKey := BugsMsg.StoreKey
	END Init;

BEGIN
	Init
END MapsMessages.
