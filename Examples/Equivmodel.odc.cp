	model
	{
		for( k in 1 : P ) {
			for( i in 1 : N ) {
				Y[i , k] ~ dnorm(m[i , k], tau1)
				m[i , k] <- mu + sign[T[i , k]] * phi / 2 + sign[k] * pi / 2 + delta[i]
				T[i , k] <- group[i] * (k - 1.5) + 1.5
			}
		}
		for( i in 1 : N ) {
			delta[i] ~ dnorm(0.0, tau2)
		}
		tau1 ~ dgamma(0.001, 0.001) sigma1 <- 1 / sqrt(tau1)
		tau2 ~ dgamma(0.001, 0.001) sigma2 <- 1 / sqrt(tau2)
		mu ~ dnorm(0.0, 1.0E-6)
		phi ~ dnorm(0.0, 1.0E-6)
		pi ~ dnorm(0.0, 1.0E-6)
		theta <- exp(phi)
		equiv <- step(theta - 0.8) - step(theta - 1.2)
	}

