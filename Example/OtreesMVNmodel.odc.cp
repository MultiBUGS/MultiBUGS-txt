	model {
		for (i in 1:K) {
			for (j in 1:n) {
				Y[i, j] ~ dnorm(eta[i, j], tauC)
				eta[i, j] <- phi[i, 1] / (1 + phi[i, 2] * exp(phi[i, 3] * x[j]))
			}
			phi[i, 1] <- exp(theta[i, 1])
			phi[i, 2] <- exp(theta[i, 2]) - 1
			phi[i, 3] <- -exp(theta[i, 3])
			theta[i, 1:3] ~ dmnorm(mu[1:3], tau[1:3, 1:3])
		}
		mu[1:3] ~ dmnorm(mean[1:3], prec[1:3, 1:3])
		tau[1:3, 1:3] ~ dwish(R[1:3, 1:3], 3)
		sigma2[1:3, 1:3] <- inverse(tau[1:3, 1:3]) 
		for (i in 1 : 3) {sigma[i] <- sqrt(sigma2[i, i]) }
		tauC ~ dgamma(1.0E-3, 1.0E-3)
		sigmaC <- 1 / sqrt(tauC)
	}

