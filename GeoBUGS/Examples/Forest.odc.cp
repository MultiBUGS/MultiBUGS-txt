	Poisson-gamma spatial moving
							average (convolution)model: 
							Distribution of hickory trees
							in Duke forest

Ickstadt and Wolpert (1998) and Wolpert and Ickstadt (1998) analyse the spatial distribution of hickory trees (a subdominant species) in a 140 metre x 140 metre research plot in Duke Forest, North Carolina, USA, with the aim of investigating the regularity of the distribution of trees: spatial clustering would reveal that the subdominant species is receding from, or encroaching into, a stand recovering from a disturbance, while regularity would suggest a more stable and undisturbed recent history.

The data comprise counts of trees, Yi, in each of i=1,...,16 30m x 30m quadrats (each having area Ai = 900 m2) on a 4x4 grid covering the research plot (excluding a 10m boundary region to minimise edge effects), together with the x and y co-ordinates of the centroid of each plot (relative to the south-west corner of the research plot). Ickstadt and Wolpert (1998) fit a Poisson-gamma spatial moving average model as follows:

			Yi ~   Poisson( m i )          i = 1,...,16
			m i    =   Ai * l i  
			l i    =   Sj kij g j

where g j can be thought of as latent unobserved risk factors associated with locations or sub-regions of the study region indexed by j, and kij is a kernel matrix of 'weights' which depend on the distance or proximity between latent location j  and quadrat i of the study region (see Appendix 2 for further details of this convolution model). Ickstadt and Wolpert (1998) take the locations of the latent g j to be the same as the partition of the study region used for the observed data i.e. j = 1,....16 with latent sub-region j being the same as quadrat i of the study region. Note that it is not necessary for the latent partition to be the same as the observed partition -  see Hudersfield.  The g j are assigned independent gamma prior distributions

			g j    ~   Gamma(a, b)        j = 1,....,16.

Ickstadt and Wolpert (1998) consider two alternative kernel functions: an adjacency-based kernel and a distance-based kernel. The adjacency based kernel is defined as:

			kij = 1                  if i = j
			kij = exp(q2)/ni      if j is a nearest neighbour of i  (only north-south and east-west
			 neighbours considered)  and ni is the number of neighbours of area i
			kij = 0                  otherwise

The distance based kernel is defined as:

    kij = exp(- [dij / exp(q2)]2)       where dij is the distance between the centroid of areas i and j

For both kernels, the parameter exp(q2) reflects the degree of spatial dependence or clustering in the data, with large values of q2 suggesting spatial correlation and irregular distribution of hickory trees.

Ickstadt and Wolpert (1998) elicit expert prior opinion concerning the unknown hyperparameters q0 = log(a), q1 = -log(b) and q2 and translate this information into Normal prior distributions for q0, q1 and q2.

This model may be implemented in BUGS using the pois.conv distribution. The code is given below. 

Model

			model {

			# likelihood
				for (i in 1:N) {
					Y[i] ~ dpois.conv(mu[i,])
					for (j in 1:J) {
						mu[i, j] <- A[i] * lambda[i, j]
						lambda[i, j] <- k[i, j] * gamma[j]
					}
				}

			# priors   (see Ickstadt and Wolpert (1998) for details of prior elicitation)
				for (j in 1:J) {
					gamma[j] ~ dgamma(alpha, beta)
				}
				alpha <- exp(theta0)
				beta <- exp(-theta1)
				
				theta0 ~ dnorm(-0.383, 1)       
				theta1 ~ dnorm(-5.190, 1)
				theta2 ~ dnorm(-1.775, 1)				# prior on theta2 for adjacency-based kernel
				#	theta2 ~ dnorm(1.280, 1)			# prior on theta2 for distance-based kernel
      
			# compute adjacency-based kernel
			# Note N = J in this example (necessary for adjacency-based kernel)
				for (i in 1:N) {
					k[i, i] <- 1
					for (j in 1:J) {
			# distance between areas i and j
						d[i, j] <- sqrt((x[i] - x[j])*(x[i] - x[j]) + (y[i] - y[j])*(y[i] - y[j]))
			# (only needed to help compute nearest neighbour weights;
			#  alternatively, read matrix w from file)
						w[i, j] <- step(30.1 - d[i, j])			# nearest neighbour weights:
			# areas are 30 sq m, so any pair of areas with centroids > 30m apart are not 
			# nearest neighbours (0.1 added to account for numeric imprecision in d)
					}
					for (j in (i+1):J) {        
						k[i, j] <- w[i, j] * exp(theta2) / (sum(w[i,]) - 1)
						k[j, i] <- w[j, i] * exp(theta2) / (sum(w[j,]) - 1)
					}
				}

			# alternatively, compute distance-based kernel
			#	for (i in 1:N) {    
			#		k[i, i] <- 1      
			#		for (j in 1:J) {
			# distance between areas i and j
			#			d[i, j] <- sqrt((x[i] - x[j])*(x[i] - x[j]) + (y[i] - y[j])*(y[i] - y[j]))
			#		}
			#		for (j in (i+1):J) {         
			#			k[i, j] <- exp(-pow(d[i, j]/exp(theta2), 2))
			#			k[j, i] <- exp(-pow(d[j, i]/exp(theta2), 2))
			#		}
			#	}

			# summary quantities for posterior inference 
				for (i in 1:N) {
			# smoothed density of hickory trees (per sq metre) in area i
					density[i] <- sum(lambda[i, ])
			# observed density of hickory trees (per sq metre) in area i
					obs.density[i] <- Y[i]/A[i]
				}
			# large values indicate strong spatial dependence;
			# spatial.effect -> 0 indicates spatial independence
				spatial.effect <- exp(theta2)
			}

			
Data 	(Click to open)

Inits for chain 1 	  Inits for chain 2	(Click to open)

Results

Assuming adjacency-based kernel (equivalent to Ickstadt and Wolpert's model MS):	

One update took 0.4ms

Update methods

		Updater type	Size	Depth
	gamma[1]	conjugate gamma updater	1	2
	...
	gamma[16]	conjugate gamma updater	1	2
	theta0	adaptive metropolis 1D updater	1	1
	theta1	slice updater	1	1
	theta2	adaptive metropolis 1D updater	1	1

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	spatial.effect	0.2843	0.2534	0.1768	0.00718	0.04118	0.692	2001	20000	606
	theta0	-0.6121	-0.6093	0.4754	0.01936	-1.549	0.3323	2001	20000	603
	theta1	-5.109	-5.133	0.5328	0.01563	-6.072	-4.001	2001	20000	1162
	theta2	-1.478	-1.373	0.7244	0.02981	-3.19	-0.3682	2001	20000	590


Ickstadt and Wolpert report a posterior mean (sd) of 0.281 (0.156) for the spatial effect, exp(q2), from their analysis using a 4x4 partition of the study region (Table 1.3).


Assuming distance-based kernel (equivalent to Ickstadt and Wolpert's model MD):

One update took 1.1ms

Update methods

		Updater type	Size	Depth
	gamma[1]	conjugate gamma updater	1	2
...
	gamma[16]	conjugate gamma updater	1	2
	theta0	adaptive metropolis 1D updater	1	1
	theta1	slice updater	1	1
	theta2	adaptive metropolis 1D updater	1	1




			mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	spatial.effect	7.077	4.157	6.839	0.279	0.5397	22.82	10001	100000	600
	theta0	-0.2446	-0.1996	0.4956	0.01475	-1.403	0.62	10001	100000	1129
	theta1	-5.307	-5.336	0.5126	0.00993	-6.232	-4.213	10001	100000	2665
	theta2	1.455	1.425	1.064	0.03283	-0.6167	3.128	10001	100000	1050

	
	

Ickstadt and Wolpert report a posterior mean (sd) of 7.449 (6.764) for the spatial effect, exp(q2), from their analysis using a 4x4 partition of the study region (Table 1.3), and posterior means of -0.02, -5.28 and 1.54 respectively for theta0, theta1 and theta2. Ickstadt and Wolpert noted that the posterior for the spatial effect was multi-modal:

Note however that convergence of this distance-based model is not very good, and the results reported above are not particularly stable (the adjacency-based model appears to converge much better). This may be partly due to the rather strong prior information about theta2 for the distance-based model which, as noted by Ickstadt and Wolpert, appears to conflict with the data. 


