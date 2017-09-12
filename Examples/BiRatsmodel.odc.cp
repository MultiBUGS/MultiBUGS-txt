	model
	{
		for( i in 1 : N ) {
			beta[i , 1 : 2] ~ dmnorm(mu.beta[], R[ , ])
			for( j in 1 : T ) {
				Y[i, j] ~ dnorm(mu[i , j], tauC)
				mu[i, j] <- beta[i, 1] + beta[i, 2] * x[j]
			}
		}

		mu.beta[1 : 2] ~ dmnorm(mean[], prec[ , ])
		R[1 : 2 , 1 : 2] ~ dwish(Omega[ , ], 2)
		tauC ~ dgamma(0.001, 0.001)
		sigma <- 1 / sqrt(tauC)
	}

