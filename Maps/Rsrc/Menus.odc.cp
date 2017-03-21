MENU "Map"
	"Mapping Tool..."	""	"StdCmds.OpenToolDialog('Maps/Rsrc/dialog0', 'Map Tool')"	""
	"Adjacency Tool..."	""	"StdCmds.OpenToolDialog('Maps/Rsrc/dialog1', 'Adjacency Tool')"	""
SEPARATOR	
	"Import ArcInfo"	""	"MapsImporter.Import('MapsArcinfo')"	"TextCmds.FocusGuard"
	"Import Epimap"	""	"MapsImporter.Import('MapsEpimap')"	"TextCmds.FocusGuard"	
	"Import Splus"	""	"MapsImporter.Import('MapsSplus')"	"TextCmds.FocusGuard"		
SEPARATOR	
	"Export Splus"	""	"MapsCmds.Print"	""
END 

