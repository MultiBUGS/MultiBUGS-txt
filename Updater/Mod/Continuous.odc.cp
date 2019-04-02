(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



   *)

MODULE UpdaterContinuous;


	

	IMPORT 
		GraphNodes, GraphMultivariate, GraphStochastic, GraphUnivariate,
		UpdaterUnivariate;

	TYPE
		Updater* = POINTER TO ABSTRACT RECORD(UpdaterUnivariate.Updater) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (updater: Updater) GenerateInit* (fixFounder: BOOLEAN; OUT res: SET);
		VAR
			prior: GraphStochastic.Node;
			mv: GraphMultivariate.Node;
			location: REAL;
			i, size, numData: INTEGER;
		CONST
			bounds = {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
			univariate = FALSE;
			multivariate = TRUE;
	BEGIN
		res := {};
		prior := updater.prior;
		IF GraphStochastic.initialized IN prior.props THEN RETURN END;
		IF ~fixFounder OR (prior.depth # 1) OR (prior.props * bounds # {}) THEN
			IF prior IS GraphMultivariate.Node THEN
				IF ~prior.CanSample(multivariate) THEN res := {GraphNodes.lhs}; RETURN END;
				mv := prior(GraphMultivariate.Node);
				i := 0;
				size := mv.Size();
				numData := 0;
				WHILE i < size DO
					IF GraphStochastic.data IN mv.components[i].props THEN INC(numData) END;
					INC(i)
				END;
				IF numData = 0 THEN
					IF ~prior.CanSample(multivariate) THEN res := {GraphNodes.lhs}; RETURN END;
					mv.MVSample(res)
				ELSE
					IF ~prior.CanSample(univariate) THEN res := {GraphNodes.lhs}; RETURN END;
					prior.Sample(res)
				END
			ELSE
				IF ~prior.CanSample(univariate) THEN res := {GraphNodes.lhs}; RETURN END;
				prior.Sample(res)
			END;
			IF res # {} THEN RETURN END
		ELSE
			IF ~prior.CanSample(univariate) THEN res := {GraphNodes.lhs}; RETURN END;
			location := prior.Location();
			prior.SetValue(location);
			res := {};
		END;
		prior.SetProps(prior.props + {GraphStochastic.initialized})
	END GenerateInit;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END UpdaterContinuous.
