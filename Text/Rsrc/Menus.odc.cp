MENU "#Text:&Text" ("TextViews.View")
	"#Text:&Find / Replace..."	"F"	"TextCmds.InitFindDialog; 
	StdCmds.OpenToolDialog('Text/Rsrc/Cmds', '#Text:Find / Replace')"	""
	"#Text:&Find Again"	"F3"	"TextCmds.InitFindDialog; TextCmds.FindAgain('~R')"		"TextCmds.FindAgainGuard"
	"#Text:&Find Previous"	"*F3"	"TextCmds.InitFindDialog; TextCmds.FindAgain('R')"		"TextCmds.FindAgainGuard"
	"#Text:&Find First"	"F4"	"TextCmds.InitFindDialog; TextCmds.FindFirst('~R')"		"TextCmds.FindAgainGuard"
	"#Text:&Find Last"	"*F4"	"TextCmds.InitFindDialog; TextCmds.FindFirst('R')"		"TextCmds.FindAgainGuard"
	SEPARATOR
	"#Text:&Shift Left"	"F11"	"TextCmds.ShiftLeft"	"TextCmds.EditGuard"
	"#Text:&Shift Right"	"F12"	"TextCmds.ShiftRight"	"TextCmds.EditGuard"
	"#Text:&Superscript"	""	"TextCmds.Superscript"	"TextCmds.SelectionGuard"
	"#Text:&Subscript"	""	"TextCmds.Subscript"	"TextCmds.SelectionGuard"
	SEPARATOR
	"#Text:&Insert Paragraph"	"M"	"TextCmds.InsertParagraph"	"StdCmds.PasteCharGuard"
	"#Text:&Insert Ruler"	"J"	"TextCmds.InsertRuler"	"StdCmds.PasteViewGuard"
	"#Text:&Insert Soft-Hyphen"	""	"TextCmds.InsertSoftHyphen"	"StdCmds.PasteCharGuard"
	"#Text:&Insert Non-Brk Hyphen"	""	"TextCmds.InsertNBHyphen"	"StdCmds.PasteCharGuard"
	"#Text:&Insert Non-Brk Space"	""	"TextCmds.InsertNBSpace"	"StdCmds.PasteCharGuard"
	"#Text:&Insert Digit Space"	""	"TextCmds.InsertDigitSpace"	"StdCmds.PasteCharGuard"
	"#Text:&Toggle Marks"	"H"	"TextCmds.ToggleMarks"	"TextCmds.ToggleMarksGuard"
	SEPARATOR
	"#Text:&Make Default Attributes"	""	"TextCmds.MakeDefaultAttributes"	"TextCmds.SelectionGuard"
	"#Text:&Make Default Ruler"	""	"TextCmds.MakeDefaultRuler"	"StdCmds.SingletonGuard"
END

MENU "*" ("TextViews.View")
	"#Text:&Cut"	""	"HostCmds.Cut"	"HostCmds.CutGuard"
	"#Text:&Copy"	""	"HostCmds.Copy"	"HostCmds.CopyGuard"
	"#Text:&Paste"	""	"HostCmds.Paste"	"HostCmds.PasteGuard"
	"#Text:&Delete"	""	"StdCmds.Clear"	"HostCmds.CutGuard"
	SEPARATOR
	"#Text:&Source"	""	"DevReferences.ShowSource"	"TextCmds.SelectionGuard"
	"#Text:&Interface"	""	"DevBrowser.ShowInterface('')"	"TextCmds.SelectionGuard"
	"#Text:&Documentation"	""	"DevReferences.ShowDocu"	"TextCmds.SelectionGuard"
	SEPARATOR
	"#Text:&Properties..."	""	"StdCmds.ShowProp"	"StdCmds.ShowPropGuard"
	"#Text:&Object"	""	"HostMenus.ObjectMenu"	"HostMenus.ObjectMenuGuard"
END

