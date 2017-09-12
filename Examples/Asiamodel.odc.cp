	model
	{
		smoking ~ dcat(p.smoking[1:2])
		tuberculosis ~ dcat(p.tuberculosis[asia,1:2])
		lung.cancer ~ dcat(p.lung.cancer[smoking,1:2])
		bronchitis ~ dcat(p.bronchitis[smoking,1:2])
		either <- max(tuberculosis,lung.cancer)
		xray ~ dcat(p.xray[either,1:2])
		dyspnoea ~ dcat(p.dyspnoea[either,bronchitis,1:2])
	}

