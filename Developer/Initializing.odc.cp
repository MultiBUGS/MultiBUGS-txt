Initializing OpenBUGS 


Before the OpenBUGS software runs it needs to be initialized.  The code to initialize OpenBUGS is in module Startup in file Bugs/Startup.odc in the Setup procedure. Each subsystem, Xxx,  that needs initializing contains a XxxResources module which contains an exported initialization procedure Load  to load  resources. Meta programming is used to search for modules of with name XxxResources and load them if not already loaded, their Load procedures are then called. 

