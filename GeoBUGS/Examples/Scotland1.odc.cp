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
				S[1:N] ~ car.proper(theta[], C[], adj[], num[], m[], prec, gamma)

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

Data	(Click to open)

Inits for chain 1	Inits for chain 2	(Click to open)

One update took 0.6ms

Update methods

		Updater type	Size	Depth
	S[1]	wrapper for chain graph updater	56	2
	<S[2]>

	<S[56]>
	alpha	conjugate normal updater	1	1
	gamma	slice updater	1	1
	prec	conjugate gamma updater	1	1






		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	-0.1418	-0.1444	0.1586	0.001663	-0.4565	0.1757	1001	20000	9096
	gamma	0.1629	0.1687	0.01945	2.558E-4	0.112	0.1822	1001	20000	5780
	sigma	1.778	1.757	0.2464	0.003308	1.35	2.32	1001	20000	5549



GMRF updater

One update took 6s

Choice of updaters

		Updater type	Size	Depth
	S[1]	general GMRF updater	56	2
	<S[2]>
	...
	<S[56]>
	alpha	conjugate normal updater	1	1
	gamma	slice updater	1	1
	prec	conjugate gamma updater	1	1


		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	-0.1368	-0.1381	0.1592	0.002673	-0.4468	0.1774	1001	20000	3547
	gamma	0.1632	0.1688	0.0189	2.887E-4	0.1129	0.1823	1001	20000	4283
	sigma	1.776	1.758	0.2478	0.005966	1.338	2.313	1001	20000	1724



Check of externalization first process

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	-0.1441	-0.1466	0.158	0.002404	-0.459	0.1764	1001	10000	4319
	gamma	0.1625	0.1684	0.01984	3.899E-4	0.1116	0.1822	1001	10000	2590
	sigma	1.786	1.763	0.247	0.004855	1.363	2.334	1001	10000	2589

second process

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	-0.1441	-0.1466	0.158	0.002404	-0.459	0.1764	1001	10000	4319
	gamma	0.1625	0.1684	0.01984	3.899E-4	0.1116	0.1822	1001	10000	2590
	sigma	1.786	1.763	0.247	0.004855	1.363	2.334	1001	10000	2589
	
		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	-0.1418	-0.1444	0.1586	0.001663	-0.4565	0.1757	1001	20000	9096
	gamma	0.1629	0.1687	0.01945	2.558E-4	0.112	0.1822	1001	20000	5780
	sigma	1.778	1.757	0.2464	0.003308	1.35	2.32	1001	20000	5549

Check of externalization first process

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	-0.1326	-0.1323	0.159	0.003669	-0.4409	0.1829	1001	10000	1878
	gamma	0.1631	0.1686	0.01887	4.113E-4	0.113	0.1823	1001	10000	2104
	sigma	1.778	1.759	0.2482	0.007915	1.338	2.312	1001	10000	982

second process

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	-0.1326	-0.1323	0.159	0.003669	-0.4409	0.1829	1001	10000	1878
	gamma	0.1631	0.1686	0.01887	4.113E-4	0.113	0.1823	1001	10000	2104
	sigma	1.778	1.759	0.2482	0.007915	1.338	2.312	1001	10000	982
	
		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	-0.1368	-0.1381	0.1592	0.002673	-0.4468	0.1774	1001	20000	3547
	gamma	0.1632	0.1688	0.0189	2.887E-4	0.1129	0.1823	1001	20000	4283
	sigma	1.776	1.758	0.2478	0.005966	1.338	2.313	1001	20000	1724

