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
	"Show Distribution"	""	"BugsCmds.ShowDistribution"	"BugsCmds.NotCompiledGuard"
	"Show Deviance Dist"	""	"BugsCmds.ShowDistributionDeviance"	"BugsCmds.DevianceGuard"
	SEPARATOR
	"Node info..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/InfoDialog', 'Node Tool')"		""
	"Metrics"	""	"BugsCmds.Metrics"	"BugsCmds.NotCompiledGuard"
	"Modules"	""	"BugsCmds.Modules"	""
	"Memory"	""	"BugsCmds.AllocatedMemory"	""
	"Distributed graph..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/ParallelDebug', 'Parallel Graph Tool')"		""
	"Distribution info"	""	"BugsCmds.DistributeInfo"	""
END

MENU "Model" 
	"Specification..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/SpecificationDialog', 'Specification Tool')"	""
	SEPARATOR
	"PK Specification..."	""	"StdCmds.OpenToolDialog('PKBugs/Rsrc/SpecificationDialog', 'PK Specification Tool')"	""
	SEPARATOR
	"Update..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/UpdateDialog', 'Update Tool')"		"BugsCmds.UpdateGuard"
	SEPARATOR
	"Random number generator..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/RNDialog', 'RN state')"		"BugsCmds.NotCompiledGuard"	
	SEPARATOR	
	"Script"	""	"BugsCmds.Script('')"	""
	SEPARATOR
	"Output options..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/DisplayDialog', 'Display options')"	 ""
	"Compile options..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/CompileDialog', 'Compile options')"	 ""		
	"Updater options..."	""	"StdCmds.OpenToolDialog('Updater/Rsrc/SettingsDialog', 'Updater options')"	 ""	
	SEPARATOR	
	"Externalize..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/OutBugDialog', 'Externalize model')"	""
	"Internalize..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/InBugDialog', 'Internalize model')"	""
	SEPARATOR
	"Debug"	""	"BugsCmds.Debug"	"BugsCmds.DebugGuard"
END

MENU "Inference"
	"Samples..."	""	"StdCmds.OpenToolDialog('Samples/Rsrc/Dialog', 'Sample Monitor Tool')"	"BugsCmds.UpdateGuard"
	"Compare..."	""	"StdCmds.OpenToolDialog('Compare/Rsrc/Dialog', 'Comparison Tool')"	"BugsCmds.UpdateGuard"
	"Correlations..."	""	"StdCmds.OpenToolDialog('Correl/Rsrc/Dialog', 'Correlation Tool')"	"BugsCmds.UpdateGuard"
	SEPARATOR
	"Summary..."	""	"StdCmds.OpenToolDialog('Summary/Rsrc/Dialog', 'Summary MonitorTool')" "BugsCmds.UpdateGuard"
	"Rank..."	""	"StdCmds.OpenToolDialog('Ranks/Rsrc/Dialog', 'Rank Monitor Tool')"	"BugsCmds.UpdateGuard"
	"Model..."	""	"StdCmds.OpenToolDialog('Models/Rsrc/Dialog', 'Model Monitor Tool')"	"BugsCmds.UpdateGuard"
	SEPARATOR
	"Information Criterion..."	""	"StdCmds.OpenToolDialog('Deviance/Rsrc/Dialog', ' Information Criterion Tool')"	
	"BugsCmds.DevianceGuard"
	SEPARATOR	
	"MAP"	""	"BugsCmds.MAP"	"BugsCmds.MAPGuard"	

END 


