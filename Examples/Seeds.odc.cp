	Seeds: Random effect logistic
			regression

This example is taken from Table 3 of Crowder (1978), and concerns the proportion of seeds that germinated on each of 21 plates arranged according to a 2 by 2 factorial layout by seed and type of root extract.   The data are shown below, where ri and ni are the number of germinated and the total number of seeds on the i th plate, i =1,...,N.  These data are also analysed by, for example, Breslow: and Clayton (1993). 


		seed O. aegyptiaco 75						seed O. aegyptiaco 73
		Bean			Cucumber			Bean			Cucumber
			r	n	r/n	r	n	r/n	r	n	r/n	r	n	r/n
	_________________________________________________________________
	10	39	0.26	5	6	0.83	8	16	0.50	3	12	0.25
	23	62	0.37	53	74	0.72	10	30	0.33	22	41	0.54	
	23	81	0.28	55	72	0.76	8	28	0.29	15	30	0.50
	26	51	0.51	32	51	0.63	23	45	0.51	32	51	0.63
	17	39	0.44	46	79	0.58	0	4	0.00	3	7	0.43
				10	13	0.77	

The model is essentially a random effects logistic, allowing for over-dispersion.  If pi is the probability of germination on the i th plate,  we assume

	ri  ~  Binomial(pi, ni)
	
	logit(pi) = a0 + a1x1i + a2x2i + a12x1ix2i + bi
	
	bi  ~ Normal(0, t)
	
where x1i  , x2i   are the seed type and root extract of the i th plate, and an interaction term a12x1ix2i   is included.   a0 , a1 ,  a2 ,   a12 ,   t  are given independent "noninformative" priors.  

Graphical model for seeds  example

 
BUGS language for seeds example

	
		model
		{
			for( i in 1 : N ) {
				r[i] ~ dbin(p[i],n[i])
				beta[i] ~ dnorm(0.0,tau)
				logit(p[i]) <- alpha0 + alpha1 * x1[i] + alpha2 * x2[i] + 
					alpha12 * x1[i] * x2[i] + beta[i]
			}
			alpha0 ~ dnorm(0.0,1.0E-6)
			alpha1 ~ dnorm(0.0,1.0E-6)
			alpha2 ~ dnorm(0.0,1.0E-6)
			alpha12 ~ dnorm(0.0,1.0E-6)
			sigma ~ dunif(0,10)
			tau <- 1 / pow(sigma, 2)
		}


Data ( click to open )

Inits for chain 1 	Inits for chain 2	( click to open )


Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha0	-0.5484	-0.5498	0.2143	0.002605	-0.9729	-0.1128	1001	40000	6764
	alpha1	0.06001	0.07041	0.3478	0.004114	-0.6584	0.7215	1001	40000	7147
	alpha12	-0.8343	-0.8288	0.4845	0.005889	-1.824	0.1046	1001	40000	6769
	alpha2	1.36	1.357	0.3077	0.004021	0.7455	1.988	1001	40000	5854
	sigma	0.3543	0.3425	0.1491	0.003213	0.09024	0.6808	1001	40000	2153

We may compare simple logistic, maximum likelihood (from EGRET), penalized quasi-likelihood  (PQL) Breslow and Clayton (1993) with the BUGS results




Heirarchical centering is an interesting reformulation of random effects models. Introduce the variables

	mi  = a0 + a1x1i + a2x2i + a12x1ix2i  

	bi  =  mi + bi

the model then becomes

	ri  ~  Binomial(pi, ni)

	logit(pi) = bi

	bi  ~ Normal(mi , t)

The graphical model is shown below	


	
	model
	{
		for( i in 1 : N ) {
			beta[i] ~ dnorm(mu[i], tau)
			r[i] ~ dbin(p[i], n[i])
			mu[i] <- alpha0 + alpha1 * x1[i] + alpha2 * x2[i] + alpha12 * x1[i] * x2[i]
			logit(p[i]) <- beta[i]
		}
		alpha0 ~ dnorm(0.0, 1.0E-6)
		alpha1 ~ dnorm(0.0, 1.0E-6)
		alpha12 ~ dnorm(0.0, 1.0E-6)
		alpha2 ~ dnorm(0.0, 1.0E-6)
		sigma ~ dunif(0,10)
		tau <- 1 / pow(sigma, 2)
	}

Results	


		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha0	-0.5526	-0.5519	0.2144	0.004155	-0.9909	-0.1293	1001	40000	2662
	alpha1	0.06286	0.07425	0.3438	0.007168	-0.6464	0.7207	1001	40000	2300
	alpha12	-0.8416	-0.8382	0.4792	0.01004	-1.815	0.1063	1001	40000	2278
	alpha2	1.366	1.359	0.3025	0.005881	0.7832	1.999	1001	40000	2646
	sigma	0.3481	0.3387	0.1576	0.003817	0.05251	0.6937	1001	40000	1704

This formulation of the model has two advantages: the squence of random numbers generated by the Gibbs sampler has better correlation properties and the time per update is reduced because the updating for the a parameters is now conjugate.
