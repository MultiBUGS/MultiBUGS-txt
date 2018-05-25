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

DevLinker.Link
MultiBUGS.exe := Kernel$ + Files HostFiles  HostPackedFiles StdLoader
1 Bugslogo.ico 2 Doclogo.ico 3 SFLogo.ico 4 CFLogo.ico 5 DtyLogo.ico
6 folderimg.ico 7 openimg.ico 8 leafimg.ico
1 Move.cur 2 Copy.cur 3 Link.cur 4 Pick.cur 5 Stop.cur 6 Hand.cur 7 Table.cur 

To bind the licence file, menu files and about box into the executable click in the round .blob containing an ! mark with the mouse.

 DevPacker.PackThis MultiBUGS.exe :=
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


To create a dos version of MultiBUGS click into the round blob containing a ! with the mouse

DevLinker.Link
dos ClassicBUGS.exe := BugsClassic$ 1 Bugslogo.ico 


To make a fully linked version of the dynamic link library for Windows click in the round blob below. Modules in red are BlackBox modules. The other modules, the MultiBUGS modules, should occur in the same order as in the make file and not include any of the Windows only modules. Module WinConsole contains Windows IO code. The file /Bugs/Mod/Startup.odc contains the module Startup, hence just Startup in the list of modules to be linked. To make a library that does not use the taucs sparse matrix code remove the modules MathTaucsLib MathTaucsImp from the list of linked modules. 

DevLinker.LinkDynDll
libMultiBUGS.dll := Kernel$+ Files HostFiles StdLoader Math Strings Meta Log Dialog Stores Services 

Console

BugsInterpreter BugsMappers BugsStrings BugsFiles BugsStdInterpreter

BugsMsg BugsRegistry 

BugsScripting

MathSort MathFunc MathMatrix MathSparsematrix  
MathCumulative MathRandnum MathODE MathFunctional MathRungeKutta45 MathAESolver MathIntegrate MathLincon MathTT800 MathDiagmatrix MathBGR MathSmooth

BugsRandnum

GraphRules GraphNodes GraphGrammar GraphLogical GraphStochastic GraphLimits GraphScalar GraphLinkfunc GraphVector GraphParamtrans GraphWeight GraphUnivariate GraphConjugateUV GraphVD GraphCensoringTruncation GraphMultivariate GraphConjugateMV GraphChain GraphGMRF GraphFunctional GraphBlock GraphMessages GraphResources 

MonitorMonitors MonitorSamples MonitorSummary MonitorDeviance MonitorPlugin MonitorRanks MonitorModel

UpdaterUpdaters UpdaterMethods UpdaterUnivariate UpdaterMultivariate UpdaterContinuous
UpdaterConjugateUV UpdaterConjugateMV UpdaterMetropolisUV UpdaterMetropolisMV UpdaterRandWalkUV UpdaterActions UpdaterAuxillary UpdaterEmpty UpdaterMetmonitor UpdaterParallel
UpdaterMessages UpdaterExternal UpdaterResources

GraphConstant GraphPi  GraphMixture GraphStack GraphFlat GraphDummy GraphGeneric GraphMAP GraphPPNorm

LindevCPM LindevCPT LindevCPB LindevCPS LindevCPP LindevCPE LindevCPH LindevCPL486 LindevCPC486 LindevCPV486

DeviancePlugin

BugsVersion BugsNames BugsIndex BugsVariables BugsParser BugsEvaluate BugsNodes BugsOptimize BugsCodegen BugsCPWrite BugsCPCompiler BugsData BugsDeviance BugsGraph BugsPrettyprinter BugsLatexprinter BugsSerialize BugsInterface BugsRobjects BugsInfo BugsTraphandler1 BugsEmbed BugsExternal BugsMAP BugsMessages BugsScripts BugsResources


BugsRectData BugsSplusData

SamplesMonitors SamplesIndex SamplesStatistics SamplesInterface SamplesFormatted 
SamplesEmbed SamplesMessages SamplesResources

SummaryMonitors SummaryIndex SummaryInterface SummaryFormatted SummaryEmbed SummaryMessages SummaryResources

RanksMonitors RanksIndex RanksInterface RanksFormatted RanksEmbed
RanksMessages RanksResources

ModelsMonitors ModelsIndex ModelsInterface ModelsFormatted ModelsEmbed
ModelsMessages ModelsResources

DevianceMonitors DeviancePluginS DeviancePluginD DevianceParents 
DevianceIndex DevianceInterface DevianceFormatted DevianceEmbed DevianceMessages DevianceResources

CorrelInterface CorrelFormatted CorrelEmbed CorrelMessages CorrelResources

GraphAESolver GraphCloglog GraphCut GraphValDiff GraphDensity GraphEigenvals GraphGammap GraphInprod GraphIntegrate GraphInverse GraphKepler GraphLog GraphLogdet GraphLogit GraphProbit GraphProduct GraphRanks GraphSumation GraphTable GraphODEmath GraphODElang GraphODElangRK45 GraphPValue GraphReplicate GraphItermap GraphPiecewise GraphODEBlockL GraphODEBlockLRK45 GraphODEBlockM

GraphBern GraphBinomial GraphCat GraphFounder GraphGeometric GraphHypergeometric GraphMendelian GraphNegbin GraphPoisson GraphRecessive GraphZipf

GraphMultinom

GraphBeta GraphChisqr GraphDbexp GraphExp GraphF GraphGEV GraphGPD GraphGamma GraphGengamma GraphLogistic GraphLognorm GraphNormal GraphPareto GraphPolygene GraphStable GraphT GraphTrapezium GraphTriangle GraphUniform GraphWeibull GraphWeibullShifted

GraphSample GraphPriorNP

GraphDirichlet GraphMVNormal GraphMVT GraphRandwalk GraphRENormal GraphStochtrend
GraphWishart

GraphCoSelection GraphSpline

GraphScalarT GraphVectorT GraphUnivariateT GraphScalartemp1 GraphVectortemp1
GraphUnivariatetemp1

UpdaterAM UpdaterDE UpdaterHamiltonian

UpdaterForward

UpdaterCatagorical UpdaterDescreteSlice

UpdaterICM

UpdaterBeta UpdaterGriddy UpdaterGamma UpdaterMetover UpdaterMetbinomial UpdaterMetnormal UpdaterNormal UpdaterPareto UpdaterPoisson UpdaterRejection UpdaterSCAAR UpdaterSCAM UpdaterSCDE UpdaterSDScale UpdaterSlice UpdaterNaivemet UpdaterMultinomialorder

UpdaterAMblock UpdaterAMNLLS UpdaterChain UpdaterDEblock UpdaterDirichlet
UpdaterDirichletprior UpdaterDirichletpriorRW UpdaterGLM UpdaterGMRF 
UpdaterMultinomial UpdaterMVNormal UpdaterMVNLinear UpdaterWishart UpdaterHamiltonianglm UpdaterMAPproposal UpdaterRandscan

 UpdaterVD UpdaterVDMVN UpdaterVDMVNDescrete UpdaterVDMVNContinuous

UpdaterStage1 UpdaterStage1M UpdaterStage1P

UpdaterUnivariateT UpdaterMultivariateT

SpatialExternal SpatialResources
SpatialBound SpatialCARl1 SpatialCARNormal SpatialCARProper SpatialStrucMVN SpatialDiscKrig SpatialExpKrig SpatialMaternKrig SpatialPoissconv SpatialMVCAR

ReliabilityBS ReliabilityBurrXII ReliabilityBurrX ReliabilityExpPower ReliabilityExpoWeibull ReliabilityExtExp ReliabilityExtendedWeibull ReliabilityFlexibleWeibull ReliabilityGenExp ReliabilityGPWeibull ReliabilityGompertz ReliabilityGumbel ReliabilityInvGauss ReliabilityInvWeibull ReliabilityLinearFailure ReliabilityLogisticExp ReliabilityLogLogistic ReliabilityLogWeibull ReliabilityModifiedWeibull ReliabilitySystem ReliabilityExternal 
ReliabilityResources

PharmacoInputs PharmacoSum PharmacoModel
PharmacoExternal PharmacoResources 

PharmacoPKIVbol1 PharmacoPKIVbol2  PharmacoPKIVbol3 PharmacoPKIVinf1 PharmacoPKIVinf2 PharmacoPKIVinf3 PharmacoPKFO1 PharmacoPKFO2 
PharmacoPKFO3 PharmacoPKZO1 PharmacoPKZO2  PharmacoPKFOlag1
PharmacoPKFOlag2 PharmacoPKZOlag1 PharmacoPKZOlag2 PharmacoPKIVbol1ss
PharmacoPKIVbol2ss PharmacoPKIVbol3ss PharmacoPKIVinf1ss PharmacoPKIVinf2ss
PharmacoPKIVinf3ss PharmacoPKFO1ss PharmacoPKFO2ss PharmacoPKZO1ss
PharmacoPKZO2ss PharmacoPKFOlag1ss PharmacoPKFOlag2ss PharmacoPKZOlag1ss
PharmacoPKZOlag2ss

PKBugsScanners PKBugsNames PKBugsData PKBugsParse PKBugsCovts
PKBugsPriors PKBugsNodes PKBugsTree PKBugsMessages PKBugsResources

DiffExternal DiffResources DiffLotkaVolterra DiffFiveCompModel DiffExponential
DiffChangePoints

MapsMessages MapsResources

Startup BugsCLI BugsC# 

(*	need to have Linux version of Kernel.ocf and HostFile.ocf in correct place	*)

To create  a fully linked ELF shared library version of MultiBUGS for Linux click in the round blob.
Modules in red are BlackBox modules. The other modules, the MultiBUGS modules, should occur in the same order as in the make file and not include any of the Windows only modules. Module LinConsole contains Linux IO code. The file /Bugs/Mod/Startup.odc contains the module Startup, hence just Startup in the list of modules to be linked. To make a library that does not use the taucs sparse matrix code remove the modules MathTaucsLib MathTaucsImp from the list of linked modules. 

DevElfLinker.LinkDynDll
libMultiBUGS.so := LinKernel$+ Files LinHostFiles StdLoader Meta Math Strings Dialog Stores Services

Console LinConsole 

BugsInterpreter BugsMappers BugsStrings BugsFiles BugsStdInterpreter

BugsMsg BugsRegistry

BugsScripting

MathSort MathFunc MathMatrix MathSparsematrix 
MathCumulative MathRandnum MathODE MathFunctional MathRungeKutta45 MathAESolver MathIntegrate MathLincon MathTT800 MathDiagmatrix MathBGR MathSmooth

GraphRules GraphNodes  GraphGrammar GraphLogical GraphStochastic GraphLimits GraphScalar GraphLinkfunc GraphVector GraphParamtrans GraphWeight GraphUnivariate GraphVD GraphCensoringTruncation GraphMultivariate GraphConjugateMV GraphChain GraphGMRF GraphBlock GraphMessages GraphResources 

MonitorsSamples MonitorsSummary MonitorsDeviance MonitorsPlugin MonitorsRanks MonitorsModel

UpdaterUpdaters UpdaterMethods UpdaterUnivariate UpdaterMultivariate UpdaterContinuous
UpdaterMetropolis UpdaterActions UpdaterAuxillary UpdaterEmpty UpdaterMetmonitor UpdaterMessages UpdaterExternal UpdaterResources

GraphConstant GraphPi  GraphMixture GraphStack GraphFlat GraphGeneric GraphClock 
GraphMAP

LindevCPM LindevCPT LindevCPB LindevCPS LindevCPP LindevCPE LindevCPH LindevCPL486 LindevCPC486 LindevCPV486

DeviancePlugin

BugsVersion BugsGrammar BugsNames BugsIndex BugsVariables BugsParser BugsEvaluate BugsNodes BugsOptimize BugsCodegen BugsCPWrite BugsCPCompiler BugsData BugsDeviance  BugsPrettyprinter BugsLatexprinter BugsInterface BugsRobjects BugsInfo BugsTraphandler1 BugsEmbed BugsExternal BugsMLE BugsMessages BugsScripts BugsResources


BugsRectData BugsSplusData

SamplesMonitors SamplesIndex SamplesStatistics SamplesInterface SamplesFormatted 
SamplesEmbed SamplesMessages SamplesResources

SummaryMonitors SummaryIndex SummaryInterface SummaryFormatted SummaryEmbed SummaryMessages SummaryResources

RanksMonitors RanksIndex RanksInterface RanksFormatted RanksEmbed
RanksMessages RanksResources

ModelsMonitors ModelsIndex ModelsInterface ModelsFormatted ModelsEmbed
ModelsMessages ModelsResources

DevianceMonitors DeviancePluginS DeviancePluginD DevianceParents 
DevianceIndex DevianceInterface DevianceFormatted DevianceEmbed DevianceMessages DevianceResources

CorrelInterface CorrelFormatted CorrelEmbed CorrelMessages CorrelResources

GraphCloglog GraphCut GraphValDiff GraphDensity GraphEigenvals GraphGammap GraphInprod GraphInverse GraphKepler GraphLog GraphLogdet GraphLogit GraphProbit GraphProduct GraphRanks GraphSumation GraphTable GraphFunctional GraphODEmath GraphODElang GraphPValue GraphReplicate GraphItermap GraphPiecewise GraphODEBlockL GraphODEBlockM

GraphBern GraphBinomial GraphCat GraphFounder GraphGeometric GraphHypergeometric GraphMendelian GraphNegbin GraphPoisson GraphRecessive GraphZipf

GraphMultinom

GraphBeta GraphChisqr GraphDbexp GraphExp GraphF GraphGEV GraphGPD GraphGamma GraphGengamma GraphLogistic GraphLognorm GraphNormal GraphPareto GraphPolygene GraphStable GraphT GraphTrapezium GraphTriangle GraphUniform GraphWeibull 
GraphWeibullShifted

GraphSample GraphPriorNP

GraphDirichlet GraphMVNormal GraphMVT GraphRENormal GraphStochtrend
GraphWishart

GraphCoSelection GraphSpline

GraphScalarT GraphVectorT GraphUnivariateT GraphScalartemp1 GraphVectortemp1
GraphUnivariatetemp1

UpdaterAM UpdaterDE UpdaterHamiltonian

UpdaterForward

 UpdaterCatagorical UpdaterDescreteSlice

UpdaterICM

UpdaterBeta UpdaterGriddy UpdaterGamma UpdaterMetover UpdaterMetbinomial UpdaterMetnormal UpdaterNormal UpdaterPareto UpdaterPoisson UpdaterRejection UpdaterSCAAR UpdaterSDScale UpdaterSCAM UpdaterSCDE UpdaterSlice UpdaterNaivemet UpdaterMultinomialorder

UpdaterAMblock UpdaterAMNLLS UpdaterChain UpdaterDEBlock UpdaterDirichlet
UpdaterDirichletprior UpdaterDirichletpriorRW UpdaterGLM UpdaterGMRF 
UpdaterMultinomial UpdaterMVNormal UpdaterMVNLinear UpdaterWishart UpdaterHamiltonianglm UpdaterMLE

 UpdaterVD UpdaterVDMVN UpdaterVDMVNDescrete UpdaterVDMVNContinuous

UpdaterStage1 UpdaterStage1M UpdaterStage1P

UpdaterUnivariateT UpdaterMultivariateT

SpatialExternal SpatialResources
SpatialBound SpatialCARl1 SpatialCARNormal SpatialCARProper SpatialStrucMVN SpatialDiscKrig SpatialExpKrig SpatialMaternKrig SpatialPoissconv SpatialMVCAR

ReliabilityBS ReliabilityBurrXII ReliabilityBurrX ReliabilityExpPower ReliabilityExpoWeibull ReliabilityExtExp ReliabilityExtendedWeibull ReliabilityFlexibleWeibull ReliabilityGenExp ReliabilityGPWeibull ReliabilityGompertz ReliabilityGumbel ReliabilityInvGauss ReliabilityInvWeibull ReliabilityLinearFailure ReliabilityLogisticExp ReliabilityLogLogistic ReliabilityLogWeibull ReliabilityModifiedWeibull ReliabilitySystem ReliabilityExternal 
ReliabilityResources

PharmacoInputs PharmacoSum PharmacoModel
PharmacoExternal PharmacoResources 

PharmacoPKIVbol1 PharmacoPKIVbol2  PharmacoPKIVbol3 PharmacoPKIVinf1 PharmacoPKIVinf2 PharmacoPKIVinf3 PharmacoPKFO1 PharmacoPKFO2 
PharmacoPKFO3 PharmacoPKZO1 PharmacoPKZO2  PharmacoPKFOlag1
PharmacoPKFOlag2 PharmacoPKZOlag1 PharmacoPKZOlag2 PharmacoPKIVbol1ss
PharmacoPKIVbol2ss PharmacoPKIVbol3ss PharmacoPKIVinf1ss PharmacoPKIVinf2ss
PharmacoPKIVinf3ss PharmacoPKFO1ss PharmacoPKFO2ss PharmacoPKZO1ss
PharmacoPKZO2ss PharmacoPKFOlag1ss PharmacoPKFOlag2ss PharmacoPKZOlag1ss
PharmacoPKZOlag2ss

PharmacoPKIVbol1M PharmacoPKIVbol2M PharmacoPKIVbol3M PharmacoPKIVinf1M
PharmacoPKIVinf2M PharmacoPKIVinf3M PharmacoPKFO1M PharmacoPKFO2M
PharmacoPKFO3M PharmacoPKZO1M PharmacoPKZO2M PharmacoPKFOlag1M PharmacoPKFOlag2M
PharmacoPKZOlag1M PharmacoPKZOlag2M PharmacoPKIVbol1ssM PharmacoPKIVbol2ssM
PharmacoPKIVbol3ssM PharmacoPKIVinf1ssM PharmacoPKIVinf2ssM PharmacoPKIVinf3ssM
PharmacoPKFO1ssM PharmacoPKFO2ssM PharmacoPKZO1ssM PharmacoPKZO2ssM
PharmacoPKFOlag1ssM PharmacoPKFOlag2ssM PharmacoPKZOlag1ssM
PharmacoPKZOlag2ssM

PharmacoCmds

PKBugsScanners PKBugsNames PKBugsData PKBugsParse PKBugsCovts
PKBugsPriors PKBugsNodes PKBugsTree PKBugsMessages PKBugsResources

DiffExternal DiffResources DiffLotkaVolterra DiffFiveCompModel DiffExponential
DiffChangePoints

MapsMessages MapsResources

Startup BugsCLI BugsC# 


Click the blobs to make WorkerBUGS for multiprocessor updating.

DevLinker.Link
WorkerBUGS.exe := Kernel$ + Files HostFiles  HostPackedFiles StdLoader
1 Bugslogo.ico 2 Doclogo.ico 3 SFLogo.ico 4 CFLogo.ico 5 DtyLogo.ico
6 folderimg.ico 7 openimg.ico 8 leafimg.ico
1 Move.cur 2 Copy.cur 3 Link.cur 4 Pick.cur 5 Stop.cur 6 Hand.cur 7 Table.cur 

Click the blob to bind resources to WorkerBUGS

 DevPacker.PackThis WorkerBUGS.exe :=
gpl-3.0.txt
Bugs/Rsrc/Menus.odc
Doodle/Rsrc/Menus.odc
Maps/Rsrc/Menus.odc
Rsrc/Menus.odc  => System/Rsrc/Menus.odc
Rsrc/About.odc  => System/Rsrc/About.odc


Click the blobs to make MultiBUGS for multiprocessor updating.

DevLinker.Link
MultiBUGS.exe := Kernel$ + Files HostFiles  HostPackedFiles StdLoader
1 Bugslogo.ico 2 Doclogo.ico 3 SFLogo.ico 4 CFLogo.ico 5 DtyLogo.ico
6 folderimg.ico 7 openimg.ico 8 leafimg.ico
1 Move.cur 2 Copy.cur 3 Link.cur 4 Pick.cur 5 Stop.cur 6 Hand.cur 7 Table.cur 

Click the blob to bind resources to MultiBUGS

 DevPacker.PackThis MultiBUGS.exe :=
gpl-3.0.txt
Bugs/Rsrc/MenusMulti.odc => Bugs/Rsrc/Menus.odc
Doodle/Rsrc/Menus.odc
Maps/Rsrc/Menus.odc
Rsrc/MenusMulti.odc  => System/Rsrc/Menus.odc
Rsrc/AboutMulti.odc  => System/Rsrc/About.odc
Graph/Sym/Logical.osf
Graph/Sym/Nodes.osf
Graph/Sym/Rules.osf
Graph/Sym/Scalar.osf
Graph/Sym/Stochastic.osf
Graph/Sym/Stack.osf
Math/Sym/Func.osf
System/Sym/Math.osf
System/Sym/Stores.osf

