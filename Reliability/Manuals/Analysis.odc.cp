	
 Introduction [top]

ReliaBUGS is an add-on to the OpenBUGS software that makes it easy to do reliability analysis in OpenBUGS. ReliaBUGS consists of many probability distribution useful for reliability analysis plus a small number of helpful logical functions.

Reliability Analysis [top]

Reliability analysis evaluates the performance of an equipment/system under consideration. In particular, we use the term reliability to mean the probability that a piece of equipment (component, subsystem or system) successfully performs its intended function for a given period of time under specified (design) conditions.  The International Organization for Standardization (ISO) defines reliability as “the ability of an item to perform a required function, under given environmental and operating conditions and for a stated period of time”.

Let random variable X denote the failure time (or time to failure) of some device of interest under stated environmental conditions and let f(x) denote the probability density function (pdf) of X. The cdf F(x), a measure of failure, is defined as the probability that the system will fail by time x.

						 F(x) = P(X ≤ x),     x ≥ 0. 
			
where F(x) is known as lifetime distribution.

Mathematically, reliability R(x) is the probability that a system will be successful in the interval from time 0 to time x:

R(x) = P(X> x) = 1 - F(x) ,         x ≥ 0, 

where X is a random variable denoting the time-to failure or failure time. 

Censoring [top][example]
In reliability context, the following types of censored data are of particular interest. Censoring occurs frequently because of time limits and other restrictions during the process of data collection. In a life test experiment of manufactured items, for example, it may not be feasible to continue experimentation until all items under study have failed. If the study is terminated before all have failed, then for items that have not failed at the time of termination only a lower bound on lifetime is available.
 
Let X1, X2; . . . , Xn denote a sample of n-independent random variables from a distribution function F(x, q), where q is a k-dimensional parameter. These correspond to the life times for n different items in the reliability context.. Let  x1, x2; . . . , xn denote the actual realized values of the Xi’s.

Complete Data
The data set available for estimation is the set  x1, x2; . . . , xn. In other words, the actual realized values are known for each observation in the data set.

Censored Data
In this case the actual realized values for some or all of the variables are not known, and this depends on the kind of censoring.

(a) Right Type I Censoring
Type I censoring often 'arises when a study is conducted over a specified time period. This is also known as time censoring. Let C(t) denote the fixed potential censoring time. Under type I right censoring the data available for estimation is as follows: For item i, the actual realized value of Xi is known only if xi < C(t). When xi > C(t) the only information available is that Xi > C(t).

(b) Right Type II Censoring
This censoring scheme arises when n individuals start on study at the same time, with the study terminating once r failures (or lifetimes) have been observed. This is also known as item censoring. Let 'r' denote a predetermined number such that r < n. Under type II right censoring the data available for estimation is given by xi (the actual realized value of Xi) for 'r' data and that Xi > x(max) for the remaining (n - r) data where x(max) is the maximum of the r observed xi’s.

