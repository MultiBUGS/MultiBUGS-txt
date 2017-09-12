	model
	{
		for (i in 1 : Dogs) {
			xa[i, 1] <- 0; xs[i, 1] <- 0 p[i, 1] <- 0 
			for (j in 2 : Trials) {
				xa[i, j] <- sum(Y[i, 1 : j - 1])
				xs[i, j] <- j - 1 - xa[i, j]
				log(p[i, j]) <- alpha * xa[i, j] + beta * xs[i, j]
				y[i, j] <- 1 - Y[i, j]
				y[i, j] ~ dbern(p[i, j])
			}
		} 
		alpha ~ dflat()T(, -0.00001)
		beta ~ dflat()T(, -0.00001)
		A <- exp(alpha)
		B <- exp(beta)
	}

