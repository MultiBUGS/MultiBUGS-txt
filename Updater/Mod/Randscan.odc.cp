(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

(*	this block updater updates a random subset of the block at each iteration using a current point normal
proposal. The precision of the proposal is adapted	*)

MODULE UpdaterRandscan;


	

	IMPORT
		Math, Stores,
		BugsGraph, BugsRegistry,
		GraphNodes, GraphRules, GraphStochastic,
		MathRandnum,
		UpdaterMultivariate, UpdaterUpdaters;

	TYPE
		Matrix = ARRAY OF ARRAY OF REAL;

		Vector = ARRAY OF REAL;

		Updater = POINTER TO RECORD (UpdaterMultivariate.Updater)
			updateSites: POINTER TO ARRAY OF INTEGER;
			numUpdating, numDensity, numLikelihood: INTEGER;
			iterations: POINTER TO ARRAY OF INTEGER;
			logSigma: POINTER TO ARRAY OF REAL;
			rejectCount: POINTER TO ARRAY OF INTEGER
		END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		oldVals, newVals: POINTER TO Vector;
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	CONST
		batch = 50;
		deltaMax = 0.01;
		optRate = 0.44;

	PROCEDURE (updater: Updater) ParamsSize (): INTEGER;
	BEGIN
		RETURN 0
	END ParamsSize;

	PROCEDURE (updater: Updater) Clone (): Updater;
		VAR
			u: Updater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Updater) CopyFromMultivariate (source: UpdaterUpdaters.Updater);
	BEGIN
	END CopyFromMultivariate;

	PROCEDURE (updater: Updater) FindBlock (prior: GraphStochastic.Node): GraphStochastic.Vector;
		VAR
			block: GraphStochastic.Vector;
	BEGIN
		block := BugsGraph.ConditionalsOfClass({GraphRules.genDiff.. GraphRules.mVNLin});
		RETURN block
	END FindBlock;

	PROCEDURE (updater: Updater) InitializeRandscan, NEW;
		VAR
			i, nElem: INTEGER;
	BEGIN
		nElem := updater.Size();
		NEW(updater.iterations, nElem);
		NEW(updater.rejectCount, nElem);
		NEW(updater.logSigma, nElem);
		i := 0;
		WHILE i < nElem DO
			updater.iterations[i] := 0;
			updater.rejectCount[i] := 0;
			updater.logSigma[i] := 1.0;
			INC(i)
		END;
		IF nElem > LEN(oldVals) THEN
			NEW(oldVals, nElem); NEW(newVals, nElem)
		END
	END InitializeRandscan;

	PROCEDURE (updater: Updater) InitializeMultivariate;
		VAR
			depth, i, nElem, numLikelihood: INTEGER;
			flat: BOOLEAN;
			prior: GraphStochastic.Vector;
	BEGIN
		updater.InitializeRandscan;
		nElem := updater.Size();
		i := 0;
		updater.numUpdating := 0;
		NEW(updater.updateSites, nElem);
		i := 0;
		WHILE i < nElem DO updater.updateSites[i] :=  - 1; INC(i) END;
		prior := updater.prior;
		depth := prior[0].depth;
		flat := TRUE;
		i := 0;
		WHILE flat & (i < nElem) DO
			flat := prior[i].depth = depth;
			INC(i)
		END;
		numLikelihood := 0;
		IF flat THEN
			(*numLikelihood := LEN(likelihood)*)
		END;
		updater.numLikelihood := numLikelihood
	END InitializeMultivariate;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) NewLogDensity (): REAL, NEW;
		VAR
			logDensity: REAL;
			i, j, numDensity, site, numLike: INTEGER;
			p, prior: GraphStochastic.Node;
			children: GraphStochastic.Vector;
	BEGIN
		logDensity := 0.0;
		numDensity := updater.numDensity;
		i := 0;
		WHILE i < updater.numUpdating DO
			site := updater.updateSites[i];
			prior := updater.prior[site];
			prior.SetProps(prior.props - {GraphNodes.mark});
			logDensity := logDensity + prior.LogLikelihood() + prior.LogJacobian();
			INC(i)
		END;
		i := 0;
		WHILE (i < updater.numUpdating) & (numDensity > 0) DO
			site := updater.updateSites[i];
			prior := updater.prior[site];
			children := prior.Children();
			j := 0;
			numLike := LEN(children);
			WHILE (j < numLike) & (numDensity > 0) DO
				p := children[j];
				IF GraphNodes.mark IN p.props THEN
					DEC(numDensity);
					p.SetProps(p.props - {GraphNodes.mark});
					logDensity := logDensity + p.LogLikelihood()
				END;
				INC(j)
			END;
			INC(i)
		END;
		RETURN logDensity
	END NewLogDensity;

	PROCEDURE (updater: Updater) OldLogDensity (): REAL, NEW;
		VAR
			logDensity: REAL;
			i, j, numDensity, numLikelihood, numChild, site: INTEGER;
			p, prior: GraphStochastic.Node;
			children: GraphStochastic.Vector;
			all: BOOLEAN;
	BEGIN
		logDensity := 0.0;
		numDensity := 0;
		numLikelihood := updater.numLikelihood;
		i := 0;
		WHILE i < updater.numUpdating DO
			site := updater.updateSites[i];
			prior := updater.prior[site];
			prior.SetProps(prior.props + {GraphNodes.mark});
			logDensity := logDensity + prior.LogLikelihood() + prior.LogJacobian();
			INC(i)
		END;
		i := 0;
		all := FALSE;
		WHILE ~ all & (i < updater.numUpdating) DO
			site := updater.updateSites[i];
			prior := updater.prior[site];
			children := prior.Children();
			numChild := LEN(children);
			j := 0;
			WHILE ~all & (j < numChild) DO
				p := children[j];
				IF ~(GraphNodes.mark IN p.props) THEN
					INC(numDensity);
					p.SetProps(p.props + {GraphNodes.mark});
					logDensity := logDensity + p.LogLikelihood();
					all := numDensity = numLikelihood
				END;
				INC(j)
			END;
			INC(i)
		END;
		updater.numDensity := numDensity;
		RETURN logDensity
	END OldLogDensity;

	PROCEDURE (updater: Updater) SelectActiveVariables, NEW;
		VAR
			i, numUpdating, nElem: INTEGER;
			p: REAL;
		CONST
			n1 = 2;
	BEGIN
		nElem := updater.Size();
		numUpdating := 0;
		p := MIN(1, n1 / nElem);
		i := 0;
		WHILE i < nElem DO
			IF MathRandnum.Rand() < p THEN
				updater.updateSites[numUpdating] := i;
				INC(numUpdating)
			END;
			INC(i)
		END;
		updater.numUpdating := numUpdating;
	END SelectActiveVariables;

	(*	methods specific to Filzbach	*)
	PROCEDURE (updater: Updater) AdaptProposal (rate: REAL; i: INTEGER), NEW;
		VAR
			delta: REAL;
	BEGIN
		delta := MIN(deltaMax, 1.0 / Math.Sqrt(updater.iterations[i]));
		IF rate > optRate THEN
			updater.logSigma[i] := updater.logSigma[i] + delta
		ELSE
			updater.logSigma[i] := updater.logSigma[i] - delta
		END
	END AdaptProposal;

	PROCEDURE (updater: Updater) ExternalizeMultivariate (VAR wr: Stores.Writer);
		VAR
			i, nElem: INTEGER;
	BEGIN
		nElem := updater.Size();
		i := 0;
		WHILE i < nElem DO
			wr.WriteInt(updater.iterations[i]);
			wr.WriteInt(updater.rejectCount[i]);
			wr.WriteReal(updater.logSigma[i]);
			INC(i)
		END
	END ExternalizeMultivariate;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterRandscan.Install"
	END Install;

	PROCEDURE (updater: Updater) InternalizeMultivariate (VAR rd: Stores.Reader);
		VAR
			i, nElem: INTEGER;
	BEGIN
		nElem := updater.Size();
		i := 0;
		WHILE i < nElem DO
			rd.ReadInt(updater.iterations[i]);
			rd.ReadInt(updater.rejectCount[i]);
			rd.ReadReal(updater.logSigma[i]);
			INC(i)
		END
	END InternalizeMultivariate;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			logAlpha, newLogDensity, oldLogDensity, prec, rate, mappedVal: REAL;
			i, numUpdating, site, j, iterations: INTEGER;
			accept: BOOLEAN;
			prior: GraphStochastic.Node;
	BEGIN
		iterations := fact.iterations;
		j := 0;
		WHILE j < iterations DO
			updater.SelectActiveVariables;
			oldLogDensity := updater.OldLogDensity();
			numUpdating := updater.numUpdating;
			(*	map each variable onto the whole real line and sample a new
			candidate using a random walk normal proposal	*)
			i := 0;
			WHILE i < numUpdating DO
				site := updater.updateSites[i];
				prior := updater.prior[site];
				oldVals[i] := prior.value;
				mappedVal := prior.Map();
				prec := Math.Exp( - 2.0 * updater.logSigma[site]);
				newVals[i] := MathRandnum.Normal(mappedVal, prec);
				INC(updater.iterations[site]);
				INC(i)
			END;
			(*	map new values back to correct domain	*)
			i := 0;
			WHILE i < numUpdating DO
				site := updater.updateSites[i];
				prior := updater.prior[site];
				prior.InvMap(newVals[i]);
				INC(i)
			END;
			newLogDensity := updater.NewLogDensity();
			logAlpha := newLogDensity - oldLogDensity;
			accept := logAlpha > Math.Ln(MathRandnum.Rand());
			IF ~accept THEN
				i := 0;
				WHILE i < numUpdating DO
					site := updater.updateSites[i];
					prior := updater.prior[site];
					prior.SetValue(oldVals[i]);
					INC(updater.rejectCount[site]);
					INC(i)
				END;
			END;
			i := 0;
			WHILE i < numUpdating DO
				site := updater.updateSites[i];
				IF updater.iterations[site] MOD batch = 0 THEN
					rate := (batch - updater.rejectCount[site]) / batch;
					updater.rejectCount[site] := 0;
					updater.AdaptProposal(rate, site);
				END;
				INC(i)
			END;
			INC(j)
		END;
		res := {}
	END Sample;

	(*	factory methods	*)

	PROCEDURE (f: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterRandscan.Install"
	END Install;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
		VAR
			block: GraphStochastic.Vector;
	BEGIN
		IF GraphStochastic.integer IN prior.props THEN
			RETURN FALSE
		END;
		block := BugsGraph.ConditionalsOfClass({GraphRules.genDiff.. GraphRules.mVNLin});
		IF block = NIL THEN RETURN FALSE END;
		RETURN TRUE
	END CanUpdate;

	PROCEDURE (f: Factory) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: Updater;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE (f: Factory) GetDefaults;
		VAR
			iterations, res: INTEGER;
			props: SET;
			name: ARRAY 256 OF CHAR;
	BEGIN
		f.Install(name);
		BugsRegistry.ReadInt(name + ".iterations", iterations, res); ASSERT(res = 0, 55);
		BugsRegistry.ReadSet(name + ".props", props, res); ASSERT(res = 0, 55);
		f.SetParameter(iterations, UpdaterUpdaters.iterations);
		f.SetProps(props)
	END GetDefaults;

	PROCEDURE Install*;
	BEGIN
		UpdaterUpdaters.SetFactory(fact)
	END Install;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			f: Factory;
			isRegistered: BOOLEAN;
			res: INTEGER;
			name: ARRAY 256 OF CHAR;
	BEGIN
		Maintainer;
		NEW(oldVals, 1);
		NEW(newVals, 1);
		NEW(f);
		f.Install(name);
		f.SetProps({UpdaterUpdaters.iterations});
		BugsRegistry.ReadBool(name + ".isRegistered", isRegistered, res);
		IF res = 0 THEN ASSERT(isRegistered, 55)
		ELSE
			BugsRegistry.WriteBool(name + ".isRegistered", TRUE);
			BugsRegistry.WriteInt(name + ".iterations", 1);
			BugsRegistry.WriteSet(name + ".props", f.props)
		END;
		f.GetDefaults;
		fact := f
	END Init;

BEGIN
	Init
END UpdaterRandscan.
