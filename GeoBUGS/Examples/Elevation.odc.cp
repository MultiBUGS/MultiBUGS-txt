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

Results for exponential model

One update took 71ms

Update methods

		Updater type	Size	Depth
	beta	conjugate normal updater	1	1
	height.pred[1]	univariate forward updater	1	-2
...
	height.pred[225]	univariate forward updater	1	-2
	height.pred.multi[1]	multivariate forward updater	10	-2
	<height.pred.multi[2]>
...
	<height.pred.multi[10]>
	kappa	slice updater	1	1
	phi	slice updater	1	1
	tau	conjugate gamma updater	1	1


		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	beta	858.9	853.6	72.96	0.6127	735.6	1013.0	1001	20000	14176
	height.pred[1]	872.1	872.5	8.471	0.0695	854.3	887.6	1001	20000	14856
	height.pred[2]	861.8	861.8	19.32	0.1413	822.9	899.0	1001	20000	18699
	height.pred[3]	848.7	848.8	23.12	0.1479	803.0	894.2	1001	20000	24439
	height.pred[4]	837.2	837.3	20.15	0.1408	797.4	876.9	1001	20000	20484
	height.pred[5]	829.9	829.9	8.952	0.05996	812.2	847.8	1001	20000	22293
	height.pred[6]	837.2	837.1	15.21	0.111	807.3	867.3	1001	20000	18769
	height.pred[7]	853.2	853.2	21.08	0.1397	811.0	893.7	1001	20000	22776
	height.pred[8]	869.7	870.1	21.1	0.1596	827.6	910.5	1001	20000	17484
	height.pred[9]	884.1	884.3	16.28	0.1237	851.0	915.6	1001	20000	17311
	height.pred[10]	890.9	891.2	9.883	0.06742	870.7	909.7	1001	20000	21492
	height.pred.multi[1]	872.1	872.5	8.398	0.06729	853.8	887.5	1001	20000	15576
	height.pred.multi[2]	861.6	861.8	19.47	0.1464	822.2	899.5	1001	20000	17668
	height.pred.multi[3]	849.1	849.2	23.34	0.1589	802.7	894.9	1001	20000	21587
	height.pred.multi[4]	837.5	837.3	20.39	0.1384	797.7	877.5	1001	20000	21697
	height.pred.multi[5]	830.2	830.1	8.962	0.06421	812.9	848.2	1001	20000	19480
	height.pred.multi[6]	837.0	836.9	15.39	0.1051	806.5	867.9	1001	20000	21460
	height.pred.multi[7]	853.1	853.2	21.08	0.143	811.3	894.2	1001	20000	21733
	height.pred.multi[8]	870.1	870.2	21.1	0.1392	828.0	911.6	1001	20000	22970
	height.pred.multi[9]	884.1	884.3	16.37	0.128	850.8	915.8	1001	20000	16361
	height.pred.multi[10]	890.9	891.0	9.793	0.07218	871.1	909.8	1001	20000	18406
	kappa	1.392	1.412	0.2531	0.004373	0.8523	1.815	1001	20000	3348
	phi	0.2857	0.2751	0.1399	0.003651	0.06591	0.5818	1001	20000	1467


			
			
			


Results for disc model

One update took 60ms

Update methods

		Updater type	Size	Depth
	alpha	slice updater	1	1
	beta	conjugate normal updater	1	1
	height.pred[1]	univariate forward updater	1	-2
...
	height.pred[225]	univariate forward updater	1	-2
	height.pred.multi[1]	multivariate forward updater	10	-2
	<height.pred.multi[2]>
...
	<height.pred.multi[10]>
	tau	conjugate gamma updater	1	1


		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	alpha	25.67	25.96	12.69	0.2681	5.566	46.66	1001	20000	2240
	beta	880.2	874.9	100.9	0.6966	675.1	1097.0	1001	20000	20989
	height.pred[1]	870.0	870.0	10.98	0.07968	848.0	891.8	1001	20000	18998
	height.pred[2]	858.7	858.6	21.09	0.1378	817.2	900.5	1001	20000	23425
	height.pred[3]	847.5	847.6	24.16	0.1891	800.4	895.0	1001	20000	16324
	height.pred[4]	838.1	838.0	22.01	0.1664	795.0	881.5	1001	20000	17494
	height.pred[5]	831.0	831.0	11.79	0.08173	808.0	854.2	1001	20000	20823
	height.pred[6]	838.7	838.7	17.95	0.1284	802.7	874.4	1001	20000	19556
	height.pred[7]	853.6	853.5	22.62	0.1707	809.1	898.1	1001	20000	17561
	height.pred[8]	868.3	868.3	22.64	0.1728	824.2	912.8	1001	20000	17170
	height.pred[9]	881.7	881.6	18.76	0.1369	845.0	919.2	1001	20000	18778
	height.pred[10]	890.7	890.6	12.58	0.08731	866.1	915.6	1001	20000	20760
	height.pred.multi[1]	869.9	870.0	11.04	0.07855	848.5	891.7	1001	20000	19765
	height.pred.multi[2]	858.8	858.9	21.28	0.1534	816.4	900.7	1001	20000	19235
	height.pred.multi[3]	847.6	847.5	24.49	0.1679	799.5	896.0	1001	20000	21262
	height.pred.multi[4]	838.2	838.2	22.2	0.1584	794.2	882.0	1001	20000	19650
	height.pred.multi[5]	831.0	831.1	11.74	0.08443	808.1	854.2	1001	20000	19337
	height.pred.multi[6]	838.6	838.7	17.99	0.134	802.6	873.5	1001	20000	18019
	height.pred.multi[7]	853.3	853.4	22.66	0.1614	808.8	898.0	1001	20000	19697
	height.pred.multi[8]	868.3	868.2	22.82	0.1578	823.4	913.5	1001	20000	20913
	height.pred.multi[9]	881.6	881.8	18.67	0.1236	844.8	917.8	1001	20000	22805
	height.pred.multi[10]	890.5	890.5	12.65	0.07767	865.4	915.2	1001	20000	26526
	sigma2	13550.0	13230.0	7400.0	148.5	2595.0	28970.0	1001	20000	2484









The GeoBUGS map tool can be used to produce an approximate map of the posterior mean predicted surface elevation. It is not possible to map point locations using GeoBUGS. However, a map file called elevation is already loaded in the GeoBUGS Map Tool --- this is a 15 x 15 grid with the centroids of the grid cell corresponding to the locations of each of the prediction points. You can use this to produce a map of the posterior mean (or other posterior summaries) of the vector of predicted values height.pred. 

Check of externalization first process

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	height.pred[1]	872.2	872.5	8.472	0.09855	854.6	887.9	1001	10000	7389
	height.pred[2]	861.9	861.9	19.27	0.1954	823.6	899.3	1001	10000	9728
	height.pred[3]	848.8	849.0	23.12	0.2271	802.5	893.7	1001	10000	10368
	height.pred[4]	837.1	837.2	19.86	0.1967	797.9	876.5	1001	10000	10196
	height.pred[5]	829.8	829.8	8.859	0.08161	812.3	847.9	1001	10000	11785
	height.pred[6]	837.2	837.1	15.0	0.137	807.2	867.1	1001	10000	11981
	height.pred[7]	853.1	853.4	21.2	0.1918	810.8	894.7	1001	10000	12228
	height.pred[8]	869.8	870.0	21.11	0.2363	827.6	910.2	1001	10000	7980
	height.pred[9]	884.3	884.6	16.16	0.1805	851.4	915.4	1001	10000	8012
	height.pred[10]	890.9	891.1	9.773	0.09139	871.0	909.5	1001	10000	11435
	height.pred.multi[1]	872.1	872.5	8.22	0.09317	854.3	887.3	1001	10000	7782
	height.pred.multi[2]	861.6	861.8	19.31	0.1916	822.8	899.3	1001	10000	10159
	height.pred.multi[3]	848.9	848.8	23.38	0.2136	803.2	895.8	1001	10000	11977
	height.pred.multi[4]	837.5	837.3	20.38	0.1864	797.9	877.5	1001	10000	11949
	height.pred.multi[5]	830.2	830.0	8.861	0.07929	813.3	848.2	1001	10000	12490
	height.pred.multi[6]	836.8	836.7	15.33	0.1499	806.3	867.7	1001	10000	10458
	height.pred.multi[7]	853.1	853.1	21.07	0.2139	812.0	894.9	1001	10000	9701
	height.pred.multi[8]	870.2	870.2	21.04	0.213	828.7	911.4	1001	10000	9760
	height.pred.multi[9]	884.3	884.3	16.18	0.1826	851.8	916.3	1001	10000	7847
	height.pred.multi[10]	890.8	891.0	9.664	0.1049	871.1	909.5	1001	10000	8482
	kappa	1.401	1.416	0.252	0.006415	0.8745	1.826	1001	10000	1543
	phi	0.2881	0.2788	0.1429	0.005298	0.0642	0.5901	1001	10000	727

second process

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	height.pred[1]	872.2	872.5	8.472	0.09855	854.6	887.9	1001	10000	7389
	height.pred[2]	861.9	861.9	19.27	0.1954	823.6	899.3	1001	10000	9728
	height.pred[3]	848.8	849.0	23.12	0.2271	802.5	893.7	1001	10000	10368
	height.pred[4]	837.1	837.2	19.86	0.1967	797.9	876.5	1001	10000	10196
	height.pred[5]	829.8	829.8	8.859	0.08161	812.3	847.9	1001	10000	11785
	height.pred[6]	837.2	837.1	15.0	0.137	807.2	867.1	1001	10000	11981
	height.pred[7]	853.1	853.4	21.2	0.1918	810.8	894.7	1001	10000	12228
	height.pred[8]	869.8	870.0	21.11	0.2363	827.6	910.2	1001	10000	7980
	height.pred[9]	884.3	884.6	16.16	0.1805	851.4	915.4	1001	10000	8012
	height.pred[10]	890.9	891.1	9.773	0.09139	871.0	909.5	1001	10000	11435
	height.pred.multi[1]	872.1	872.5	8.22	0.09317	854.3	887.3	1001	10000	7782
	height.pred.multi[2]	861.6	861.8	19.31	0.1916	822.8	899.3	1001	10000	10159
	height.pred.multi[3]	848.9	848.8	23.38	0.2136	803.2	895.8	1001	10000	11977
	height.pred.multi[4]	837.5	837.3	20.38	0.1864	797.9	877.5	1001	10000	11949
	height.pred.multi[5]	830.2	830.0	8.861	0.07929	813.3	848.2	1001	10000	12490
	height.pred.multi[6]	836.8	836.7	15.33	0.1499	806.3	867.7	1001	10000	10458
	height.pred.multi[7]	853.1	853.1	21.07	0.2139	812.0	894.9	1001	10000	9701
	height.pred.multi[8]	870.2	870.2	21.04	0.213	828.7	911.4	1001	10000	9760
	height.pred.multi[9]	884.3	884.3	16.18	0.1826	851.8	916.3	1001	10000	7847
	height.pred.multi[10]	890.8	891.0	9.664	0.1049	871.1	909.5	1001	10000	8482
	kappa	1.401	1.416	0.252	0.006415	0.8745	1.826	1001	10000	1543
	phi	0.2881	0.2788	0.1429	0.005298	0.0642	0.5901	1001	10000	727

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	height.pred[1]	872.1	872.5	8.471	0.0695	854.3	887.6	1001	20000	14856
	height.pred[2]	861.8	861.8	19.32	0.1413	822.9	899.0	1001	20000	18699
	height.pred[3]	848.7	848.8	23.12	0.1479	803.0	894.2	1001	20000	24439
	height.pred[4]	837.2	837.3	20.15	0.1408	797.4	876.9	1001	20000	20484
	height.pred[5]	829.9	829.9	8.952	0.05996	812.2	847.8	1001	20000	22293
	height.pred[6]	837.2	837.1	15.21	0.111	807.3	867.3	1001	20000	18769
	height.pred[7]	853.2	853.2	21.08	0.1397	811.0	893.7	1001	20000	22776
	height.pred[8]	869.7	870.1	21.1	0.1596	827.6	910.5	1001	20000	17484
	height.pred[9]	884.1	884.3	16.28	0.1237	851.0	915.6	1001	20000	17311
	height.pred[10]	890.9	891.2	9.883	0.06742	870.7	909.7	1001	20000	21492
	height.pred.multi[1]	872.1	872.5	8.398	0.06729	853.8	887.5	1001	20000	15576
	height.pred.multi[2]	861.6	861.8	19.47	0.1464	822.2	899.5	1001	20000	17668
	height.pred.multi[3]	849.1	849.2	23.34	0.1589	802.7	894.9	1001	20000	21587
	height.pred.multi[4]	837.5	837.3	20.39	0.1384	797.7	877.5	1001	20000	21697
	height.pred.multi[5]	830.2	830.1	8.962	0.06421	812.9	848.2	1001	20000	19480
	height.pred.multi[6]	837.0	836.9	15.39	0.1051	806.5	867.9	1001	20000	21460
	height.pred.multi[7]	853.1	853.2	21.08	0.143	811.3	894.2	1001	20000	21733
	height.pred.multi[8]	870.1	870.2	21.1	0.1392	828.0	911.6	1001	20000	22970
	height.pred.multi[9]	884.1	884.3	16.37	0.128	850.8	915.8	1001	20000	16361
	height.pred.multi[10]	890.9	891.0	9.793	0.07218	871.1	909.8	1001	20000	18406
	kappa	1.392	1.412	0.2531	0.004373	0.8523	1.815	1001	20000	3348
	phi	0.2857	0.2751	0.1399	0.003651	0.06591	0.5818	1001	20000	1467

