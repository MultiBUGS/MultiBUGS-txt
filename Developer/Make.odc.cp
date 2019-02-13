 Compiling OpenBUGS

This document contains a list of source code modules which constitute the OpenBUGS software.

To produce an executable version of OpenBUGS the compiled form of some of these modules must be linked with certain compiled modules of Oberonmicrosystems BlackBox software. See the Linking document for details.

List of OpenBUGS modules:

DevCompiler.CompileThis

HostMenus

Environment MPI MPIlib MPIimp MPIworker MPImaster

MathSort MathFunc MathMatrix MathSparsematrix MathTaucsLib MathTaucsImp
MathCumulative MathRandnum MathODE MathFunctional MathRungeKutta45 MathAESolver MathIntegrate MathTT800 MathDiagmatrix MathBGR MathSmooth

BugsMsg BugsRegistry BugsMappers BugsFiles

BugsInterpreter BugsScripts BugsScripting

BugsRandnum

GraphRules GraphNodes GraphGrammar GraphLogical GraphStochastic GraphScalar GraphMemory GraphLinkfunc GraphVector GraphWeight GraphUnivariate GraphMultivariate GraphConjugateUV GraphConjugateMV GraphChain GraphMRF GraphUVMRF  GraphUVGMRF GraphMVGMRF GraphVD GraphVDDescrete GraphVDContinuous
GraphFunctional GraphMessages GraphResources

GraphDummy GraphDummyMV GraphSentinel GraphDeviance

MonitorMonitors MonitorSamples MonitorSamplesDisc MonitorSummary MonitorDeviance MonitorPlugin MonitorRanks MonitorModel

UpdaterUpdaters UpdaterMethods UpdaterUnivariate UpdaterMultivariate UpdaterContinuous
UpdaterConjugateMV UpdaterMetropolisUV UpdaterMetropolisMV UpdaterRandWalkUV UpdaterAuxillary UpdaterEmpty UpdaterActions UpdaterMessages UpdaterExternal UpdaterResources

GraphConstant GraphMixture GraphMAP

DeviancePlugin

BugsVersion BugsNames BugsIndex BugsVariables BugsParser BugsEvaluate BugsCodegen BugsOptimize BugsCPWrite BugsCPCompiler BugsNodes BugsData BugsGraph BugsPrettyprinter BugsLatexprinter BugsSerialize BugsInterface BugsRobjects
BugsTraphandler1 BugsExternal BugsMAP BugsMessages BugsResources  BugsPartition BugsParallel  BugsInfo BugsComponents

BugsRectData BugsSplusData

SamplesMonitors SamplesIndex SamplesStatistics SamplesInterface SamplesFormatted
SamplesMessages SamplesResources

SummaryMonitors SummaryIndex SummaryInterface SummaryFormatted SummaryMessages SummaryResources

RanksMonitors RanksIndex RanksInterface RanksFormatted
RanksMessages RanksResources

ModelsMonitors ModelsIndex ModelsInterface ModelsFormatted
ModelsMessages ModelsResources

DevianceMonitors DeviancePluginS DevianceParents
DevianceIndex DevianceInterface DevianceFormatted DevianceMessages DevianceResources

CorrelInterface CorrelFormatted CorrelMessages CorrelResources

BugsMaster

GraphPiecewise GraphODEmath GraphODElang GraphODEBlockL GraphODEBlockM

GraphAESolver GraphCloglog GraphCut GraphHalf GraphDensity GraphGammap GraphInprod GraphIntegrate GraphKepler GraphLog GraphLogdet GraphLogit GraphMapped GraphProbit GraphProduct GraphRanks GraphSumation GraphTable GraphPValue GraphReplicate GraphPi GraphValDiff

GraphEigenvals GraphInverse GraphItermap GraphODElangRK45 GraphODEBlockLRK45 
GraphStick

GraphBern GraphBinomial GraphCat GraphCat2 GraphFounder GraphGeometric GraphHypergeometric GraphMendelian GraphNegbin GraphPoisson GraphRecessive GraphZipf

GraphMultinom

GraphBeta GraphChisqr GraphDbexp GraphExp GraphF GraphFlat GraphGeneric GraphGEV GraphGPD GraphGamma GraphGengamma GraphHalfT GraphHazard GraphLogistic GraphLognorm GraphNormal GraphPriorNP GraphPareto GraphPolygene GraphStable GraphT GraphTrapezium GraphTriangle GraphUniform GraphWeibull GraphWeibullShifted

GraphDirichlet GraphGPprior GraphMVNormal GraphMVT GraphRENormal GraphRandwalk GraphSample GraphStochtrend GraphWishart GraphFlexWishart

GraphScalarT GraphVectorT GraphUnivariateT GraphScalartemp1 GraphVectortemp1
GraphUnivariatetemp1

GraphCoSelection GraphSpline GraphSplinecon GraphSplinescalar

UpdaterAM UpdaterDE UpdaterHamiltonian 

UpdaterForward

UpdaterCatagorical UpdaterDescreteSlice UpdaterMetbinomial UpdaterSlicebase

UpdaterICM

UpdaterBeta UpdaterGamma  UpdaterGriddy UpdaterMetover UpdaterMetnormal UpdaterNaivemet UpdaterNormal UpdaterPareto UpdaterPoisson UpdaterRandEffect UpdaterRejection UpdaterSCAAR UpdaterSCDE UpdaterSDScale UpdaterSlice UpdaterSlicegamma UpdaterStage1 UpdaterStage1P UpdaterVD 

UpdaterAMblock UpdaterDEblock UpdaterDelayedDirectional1D UpdaterDirichlet
UpdaterElliptical UpdaterEllipticalD UpdaterGLM UpdaterMRFConstrain UpdaterGMRF UpdaterGMRFess  UpdaterMAPproposal UpdaterMultinomial UpdaterMVNormal UpdaterMVNLinear UpdaterStage1M UpdaterVDMVN UpdaterVDMVNContinuous UpdaterVDMVNDescrete UpdaterWishart

UpdaterUnivariateT UpdaterMultivariateT

SpatialExternal SpatialResources SpatialUVCAR
SpatialBound SpatialCARl1 SpatialCARNormal SpatialCARProper SpatialDiscKrig SpatialExpKrig SpatialMaternKrig SpatialPoissconv SpatialMVCAR

ReliabilityBS ReliabilityBurrXII ReliabilityBurrX ReliabilityExpPower ReliabilityExpoWeibull ReliabilityExtExp ReliabilityExtendedWeibull ReliabilityFlexibleWeibull ReliabilityGenExp ReliabilityGPWeibull ReliabilityGompertz ReliabilityGumbel ReliabilityInvGauss ReliabilityInvWeibull ReliabilityLinearFailure ReliabilityLogisticExp ReliabilityLogLogistic ReliabilityLogWeibull ReliabilityModifiedWeibull ReliabilitySystem ReliabilityWrapper ReliabilityExternal ReliabilityResources

PharmacoInputs PharmacoModel PharmacoExternal PharmacoResources

PharmacoPKIVbol1 PharmacoPKIVbol2 PharmacoPKIVbol3 PharmacoPKIVinf1 PharmacoPKIVinf2 PharmacoPKIVinf3 PharmacoPKFO1 PharmacoPKFO2
PharmacoPKFO3 PharmacoPKZO1 PharmacoPKZO2 PharmacoPKFOlag1
PharmacoPKFOlag2 PharmacoPKZOlag1 PharmacoPKZOlag2 PharmacoPKIVbol1ss
PharmacoPKIVbol2ss PharmacoPKIVbol3ss PharmacoPKIVinf1ss PharmacoPKIVinf2ss
PharmacoPKIVinf3ss PharmacoPKFO1ss PharmacoPKFO2ss PharmacoPKZO1ss
PharmacoPKZO2ss PharmacoPKFOlag1ss PharmacoPKFOlag2ss PharmacoPKZOlag1ss
PharmacoPKZOlag2ss PharmacoSum

PKBugsScanners PKBugsNames PKBugsData PKBugsParse PKBugsCovts
PKBugsPriors PKBugsNodes PKBugsTree PKBugsMessages PKBugsResources

DiffExternal DiffResources

DiffLotkaVolterra DiffFiveCompModel DiffExponential DiffChangePoints DiffHPS_V2_FB

MapsMessages MapsResources

BugsDialog

BugsStartup Init

UpdaterSettings

DevDebug

BugsCmds BugsBatch BugsConfig 

DoodleNodes DoodlePlates DoodleModels DoodleMenus DoodleDialog DoodleViews DoodleParser DoodleCmds DoodleMessages DoodleResources

BugsInfodebug BugsDocu BugsSearch

PlotsAxis PlotsViews PlotsDialog PlotsNomaxis PlotsStdaxis PlotsEmptyaxis

SamplesViews SamplesPlots SamplesCmds
SamplesCorrelat SamplesDensity SamplesDiagnostics SamplesHistory SamplesQuantiles SamplesTrace SamplesJumpdist SamplesAccept

SummaryCmds

RanksDensity RanksCmds

ModelsCmds

DevianceCmds

CompareViews CompareBoxplot CompareCaterpillar CompareModelFit CompareScatter CompareDenstrip CompareCmds

CorrelBivariate CorrelMatrix CorrelPlots CorrelCmds

MapsMap MapsImporter MapsIndex MapsAdjacency MapsViews MapsViews1 MapsCmds MapsArcinfo MapsEpimap MapsSplus

PKBugsCmds

StdCmds1

BugsC

TestScript

BugsDistribute

BugsPackage

MathSeeds

ParallelTraphandler ParallelRandnum ParallelActions ParallelWorker ParallelDebug

ParallelDebug 

PlotsWindows

BugsTraphandler









