	Temporal distributions

Contents

	Temporal smoothing with car.normal
	
	
Temporal smoothing with car.normal        [top]

In one dimension, the intrinsic Gaussian CAR distribution reduces to a Gaussian random walk (see e.g.Fahrmeir and Lang, 2001). Assume we have a set of temporally correlated random effects qt, t=1,..., T (where T is the number of equally-spaced time points). In the simplest case of a random walk of order 1, RW(1), we may write

                  qt  |  q-t  ~  Normal (  qt+1,  f )                       for  t = 1
                                ~  Normal (  (qt-1 + qt+1)/2,  f / 2 )    for  t = 2, ...., T-1
                                ~  Normal (  qt-1,  f )                       for  t = T

where q-t denotes all elements of q  except the qt. This is equivalent to specifying 

                  qt  |  q-t  ~  Normal ( Sk Ctk qk,  f Mtt)        for  t = 1, ..., T

where Ctk = Wtk / Wt+,    Wt+ = S k Wtk and Wtk = 1 if k = (t-1) or (t+1) and 0 otherwise; Mtt = 1/Wt+. Hence the RW(1) prior may be fitted using the car.normal distribution in WinBUGS, with appropriate specification of the weight and adjacency matrices, and num vector (see the Air Pollution Example)  

A second order random walk prior is defined as

                  qt  |  q-t  ~  Normal (  2qt+1 - qt+2,  f )                                  for  t = 1
                                ~  Normal (  (2qt-1 + 4qt+1 - qt+2)/5,  f / 5 )              for  t = 2 
                                ~  Normal (  (-qt-2 + 4qt-1 + 4qt+1 - qt+2)/6,  f / 6 )    for  t = 3, ...., T-2
                                ~  Normal (  (-qt-2 + 4qt-1 + 2qt+1)/5,  f / 5 )             for  t = T-1
                                ~  Normal (  -qt-2+ 2qt-1,  f )                                  for  t = T

Again this is equivalent to specifying 

                  qt  |  q-t  ~  Normal ( Sk Ctk qk,  f Mtt)        for  t = 1, ..., T

where Ctk is defined as above, but this time with Wtk = -1 if k = (t-2) or (t+2), Wtk = 4 if k = (t-1) or (t+1) and t in (3, T-2), Wtk = 2 if k = (t-1) or (t+1) and t = 2 or T-1, Wtk = 0 otherwise; Mtt = 1/Wt+. 

Note that if the observed time points are not equally spaced, it is necessary to include missing values (NA) for the intermediate time points (see the Air Pollution Example). 
