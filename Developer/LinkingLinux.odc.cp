	Executable versions and
					 linking


The MultiBUGS software family is unusual in consisting of a large number of unlinked modules in a binary format .These modules can not be directly accessed by the operating system. A linker tool is used to link a number of binary modules into either an executable or a library. The BlackBox developement tools contain a special module the StdLoader which is able to load and link new modules into a running MultiBUGS application. There are two approaches to packageing the MultiBUGS software. In the first only a small number of modules are linked along with StdLoader in the second all the modules in the MultiBUGS software are linked into one large file. The first approach is more dynamic and flexible and is used in the Windows version of MultiBUGS. However it has some disadvantages: it involves a large number of files in a particular directory structure which can become fragile when used with a library for interfacing to other software. For this reason we prefer to fully link the library versions of MultiBUGS.

There are various ways of running the MultiBUGS software on computers with the Microsoft Windows operating system. MultiBUGS.exe is small Microsoft Windows program which loads code files (ocf) as needed. The classicbugs.exe program has a similar interface to the old "ClassicBUGS" software. A shortcut BackBUGS runs MultiBUGS with the file name specified on the command line, after the key word /PAR, used as a source of commands in a MultiBUGS script. This allows another program to call MultiBUGS to do MCMC simulation. The BackBUGS shortcut has been set up so that no window opens while MultiBUGS runs. This can be changed by editing the shortcut so that "/HEADLESS" is removed from the command line.

The MultiBUGS dynamic link library was designed for use from within Splus/R statistical programming enviroment and only uses modues that do not depend on Microsoft Windows. The procedures exported by the brugs library are specified in the module BugsBRugs. In particular the CLI procedure sets up a simple command line interpreter -- the ClassicBUGS interface. It is also possible to use the dynamic link library from within a general C program (see BRugs header file for details). 

In general the ocf files are not operating system dependent. In the make file those modules in black should be usable from both Windows and Linux, whereas the modules in red are only used under Windows. With just a few changes the modules that are linked to form the Windows dynamic link library can also be linked to produce a shared object file for Linux.  

The R functions for interfacing to the dynamic link library / shared object file are in the BRugs package on CRAN. The names of these R functions are the same as in the MultiBUGS scripting language and have the same function. Both the scripting language and the R functions use metaprogramming to communicate with MultiBUGS, see the module BugsStdInterpreter for details.

To make the executable versions of the MultiBUGS software a linker tool is used to pack several code (ocf) files into one exe or dynamic link library (shared object file on Linux). The linker is given a list of module names (in the form subsystem prefix followed by a file name). The linker searches for the ocf file in the code subdirectory of the subsystem with the appropiate file name. Note that this ocf might correspond to a module with a name different to the name given to the linker. Winows and Linux use different binary formats for executables and libraries. Therefore two linker tools are required: DevLinker for Windows and DevElfLinker for Linux.


How to link MultiBUGS

To create the MultiBUGS.exe file click in the round .blob containing an ! mark with the mouse

Dev2Linker1.LinkElfExe Linux OpenBUGS := Kernel$ + Files HostFiles  HostGnome StdLoader
1 Bugslogo.ico 2 Doclogo.ico 3 SFLogo.ico 4 CFLogo.ico 5 DtyLogo.ico
6 folderimg.ico 7 openimg.ico 8 leafimg.ico
1 Move.cur 2 Copy.cur 3 Link.cur 4 Pick.cur 5 Stop.cur 6 Hand.cur 7 Table.cur 

