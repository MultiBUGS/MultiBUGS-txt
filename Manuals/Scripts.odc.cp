	Scripts and Batch-mode 



Scripting

As an alternative to the menu / dialog box interface,  scripting commands has been created that can 1) automate routine analyses, 2) create reproducible analyses, and 3) produce batch execution for simulation studies, etc. The scripting commands work in effect by writing values into fields and clicking on buttons in relevant dialog boxes. It is possible to combine use of  scripting with the menu / dialog box interface.

The scripting commands are very similar to those in the R interface to BUGS called BRugs. Their syntax differs from the scripting commands in WinBUGS 1.4.x, although there are corresponding WinBUGS/BUGS script commands for most tasks.  

To make use of the scripting commands, a minimum of three files is required: a file containing the script commands, a file containing the BUGS language representation of the model; and a file (or several) containing the data.  If initial values are supplied (recommended) rather than generated by BUGS, an additional file is required for each chain. 

On all operating systems, directory names are separated by a forward slash (/).  A backslash (\) is also allowed with Windows.  Quoted input can be created using either single (') or double (") quotation marks, but the quotation mark types must match.  The modelSetWD and modelGetWD script commands (Script Commands) can be used so that full path names are not required for all file names.  The linux shell commands (Linux execution) offer an alternative approach for path names in linux.



Executing script commands from the GUI

A set of script commands can be executed from the GUI by selecting the window that contains them, and then selecting Script from the Model menu.  All commands in the window are executed, unless some commands are highlighted, in which case only the highlighted commands are executed (a partially highlighted command will produce an error).  The contents of a window with script commands does not have to be saved before it is executed.  Input files may be in either native BUGS format (.odc) or text format, in which case it must have a .txt extension.

After execution of script commands, GUI menu selections can be used to view changed settings.  If a dialog box was already open when a script was executed, it may need to be closed and re-opened to ensure that its contents are refreshed.  Execution of script and GUI commands can be mixed, allowing automation of tasks such as summary setting for models with numerous parameters.



Batch-mode in Windows

BUGS can execute a file of script commands in Batch-mode from a programming shell using the following syntax:

"FULLPATH/BUGS.exe" /PAR "FULLPATH/ScriptName.txt" /HEADLESS

It is important to add  modelQuit('yes') at the end of the script file so BUGS will stop execution and return control to the initiating shell when the script commands are complete.  It is also important to include a modelDisplay('log') at the beginning of the script file, and modelSaveLog('filename.odc') before modelQuit as this is the only way to capture tabular and graphical output in batch-mode.  (If there is no graphical output, the log file can be of type '.txt'.).

The /HEADLESS option causes BUGS to run without  displaying any output.  Removing /HEADLESS will cause the usual GUI windows to appear during execution.  If /HEADLESS is removed and the modelQuit command is omitted, the user can interact with the BUGS program before returning control to the programming shell.  Using modelQuit() without 'yes' will also cause BUGS to display a dialog box for user input before exiting.

Input files (including the script) may be in either native BUGS format (.odc) or text format.  Text format requires a .txt extension.


Batch-mode in Linux

BUGS can be executed from the Linux OS on Intel-based machines in a native format that yields results that exactly agree with those from the Windows OS on the same hardware.  The GUI, and graphical output (e.g., history plots) depend on Windows-specific code, however, and are not available with Linux execution.

A bash shell script called BUGS is included in the standard Linux distribution in the /bin sub-directory.  Once this directory is added to a user's path,  the BUGS command behaves like a standard Linux program.  Typing BUGS without inputs starts the program in interactive mode with screen I/O.  To exit the program type "modelQuit()".    To execute a script file, script.txt, in batch mode with output to file log.txt:

BUGS script.txt > log.txt &
or
BUGS <script.txt >log.txt &

The paths for input and output files within a script file, such as the model code or coda output, is assumed to be the current working directory unless the paths are fully specified.  The script command modelSetWD("/path/to/dir/") can be specified to reset the working directory to /path/to/dir. 

Additional options can be found by typing BUGS --help.

Input files must be in ASCII format.  The log file (ASCII format) is always written to standard output with Linux execution, so the modelSaveLog command is disabled during Linux execution.  


The	scripting commands

Script commands and a brief synopsis of their menu/dialog box equivalent are supplied below. The script command and corresponding menu name are in bold.  The menu names are followed by their associated menu options or dialog box and potential user inputs. You can find detailed descriptions of the menu/dialog box options in the corresponding Manual entries for the Model menu, Inference menu, Info menu and File menu.

Script commands have two input types: character strings and integers. If a string argument contains non-alpha-numeric characters (including space but excluding the period) the string must be enclosed in  quotes. Arguments that are required to be supplied by the user are shown in blue italics; optional arguments are displayed in plain text.  Optional arguments will be set to default values if they are omitted.

If the menu/dialog box equivalent of the specified script command would normally be grayed out because of inappropriate timing, for example, then the script command will not execute and an error message will be produced instead.

Commands that are preceded by an asterisk (*), are not available with Linux execution.

The translations between commands in the script language and the underlying Component Pascal procedures implementing them in BUGS are in the file Bugs/Mod/Script.odc in the BUGS program directory.  Explanations of the commands and their inputs are in the documentation for the menu items.


	Script_command_description		Menu_/_dialog_box_equivalent
	
	modelGetWD()		None
	Returns the default path for file name input

	modelSetWD(string)		None
	- string = Full default path for file name input

	modelCheck(string)		Model > Specification... > 	
	- string = (full path to) Model file
			
	modelData(string)		Model > Specification... >	
	- string = (full path to) Data file
			
	modelCompile(int)		Model > Specification... >	
	- int = number of chains
	
	modelInits(string, int)		Model > Specification... >	
	- string = (full path to) Inits file
	- int = for chain (default=1)
	The initial values for each chain must be in separate files.
			
	modelGenInits()		Model > Specification... >	

	_____________________________________________________________________
	
	modelUpdate(int0, int1, int2, string)		Model > Update...> 
	- int0 = updates
	- int1 = thin.  Thinning in this way will discard the samples (c.f. the samplesThin() command)
	- int2 = frequency of refreshing the display in the Windows interface
	- string = "T" or "F" for over relax
	_____________________________________________________________________

	modelSaveState(string)		Model > Save State
	- string = fileStem
	Note that the string must be specified for Linux execution.
	_____________________________________________________________________

	modelGetRN()		Model > RN generator...
	Returns the starting (preset) state of the random number generator
	
	modelSetRN(int)		Model > RN generator...
	Set the starting (preset) state of the random number generator to 'int'.  This must be an 
	integer from 1 to 14 inclusive. 
	_____________________________________________________________________
		
	modelDisplay(string)		Model > Input/Output options...
	- string = 'window' or 'log'
	
	modelPrecision(int)		Model > Input/Output options...
	- int = number of significant digits for output
	_____________________________________________________________________

	modelDisable(string)		Model > Updater options...
	- string = name of updater to disable
	
	modelEnable(string)		Model > Updater options...
	- string = name of updater to enable
	
	modelSetAP(string, int)		Model> Updater options...
	- string = name of sampler
	- int = number of iterations in the adaptive phase 
	
	modelSetIts(string, int)		Model > Updater options...
	- string = name of sampler 
	- int = number of iterations
	
	modelSetOR(string, int)		Model > Updater options...
	- string = name of sampler
	- int = number of samples to generate for over-relaxed MCMC
	_____________________________________________________________________			
	modelExternalize(string)		Model > Externalize
	- string = file name, including the extension.  The extension is conventionally ".bug" for externalized BUGS models.
	
	modelInternalize(string)		Model > Internalize
	- string = file name, including extension. 

	modelQuit(string)		File > Exit
	- string = 'y' or 'yes' to quick without a dialog box opening before exiting
	If string is omitted, a dialog box appears before exit.
	          
	*modelSaveLog(string)		File > Save As... > 
	- string = file name
	If the file ends with ".txt" the log window is saved to a text file (with all graphics, fonts, etc., 					stripped out).
	_____________________________________________________________________
	
	samplesSet(string)		Inference > Samples... >	
	- string = node to set
	
	samplesClear(string)		Inference > Samples... >	
	- string = node to clear
	
	samplesBeg(int)		Inference > Samples...
	- int = beginning update to define subset of the stored sample for analysis
	
	samplesEnd(int)		Inference > Samples... 
	- int = ending update to define subset of the stored sample for analysis
	
	samplesFirstChain(int)		Inference > Samples...
	- int = select beginning of range of chains which contribute to statistics being calculated
	
	samplesLastChain(int)		Inference > Samples... 
	- int = select end of range of chains which contribute to statistics being calculated
	
	samplesThin(int)		Inference > Samples...
	- int = thin.  Every intth sample is used for inference.  Does not impact storage
	 requirements.
	
	samplesStats(string)		Inference > Samples... >	
	- string = node for which to calculate sample statistics.  Can use '*' for all set nodes.
	
	*samplesDensity(string)		Inference > Samples... >	
	- string = node for which to generate density plot.  Can use '*' for all set nodes.
	
	*samplesAutoC(string)		Inference > Samples... >	
	- string = node for which to generate autocorrelation plot.  Can use '*' for all set nodes.
	
	*samplesTrace(string)		Inference > Samples... >	
	- string = node for which to generate trace plot.  Can use '*' for all set nodes.
	
	*samplesHistory(string)		Inference > Samples... >	
	- string = node for which to generate history plot.  Can use '*' for all set nodes.
	
	*samplesQuantiles(string)		Inference > Samples... >	
	- string = node for which to generate quantiles plot.  Can use '*' for all set nodes.
	
	*samplesBgr(string)		Inference > Samples... >	
	- string = node for which to generate Brooks-Gelman-Rubin diagnostics.  
	Can use '*' for all set nodes.
	
	samplesCoda(string0, string1)		Inference > Samples... >	
	- string0 = node for which to generate CODA output.  Can use '*' for all set nodes.
	- string1 = 'fileStem'
	______________________________________________________________
	
	summarySet(string)		Inference > Summary... > 	
	- string = node for which to calculate running mean, standard deviation and quantiles
	
	summaryStats(string)		Inference > Summary... > 	
	- string = node for which to display approximate running summary statistics
	
	summaryMean(string)		Inference > Summary... > 	
	- string = node for which to display running mean in comma delimited form
	
	summaryClear(string)		Inference > Summary... > 	
	- string = node to remove the running summary statistics
	_____________________________________________________________________
			
	ranksSet(string)		Inference > Rank... > 
	- string = node to be ranked (must be an array)
	
	ranksStats(string)		Inference > Rank... > 
	- string = node for which to summarize the simulated ranks of the components
	
	*ranksHistogram(string)		Inference > Rank... > 
	- string = node for which to display a histogram of the simulated ranks of the components
	
	ranksClear(string)		Inference > Rank... > 
	- string = node for which to clear the running summary
	_____________________________________________________________________
	
	dicSet()		Inference > DIC... >
	Start calculating DIC and related statistics
	
	dicClear()		Inference > DIC... > 
	Clear DIC calculations from memory
	
	dicStats()		Inference >DIC... > 
	Display DIC, Dbar, Dhat, and pD
	_____________________________________________________________________
	
	infoNodeValues(string)		Info > Node info... > 
	- string = node for which to display the current value(s)
	
	infoNodeMethods(string)		Info > Node info... > 
	- string = node for which you want to see the type of updater used to sample from
	
	infoNodeTypes(string)		Info > Node info... > 
	- string = node for which you want to see the node type
	
	infoMemory()		Info > Memory
	Show the amount of memory allocated
	_____________________________________________________________________
	
	*infoUnitializedUpdaters()		Info > Uninitialized Nodes
	Shows nodes in the compiled model that have not been initialized yet.
	
	infoUpdatersbyName()		Info > Updaters(by name)
	List the nodes with their associated updater algorithm in alphabetical order .
	
	infoUpdatersbyDepth()		Info > Updaters(by depth)
	List the nodes with their associated updater algorithm in the reverse topological order to 
	which they occur in the graphical model.
	_____________________________________________________________________
	
	infoModules()		Info > Modules
	Displays all the modules (dynamic link libraries) in use. 
		


Example

The script code below shows the steps you might use to reproduce the Rats example.  To execute, highlight the entire script (or one line at a time) and select Model > Script.

# Check model syntax
modelCheck('C:/Program Files/BUGS/Examples/Ratsmodel.txt')

# Load data
modelData('C:/Program Files/BUGS/Examples/Ratsdata.txt')

# Compile with one chain
modelCompile(1)

# Load inital values for first chain
modelInits('C:/Program Files/BUGS/Examples/Ratsinits.txt',1)

# Start with 1000 update burn-in
modelUpdate(1000)

# Set nodes of interest
samplesSet('alpha0')
samplesSet('beta.c')
samplesSet('sigma')

# Follow by a further 10,000 updates
modelUpdate(10000)

# Look at sample statistics
samplesStats('*')