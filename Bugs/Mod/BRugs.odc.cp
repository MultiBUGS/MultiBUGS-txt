(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	*)

(*	

	Interface module for OpenBUGS dynamic link library, allows the library to be used from a Component Pascal
	program

*)

MODULE BugsBRugs["LibOpenBUGS"];

	IMPORT 
		SYSTEM;
		
	PROCEDURE [ccall] BugsCmd* (VAR command: POINTER TO ARRAY [untagged] OF SHORTCHAR; 
	VAR len: INTEGER);
	
	PROCEDURE [ccall] CLI*;
	
	PROCEDURE [ccall] CharArray* (VAR procedure: POINTER TO ARRAY [untagged] OF SHORTCHAR; 
	VAR len: INTEGER; VAR x: POINTER TO ARRAY [untagged] OF SHORTCHAR; VAR lenX, res: INTEGER);
	
	PROCEDURE [ccall] CmdInterpreter* (VAR command: POINTER TO ARRAY [untagged] OF SHORTCHAR;
	VAR len, res: INTEGER);
	
	PROCEDURE [ccall] Guard* (VAR procedure: POINTER TO ARRAY [untagged] OF SHORTCHAR; 
	VAR len, x, res: INTEGER);
	
	PROCEDURE [ccall] Integer* (VAR procedure: POINTER TO ARRAY [untagged] OF SHORTCHAR; 
	VAR len, x, res: INTEGER);
	
	PROCEDURE [ccall] IntegerArray* (VAR procedure: POINTER TO ARRAY [untagged] OF SHORTCHAR; 
	VAR len: INTEGER; VAR x: ARRAY [untagged] OF INTEGER; VAR lenX, res: INTEGER);
	
	PROCEDURE [ccall] Real* (VAR procedure: POINTER TO ARRAY [untagged] OF SHORTCHAR; 
	VAR len: INTEGER; VAR x, y: REAL; VAR res: INTEGER);
	
	PROCEDURE [ccall] RealArray* (VAR procedure: POINTER TO ARRAY [untagged] OF SHORTCHAR; 
	VAR len: INTEGER; VAR x: ARRAY [untagged] OF REAL; VAR lenX, res: INTEGER);
	
	PROCEDURE [ccall] SetWorkingDir* (VAR path: POINTER TO ARRAY [untagged] OF SHORTCHAR; 
	VAR len: INTEGER);
	
	PROCEDURE [ccall] SetTempDir* (VAR path: POINTER TO ARRAY [untagged] OF SHORTCHAR; 
	VAR len: INTEGER);
	
	PROCEDURE [ccall] UseBufferFile*;
	
	PROCEDURE [ccall] UseConsole*;

END BugsBRugs.

