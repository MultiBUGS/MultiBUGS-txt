STRINGS
5000	vector size incompatible with PK link function

5001	unexpected or invalid token scanned
5002	expected standard name, drop, skip, centred, or cat
5003	name already present
5004	alias already present
5005	expected alias, drop, or skip
5006	unexpected token scanned
5007	invalid token scanned
5008	expected token
5009	only covariates may be centred
5010	cat only appropriate for covariates

5100	no data
5101	no names scanned
5102	id, dv, time, and amt must be present
5103	ii must be present if ss or addl present
5104	only one date item permitted
5105	no. of data inconsistent with no. of data items
5106	id must not have missing values
5108	time must not have missing values
5109	evid must not have missing values
5110	invalid id
5112	time must be non-negative
5113	evid must be 0, 1, 2, 3, or 4
5114	expected real or integer
5116	expected time
5117	invalid addl
5118	invalid date

5200	number too large

5300	dose and/or reset events only for individual ^0
5301	inconsistent date formats for individual ^0
5302	invalid date(s) for individual ^0
5303	amt missing on dose event record for individual ^0
5304	ii missing for steady state dose for individual ^0
5305	dv missing or < 0 on observation event record for individual ^0
5306	ii missing for implied doses for individual ^0
5307	data not in chronological order for individual ^0
5308	no model for individual ^0
5309	drug inputs incompatible for individual 1
5310	drug inputs not identical for individuals ^0 and 1
5311	lower bound < 0 for individual ^0
5312	upper bound < or = 0 for individual ^0
5313	upper bound < or = lower bound for individual ^0
5314	no dosing history for individual ^0
5315	no dose events before obs/pred event for individual ^0
5316	input ^0 for individual ^1 not found in PKLink/Rsrc/Inputs.odc

5400	vector size incompatible with no. of compartments

5501	3-comp. models must have iv input in this version
5502	model cannot have ^0 compartments
5503	dv = 0 for individual ^0 -- not allowed with log-normal residuals

5601	prior not entered for ^0
5602	population mean must be > 0 for ^0
5604	inter-individual cv must be > 0 for ^0

5701	log-scale on x-axis inappropriate
5702	log-scale on y-axis inappropriate
5703	some (or all) graphs not plotted because "model" or "data" not monitored
5704	some points not plotted because "model" not monitored or data not observed
5705	no points plotted because "model" not monitored or data not observed
5706	log-scale on y-axis inappropriate for residual plots
5707	some (or all) graphs not plotted because "model" not monitored or data not observed

5801	^0 missing for individual ^1
5802	cannot log ^0 for individual ^1

5901	no plots selected
5902	some (or all) graphs not plotted because "theta" or "theta.mean" not monitored
5903	some (or all) graphs not plotted because "theta" not monitored or covariates not fully defined

namesLoaded	item names loaded
educationalVersion	the educational version of PKBugs is unable to do this problem
dataLoaded	data loaded
okModel	model ok
okPriors	priors ok
priorsLoaded	priors loaded
popInitsLoaded	initial values for population parameters loaded
thetaInitsLoaded	initial values for theta loaded
compiled	model compiled

abs2[0]	ka - lambda1

abs3[0]	abs. time

abs4[0]	lag-time
abs4[1]	ka - lambda1

abs5[0]	lag-time
abs5[1]	abs. time

disp1[0]	CL
disp1[1]	V

disp2[0]	CL
disp2[1]	Q
disp2[2]	V1
disp2[3]	V2

disp3[0]	CL
disp3[1]	Q12
disp3[2]	Q13
disp3[3]	V1
disp3[4]	V2
disp3[5]	V3 - V2

