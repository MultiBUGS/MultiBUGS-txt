	Kidney: Weibull regression with
			random efects

McGilchrist and Aisbett (1991) analyse time to first and second recurrence of infection in kidney patients on dialysis using a Cox model with a multiplicative frailty parameter for each individual. The risk variables considered are age, sex and underlying disease (coded other, GN, AN and PKD). A portion of the data are shown below.



	Patient	Recurrence	Event	Age at	Sex	Disease
	Number	time t	(2 = cens)	time t	(1 = female)	(0 = other; 1 = GN
						 2 = AN; 3 = PKD)
	______________________________________________________________________
	1	8,16	1,1	28,28	0	0
	2	23,13	1,2	48,48	1	1
	3	22,28	1,1	32,32	0	0
	4	447,318	1,1	31,32	1	0
	.....
	35	119,8	1,1	22,22	1	1
	36	54,16	2,2	42,42	1	1
	37	6,78	2,1	52,52	1	3
	38	63,8	1,2	60,60	0	3


We have analysed the same data assuming a parametric Weibull distribution for the survivor function, and including an additive random effect bi for each patient in the exponent of the hazard model as follows

	tij  ~  Weibull(r, mij)    i = 1,...,38;  j = 1,2
	
	logmij  = a + bageAGEij + bsexSEXi + bdisease1DISEASEi1 +
					bdisease2DISEASEi2 + bdisease3DISEASEi3 + bi
					
	bi  ~ Normal(0, t)
	
where AGEij is a continuous covariate, SEXi is a 2-level factor and DISEASEik (k = 1,2,3) are dummy variables representing the 4-level factor for underlying disease. Note that the the survival distribution is a truncated Weibull for censored observations as discussed in the mice example. The regression coefficients and the precision of the random effects t are given independent ``non-informative'' priors, namely	

	bk  ~  Normal(0, 0.0001)
	
	t  ~ Gamma(0.0001, 0.0001)
	
The shape parameter of the survival distribution r is given a Gamma(1, 0.0001) prior which is slowly decreasing on the positive real line.

The graphical model and BUGS language are given below. 


Graphical model for kidney example:	


BUGS language for kidney example

	model
	{
		for (i in 1 : N) {
			for (j in 1 : M) {
	# Survival times bounded below by censoring times:
				t[i,j] ~ dweib(r, mu[i,j])C(t.cen[i, j], );
				log(mu[i,j ]) <- alpha + beta.age * age[i, j] 
						+ beta.sex  *sex[i]  
						+ beta.dis[disease[i]] + b[i];
			}
	# Random effects:
			b[i] ~ dnorm(0.0, tau)   
		}
	# Priors:
		alpha ~ dnorm(0.0, 0.0001);
		beta.age ~ dnorm(0.0, 0.0001);
		beta.sex ~ dnorm(0.0, 0.0001);
	#	beta.dis[1] <- 0;  # corner-point constraint
		for(k in 2 : 4) {
			beta.dis[k] ~ dnorm(0.0, 0.0001);
		}
		tau ~ dgamma(1.0E-3, 1.0E-3);
		r ~ dgamma(1.0, 1.0E-3); 
		sigma <- 1 / sqrt(tau); # s.d. of random effects
	}


Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )


Results 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	-4.826	-4.753	1.011	0.05058	-6.98	-3.056	5001	20000	399
	beta.age	0.003459	0.003102	0.01633	4.053E-4	-0.02815	0.03706	5001	20000	1624
	beta.dis[2]	0.1652	0.161	0.5986	0.01617	-1.001	1.382	5001	20000	1370
	beta.dis[3]	0.693	0.6753	0.6033	0.01775	-0.4485	1.946	5001	20000	1154
	beta.dis[4]	-1.193	-1.22	0.899	0.0253	-2.915	0.6289	5001	20000	1262
	beta.sex	-2.053	-2.016	0.5507	0.02239	-3.235	-1.05	5001	20000	605
	r	1.274	1.272	0.1892	0.01209	0.9307	1.644	5001	20000	244
	sigma	0.7332	0.7677	0.3979	0.02343	0.04832	1.477	5001	20000	288

