	model
	{
	# transform collapsed data into full
		for (i in 1 : I){
			Y[i,1] <- 1  
			Y[i,2] <- 0
		}
	# loop around strata with case exposed, control not exposed  (n10)
		for (i in 1 : n10){ 
			est[i,1] <- 1   
			est[i,2] <- 0
		}
	# loop around strata with case not exposed, control   exposed  (n01)
		for (i in (n10+1) : (n10+n01)){  
			est[i,1] <- 0     
			est[i,2] <- 1
		}
	# loop around strata with case exposed, control   exposed  (n11)
		for (i in (n10+n01+1) : (n10+n01+n11)){ 
			est[i,1] <- 1  
			est[i,2] <- 1
		}
	# loop around strata with case not exposed, control not exposed  (n00)
		for (i in (n10+n01+n11+1) :I ){ 
			est[i,1] <- 0  
			est[i,2] <- 0
		}

	# PRIORS
		beta ~ dnorm(0,1.0E-6)
	  
	# LIKELIHOOD
		for (i in 1 : I) {                    # loop around strata	
	# METHOD 1 - logistic regression
	#        Y[i,1] ~ dbin( p[i,1], 1) 
	#         logit(p[i,1]) <- beta * (est[i,1] - est[i,J]) 
	
	# METHOD 2 - conditional likelihoods
			Y[i, 1 : J] ~ dmulti( p[i, 1 : J],1)
			for (j in 1:2){
				p[i, j] <- e[i, j] / sum(e[i, ])
				log( e[i, j] ) <- beta * est[i, j] 
			}         
	# METHOD 3 fit standard Poisson regressions relative to baseline
	#for (j in 1:J) {     
	#	Y[i, j] ~ dpois(mu[i, j]);
	#	log(mu[i, j]) <- beta0[i] +  beta*est[i, j]; 
		} 
	#beta0[i] ~ dnorm(0, 1.0E-6) 
	}

