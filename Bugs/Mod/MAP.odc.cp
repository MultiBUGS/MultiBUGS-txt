(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsMAP;

	

	IMPORT
		Math, Strings,
		TextMappers, TextModels,
		BugsFiles, BugsIndex, BugsNames,
		GraphLogical, GraphMAP, GraphStochastic,
		MathMatrix;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	CONST
		bold = 700;

	PROCEDURE WriteReal (x: REAL; VAR f: TextMappers.Formatter);
	BEGIN
		f.WriteRealForm(x, BugsFiles.prec, 0, 0, TextModels.digitspace)
	END WriteReal;

	PROCEDURE Deviance (): REAL;
		VAR
			deviance: REAL;
			name: BugsNames.Name;
			nodes: GraphLogical.Vector;
	BEGIN
		name := BugsIndex.Find("deviance");
		NEW(nodes, 1);
		nodes[0] := name.components[0](GraphLogical.Node);
		GraphLogical.Evaluate(nodes);
		deviance := nodes[0].value;
		RETURN deviance
	END Deviance;

	PROCEDURE Estimates* (priors: GraphStochastic.Vector; VAR f: TextMappers.Formatter);
		VAR
			inverse: POINTER TO ARRAY OF ARRAY OF REAL;
			deriv, oldVals: POINTER TO ARRAY OF REAL;
			i, j, dim, pos, prec: INTEGER;
			label: ARRAY 128 OF CHAR;
			deviance, sd: REAL;
			newAttr, oldAttr: TextModels.Attributes;
			error: BOOLEAN;
	BEGIN
		dim := LEN(priors);
		NEW(oldVals, dim);
		NEW(deriv, dim);
		NEW(inverse, dim, dim);
		i := 0;
		WHILE i < dim DO
			oldVals[i] := priors[i].value; INC(i)
		END;
		GraphMAP.MAP(priors, error);
		i := 0;
		WHILE i < dim DO
			deriv[i] := GraphMAP.DiffLogConditional(priors[i]); INC(i)
		END;
		IF error THEN
			f.WriteString("can not find MAP: "); f.WriteLn;
			i := 0;
			WHILE i < dim DO
				WriteReal(priors[i].value, f); f.WriteTab; WriteReal(deriv[i], f); f.WriteLn; INC(i)
			END;
			i := 0; WHILE i < dim DO priors[i].value := oldVals[i]; INC(i) END;
			RETURN
		END;
		GraphMAP.Hessian(priors, inverse, error);
		IF ~error THEN
			i := 0;
			WHILE i < dim DO
				j := 0;
				WHILE j < dim DO
					inverse[i, j] := - inverse[i, j];
					INC(j)
				END;
				INC(i)
			END;
			MathMatrix.Invert(inverse, dim)
		END;
		deviance := Deviance();
		prec := BugsFiles.prec;
		BugsFiles.SetPrec(8);
		f.WriteString("deviance: ");
		WriteReal(deviance, f);
		f.WriteString("   AIC: ");
		WriteReal(deviance + 2 * dim, f);
		BugsFiles.SetPrec(5);
		f.WriteLn; f.WriteLn;
		oldAttr := f.rider.attr;
		newAttr := TextModels.NewWeight(oldAttr, bold);
		f.rider.SetAttr(newAttr);
		f.WriteTab;
		f.WriteString(" name"); f.WriteTab;
		f.WriteString("MAP"); f.WriteTab;
		IF ~error THEN f.WriteString("sd"); f.WriteTab END;
		f.WriteString("derivative"); f.WriteTab;
		IF ~error THEN f.WriteString("correlations") END; ;
		f.WriteLn;
		f.rider.SetAttr(oldAttr);
		i := 0;
		WHILE i < dim DO
			f.WriteTab;
			BugsIndex.FindGraphNode(priors[i], label);
			Strings.Find(label, ">", 0, pos);
			label[pos] := 0X;
			label[0] := " ";
			f.WriteString(label); f.WriteTab;
			WriteReal(priors[i].value, f); f.WriteTab;
			IF ~error THEN
				sd := Math.Sqrt(inverse[i, i]);
				WriteReal(sd, f); f.WriteTab;
			END;
			WriteReal(deriv[i], f); f.WriteTab;
			IF ~error THEN
				j := 0;
				WHILE j <= i DO
					WriteReal(inverse[i, j] / Math.Sqrt(inverse[i, i] * inverse[j, j]), f); f.WriteTab; INC(j)
				END
			END;
			f.WriteLn;
			INC(i)
		END;
		i := 0;
		WHILE i < dim DO
			priors[i].value := oldVals[i]; INC(i)
		END;
		GraphLogical.EvaluateAllDiffs;
		BugsFiles.SetPrec(prec)
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

