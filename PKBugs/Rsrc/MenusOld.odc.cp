MENU "PKBugs"
	"Load item names"	""	"PKBugsCmds.ReadNames"	""
	"Load data"	""	"PKBugsCmds.ReadData"	"PKBugsCmds.DataGuard"
	"Define model..."	""	"StdCmds.OpenToolDialog('PKBugs/Rsrc/dialog0', 'Define Model')"	"PKBugsCmds.CheckGuard"
	"Priors..."	""	"StdCmds.OpenToolDialog('PKBugs/Rsrc/dialog1', 'Define Priors')"	"PKBugsCmds.PriorGuard"
	"Load priors"	""	"PKBugsCmds.LoadPriors"	"PKBugsCmds.LPGuard"
	"Load inits (pop)"	""	"PKBugsCmds.LoadPopInits"	"PKBugsCmds.PopGuard"
	"Load inits (theta)"	""	"PKBugsCmds.LoadThetaInits"	"PKBugsCmds.ThetaGuard"
	"Print model"	""	"PKBugsCmds.Print"	"PKBugsCmds.CompileGuard"
	"Compile"	""	"PKBugsCmds.Compile"	"PKBugsCmds.CompileGuard"
	SEPARATOR
	"Update..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/UpdateDialog', 'Update Tool')"	"BugsCmds.UpdateGuard"
	SEPARATOR
	"Metropolis monitor"	""	"UpdaterMetplots.OpenView('Metropolis acceptance rates')"	"UpdaterMetplots.PlotGuard"
	SEPARATOR
	"Save state"	""	"BugsState.Save"	"BugsCmds.UpdateGuard"
	SEPARATOR
	"Seed..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/SeedDialog', 'Seed Tool')"	""
END

MENU "PKHelp"
	"PKBugs User Guide"	"F2"	"StdCmds.OpenBrowser('PKBugs/Docu/Manual', 'PKBugs User Guide')"	""
	"PKBugs Examples"	""	"StdCmds.OpenBrowser('Examples/PKBugs/Index', 'PKBugs Examples')"	""
	SEPARATOR
	"PKBugs Licence"	""	"StdCmds.OpenBrowser('PKBugs/Docu/Licence', 'PKBugs Licence')"	""
	"About PKBugs"	""	"StdCmds.OpenToolDialog('PKBugs/Rsrc/About', 'About PKBugs')"	""
END

