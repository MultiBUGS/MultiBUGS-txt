	 CAR model:  Lip cancer
						 revisited

This example illustrates the use of the proper CAR distribution (car.proper) rather than the intrinsic CAR distribution for the area-specific random effects in the Lip cancer example (9.1). We use the definition of the C and M matrices proposed by Cressie and colleagues (see section on proper CAR models in Appendix 1).


Model

			model {

			# Set up 'data' to define spatial dependence structure
			# =====================================
				for(i in 1 : N) { 
					m[i] <- 1/E[i]       # scaling factor for variance in each cell
				} 

			# The vector C[] required as input into the car.proper distribution is a vector
			# respresention of the weight matrix with elements Cij. The first J1 elements of the C[]
			# vector contain the weights for the J1 neighbours of area i=1; the (J1+1) to J2
			# elements of the C[] vector contain the weights for the J2 neighbours of area i=2;
			# etc. To set up this vector, we need to define a variable cumsum, which gives the
			# values of J1, J2, etc.; we then set up an index matrix pick[,] with N columns
			#  corresponding to the  i=1,...,N areas, and with the same number of rows as there are
			# elements in the C[] vector (i.e. sumNumNeigh). The elements 
			#C[ (cumsum[i]+1):cumsum[i+1] ] correspond to 
			# the set of weights Cij associated with area i, and so we set up ith column of the
			# matrix pick[,]to have a 1 in all the rows k for which 
			#cumsum[i] < k <= cumsum[i+1], and 0's elsewhere. 
			# For example, let N=4 and cumsum=c(0,3,5,6,8), so area i=1 has 3 neighbours, area
			# i=2 has 2 neighbours, area i=3 has 1 neighbour and area i=4 has 2 neighbours. The
			# the matrix pick[,] is:
			#                pick                  
			#             1, 0, 0, 0,                   
			#             1, 0, 0, 0,                   
			#             1, 0, 0, 0,                    
			#             0, 1, 0, 0,                   
			#             0, 1, 0, 0,                   
			#             0, 0, 1, 0,                   
			#             0, 0, 0, 1,                   
			#             0, 0, 0, 1,                   
			#
			# We can then use the inner product (inprod(,)) function in WinBUGS and the kth
			# row of pick to select which area corresponds to the kth element in the vector C[];
			# likewise, we can use inprod(,) 
			# and the ith column of pick to select the elements of C[] which correspond to area i.
			#
			# Note: this way of setting up the C vector is somewhat convoluted!!!! In future
			# versions, we hope the GeoBUGS adjacency matrix tool will be able to dump out the
			# relevant  vectors required. Alternatively, the C vector could be created using another
			# package (e.g. Splus) and read into WinBUGS as data.
			#
				cumsum[1] <- 0
				for(i in 2:(N+1)) {
					cumsum[i] <- sum(num[1:(i-1)])
				}	
				for(k in 1 : sumNumNeigh) { 
					for(i in 1:N) {
						pick[k,i] <- step(k - cumsum[i] - epsilon)  * step(cumsum[i+1] - k)   
						#  pick[k,i] = 1    if     cumsum[i] < k <= cumsum[i=1];  otherwise, pick[k,i] = 0
					}                                                       
					C[k] <- sqrt(E[adj[k]] / inprod(E[], pick[k,]))    # weight for each pair of neighbours
				}
				epsilon <- 0.0001

			# Model
			# =====

			# Likelihood
				for (i in 1 : N) {
					O[i]  ~ dpois(mu[i])
					log(mu[i]) <- log(E[i]) + S[i]
			# Area-specific relative risk 
					RR[i] <- exp(S[i])
					theta[i] <- alpha
				}

			# Proper CAR prior distribution for spatial random effects: 
				S[1:N] ~ car.proper(adj[], C[], num[], theta[], m[], prec, gamma)

			# Other priors:
				alpha  ~ dnorm(0, 0.0001)  
			# prior on precision
				prec  ~ dgamma(0.5, 0.0005)
				v <- 1/prec				# variance
				sigma <- sqrt(1 / prec)				# standard deviation

				gamma.min <- min.bound(C[], adj[], num[], m[])
				gamma.max <- max.bound(C[], adj[], num[], m[])
				gamma ~ dunif(gamma.min, gamma.max)

			}



Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )








		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	-0.1419	-0.1426	0.1588	0.001748	-0.4518	0.1696	1001	20000	8248
	gamma	0.1632	0.1687	0.01883	2.423E-4	0.1131	0.1823	1001	20000	6035
	sigma	1.784	1.767	0.2471	0.003224	1.361	2.326	1001	20000	5878

