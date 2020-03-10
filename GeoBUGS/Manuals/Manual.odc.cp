	GeoBUGS User Manual

		Version 3.2.1, March 2011


		Andrew Thomas1	Nicky Best2 	Dave Lunn2      
		Richard Arnold3 	David Spiegelhalter4       
 
		1 Dept of Mathematics & Statistics, University of St Andrews
		2 Department of Epidemiology & Public Health, Imperial College School of Medicine
		3 School of Mathematical and Computing Sciences, Victoria University
		4 MRC Biostatistics Unit, Cambridge

e-mail:	bugs@mrc-bsu.cam.ac.uk	[general]
helsinkiant@gmail.com	[technical]

Contents

	Importing/exporting maps
	Adjacency matrices
	Creating maps
	Spatial distributions
	Temporal distributions
	References

Introduction

GeoBUGS is an add-on module to WinBUGS which provides an interface for: 
* producing maps of the output from disease mapping and other spatial models
* creating and manipulating adjacency matrices that are required as input for the spatial models available in OpenBUGS for carrying out spatial smoothing. 

GeoBUGS contains map files for
* Districts in Scotland (called Scotland)
* Wards in a London Health Authority (called London_HA)
* Counties in Great Britain (called GB_Counties)
* Departements in France (called France)
* Nomoi in Greece (called Greecenomoi)
* Districts in Belgium (called Belgium)
* Communes in Sardinia (called Sardinia)
* Subquarters in Munich (called Munich)
* A 15 x 15 regular grid (called Elevation)
* Wards in West Yorkshire (UK) (called WestYorkshire)
* A 4 x 4 regular grid (called Forest)
* A grid of 750 m2 grid cells covering the town of Huddersfield and surroundings in northern England (called Huddersfield_750m_grid)

A list of the area IDs for each map and the order in which the areas are stored in the map file can be obtained using the export Splus command. 

GeoBUGS also has facilities for importing user-defined maps reading polygon formats from Splus, ArcInfo and Epimap, plus a link to a program written by Yue Cui for importing ArcView shape files. 

