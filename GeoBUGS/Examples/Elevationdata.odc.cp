list(N=52, M= 225, 
    x = c(0.3, 1.4, 2.4, 3.6, 5.7, 1.6, 2.9, 3.4, 3.4, 4.8, 5.3, 6.2, 
	0.2, 0.9, 2.3, 2.5, 3, 3.5, 4.1, 4.9, 6.3, 0.9, 1.7, 2.4, 3.7, 4.5, 5.2,
	6.3, 0.3, 2, 3.8, 6.3, 0.6, 1.5, 2.1, 2.1, 3.1, 4.5, 5.5, 5.7, 6.2, 0.4,
	1.4, 1.4, 2.1, 2.3, 3.1, 4.1, 5.4, 6, 5.7, 3.6), 
	y = c(6.1, 6.2, 6.1, 6.2, 6.2, 5.2, 5.1, 5.3, 5.7, 5.6, 5, 5.2, 4.3, 
	4.2, 4.8, 4.5, 4.5, 4.5, 4.6, 4.2, 4.3, 3.2, 3.8, 3.8, 3.5, 3.2, 3.2, 
	3.4, 2.4, 2.7, 2.3, 2.2, 1.7, 1.8, 1.8, 1.1, 1.1, 1.8, 1.7, 1, 1, 0.5, 
	0.6, 0.1, 0.7, 0.3, 0, 0.8, 0.4, 0.1, 3, 6), 
	height = c(870, 793, 755, 690, 800, 800, 730, 728,
	710, 780, 804, 855, 830, 813, 762, 765, 740, 765, 760, 790, 820, 855, 
	812, 773, 812, 827, 805, 840, 890, 820, 873, 875, 873, 865, 841, 862, 
	908, 855, 850, 882, 910, 940, 915, 890, 880, 870, 880, 960, 890, 860, 
	830, 705),
	x.pred=c(0.21, 0.21, 0.21, 0.21, 0.21, 0.21, 0.21, 0.21, 0.21, 0.21, 0.21, 0.21, 0.21, 
	0.21, 0.21, 0.63, 0.63, 0.63, 0.63, 0.63, 0.63, 0.63, 0.63, 0.63, 0.63, 
	0.63, 0.63, 0.63, 0.63, 0.63, 1.05, 1.05, 1.05, 1.05, 1.05, 1.05, 1.05, 
	1.05, 1.05, 1.05, 1.05, 1.05, 1.05, 1.05, 1.05, 1.47, 1.47, 1.47, 1.47, 
	1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.89, 
	1.89, 1.89, 1.89, 1.89, 1.89, 1.89, 1.89, 1.89, 1.89, 1.89, 1.89, 1.89, 
	1.89, 1.89, 2.31, 2.31, 2.31, 2.31, 2.31, 2.31, 2.31, 2.31, 2.31, 2.31, 
	2.31, 2.31, 2.31, 2.31, 2.31, 2.73, 2.73, 2.73, 2.73, 2.73, 2.73, 2.73, 
	2.73, 2.73, 2.73, 2.73, 2.73, 2.73, 2.73, 2.73, 3.15, 3.15, 3.15, 3.15, 
	3.15, 3.15, 3.15, 3.15, 3.15, 3.15, 3.15, 3.15, 3.15, 3.15, 3.15, 3.57, 
	3.57, 3.57, 3.57, 3.57, 3.57, 3.57, 3.57, 3.57, 3.57, 3.57, 3.57, 3.57, 
	3.57, 3.57, 3.99, 3.99, 3.99, 3.99, 3.99, 3.99, 3.99, 3.99, 3.99, 3.99, 
	3.99, 3.99, 3.99, 3.99, 3.99, 4.41, 4.41, 4.41, 4.41, 4.41, 4.41, 4.41, 
	4.41, 4.41, 4.41, 4.41, 4.41, 4.41, 4.41, 4.41, 4.83, 4.83, 4.83, 4.83, 
	4.83, 4.83, 4.83, 4.83, 4.83, 4.83, 4.83, 4.83, 4.83, 4.83, 4.83, 5.25, 
	5.25, 5.25, 5.25, 5.25, 5.25, 5.25, 5.25, 5.25, 5.25, 5.25, 5.25, 5.25, 
	5.25, 5.25, 5.67, 5.67, 5.67, 5.67, 5.67, 5.67, 5.67, 5.67, 5.67, 5.67, 
	5.67, 5.67, 5.67, 5.67, 5.67, 6.09, 6.09, 6.09, 6.09, 6.09, 6.09, 6.09, 
	6.09, 6.09, 6.09, 6.09, 6.09, 6.09, 6.09, 6.09), 
y.pred=c(6.09, 5.67, 5.25, 4.83, 4.41, 3.99, 3.57, 3.15, 2.73, 2.31, 1.89, 1.47, 1.05, 
	0.63, 0.21, 6.09, 5.67, 5.25, 4.83, 4.41, 3.99, 3.57, 3.15, 2.73, 2.31, 
	1.89, 1.47, 1.05, 0.63, 0.21, 6.09, 5.67, 5.25, 4.83, 4.41, 3.99, 3.57, 
	3.15, 2.73, 2.31, 1.89, 1.47, 1.05, 0.63, 0.21, 6.09, 5.67, 5.25, 4.83, 
	4.41, 3.99, 3.57, 3.15, 2.73, 2.31, 1.89, 1.47, 1.05, 0.63, 0.21, 6.09, 
	5.67, 5.25, 4.83, 4.41, 3.99, 3.57, 3.15, 2.73, 2.31, 1.89, 1.47, 1.05, 
	0.63, 0.21, 6.09, 5.67, 5.25, 4.83, 4.41, 3.99, 3.57, 3.15, 2.73, 2.31, 
	1.89, 1.47, 1.05, 0.63, 0.21, 6.09, 5.67, 5.25, 4.83, 4.41, 3.99, 3.57, 
	3.15, 2.73, 2.31, 1.89, 1.47, 1.05, 0.63, 0.21, 6.09, 5.67, 5.25, 4.83, 
	4.41, 3.99, 3.57, 3.15, 2.73, 2.31, 1.89, 1.47, 1.05, 0.63, 0.21, 6.09, 
	5.67, 5.25, 4.83, 4.41, 3.99, 3.57, 3.15, 2.73, 2.31, 1.89, 1.47, 1.05, 
	0.63, 0.21, 6.09, 5.67, 5.25, 4.83, 4.41, 3.99, 3.57, 3.15, 2.73, 2.31, 
	1.89, 1.47, 1.05, 0.63, 0.21, 6.09, 5.67, 5.25, 4.83, 4.41, 3.99, 3.57, 
	3.15, 2.73, 2.31, 1.89, 1.47, 1.05, 0.63, 0.21, 6.09, 5.67, 5.25, 4.83, 
	4.41, 3.99, 3.57, 3.15, 2.73, 2.31, 1.89, 1.47, 1.05, 0.63, 0.21, 6.09, 
	5.67, 5.25, 4.83, 4.41, 3.99, 3.57, 3.15, 2.73, 2.31, 1.89, 1.47, 1.05, 
	0.63, 0.21, 6.09, 5.67, 5.25, 4.83, 4.41, 3.99, 3.57, 3.15, 2.73, 2.31, 
	1.89, 1.47, 1.05, 0.63, 0.21, 6.09, 5.67, 5.25, 4.83, 4.41, 3.99, 3.57, 
	3.15, 2.73, 2.31, 1.89, 1.47, 1.05, 0.63, 0.21))
