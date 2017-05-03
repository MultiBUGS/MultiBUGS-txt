# create.kernel: Generates kernel matrix k_ij for use in WinBUGS pois.conv model
#
# Written by Robert Wolpert; last modified by Nicky Best August 2004.
#
create.kernel  <- function(rho=stop("Need radius rho in km"), sig=10,
                     nx, ny, Xx, Xy, outfile="", band=0, seed= 17,
                     sw, ne) {
#
# rho = radius of Gaussian kernel (ASSUMED TO BE GIVEN IN KILOMETRES)
# sig = number of significant figures for kernel matrix
# nx = number of 'columns' of latent grid cells 
# ny = number of 'rows' of latent grid cells
# Xx, Xy = vectors containing, respectivel, x and y co-ordinates of centroids of each area 
#        in the main study region in metres, ASSUMED TO BE MEASURED IN METRES 
# outfile = name of file to write output to
# band = width (in km) of boundary to include round edge of study region
#        to allow for edge effects
# seed = random number seed (Use seed=0 for "random" seed)
# sw = vector of x and y co-ordinates of south-west corner of study region
# ne = vectore of x and y co-ordinates of north-east corner of study region
#
# Note: latent grid cells will be defined by splitting the horizontal distance
#       between (sw$x - band*rho) and (ne$x + band*rho) into ncol equally-spaced 'columns'
#       and the vertical distance between (sw$y - band*rho) and (ne$y + band*rho) into
#       nrow equally spaced rows.
#
    if(seed>0) set.seed(seed);     
    if(rho <= 0) stop("Radius must be positive");
    if(rho > 50) rho <- rho/1000; # Assume rho must be give in metres, so convert to km
#
# Make (nx*ny) by 2 matrix of latent cells B_{ij}:
#
# Define latent grid space (Sdat) as study region, plus a boundary of width band*rho (in km):
#
    Sdat <- list(sw=sw-rep(band*rho*1000,2),
                 ne=ne+rep(band*rho*1000,2));
    dxy  <- 0.001*(Sdat$ne-Sdat$sw)/c(nx,ny);     # Dim (in km) of each latent grid cell B_{ij}
    NS   <- nx*ny;     # number of latent grid cells
#
# Read in study region area centroids (in m):
#
    NX   <- length(Xx);                                              # number of areas in study region
#
# Make NX x NS kernel matrix of X,S pairs:
#
    Sx <- seq(Sdat$sw[1],,1000*dxy[1],nx+1);       # latent grid cell (B_{ij}) boundarys, in metres
    Sy <- seq(Sdat$sw[2],,1000*dxy[2],ny+1);
    ker <- matrix(0,nrow=NX,ncol=NS);
    
    # evaluate the kernel (involves Gaussian integral over each latent grid cell)
    for(i in 1:NX) {
      Fx <- pnorm(.001*(Sx-Xx[i])/rho);  dFx<-Fx[-1]-Fx[-(nx+1)];
      Fy <- pnorm(.001*(Sy-Xy[i])/rho);  dFy<-Fy[-1]-Fy[-(ny+1)];
      ker[i,] <- rep(dFx,rep(ny,nx)) * rep(dFy,nx);
    }
    ker <- 10000 * ker                          # Improve precision
    prec <- 10000 * prod(dxy);           # Inverse scale factor
    dimnames(ker)   <- NULL;             # Remove dim names
    ker[is.na(ker)] <- 0;                       # Null any NA's
    minker <- 10^(-4);
    ker[ker<minker] <- minker;            # Floor to fool WinBUGS (doesn't allow zeros in kernel)
#
# Fill out list for output:
#
    dpg <- list(
      ker = t(ker),                 # in ROW-major format for BUGS
                                          # ********** VERY IMPORTANT *********************
                                          # REMEMBER TO EDIT .Dim statement in output
                                          # file to switch dimensions of ker
                                          # before reading into WinBUGS
                                          # ***********************************************
      prec   = prec,              # Inverse scale factor for kernel
      J  = NS, I = NX,            # Numbers of areas in study region (I) and 
                                          # number of latent grid cells (J)
      area= prod(dxy),        # Area in km^2 of each latent grid cell B_j
      dx = dxy[1], dy = dxy[2], rho = rho);                # Dimensions in km of B_j and radius of Gaussian
                                                                               # kernel (not needed by WinBUGS, so delete before
                                                                               # reading output file into WinBUGS, or include as
                                                                               # dummy variables in bugs code)

    if(sig<10) {
      dpg$ker <- round (dpg$ker, sig);
      dpg$prec <- round (dpg$prec, sig);
      dpg$area<- round (dpg$area,sig);
      dpg$dx  <- round (dpg$dx,  sig);
      dpg$dy  <- round (dpg$dy,  sig);
      dpg$rho <- round (dpg$rho, sig);
    }
    if(outfile!="") { dput(dpg,outfile); }
    invisible(dpg);
}

#####
##### example of how to call function to create kernel for air pollution and respiratory illness example in GeoBUGS 1.2 manual:
#####
##### create.kernel(rho=1, sig=6, nx=12, ny=8, x, y, outfile="kernel.txt", 
#####               band=2, seed= 17, sw=c(399000,402000), ne=c(428250,422250))
#####
##### where x and y are vectores containing the x and y co-ordinates
##### of each area centroid in the study region    
#####
##### This function call will create a latent grid with 12 x 8 latent cells, and calculate
##### the kernel weights k[i,j] = 1/(2*pi*rho^2) * exp(-|x[i] - s[j]|^2 / 2*rho^2)
##### where |x[i] - s[j]| is the euclidean distance between the centroid of area i of the study
##### region and latent grid cell j   

