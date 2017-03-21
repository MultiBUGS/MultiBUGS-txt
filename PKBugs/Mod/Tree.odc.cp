(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE PKBugsTree;

	(* PKBugs Version 1.1 *)

	

	IMPORT
		PKBugsCovts,
		Strings, 
		BugsIndex, BugsMappers, BugsParser, BugsStrings;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE ForLoop (loop: BugsParser.Statement; IN string: ARRAY OF CHAR): BugsParser.Statement;
		VAR
			s: BugsMappers.Scanner;
	BEGIN
		BugsStrings.ConnectScanner(s, string);
		s.SetPos(0);
		s.Scan;
		RETURN BugsParser.ParseForLoop(loop, s)
	END ForLoop;

	PROCEDURE Variable (loop: BugsParser.Statement; IN string: ARRAY OF CHAR): BugsParser.Variable;
		VAR
			s: BugsMappers.Scanner;
	BEGIN
		BugsStrings.ConnectScanner(s, string);
		s.SetPos(0);
		s.Scan;
		RETURN BugsParser.ParseVariable(loop, s)
	END Variable;

	PROCEDURE Density (loop: BugsParser.Statement; IN string: ARRAY OF CHAR): BugsParser.Density;
		VAR
			s: BugsMappers.Scanner;
	BEGIN
		BugsStrings.ConnectScanner(s, string);
		s.SetPos(0);
		s.Scan;
		RETURN BugsParser.ParseDensity(loop, s)
	END Density;

	PROCEDURE Expression (loop: BugsParser.Statement; IN string: ARRAY OF CHAR): BugsParser.Node;
		VAR
			s: BugsMappers.Scanner;
	BEGIN
		BugsStrings.ConnectScanner(s, string);
		s.SetPos(0);
		s.Scan;
		RETURN BugsParser.ParseExpression(loop, s)
	END Expression;

	PROCEDURE Build* (nComp: INTEGER; log: BOOLEAN);
		VAR
			n, name, sp, sq, str: ARRAY 1024 OF CHAR;
			loops, model, statement: BugsParser.Statement;
			variable: BugsParser.Variable;
			density: BugsParser.Density;
			expression: BugsParser.Node;
			i, j, nCov, nPar, off: INTEGER;
	BEGIN
		Strings.IntToString(nComp, n);
		loops := NIL;
		model := NIL;
		statement := ForLoop(loops, "(i in 1 : n.ind) {");
		statement.CopyToList(loops);
		statement := ForLoop(loops, "(j in off.data[i] : off.data[i + 1] - 1){");
		statement.CopyToList(loops);
		variable := Variable(loops, "data[j]");
		str := "dnorm(model[j], tau)";
		IF BugsIndex.Find("lower") # NIL THEN
			IF BugsIndex.Find("upper") # NIL THEN
				str := str + "C(lower[j], upper[j])"
			ELSE
				str := str + "C(lower[j], )"
			END
		ELSIF BugsIndex.Find("upper") # NIL THEN
			str := str + "C(, upper[j])"
		END;
		density := Density(loops, str);
		statement := BugsParser.BuildStochastic(variable, density);
		statement.CopyToList(model); loops.MergeLists(model);
		variable := Variable(loops, "model[j]");
		IF log THEN str := "log." ELSE str := "" END;
		str := str + "pk.model(" + n + " , theta[i, 1:p], time[j], hist[off.hist[i]:(off.hist[i + 1] - 1), 1:n.col], pos[j])";
		expression := Expression(loops, str);
		statement := BugsParser.BuildLogical(variable, expression);
		statement.CopyToList(model); loops.MergeLists(model);
		loops := loops.next;
		variable := Variable(loops, "theta[i, 1:p]");
		density := Density(loops, "dmnorm(theta.mean[i, 1:p], omega.inv[1:p, 1:p])");
		statement := BugsParser.BuildStochastic(variable, density);
		statement.CopyToList(model); loops.MergeLists(model);

		(*	covariate stuff	*)
		ASSERT(PKBugsCovts.covariates # NIL, 25); nPar := LEN(PKBugsCovts.covariates);
		IF PKBugsCovts.covariateNames # NIL THEN
			nCov := LEN(PKBugsCovts.covariateNames)
		ELSE
			nCov := 0
		END;
		i := 0; off := 1;
		WHILE i < nPar DO
			Strings.IntToString(i + 1, sp); Strings.IntToString(off, sq); INC(off);
			str := "theta.mean[i, " + sp + "]";
			variable := Variable(loops, str);
			str := "mu[" + sq + "]";
			j := 0;
			WHILE j < 2 * nCov DO
				IF j IN PKBugsCovts.covariates[i] THEN
					name := PKBugsCovts.covariateNames[j]$;
					Strings.IntToString(off, sq); INC(off);
					str := str + (" + mu[") + sq + "] * " + name + "[i]"
				END;
				INC(j)
			END;
			expression := Expression(loops, str);
			statement := BugsParser.BuildLogical(variable, expression);
			statement.CopyToList(model); loops.MergeLists(model);
			INC(i)
		END;

		loops := loops.next; ASSERT(loops = NIL, 33);
		variable := Variable(loops, "tau");
		density := Density(loops, "dgamma(tau.a, tau.b)");
		statement := BugsParser.BuildStochastic(variable, density);
		statement.CopyToList(model);
		variable := Variable(loops, "sigma");
		expression := Expression(loops, "1 / sqrt(tau)");
		statement := BugsParser.BuildLogical(variable, expression);
		statement.CopyToList(model);
		variable := Variable(loops, "sigma.sq");
		expression := Expression(loops, "1 / tau");
		statement := BugsParser.BuildLogical(variable, expression);
		statement.CopyToList(model);
		variable := Variable(loops, "mu[1:q]");
		density := Density(loops, "dmnorm(mu.prior.mean[1:q], mu.prior.precision[1:q, 1:q])");
		statement := BugsParser.BuildStochastic(variable, density);
		statement.CopyToList(model);
		variable := Variable(loops, "omega.inv[1:p, 1:p]");
		density := Density(loops, "dwish(omega.inv.matrix[1:p, 1:p], omega.inv.dof)");
		statement := BugsParser.BuildStochastic(variable, density);
		statement.CopyToList(model);
		variable := Variable(loops, "omega[1:p, 1:p]");
		expression := Expression(loops, "inverse(omega.inv[1:p, 1:p])");
		statement := BugsParser.BuildLogical(variable, expression);
		statement.CopyToList(model);
		model := BugsParser.ReverseList(model);
		BugsParser.SetModel(model)
	END Build;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END PKBugsTree.
