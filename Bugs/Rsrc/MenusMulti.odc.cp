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
	"State"	""	"BugsCmds.WriteChains"	"BugsCmds.CompiledGuardWin"
	SEPARATOR
	"Met accept"	""	"UpdaterMetplots.OpenView('Metropolis acceptance rate')"	"UpdaterMetplots.PlotGuardWin"
	SEPARATOR
	"Node info..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/InfoDialog', 'Node Tool')"		""
	"Modules"	""	"BugsCmds.Modules"	""
	"Memory"	""	"BugsCmds.AllocatedMemory"	""
	"&Global Variables"	""	"DevDebug.ShowGlobalVariables"	"TextCmds.SelectionGuard"	
END

MENU "Model" 
	"Specification..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/SpecificationDialogMulti', 'Specification Tool')"	""
	SEPARATOR
	"PK Specification..."	""	"StdCmds.OpenToolDialog('PKBugs/Rsrc/SpecificationDialogMulti', 'PK Specification Tool')"	""
	SEPARATOR
	"Update..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/UpdateDialogMulti', 'Update Tool')"		"BugsCmds.UpdateGuardWin"
	SEPARATOR
	"Random number generator..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/RNDialog', 'RN state')"		"BugsCmds.NotCompiledGuardWin"	
	SEPARATOR	
	"Script"	""	"BugsCmdsmulti.Script"	""
	SEPARATOR
	SEPARATOR	
	"Input/Output options..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/DisplayDialog', 'Display options')"	 ""
	"Compile options..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/CompileDialog', 'Compile options')"	 ""		
	"Updater options..."	""	"StdCmds.OpenToolDialog('Updater/Rsrc/SettingsDialog', 'Updater options')"	 ""	
	SEPARATOR	
	"Externalize..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/OutBugDialogmulti', 'Externalize model')"	""
	"Internalize..."	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/InBugDialogmulti', 'Internalize model')"	""
END

MENU "Inference"
	"Samples..."	""	"StdCmds.OpenToolDialog('Samples/Rsrc/DialogMulti', 'Sample Monitor Tool')"	"BugsCmds.UpdateGuardWin"
	"Compare..."	""	"StdCmds.OpenToolDialog('Compare/Rsrc/DialogMulti', 'Comparison Tool')"	"BugsCmds.UpdateGuardWin"
	"Correlations..."	""	"StdCmds.OpenToolDialog('Correl/Rsrc/DialogMulti', 'Correlation Tool')"	"BugsCmds.UpdateGuardWin"
	SEPARATOR
	"Summary..."	""	"StdCmds.OpenToolDialog('Summary/Rsrc/DialogMulti', 'Summary MonitorTool')" "BugsCmds.UpdateGuardWin"
	"Rank..."	""	"StdCmds.OpenToolDialog('Ranks/Rsrc/DialogMulti', 'Rank Monitor Tool')"	"BugsCmds.UpdateGuardWin"
	"Model..."	""	"StdCmds.OpenToolDialog('Models/Rsrc/DialogMulti', 'Model Monitor Tool')"	"BugsCmds.UpdateGuardWin"	
	SEPARATOR
	"DIC..."	""	"StdCmds.OpenToolDialog('Deviance/Rsrc/DialogMulti', 'DIC Tool')"	"BugsCmds.UpdateGuardWin"	
END 


