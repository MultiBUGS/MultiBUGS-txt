
(*
   Template module as described in the OpenBUGS differential equation solver
   document. For a detailed description of what to do with this file, see the 
   section entitled "Create your own module." 

   WARNING: Various sections of this module are not to be altered by the 
   user. If you do alter any of those sections, the result will probably 
   not work.
 
   Module name is 'DiffTemplateModule'. 
*) 

(*
	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"
*)

MODULE DiffTemplateModule;

	


  (* 
    List of modules required by this one. 
    Use must not delete or change these (except to 
    uncomment 'Out' if required.)
  *) 
	IMPORT
		GraphNodes, 
		GraphODEmath, 
		GraphVector,
		MathODE, 
		Math,
		Out,
		MathRungeKutta45;

  (*  
    Definitions to interface with the rest of OpenBUGS. 
    Use must not delete or change these. 
  *) 
	TYPE
		Equations = POINTER TO RECORD (MathODE.Equations) END;

		Factory = POINTER TO RECORD (GraphVector.Factory)
			solverFact: MathODE.Factory
		END;


  (* 
     nEq is the number of differential equations in the system. 
     Should be greater than or equal to unity. 
  *) 
	CONST
		nEq = 0;


  (* 
    Variables used by this module. User must not delete any of these, 
    although variables may be added if necessary. 
  *) 
	VAR
		fact: Factory;
		equations: Equations;
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;



  (* 
    This procedure (function) computes the right hand sides of the 
    system of ODEs. The parameters are stored in the array 'parameters';
    the array of values of the solution is 'C'; the number of 
    equations is 'n'; the value of time is 't'; and the output 
    array of derivatives is 'dCdt'. 
  *) 
	PROCEDURE (e: Equations) Derivatives (IN parameters, C: ARRAY OF REAL; 
	                                      n: INTEGER; 
	                                      t: REAL;
																	    	OUT dCdt: ARRAY OF REAL);
	VAR
	BEGIN
	
	END Derivatives;



  (* 
    A placeholder for possible future work. Currently not used by OpenBUGS. 
  *) 
	PROCEDURE (e: Equations) SecondDerivatives (IN theta, x: ARRAY OF REAL; 
	                                            numEq: INTEGER; 
	                                            t: REAL;
																				      OUT d2xdt2: ARRAY OF REAL);
	BEGIN
		HALT(126)
	END SecondDerivatives;


  (* 
    A placeholder for possible future work. Currently not used by OpenBUGS. 
  *) 
	PROCEDURE (e: Equations) Jacobian (IN theta, x: ARRAY OF REAL; 
	                                   numEq: INTEGER; 
	                                   t: REAL;
																	   OUT jacob: ARRAY OF ARRAY OF REAL);
	BEGIN
		HALT(126)
	END Jacobian;


  (* 
    Procedure (function) to create factory object for this module. 
    User must not alter this. 
  *) 
	PROCEDURE (f: Factory) New (option: INTEGER): GraphVector.Node;
		VAR
			node: GraphNodes.Node;
	BEGIN
		NEW(equations);
		node := GraphODEmath.New(f.solverFact, equations, nEq); 
		RETURN node(GraphVector.Node)
	END New;


  (* 
    User must not alter this. 
  *) 
	PROCEDURE (f: Factory) Signature (OUT signature: ARRAY OF CHAR);
	BEGIN
		signature := "vvvsv"
	END Signature;


  (* 
    User must not alter this. 
  *) 
	PROCEDURE Install*;
	BEGIN
		GraphNodes.SetFactory(fact)
	END Install;


  (* 
    Version and maintainer of the current module. User may alter this. 
  *) 
	PROCEDURE Maintainer;
	BEGIN
		version := 307;
		maintainer := "S. Miller"
	END Maintainer;


  (* 
    Procedure to follow at startup. User must not alter this. 
  *) 
	PROCEDURE Init;
	BEGIN
		Maintainer;
		NEW(fact);
		fact.solverFact := MathRungeKutta45.fact
	END Init;


(* 
  End of module. 
*) 
  BEGIN
	  Init
END DiffTemplateModule.
