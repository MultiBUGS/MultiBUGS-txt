	Writing OpenBUGS Extensions


Contents

	Introduction
	
	The Graph hierarchy
	
	The Updater hierarchy
	
	
Introduction [top]

The OpenBUGS software is a framework that handles the logic of interaction between several class hierarchies. One class hierarchy, the Graph hierarchy, is used to build descriptions of complex Bayesian statistical models. A second, the Updater hierarchy, implements sampling algorithms for variables in the statistical model. New types of logical functions and distributions can be added to the BUGS language by implementing new classes in the Graph hierarchy. New
sampling algorithms can be added to Updater hierarchy.

Component Pascal uses the key word ABSTRACT to denote abstract classes, that is classes used as templates for other concrete classes. The same key word is also used to denote abstract methods. The OpenBUGS software has been designed so that only abstract classes are exported, made visible outside the module in which they are defined. Concrete classes are completely enclosed in modules and only an object, the factory object, is exported  to allow the creation of instances of the concrete class. This means that concrete classes can only be derived from abstract classes. OpenBUGS also follows the convention that method are either abstract or final (can not be modified). The only exception to this is some methods are empty (denoted by key word EMPTY) that is their implementation in concrete classes is optional. Writing a new class in OpenBUGS involves extending an abstract class such that all the abstract methods are implemented and implementing any empty methods that are relevent. The Component Pascal compiler checks that a concrete class has no abstract methods and the browser tool (Extension Interface option) can be used to check which methods of a class need implementing.


The Graph hierarchy	 [top]

OpenBUGS describes statistical models in terms of a graph. Each node in the graph is represented by an object belonging to one of the classes in the Graph hierarchy. Each of these classes is descendant from the class GraphNodes.Node (that is the class Node in module GraphNodes). The class GraphNodes.Node is first specialized to GraphLogial.Node to represent logical function and GraphStochastic.Node to represent distributions in the BUGS language. The class GraphLogical.Node is futher specialized to GraphScalar.Node to represent scalar valued logical functions eg the logit link and to GraphVector.Node to represent vector (or matrix) valued logical funtions eg matrix inversion. The class GraphStohcastic.Node is specialized to GraphUnivariate.Node to represent univariate distributions eg the beta distribution and to GraphMultivariate.Node to represent multivariate distributions. The class GraphMultivariate.Node is furher specialized to GraphConjugateMV to represent multivariate distributions with conjugate properties eg the multivariate normal and GraphChain.Node to represent chain graph distributions eg the spatial CAR model. These node classes are abstract, that is they have unimplemented methods, objects of these type can not be created however they can be futher specialised to concrete classes, where all the methods are implemented, which can be used to create nodes in the graph describing the statistical model.. 

To ease the writing of new logical nodes some additional abstract classes have been developed. These class implement as many of the methods of the logical node classes as have reasonable default behavoir and leave the developer with the minimum number of methods to implement. These semi implemented classes are in modules GraphScalarT and GraphVectorT.
Using these classes does not allow as full an exploitation of functionality of the OpenBUGS software as extending the classes in GraphScalar and GraphVector would but is much less work and adequate for some applications.Their use is illustrated in modules GraphScalartemp1 (implementing the calculation of the harmonic mean) and GraphVectortemp1 (implementing the calculation of an integer power of a square matrix). Modules GraphScalartemp1 and GraphVectortemp1 can be used as templates for developing other logical nodes. Similliarly to ease the writing of new univariate stochastic nodes some additional abstract classes have been developed. These classes are in module GraphUnivariateT and their use is illustrated in module GraphUnivariatetemp1 (implementing the normal distribution but with no knowedge of conjugacy).

Once a new node type has been implemented OpenBUGS must be told what name to use for this new node type in the BUGS language and the module where it is implemented. This information is stored in the configuration module BugsExternal. This module contains pairs of string:, the first one being the name of the node type and the second the name of the module which implements the new node type followed by a period followed by the "install procedure" usually called Install.


The Updater hierarchy	 [top]

There is also a hierarchy of updater classes to implement the updating or sampling algorithms. The sampling algorithm for each node or block of nodes in the graphical model is contained in an object of the appropiate class. These updater classes use the graphical model to efficiently calculate the required conditional probabilities. All the updater objects for the graphical model are stored in a linked list sorted according to the topological order of the ascociated nodes in the graphical model (nodes in the graphical model purely used for prediction have by convention their depth in the graph multiplied by minus one). Using this ordering of updaters MCMC can be used for nodes with observed offspring and forward sampling for the nodes with no observed offspring. One complete update of the model can be carried out by traversing this linked list and calling the appropiate sampling method.

All updater classes are derived from class UpdaterUpdaters.Updater. This class is first specialised into univariate and multivariate (block) updaters. The univariate class is specialized to the continous updaters. The multivariate class is specialized to conjugate multivariate updaters. Writing a new updater class involves specializing one of the folllowing classes: UpdaterUnivariate.Updater, UpdaterContinuous.Updater, UpdaterMultivariate.Updater and UpdaterMultivariate.Conjugate. The methods that must be implemented when these classes are made concrete can be found by selecting the modules name followed by the class name (eg "UpdaterMultivariate.Conjugate") and then picking "Extension Interface" from the Info menu. Concrete updaters are made by factory objects. Factory objects have a New method which takes one argument the node in the graphical model for which an updater is required. This New method can return a new updater object if this is a correct and desirable thing to do otherwise it returns a nil pointer. 

Two template modules UpdaterUnivariateT and UpdaterMultivariateT have been writen to ease the writing of new univariate and block updaters.

Once a new updater has been written OpenBUGS must be told in which module it is implemented. This information is stored in the configuration module UpdaterExternal, which contains a list of install procedures to install updater algorithms. A list of installed updater algorithms is displayed in the Updater options tool of the Model menu.
