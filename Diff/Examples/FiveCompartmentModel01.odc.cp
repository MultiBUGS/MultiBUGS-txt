model {				# 1

#	PBPK system equations specified via BUGS language:
	solution[1:n.grid, 1:dim] <- ode(init[1:dim], grid[1:n.grid], D(C[1:dim], t), origin, tol)			# 2
	
	D(C[PP], t) <- KI[PP] * C[ART] - KO[PP] * C[PP]			# 3
	D(C[RP], t) <- KI[RP] * C[ART] - KO[RP] * C[RP]			# 4
	D(C[GU], t) <- KI[GU] * C[ART] - KO[GU] * C[GU]			# 5
	D(C[LI], t) <- (QH * C[ART] + KO[GU] * V[GU] * C[GU] + RA - RM) / V[LI] - KO[LI] * C[LI]			# 6
	D(C[LU], t) <- KI[LU] * C[VEN] - KO[LU] * C[LU]			# 7
	D(C[VEN], t) <- (KO[PP] * V[PP] * C[PP] + KO[RP] * V[RP] * C[RP] + KO[LI] * V[LI] * C[LI]			# 8
			- KI[LU] * V[LU] * C[VEN]) / V[VEN]	# 9
	D(C[ART], t) <- (KO[LU] * V[LU] * C[LU] - QTOT * C[ART]) / V[ART]			# 10
	
	for (T in PP:LU) {			# 11
		KI[T] <- Q[T] / V[T]		# 12
		KO[T] <- KI[T] / KP[T]		# 13
		KP[T] <- exp(log.KP[T])		# 14
	}			# 15
	
	ka <- exp(log.ka)			# 16
	RA <- ka * frac * dose * exp(-ka * t)			# 17
	RM <- Vmax * C[LI] / (Km + C[LI])			# 18
	
	QH <- Q[LI] - Q[GU]			# 19
	QTOT <- Q[LU]			# 20
	
#	Hard-wired (fast) version of same PBPK model:
#	solution[1:n.grid, 1:dim] <- diff.five.comp(init[1:dim], grid[1:n.grid], theta[1:n.par], origin, tol)			# 21

#	theta[1] <- Q[PP]; theta[2] <- Q[RP]; theta[3] <- Q[GU]; theta[4] <- Q[LI]; theta[5] <- Q[LU]			# 22
#	theta[6] <- V[PP]; theta[7] <- V[RP]; theta[8] <- V[GU]; theta[9] <- V[LI]; theta[10] <- V[LU]			# 23
#	theta[11] <- V[VEN]; theta[12] <- V[ART]			# 24
#	theta[13] <- log.KP[PP]; theta[14] <- log.KP[RP]; theta[15] <- log.KP[GU]			# 25
#	theta[16] <- log.KP[LI]; theta[17] <- log.KP[LU]			# 26
#	theta[18] <- Vmax; theta[19] <- Km; theta[20] <- dose; theta[21] <- frac; theta[22] <- log.ka			# 27
	
#	Initial conditions:
	init[PP] <- 0; init[RP] <- 0; init[GU] <- 0; init[LI] <- 0; init[LU] <- 0; init[VEN] <- 0; init[ART] <- 0			# 28
	
#	Stochastic model:
	for (i in 1:n.grid) {			# 29
		data[i] ~ dnorm(solution[i, VEN], tau)		# 30
	}			# 31
	
	tau <- tau.simulate			# 33
	
	for (T in PP:LU) {			# 34
		log.KP[T] ~ dnorm(log.KP.mean[T], log.param.prec)		# 35
		log.KP.mean[T] <- log(data.KP[T])		# 36
	}			# 37
	
	log.ka ~ dnorm(log.ka.mean, log.param.prec)			# 38
	log.ka.mean <- log(data.ka)			# 39
	
}				# 40

Simulation data:
list(
PP = 1, RP = 2, GU = 3, LI = 4, LU = 5, VEN = 6, ART = 7,
n.grid = 12,
dim = 7,
origin = 0,
tol = 1.0E-3,
grid = c(0.1, 0.2, 0.5, 1, 2, 3, 4, 8, 12, 24, 48, 72),
Q = c(45, 30, 20, 25, 120),
V = c(100, 5, 5, 5, 1, 5, 3),
data.KP = c(10, 2, 2, 2, 2),
dose = 100,
frac = 1,
data.ka = 0.1,
Vmax = 10, Km = 10,
tau.simulate = 10000,
log.param.prec = 13.81
)




# summary statistics as described in documentation 

   node           mean	            sd	          MC_error         val2.5pc        median      val97.5pc    	start     sample
log.KP[1]	       2.298	        0.2677	       0.007721           1.776             2.292         2.828              1        1000
log.KP[2]	       0.6857      	0.2675	      0.009519           0.1909          0.6854        1.261              1        1000
log.KP[3]	       0.6824      	0.2716	      0.008179           0.1682          0.6865        1.215              1        1000
log.KP[4]       	0.6973	      0.2658	      0.00786             0.1621          0.7002        1.211              1        1000
log.KP[5]	       0.6968	      0.2702      	0.007572           0.1814          0.6986        1.243              1        1000
log.ka	           -2.302	        0.265	         0.00681            -2.839            -2.291         -1.81               1        1000

# plot as described in documentation 






# data from 'save state' as described in the documentation 

list(
data = c(0.005103,0.02779,0.07123,0.09213,0.1235,0.1186,
0.1149,0.09567,0.0983,0.05143,0.009843,0.02112),
log.KP = c(2.004,1.236,0.6129,1.035,0.7959),
log.ka = -2.188)



