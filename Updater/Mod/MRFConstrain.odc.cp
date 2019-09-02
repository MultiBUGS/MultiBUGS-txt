(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterMRFConstrain;


	

	IMPORT
		Stores := Stores64, 
		GraphMRF, GraphMultivariate, GraphNodes, GraphStochastic,
		UpdaterUpdaters;

	TYPE
		Updater = POINTER TO RECORD(UpdaterUpdaters.Updater)
			numConstraints: INTEGER;
			constraints: POINTER TO ARRAY OF ARRAY OF REAL;
			mrf: GraphMRF.Node
		END;

		Factory = POINTER TO RECORD (UpdaterUpdaters.Factory) END;

	VAR
		fact-: UpdaterUpdaters.Factory;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (updater: Updater) Children (): GraphStochastic.Vector;
	BEGIN
		RETURN NIL
	END Children;

	PROCEDURE (updater: Updater) Clone (): Updater;
		VAR
			u: Updater;
	BEGIN
		NEW(u);
		RETURN u
	END Clone;

	PROCEDURE (updater: Updater) CopyFrom (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
	BEGIN
		s := source(Updater);
		updater.constraints := s.constraints;
		updater.mrf := s.mrf
	END CopyFrom;

	PROCEDURE (updater: Updater) Depth (): INTEGER;
		VAR
			depth: INTEGER;
			mRF: GraphMRF.Node;
	BEGIN
		mRF := updater.mrf;
		depth := mRF.depth;
		IF mRF.children = NIL THEN depth := - depth END;
		RETURN depth
	END Depth;

	PROCEDURE (updater: Updater) Externalize (VAR wr: Stores.Writer);
	BEGIN
	END Externalize;

	PROCEDURE (updater: Updater) ExternalizePrior (VAR wr: Stores.Writer);
	BEGIN
		GraphNodes.Externalize(updater.mrf, wr)
	END ExternalizePrior;

	PROCEDURE (updater: Updater) GenerateInit (fixFounder: BOOLEAN; OUT res: SET);
		VAR
			mRF: GraphMRF.Node;
			p: GraphStochastic.Node;
			i, size: INTEGER;
	BEGIN
		mRF := updater.mrf;
		size := mRF.Size();
		i := 0;
		res := {};
		WHILE (i < size) & (res = {}) DO
			p := mRF.components[i];
			IF ~(GraphStochastic.initialized IN p.props) THEN
				res := {GraphNodes.lhs}
			END;
			INC(i)
		END;
		IF res # {} THEN mRF.MVSample(res) END;
		i := 0;
		WHILE i < size DO 
			p := mRF.components[i];
			INCL(p.props, GraphStochastic.initialized);
			INC(i)
		END
	END GenerateInit;

	PROCEDURE (updater: Updater) Initialize;
		VAR
			nElements, type, num, size: INTEGER;
			mRF: GraphMRF.Node;
			colPtr, rowInd: POINTER TO ARRAY OF INTEGER;
	BEGIN
		mRF := updater.mrf;
		size := mRF.Size();
		updater.numConstraints := mRF.NumberConstraints();
		num := updater.numConstraints;
		IF num > 0 THEN
			NEW(updater.constraints, num, size);
			updater.mrf.Constraints(updater.constraints);
		ELSE
			updater.constraints := NIL
		END
	END Initialize;

	PROCEDURE (updater: Updater) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterMRFConstrain.Install"
	END Install;

	PROCEDURE (updater: Updater) Internalize (VAR rd: Stores.Reader);
	BEGIN
	END Internalize;

	PROCEDURE (updater: Updater) InternalizePrior (VAR rd: Stores.Reader);
		VAR
			p: GraphNodes.Node;
			mV: GraphMultivariate.Node;
			i: INTEGER;
	BEGIN
		p := GraphNodes.Internalize(rd);
		mV := p(GraphMultivariate.Node);
		i := 0;
		REPEAT
			p := mV.components[i];
			INC(i)
		UNTIL p IS GraphMRF.Node;
		updater.mrf := p(GraphMRF.Node)
	END InternalizePrior;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) LogConditional (): REAL;
	BEGIN
		RETURN 0.0
	END LogConditional;

	PROCEDURE (updater: Updater) LogLikelihood (): REAL;
	BEGIN
		RETURN 0.0
	END LogLikelihood;

	PROCEDURE (updater: Updater) LogPrior (): REAL;
	BEGIN
		RETURN 0.0
	END LogPrior;

	PROCEDURE (updater: Updater) Node (index: INTEGER): GraphStochastic.Node;
	BEGIN
		RETURN updater.mrf
	END Node;

	PROCEDURE (updater: Updater) Prior (index: INTEGER): GraphStochastic.Node;
	BEGIN
		RETURN updater.mrf.components[index]
	END Prior;

	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			i, size: INTEGER;
			sum, mean, num, val, sum1: REAL;
			mRF: GraphMRF.Node;
			com: GraphStochastic.Vector;
	BEGIN
		res := {};
		mRF := updater.mrf;
		com := mRF.components;
		size := mRF.Size();
		sum := 0.0;
		num := 0.0;
		i := 0;
		WHILE i < size DO
			sum := sum + updater.constraints[0, i] * com[i].value;
			num := num + updater.constraints[0, i];
			INC(i)
		END;
		mean := sum / num;
		sum1 := 0.0;
		i := 0;
		WHILE i < size DO
			val := com[i].value - mean;
			sum1 := sum1 + val;
			com[i].value := val;
			INC(i)
		END
	END Sample;

	PROCEDURE (updater: Updater) SetPrior (prior: GraphStochastic.Node);
		VAR
			mRF: GraphMRF.Node;
			p: GraphStochastic.Node;
	BEGIN
		mRF := prior(GraphMRF.Node);
		p := mRF.components[0];
		updater.mrf := p(GraphMRF.Node)
	END SetPrior;

	PROCEDURE (updater: Updater) Size (): INTEGER;
	BEGIN
		RETURN 0
	END Size;

	PROCEDURE (f: Factory) Install (OUT install: ARRAY OF CHAR);
	BEGIN
		install := "UpdaterMRFConstrain.Install"
	END Install;

	PROCEDURE (f: Factory) CanUpdate (prior: GraphStochastic.Node): BOOLEAN;
	BEGIN
		WITH prior: GraphMRF.Node DO
			RETURN prior.NumberConstraints() # 0
		ELSE
			RETURN FALSE
		END
	END CanUpdate;

	PROCEDURE (f: Factory) Create (): UpdaterUpdaters.Updater;
		VAR
			updater: Updater;
	BEGIN
		NEW(updater);
		RETURN updater
	END Create;

	PROCEDURE (f: Factory) GetDefaults;
	BEGIN
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
	BEGIN
		Maintainer;
		NEW(f);
		f.SetProps({UpdaterUpdaters.enabled});
		fact := f
	END Init;

BEGIN
	Init
END UpdaterMRFConstrain.
