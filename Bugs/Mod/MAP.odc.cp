(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsMAP;

	

	IMPORT
		Math, Strings,
		BugsIndex, BugsMappers, BugsNames, 
		GraphMAP, GraphStochastic, 
		MathMatrix;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Differential (prior: GraphStochastic.Node): REAL;
		VAR
			diff: REAL;
			children: GraphStochastic.Vector;
			i, num: INTEGER;
	BEGIN
		diff := prior.DiffLogPrior();
		children := prior.Children();
		IF children # NIL THEN num := LEN(children) ELSE num := 0 END;
		i := 0;
		WHILE i < num DO
			diff := diff + children[i].DiffLogLikelihood(prior);
			INC(i)
		END;
		RETURN diff
	END Differential;

	PROCEDURE Deviance (): REAL;
		VAR
			deviance: REAL;
			name: BugsNames.Name;
	BEGIN
		name := BugsIndex.Find("deviance");
		deviance := name.components[0].Value();
		RETURN deviance
	END Deviance;

	PROCEDURE Estimates* (priors: GraphStochastic.Vector; VAR f: BugsMappers.Formatter);
		VAR
			inverse: POINTER TO ARRAY OF ARRAY OF REAL;
			oldVals: POINTER TO ARRAY OF REAL;
			i, j, dim, pos, prec: INTEGER;
			label: ARRAY 128 OF CHAR;
			deviance, sd: REAL;
	BEGIN
		dim := LEN(priors);
		NEW(oldVals, dim);
		NEW(inverse, dim, dim);
		i := 0;
		WHILE i < dim DO
			oldVals[i] := priors[i].value; INC(i)
		END;
		GraphMAP.MAP(priors);
		GraphMAP.Hessian(priors, inverse);
		i := 0;
		WHILE i < dim DO
			j := 0;
			WHILE j < dim DO
				inverse[i, j] := - inverse[i, j];
				INC(j)
			END;
			INC(i)
		END;
		MathMatrix.Invert(inverse, dim);
		deviance := Deviance();
		prec := BugsMappers.prec;
		BugsMappers.SetPrec(8);
		f.WriteString("deviance: ");
		f.WriteReal(deviance);
		f.WriteString("   AIC: ");
		f.WriteReal(deviance + 2 * dim); 
		BugsMappers.SetPrec(5);
		f.WriteLn; f.WriteLn;
		f.Bold;
		f.WriteTab;
		f.WriteString(" name"); f.WriteTab;
		f.WriteString("MAP"); f.WriteTab;
		f.WriteString("sd"); f.WriteTab;
		f.WriteString("derivative"); f.WriteTab;
		f.WriteString("correlations");
		f.WriteLn;
		f.Bold;
		i := 0;
		WHILE i < dim DO
			f.WriteTab;
			BugsIndex.FindGraphNode(priors[i], label);
			Strings.Find(label, ">", 0, pos);
			label[pos] := 0X;
			label[0] := " ";
			f.WriteString(label); f.WriteTab;
			f.WriteReal(priors[i].value); f.WriteTab;
			sd := Math.Sqrt(inverse[i, i]);
			f.WriteReal(sd); f.WriteTab;
			f.WriteReal(Differential(priors[i])); f.WriteTab;
			j := 0;
			WHILE j <= i DO
				f.WriteReal(inverse[i, j] / Math.Sqrt(inverse[i, i] * inverse[j, j])); f.WriteTab; INC(j)
			END;
			f.WriteLn;
			INC(i)
		END;
		i := 0;
		WHILE i < dim DO
			priors[i].SetValue(oldVals[i]); INC(i)
		END;
		BugsMappers.SetPrec(prec)
	END Estimates;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer
	END Init;

BEGIN
	Init
END BugsMAP.

