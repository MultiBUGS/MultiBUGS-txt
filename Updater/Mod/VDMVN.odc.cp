(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"

*)

(*

The first component of the updater is the number of features selected (covariates or knot points say...),
the priors of beta come next, then a block of catagoricals indicating selected or not selected,
followed by the final set of parameters that are  hyper priors of the beta.

*)

MODULE UpdaterVDMVN;

	

	IMPORT
		Math, Stores, 
		GraphConjugateUV, GraphConstant, GraphNodes, GraphRules, GraphStochastic, GraphVD,
		MathMatrix, MathRandnum,
		UpdaterUpdaters, UpdaterVD;

	TYPE
		Updater* = POINTER TO ABSTRACT RECORD(UpdaterVD.Updater)
			predictor: GraphNodes.Vector;
			(*	working storage for MVN parameters	*)
			c, m, mu, t, tauMu, value: POINTER TO ARRAY OF REAL;
			tau, z: POINTER TO ARRAY OF ARRAY OF REAL;
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE (updater: Updater) CalculateBetaParams- (dim: INTEGER);
		VAR
			i, j, l, numLikelihood: INTEGER;
			betaMu, betaTau: REAL;
			c, m, mu, t, tauMu: POINTER TO ARRAY OF REAL;
			tau, z: POINTER TO ARRAY OF ARRAY OF REAL;
			child: GraphConjugateUV.Node;
			predictor: GraphNodes.Vector;
			children: GraphStochastic.Vector;
		CONST
			as = GraphRules.normal;
	BEGIN
		mu := updater.mu;
		tauMu := updater.tauMu;
		tau := updater.tau;
		predictor := updater.predictor;
		c := updater.c;
		m := updater.m;
		t := updater.t;
		z := updater.z;
		numLikelihood := LEN(updater.predictor);
		(*	prior contributions	*)
		betaMu := 0.0;
		betaTau := updater.vdNode.p1.Value();
		i := 0;
		WHILE i < dim DO
			tauMu[i] := betaMu;
			j := 0;
			WHILE j < dim DO
				tau[i, j] := 0.0;
				INC(j)
			END;
			tau[i, i] := betaTau;
			INC(i)
		END;
		i := 0;
		WHILE i < dim DO
			updater.prior[1 + i].SetValue(0.0);
			INC(i)
		END;
		l := 0;
		children := updater.Children();
		WHILE l < numLikelihood DO
			child := children[l](GraphConjugateUV.Node);
			child.LikelihoodForm(as, predictor[l], m[l], t[l]);
			c[l] := predictor[l].Value();
			INC(l)
		END;
		i := 0;
		WHILE i < dim DO
			updater.prior[1 + i].SetValue(1.0);
			l := 0;
			WHILE l < numLikelihood DO
				z[i, l] := predictor[l].Value() - c[l];
				INC(l);
			END;
			updater.prior[1 + i].SetValue(0.0);
			INC(i)
		END;
		l := 0;
		WHILE l < numLikelihood DO
			i := 0;
			WHILE i < dim DO
				tauMu[i] := tauMu[i] + (m[l] - c[l]) * t[l] * z[i, l];
				j := 0;
				WHILE j < dim DO
					tau[i, j] := tau[i, j] + z[i, l] * z[j, l] * t[l];
					INC(j)
				END;
				INC(i)
			END;
			INC(l)
		END;
		i := 0; WHILE i < dim DO mu[i] := tauMu[i]; INC(i) END;
		MathMatrix.Cholesky(tau, dim);
		MathMatrix.ForwardSub(tau, mu, dim);
		MathMatrix.BackSub(tau, mu, dim)
	END CalculateBetaParams;

	PROCEDURE (updater: Updater) SampleBeta- (dim: INTEGER);
		VAR
			i: INTEGER;
	BEGIN
		MathRandnum.MNormal(updater.tau, updater.mu, dim, updater.value);
		i := 0;
		WHILE i < dim DO
			updater.prior[i + 1].SetValue(updater.value[i]);
			INC(i)
		END;
	END SampleBeta;

	
	PROCEDURE (updater: Updater) CopyFromVDMVN- (source: UpdaterUpdaters.Updater), NEW, ABSTRACT;

	PROCEDURE (updater: Updater) CopyFromVD- (source: UpdaterUpdaters.Updater);
		VAR
			s: Updater;
			size: INTEGER;
	BEGIN
		size := source.Size();
		s := source(Updater);
		updater.predictor := s.predictor;
		updater.c := s.c;
		updater.m := s.m;
		updater.mu := s.mu;
		updater.t := s.t;
		updater.tauMu := s.tauMu;
		updater.value := s.value;
		updater.tau := s.tau;
		updater.z := s.z;
		updater.CopyFromVDMVN(source)
	END CopyFromVD;
	
	PROCEDURE (updater: Updater) InitializeVDMVN-, NEW, ABSTRACT;

	PROCEDURE (updater: Updater) InitializeVD-;
		VAR
			i, class, numLike, numPhi, maxNumBeta, size: INTEGER;
			children: GraphStochastic.Vector;
	BEGIN
		maxNumBeta := LEN(updater.vdNode.beta);
		children := updater.Children();
		numLike := LEN(children);
		NEW(updater.predictor, numLike);
		i := 0; WHILE i < numLike DO updater.predictor[i] := NIL; INC(i) END;
		NEW(updater.value, maxNumBeta);
		NEW(updater.c, numLike);
		NEW(updater.m, numLike);
		NEW(updater.t, numLike);
		NEW(updater.z, maxNumBeta, numLike);
		NEW(updater.mu, maxNumBeta);
		NEW(updater.tauMu, maxNumBeta);
		NEW(updater.tau, maxNumBeta, maxNumBeta);
		updater.InitializeVDMVN
	END InitializeVD;

	PROCEDURE (updater: Updater) Integral- (dim: INTEGER): REAL;
		VAR
			i: INTEGER;
			betaTau, integral: REAL;
	BEGIN
		betaTau := updater.vdNode.p1.Value();	
		integral := 0.5 * dim * Math.Ln(betaTau) (*- 0.5 * dim * Math.Ln(2 * Math.Pi())*);
		i := 0;
		WHILE i < dim DO
			integral := integral - Math.Ln(updater.tau[i, i]);
			INC(i)
		END;
		i := 0;
		WHILE i < dim DO
			integral := integral + 0.5 * updater.mu[i] * updater.tauMu[i];
			INC(i)
		END;
		RETURN integral
	END Integral;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

BEGIN
	Maintainer
END UpdaterVDMVN.
