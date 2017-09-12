	Asia: expert system

Evidence propagation

Lauritzen and Spiegelhalter (1988) introduce a fictitious "expert system" representing the diagnosis of a patient presenting to a chest clinic, having just come back from a trip to Asia and showing dyspnoea (shortness-of-breath). The BUGS code is shown  below and the conditional probabilities used are given in Lauritzen and Spiegelhalter (1988). Note the use of max  to do the logical-or. The dcat distribution is used to sample values with domain (1,2) with probability distribution given by the relevant entries in the conditional probability
tables. 
  

	model
	{
		smoking ~ dcat(p.smoking[1:2])
		tuberculosis ~ dcat(p.tuberculosis[asia,1:2])
		lung.cancer ~ dcat(p.lung.cancer[smoking,1:2])
		bronchitis ~ dcat(p.bronchitis[smoking,1:2])
		either <- max(tuberculosis,lung.cancer)
		xray ~ dcat(p.xray[either,1:2])
		dyspnoea ~ dcat(p.dyspnoea[either,bronchitis,1:2])
	}


Data ( click to open )

Inits for chain 1		Inits for chain 2	( click to open )

Results 
		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	either	1.183	1.0	0.3863	9.705E-4	1.0	2.0	5001	200000	158429
	lung.cancer	1.1	1.0	0.2997	7.383E-4	1.0	2.0	5001	200000	164750
	smoking	1.627	2.0	0.4835	0.001165	1.0	2.0	5001	200000	172159
	tuberculosis	1.088	1.0	0.2827	7.118E-4	1.0	2.0	5001	200000	157745
	xray	1.22	1.0	0.4142	9.834E-4	1.0	2.0	5001	200000	177394


