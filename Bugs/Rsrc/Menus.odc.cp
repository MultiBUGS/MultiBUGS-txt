MENU "Info"
	"Open Log"	""	"StdLog.Open"	""
	"Clear Log"	""	"StdLog.Clear"	""
	SEPARATOR
	"Uninitialized Nodes"	""	"BugsCmds.WriteUninitNodes"	"BugsCmds.NotCompiledGuard"
	"Updaters(by name)"	""	"BugsCmds.UpdatersByName"	"BugsCmds.NotCompiledGuard"
	"Updaters(by depth)"	""	"BugsCmds.UpdatersByDepth"	"BugsCmds.NotCompiledGuard"
	SEPARATOR
	"Model"	""	"BugsCmds.PrintModel"	"BugsCmds.ParsedGuard"
	"Latex"	""	"BugsCmds.PrintLatex"	"BugsCmds.ParsedGuard"
	"Data"	""	"BugsCmds.WriteData"	"BugsCmds.UpdateGuard"
	"State"	""	"BugsCmds.WriteChains"	"BugsCmds.NotCompiledGuard"
	SEPARATOR
	"Metrics"	""	"BugsCmds.Metrics"	""
	"Node info..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/InfoDialog', 'Node Tool')"		""
	SEPARATOR
	"Show Distribution"	""	"BugsCmds.ShowDistribution"	"BugsCmds.NotCompiledGuard"
	"Show Deviance Dist"	""	"BugsCmds.ShowDistributionDeviance"	"BugsCmds.DevianceGuard"
	"Distributed graph..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/ParallelDebug', 'Parallel Graph Tool')"		""
	"Distribution info"	""	"BugsCmds.DistributeInfo"	""
	SEPARATOR	
	"Modules"	""	"BugsCmds.Modules"	""
	"Memory"	""	"BugsCmds.AllocatedMemory"	""
END

MENU "Model" 
	"Specification..."	""	"BugsCmds.OpenSpecificationDialog"	""
	SEPARATOR
	"Update..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/UpdateDialog', 'Update Tool')"		"BugsCmds.UpdateGuard"
	"Script"	""	"BugsCmds.Script('')"	""
	SEPARATOR
	"Output options..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/DisplayDialog', 'Display options')"	 ""
	"Compile options..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/CompileDialog', 'Compile options')"	 ""		
	"Updater options..."	""	"StdCmds.OpenToolDialog('Updater/Rsrc/SettingsDialog', 'Updater options')"	 ""	
	"Random number generator..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/RNDialog', 'RN state')"		"BugsCmds.NotCompiledGuard"	
	SEPARATOR	
	"Externalize..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/OutBugDialog', 'Externalize model')"	""
	"Internalize..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/InBugDialog', 'Internalize model')"	""
END

MENU "Inference"
	"Samples..."	""	"StdCmds1.OpenToolDialog('Samples/Rsrc/Dialog', 'Sample Monitor Tool')"	"BugsCmds.UpdateGuard"
	"Compare..."	""	"StdCmds.OpenToolDialog('Compare/Rsrc/Dialog', 'Comparison Tool')"	"BugsCmds.UpdateGuard"
	"Correlations..."	""	"StdCmds.OpenToolDialog('Correl/Rsrc/Dialog', 'Correlation Tool')"	"BugsCmds.UpdateGuard"
	SEPARATOR
	"Summary..."	""	"StdCmds1.OpenToolDialog('Summary/Rsrc/Dialog', 'Summary Monitor Tool')" "BugsCmds.UpdateGuard"
	"Rank..."	""	"StdCmds.OpenToolDialog('Ranks/Rsrc/Dialog', 'Rank Monitor Tool')"	"BugsCmds.UpdateGuard"
	"Model..."	""	"StdCmds.OpenToolDialog('Models/Rsrc/Dialog', 'Model Monitor Tool')"	"BugsCmds.UpdateGuard"
	"Information Criterion..."	""	"StdCmds.OpenToolDialog('Deviance/Rsrc/Dialog', ' Information Criterion Tool')"	
	"BugsCmds.DevianceGuard"
	SEPARATOR	
	"MAP"	""	"BugsCmds.MAP"	"BugsCmds.MAPGuard"	

END 


