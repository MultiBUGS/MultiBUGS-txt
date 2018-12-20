(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



   *)

MODULE UpdaterContinuous;


	

	IMPORT 
		GraphNodes, GraphStochastic, GraphUnivariate,
		UpdaterUnivariate;

	TYPE
		Updater* = POINTER TO ABSTRACT RECORD(UpdaterUnivariate.Updater) END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (updater: Updater) GenerateInit* (fixFounder: BOOLEAN; OUT res: SET);
		VAR
			prior: GraphStochastic.Node;
			location: REAL;
		CONST
			bounds = {GraphStochastic.leftImposed, GraphStochastic.rightImposed};
			univariate = FALSE;
	BEGIN
		res := {};
		prior := updater.prior;
		IF GraphStochastic.initialized IN prior.props THEN RETURN END;
		IF ~prior.CanSample(univariate) THEN res := {GraphNodes.lhs}; RETURN END;
		IF ~fixFounder OR (prior.depth # 1) OR (prior.props * bounds # {}) THEN
			prior.Sample(res);
			IF res # {} THEN RETURN END
		ELSE
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
