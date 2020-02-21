
	 Camel: Multivariate normal with
	 structured missing data


Tanner and Wong present a data set with missing values modeled as a bivariate normal.  No closed form for the  deviance of the model can be found and so no deviance node is created by the BUGS compiler. For the same reason the DIC menu is greyed-out 
 	model
	{
		for (i in 1 : N){
			Y[i, 1 : 2] ~ dmnorm(mu[], tau[ , ])
		}
		mu[1] <- 0
		mu[2] <- 0
		tau[1 : 2,1 : 2] ~ dwish(R[ , ], 2)
		R[1, 1] <- 0.001  
		R[1, 2] <- 0
		R[2, 1] <- 0; 
		R[2, 2] <- 0.001
		Sigma2[1 : 2,1 : 2] <- inverse(tau[ , ])
		rho <- Sigma2[1, 2] / sqrt(Sigma2[1, 1] * Sigma2[2, 2])
	}


Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )

Results

		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	Sigma2[1,1]	3.194	2.655	2.075	0.007911	1.12	8.492	1001	200000	68790
	Sigma2[1,2]	-0.03075	-0.05006	2.465	0.02626	-4.696	4.621	1001	200000	8812
	Sigma2[2,1]	-0.03075	-0.05006	2.465	0.02626	-4.696	4.621	1001	200000	8812
	Sigma2[2,2]	3.2	2.661	2.079	0.008246	1.116	8.495	1001	200000	63575
	rho	-0.008481	-0.02377	0.6591	0.008001	-0.9067	0.9063	1001	200000	6786
	tau[1,1]	0.8616	0.7423	0.5124	0.002267	0.2228	2.179	1001	200000	51088
	tau[1,2]	0.007362	0.0102	0.7131	0.007907	-1.398	1.403	1001	200000	8134
	tau[2,1]	0.007362	0.0102	0.7131	0.007907	-1.398	1.403	1001	200000	8134
	tau[2,2]	0.8616	0.7405	0.5149	0.002242	0.2225	2.177	1001	200000	52721

