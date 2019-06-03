model {				# 1

#	PBPK system equations specified via BUGS language:
#	solution[1:n.grid, 1:dim] <- ode(init[1:dim], grid[1:n.grid], D(C[1:dim], t), origin, tol)			# 2
	
#	D(C[PP], t) <- KI[PP] * C[ART] - KO[PP] * C[PP]			# 3
#	D(C[RP], t) <- KI[RP] * C[ART] - KO[RP] * C[RP]			# 4
#	D(C[GU], t) <- KI[GU] * C[ART] - KO[GU] * C[GU]			# 5
#	D(C[LI], t) <- (QH * C[ART] + KO[GU] * V[GU] * C[GU] + RA - RM) / V[LI] - KO[LI] * C[LI]			# 6
#	D(C[LU], t) <- KI[LU] * C[VEN] - KO[LU] * C[LU]			# 7
#	D(C[VEN], t) <- (KO[PP] * V[PP] * C[PP] + KO[RP] * V[RP] * C[RP] + KO[LI] * V[LI] * C[LI]			# 8
#			- KI[LU] * V[LU] * C[VEN]) / V[VEN]	# 9
#	D(C[ART], t) <- (KO[LU] * V[LU] * C[LU] - QTOT * C[ART]) / V[ART]			# 10
	
#	for (T in PP:LU) {			# 11
#		KI[T] <- Q[T] / V[T]		# 12
#		KO[T] <- KI[T] / KP[T]		# 13
#		KP[T] <- exp(log.KP[T])		# 14
#	}			# 15
	
#	ka <- exp(log.ka)			# 16
#	RA <- ka * frac * dose * exp(-ka * t)			# 17
#	RM <- Vmax * C[LI] / (Km + C[LI])			# 18
	
#	QH <- Q[LI] - Q[GU]			# 19
#	QTOT <- Q[LU]			# 20
	
#	Hard-wired (fast) version of same PBPK model:
	solution[1:n.grid, 1:dim] <- diff.five.comp(init[1:dim], grid[1:n.grid], theta[1:n.par], origin, tol)			# 21

	theta[1] <- Q[PP]; theta[2] <- Q[RP]; theta[3] <- Q[GU]; theta[4] <- Q[LI]; theta[5] <- Q[LU]			# 22
	theta[6] <- V[PP]; theta[7] <- V[RP]; theta[8] <- V[GU]; theta[9] <- V[LI]; theta[10] <- V[LU]			# 23
	theta[11] <- V[VEN]; theta[12] <- V[ART]			# 24
	theta[13] <- log.KP[PP]; theta[14] <- log.KP[RP]; theta[15] <- log.KP[GU]			# 25
	theta[16] <- log.KP[LI]; theta[17] <- log.KP[LU]			# 26
	theta[18] <- Vmax; theta[19] <- Km; theta[20] <- dose; theta[21] <- frac; theta[22] <- log.ka			# 27
	
#	Initial conditions:
	init[PP] <- 0; init[RP] <- 0; init[GU] <- 0; init[LI] <- 0; init[LU] <- 0; init[VEN] <- 0; init[ART] <- 0			# 28
	
#	Stochastic model:
	for (i in 1:n.grid) {			# 29
		data[i] ~ dnorm(solution[i, VEN], tau)		# 30
	}			# 31

	
	tau ~ dgamma(tau.a, tau.b)			# 33
	
	for (T in PP:LU) {			# 34
		log.KP[T] ~ dnorm(log.KP.mean[T], log.param.prec)		# 35
		log.KP.mean[T] <- log(data.KP[T])		# 36
	}			# 37
	
	log.ka ~ dnorm(log.ka.mean, log.param.prec)			# 38
	log.ka.mean <- log(data.ka)			# 39
	
}				# 40

Inference data:
list(
PP = 1, RP = 2, GU = 3, LI = 4, LU = 5, VEN = 6, ART = 7,
n.grid = 12,
dim = 7,
origin = 0,
tol = 1.0E-3,
grid = c(0.1, 0.2, 0.5, 1, 2, 3, 4, 8, 12, 24, 48, 72),
data = c(0.005103,0.02779,0.07123,0.09213,0.1235,0.1186,
0.1149,0.09567,0.0983,0.05143,0.009843,0.02112),
Q = c(45, 30, 20, 25, 120),
V = c(100, 5, 5, 5, 1, 5, 3),
data.KP = c(10, 2, 2, 2, 2),
dose = 100,
frac = 1,
data.ka = 0.1,
Vmax = 10, Km = 10,
n.par = 22,
tau.a = 0.001,
tau.b = 0.001,
log.param.prec = 13.81
)

Initial values:
list(tau = 10000)
list(tau = 1)



# node statistics as described in the documentation 
		mean	sd	MC_error	val2.5pc	median	val97.5pc	start	sample
	log.KP[1]	2.168	0.2527	0.01568	1.719	2.152	2.692	4000	4001
	log.KP[2]	0.7067	0.2682	0.01682	0.1926	0.7122	1.23	4000	4001
	log.KP[3]	0.6976	0.2552	0.0154	0.2159	0.6979	1.216	4000	4001
	log.KP[4]	0.6622	0.2297	0.01507	0.1918	0.672	1.099	4000	4001
	log.KP[5]	0.6821	0.277	0.01788	0.1193	0.6897	1.175	4000	4001
	log.ka	-2.243	0.1071	0.006542	-2.471	-2.237	-2.052	4000	4001
	tau	3473.0	1549.0	43.6	1191.0	3249.0	7033.0	4000	4001



# plot as described in the documentation 



