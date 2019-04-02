		Alligators: multinomial - logistic
		 regression

Agresti (1990) analyses a set of data on the feeding choice of  221 alligators, where the response measure for each alligator is one of 5 categories: fish, invertebrate, reptile, bird, other. Possible explanatory factors are the  length of alligator (two categories: <= 2.3 metres and > 2.3 metres), and  the lake (4 catgeories: Hancock, Oklawaha, Trafford, George). The full data is shown below.



		  	    		Primary Food Choice
	Lake	Size	 Fish	Invertebrate	Reptile	Bird	Other
		______________________________________________________________		
	Hancock 	<= 2.3	23	4	2	2	8
        		> 2.3	7	0	1	3	5
	Oklawaha	<=2.3	5	11	1	0	3
        		> 2.3	13	8	6	1	0
	Trafford 	<=2.3	5	11	2	1	5
        		> 2.3	8	7	6	3	5
	George	<=2.3	16	19	1	2	3
		> 2.3	17	1	0	1	3

Each combination of explanatory factors is assumed to give rise to a multinomial response with a logistic link, so that for lake i, size j, the observed vector of counts Xij. = Xij1,...,Xij5 has distribution

		Xij. ~  Multinomial(pij.,nij) 
		pijk   =  fijk / Sk fijk
		fijk   =  eak + bik  + gjk
  
where nij = Sk Xijk, and a1, bi1, b1k, gj1, g1k = 0 for identifiability. This model is discussed in detail in the Classic BUGS manual (version 0.5) in the section on Multionomial LogisticModels. All unknown a's, b's , g's  are initially given independent "noninformative" priors. 

The Classic BUGS manual (version 0.5) discusses two ways of fitting this model: directly in the form given above or by using the multinomial-Poisson transformation which will be somewhat more efficient.  Both techniques are illustrated in the code given below.  


	model
	{

	#  PRIORS
		alpha[1] <- 0;       # zero contrast for baseline food
		for (k in 2 : K) { 
			alpha[k] ~ dnorm(0, 0.00001) # vague priors
		} 
	# Loop around lakes:
		for (k in 1 : K){  
			beta[1, k] <- 0 
		} # corner-point contrast with first lake 
		for (i in 2 : I) {     
			beta[i, 1] <- 0 ;  # zero contrast for baseline food
			for (k in 2 : K){  
				beta[i, k] ~ dnorm(0, 0.00001) # vague priors
			} 
		}
	# Loop around sizes:
		for (k in 1 : K){  
			gamma[1, k] <- 0 # corner-point contrast with first size 
		}  
		for (j in 2 : J) {     
			gamma[j, 1] <- 0 ;  # zero contrast for baseline food
			for ( k in 2 : K){ 
				gamma[j, k] ~ dnorm(0, 0.00001) # vague priors
			} 
		}

	# LIKELIHOOD	
		for (i in 1 : I) {     # loop around lakes
			for (j in 1 : J) {     # loop around sizes

	# Multinomial response
	#     X[i,j,1 : K] ~ dmulti( p[i, j, 1 : K] , n[i, j]  )
	#     n[i, j] <- sum(X[i, j, ])
	#     for (k in 1 : K) {     # loop around foods
	#        p[i, j, k]        <- phi[i, j, k] / sum(phi[i, j, ])
	#        log(phi[i ,j, k]) <- alpha[k] + beta[i, k]  + gamma[j, k]
	#       }

	# Fit standard Poisson regressions relative to baseline
				lambda[i, j] ~ dflat()	# vague priors 
				for (k in 1 : K) {     # loop around foods
					X[i, j, k] ~ dpois(mu[i, j, k])
					log(mu[i, j, k]) <- lambda[i, j] + alpha[k] + beta[i, k]  + gamma[j, k]
					cumulative.X[i, j, k] <- cdf.pois(X[i, j, k], mu[i, j, k])
				}
			}  
		}

	# TRANSFORM OUTPUT TO ENABLE COMPARISON 
	#  WITH AGRESTI'S RESULTS

		for (k in 1 : K) {     # loop around foods
			for (i in 1 : I) {     # loop around lakes
				b[i, k] <- beta[i, k] - mean(beta[, k]);   # sum to zero constraint
			}
			for (j in 1 : J) {     # loop around sizes
				g[j, k] <- gamma[j, k] - mean(gamma[, k]); # sum to zero constraint
			}
		}
	}  

Data	( click to open )


Inits for chain 1   		Inits for chain 2 ( click to open )


Results 

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	b[1,2]	-1.809	-1.779	0.4705	0.03024	-2.824	-0.9281	1001	20000	242
	b[1,3]	-0.337	-0.3377	0.614	0.02376	-1.529	0.8717	1001	20000	667
	b[1,4]	0.5645	0.5616	0.5681	0.01522	-0.5276	1.683	1001	20000	1392
	b[1,5]	0.2861	0.2841	0.3644	0.01149	-0.4166	1.014	1001	20000	1005
	b[2,2]	0.8471	0.8587	0.3415	0.01698	0.1787	1.517	1001	20000	404
	b[2,3]	0.9382	0.9249	0.5172	0.01558	-0.02706	2.009	1001	20000	1102
	b[2,4]	-1.279	-1.139	1.01	0.02699	-3.672	0.337	1001	20000	1399
	b[2,5]	-0.6744	-0.6351	0.54	0.0157	-1.859	0.2812	1001	20000	1183
	b[3,2]	1.076	1.077	0.3502	0.0162	0.3559	1.759	1001	20000	467
	b[3,3]	1.451	1.427	0.5103	0.01207	0.507	2.517	1001	20000	1787
	b[3,4]	0.9416	0.9329	0.5918	0.01342	-0.2087	2.114	1001	20000	1945
	b[3,5]	0.9967	1.001	0.3929	0.01115	0.2209	1.755	1001	20000	1242
	b[4,2]	-0.1136	-0.1104	0.2929	0.01295	-0.6957	0.4493	1001	20000	511
	b[4,3]	-2.053	-1.903	1.007	0.01081	-4.405	-0.5143	1001	20000	8671
	b[4,4]	-0.2273	-0.212	0.6213	0.01092	-1.497	0.9715	1001	20000	3237
	b[4,5]	-0.6084	-0.6005	0.4099	0.008046	-1.432	0.1685	1001	20000	2595
	g[1,2]	0.7646	0.7564	0.2024	0.01025	0.3956	1.16	1001	20000	390
	g[1,3]	-0.1807	-0.1805	0.3109	0.01371	-0.7831	0.4249	1001	20000	514
	g[1,4]	-0.343	-0.3336	0.3391	0.01314	-1.006	0.3063	1001	20000	665
	g[1,5]	0.1898	0.1899	0.235	0.008351	-0.2776	0.6417	1001	20000	791
	g[2,2]	-0.7646	-0.7564	0.2024	0.01025	-1.16	-0.3956	1001	20000	390
	g[2,3]	0.1807	0.1805	0.3109	0.01371	-0.4241	0.7831	1001	20000	514
	g[2,4]	0.343	0.3336	0.3391	0.01314	-0.3057	1.006	1001	20000	665
	g[2,5]	-0.1898	-0.1899	0.235	0.008351	-0.6405	0.2776	1001	20000	791

