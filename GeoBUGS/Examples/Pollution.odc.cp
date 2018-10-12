	Random walk priors for
					 temporal smoothing of daily
					 air pollution estimates

Shaddick and Wakefield (2002) consider spatiotemporal modelling of daily ambient air pollution at a number of monitoring sites in London. Here we take a subset of their data on a single pollutant measured at one site for 366 days, and model temporal autocorrelation using a random walk prior. 

Conditional on the underlying mean concentration mt on day t, the likelihood for the observed pollution concetration Yt is assumed to be independent Normal i.e.

			Yt ~ Normal(mt, terr)            where 1/terr  is the measurement error variance
			mt = b + qt      

where b is the overall mean pollution concentration at the site, and qt is a (zero mean) random error term representing daily fluctuations about this mean. To reflect the prior belief that these daily fluctuations are correlated, a random walk prior is assumed for q  = {q1 , ......, q366 } (see equation 7 in Shaddick and Wakefield):
			qt  |  q-t  	~  Normal (  qt+1,  f )	for  t = 1
				~  Normal (  (qt-1 + qt+1)/2,  f / 2 )	for  t = 2, ...., T-1
				~  Normal (  qt-1,  f )	for  t = T

where q-t denotes all elements of q  except the qt. This prior may be specified in WinBUGS 1.4 using the  car.normal  distribution, with adjacency vector adj[] listing neighbouring time points (i.e. (t-1) and (t+1) are neighbours of time point t), corresponding weight vector weight[] set to a sequence of 1's, and vector giving the number of neighbours, num[], set to 2 for all time points except num[1] and num[T] which are set to 1. 

The RW(1) reflects prior beliefs about smoothness of first differences, i.e. sudden jumps between consecutive values of q  are unlikely. Alternatively, we may assume a second order random walk prior RW(2) for q , which represents prior beliefs that the rate of change (gradient) of q  is smooth:
			qt  |  q-t   ~  Normal (  2qt+1 - qt+2,  f ) for  t = 1
				~  Normal (  (2qt-1 + 4qt+1 - qt+2) / 5,  f / 5 ) for  t = 2 
				~  Normal (  (-qt-2 + 4qt-1 + 4qt+1 - qt+2) / 6,  f / 6 ) for  t = 3, ...., T - 2
				~  Normal (  (-qt-2 + 4qt-1 + 2qt+1) / 5,  f / 5 ) for  t = T  -1
				~  Normal (  -qt-2+ 2qt-1,  f ) for  t = T

Again this may be specified using the car.normal  distribution in OpenBUGS via appropriate specification of the adj[], weight[] and num[] vectors.

The model code for fitting these two models is given below.

Model

			model {

			 #likelihood
				for(t in 1:T) {
					y[t] ~ dnorm(mu[t], tau.err)
					mu[t] <- beta + theta[t]
				}
    
				# prior for temporal effects 
			# RW prior for theta[t] - specified using car.normal with 
			#neighbours (t-1) and (t+1) 
			# for theta[2],....,theta[T-1], and neighbours (t+1) for 
			#theta[1] and (t-1) for theta[T]

				theta[1:T] ~ car.normal(adj[], weights[], num[], tau)    
				beta ~ dflat()
				
			# Specify weight matrix and adjacency matrix corresponding to RW(1) prior
			# (Note - this could be given in the data file instead)

				for(t in 1:1) {
					weights[t] <- 1;
					adj[t] <- t+1;
					num[t] <- 1
				}
				for(t in 2:(T-1)) {
					weights[2+(t-2)*2] <- 1;
					adj[2+(t-2)*2] <- t-1
					weights[3+(t-2)*2] <- 1;
					adj[3+(t-2)*2] <- t+1;
					num[t] <- 2
				}
				for(t in T:T) {
					weights[(T-2)*2 + 2] <- 1;
					adj[(T-2)*2 + 2] <- t-1;
					num[t] <- 1
				}
				
			# Alternatively, a weight matrix and adjacency matrix 
			#corresponding to RW(2) prior can
			# be specified or given in the data file 
			#(note, no need to change the prior distribution
			# on theta,  just the weights/adjacencies)
			#	for(t in 1:1) {		
			#		weights[t] <- 2;		adj[t] <- t+1
			#		weights[t+1] <- -1;		adj[t+1] <- t+2;			num[t] <- 2
			#	}
			#	for(t in 2:2) {		
			#	weights[t+1] <- 2;			adj[t+1] <- t-1
			#	weights[t+2] <- 4;			adj[t+2] <- t+1
			#	weights[t+3] <- -1;			adj[t+3] <- t+2;			num[t] <- 3
			#	}
			#for(t in 3:(T-2)) {
			#		weights[6+(t-3)*4] <- -1;		adj[6+(t-3)*4] <- t-2
			#		weights[7+(t-3)*4] <- 4;		adj[7+(t-3)*4] <- t-1
			#		weights[8+(t-3)*4] <- 4;		adj[8+(t-3)*4] <- t+1
			#		weights[9+(t-3)*4] <- -1;		adj[9+(t-3)*4] <- t+2;			num[t] <- 4
			#	}
			#	for(t in (T-1):(T-1)) {		
			#		weights[(T-4)*4 + 6] <- 2;		adj[(T-4)*4 + 6] <- t+1
			#		weights[(T-4)*4 + 7] <- 4;		adj[(T-4)*4 + 7] <- t-1
			#		weights[(T-4)*4 + 8] <- -1;		adj[(T-4)*4 + 8] <- t-2;			num[t] <- 3
			#	}
			#	for(t in T:T) {		
			#		weights[(T-4)*4 + 9] <- 2;		adj[(T-4)*4 + 9] <- t-1
			#		weights[(T-4)*4 + 10] <- -1;		adj[(T-4)*4 + 10] <- t-2;			num[t] <- 2
			#	}

			# other priors
				tau.err  ~ dgamma(0.01, 0.01)		  # measurement error precision
				sigma.err <- 1 / sqrt(tau.err)
				sigma2.err <- 1/tau.err

				tau  ~ dgamma(0.01, 0.01)				# random walk precision
				sigma <- 1 / sqrt(tau)
				sigma2 <- 1/tau
				sum <- sum(theta[])
			# include this variable to use in time series (model fit) plot
				for(t in 1:T) {    day[t] <- t   }				

			}

Note that pollution concentrations were not measured every day. However it is necessary to include days with no measurements as missing values (NA) in the data set, otherwise the temporal neighbourhood structure cannot be specified correctly.

Data 	(Click to open) 

Inits for chain 1	Inits for chain 2	(Click to open) 



Plus click on gen inits to generate initial values for the missing data



Results

RW(1) prior:


		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	beta	3.03	3.03	0.02003	6.173E-4	2.991	3.07	1001	20000	1052
	mu[1]	2.869	2.868	0.197	0.001898	2.486	3.258	1001	20000	10773
	mu[2]	2.833	2.831	0.181	0.003193	2.488	3.191	1001	20000	3213
	mu[3]	3.114	3.115	0.1687	0.001747	2.786	3.446	1001	20000	9315
	mu[4]	3.327	3.327	0.186	0.004047	2.958	3.693	1001	20000	2113
	mu[5]	3.11	3.11	0.1685	0.001655	2.775	3.441	1001	20000	10366
	mu[360]	2.582	2.583	0.167	0.001642	2.248	2.906	1001	20000	10350
	mu[361]	2.387	2.385	0.173	0.002564	2.053	2.728	1001	20000	4550
	mu[362]	2.421	2.422	0.1699	0.001788	2.085	2.757	1001	20000	9024
	mu[363]	2.475	2.475	0.1683	0.00154	2.14	2.804	1001	20000	11945
	mu[364]	2.516	2.517	0.1702	0.002125	2.18	2.844	1001	20000	6409
	mu[365]	2.404	2.403	0.1723	0.001557	2.063	2.745	1001	20000	12242
	mu[366]	2.294	2.291	0.1976	0.002129	1.915	2.689	1001	20000	8614
	sigma	0.266	0.2647	0.03997	0.001869	0.192	0.3488	1001	20000	457
	sigma.err	0.2476	0.2498	0.03555	0.001617	0.1722	0.311	1001	20000	483



Plot of posterior median (red line) and posterior 95% intervals (dashed blue lines) for mu[t] (the true mean daily pollutant concentration), with observed concentrations shown as black dots. (This plot was produced by selecting the model fit option from the Compare menu (available from the Inference menu), with mu specified as the node, day as the axis and y as other). Note that the dashed blue line shows the posterior 95% interval for the estimated mean daily concentration, and is not a predictive interval - hence we would not necessarily expect all of the observed data points to lie within the interval.
	


Equivalent plot assuming an RW(2) prior. Note the greater amount of smoothing imposed by this prior:


  


