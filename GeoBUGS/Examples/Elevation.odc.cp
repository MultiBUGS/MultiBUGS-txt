	Bayesian kriging and spatial
							prediction:Surface elevation

The data for this example are taken from Diggle and Riberio (2000) (and came originally from Davis (1973)). The data file contains the variables height, x and y, giving surface elevation at each of 52 locations (x, y) within a 310-foot square.  The unit of distance is 50 feet, whereas one unit of height represents 10 feet of elevation. A Gaussian kriging model can be fitted to these data in OpenBUGS using either the spatial.exp or spatial.disc distributions. The data file also contains a set of locations x.pred and y.pred representing a 15 x 15 grid of points at which we wish to predict surface elevation. Predictions can be obtained using either the spatial.pred or spatial.unipred predictive distributions in OpenBUGS

Model

			model {

			# Spatially structured multivariate normal likelihood  
			# exponential correlation function
				height[1:N] ~ spatial.exp(mu[], x[], y[], tau, phi, kappa)
			# disc correlation function 
		   #	height[1:N] ~ spatial.disc(mu[], x[], y[], tau, alpha)

				for(i in 1:N) {
					mu[i] <- beta
				}

			# Priors
				beta ~ dflat()
				tau ~ dgamma(0.001, 0.001) 
				sigma2 <- 1/tau

			# priors for spatial.exp parameters
			# prior range for correlation at min distance (0.2 x 50 ft) is 0.02 to 0.99
				phi ~ dunif(0.05, 20)
			# prior range for correlation at max distance (8.3 x 50 ft) is 0 to 0.66
				kappa ~ dunif(0.05,1.95)

			# priors for spatial.disc parameter
			# prior range for correlation at min distance (0.2 x 50 ft) is 0.07 to 0.96
			#	alpha ~ dunif(0.25, 48)
			# prior range for correlation at max distance (8.3 x 50 ft) is 0 to 0.63

			# Spatial prediction

			# Single site prediction
				for(j in 1:M) {
					height.pred[j] ~ spatial.unipred(beta, x.pred[j], y.pred[j], height[])
				}

			# Only use joint prediction for small subset of points, due to length of time it takes to run
				for(j in 1:10) { mu.pred[j] <- beta }
					height.pred.multi[1:10] ~ spatial.pred(mu.pred[], x.pred[1:10], y.pred[1:10], height[])   
				}
			}

Data	(Click to open)

Initial values for exponential model

Inits for chain 1	Inits for chain 2	(Click to open)


Initial values for disc model

Inits for chain 1	Inits for chain 2	(Click to open)


The GeoBUGS map tool can be used to produce an approximate map of the posterior mean predicted surface elevation. It is not possible to map point locations using GeoBUGS. However, a map file called elevation is already loaded in the GeoBUGS Map Tool --- this is a 15 x 15 grid with the centroids of the grid cell corresponding to the locations of each of the prediction points. You can use this to produce a map of the posterior mean (or other posterior summaries) of the vector of predicted values height.pred. 

Results for exponential model



		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	beta	857.7	852.8	61.02	0.6021	746.7	992.4	1001	20000	10268
	height.pred[1]	872.0	872.5	8.346	0.063	854.0	887.3	1001	20000	17552
	height.pred[2]	861.7	861.9	19.2	0.1317	823.4	899.3	1001	20000	21259
	height.pred[3]	848.6	848.9	23.54	0.1712	801.9	894.8	1001	20000	18915
	height.pred[4]	837.1	837.1	20.25	0.1345	796.6	876.7	1001	20000	22649
	height.pred[5]	830.0	829.9	8.806	0.06528	812.8	847.9	1001	20000	18193
	height.pred[6]	837.0	836.9	15.18	0.1093	807.2	867.7	1001	20000	19308
	height.pred[7]	853.2	853.4	21.23	0.1538	811.3	894.6	1001	20000	19072
	height.pred[8]	870.0	870.2	20.84	0.1514	828.1	910.3	1001	20000	18948
	height.pred[9]	884.0	884.2	16.29	0.125	851.2	915.4	1001	20000	16980
	height.pred[10]	890.8	891.0	9.553	0.06955	871.1	909.5	1001	20000	18866
	height.pred.multi[1]	872.1	872.5	8.409	0.06452	854.4	887.7	1001	20000	16987
	height.pred.multi[2]	861.8	861.9	19.28	0.1353	823.3	899.1	1001	20000	20299
	height.pred.multi[3]	848.8	848.8	23.04	0.1723	802.8	893.8	1001	20000	17886
	height.pred.multi[4]	837.0	837.0	20.16	0.1494	797.5	876.9	1001	20000	18198
	height.pred.multi[5]	830.0	829.9	8.874	0.05992	812.4	847.9	1001	20000	21927
	height.pred.multi[6]	836.9	836.9	15.31	0.1165	806.4	867.9	1001	20000	17274
	height.pred.multi[7]	853.0	853.2	20.98	0.1549	811.6	894.2	1001	20000	18344
	height.pred.multi[8]	870.0	870.0	21.03	0.1566	827.8	911.0	1001	20000	18033
	height.pred.multi[9]	884.0	884.2	16.06	0.115	851.1	915.1	1001	20000	19515
	height.pred.multi[10]	890.9	891.1	9.714	0.07302	871.2	909.9	1001	20000	17693
	kappa	1.393	1.416	0.251	0.004646	0.8562	1.807	1001	20000	2919
	phi	0.2906	0.2781	0.1378	0.003875	0.07161	0.5806	1001	20000	1264




			
			
			

Results for disc model



		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	25.99	26.31	12.69	0.3325	5.656	46.84	1001	20000	1457
	beta	881.2	877.2	101.1	0.7449	672.0	1094.0	1001	20000	18436
	height.pred[1]	870.0	869.9	11.02	0.07299	848.3	891.7	1001	20000	22775
	height.pred[2]	858.9	858.8	21.17	0.144	817.0	900.9	1001	20000	21627
	height.pred[3]	847.9	847.9	24.09	0.1776	800.4	895.5	1001	20000	18388
	height.pred[4]	837.9	837.9	21.94	0.1519	794.2	881.5	1001	20000	20870
	height.pred[5]	831.0	831.0	11.81	0.08761	807.8	854.4	1001	20000	18171
	height.pred[6]	838.6	838.6	17.95	0.1252	803.3	874.2	1001	20000	20545
	height.pred[7]	853.4	853.4	22.6	0.1534	809.1	898.3	1001	20000	21695
	height.pred[8]	868.2	868.3	22.64	0.1504	823.8	912.9	1001	20000	22653
	height.pred[9]	881.6	881.7	18.85	0.1359	844.3	918.4	1001	20000	19237
	height.pred[10]	890.6	890.7	12.52	0.08819	866.0	915.7	1001	20000	20149
	height.pred.multi[1]	869.9	869.8	11.03	0.07467	848.0	891.8	1001	20000	21805
	height.pred.multi[2]	858.4	858.6	21.26	0.1404	816.4	900.0	1001	20000	22934
	height.pred.multi[3]	847.3	847.2	24.28	0.1802	800.0	895.3	1001	20000	18157
	height.pred.multi[4]	837.9	837.9	22.16	0.1688	794.1	881.6	1001	20000	17232
	height.pred.multi[5]	830.8	830.8	11.81	0.08583	807.5	854.1	1001	20000	18933
	height.pred.multi[6]	838.8	838.6	17.79	0.1227	803.8	873.7	1001	20000	21000
	height.pred.multi[7]	853.3	853.3	22.71	0.1771	808.5	898.0	1001	20000	16433
	height.pred.multi[8]	868.4	868.4	22.71	0.1665	823.5	913.1	1001	20000	18588
	height.pred.multi[9]	881.5	881.4	18.88	0.1391	844.4	919.0	1001	20000	18431
	height.pred.multi[10]	890.5	890.5	12.62	0.08265	865.8	915.6	1001	20000	23300
	sigma2	13730.0	13320.0	7417.0	185.0	2631.0	29080.0	1001	20000	1607





