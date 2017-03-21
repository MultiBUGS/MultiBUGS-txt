MENU "Info"
	"Open Log"	""	"StdLog.Open"	""
	"Clear Log"	""	"StdLog.Clear"	""
	SEPARATOR
	"Uninitialized Nodes"	""	"BugsCmds.WriteUninitNodes"	"BugsCmds.NotCompiledGuardWin"
	"Updaters(by name)"	""	"BugsCmds.UpdatersByName"	"BugsCmds.NotCompiledGuardWin"
	"Updaters(by depth)"	""	"BugsCmds.UpdatersByDepth"	"BugsCmds.NotCompiledGuardWin"
	SEPARATOR
	"Model"	""	"BugsCmds.PrintModel"	"BugsCmds.ParsedGuardWin"
	"Latex"	""	"BugsCmds.PrintLatex"	"BugsCmds.ParsedGuardWin"
	"Data"	""	"BugsCmds.WriteData"	"BugsCmds.UpdateGuardWin"
	"State"	""	"BugsCmds.WriteChains"	"BugsCmds.NotCompiledGuardWin"
	SEPARATOR
	"Show Distribution"	""	"BugsCmds.ShowDistribution"	"BugsCmds.NotCompiledGuardWin"
	"Show Deviance Dist"	""	"BugsCmds.ShowDistributionDeviance"	"BugsCmds.NotCompiledGuardWin"
	"Parallel updaters"	""	"StdCmds.OpenToolDialog('Parallel/Rsrc/Dialog', 'Parallel Debug Tool')"	"BugsCmds.NotCompiledGuardWin"
	SEPARATOR
	"Node info..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/InfoDialog', 'Node Tool')"		""
	"Metrics"	""	"BugsCmds.Metrics"	"BugsCmds.NotCompiledGuardWin"
	"Modules"	""	"BugsCmds.Modules"	""
	"Memory"	""	"BugsCmds.AllocatedMemory"	""
	"Distribution info"	""	"BugsCmds.DistributeInfo"	""
END

MENU "Model" 
	"Specification..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/SpecificationDialog', 'Specification Tool')"	""
	SEPARATOR
	"PK Specification..."	""	"StdCmds.OpenToolDialog('PKBugs/Rsrc/SpecificationDialog', 'PK Specification Tool')"	""
	SEPARATOR
	"Update..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/UpdateDialog', 'Update Tool')"		"BugsCmds.UpdateGuardWin"
	SEPARATOR
	"Random number generator..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/RNDialog', 'RN state')"		"BugsCmds.NotCompiledGuardWin"	
	SEPARATOR	
	"Script"	""	"BugsCmds.Script"	""
	SEPARATOR
	"Output options..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/DisplayDialog', 'Display options')"	 ""
	"Compile options..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/CompileDialog', 'Compile options')"	 ""		
	"Updater options..."	""	"StdCmds.OpenToolDialog('Updater/Rsrc/SettingsDialog', 'Updater options')"	 ""	
	SEPARATOR	
	"Externalize..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/OutBugDialog', 'Externalize model')"	""
	"Internalize..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/InBugDialog', 'Internalize model')"	""
END

MENU "Inference"
	"Samples..."	""	"StdCmds.OpenAuxDialog('Samples/Rsrc/Dialog', 'Sample Monitor Tool')"	"BugsCmds.UpdateGuardWin"
	"Compare..."	""	"StdCmds.OpenToolDialog('Compare/Rsrc/Dialog', 'Comparison Tool')"	"BugsCmds.UpdateGuardWin"
	"Correlations..."	""	"StdCmds.OpenToolDialog('Correl/Rsrc/Dialog', 'Correlation Tool')"	"BugsCmds.UpdateGuardWin"
	SEPARATOR
	"Summary..."	""	"StdCmds.OpenToolDialog('Summary/Rsrc/Dialog', 'Summary MonitorTool')" "BugsCmds.UpdateGuardWin"
	"Rank..."	""	"StdCmds.OpenToolDialog('Ranks/Rsrc/Dialog', 'Rank Monitor Tool')"	"BugsCmds.UpdateGuardWin"
	"Model..."	""	"StdCmds.OpenToolDialog('Models/Rsrc/Dialog', 'Model Monitor Tool')"	"BugsCmds.UpdateGuardWin"
	SEPARATOR
	"Information Criterion..."	""	"StdCmds.OpenToolDialog('Deviance/Rsrc/Dialog', '
	Information Criterion Tool')"	"BugsCmds.UpdateGuardWin"
	SEPARATOR	
	"MAP"	""	"BugsCmds.MAP"	"BugsCmds.MAPGuardWin"	

END 


