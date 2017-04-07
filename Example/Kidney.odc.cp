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


Data ( click to open )

Inits for chain 1 		Inits for chain 2	( click to open )

Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	-4.695	-4.614	0.9649	0.04771	-6.796	-3.019	5001	20000	408
	beta.age	0.002553	0.00238	0.01578	3.907E-4	-0.02852	0.03442	5001	20000	1631
	beta.dis[2]	0.1405	0.1265	0.5916	0.01565	-1.006	1.342	5001	20000	1428
	beta.dis[3]	0.674	0.6499	0.5756	0.01667	-0.4037	1.89	5001	20000	1192
	beta.dis[4]	-1.168	-1.169	0.8988	0.02882	-2.924	0.6325	5001	20000	972
	beta.sex	-1.999	-1.96	0.5472	0.02196	-3.181	-1.015	5001	20000	620
	r	1.246	1.228	0.1865	0.01179	0.9274	1.645	5001	20000	250
	sigma	0.6985	0.7141	0.385	0.02236	0.0379	1.454	5001	20000	296


