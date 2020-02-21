	Change points: A piecewise
								 differential equation model

All of the previous examples have first derivatives which are smooth functions of time (i.e. the second derivatives, with respect to time, are all continuous). OpenBUGS is capable of handling situations where this is not the case but it requires additional information regarding the times at which discontinuities occur. It needs this information so that it can break up the problem
into “blocks” of time where, throughout each block, all second derivatives are continuous. Because specifying such models is somewhat more complicated than with their smooth counterparts, OpenBUGS provides an additional BUGS-language component (ode.block) for solving these problems. The example from this section is also provided as a compiled component (DiffChangePoints).

Population PK Model
Our model for this example is designed to illustrate several features of ‘advanced’ OpenBUGS use, such as the specification of population models, handling discontinuities in time, and allowing compartments to empty into each other. In pharmacokinetics, a population model is required when, in order to draw inferences about the target population, a study is conducted whereby a number of healthy volunteers or patients each receive one or more doses of the drug under investigation and concentration measurements are taken from each one. Typically we have a (parametric) structural model that describes the shape of the concentration-time profile and we model variability throughout the study population by allowing the parameters of that structural model to differ between individuals. The structural model for this example is shown in the figure.

				

 The human body is represented by a single compartment (“Compartment 1”) and as such is assumed to be homogeneous – the blood and all organs/tissues contain the same concentration of drug, C, which is given by the total amount of drug in the body at time t, A1 = A1(t), divided by an apparent volume of distribution, V . Drug is eliminated (or cleared) from the body via a ‘flow-rate’ of CL (clearance) – the rate of elimination, i.e. the rate at which drug leaves the body, is given by dAel / dt = CL¡ C = CL¡ A1 / V . Compartments 2 and 3 contain doses of drug to be administered, to Compartment 1, at times t = tb2 and t = tzo, respectively. These have been incorporated purely for illustrative purposes – they are actually unnecessary as we could easily specify the model using Compartment 1 alone (with an appropriate sequence of initial conditions for each block of time). The full sequence of events that generates our model is described as follows. At time t = 0, the volunteer/patient receives an intravenous bolus dose 1 of size D. At time t = tb2 > 0 a second bolus, of the same size, is administered. This is the process represented by the dashed arrow from Compartments 2 to 1 in the figure – dashed arrows represent processes whereby one compartment empties into another instantaneously at the specified time (t = tb2 in this case) whereas normal arrows denote rates of movement of drug. At time t = tzo > tb2 a zero-order process is initiated between Compartments 3 and 1 whereby the dose initially in Compartment 3 (again, D) is transferred to Compartment 1 at a constant rate given by D/TI, where TI denotes the duration of the zero-order input process. Clearly this process must terminate at t = tzo + TI. All of this means that the somewhat abstract quantity R31 in the figure , which simply represents the rate of movement of drug from Compartments 3 to 1, is a piecewise smooth function of time. Explicitly, it is given by

		
More generally, we can write R31 = pw(v, o, t), where v and o are vectors containing each smooth function and the times at which they begin, respectively; in this case, v = (0, 0,D/TI, 0), o = (0, tb2, tzo, tzo +TI) and pw = vb, where b =Xi i  I(t ∈ [oi, oi+1)) (o5 = ∞), that is, b simply specifies which ‘block’ of time, defined by a pair of ‘changepoints’, contains the current value of t. OpenBUGS provides a new BUGSlanguage component called piecewise(.) that can be used for specifying arbitrary piecewise-smooth functions within differential equation models. Only the vector v is required as an input parameter, however. This is because the ‘origins’ in o are passed into the associated ode.block(.) component instead, so that it knows where to break the problem up into ‘smooth’ sub-problems (t is defined/controlled by the ODE solving algorithm). For this reason, the o vector passed into ode.block(.) must comprise the union of all ‘changepoints’ in the model and each instance of piecewise(.) must be defined in terms of a smooth function vector (v) of the same length. This is why R31 above incorporates components for both [0, tb2) and [tb2, tzo) even though it has the same value throughout both intervals: ode.block(.) needs to know that a discrete event occurs at t = tb2 and so all of its piecewise parents (just R31 in this case) must be ‘split’ accordingly.

The system equations are as follows:

			dA1/dt	= R31(t) − CL  A1  / V
			
			dA2/dt 	= 0
			
			dA3/	dt = −R31(t)
			
(Note that neither bolus dose is represented in the system equations. This is because bolus doses are instantaneous and are thus best described via the initial conditions, as we discuss below.) The structural model is completed by the specification of a sequence of initial conditions for each compartment. First in the sequence are the initial conditions proper, which pertain to the time origin given by o1. Note that the easiest way in which to model the first intravenous bolus dose is to simply specify A1(o1) = A1(0) = D in the initial conditions. At each subsequent origin (e.g. o2, o3, o4) we may, generally speaking, wish to apply an instantaneous adjustment to the system to account for certain types of discrete event that may have occurred, such as the administration of a bolus dose (which cannot be represented by a finite rate). In OpenBUGS we specify such adjustments by declaring an amount (of whatever quantity the differential equations are to be solved for) to be added to each compartment at the appropriate time. If no adjustment is specified (or if a value of zero is declared) then the relevant part of the system is left unchanged. Thus for the example above, we specify the following matrix of ‘initial conditions’, where rows correspond to origins (o1, o2, o3, o4) and the columns represent compartments:

					
The −A2 in column 2 represents the change to the solution to the second differential equation at the relevant time, i.e. t = tb2 (the second ‘origin’). Thus the amount of drug in Compartment 2 instantaneously changes from A2(tb2−) to zero at t = tb2 (because its value is reduced by A2(tb2−)). In contrast, the amount of drug in Compartment 1 changes from A1(tb2−) to A1(tb2−)+A2(tb2−). In other words, Compartment 2 instantaneously empties into Compartment 1 at time t = tb2. Since Compartment 2 initially contains an amount D of drug and dA2 dt ≡ 0, this is equivalent to an intravenous bolus dose being administered, to Compartment 1, at t = tb2. The statistical model is defined as follows. First note that the measurable quantity here is concentration of drug in Compartment 1, i.e. C = A1/V . This is a function of the dose D, time t, and additional parameters, namely CL, V and TI, that we collectively denote by : C = C(, t,D). For this example we will allow both  and D to vary between individuals, although the doses are assumed known, as they would be in practice (normally). Suppose we have n concentration measurements, indexed by j, from each of K individuals, indexed by i. We denote these by yij , i = 1, ...,K, j = 1, ..., n. Often in pharmacokinetics, the size of the error associated with each concentration measurement is proportional to the underlying true concentration and so we tend to model the natural logarithm of the data as a function of log C. At the first stage of the statistical model we assume

	 log yij = log C(i, tij ,Di) + eij , i = 1, ...,K, j = 1, ..., n,

where tij denotes the time at which yij  was collected and the eij terms are independent and identically distributed normal random variables with mean zero and variance t−1 (i.e. precision =  t). At the second stage of the model we have

	Qi ~ MVN3(m,Ω), i = 1, ...,K,
	
where  and Ω are the population mean and the variance-covariance of pharmacokinetic
parameters, respectively. At the final stage of our population model, appropriate multivariate normal, Wishart and gamma priors are specified for m, Ω−1 and t, respectively.

In the BUGS language the model is

	model {
		for (i in 1:n.ind) {
			for (j in 1:n.grid) {
				log.data[i, j] ~ dnorm(log.model[i, j], tau)
				log.model[i, j] <- log(model[i, j])
				model[i, j] <-  solution[i, j, 1] / V[i]
			}
			solution[i, 1:n.grid, 1:dim] <- ode.block(inits[i, 1:2, 1:dim],                                      
				                 grid[1:n.grid], D(A[i, 1:dim], t[i]), origins[i, 1:n.block], tol[])
				
			D(A[i, 1], t[i]) <- R31[i] - CL[i] * A[i, 1] / V[i]
			D(A[i, 2], t[i]) <- 0
			D(A[i, 3], t[i]) <- -R31[i]
			R31[i] <- piecewise(vec.R31[i, 1:n.block])
			vec.R31[i, 1] <- 0
			vec.R31[i, 2] <- 0
			vec.R31[i, 3] <- dose[i] / TI[i]
			vec.R31[i, 4] <- 0
			
			CL[i] <- exp(theta[i, 1])
			V[i] <- exp(theta[i, 2])
			TI[i] <- exp(theta[i, 3])
			
			theta[i, 1:p] ~ dmnorm(mu[1:p], omega.inv[1:p, 1:p])

			inits[i, 1, 1] <- dose[i]
			inits[i, 1, 2] <- dose[i]
			inits[i, 1, 3] <- dose[i]
			inits[i, 2, 1] <- A[i, 2]
			inits[i, 2, 2] <- -A[i, 2]
			inits[i, 2, 3] <- 0
			
			origins[i, 1] <- 0
			origins[i, 2] <- second.bolus.time
			origins[i, 3] <- zo.start.time
			origins[i, 4] <- zo.start.time + TI[i]
		}

		#hyper priors
		mu[1:p] ~ dmnorm(mu.prior.mean[1:p], mu.prior.prec[1:p, 1:p])
		omega.inv[1:p, 1:p] ~ dwish(omega.inv.matrix[1:p, 1:p], omega.inv.dof)
		omega[1:p, 1:p] <- inverse(omega.inv[1:p, 1:p])
		tau ~ dgamma(0.001, 0.001)
	}


Data	( click to open )

Inits for chain 1    	Inits for chain 2	 ( click to open )

Results


		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	mu[1]	0.9614	0.9613	0.04242	5.28E-4	0.8762	1.045	2001	20000	6453
	mu[2]	1.946	1.946	0.03133	3.63E-4	1.885	2.008	2001	20000	7452
	mu[3]	-0.1746	-0.1533	0.2283	0.01351	-0.6878	0.218	2001	20000	285
	omega[1,1]	0.01733	0.01478	0.01029	1.172E-4	0.006357	0.04287	2001	20000	7713
	omega[1,2]	-0.003215	-0.002641	0.005131	5.467E-5	-0.01454	0.005195	2001	20000	8807
	omega[1,3]	0.0143	0.008009	0.02548	0.001096	-0.01542	0.08155	2001	20000	540
	omega[2,2]	0.008829	0.00753	0.005288	6.017E-5	0.003165	0.02237	2001	20000	7722
	omega[2,3]	-0.005305	-0.002886	0.01299	4.19E-4	-0.03715	0.01285	2001	20000	961
	omega[3,3]	0.07371	0.032	0.1291	0.006089	0.004875	0.42	2001	20000	449
	tau	93.67	93.22	12.28	0.2202	71.22	119.3	2001	20000	3110


Data (simulation):
list(
p = 3, dim = 3,
tol = 1.0E-6,
n.ind = 10, n.grid = 14, n.block = 4,
grid = c(0.05, 0.1, 0.2, 0.4, 0.6, 1.0, 1.5, 2, 3, 4, 8, 12, 16, 24),
dose = c(100, 95, 90, 85, 80, 75, 70, 65, 60, 55),
second.bolus.time = 2, zo.start.time = 8,
tau = 100,
mu = c(1, 2, 0),
omega.inv = structure(
.Data = c(
100, 0, 0,
0, 100, 0,
0, 0, 100),
.Dim = c(3, 3))
)


		mean	median	sd	MC_error	val2.5pc	val97.5pc	start	sample	ESS
	mu[1]	0.9608	0.961	0.04307	8.031E-4	0.876	1.046	2001	10000	2875
	mu[2]	1.946	1.946	0.03179	5.681E-4	1.884	2.01	2001	10000	3131
	mu[3]	-0.1812	-0.1489	0.2566	0.021	-0.8321	0.2276	2001	10000	149
	omega[1,1]	0.01739	0.01478	0.01029	1.676E-4	0.006415	0.04353	2001	10000	3773
	omega[1,2]	-0.003268	-0.002778	0.005045	7.311E-5	-0.0151	0.005247	2001	10000	4761
	omega[1,3]	0.01512	0.008399	0.02672	0.001462	-0.01356	0.08375	2001	10000	333
	omega[2,1]	-0.003268	-0.002778	0.005045	7.311E-5	-0.0151	0.005247	2001	10000	4761
	omega[2,2]	0.008861	0.007541	0.005261	7.351E-5	0.003257	0.02278	2001	10000	5121
	omega[2,3]	-0.005532	-0.002809	0.01437	5.538E-4	-0.03858	0.01257	2001	10000	673
	omega[3,1]	0.01512	0.008399	0.02672	0.001462	-0.01356	0.08375	2001	10000	333
	omega[3,2]	-0.005532	-0.002809	0.01437	5.538E-4	-0.03858	0.01257	2001	10000	673
	omega[3,3]	0.07787	0.03082	0.1504	0.009145	0.004836	0.4485	2001	10000	270
	tau	93.99	93.5	12.33	0.234	71.31	119.8	2001	10000	2774



