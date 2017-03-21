	Documenting the OpenBUGS API


A tool has been developed to produce documentation of the interface of each module in the OpenBUGS software. This documentation contains the needed mark up so that the documentation tools of BlackBox component builder work. The tool uses a modified version of the browser from BlackBox (module DevBrowser). The tools works by extracting tagged comments from the source code and displaying them in a relavent position in the documentation. The standard BlackBox target views are used to tag comments in the source code. The following tag labels are used: module name, constant names, type names, field names in stuctured types,
names of type bound (method), variable names and procedure names.

As a example of a documented module look at  BugsNames. In the Text menu pick the Show Marks option to make the tag pairs visible. Holding the control key down and clicking into one of the circle marks will show the tag pair in text form. The tag can then be closed again by highlighting the text between the openening < and the final > in the closing <> and then picking Create Target from the Tools menu.

A dialog box for the documentation tool is under DevInfo. Documentation for a complete subsystem is produced and placed in the relevant files in the appropiate Docu subdirectory. The tool also produces a hypertext index of all the source code and documentation files in the subsystem, this is placed in file SysMap in the root directory of the subsystem. A version of the tool to produce documentation for the module in the focus window is also available. This version of the tool will open a new window containing the module documentation. 
	


