(*		GNU General Public Licence  *)

(*
license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"
*)

MODULE MathTaucsLib["libtaucs"];

	

	IMPORT SYSTEM;

	CONST
		lower* = 0;
		symmetric* = 3;
		double* = 11;

	TYPE
		Matrix* = POINTER TO RECORD[untagged]
			n*: INTEGER;
			m*: INTEGER;
			flags*: SET;
			colptr*: POINTER TO ARRAY[untagged]OF INTEGER;
			rowind*: POINTER TO ARRAY[untagged]OF INTEGER;
			values*: POINTER TO ARRAY[untagged]OF REAL;
		END;

	PROCEDURE[ccall] Create*["taucs_ccs_create"] (m, n, nnz: INTEGER; flags: SET): Matrix;

		PROCEDURE[ccall] FactorLLT*["taucs_ccs_factor_llt"] (A: Matrix; droptol: REAL;
		modified: INTEGER): Matrix;

			PROCEDURE[ccall] Free*["taucs_ccs_free"] (A: Matrix);

				PROCEDURE[ccall] SupernodalFactorLLT*["taucs_ccs_factor_llt_mf"] (A: Matrix): ANYPTR;

					PROCEDURE[ccall] SupernodalToCCS*["taucs_supernodal_factor_to_ccs"] (A: ANYPTR): Matrix;

						PROCEDURE[ccall] FreeSupernodalFactor*["taucs_supernodal_factor_free"] (A: ANYPTR);

							PROCEDURE[ccall] MatrixTimesVector*["taucs_ccs_times_vec"] (A: Matrix; x, b: POINTER TO ARRAY[untagged]OF REAL);

							END MathTaucsLib.

