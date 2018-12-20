	Executable versions and
					 linking


The OpenBUGS software family is unusual in consisting of a large number of unlinked modules in a binary format .These modules can not be directly accessed by the operating system. A linker tool is used to link a number of binary modules into either an executable or a library. The BlackBox developement tools contain a special module the StdLoader which is able to load and link new modules into a running OpenBUGS application. 

The OpenBUGS dynamic link library was designed for use from within Splus/R statistical programming enviroment. The procedures exported by the brugs library are specified in the module BugsBRugs. In particular the CLI procedure sets up a simple command line interpreter -- the ClassicBUGS interface. It is also possible to use the dynamic link library from within a general C program (see BRugs header file for details). 

In general the ocf files are not operating system dependent. With just a few changes the modules that are linked to form the Windows dynamic link library can also be linked to produce a shared object file for Linux.  

The R functions for interfacing to the dynamic link library / shared object file are in the BRugs package on CRAN. The names of these R functions are the same as in the OpenBUGS scripting language and have the same function. Both the scripting language and the R functions use metaprogramming to communicate with OpenBUGS, see the module BugsStdInterpreter for details.

To make the executable versions of the OpenBUGS software a linker tool is used to pack several code (ocf) files into one exe or dynamic link library (shared object file on Linux). The linker is given a list of module names (in the form subsystem prefix followed by a file name). The linker searches for the ocf file in the code subdirectory of the subsystem with the appropiate file name. Note that this ocf might correspond to a module with a name different to the name given to the linker. Winows and Linux use different binary formats for executables and libraries. Therefore two linker tools are required: DevLinker for Windows and DevElfLinker for Linux.


How to link OpenBUGS

To create the OpenBUGS.exe file click in the round .blob containing an ! mark with the mouse

DevLinker.Link
OpenBUGS.exe := Kernel$ + Files HostFiles  HostPackedFiles StdLoader
1 Bugslogo.ico 2 Doclogo.ico 3 SFLogo.ico 4 CFLogo.ico 5 DtyLogo.ico
6 folderimg.ico 7 openimg.ico 8 leafimg.ico
1 Move.cur 2 Copy.cur 3 Link.cur 4 Pick.cur 5 Stop.cur 6 Hand.cur 7 Table.cur 

To bind the licence file, menu files and about box into the executable click in the round .blob containing an ! mark with the mouse.

 DevPacker.PackThis OpenBUGS.exe :=
gpl-3.0.txt
Bugs/Rsrc/Menus.odc
Doodle/Rsrc/Menus.odc
Maps/Rsrc/Menus.odc
Graph/Sym/Logical.osf
Graph/Sym/Nodes.osf
Graph/Sym/Rules.osf
Graph/Sym/Scalar.osf
Graph/Sym/Stochastic.osf
Math/Sym/Func.osf
System/Sym/Math.osf
System/Sym/Stores.osf


To create the dynamic link library LibOpenBUGS.dll click in the round blob containg an ! mark with the mouse

DevLinker.LinkDynDll
LibOpenBUGS.dll := Kernel$ + Files HostFiles HostPackedFiles StdLoader Math Strings Meta Console BugsMappers BugsFiles BugsInterpreter BugsMsg BugsStrings BugsScripting BugsCLI BugsC#





