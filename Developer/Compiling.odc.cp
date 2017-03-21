 Compiling the Source Code


To compile the complete Component Pascal source code open the make file and use the mouse to click into the round blob containing an ! mark. If the compiler detects an error in a module the compilation will stop at that module which will be opened in a window with the error marked and described. If the directory into which the compiler wishes to place a code or symbol file does not exist the user will be prompted to create the directory. Note that if you change the interface to a module your new module may not be able to work with other developers versions of OpenBUGS. The Component Pascal module loader checks the "finger prints" (crytographic check sums) of the the exported items of each module it loads against the "finger prints" the importing module expects and flags an error if these are inconsistant. 

OpenBUGS can be run from inside the BlackBox developement enviroment. Indeed this has the advantage of making the full BlackBox debugger (DevDebug) available for diagnosing run time traps. The exact point in the source code where the problem occurs is usually detected!

There are also a few other modules in the OpenBUGS source code. These files need to the compiled when the executable version are linked (see below for details)

