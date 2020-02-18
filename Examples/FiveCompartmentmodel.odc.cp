	model {

	solution[1:n.grid, 1:dim] <- ode.solution(init[1:dim], grid[1:n.grid], D(C[1:dim], t), 
											origin, tol)

	D(C[PP], t) <- KI[PP] * C[ART] - KO[PP] * C[PP]
	D(C[RP], t) <- KI[RP] * C[ART] - KO[RP] * C[RP]
	D(C[GU], t) <- KI[GU] * C[ART] - KO[GU] * C[GU]
	D(C[LI], t) <- (QH * C[ART] + KO[GU] * V[GU] * C[GU] + RA - RM) / V[LI] - KO[LI] * C[LI]
	D(C[LU], t) <- KI[LU] * C[VEN] - KO[LU] * C[LU]
	D(C[VEN], t) <- (KO[PP] * V[PP] * C[PP] + KO[RP] * V[RP] * C[RP] + 
	KO[LI] * V[LI] * C[LI]
	- KI[LU] * V[LU] * C[VEN]) / V[VEN]
	D(C[ART], t) <- (KO[LU] * V[LU] * C[LU] - QTOT * C[ART]) / V[ART]

	for (T in PP:LU) {
		KI[T] <- Q[T] / V[T]
		KO[T] <- KI[T] / KP[T]
		KP[T] <- exp(log.KP[T])
	}

	ka <- exp(log.ka)
	RA <- ka * frac * dose * exp(-ka * t)
	RM <- Vmax * C[LI] / (Km + C[LI])

	QH <- Q[LI] - Q[GU]
	QTOT <- Q[LU]

	#	Initial conditions:
	init[PP] <- 0 init[RP] <- 0 init[GU] <- 0 init[LI] <- 0 init[LU] <- 0
	init[VEN] <- 0 init[ART] <- 0

	#	Stochastic model:
	for (i in 1:n.grid) {
		sol_VEN[i] <- solution[i, VEN]
		data[i] ~ dnorm(sol_VEN[i], tau)
	}

	tau ~ dgamma(tau.a, tau.b)

	for (T in PP:LU) {
		log.KP[T] ~ dnorm(log.KP.mean[T], log.param.prec)
		log.KP.mean[T] <- log(data.KP[T])
	}

	log.ka ~ dnorm(log.ka.mean, log.param.prec)
	log.ka.mean <- log(data.ka)

	}
