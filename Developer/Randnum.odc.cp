	Random Number Generators 



The module MathRandnum defines an abstract class Generator for generating pseudo random numbers with a uniform distribution on [0,1]. Two implementations of the Generator class are given in modules MathLincon (a linear congruance algorithm) and MathTT800 (a twisted GFSR algorithm). The module MathRandnum implements several non-uniform random number generators using an object of class Generator to generate random uniforms. The state of these non-uniform random number generators only depends on the state of the Generator object used. MathRandnum contains procedures for manipulating the current Generator object.

The procedures MathLincon.Install and MathTT800.Install cause the new Generator object in MathRandnum to use a linear congruance and a twisted GFSR algorithm respectively. The procedure NewGenerator in MathRandnum  will create a new Generator object. Options for creating Generator objects with internal state set such that samples will be independent of each other have been developed for MathTT800. The small program in seeds gives seeds for the MathT800 updater that belong to random deviates 5*10^11 iterations apart. By default OpenBUGS uses a single Generator object for all the chains it simulates. This can be altered in the Compile options dialog of the Model menu so that each chain uses a seperate Generator object. Using a different Generator object for each chain will be needed in any parallel implementation of OpenBUGS.

A small test model -- model{  u ~ dunif(0, 1) } -- was updated for 10 000 000 iterations to give a quick check of the MathLincon and MathTT800 algorithms, with the following results:

mean	sd	2.5%	5%	10%	25%	50%	75%	90%	95%	97.5%
0.5001	0.2887	0.02501	0.05004	0.1	0.25	0.5001	0.7501	0.9	0.9499	0.975	
0.4999	0.2887	0.02491	0.04994	0.09993	0.2499	0.4998	0.7499	0.9	0.9499	0.975

Both random number generators appear to work well, but we have little experience with the MathTT800 algorithm and have had some difficulty coding this algorithm.
