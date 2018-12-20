MENU "#Dev:&Info"
	"#Dev:&Open Log"	""	"StdLog.Open"	""
	"#Dev:&Clear Log"	""	"StdLog.Clear"	""
	SEPARATOR
	"#Dev:&Loaded Modules"	""	"DevDebug.ShowLoadedModules"	""
	"#Dev:&Global Variables"	""	"DevDebug.ShowGlobalVariables"	"TextCmds.SelectionGuard"
	"#Dev:&View State"	""	"DevDebug.ShowViewState"	"StdCmds.SingletonGuard"
	"#Dev:&About Alien"	""	"DevAlienTool.Analyze"	"StdCmds.SingletonGuard"
	"#Dev:&Heap Spy..."	""	"StdCmds.OpenToolDialog('Dev/Rsrc/HeapSpy', '#Dev:Heap Spy')"	""
	"#Dev:&Message Spy..."	""	"DevMsgSpy.OpenDialog('Dev/Rsrc/MsgSpy', '#Dev:Message Spy')"	""
	"#Dev:&Control List"	""	"DevCmds.ShowControlList"	"StdCmds.ContainerGuard"
	SEPARATOR
	"#Dev:&Source"	""	"DevReferences.ShowSource"	"TextCmds.SelectionGuard"
	"#Dev:&Client Interface"	"D"	"DevBrowser.ShowInterface('@c')"	"TextCmds.SelectionGuard"
	"#Dev:&Extension Interface"	"*D"	"DevBrowser.ShowInterface('@e')"	"TextCmds.SelectionGuard"
	"#Dev:&Interface..."	""	"StdCmds.OpenToolDialog('Dev/Rsrc/Browser', '#Dev:Browser')"	""
	"#Dev:&Documentation"	""	"DevReferences.ShowDocu"	"TextCmds.SelectionGuard"
	"#Dev:&Dependencies"	""	"DevDependencies.Deposit;StdCmds.Open"	"TextCmds.SelectionGuard"
	"#Dev:&Create Tool"	""	"DevDependencies.CreateTool"	"TextCmds.SelectionGuard"
	"#Dev:&Repository"	""	"DevRBrowser.ShowRepository"	""
	SEPARATOR
	"#Dev:&Search In Sources"	""	"TextCmds.InitFindDialog; DevSearch.SearchInSources"	"TextCmds.SelectionGuard"
	"#Dev:&Search In Docu (Case Sensitive)"	""	"TextCmds.InitFindDialog; DevSearch.SearchInDocu('s')"	"TextCmds.SelectionGuard"
	"#Dev:&Search In Docu (Case Insensitive)"	""	"TextCmds.InitFindDialog; DevSearch.SearchInDocu('i')"	"TextCmds.SelectionGuard"
	"#Dev:&Compare Texts"	"F9"	"DevSearch.Compare"	"TextCmds.FocusGuard"
	"#Dev:&Check Links..."	""	"StdCmds.OpenToolDialog('Dev/Rsrc/LinkChk', '#Dev:Check Links')"	""
	"#Dev:&Analyzer Options..."	""	"StdCmds.OpenToolDialog('Dev/Rsrc/Analyzer', '#Dev:Analyzer')"	""
	"#Dev:&Analyze Module"	""	"DevAnalyzer.Analyze"	"TextCmds.FocusGuard"
	SEPARATOR
	"#Dev:&Menus"	""	"StdMenuTool.ListAllMenus"	""
	"#Dev:&Update Menus"	""	"StdMenuTool.UpdateAllMenus"	""
	SEPARATOR
	"Make file"	""	"StdCmds.OpenDoc('Developer/Make')"	""
END

MENU "#Dev:&Dev"
	"#Dev:&Edit Mode"	""	"StdCmds.SetEditMode"	"StdCmds.SetEditModeGuard"
	"#Dev:&Layout Mode"	""	"StdCmds.SetLayoutMode"	"StdCmds.SetLayoutModeGuard"
	"#Dev:&Browser Mode"	""	"StdCmds.SetBrowserMode"	"StdCmds.SetBrowserModeGuard"
	"#Dev:&Mask Mode"	""	"StdCmds.SetMaskMode"	"StdCmds.SetMaskModeGuard"
	SEPARATOR
	"#Dev:&Open Module List"	"0"	"DevCmds.OpenModuleList"	"TextCmds.SelectionGuard"
	"#Dev:&Open File List"	""	"DevCmds.OpenFileList"	"TextCmds.SelectionGuard"
	SEPARATOR
	"#Dev:&Compile"	"K"	"DevCompiler.Compile"	"TextCmds.FocusGuard"
	"#Dev:&Compile And Unload"	""	"DevCompiler.CompileAndUnload"	"TextCmds.FocusGuard"
	"#Dev:&Compile Selection"	""	"DevCompiler.CompileSelection"	"TextCmds.SelectionGuard"
	"#Dev:&Compile Module List"	""	"DevCompiler.CompileModuleList"	"TextCmds.SelectionGuard"
	SEPARATOR
	"#Dev:&Unmark Errors"	""	"DevMarkers.UnmarkErrors"	"TextCmds.FocusGuard"
	"#Dev:&Next Error"	"E"	"DevMarkers.NextError"	"TextCmds.FocusGuard"
	"#Dev:&Toggle Error Mark"	"T"	"DevMarkers.ToggleCurrent"	"TextCmds.FocusGuard"
	SEPARATOR
	"#Dev:&Execute"	""	"DevDebug.Execute"	"TextCmds.SelectionGuard"
	"#Dev:&Unload"	""	"DevDebug.Unload"	"TextCmds.FocusGuard"
	"#Dev:&Unload Module List"	""	"DevDebug.UnloadModuleList"	"TextCmds.SelectionGuard"
	"#Dev:&Flush Resources"	""	"DevCmds.FlushResources"	""
	"#Dev:&Revalidate View"	""	"DevCmds.RevalidateView"	"DevCmds.RevalidateViewGuard"
	SEPARATOR
	"#Dev:&Set Profile List"	""	"DevProfiler.SetProfileList"	"DevProfiler.StartGuard"
	"#Dev:&Start Profiler"	""	"DevProfiler.Start"	"DevProfiler.StartGuard"
	"#Dev:&Stop Profiler"	""	"DevProfiler.Stop; DevProfiler.ShowProfile"	"DevProfiler.StopGuard"
	"#Dev:&Timed Execute"	""	"DevProfiler.Execute"	"TextCmds.SelectionGuard"
	SEPARATOR
	"Beautify"	""	"CpcBeautifier.Beautify"	"TextCmds.FocusGuard"
END

MENU "#Dev:&Tools"
	"#Dev:&Document Size..."	""	"StdCmds.InitLayoutDialog; StdCmds.OpenToolDialog('Std/Rsrc/Cmds1', '#Dev:Document Size')"
				"StdCmds.WindowGuard"
	"#Dev:&View Size..." 	""	"StdViewSizer.InitDialog;StdCmds.OpenToolDialog('Std/Rsrc/ViewSizer', '#Dev:View Size')"	
				"StdCmds.SingletonGuard" 
	SEPARATOR
	"#Dev:&Insert OLE Object..."	""	"OleClient.InsertObject"	"StdCmds.PasteViewGuard"
	"#Dev:&Insert Commander"	"Q"	"DevCommanders.Deposit; StdCmds.PasteView"	"StdCmds.PasteViewGuard"
	"#Dev:&Insert Stamp"	""	"StdStamps.Deposit; StdCmds.PasteView"	"StdCmds.PasteViewGuard"
	"#Dev:&Insert Clock"	""	"StdClocks.Deposit; StdCmds.PasteView"	"StdCmds.PasteViewGuard"
	"#Dev:&Insert Header"	""	"StdHeaders.Deposit; StdCmds.PasteView; TextCmds.ShowMarks"	"TextCmds.FocusGuard"
	SEPARATOR
	"#Dev:&Add Scroller"	""	"StdScrollers.AddScroller"	"StdCmds.SingletonGuard"
	"#Dev:&Remove Scroller"	""	"StdScrollers.RemoveScroller"	"StdCmds.SingletonGuard"
	SEPARATOR
	"#Dev:&Create Link"	"L"	"StdLinks.CreateLink"	"StdLinks.CreateGuard"
	"#Dev:&Create Target"	""	"StdLinks.CreateTarget"	"StdLinks.CreateGuard"
	SEPARATOR
	"#Dev:&Create Fold"	""	"StdFolds.Create(1)"	"StdFolds.CreateGuard"
	"#Dev:&Expand All"	""	"StdFolds.Expand"	"TextCmds.FocusGuard"
	"#Dev:&Collapse All"	""	"StdFolds.Collapse"	"TextCmds.FocusGuard"
	"#Dev:&Fold..."	""	"StdCmds.OpenToolDialog('Std/Rsrc/Folds', '#Dev:Zoom')"	""
	SEPARATOR
	"#Dev:&Encode Document"	""	"StdCoder.EncodeDocument"	"StdCmds.WindowGuard"
	"#Dev:&Encode Selection"	""	"StdCoder.EncodeSelection"	"TextCmds.SelectionGuard"
	"#Dev:&Encode File..."	""	"StdCoder.EncodeFile"	""
	"#Dev:&Encode File List"	""	"StdCoder.EncodeFileList"	"TextCmds.SelectionGuard"
	"#Dev:&Decode"	""	"StdCoder.Decode"	"TextCmds.FocusGuard"
	"#Dev:&About Encoded Material"	""	"StdCoder.ListEncodedMaterial"	"TextCmds.FocusGuard"
	SEPARATOR
	"#Dev:&Create Subsystem..."	""	"StdCmds.OpenToolDialog('Dev/Rsrc/Create', '#Dev:Create Subsystem')"	""
END

MENU "#Dev:&Controls"
	"#Dev:&New Form..."	""	"StdCmds.OpenToolDialog('Form/Rsrc/Gen', '#Dev:New Form')"	""
	"#Dev:&Open As Tool Dialog"	""	"StdCmds.OpenAsToolDialog"	"StdCmds.ContainerGuard"
	"#Dev:&Open As Aux Dialog"	""	"StdCmds.OpenAsAuxDialog"	"StdCmds.ContainerGuard"
	SEPARATOR
	"#Dev:&Insert Tab View"	""	"StdTabViews.Deposit; StdCmds.PasteView"	"StdCmds.PasteViewGuard"
	SEPARATOR
	"#Dev:&Insert Command Button"	""	"Controls.DepositPushButton; StdCmds.PasteView"	"StdCmds.PasteViewGuard"
	"#Dev:&Insert Check Box"	""	"Controls.DepositCheckBox; StdCmds.PasteView"	"StdCmds.PasteViewGuard"
	"#Dev:&Insert Radio Button"	""	"Controls.DepositRadioButton; StdCmds.PasteView"	"StdCmds.PasteViewGuard"
	"#Dev:&Insert Edit Field"	""	"Controls.DepositField; StdCmds.PasteView"	"StdCmds.PasteViewGuard"
	"#Dev:&Insert List Box"	""	"Controls.DepositListBox; StdCmds.PasteView"	"StdCmds.PasteViewGuard"
	"#Dev:&Insert Selection Box"	""	"Controls.DepositSelectionBox; StdCmds.PasteView"	"StdCmds.PasteViewGuard"
	"#Dev:&Insert Combo Box"	""	"Controls.DepositComboBox; StdCmds.PasteView"	"StdCmds.PasteViewGuard"
	"#Dev:&Insert Up/Down Field"	""	"Controls.DepositUpDownField; StdCmds.PasteView"	"StdCmds.PasteViewGuard"
	"#Dev:&Insert Time Field"	""	"Controls.DepositTimeField; StdCmds.PasteView"	"StdCmds.PasteViewGuard"
	"#Dev:&Insert Date Field"	""	"Controls.DepositDateField; StdCmds.PasteView"	"StdCmds.PasteViewGuard"
	"#Dev:&Insert Color Field"	""	"Controls.DepositColorField; StdCmds.PasteView"	"StdCmds.PasteViewGuard"
	"#Dev:&Insert Tree Control"	""	"Controls.DepositTreeControl; StdCmds.PasteView"	"StdCmds.PasteViewGuard"
	"#Dev:&Insert Table Control"	""	"StdTables.DepositControl; StdCmds.PasteView"	"StdCmds.PasteViewGuard"
	SEPARATOR
	"#Dev:&Insert Cancel Button"	""	"Controls.DepositCancelButton; StdCmds.PasteView"	"StdCmds.PasteViewGuard"
	"#Dev:&Insert Caption"	""	"Controls.DepositCaption; StdCmds.PasteView"	"StdCmds.PasteViewGuard"
	"#Dev:&Insert Group Box"	""	"Controls.DepositGroup; FormCmds.InsertAround; FormCmds.SetAsFirst"	"StdCmds.PasteViewGuard"
END

MENU "*" ("DevDependencies.View")
	"#Dev:&Expand"	""	"DevDependencies.ExpandClick"	"DevDependencies.SubsGuard"
	"#Dev:&Collapse"	""	"DevDependencies.CollapseClick"	"DevDependencies.ModsGuard"
	"#Dev:&New Analysis"	""	"DevDependencies.NewAnalysisClick"	"DevDependencies.ModsGuard"
	"#Dev:&Hide"	""	"DevDependencies.HideClick"	"DevDependencies.SelGuard"
	SEPARATOR
	"#Dev:&Show All Items"	""	"DevDependencies.ShowAllClick"	""
	"#Dev:&Show Basic System"	""	"DevDependencies.ToggleBasicSystemsClick"	"DevDependencies.ShowBasicGuard"
	"#Dev:&Expand All"	""	"DevDependencies.ExpandAllClick"	""
	"#Dev:&Collapse All"	""	"DevDependencies.CollapseAllClick"	""
	"#Dev:&Arrange Items"	""	"DevDependencies.ArrangeClick"	""
	SEPARATOR
	"#Dev:&Create tool..."	""	"DevDependencies.CreateToolClick"	""
	SEPARATOR
	"#Dev:&Properties..."	""	"StdCmds.ShowProp"	"StdCmds.ShowPropGuard"
END
