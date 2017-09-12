	Cervix: case - control study 
		with errors in covariates

Carroll, Gail and Lubin (1993) consider the problem of estimating the odds ratio of a disease d in a case-control study where the binary exposure variable is measured with error. Their example concerns exposure to herpes simplex virus (HSV) in women with invasive cervical cancer (d=1) and in controls (d=0). Exposure to HSV is measured by a relatively inaccurate western blot procedure w for 1929 of the 2044 women, whilst for 115 women, it is also measured by a refined or "gold standard'' method x. The data are given in the table below. They show a substantial amount of misclassification, as indicated by low sensitivity and specificity of w in the "complete'' data, and Carroll, Gail and Lubin also found that the degree of misclassification was significantly higher for the controls than for the cases (p=0.049 by Fisher's exact test).
 


		d	x	w	Count
                          __________________________
	Complete data
	__________________________
		1	0	0	13	
		1	0	1	3	
		1	1	0	5	
		1	1	1	18	
		0	0	0	33	
		0	0	1	11	
		0 	1	0	16	
		0 	1	1	16
	_________________________	
	Incomplete data
		_________________________
		1		0	318	
		1		1	375	
		1		0	701	
		1		1	535	

They fitted a prospective logistic model to the case-control data as follows

 	di	 ~	Bernoulli(pi)	i = 1,...,2044 
 	logit(pi)	=	b0C +  bxi	i = 1,...,2044 

where b is the log odds ratio of disease. Since the relationship between d and x is only directly observable in the 115 women with "complete'' data, and because there is evidence of differential measurement error, the following parameters are required in order to estimate the logistic model

  	f1,1	=	P(w=1 | x=0, d=0)
  	f1,2	=	P(w=1 | x=0, d=1)
  	f2,1	=	P(w=1 | x=1, d=0)
  	f2,2	=	P(w=1 | x=1, d=1)
  	q	=	P(x=1)


The differential probability of being exposed to HSV (x=1) for cases and controls is calculated as follows



 	g1	=	P(x=1 | d=1)

 		=	P(d=1 | x=1) P(x=1)
			-----------------------------
				P(d=1) 
				
		=		1	1 - q
			-----------------------------------------------------		--------
			1 + (1 + exp b0C + b) / (1 + exp b0C)		q
			
	g2	=	P(x=1 | d=0)

 		=	P(d=0 | x=1) P(x=1)
			-----------------------------
				P(d=0) 
				
		=		1	1 - q
			-----------------------------------------------------		--------
			1 + (1 + exp -b0C - b) / (1 + exp -b0C)		q

The BUGS code is given below. The role of the variables x1 and d1 is to pick the appropriate value of f (the incidence of w) for any given true exposure status x and disease status d. Since x and d take the values 0 or 1, and the subscripts for f take values 1 or 2, we must first add 1 to each x[i] and d[i] in the BUGS code before using them as index values for f. BUGS does not allow subscripts to be functions of variable quantities --- hence the need to create x1and d1 for use as subscripts. In addition, note that g1 and g2 were not simulated directly in BUGS, but were calculated as functions of other parameters. This is because the dependence of g1 and g2 on d would have led to a cycle in the graphical model which would no longer define a probability distribution. 

	model 
	{
		for (i in 1 : N) {
			x[i]   ~ dbern(q)         # incidence of HSV
			logit(p[i]) <- beta0C + beta * x[i]	 # logistic model
			d[i]  ~ dbern(p[i])        # incidence of cancer
			x1[i] <- x[i] + 1 
			d1[i] <- d[i] + 1  
			w[i]  ~ dbern(phi[x1[i], d1[i]])	 # incidence of w
		}                                       
		q      ~ dunif(0.0, 1.0)           # prior distributions
		beta0C ~ dnorm(0.0, 0.00001);
		beta   ~ dnorm(0.0, 0.00001);
		for(j in 1 : 2) {
			for(k in 1 : 2){
				phi[j, k] ~ dunif(0.0, 1.0)
			}
		}
	# calculate gamma1 = P(x=1|d=0) and gamma2 = P(x=1|d=1) 
		gamma1 <- 1 / (1 + (1 + exp(beta0C + beta)) / (1 + exp(beta0C)) * (1 - q) / q)
		gamma2 <- 1 / (1 + (1 + exp(-beta0C - beta)) / (1 + exp(-beta0C)) * (1 - q) / q)
	}



Data ( click to open )


Inits for chain 1		  Inits for chain 2	 ( click to open )


Results 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	beta	0.6156	0.6096	0.3406	0.01362	-0.04223	1.292	1001	20000	625
	beta0C	-0.902	-0.8961	0.1892	0.007264	-1.29	-0.5442	1001	20000	678
	gamma1	0.4376	0.4381	0.05305	0.002121	0.3352	0.5416	1001	20000	625
	gamma2	0.588	0.5886	0.06298	0.002326	0.4601	0.7108	1001	20000	732
	phi[1,1]	0.3159	0.3163	0.05324	0.00204	0.2102	0.4172	1001	20000	681
	phi[1,2]	0.2167	0.2106	0.08086	0.002928	0.07517	0.3904	1001	20000	762
	phi[2,1]	0.5716	0.5721	0.06357	0.002301	0.4476	0.6958	1001	20000	762
	phi[2,2]	0.7688	0.7697	0.0639	0.002301	0.6417	0.8897	1001	20000	771
	q	0.4915	0.4913	0.04115	0.001537	0.4111	0.5706	1001	20000	716

