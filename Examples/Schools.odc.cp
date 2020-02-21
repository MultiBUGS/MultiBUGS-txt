	Schools: ranking schoolexamination
			results using multivariate
			hierarcical models

Goldstein et al. (1993) present an analysis of examination results from inner London schools. They use hierarchical or multilevel models to study the between-school variation, and calculate school-level residuals in an attempt to differentiate between `good' and `bad' schools. Here we analyse a subset of this data  and show how to calculate a rank ordering of schools and obtain credible intervals on each rank.

Data

Standardized mean examination scores (Y) were available for 1978 pupils from 38 different schools. The median number of pupils per school was 48, with a range of 1--198. Pupil-level covariates included gender plus a standardized London Reading Test (LRT) score and a verbal reasoning (VR) test category (1, 2 or 3, where 1 represents the highest ability group) measured when each child was aged 11. Each school was classified by gender intake (all girls, all boys or mixed) and denomination (Church of England, Roman Catholic, State school or other); these were used as categorical school-level covariates.

Model

We consider the following model, which essentially corresponds to Goldstein et al.'s model 1. 


	Yij	~	Normal(mij, tij)
	mij	=	a1j + a2j LRTij + a3j VR1ij + b1 LRTij2 + b2 VR2ij + b3 Girlij
			+ b4 Girls' schoolj + b5 Boys' schoolj + b6 CE schoolj
			+ b7 RC schoolj + b8 other schoolj
	log tij	 =	q + f LRTij

where i refers to pupil and j indexes school. We wish to specify a regression model for the variance components, and here we model the logarithm of tij (the inverse of the between-pupil variance) as a linear function of each pupil's LRT score. This differs from Goldstein et al.'s model which allows the variance s2ij to depend linearly on LRT. However, such a parameterization may lead to negative estimates of s2ij.

Prior distributions

The fixed effects bk (k=1,...,8), q and f were assumed to follow vague independent Normal distributions with zero mean and low precision = 0.0001. The random school-level coefficients akj (k = 1,2,3) were assumed to arise from a multivariate normal population distribution with unknown mean g and covariance matrix S. A non-informative multivariate normal prior was then specified for the population mean g, whilst the inverse covariance matrix T = S-1  was assumed to follow a Wishart distribution. To represent vague prior knowledge, we chose the degrees of freedom for this distribution to be as small as possible (i.e. 3, the rank of T). The scale matrix R was specified as

 
		0.1	0.005	0.005	
		0.005	0.01	0.005	
		0.005	0.005	0.01	


which represents our prior guess at the order of magnitude of S.
 
The BUGS code is given below:

	model
	{
		for(p in 1 : N) {
			Y[p] ~ dnorm(mu[p], tau[p])
			mu[p] <- alpha[school[p], 1] + alpha[school[p], 2] * LRT[p] 
				+ alpha[school[p], 3] * VR[p, 1] + beta[1] * LRT2[p] 
				+ beta[2] * VR[p, 2] + beta[3] * Gender[p] 
				+ beta[4] * School.gender[p, 1] + beta[5] * School.gender[p, 2]
				+ beta[6] * School.denom[p, 1] + beta[7] * School.denom[p, 2]
				+ beta[8] * School.denom[p, 3]
			log(tau[p]) <- theta + phi * LRT[p]
			sigma2[p] <- 1 /  tau[p]
			LRT2[p] <- LRT[p] * LRT[p]
		  }
		  min.var <- exp(-(theta + phi * (-34.6193))) # lowest LRT score = -34.6193
		  max.var <- exp(-(theta + phi * (37.3807)))  # highest LRT score = 37.3807

	 # Priors for fixed effects:
		for (k in 1 : 8) {  
			beta[k] ~ dnorm(0.0, 0.0001)   
		}
		theta ~ dnorm(0.0, 0.0001)
		phi ~ dnorm(0.0, 0.0001)

	# Priors for random coefficients:
		for (j in 1 : M) {
			alpha[j, 1 : 3] ~ dmnorm(gamma[1:3 ], T[1:3 ,1:3 ]); 
			alpha1[j] <- alpha[j,1]
		}
 
	# Hyper-priors:
		gamma[1 : 3] ~ dmnorm(mn[1:3 ], prec[1:3 ,1:3 ]);
		T[1 : 3, 1 : 3 ] ~ dwish(R[1:3 ,1:3 ], 3)
	}


Data	( click to open )


Note that school is a 1978 x 3 matrix taking value 1 for all pupils in school 1, 2 for all pupils in school 2 and so on. For computational convenience, Y, mu and tau are indexed over a single dimension p = 1,...,1978  rather than as pupil i within school j as used in equations above. The appropriate school-level coefficients for pupil p are then selected using the school indicator in row p of the data array --- for example alpha[school[p],1]. 


Inits for chain 1    	Inits for chain 2	 ( click to open )


Results 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	beta[1]	2.589E-4	2.589E-4	9.8E-5	1.195E-6	6.877E-5	4.508E-4	150001	40000	6728
	beta[2]	0.4143	0.4146	0.0597	0.00154	0.2968	0.5329	150001	40000	1502
	beta[3]	0.1698	0.1698	0.04795	7.183E-4	0.07699	0.2641	150001	40000	4455
	beta[4]	0.1119	0.1112	0.132	0.002941	-0.1452	0.3767	150001	40000	2015
	beta[5]	0.05174	0.05343	0.1048	0.002525	-0.1604	0.2537	150001	40000	1723
	beta[6]	-0.2844	-0.2848	0.1853	0.003784	-0.652	0.08098	150001	40000	2397
	beta[7]	0.1433	0.1429	0.1049	0.002043	-0.06573	0.3509	150001	40000	2638
	beta[8]	-0.1671	-0.1666	0.1792	0.003683	-0.526	0.1848	150001	40000	2368
	gamma[1]	-0.662	-0.6609	0.09236	0.002776	-0.8444	-0.4826	150001	40000	1106
	gamma[2]	0.03141	0.03138	0.01023	7.097E-5	0.01105	0.05164	150001	40000	20779
	gamma[3]	0.9507	0.951	0.08317	0.002161	0.7846	1.115	150001	40000	1481
	phi	-0.002685	-0.002676	0.00283	1.764E-5	-0.008264	0.002837	150001	40000	25744
	theta	0.5801	0.5803	0.03232	1.81E-4	0.5165	0.6433	150001	40000	31887


