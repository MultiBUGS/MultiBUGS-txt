(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	   *)

MODULE CorrelCmds;

	

	IMPORT
		Dialog, 
		TextMappers, TextModels, 
		BugsCmds, BugsDialog, BugsFiles, BugsInterface, BugsMsg,
		CorrelFormatted, CorrelPlots,
		SamplesIndex;

	TYPE
		DialogBox* = POINTER TO RECORD(BugsDialog.DialogBox)
			beg*, end*, firstChain*, lastChain*: INTEGER;
			string0*, string1*: Dialog.String
		END;

	VAR
		dialog*: DialogBox;

		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE SetVariable0* (node: ARRAY OF CHAR);
	BEGIN
		dialog.string0 := node$
	END SetVariable0;

	PROCEDURE SetVariable1* (node: ARRAY OF CHAR);
	BEGIN
		dialog.string1 := node$
	END SetVariable1;

	PROCEDURE InitDialog (dialog: DialogBox);
	BEGIN
		dialog.beg := 1;
		dialog.end := 1000000;
		dialog.firstChain := 1;
		dialog.lastChain := BugsCmds.specificationDialog.numChains;
		dialog.string0 := "";
		dialog.string1 := ""
	END InitDialog;

	PROCEDURE (dialog: DialogBox) Init-;
	BEGIN
		InitDialog(dialog)
	END Init;

	PROCEDURE (dialog: DialogBox) Update-;
	BEGIN
		Dialog.Update(dialog)
	END Update;

	PROCEDURE DrawMatrix*;
		VAR
			beg, end, thin, firstChain, lastChain: INTEGER;
			string0, string1: Dialog.String;
			f: TextMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		beg := dialog.beg;
		end := dialog.end;
		thin := 1;
		string0 := dialog.string0$;
		string1 := dialog.string1$;
		firstChain := dialog.firstChain;
		lastChain := MIN(dialog.lastChain, BugsCmds.specificationDialog.numChains);
		CorrelPlots.MatrixPlot(string0, string1, beg, end, thin, firstChain, lastChain, f);
		BugsFiles.Open("Corellation Matrix", text)
	END DrawMatrix;

	PROCEDURE PrintMatrix*;
		VAR
			beg, end, thin, firstChain, lastChain: INTEGER;
			string0, string1: Dialog.String;
			f: TextMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		beg := dialog.beg;
		end := dialog.end;
		thin := 1;
		string0 := dialog.string0$;
		string1 := dialog.string1$;
		firstChain := dialog.firstChain;
		lastChain := MIN(dialog.lastChain, BugsCmds.specificationDialog.numChains);
		CorrelFormatted.PrintMatrix(string0, string1, beg, end, thin, firstChain, lastChain, f);
		BugsFiles.Open("Corellation Matrix", text)
	END PrintMatrix;

	PROCEDURE ScatterPlots*;
		VAR
			beg, end, thin, firstChain, lastChain: INTEGER;
			string0, string1: Dialog.String;
			f: TextMappers.Formatter;
			text: TextModels.Model;
	BEGIN
		text := TextModels.dir.New();
		f.ConnectTo(text);
		f.SetPos(0);
		beg := dialog.beg;
		end := dialog.end;
		thin := 1;
		string0 := dialog.string0$;
		string1 := dialog.string1$;
		firstChain := dialog.firstChain;
		lastChain := MIN(dialog.lastChain, BugsCmds.specificationDialog.numChains);
		CorrelPlots.ScatterPlots(string0, string1, beg, end, thin, firstChain, lastChain, f);
		BugsFiles.Open("Bivariate Scatter Plot", text)
	END ScatterPlots;

	PROCEDURE Guard* (VAR par: Dialog.Par);
		VAR
			i: INTEGER;
			string: Dialog.String;
			p: ARRAY 1 OF Dialog.String;
	BEGIN
		par.disabled := FALSE;
		IF ~BugsInterface.IsInitialized() THEN
			par.disabled := TRUE;
			IF BugsCmds.script THEN BugsMsg.Lookup("CorrelCmds:NotInitialized", par.label) END
		ELSE
			IF BugsInterface.IsAdapting() THEN
				par.disabled := TRUE;
				IF BugsCmds.script THEN BugsMsg.Lookup("CorrelCmds:Adapting", par.label) END
			ELSE
				i := 0;
				WHILE (dialog.string0[i] # 0X) & (dialog.string0[i] # "[") DO
					string[i] := dialog.string0[i];
					INC(i)
				END;
				string[i] := 0X;
				IF SamplesIndex.Find(string) = NIL THEN
					par.disabled := TRUE;
					IF BugsCmds.script THEN
						p[0] := string;
						BugsMsg.LookupParam("CorrelCmds:NotMonitored", p, par.label);
					END
				ELSE
					IF dialog.string1 # "" THEN
						i := 0;
						WHILE (dialog.string1[i] # 0X) & (dialog.string1[i] # "[") DO
							string[i] := dialog.string1[i];
							INC(i)
						END;
						string[i] := 0X;
						IF SamplesIndex.Find(string) = NIL THEN
							par.disabled := TRUE;
							IF BugsCmds.script THEN
								p[0] := string;
								BugsMsg.LookupParam("CorrelCmds:NotMonitored", p, par.label);
							END
						END
					END
				END
			END
		END
	END Guard;

	PROCEDURE FirstNotifier* (op, from, to: INTEGER);
	BEGIN
		dialog.firstChain := MAX(1, dialog.firstChain);
		dialog.firstChain := MIN(dialog.firstChain, BugsCmds.specificationDialog.numChains)
	END FirstNotifier;

	PROCEDURE LastNotifier* (op, from, to: INTEGER);
	BEGIN
		dialog.lastChain := MAX(1, dialog.lastChain);
		dialog.lastChain := MIN(dialog.lastChain, BugsCmds.specificationDialog.numChains)
	END LastNotifier;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		NEW(dialog);
		InitDialog(dialog);
		BugsDialog.AddDialog(dialog)
	END Init;

BEGIN
	Init
END CorrelCmds.

