(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE UpdaterMRFConstrain;


	

	IMPORT
		Stores,
		GraphMRF, GraphNodes, GraphStochastic,
		UpdaterUpdaters;

	TYPE
		Updater = POINTER TO RECORD(UpdaterUpdaters.Updater)
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
			mrf: GraphMRF.Node;
	BEGIN
		mrf := updater.mrf;
		depth := mrf.depth;
		IF mrf.Children() = NIL THEN depth := -depth END;
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
	BEGIN
		res := {}
	END GenerateInit;
	
	PROCEDURE (updater: Updater) Initialize;
		VAR
			constraint: POINTER TO ARRAY OF ARRAY OF REAL;
			num, size: INTEGER;
			mrf: GraphMRF.Node;
	BEGIN
		mrf := updater.mrf;
		size := mrf.Size();
		num := mrf.NumberConstraints();
		IF num > 0 THEN
			NEW(updater.constraints, num, size);
			updater.mrf.Constraints(updater.constraints)
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
	BEGIN
		p := GraphNodes.Internalize(rd);
		updater.mrf := p(GraphMRF.Node)
	END InternalizePrior;

	PROCEDURE (updater: Updater) IsAdapting (): BOOLEAN;
	BEGIN
		RETURN FALSE
	END IsAdapting;

	PROCEDURE (updater: Updater) IsInitialized (): BOOLEAN;
	BEGIN
		RETURN TRUE
	END IsInitialized;

	PROCEDURE (updater: Updater) LoadSample;
	BEGIN
	END LoadSample;
	
	PROCEDURE (updater: Updater) LogConditional (): REAL;
	BEGIN
		RETURN 0.0
	END LogConditional;
	
	PROCEDURE (updater: Updater) LogLikelihood (): REAL;
	BEGIN
		RETURN 0.0
	END LogLikelihood;
	
	PROCEDURE (updater: Updater) Prior (index: INTEGER): GraphStochastic.Node;
	BEGIN
		RETURN updater.mrf.components[index]
	END Prior;
	
	PROCEDURE (updater: Updater) Sample (overRelax: BOOLEAN; OUT res: SET);
		VAR
			mrf: GraphMRF.Node;
			components: GraphStochastic.Vector;
			i, j, numConstraints, size: INTEGER;
			numNonZero, sum, value: REAL;
	BEGIN
		res := {};
		mrf := updater.mrf;
		components := mrf.components;
		size := mrf.Size();
		IF updater.constraints # NIL THEN
			numConstraints := LEN(updater.constraints);
			mrf.Constraints(updater.constraints);
			j := 0;
			WHILE j < numConstraints DO
				i := 0;
				sum := 0.0;
				numNonZero := 0;
				WHILE i < size DO
					sum := sum + updater.constraints[j, i] * components[i].value;
					numNonZero := numNonZero + updater.constraints[j, i];
					INC(i)
				END;
				sum := sum / numNonZero;
				i := 0;
				WHILE i < size DO
					IF updater.constraints[j, i] > 0.5 THEN
						value := components[i].value - sum;
						components[i].SetValue(value)
					END;
					INC(i)
				END;
				INC(j)
			END
		END
	END Sample;

	PROCEDURE (updater: Updater) SetChildren (children: GraphStochastic.Vector);
	BEGIN
	END SetChildren;
	
	PROCEDURE (updater: Updater) SetPrior (prior: GraphStochastic.Node);
		VAR
			mrf: GraphMRF.Node;
			p: GraphStochastic.Node;
	BEGIN
		mrf := prior(GraphMRF.Node);
		p := mrf.components[0];
		updater.mrf := p(GraphMRF.Node)
	END SetPrior;
	
	PROCEDURE (updater: Updater) Size (): INTEGER;
	BEGIN
		RETURN 0
	END Size;
	
	PROCEDURE (updater: Updater) StoreSample;
	BEGIN
	END StoreSample;
	
	PROCEDURE (updater: Updater) UpdatedBy (index: INTEGER): GraphStochastic.Node;
	BEGIN
		RETURN updater.mrf
	END UpdatedBy;
	
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
