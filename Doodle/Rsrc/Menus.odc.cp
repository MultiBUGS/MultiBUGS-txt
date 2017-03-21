MENU "Doodle" 
	"New..."	""	"StdCmds.OpenToolDialog('Doodle/Rsrc/sizes', 'New Doodle')" ""
	SEPARATOR
	"No grid"	""	"DoodleCmds.SetGrid(0)"	"DoodleCmds.GridGuard(0)"
	"grid 1mm"	""	"DoodleCmds.SetGrid(1)"	"DoodleCmds.GridGuard(1)"
	"grid 2mm"	""	"DoodleCmds.SetGrid(2)"	"DoodleCmds.GridGuard(2)"
	"grid 5mm"	""	"DoodleCmds.SetGrid(5)"	"DoodleCmds.GridGuard(5)"	
	SEPARATOR
	"Scale Model"	""	"StdCmds.OpenToolDialog('Doodle/Rsrc/scale', 'Scale Doodle')" "DoodleCmds.FocusGuard"
	SEPARATOR
	"Remove Selection"	""	"DoodleCmds.RemoveSelection"	"DoodleCmds.FocusGuard"
END


