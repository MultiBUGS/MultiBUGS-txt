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

