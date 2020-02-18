	Five compartments: A complex
								 differential equation model

This example makes use of a physiologically based pharmacokinetic (PBPK) model for a hepatically cleared drug given orally to some hypothetical animal. The purpose of this model is to demonstrate the OpenBUGS Differential Equation Interface – all of the physiological parameter values specified are arbitrary. The model is depicted in the figure below.

Physiologically based pharmacokinetic model for a hypothetical, orally administered, hepatically cleared drug. Note that orally administered drug is generally absorbed across the gut wall into the hepatic portal vein. In order to separate any drug that is being absorbed thus from that which is recirculating into the hepatic portal vein, via the tissues of the gut, we tend
to assume that as far as the model is concerned oral absorption processes input drug directly into the “LIVER” compartment, as shown, rather than the “GUT” compartment.

It can be described mathematically via:

		dCPP/dt	= KIPPCART − KOPPCPP
		
		dCRP/dt	= KIRPCART − KORPCRP
		
		dCGU/dt	= KIGUCART − KOGUCGU
		
		dCLI/dt	= (QHCART + KOGUVGUCGU + RA − RM) / VLI − KOLICLI
		
		dCLU/dt	= KILUCV EN − KOLUCLU
		
		dCV EN/dt	= (KOPP VPPCPP + KORP VRPCRP + KOLIVLICLI 	− KILUVLUCV EN) / VV EN
		
		dCART/dt	= (KOLUVLUCLU − QTOTCART ) / VART


Here C. denotes the concentration of drug within the indicated compartment, KIT = QT /VT and KOT = QT /VTKPT , where QT , VT and KPT denote the rate of blood flow, the volume, and the tissue-to-blood partition coefficient associated with tissue/compartment T, respectively. In addition, QH represents the liver’s arterial blood supply (QH = QLI − QGU; see figure) and QTOT is the total cardiac output, which is equal to QLU as all blood must flow through the lungs to be oxygenated. (Note that for mass balance we must also have that QTOT = QLU = QPP + QRP + QGU + QH.) Also, for this particular example, the rate of absorption is given by 
RA = ka F Dose exp(−kat) (first-order absorption) and the rate of metabolism, RM, is given by Vmax CLI   /(Km + CLI) (Michaelis-Menten metabolism), where ka, F, Vmax and Km are the first-order absorption rate constant, the fraction of the administered dose that enters the liver, the maximum rate of metabolism, and the concentration of drug in the liver at which exactly half the maximum rate of metabolism occurs, respectively.

In the BUGS language the model is


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


Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )

Results using adaptive block updating with delayed rejection
	
		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	log.KP[1]	2.163	2.158	0.2416	0.005251	1.697	2.645	1001	20000	2116
	log.KP[2]	0.7044	0.7056	0.2738	0.005685	0.1689	1.228	1001	20000	2319
	log.KP[3]	0.6964	0.6929	0.2693	0.006002	0.1788	1.222	1001	20000	2012
	log.KP[4]	0.6788	0.6809	0.2289	0.005095	0.228	1.127	1001	20000	2017
	log.KP[5]	0.6913	0.6913	0.2714	0.006252	0.166	1.226	1001	20000	1884
	log.ka	-2.237	-2.238	0.1056	0.002265	-2.447	-2.025	1001	20000	2173
	sol_VEN[1]	0.01281	0.01263	0.002292	4.95E-5	0.008912	0.01789	1001	20000	2143
	sol_VEN[2]	0.02944	0.02916	0.004648	9.944E-5	0.02126	0.03946	1001	20000	2184
	sol_VEN[3]	0.06542	0.06521	0.007517	1.551E-4	0.051	0.08098	1001	20000	2347
	sol_VEN[4]	0.09689	0.09674	0.008491	1.705E-4	0.08021	0.1139	1001	20000	2481
	sol_VEN[5]	0.1181	0.118	0.008907	1.798E-4	0.1008	0.1357	1001	20000	2454
	sol_VEN[6]	0.12	0.1201	0.008378	1.681E-4	0.1038	0.1366	1001	20000	2482
	sol_VEN[7]	0.116	0.116	0.007478	1.479E-4	0.1014	0.1307	1001	20000	2557
	sol_VEN[8]	0.09257	0.09236	0.005524	1.1E-4	0.08255	0.1041	1001	20000	2521
	sol_VEN[9]	0.07476	0.0744	0.006218	1.344E-4	0.06365	0.08784	1001	20000	2139
	sol_VEN[10]	0.04647	0.04633	0.006375	1.444E-4	0.03452	0.05932	1001	20000	1949
	sol_VEN[11]	0.02636	0.02652	0.002756	6.268E-5	0.02055	0.03133	1001	20000	1933
	sol_VEN[12]	0.01712	0.01719	0.001063	2.237E-5	0.01471	0.01902	1001	20000	2256
	tau	3507.0	3270.0	1527.0	16.33	1196.0	7100.0	1001	20000	8743



