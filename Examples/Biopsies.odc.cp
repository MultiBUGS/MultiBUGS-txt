	Biopsies: discrete variable
		 latent class model

Spiegelhalter and Stovin (1983) presented data on repeated biopsies of transplanted hearts, in which a total of 414 biopsies had been taken at 157 sessions.  Each biopsy was graded on evidence of rejection using a  4 category scale of none (O), minimal (M), mild (+) and  moderate-severe (++).  Part of the data is shown below.


	Combination	Multinomial response	Session frequency
	_______________________________________________________
	  O  O	(2, 0, 0, 0)	12
	  M  M  O	(1, 2, 0, 0)	10
	  +  +   O	(1, 0, 2, 0)	17
	 ++ ++ ++	(0, 0, 0, 3)	5

The sampling procedure may not detect the area of maximum rejection, which is considered the true underlying state at the time of the session and denoted ti --- the underlying probability distribution  of the four true states is denoted by the vector p. It is then assumed that each of the observed biopsies are conditionally independent given this truestate with the restriction that there are no`false positives': i.e. one cannot observe a biopsy worse than the true state.  We then have the sampling model	

	bi  ~  Multinomial(eti, ni)

	ti  ~  Categorical(p)


where bi denotes the multinomial response at session i where ni biopsies have been taken, and ejk is the probability that a true state ti = j generates a biopsy in state k.The no-false-positive restriction means that e12 = e13 = e14 = e23 = e24 = e34 = 0. Spiegelhalter and Stovin (1983) estimated the parameters ej and p using the EM algorithm, with some smoothing to avoid zero estimates.

The appropriate graph is shown below, where the role of the true state ti is simply to pick the appropriate row from the 4 x 4 error matrix e. Here  the probability vectors ej (j = 1,...,4) and p are assumed to have uniform priors on the unit simplex, which correspond to Dirichlet priors with all parameters being 1.

The BUGS code for this model is given below. No initial values are provided for the latent states, since the forward sampling procedure will find a configuration of starting values that is compatible with the expressed constraints. We also note the apparent ``cycle'' in the graph created by the expression nbiops[i] <- sum(biopsies[i,]). This will lead Such ``cycles'' are permitted provided that they are only data transformation statements, since this does not affect the essential probability model. 


	model
	{
		for (i in 1 : ns){
			nbiops[i] <- sum(biopsies[i, ])  
			true[i]  ~ dcat(p[])
			biopsies[i, 1 : 4]  ~ dmulti(error[true[i], ], nbiops[i])
		}
		error[2,1 : 2] ~ ddirich(prior[1 : 2])
		error[3,1 : 3] ~ ddirich(prior[1 : 3])
		error[4,1 : 4] ~ ddirich(prior[1 : 4])
		p[1 : 4] ~ ddirich(prior[]);     # prior for p
	}

 
Data ( click to open )

Inits for chain 1 		Inits for chain 2	( click to open )

Results 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	error[2,1]	0.5875	0.5882	0.06656	0.001323	0.4549	0.7153	1001	20000	2529
	error[2,2]	0.4125	0.4118	0.06656	0.001323	0.2847	0.5451	1001	20000	2529
	error[3,1]	0.3416	0.3404	0.0449	4.824E-4	0.2573	0.4344	1001	20000	8661
	error[3,2]	0.03708	0.03468	0.01763	1.827E-4	0.009259	0.0781	1001	20000	9308
	error[3,3]	0.6213	0.6225	0.04684	4.999E-4	0.526	0.71	1001	20000	8779
	error[4,1]	0.0992	0.09293	0.04295	3.783E-4	0.03287	0.197	1001	20000	12891
	error[4,2]	0.02187	0.01467	0.02251	2.327E-4	5.109E-4	0.08306	1001	20000	9361
	error[4,3]	0.2055	0.2002	0.06041	6.076E-4	0.1036	0.3371	1001	20000	9886
	error[4,4]	0.6734	0.6774	0.07332	7.622E-4	0.5202	0.8042	1001	20000	9253
	p[1]	0.1527	0.1537	0.04979	0.001058	0.04767	0.248	1001	20000	2214
	p[2]	0.3116	0.3074	0.05514	0.001129	0.2151	0.4316	1001	20000	2384
	p[3]	0.3886	0.3882	0.04336	4.073E-4	0.3048	0.4746	1001	20000	11335
	p[4]	0.1471	0.1453	0.02971	2.401E-4	0.09454	0.2103	1001	20000	15309

