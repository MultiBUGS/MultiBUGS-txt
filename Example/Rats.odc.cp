	Rats: a normal hierarchical model

This example is taken from  section 6 of Gelfand et al (1990), and concerns 30 young rats whose weights were measured weekly for five weeks. Part of the data is shown below, where Yij is the weight of the ith rat measured at age xj.   


		Weights Yij of rat i on day xj
		 xj = 8	15	22	29 	36 
	__________________________________		
	Rat 1	151	199	246	283	320
	Rat 2	145	199	249	293	354
	.......
	Rat 30	153	200	244	286	324 


A plot of the 30 growth curves suggests some evidence of downward curvature.

The model is essentially a random effects linear growth curve

	Yij ~  Normal(ai + bi(xj - xbar), tc)

	ai  ~  Normal(ac, ta)

	bi  ~  Normal(bc, tb)

where xbar = 22, and t represents the precision (1/variance) of a normal distribution. We note the absence of a parameter representing correlation between ai and bi unlike in Gelfand et al 1990. However, see the Birats example in Volume 2 which does explicitly model the covariance between ai   and bi. For now, we standardise the xj's around their mean to reduce dependence between ai and bi in their likelihood: in fact for the full balanced data, complete independence is achieved. (Note that, in general, prior independence does not force the posterior distributions to be independent).

ac , ta , bc , tb , tc are given independent ``noninformative'' priors.  Interest particularly focuses on the intercept at zero time (birth), denoted a0 = ac - bc xbar.  

Graphical model for rats example:



BUGS language for rats example:


	model
	{
		for( i in 1 : N ) {
			for( j in 1 : T ) {
				Y[i , j] ~ dnorm(mu[i , j],tau.c)
				mu[i , j] <- alpha[i] + beta[i] * (x[j] - xbar)
			}
			alpha[i] ~ dnorm(alpha.c,alpha.tau)
			beta[i] ~ dnorm(beta.c,beta.tau)
		}
		tau.c ~ dgamma(0.001,0.001)
		sigma <- 1 / sqrt(tau.c)
		alpha.c ~ dnorm(0.0,1.0E-6)	   
		alpha.tau ~ dgamma(0.001,0.001)
		beta.c ~ dnorm(0.0,1.0E-6)
		beta.tau ~ dgamma(0.001,0.001)
		alpha0 <- alpha.c - xbar * beta.c	
	}

Note the use of a very flat but conjugate  prior for the population effects: a locally uniform prior could also have been used.

Data ( click to open )

Inits for chain 1 		Inits for chain 2	( click to open )


(Note: the response data (Y) for the rats example can also be found in the file ratsy.odc in rectangular format. The covariate data (x) can be found in S-Plus format in file ratsx.odc. To load data from each of these files, focus the window containing the open data file before clicking on "load data" from the "Specification" dialog.)


Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha0	106.6	106.6	3.639	0.02987	99.33	113.7	1001	20000	14840
	beta.c	6.186	6.186	0.1081	9.707E-4	5.975	6.399	1001	20000	12389
	sigma	6.099	6.065	0.4681	0.005405	5.265	7.111	1001	20000	7500

These results may be compared with Figure 5 of Gelfand et al 1990 --- we note that the mean gradient of independent fitted straight lines is 6.19.

Gelfand et al 1990 also consider the problem of missing data, and delete the last observation of cases 6-10, the last two from 11-20, the last 3 from 21-25 and the last 4 from 26-30.  The appropriate data file is obtained by simply replacing data values by NA (see below). The model specification is unchanged, since the distinction between observed and unobserved quantities is made in the data file and not the model specification.

Data ( click to open )

Gelfand et al 1990 focus on the parameter estimates and the predictions for the final 4 observations on rat 26. These predictions are obtained automatically in BUGS by monitoring the relevant Y[] nodes. The following estimates were obtained:

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	Y[26,2]	204.7	204.6	8.764	0.08534	187.6	221.9	1001	20000	10547
	Y[26,3]	250.0	250.1	10.24	0.1229	229.9	270.2	1001	20000	6936
	Y[26,4]	295.5	295.5	12.65	0.1803	270.5	320.1	1001	20000	4923
	Y[26,5]	341.0	341.0	15.47	0.2252	310.9	371.3	1001	20000	4715
	beta.c	6.576	6.574	0.148	0.002555	6.291	6.874	1001	20000	3356

We note that our estimate 6.58 of bc is substantially greater than that shown in Figure 6 of  Gelfand et al 1990.  However, plotting the growth curves indicates some curvature with steeper gradients at the beginning: the mean of the estimated gradients of the reduced data is 6.66, compared to 6.19 for the full data.  Hence we are inclined to believe our analysis.  The observed weights for rat 26 were 207, 257, 303 and 345, compared to our predictions of 204, 250, 295 and 341.