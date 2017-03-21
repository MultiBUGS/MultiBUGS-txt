(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE GraphRules;


	CONST

		(* classifications of form of logical functions *)

		const* = 0; ident* = 1; prod* = 2; linear* = 3;
		logLink* = 4; logitLink* = 5; probitLink* = 6; cloglogLink* = 7;
		linkFun* = 8; expon* = 9; linExp* = 11; power* = 11; differ* = 12;
		other* = 13;
		noClasses = 14;


		(* classification of form of distribution *)

		invalid* = 0; catagorical* = 1; descrete* = 2; poisson* = 3;  multinomial* = 4; general* = 5; 
		genDiff* = 6; beta* = 7; beta1* = 8; cloglogReg* = 9; 
		dirichletPrior* = 10; gamma* = 11; gamma1* = 12; logCon* = 13; logReg* = 14;
		logitReg* = 15; normal* = 16; pareto* = 17; probitReg* = 18; 
		unif* = 19; mVN* = 20; mVNLin* = 21; dirichlet* = 22; wishart* = 23;
		noDensity = 24;

	VAR

		(* rules for combining forms of logical function *)

		expF*, linkF*, logF*, logitF*, probitF*, cloglogF*,
		uminusF*, uopF*, nonDiff1F*: ARRAY noClasses OF INTEGER;
		addF*, divF*, nonDiff2F*, multF*,
		orF*, powF*, subF*: ARRAY noClasses, noClasses OF INTEGER;

		(* rule for combining forms of distributions *)

		product*: ARRAY noDensity, noDensity OF INTEGER;

		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE ClassifyProportion* (form: INTEGER): INTEGER;
		VAR
			density: INTEGER;
	BEGIN
		CASE form OF
		|const:
			density := unif
		|ident:
			density := beta1
		|cloglogLink:
			density := cloglogReg
		|logitLink:
			density := logitReg
		|probitLink:
			density := probitReg
		|linkFun:
			density := logCon
		|logLink:
			density := genDiff
		|differ:
			density := genDiff
		ELSE
			density := general
		END;
		RETURN density
	END ClassifyProportion;

	PROCEDURE ClassifyPrecision* (form: INTEGER): INTEGER;
		VAR
			density: INTEGER;
	BEGIN
		CASE form OF
		|const:
			density := unif
		|ident, prod:
			density := gamma1
		|linear, expon:
			density := logCon
		|logLink:
			density := logReg
		|differ:
			density := genDiff
		ELSE
			density := general
		END;
		RETURN density
	END ClassifyPrecision;

	PROCEDURE ClassifyShape* (form: INTEGER): INTEGER;
		VAR
			density: INTEGER;
	BEGIN
		CASE form OF
		|const:
			density := unif
		|ident, prod, linear:
			density := logCon
		|differ:
			density := genDiff
		ELSE
			density := general
		END;
		RETURN density
	END ClassifyShape;

	PROCEDURE UminusRule;
		VAR
			i: INTEGER;
	BEGIN
		i := 0;
		WHILE i < noClasses DO
			uminusF[i] := differ;
			INC(i)
		END;
		uminusF[const] := const;
		uminusF[ident] := prod;
		uminusF[prod] := prod;
		uminusF[linear] := linear;
		uminusF[other] := other
	END UminusRule;

	PROCEDURE ExpRule;
		VAR
			i: INTEGER;
	BEGIN
		i := 0;
		WHILE i < noClasses DO
			expF[i] := differ;
			INC(i)
		END;
		expF[const] := const;
		expF[ident] := expon;
		expF[prod] := expon;
		expF[linear] := expon;
		expF[other] := other
	END ExpRule;

	PROCEDURE LinkRule;
		VAR
			i: INTEGER;
	BEGIN
		i := 0;
		WHILE i < noClasses DO
			linkF[i] := differ;
			INC(i)
		END;
		linkF[const] := const;
		linkF[ident] := linkFun;
		linkF[prod] := linkFun;
		linkF[linear] := linkFun;
		linkF[other] := other
	END LinkRule;

	PROCEDURE LogRule;
		VAR
			i: INTEGER;
	BEGIN
		i := 0;
		WHILE i < noClasses DO
			logF[i] := differ;
			INC(i)
		END;
		logF[const] := const;
		logF[ident] := logLink;
		logF[prod] := logLink;
		logF[linear] := logLink;
		logF[other] := other
	END LogRule;

	PROCEDURE LogitRule;
		VAR
			i: INTEGER;
	BEGIN
		i := 0;
		WHILE i < noClasses DO
			logitF[i] := differ;
			INC(i)
		END;
		logitF[const] := const;
		logitF[ident] := logitLink;
		logitF[prod] := logitLink;
		logitF[linear] := logitLink;
		logitF[other] := other
	END LogitRule;

	PROCEDURE ProbitRule;
		VAR
			i: INTEGER;
	BEGIN
		i := 0;
		WHILE i < noClasses DO
			probitF[i] := differ;
			INC(i)
		END;
		probitF[const] := const;
		probitF[ident] := probitLink;
		probitF[prod] := probitLink;
		probitF[linear] := probitLink;
		probitF[other] := other
	END ProbitRule;

	PROCEDURE CloglogRule;
		VAR
			i: INTEGER;
	BEGIN
		i := 0;
		WHILE i < noClasses DO
			cloglogF[i] := differ;
			INC(i)
		END;
		cloglogF[const] := const;
		cloglogF[ident] := cloglogLink;
		cloglogF[prod] := cloglogLink;
		cloglogF[linear] := cloglogLink;
		cloglogF[other] := other
	END CloglogRule;

	PROCEDURE NonDiff1Rule;
		VAR
			i: INTEGER;
	BEGIN
		i := 0;
		WHILE i < noClasses DO
			nonDiff1F[i] := other;
			INC(i)
		END;
		nonDiff1F[const] := const
	END NonDiff1Rule;

	PROCEDURE UOpRule;
		VAR
			i: INTEGER;
	BEGIN
		i := 0;
		WHILE i < noClasses DO
			uopF[i] := differ;
			INC(i)
		END;
		uopF[const] := const;
		uopF[other] := other
	END UOpRule;

	PROCEDURE AddRule;
		VAR
			i, j: INTEGER;
	BEGIN
		i := 0;
		WHILE i < noClasses DO
			j := 0;
			WHILE j < noClasses DO
				addF[i, j] := differ;
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < noClasses DO
			addF[other, i] := other;
			addF[i, other] := other;
			INC(i)
		END;
		addF[const, const] := const;
		addF[const, ident] := linear;
		addF[const, prod] := linear;
		addF[const, linear] := linear;
		addF[const, logitLink] := linkFun;
		addF[const, linkFun] := linkFun;
		addF[const, expon] := linExp;
		addF[const, linExp] := linExp;
		addF[ident, const] := linear;
		addF[ident, ident] := prod;
		addF[ident, prod] := prod;
		addF[ident, linear] := linear;
		addF[prod, const] := linear;
		addF[prod, ident] := prod;
		addF[prod, prod] := prod;
		addF[prod, linear] := linear;
		addF[linear, const] := linear;
		addF[linear, ident] := linear;
		addF[linear, prod] := linear;
		addF[linear, linear] := linear;
		addF[linkFun, const] := linkFun;
		addF[logitLink, const] := linkFun;
		addF[expon, const] := linExp;
		addF[expon, expon] := linExp;
		addF[expon, linExp] := linExp;
		addF[linExp, const] := linExp;
		addF[linExp, expon] := linExp;
		addF[linExp, linExp] := linExp
	END AddRule;

	PROCEDURE SubRule;
		VAR
			i, j: INTEGER;
	BEGIN
		i := 0;
		WHILE i < noClasses DO
			j := 0;
			WHILE j < noClasses DO
				subF[i, j] := differ;
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < noClasses DO
			subF[other, i] := other;
			subF[i, other] := other;
			INC(i)
		END;
		subF[const, const] := const;
		subF[const, ident] := linear;
		subF[const, prod] := linear;
		subF[const, linear] := linear;
		subF[const, logitLink] := linkFun;
		subF[const, linkFun] := linkFun;
		subF[ident, const] := linear;
		subF[ident, ident] := const;
		subF[ident, prod] := linear;
		subF[ident, linear] := linear;
		subF[prod, const] := linear;
		subF[prod, ident] := linear;
		subF[prod, prod] := linear;
		subF[prod, linear] := linear;
		subF[linear, const] := linear;
		subF[linear, ident] := linear;
		subF[linear, prod] := linear;
		subF[linear, linear] := linear;
		subF[logitLink, const] := linkFun;
		subF[logitLink, linkFun] := linkFun;
		subF[logitLink, logitLink] := linkFun;
		subF[linkFun, const] := linkFun;
		subF[linkFun, logitLink] := linkFun;
		subF[linkFun, linkFun] := linkFun
	END SubRule;

	PROCEDURE MultRule;
		VAR
			i, j: INTEGER;
	BEGIN
		i := 0;
		WHILE i < noClasses DO
			j := 0;
			WHILE j < noClasses DO
				multF[i, j] := differ;
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < noClasses DO
			multF[other, i] := other;
			multF[i, other] := other;
			INC(i)
		END;
		multF[const, const] := const;
		multF[const, ident] := prod;
		multF[const, prod] := prod;
		multF[const, linear] := linear;
		multF[const, expon] := expon;
		multF[ident, const] := prod;
		multF[prod, const] := prod;
		multF[linear, const] := linear;
		multF[expon, const] := expon;
		multF[expon, expon] := expon
	END MultRule;

	PROCEDURE DivRule;
		VAR
			i, j: INTEGER;
	BEGIN
		i := 0;
		WHILE i < noClasses DO
			j := 0;
			WHILE j < noClasses DO
				divF[i, j] := differ;
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < noClasses DO
			divF[other, i] := other;
			divF[i, other] := other;
			INC(i)
		END;
		divF[const, const] := const;
		divF[const, linExp] := linkFun;
		divF[ident, const] := prod;
		divF[prod, const] := prod;
		divF[linear, const] := linear;
		divF[expon, expon] := expon;
		divF[expon, linExp] := linkFun
	END DivRule;

	PROCEDURE PowRule;
		VAR
			i, j: INTEGER;
	BEGIN
		i := 0;
		WHILE i < noClasses DO
			j := 0;
			WHILE j < noClasses DO
				powF[i, j] := differ;
				INC(j)
			END;
			INC(i)
		END;
		i := 0;
		WHILE i < noClasses DO
			powF[other, i] := other;
			powF[i, other] := other;
			INC(i)
		END;
		powF[const, const] := const;
		powF[const, ident] := expon;
		powF[const, linear] := expon;
		powF[const, prod] := expon;
		powF[const, linear] := expon;
		powF[ident, const] := power;
		powF[prod, const] := power;
		powF[expon, const] := expon;
		powF[power, const] := power
	END PowRule;

	PROCEDURE OrRule;
		VAR
			i, j: INTEGER;
	BEGIN
		i := 0;
		WHILE i < noClasses DO
			j := 0;
			WHILE j < noClasses DO
				orF[i, j] := other;
				INC(j)
			END;
			INC(i)
		END;
		orF[const, const] := const;
		orF[const, ident] := ident;
		orF[const, prod] := prod;
		orF[const, linear] := linear;
		orF[const, logitLink] := linkFun;
		orF[const, linkFun] := linkFun;
		orF[const, expon] := expon;
		orF[ident, const] := ident;
		orF[ident, ident] := ident;
		orF[ident, prod] := prod;
		orF[ident, linear] := linear;
		orF[prod, const] := prod;
		orF[prod, ident] := prod;
		orF[prod, prod] := prod;
		orF[prod, linear] := linear;
		orF[linear, const] := linear;
		orF[linear, ident] := linear;
		orF[linear, prod] := linear;
		orF[linear, linear] := linear;
		orF[logitLink, const] := linkFun;
		orF[logitLink, logitLink] := linkFun;
		orF[logitLink, linkFun] := linkFun;
		orF[linkFun, const] := linkFun;
		orF[linkFun, logitLink] := linkFun;
		orF[linkFun, linkFun] := linkFun;
		orF[expon, const] := expon
	END OrRule;

	PROCEDURE NonDiff2Rule;
		VAR
			i, j: INTEGER;
	BEGIN
		i := 0;
		WHILE i < noClasses DO
			j := 0;
			WHILE j < noClasses DO
				nonDiff2F[i, j] := other;
				INC(j)
			END;
			INC(i)
		END;
		nonDiff2F[const, const] := const
	END NonDiff2Rule;

	PROCEDURE ProductRule;
		VAR
			i, j: INTEGER;
	BEGIN
		i := 0;
		WHILE i < noDensity DO
			j := 0;
			WHILE j < noDensity DO
				product[i, j] := genDiff;
				INC(j)
			END;
			INC(i)
		END;

		i := 0;
		WHILE i < noDensity DO
			product[multinomial, i] := multinomial;
			product[i, multinomial] := multinomial;
			INC(i)
		END;

		i := 0;
		WHILE i < noDensity DO
			product[general, i] := general;
			product[i, general] := general;
			INC(i)
		END;

		product[gamma, gamma] := gamma;
		product[gamma, gamma1] := gamma1;
		product[gamma, unif] := gamma;
		product[gamma, pareto] := gamma;

		product[gamma1, gamma] := gamma1;
		product[gamma1, gamma1] := gamma1;
		product[gamma1, logCon] := logCon;
		product[gamma1, unif] := gamma1;
		product[gamma1, cloglogReg] := logCon;
		product[gamma1, logitReg] := logCon;
		product[gamma1, probitReg] := logCon;
		product[gamma1, logReg] := logCon;
		product[gamma1, pareto] := gamma;
		product[gamma1, normal] := logCon;

		product[beta, beta] := beta;
		product[beta, beta1] := beta1;
		product[beta, unif] := beta;

		product[beta1, beta] := beta1;
		product[beta1, beta1] := beta1;
		product[beta1, unif] := beta1;
		product[beta1, logCon] := logCon;
		product[beta1, logCon] := logCon;
		product[beta1, logitReg] := logCon;
		product[beta1, logReg] := logCon;

		product[pareto, pareto] := pareto;
		product[pareto, unif] := pareto;
		product[pareto, gamma] := gamma;
		product[pareto, gamma1] := gamma;

		product[normal, normal] := normal;
		product[normal, mVN] := normal;
		product[normal, logCon] := logCon;
		product[normal, unif] := normal;
		product[normal, cloglogReg] := cloglogReg;
		product[normal, logitReg] := logitReg;
		product[normal, probitReg] := probitReg;
		product[normal, logReg] := logReg;
		product[normal, mVNLin] := normal;
		product[normal, gamma1] := logCon;

		product[mVN, normal] := normal;
		product[mVN, mVN] := mVN;
		product[mVN, unif] := mVN;
		product[mVN, logCon] := logCon;
		product[mVN, cloglogReg] := cloglogReg;
		product[mVN, logitReg] := logitReg;
		product[mVN, probitReg] := probitReg;
		product[mVN, logReg] := logReg;
		product[mVN, mVNLin] := mVNLin;

		product[mVNLin, normal] := normal;
		product[mVNLin, mVNLin] := mVNLin;
		product[mVNLin, mVN] := mVNLin;
		product[mVNLin, unif] := mVNLin;
		product[mVNLin, cloglogReg] := cloglogReg;
		product[mVNLin, logitReg] := logitReg;
		product[mVNLin, probitReg] := probitReg;
		product[mVNLin, logReg] := logReg;

		i := 0;
		WHILE i < noDensity DO
			product[wishart, i] := invalid;
			product[i, wishart] := invalid;
			INC(i)
		END;
		product[wishart, wishart] := wishart;
		product[wishart, unif] := wishart;
		product[unif, wishart] := wishart;

		i := 0;
		WHILE i < noDensity DO
			product[dirichlet, i] := dirichletPrior;
			product[i, dirichlet] := dirichletPrior;
			INC(i)
		END;
		product[dirichlet, dirichlet] := dirichlet;
		product[dirichlet, unif] := dirichlet;
		product[unif, dirichlet] := dirichlet;


		product[logCon, gamma1] := logCon;
		product[logCon, normal] := logCon;
		product[logCon, mVN] := logCon;
		product[logCon, logCon] := logCon;
		product[logCon, unif] := logCon;
		product[logCon, cloglogReg] := logCon;
		product[logCon, logitReg] := logCon;
		product[logCon, probitReg] := logCon;
		product[logCon, logReg] := logCon;

		product[unif, gamma] := gamma;
		product[unif, gamma1] := gamma1;
		product[unif, beta] := beta;
		product[unif, beta1] := beta1;
		product[unif, pareto] := pareto;
		product[unif, normal] := normal;
		product[unif, mVN] := mVN;
		product[unif, wishart] := wishart;
		product[unif, dirichlet] := dirichlet;
		product[unif, logCon] := logCon;
		product[unif, unif] := unif;
		product[unif, cloglogReg] := cloglogReg;
		product[unif, logitReg] := logitReg;
		product[unif, probitReg] := probitReg;
		product[unif, logReg] := logReg;
		product[unif, mVNLin] := mVNLin;

		product[cloglogReg, normal] := cloglogReg;
		product[cloglogReg, unif] := cloglogReg;
		product[cloglogReg, cloglogReg] := cloglogReg;
		product[cloglogReg, logitReg] := logCon;
		product[cloglogReg, probitReg] := logCon;
		product[cloglogReg, logCon] := logCon;
		product[cloglogReg, logReg] := logCon;
		product[cloglogReg, mVN] := cloglogReg;
		product[cloglogReg, mVNLin] := cloglogReg;

		product[logitReg, normal] := logitReg;
		product[logitReg, unif] := logitReg;
		product[logitReg, cloglogReg] := logCon;
		product[logitReg, logitReg] := logitReg;
		product[logitReg, probitReg] := logCon;
		product[logitReg, logCon] := logCon;
		product[logitReg, logReg] := logCon;
		product[logitReg, mVN] := logitReg;
		product[logitReg, mVNLin] := logitReg;

		product[probitReg, normal] := probitReg;
		product[probitReg, unif] := probitReg;
		product[probitReg, cloglogReg] := logCon;
		product[probitReg, logitReg] := logCon;
		product[probitReg, probitReg] := probitReg;
		product[probitReg, logCon] := logCon;
		product[probitReg, logReg] := logCon;
		product[probitReg, mVN] := probitReg;
		product[probitReg, mVNLin] := probitReg;

		product[logReg, normal] := logReg;
		product[logReg, unif] := logReg;
		product[logReg, logReg] := logReg;
		product[logReg, logCon] := logCon;
		product[logReg, cloglogReg] := logCon;
		product[logReg, logitReg] := logCon;
		product[logReg, probitReg] := logCon;
		product[logReg, mVN] := logCon;
		product[logReg, mVNLin] := logReg;

		i := 0;
		WHILE i < noDensity DO
			product[catagorical, i] := catagorical;
			product[i, catagorical] := catagorical;
			product[descrete, i] := descrete;
			product[i, descrete] := descrete;
			product[poisson, i] := descrete;
			product[i, poisson] := descrete;
			INC(i)
		END;
		product[poisson, poisson] := poisson;

		i := 0;
		WHILE i < noDensity DO
			product[invalid, i] := invalid;
			product[i, invalid] := invalid;
			INC(i)
		END

	END ProductRule;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		UminusRule;
		ExpRule;
		LinkRule;
		LogRule;
		LogitRule;
		ProbitRule;
		CloglogRule;
		UOpRule;
		NonDiff1Rule;
		AddRule;
		SubRule;
		MultRule;
		DivRule;
		OrRule;
		NonDiff2Rule;
		PowRule;
		ProductRule
	END Init;

BEGIN
	Init
END GraphRules.

