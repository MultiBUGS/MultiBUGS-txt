 Compiling OpenBUGS

This document contains a list of source code modules which constitute the OpenBUGS software.

To produce an executable version of OpenBUGS the compiled form of some of these modules must be linked with certain compiled modules of Oberonmicrosystems BlackBox software. See the Linking document for details.

List of OpenBUGS modules:

DevCompiler.CompileThis 

MPI MPImslib MPImsimp MPIworker

MathSort MathFunc MathMatrix MathSparsematrix MathTaucsLib MathTaucsImp
MathCumulative MathRandnum MathODE MathFunctional MathRungeKutta45 MathAESolver MathIntegrate MathLincon MathTT800 MathDiagmatrix MathBGR MathSmooth

BugsMsg BugsRegistry BugsMappers BugsFiles 

BugsInterpreter BugsScripts BugsScripting

BugsRandnum

GraphRules GraphNodes GraphGrammar GraphLogical GraphStochastic GraphLimits GraphScalar GraphLinkfunc GraphVector GraphParamtrans GraphWeight GraphUnivariate GraphConjugateUV GraphVD GraphCenTrunc GraphMultivariate GraphConjugateMV GraphChain GraphMRF GraphFunctional GraphMessages GraphResources GraphFlat
GraphDeviance

MonitorMonitors MonitorSamples MonitorSummary MonitorDeviance MonitorPlugin MonitorRanks MonitorModel

UpdaterUpdaters UpdaterMethods UpdaterUnivariate UpdaterMultivariate UpdaterContinuous
UpdaterConjugateUV UpdaterConjugateMV UpdaterMetropolisUV UpdaterMetropolisMV UpdaterRandWalkUV UpdaterAuxillary UpdaterEmpty UpdaterActions UpdaterParallel UpdaterMessages UpdaterExternal UpdaterResources

GraphConstant GraphSentinel GraphPi  GraphHalf GraphMixture GraphDummy GraphGeneric GraphMAP  

DeviancePlugin

BugsVersion BugsNames BugsIndex BugsVariables BugsParser BugsEvaluate BugsCodegen BugsOptimize BugsCPWrite BugsCPCompiler BugsNodes BugsData  BugsGraph BugsPrettyprinter BugsLatexprinter BugsSerialize BugsInterface BugsRobjects 
BugsBlueDiamonds BugsInfo BugsTraphandler1 BugsExternal BugsMAP BugsMessages BugsResources BugsComponents

BugsRectData BugsSplusData

SamplesMonitors SamplesIndex SamplesStatistics SamplesInterface SamplesFormatted 
SamplesMessages SamplesResources

SummaryMonitors SummaryIndex SummaryInterface SummaryFormatted SummaryMessages SummaryResources

RanksMonitors RanksIndex RanksInterface RanksFormatted
RanksMessages RanksResources

ModelsMonitors ModelsIndex ModelsInterface ModelsFormatted 
ModelsMessages ModelsResources

DevianceMonitors DeviancePluginS DeviancePluginD DevianceParents 
DevianceIndex DevianceInterface DevianceFormatted DevianceMessages DevianceResources

CorrelInterface CorrelFormatted CorrelMessages CorrelResources

BugsMaster

GraphAESolver GraphCloglog GraphCut GraphValDiff GraphDensity GraphEigenvals GraphGammap GraphInprod GraphIntegrate GraphInverse GraphKepler GraphLog GraphLogdet GraphLogit GraphProbit GraphProduct GraphRanks GraphSumation GraphTable GraphODEmath GraphODElang GraphODElangRK45 GraphPValue GraphReplicate GraphItermap GraphPiecewise GraphODEBlockL GraphODEBlockLRK45 GraphODEBlockM

GraphBern GraphBinomial GraphCat GraphCat2 GraphFounder GraphGeometric GraphHypergeometric GraphMendelian GraphNegbin GraphPoisson GraphRecessive GraphZipf

GraphMultinom

GraphBeta GraphChisqr GraphDbexp GraphExp GraphF GraphGEV GraphGPD GraphGamma GraphGengamma GraphHalfT GraphLogistic GraphLognorm GraphNormal GraphPareto GraphPolygene GraphStable GraphT GraphTrapezium GraphTriangle GraphUniform GraphWeibull GraphWeibullShifted

GraphSample GraphPriorNP

GraphDirichlet GraphMVNormal GraphMVT GraphUVGMRF GraphRENormal GraphRandwalk GraphStochtrend GraphWishart GraphFlexWishart

GraphCoSelection GraphSpline

GraphScalarT GraphVectorT GraphUnivariateT GraphScalartemp1 GraphVectortemp1
GraphUnivariatetemp1

UpdaterAM UpdaterDE UpdaterHamiltonian

UpdaterForward

UpdaterCatagorical UpdaterDescreteSlice UpdaterMetbinomial

UpdaterICM

UpdaterBeta UpdaterGriddy UpdaterGamma UpdaterMetover  UpdaterMetnormal UpdaterNormal UpdaterPareto UpdaterPoisson UpdaterRejection UpdaterSCAAR UpdaterSCDE UpdaterSDScale UpdaterSlice UpdaterNaivemet UpdaterAMblock UpdaterChain UpdaterDEblock UpdaterDirichlet UpdaterDirichletprior UpdaterDirichletpriorRW UpdaterGLM UpdaterGMRF UpdaterMultinomial UpdaterMVNormal UpdaterMVNLinear UpdaterWishart UpdaterMRFConstrain UpdaterHamiltonianglm UpdaterMAPproposal UpdaterDelayedDirectional1D
UpdaterRandscan

UpdaterVD UpdaterVDMVN UpdaterVDMVNDescrete UpdaterVDMVNContinuous

UpdaterStage1 UpdaterStage1M UpdaterStage1P

UpdaterUnivariateT UpdaterMultivariateT

SpatialExternal SpatialResources
SpatialBound SpatialUVCAR SpatialCARl1 SpatialCARNormal SpatialCARProper SpatialStrucMVN SpatialDiscKrig SpatialExpKrig SpatialMaternKrig SpatialPoissconv SpatialMVCAR

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
DiffChangePoints DiffHPS_V2_FB

MapsMessages MapsResources

BugsStartup 

BugsDialog

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

BugsC

StdCmds1

TestScript

BugsDistribute 

MathSeeds 

ParallelTraphandler ParallelRandnum ParallelActions ParallelDebug ParallelWorker 








