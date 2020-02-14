(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterGLM;


	

	IMPORT
		MPIworker, Math, Stores := Stores64,
		BugsRegistry,
		GraphConjugateMV, GraphConjugateUV, GraphLinkfunc, GraphLogical, GraphNodes, GraphRules,
		GraphStochastic,
		MathMatrix, MathRandnum,
		UpdaterMetropolisMV, UpdaterMultivariate, UpdaterRejection, UpdaterUnivariate, UpdaterUpdaters;

	TYPE

		Updater = POINTER TO ABSTRACT RECORD(UpdaterMetropolisMV.Updater)
			predictors: GraphNodes.Vector
		END;

		UpdaterGLM = POINTER TO ABSTRACT RECORD(Updater)
			singleSiteUpdaters: POINTER TO ARRAY OF UpdaterUnivariate.Updater
		END;

		UpdaterLogit = POINTER TO RECORD(UpdaterGLM) END;

		UpdaterLoglin = POINTER TO RECORD(UpdaterGLM) END;

		UpdaterNormal = POINTER TO RECORD(Updater) END;

		FactoryLogit = POINTER TO RECORD(UpdaterUpdaters.Factory) END;

		FactoryLoglin = POINTER TO RECORD(UpdaterUpdaters.Factory) END;

		FactoryNormal = POINTER TO RECORD(UpdaterUpdaters.Factory) END;

	VAR
		factLogit-, factLoglin-, factNormal-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		newValues, oldValues, offset, mu, w, n, y, derivLogLik,
		priorMu, propMu: POINTER TO ARRAY OF REAL;
		choleskiDecomp, priorTau, deriv2LogLik, z: POINTER TO ARRAY OF ARRAY OF REAL;

	PROCEDURE FindBlock (prior: GraphStochastic.Node; allowBounds: BOOLEAN): GraphStochastic.Vector;
		CONST
			mult = 5;
		VAR
			block: GraphStochastic.Vector;
			likelihood: GraphStochastic.Vector;
			class: SET;
			i, len, size: INTEGER;
	BEGIN
		WITH prior: GraphConjugateMV.Node DO
			block := prior.components;
			i := 0;
			size := LEN(block);
			WHILE i < size DO
				IF GraphNodes.data IN block[i].props THEN block := NIL END;
				INC(i)
			END
		ELSE
			class := {prior.classConditional};
			block := UpdaterMultivariate.FixedEffects(prior, class, allowBounds);
			IF block # NIL THEN
				size := LEN(block);
				likelihood := UpdaterMultivariate.BlockLikelihood(block);
				(*	huristic to try and stop algorithm finding random effect blocks	*)
				len := LEN(likelihood);
				IF (len < mult * size) & (size > 2) THEN block := NIL END;
				i := 0;
				WHILE i < len DO
					IF likelihood[i] IS GraphConjugateMV.Node THEN block := NIL END;
					INC(i)
				END
			END
		END;
		RETURN block
	END FindBlock;

	PROCEDURE (updater: Updater) ParamsSize (): INTEGER;
		VAR
			size: INTEGER;
	BEGIN
		size := updater.Size();
		RETURN size + size * size
	END ParamsSize;

	PROCEDURE (updater: Updater) CalculateGLMParams, NEW, ABSTRACT;

	PROCEDURE (updater: Updater) DesignMatrix, NEW;
		VAR
			i, j, size, num: INTEGER;
			children: GraphStochastic.Vector;
			prior: GraphStochastic.Node;
	BEGIN
		children := updater.Children();
		IF children # NIL THEN
			prior := updater.prior[0];
			IF ~(GraphStochastic.optimizeDiffs IN prior.props) THEN
				GraphLogical.EvaluateDiffs(updater.dependents)
			END;
			size := updater.Size();
			num := LEN(children);
			j := 0;
			WHILE j < size DO
				(*	evaluate derivatives	*)
				i := 0;
				WHILE i < num DO
					z[j, i] := updater.predictors[i].Diff(updater.prior[j]);
					INC(i)
				END;
				INC(j)
			END;
			IF ~(GraphStochastic.optimizeDiffs IN prior.props) THEN
				GraphLogical.ClearMarks(updater.dependents, {GraphLogical.diff})
			END;
		END
	END DesignMatrix;

	PROCEDURE (updater: Updater) CalculateDerivatives (distributed: BOOLEAN), NEW;
		VAR
			i, j, k, size, num: INTEGER;
			children: GraphStochastic.Vector;
	BEGIN
		size := updater.Size();
		children := updater.Children();
		j := 0;
		WHILE j < size DO
			derivLogLik[j] := 0.0;
			k := 0;
			WHILE k < size DO
				deriv2LogLik[j, k] := 0;
				INC(k)
			END;
			INC(j)
		END;
		IF children # NIL THEN
			num := LEN(children);
			i := 0;
			WHILE i < num DO
				j := 0;
				WHILE j < size DO
					derivLogLik[j] := derivLogLik[j] + z[j, i] * (y[i] - mu[i]);
					k := 0;
					WHILE k < size DO
						deriv2LogLik[j, k] := deriv2LogLik[j, k] + z[j, i] * w[i] * z[k, i];
						INC(k)
					END;
					INC(j)
				END;
				INC(i)
			END
		END;
		IF distributed THEN
			j := 0;
			WHILE j < size DO
				updater.params[j] := derivLogLik[j];
				k := 0;
				WHILE k < size DO
					updater.params[size + j * size + k] := deriv2LogLik[j, k];
					INC(k)
				END;
				INC(j)
			END;
			MPIworker.SumReals(updater.params);
			j := 0;
			WHILE j < size DO
				derivLogLik[j] := updater.params[j];
				k := 0;
				WHILE k < size DO
					deriv2LogLik[j, k] := updater.params[size + j * size + k];
					INC(k)
				END;
				INC(j)
			END
		END;
		j := 0; (*	add prior contribution	*)
		WHILE j < size DO
			k := 0;
			WHILE k < size DO
				derivLogLik[j] := derivLogLik[j] - priorTau[j, k] * (updater.prior[k].value - priorMu[k]);
				deriv2LogLik[j, k] := deriv2LogLik[j, k] + priorTau[j, k];
				INC(k)
			END;
			INC(j)
		END;
		j := 0; 	(*	what does this loop do?	*) (*	taylors series expansion	*)
		WHILE j < size DO
			k := 0;
			WHILE k < size DO
				derivLogLik[j] := derivLogLik[j] + deriv2LogLik[j, k] * updater.prior[k].value;
				INC(k)
			END;
			INC(j)
		END;
		j := 0;
		WHILE j < size DO
			k := 0;
			WHILE k < size DO
				choleskiDecomp[j, k] := deriv2LogLik[j, k];
				INC(k)
			END;
			INC(j)
		END;
		MathMatrix.Cholesky(choleskiDecomp, size);
		MathMatrix.ForwardSub(choleskiDecomp, derivLogLik, size);
		MathMatrix.BackSub(choleskiDecomp, derivLogLik, size);
		j := 0;
		WHILE j < size DO
			propMu[j] := derivLogLik[j];
			INC(j)
		END
	END CalculateDerivatives;

	PROCEDURE (updater: Updater) InitializeMetropolisMV;
		VAR
			i, numLike, size: INTEGER;
			factInner: UpdaterUpdaters.Factory;
			u: UpdaterUpdaters.Updater;
			children: GraphStochastic.Vector;
	BEGIN
		size := updater.Size();
		children := updater.Children();
		IF children # NIL THEN
			numLike := LEN(children); NEW(updater.predictors, numLike);
		ELSE
			numLike := 0; updater.predictors := NIL
		END;
		i := 0;
		WHILE i < numLike DO
			updater.predictors[i] := NIL;
			INC(i)
		END;
		IF numLike > LEN(offset) THEN
			NEW(z, MAX(size, LEN(derivLogLik)), numLike);
			NEW(offset, numLike);
			NEW(mu, numLike);
			NEW(n, numLike);
			NEW(y, numLike);
			NEW(w, numLike)
		END;
		IF size > LEN(derivLogLik) THEN
			numLike := MAX(numLike, LEN(offset));
			NEW(z, size, numLike);
			NEW(derivLogLik, size);
			NEW(priorMu, size);
			NEW(newValues, size);
			NEW(oldValues, size);
			NEW(propMu, size);
			NEW(priorTau, size, size);
			NEW(deriv2LogLik, size, size);
			NEW(choleskiDecomp, size, size)
		END;
		WITH updater: UpdaterGLM DO
			NEW(updater.singleSiteUpdaters, size);
			i := 0;
			IF updater IS UpdaterLogit THEN
				factInner := UpdaterRejection.factLogit
			ELSE
				factInner := UpdaterRejection.factLoglin
			END;
			WHILE i < size DO
				u := factInner.New(updater.prior[i]);
				updater.singleSiteUpdaters[i] := u(UpdaterUnivariate.Updater);
				factInner.SetProps(factInner.props + {UpdaterUpdaters.active});
				INC(i)
			END
		ELSE
		END
	END InitializeMetropolisMV;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) LikelihoodParameters, NEW, ABSTRACT;

	PROCEDURE (updater: Updater) Optimize;
		VAR
			i, num: INTEGER;
			prior: GraphStochastic.Node;
			optimizeDiffs: BOOLEAN;
			p: GraphLogical.Node;
	BEGIN
		optimizeDiffs := TRUE;
		IF updater.dependents # NIL THEN
			num := LEN(updater.dependents);
			i := 0;
			WHILE (i < num) & optimizeDiffs DO
				p := updater.dependents[i];
				IF ~(GraphLogical.prediction IN p.props) THEN
					IF ~(GraphLogical.constDiffs IN p.props) THEN optimizeDiffs := p IS GraphLinkfunc.Node END
				END;
				INC(i)
			END;
		END;
		IF optimizeDiffs THEN
			num := LEN(updater.prior);
			i := 0;
			WHILE i < num DO
				prior := updater.prior[i];
				INCL(prior.props, GraphStochastic.optimizeDiffs);
				INC(i)
			END
		END
	END Optimize;

	PROCEDURE (updater: Updater) PriorParameters, NEW;
		VAR
			i, j, size: INTEGER;
			p0, p1: REAL;
			univariate: GraphConjugateUV.Node;
		CONST
			as = GraphRules.normal;
	BEGIN
		size := updater.Size();
		IF updater.prior[0] IS GraphConjugateMV.Node THEN
			updater.prior[0](GraphConjugateMV.Node).MVPriorForm(priorMu, priorTau)
		ELSE
			i := 0;
			WHILE i < size DO
				univariate := updater.prior[i](GraphConjugateUV.Node);
				univariate.PriorForm(as, p0, p1);
				priorMu[i] := p0;
				j := 0;
				WHILE j < size DO
					priorTau[i, j] := 0.0;
					INC(j)
				END;
				priorTau[i, i] := p1;
				INC(i)
			END
		END
	END PriorParameters;

	PROCEDURE (updater: Updater) ProposalDensity (IN values: ARRAY OF REAL): REAL, NEW;
		VAR
			j, k, size: INTEGER;
			density, x, y: REAL;
	BEGIN
		size := updater.Size();
		density := 0.0;
		j := 0;
		WHILE j < size DO
			density := density + Math.Ln(choleskiDecomp[j, j]);
			x := values[j] - propMu[j];
			k := 0;
			WHILE k < size DO
				y := values[k] - propMu[k];
				density := density - 0.5 * x * deriv2LogLik[j, k] * y;
				INC(k)
			END;
			INC(j)
		END;
		RETURN density
	END ProposalDensity;

	PROCEDURE (updater: UpdaterGLM) CopyFromMetropolisMV (source: UpdaterUpdaters.Updater);
		VAR
			s: UpdaterGLM;
			i, size: INTEGER;
			copy: UpdaterUpdaters.Updater;
	BEGIN
		s := source(UpdaterGLM);
		updater.predictors := s.predictors;
		size := updater.Size();
		NEW(updater.singleSiteUpdaters, size);
		i := 0;
		WHILE i < size DO
			copy := UpdaterUpdaters.CopyFrom(s.singleSiteUpdaters[i]);
			updater.singleSiteUpdaters[i] := copy(UpdaterUnivariate.Updater);
			INC(i)
		END
	END CopyFromMetropolisMV;

	PROCEDURE (updater: UpdaterGLM) ExternalizeMetropolisMV (VAR wr: Stores.Writer);
		VAR
			i, size: INTEGER;
	BEGIN
		size := updater.Size();
		i := 0;
		WHILE i < size DO
			UpdaterUpdaters.Externalize(updater.singleSiteUpdaters[i], wr); INC(i)
		END
	END ExternalizeMetropolisMV;

	PROCEDURE (updater: UpdaterGLM) InternalizeMetropolisMV (VAR rd: Stores.Reader);
		VAR
			i, size: INTEGER;
	BEGIN
		size := updater.Size();
		i := 0;
		WHILE i < size DO
			UpdaterUpdaters.Internalize(updater.singleSiteUpdaters[i], rd); INC(i)
		END;
	END InternalizeMetropolisMV;

	PROCEDURE (updater: UpdaterGLM) Sample (overRelax: BOOLEAN; OUT res: SET);
		CONST
			batch = 10;
		VAR
			i, size: INTEGER;
			distributed: BOOLEAN;
			acceptProb, logLik, newLD, newProp, oldLD, oldProp: REAL;
	BEGIN
		res := {};
		size := updater.Size();
		distributed := GraphStochastic.distributed IN updater.prior[0].props;
		IF (updater.iteration MOD batch # 0) & (updater.iteration > 100) THEN
			updater.GetValue(oldValues);
			updater.Store;
			logLik := updater.LogLikelihood();
			updater.PriorParameters;
			updater.LikelihoodParameters;
			updater.DesignMatrix;
			updater.CalculateGLMParams;
			updater.CalculateDerivatives(distributed);
			oldLD := logLik + updater.LogPrior();
			MathRandnum.MNormal(choleskiDecomp, propMu, size, newValues);
			updater.SetValue(newValues); GraphLogical.Evaluate(updater.dependents);
			oldProp := updater.ProposalDensity(newValues);
			logLik := updater.LogLikelihood();
			updater.CalculateGLMParams;
			updater.CalculateDerivatives(distributed);
			newProp := updater.ProposalDensity(oldValues);
			newLD := logLik + updater.LogPrior();
			acceptProb := newLD - oldLD + newProp - oldProp;
			IF acceptProb < Math.Ln(MathRandnum.Rand()) THEN updater.Restore END
		ELSE
			i := 0;
			WHILE i < size DO
				updater.singleSiteUpdaters[i].Sample(overRelax, res);
				INC(i)
			END
		END;
		INC(updater.iteration)
	END Sample;

	PROCEDURE (updater: UpdaterLogit) CalculateGLMParams;
		VAR
			i, num: INTEGER;
			predictor: REAL;
			children: GraphStochastic.Vector;
	BEGIN
		children := updater.Children();
		IF children # NIL THEN
			i := 0;
			num := LEN(children);
			WHILE i < num DO
				predictor := updater.predictors[i].value;
				mu[i] := n[i] / (1.0 + Math.Exp( - predictor));
				w[i] := (1.0 - mu[i] / n[i]) * mu[i];
				INC(i)
			END
		END
	END CalculateGLMParams;

	PROCEDURE (updater: UpdaterLogit) Clone (): UpdaterLogit;
		VAR
			u: UpdaterLogit;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: UpdaterLogit) FindBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
	BEGIN
		RETURN FindBlock(prior, FALSE)
	END FindBlock;

	PROCEDURE (updater: UpdaterLogit) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterGLM.InstallLogit"
	END Install;

	PROCEDURE (updater: UpdaterLogit) LikelihoodParameters;
		VAR
			as, i, num: INTEGER;
			children: GraphStochastic.Vector;
			node: GraphConjugateUV.Node;
			p0, p1: REAL;
			x: GraphNodes.Node;
	BEGIN
		children := updater.Children();
		IF children # NIL THEN
			as := GraphRules.beta;
			i := 0;
			num := LEN(children);
			WHILE i < num DO
				node := children[i](GraphConjugateUV.Node);
				node.LikelihoodForm(as, x, p0, p1);
				updater.predictors[i] := x(GraphLinkfunc.Node).predictor;
				y[i] := p0;
				n[i] := p1 + p0;
				INC(i)
			END
		END
	END LikelihoodParameters;

	PROCEDURE (updater: UpdaterLoglin) CalculateGLMParams;
		VAR
			i, num: INTEGER;
			predictor: REAL;
			children: GraphStochastic.Vector;
	BEGIN
		children := updater.Children();
		IF children # NIL THEN
			i := 0;
			num := LEN(children);
			WHILE i < num DO
				predictor := updater.predictors[i].value;
				mu[i] := n[i] * Math.Exp(predictor);
				w[i] := n[i] * Math.Exp(predictor);
				INC(i)
			END
		END
	END CalculateGLMParams;

	PROCEDURE (updater: UpdaterLoglin) Clone (): UpdaterLoglin;
		VAR
			u: UpdaterLoglin;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: UpdaterLoglin) FindBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
	BEGIN
		RETURN FindBlock(prior, FALSE)
	END FindBlock;

	PROCEDURE (updater: UpdaterLoglin) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterGLM.InstallLoglin"
	END Install;

	PROCEDURE (updater: UpdaterLoglin) LikelihoodParameters;
		VAR
			i, as, num: INTEGER;
			p0, p1: REAL;
			x: GraphNodes.Node;
			node: GraphConjugateUV.Node;
			children: GraphStochastic.Vector;
	BEGIN
		children := updater.Children();
		IF children # NIL THEN
			as := GraphRules.gamma;
			i := 0;
			num := LEN(children);
			WHILE i < num DO
				node := children[i](GraphConjugateUV.Node);
				node.LikelihoodForm(as, x, p0, p1);
				updater.predictors[i] := x(GraphLinkfunc.Node).predictor;
				y[i] := p0;
				n[i] := p1;
				INC(i)
			END
		END
	END LikelihoodParameters;

	PROCEDURE (updater: UpdaterNormal) CalculateGLMParams;
		VAR
			i, num: INTEGER;
			predictor: REAL;
			children: GraphStochastic.Vector;
	BEGIN
		children := updater.Children();
		IF children # NIL THEN
			i := 0;
			num := LEN(children);
			WHILE i < num DO
				predictor := updater.predictors[i].value;
				mu[i] := n[i] * predictor;
				w[i] := n[i];
				INC(i)
			END
		END
	END CalculateGLMParams;

	PROCEDURE (updater: UpdaterNormal) CheckBounds (values: POINTER TO ARRAY OF REAL): BOOLEAN, NEW;
		VAR
			lower, upper: REAL;
			i, size: INTEGER;
			boundsOk: BOOLEAN;
	BEGIN
		boundsOk := TRUE;
		size := updater.Size();
		i := 0;
		WHILE boundsOk & (i < size) DO
			updater.prior[i].Bounds(lower, upper);
			boundsOk := (lower < values[i]) & (upper > values[i]);
			INC(i)
		END;
		RETURN TRUE
	END CheckBounds;

	PROCEDURE (updater: UpdaterNormal) Clone (): UpdaterNormal;
		VAR
			u: UpdaterNormal;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: UpdaterNormal) CopyFromMetropolisMV (source: UpdaterUpdaters.Updater);
		VAR
			s: UpdaterNormal;
	BEGIN
		s := source(UpdaterNormal);
		updater.predictors := s.predictors;
	END CopyFromMetropolisMV;

	PROCEDURE (updater: UpdaterNormal) ExternalizeMetropolisMV (VAR wr: Stores.Writer);
	BEGIN
	END ExternalizeMetropolisMV;

	PROCEDURE (updater: UpdaterNormal) FindBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
	BEGIN
		RETURN FindBlock(prior, TRUE)
	END FindBlock;

	PROCEDURE (updater: UpdaterNormal) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterGLM.InstallNormal"
	END Install;

	PROCEDURE (updater: UpdaterNormal) InternalizeMetropolisMV (VAR rd: Stores.Reader);
	BEGIN
	END InternalizeMetropolisMV;

	PROCEDURE (updater: UpdaterNormal) LikelihoodParameters;
		VAR
			as, i, num: INTEGER;
			children: GraphStochastic.Vector;
			node: GraphConjugateUV.Node;
			p0, p1: REAL;
			x: GraphNodes.Node;
	BEGIN
		children := updater.Children();
		IF children # NIL THEN
			num := LEN(children);
			as := GraphRules.normal;
			i := 0;
			WHILE i < num DO
				node := children[i](GraphConjugateUV.Node);
				node.LikelihoodForm(as, x, p0, p1);
				updater.predictors[i] := x;
				y[i] := p0 * p1;
				n[i] := p1;
				INC(i)
			END
		END
	END LikelihoodParameters;

	PROCEDURE (updater: UpdaterNormal) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			i, size: INTEGER;
			alpha: REAL;
			distributed: BOOLEAN;
	BEGIN
		res := {};
		distributed := GraphStochastic.distributed IN updater.prior[0].props;
		size := updater.Size();
		updater.GetValue(oldValues);
		updater.PriorParameters;
		updater.LikelihoodParameters;
		updater.DesignMatrix;
		updater.CalculateGLMParams;
		updater.CalculateDerivatives(distributed);
		REPEAT
			IF overRelax THEN
				alpha := - (1 - 1 / Math.Sqrt(factNormal.overRelaxation));
				MathRandnum.RelaxedMNormal(choleskiDecomp, propMu, oldValues, size, alpha, newValues)
			ELSE
				MathRandnum.MNormal(choleskiDecomp, propMu, size, newValues)
			END
		UNTIL updater.CheckBounds(newValues);
		updater.SetValue(newValues)
	END Sample;

	PROCEDURE (f: FactoryLogit) GetDefaults;
		VAR
			res: INTEGER;
			props: SET;
			name: ARRAY 256 OF CHAR;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: FactoryLogit) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterGLM.InstallLogit"
	END Install;

	PROCEDURE (f: FactoryLogit) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		CONST
			bounds = {GraphStochastic.leftNatural, GraphStochastic.leftImposed,
			GraphStochastic.rightNatural, GraphStochastic.rightImposed};
		VAR
			block: GraphStochastic.Vector;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF bounds * prior.props # {} THEN RETURN FALSE END;
		IF prior.classConditional # GraphRules.logitReg THEN RETURN FALSE END;
		block := FindBlock(prior, FALSE);
		IF block = NIL THEN RETURN FALSE END;
		IF ~UpdaterMultivariate.IsHomologous(block) THEN RETURN FALSE END;
		IF UpdaterMultivariate.ClassifyBlock(block) # GraphRules.logitReg THEN RETURN FALSE END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: FactoryLogit) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: UpdaterLogit;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE (f: FactoryLoglin) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterGLM.InstallLoglin"
	END Install;

	PROCEDURE (f: FactoryLoglin) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		CONST
			bounds = {GraphStochastic.leftNatural, GraphStochastic.leftImposed,
			GraphStochastic.rightNatural, GraphStochastic.rightImposed};
		VAR
			block: GraphStochastic.Vector;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF bounds * prior.props # {} THEN RETURN FALSE END;
		IF prior.classConditional # GraphRules.logReg THEN RETURN FALSE END;
		block := FindBlock(prior, FALSE);
		IF block = NIL THEN RETURN FALSE END;
		IF ~UpdaterMultivariate.IsHomologous(block) THEN RETURN FALSE END;
		IF UpdaterMultivariate.ClassifyBlock(block) # GraphRules.logReg THEN RETURN FALSE END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: FactoryLoglin) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: UpdaterLoglin;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE (f: FactoryLoglin) GetDefaults;
		VAR
			res: INTEGER;
			props: SET;
			name: ARRAY 256 OF CHAR;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: FactoryNormal) GetDefaults;
		VAR
			overRelaxation, res: INTEGER;
			props: SET;
			name: ARRAY 256 OF CHAR;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadInt(name + ".overRelaxation", overRelaxation, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetParameter(overRelaxation, UpdaterUpdaters.overRelaxation);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE (f: FactoryNormal) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterGLM.InstallNormal"
	END Install;

	PROCEDURE (f: FactoryNormal) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		CONST
			bounds = {GraphStochastic.leftNatural, GraphStochastic.leftImposed,
			GraphStochastic.rightNatural, GraphStochastic.rightImposed};
		VAR
			block: GraphStochastic.Vector;
			i, len: INTEGER;
	BEGIN
		IF ~UpdaterMultivariate.normalBlock THEN RETURN FALSE END;
		IF GraphStochastic.integer IN prior.props THEN RETURN FALSE END;
		IF prior.classConditional # GraphRules.normal THEN RETURN FALSE END;
		block := FindBlock(prior, TRUE);
		IF block = NIL THEN RETURN FALSE END;
		IF ~UpdaterMultivariate.IsHomologous(block) THEN RETURN FALSE END;
		IF UpdaterMultivariate.ClassifyBlock(block) # GraphRules.normal THEN RETURN FALSE END;
		(*	only use this updater for fixed effects	*)
		i := 0;
		len := LEN(block);
		WHILE i < len DO
			IF block[i].depth > 1 THEN RETURN FALSE END;
			INC(i)
		END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: FactoryNormal) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: UpdaterNormal;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE InstallLogit*;
	BEGIN
		UpdaterUpdaters.SetFactory(factLogit)
	END InstallLogit;

	PROCEDURE InstallLoglin*;
	BEGIN
		UpdaterUpdaters.SetFactory(factLoglin)
	END InstallLoglin;

	PROCEDURE InstallNormal*;
	BEGIN
		UpdaterUpdaters.SetFactory(factNormal)
	END InstallNormal;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		CONST
			blockSize = 1;
			numLike = 1;
		VAR
			isRegistered: BOOLEAN;
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
			fLogit: FactoryLogit;
			fLoglin: FactoryLoglin;
			fNormal: FactoryNormal;
	BEGIN
		Maintainer;
		NEW(z, blockSize, numLike);
		NEW(offset, numLike);
		NEW(mu, numLike);
		NEW(n, numLike);
		NEW(y, numLike);
		NEW(w, numLike);
		NEW(derivLogLik, blockSize);
		NEW(priorMu, blockSize);
		NEW(newValues, blockSize);
		NEW(oldValues, blockSize);
		NEW(propMu, blockSize);
		NEW(priorTau, blockSize, blockSize);
		NEW(deriv2LogLik, blockSize, blockSize);
		NEW(choleskiDecomp, blockSize, blockSize);
		NEW(fLogit);
		fLogit.Install(name);
		fLogit.SetProps({UpdaterUpdaters.enabled});
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteSet(name + ".props", fLogit.props)
		END;
		fLogit.GetDefaults;
		factLogit := fLogit;
		NEW(fLoglin);
		fLoglin.Install(name);
		fLoglin.SetProps({UpdaterUpdaters.enabled});
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteSet(name + ".props", fLoglin.props)
		END;
		fLoglin.GetDefaults;
		factLoglin := fLoglin;
		NEW(fNormal);
		fNormal.Install(name);
		fNormal.SetProps({UpdaterUpdaters.enabled, UpdaterUpdaters.overRelaxation});
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteInt(name + ".overRelaxation", 16);
			BugsRegistry.WriteSet(name + ".props", fNormal.props)
		END;
		fNormal.GetDefaults;
		factNormal := fNormal
	END Init;

BEGIN
	Init
END UpdaterGLM.
