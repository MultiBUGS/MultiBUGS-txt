MENU "File"
	"&New"	"N"	"StdCmds.New"	""
	"&Open..."	"O"	"HostCmds.Open"	""
	"&Save"	"S"	"HostCmds.Save"	"HostCmds.SaveGuard"
	"Save &As..."	""	"HostCmds.SaveAs"	"StdCmds.WindowGuard"
	"&Close"	""	"HostCmds.Close"	"StdCmds.WindowGuard"
	SEPARATOR
	"Page Se&tup..."	""	"HostDialog.InitPageSetup; StdCmds.OpenToolDialog('HostDialog.setup', 'Page Setup')"
				"StdCmds.WindowGuard"
	"&Print..."	"P"	"HostCmds.Print"	"HostCmds.PrintGuard"
	SEPARATOR
	"Send &Document..."	""	"HostMail.SendDocument"	"HostMail.SendDocumentGuard"
	"Send Not&e..."	""	"HostMail.SendNote"	"HostMail.SendNoteGuard"
	SEPARATOR
	"E&xit"	""	"BugsCmds.Quit('no')"	""
END

MENU "Edit"
	"&Undo"	"Z"	"StdCmds.Undo"	"StdCmds.UndoGuard"
	"R&edo"	"R"	"StdCmds.Redo"	"StdCmds.RedoGuard"
	SEPARATOR
	"Cu&t"	"X"	"HostCmds.Cut"	"HostCmds.CutGuard"
	"&Copy"	"C"	"HostCmds.Copy"	"HostCmds.CopyGuard"
	"&Paste"	"V"	"HostCmds.Paste"	"HostCmds.PasteGuard"
	"&Delete	Delete"	""	"StdCmds.Clear"	"HostCmds.CutGuard"
	SEPARATOR
	"Paste O&bject"	""	"HostCmds.PasteObject"	"HostCmds.PasteObjectGuard"
	"Paste &Special..."	""	"OleClient.PasteSpecial"	"HostCmds.PasteObjectGuard"
	"Paste to &Window"	""	"HostCmds.PasteToWindow"	"HostCmds.PasteToWindowGuard"
	SEPARATOR
	"&Insert Object..."	""	"OleClient.InsertObject"	"StdCmds.PasteViewGuard"
	"Object P&roperties...	Alt+Enter"	""	"StdCmds.ShowProp"	"StdCmds.ShowPropGuard"
	"&Object"	""	"HostMenus.ObjectMenu"	"HostMenus.ObjectMenuGuard"
	SEPARATOR
	"Select Docu&ment"	" "	"StdCmds.SelectDocument"	"StdCmds.WindowGuard"
	"Select &All"	"A"	"StdCmds.SelectAll"	"StdCmds.SelectAllGuard"
	"Select &Next Object"	"F6"	"StdCmds.SelectNextView"	"StdCmds.ContainerGuard"
	SEPARATOR
	"Pre&ferences..."	""	"HostDialog.InitPrefDialog; StdCmds.OpenToolDialog('Host/Rsrc/Prefs', 'Preferences')"	""
END

MENU "Attributes"
	"&Regular"	""	"StdCmds.Plain"	"StdCmds.PlainGuard"
	SEPARATOR
	"&Bold"	"B"	"StdCmds.Bold"	"StdCmds.BoldGuard"
	"&Italic"	"I"	"StdCmds.Italic"	"StdCmds.ItalicGuard"
	"&Underline"	"U"	"StdCmds.Underline"	"StdCmds.UnderlineGuard"
	SEPARATOR
	" &8 Point"	""	"StdCmds.Size(8)"	"StdCmds.SizeGuard(8)"
	" &9 Point"	""	"StdCmds.Size(9)"	"StdCmds.SizeGuard(9)"
	"&10 Point"	""	"StdCmds.Size(10)"	"StdCmds.SizeGuard(10)"
	"1&2 Point"	""	"StdCmds.Size(12)"	"StdCmds.SizeGuard(12)"
	"1&6 Point"	""	"StdCmds.Size(16)"	"StdCmds.SizeGuard(16)"
	"2&0 Point"	""	"StdCmds.Size(20)"	"StdCmds.SizeGuard(20)"
	"2&4 Point"	""	"StdCmds.Size(24)"	"StdCmds.SizeGuard(24)"
	"&Size..."	""	"StdCmds.InitSizeDialog; StdCmds.OpenToolDialog('Std/Rsrc/Cmds', 'Size')"
				"StdCmds.SizeGuard(-1)"
	SEPARATOR
	"&Default Color"	""	"StdCmds.Color(1000000H)"	"StdCmds.ColorGuard(1000000H)"
	"Blac&k"	""	"StdCmds.Color(0000000H)"	"StdCmds.ColorGuard(0000000H)"
	"R&ed"	""	"StdCmds.Color(00000FFH)"	"StdCmds.ColorGuard(00000FFH)"
	"&Green"	""	"StdCmds.Color(000FF00H)"	"StdCmds.ColorGuard(000FF00H)"
	"B&lue"	""	"StdCmds.Color(0FF0000H)"	"StdCmds.ColorGuard(0FF0000H)"
	"&Color..."	""	"HostDialog.ColorDialog"	"StdCmds.ColorGuard(-1)"
	SEPARATOR
	"Default F&ont"	""	"StdCmds.DefaultFont"	"StdCmds.DefaultFontGuard"
	"&Font..."	""	"HostDialog.FontDialog"	"StdCmds.TypefaceGuard"
	"&Typeface..."	""	"HostDialog.TypefaceDialog"	"StdCmds.TypefaceGuard"
END

MENU "Tools"
	"Document Size..."	""	"StdCmds.InitLayoutDialog; StdCmds.OpenToolDialog('Std/Rsrc/Cmds1', 'Document Size')"
				""
	SEPARATOR
	"Maths tool..."	""	"StdCmds.OpenToolDialog('Gft/Rsrc/Glyphs', 'Maths symbols')"	""
	"Insert OLE &Object..."	""	"OleClient.InsertObject"	"StdCmds.PasteViewGuard"
	"Insert &Header"	""	"StdHeaders.Deposit; StdCmds.PasteView; TextCmds.ShowMarks"	"TextCmds.FocusGuard"
	SEPARATOR
	"Create &Link"	"L"	"StdLinks.CreateLink"	"StdLinks.CreateGuard"
	"Create &Target"	""	"StdLinks.CreateTarget"	"StdLinks.CreateGuard"
	SEPARATOR
	"Create Fold"	""	"StdFolds.Create(1)"	"StdFolds.CreateGuard"
	"Expand All"	""	"StdFolds.Expand"	"TextCmds.FocusGuard"
	"Collapse All"	""	"StdFolds.Collapse"	"TextCmds.FocusGuard"
	"Fold..."	"" 	"StdCmds.OpenToolDialog('Std/Rsrc/Folds', 'Zoom')"	""
	SEPARATOR
	"Encode Document"	""	"StdCoder.EncodeDocument"	"StdCmds.WindowGuard"
	"Encode Selection"	""	"StdCoder.EncodeSelection"	"TextCmds.SelectionGuard"
	"Encode File..."	""	"StdCoder.EncodeFile"	""
	"Encode File List"	""	"StdCoder.EncodeFileList"	"TextCmds.SelectionGuard"
	"Decode"	""	"StdCoder.Decode"	"TextCmds.FocusGuard"
	"About Encoded Material"	""	"StdCoder.ListEncodedMaterial"	"TextCmds.FocusGuard"
END

INCLUDE "*"

MENU "Window"
	"&New Window"	""	"StdCmds.NewWindow"	"StdCmds.WindowGuard"
	SEPARATOR
	"&Cascade"	""	"HostMenus.Cascade"	"StdCmds.WindowGuard"
	"Tile &Horizontal"	""	"HostMenus.TileHorizontal"	"StdCmds.WindowGuard"
	"&Tile Vertical"	""	"HostMenus.TileVertical"	"StdCmds.WindowGuard"
	"&Arrange Icons"	""	"HostMenus.ArrangeIcons"	"StdCmds.WindowGuard"
	SEPARATOR
	"*"	""	"HostMenus.WindowList"	""
END

MENU "Examples"
	"Examples Vol I"	""	"StdCmds1.OpenBrowser('Examples/VolumeI', 'Examples Volume I')"	""
	"Examples Vol II"	""	"StdCmds1.OpenBrowser('Examples/VolumeII', 'Examples Volume II')"	""
	"Examples Vol III"	""	"StdCmds1.OpenBrowser('Examples/VolumeIII', 'Examples Volume III')"	""	
	"Examples Vol IV"	""	"StdCmds1.OpenBrowser('Examples/VolumeIV', 'Examples Volume IV')"	""	
	SEPARATOR
	"GeoBUGS Examples"	""	"StdCmds1.OpenBrowser('GeoBUGS/Examples/Examples', 'GeoBUGS Examples')"	""	
	"Ecology Examples"	""	"StdCmds1.OpenBrowser('Examples/VolumeEco', 'Ecology Examples')"	""	
	"Reliability Examples"	""	"StdCmds1.OpenBrowser('Reliability/Examples/Vol', 'Reliability Examples')"	""	
	"PK Examples"	""	"StdCmds1.OpenBrowser('PKBugs/Examples/Examples', 'PK Examples')"	""	
	SEPARATOR
	"Print Examples Vol I"	""	"BugsDocu.PrintExamplesVolI"	""
	"Print Examples Vol II"	""	"BugsDocu.PrintExamplesVolII"	""
	"Print Examples Vol III"	""	"BugsDocu.PrintExamplesVolIII"	""
	"Print Examples Vol IV"	""	"BugsDocu.PrintExamplesVolIV"	""
	"Print GeoBUGS Examples"	""	"BugsDocu.PrintExamplesGeoBUGS"	""
	"Print Ecology Examples"	""	"BugsDocu.PrintExamplesEcology"	""
	"Print Reliability Examples"	""	"BugsDocu.PrintReliabilityManual"	""
END

MENU "Manuals"
	"Getting started"	""	"StdCmds1.OpenBrowser('Manuals/Manual', 'Getting started')"	""
	SEPARATOR
	"MultiBUGS User Manual"	"F1"	"StdCmds1.OpenBrowser('Manuals/Contents', 'MultiBUGS User Manual Contents')"	""
	"DoodleBUGS User Manual"	""	"StdCmds1.OpenBrowser('Manuals/DoodleBUGS', 'DoodleBUGS User Manual')"	""
	"GeoBUGS User Manual"	""	"StdCmds1.OpenBrowser('GeoBUGS/Manuals/Manual', 'GeoBUGS User Manual')"	""		
	"ReliaBUGS User Manual"	""	"StdCmds1.OpenBrowser('Reliability/Manuals/Contents', 'ReliaBUGS User Manual')"	""	
	"PKBUGS User Manual"	""	"StdCmds1.OpenBrowser('PKBugs/Manuals/Manual', 'PKBUGS User Manual')"	""	
	"OpenBUGS Developer Manual"	""	"StdCmds1.OpenBrowser('Developer/Manual','OpenBUGS Developer Manual')" ""
	SEPARATOR
	"Print MultiBUGS User Manual"	""	"BugsDocu.PrintUserManual"	""
	"Print GeoBUGS User Manual"	""	"BugsDocu.PrintGeoBUGSManual"	""
	"Print ReliaBUGS User Manual"	""	"BugsDocu.PrintReliabilityManual"	""
	"Print MultiBUGS Developer Manual"	""	"BugsDocu.PrintDeveloperManual"	""
END

MENU "Help"
	"Distributions"	""	"StdCmds.OpenBrowser('Manuals/ModelSpecification', 'OpenBUGS User Manual');StdLinks.ShowTarget('ContentsAI')"	""
	"Functions"	""	"StdCmds.OpenBrowser('Manuals/ModelSpecification', 'OpenBUGS User Manual');StdLinks.ShowTarget('ContentsAII')"	""
	SEPARATOR	
	"Search"	""	"StdCmds.OpenToolDialog('Bugs/Rsrc/Search', 'Search Tool')"		""
	SEPARATOR	
	"Licence"	""	"StdCmds1.OpenBrowser('gpl-3.0.txt', 'GNU GPL Licence')"	""
	"About MultiBUGS"	""	"StdCmds.OpenToolDialog('System/Rsrc/About', 'About MultiBUGS')"	""
END

MENU "*" 
	"Cu&t"	""	"HostCmds.Cut"	"HostCmds.CutGuard"
	"&Copy"	""	"HostCmds.Copy"	"HostCmds.CopyGuard"
	"&Paste"	""	"HostCmds.Paste"	"HostCmds.PasteGuard"
	"&Delete"	""	"StdCmds.Clear"	"HostCmds.CutGuard"
	SEPARATOR
	"P&roperties..."	""	"StdCmds.ShowProp"	"StdCmds.ShowPropGuard"
	"&Object"	""	"HostMenus.ObjectMenu"	"HostMenus.ObjectMenuGuard"
END
