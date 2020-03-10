	Functions and functionals

Contents

	Introduction
	Scalar functions
	Vector functions
	Properties of distributions
	
	
Introduction        [top]

Function arguments represented by e can be expressions, those by s must be scalar-valued nodes in the graph and those represented by v must be vector-valued nodes in a graph. Some function arguments must be stochastic nodes. Functionals are described using a similar notation to functions, the special notation F(x) is used to describe the function on which the functional acts. See example Functionals for details.

Systems of ordinary differential equations and their solution can be described in the BUGS language by using the special D(x[1:n], t) notation. See example ode for details. 

Scalar functions        [top]

abs(e)	absolute value of e, |e|

arccos(e)	inverse cosine of e

arccosh(e)	inverse hyperbolic cosine of e

arcsin(e)	inverse sine of e

arcsinh(e)	inverse hyperbolic sine of e

arctan(e)	inverse tangent of e

arctanh(e)	inverse hyperbolic tangent of e

cloglog(e)	complementary log log of e, ln(-ln(1 - e))

cos(e)	cosine of e

cosh(e)	hyperbolic cosine of e

cut(e)	cuts edges in the graph - see Tip and tricks page for details.

equals(e1, e2)	1 if value of e1 equals value of  e2; 0 otherwise

exp(e)	exp(e)

gammap(s1, s2)	partial (incomplete) gamma function, value of standard gamma density with parameter s1 integrated up to s2

ilogit(e)	exp(e) / (1 + exp(e))

icloglog(e)	1 - exp( - exp(e))

integral(F(s), s1, s2, s3)	definite integral of function F(s) between s = s1 and s = s2 to accuracy s3

log(e)	natural logarithm of e

logfact(e)	ln(e!)

loggam(e)	logarithm of gamma function of e

logit(e)	ln(e / (1 - e))

max(e1, e2)	e1 if e1 > e2; e2 otherwise

min(e1, e2)	e1 if e1 < e2; e2 otherwise

phi(e)	standard normal cdf

post.p.value(s)	s must be a stochastic node, returns one if a sample from the prior is less than the value of s

pow(e1, e2)	e1e2 

prior.p.value(s)	s must be a stochastic node, returns one if a sample from the prior after resampling its stochastic parents is less than value of s.

probit(e)	inverse of phi(e)

replicate.post(s)	replicate from distribution of s, s must be stochastic node

replicate.prior.(s)	replicate from distribution of s after replicating from it parents if they are stochastic, s must be stochastic node

round(e)	nearest integer to e

sin(e)	sine of e

sinh(e)	hyperbolic sine of e

solution(F(s), s1, s2, s3)	a solution of equation F(s) = 0 lying between s = s1 and s = s2 to accuracy s3, s1 and s2 must bracket a solution

sqrt(e)	e1/2 

step(e)	1 if e >= 0; 0 otherwise

tan(e)	tangent of e

tanh(e)	hyperbolic tangent of e

trunc(e)	greatest integer less than or equal to e


Vector functions        [top]

inprod(v1, v2)	inner product Si  v1i v2i  of v1 and v2 

interp.lin(e, v1, v2)	Given function values in the vector v2 evaluated at the points in v1, this estimates the function value at a new point e by simple linear interpolation using the closest bounding pair of points. For example, given the population in 1991,2001 and 2011, we might want to estimate the population in 2004. Specifically, v2p + (v2p + 1 - v2p) * (e - v1p) / (v1p + 1 - v1p), where p is such that v1p < e < v1p + 1 and the elements of v1 are in ascending order

inverse(v)	inverse of symmetric positive-definite matrix v

logdet(v)	log of determinant of v for symmetric positive-definite v

mean(v)	Si  vi / n,    where n = dim(v)

eigen.vals(v)	eigenvalues of matrix v

ode(v1, v2, D(v3, s1), s2, s3)	solution of system of ordinary differential equations at grid of points v2 given initial values v1  at time s2 solved to accuracy s3. v3 is a vector of components of the system of ode and s1 is the time variable. See the PDF files in the Diff/Docu directory of the BUGS installation for further details.

prod(v)	Pi  vi 

p.valueM(v)	v must be a multivariate stochastic node, returns a vector of ones and zeros depending on if a sample from the prior is less than value of the corresponding component of v

rank(v, s)	number of components of v less than or equal to s

ranked(v, s)	the sth smallest component of v

replicate.postM(v)	replicate from multivariate distribution of v, v must be stochastic and multivariate

sd(v)	standard deviation of components of v (n - 1 in denominator)

sort(v)	vector v sorted in ascending order

sum(v)	Si  vi


Properties of distributions        [top]

The cumulative distribution function (CDF), probability density function (PDF) and deviance for the distributions available in BUGS can be calculated using the cdf.dist, pdf.dist and dev.dist family of functions, where dist corresponds to one of the distributions listed above. The first argument of each function is the value at which to evaluate the function. The other arguments are the parameters of the distribution itself. For example, for the normal distribution:

cdf.norm(s, mu, tau)	cumulative distribution function at the value of s for a normal distribution with parameters mu and tau

pdf.norm(s, mu, tau)	probability density of distribution at value of s for a normal distribution with parameters mu and tau

dev.norm(s, mu tau)	deviance of distribution at value of s for a normal distribution with parameters mu and tau

The following cumulative distribution functions are available: cdf.bern, cdf.beta,  cdf.bin, cdf.cat, cdf.chisqr, cdf.dexp, cdf.exp, cdf.f, cdf.gamma,  cdf.ggamma, cdf.geom0, cdf.geom1, cdf.hyper, cdf.lnorm, cdf.logis,  cdf.negbin, cdf.norm, cdf.par, cdf.pois, cdf.polygene, cdf.t,  cdf.trap,  cdf.triang, cdf.unif, cdf.weib, cdf.weib3, cdf.gev, cdf.gpar. No cumulative distribution function is available for flat (dflat), generic (dloglik), stable (dstable) or Zipf (dzipf) distributions.

The following probability density functions are available for univariate distributions: pdf.bern, pdf.beta, pdf.bin, pdf.cat, pdf.chisqr, pdf.dexp, pdf.exp, pdf.f, pdf.flat, pdf.gamma, pdf.ggamma, pdf.geom0, pdf.geom1, pdf.hyper, pdf.lnorm, pdf.logis, pdf.negbin, pdf.norm, pdf.par, pdf.pois, pdf.polygene, pdf.t, pdf.trap, pdf.triang, pdf.unif, pdf.weib, pdf.weib3, pdf.gev, pdf.gpar, pdf.zipf. No probability distribution function is available for generic (dloglik) or stable (dstable) distributions.

For multivariate distributions the following probability density functions are available: pdf.dirich, pdf.multi, pdf.mnorm, pdf.mt, pdf.wish

The following functions calculate the deviance of the corresponding distribution: dev.bern, dev.beta, dev.bin, dev.cat, dev.chisqr, dev.dexp, dev.exp, dev.f, dev.flat, dev.gamma, dev.ggamma, dev.geom0, dev.geom1, dev.hyper, dev.lnorm, dev.logis, dev.negbin, dev.norm, dev.par, dev.pois, dev.polygene, dev.t, dev.trap, dev.triang, dev.unif, dev.weib, dev.weib3, dev.gev, dev.gpar, dev.zipf.  No deviance function is available for generic (dloglik) or stable (dstable) distributions.

For multivariate distributions, the following functions calculate deviance: dev.dirich, dev.multi, dev.mnorm, dev.mt, dev.wish
