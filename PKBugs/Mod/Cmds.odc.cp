(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE PKBugsCmds;

		(* PKBugs Version 1.1 *)

	IMPORT
		Controllers, Dialog, Models, PKBugsCovts,
		PKBugsData, PKBugsNames,
		PKBugsNodes, PKBugsParse, PKBugsPriors, PKBugsScanners, PKBugsTree, Strings, TextControllers,
		TextMappers, TextModels,
		BugsCmds, BugsDialog, BugsFiles, BugsGraph, BugsInterface, BugsMappers, BugsMsg, 
		MathRandnum;

	CONST
		namesLoaded* = 1; dataLoaded* = 2; modelCompiled* = 3;
		oneComp = 1; twoComp = 2; threeComp = 3;
		notLog = 0; log = 1;
		defaultNumChains = 2;
		dummy = 31;

	TYPE

		SpecificationDialog* = POINTER TO RECORD(BugsDialog.DialogBox)
			nComp*, residuals*: INTEGER;
			parameters*: Dialog.List;
			covariates*: Dialog.Selection;
			mean*, cv*: REAL;
			numChains*: INTEGER
		END;

		Reader = POINTER TO RECORD(PKBugsScanners.Reader)
			textRd: TextModels.Reader
		END;

	VAR
		specificationDialog*: SpecificationDialog;

		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

		status-: SET;
		disp, absList: Dialog.List;


	PROCEDURE SetStatus* (s: SET);
	BEGIN
		status := s
	END SetStatus;

	PROCEDURE InitDialog (dialogBox: SpecificationDialog);
	BEGIN
		dialogBox.nComp := oneComp;
		dialogBox.residuals := notLog;
		dialogBox.parameters.SetLen(0);
		dialogBox.covariates.SetLen(0);
		dialogBox.mean := PKBugsPriors.defaultMean;
		dialogBox.cv := PKBugsPriors.defaultCV;
		dialogBox.numChains := defaultNumChains;
		Dialog.Update(specificationDialog);
	END InitDialog;

	PROCEDURE (dialogBox: SpecificationDialog) Init-;
	BEGIN
		InitDialog(dialogBox)
	END Init;

	PROCEDURE (dialogBox: SpecificationDialog) Update-;
	BEGIN
	END Update;

	PROCEDURE (rd: Reader) ReadChar (OUT ch: CHAR);
	BEGIN
		IF ~rd.eot THEN
			rd.textRd.ReadChar(ch);
			rd.eot := rd.textRd.eot
		ELSE
			ch := 0X
		END
	END ReadChar;

	PROCEDURE (rd: Reader) Pos (): INTEGER;
	BEGIN
		RETURN rd.textRd.Pos()
	END Pos;

	PROCEDURE (rd: Reader) SetPos (pos: INTEGER);
	BEGIN
		ASSERT(pos >= 0, 21);
		rd.textRd.SetPos(pos);
		rd.eot := rd.textRd.eot
	END SetPos;

	PROCEDURE Reset;
	BEGIN
		status := {};
		PKBugsNames.Reset; PKBugsData.Reset; PKBugsParse.Reset;
		PKBugsNodes.Reset; PKBugsPriors.Reset; PKBugsCovts.Reset;
		BugsCmds.Clear
	END Reset;

	PROCEDURE ConnectScanner (VAR s: PKBugsScanners.Scanner; text: TextModels.Model);
		VAR
			rd: Reader;
	BEGIN
		NEW(rd);
		rd.textRd := text.NewReader(NIL);
		s.SetReader(rd)
	END ConnectScanner;

	PROCEDURE Error* (m: TextModels.Model; pos, res: INTEGER; string: ARRAY OF CHAR);
		VAR
			s: PKBugsScanners.Scanner;
			ch: CHAR;
			error, mes: ARRAY 256 OF CHAR;
			p: ARRAY 1 OF ARRAY 256 OF CHAR;
			msg: ARRAY 256 OF CHAR;
	BEGIN
		Strings.IntToString(res, error); error := "PKBugs:" + error;
		p[0] := string$;
		BugsMsg.LookupParam(error, p, msg);
		BugsFiles.ShowStatus(msg);
		IF m # NIL THEN
			ConnectScanner(s, m);
			s.SetPos(pos); ch := s.nextCh; s.FindToken(ch); TextControllers.SetCaret(m, s.Pos())
		END
	END Error;

	PROCEDURE BuildLists;
		VAR
			nBio, nDisp, nAbs, len, i, off, nCov, pos: INTEGER;
			s0, s1: ARRAY 80 OF CHAR;
			item: Dialog.String;
			bio, abs: POINTER TO ARRAY OF ARRAY OF CHAR;
	BEGIN
		IF PKBugsCovts.covariateNames # NIL THEN
			nCov := LEN(PKBugsCovts.covariateNames)
		ELSE
			nCov := 0
		END;
		specificationDialog.covariates.SetLen(0); specificationDialog.covariates.SetLen(nCov);
		off := 0;
		WHILE off # nCov DO
			specificationDialog.covariates.SetItem(off, PKBugsCovts.covariateNames[off]); INC(off)
		END;
		PKBugsParse.GetParLabels(bio, abs);
		s0 := "#PKBugs:disp"; Strings.IntToString(specificationDialog.nComp, s1); s0 := s0 + s1;
		disp.SetLen(0); disp.SetResources(s0);
		IF bio = NIL THEN nBio := 0 ELSE nBio := LEN(bio, 0) END;
		IF abs = NIL THEN nAbs := 0 ELSE nAbs := LEN(abs, 0) END;
		nDisp := 2 * specificationDialog.nComp; len := nBio + nDisp + nAbs;
		specificationDialog.parameters.SetLen(0); specificationDialog.parameters.SetLen(len);
		i := 0; WHILE i < nBio DO specificationDialog.parameters.SetItem(i, bio[i]); INC(i) END;
		i := 0;
		WHILE i < nDisp DO
			disp.GetItem(i, item); IF item = "" THEN item := "-- no label --" END;
			specificationDialog.parameters.SetItem(i + nBio, item);
			INC(i)
		END;
		IF nAbs > 0 THEN
			absList.SetLen(0);
			Strings.Find(abs[0], "ZOlag", 0, pos);
			IF pos # - 1 THEN
				absList.SetResources("#PKBugs:abs5")
			ELSE
				Strings.Find(abs[0], "FOlag", 0, pos);
				IF pos # - 1 THEN
					absList.SetResources("#PKBugs:abs4")
				ELSE
					Strings.Find(abs[0], "ZO", 0, pos);
					IF pos # - 1 THEN
						absList.SetResources("#PKBugs:abs3")
					ELSE
						Strings.Find(abs[0], "FO", 0, pos);
						IF pos # - 1 THEN
							absList.SetResources("#PKBugs:abs2")
						END
					END
				END
			END
		END;
		i := 0;
		WHILE i < nAbs DO
			absList.GetItem(i, item); IF item = "" THEN item := "-- no label --" END;
			specificationDialog.parameters.SetItem(i + nBio + nDisp, item);
			INC(i)
		END;
		PKBugsPriors.InitPriors(len); PKBugsCovts.InitCovts(len);
		Dialog.UpdateList(specificationDialog)
	END BuildLists;

	(* Guards *)

	PROCEDURE DataGuard* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~(namesLoaded IN status) OR (dataLoaded IN status)
	END DataGuard;

	PROCEDURE CompileGuard* (VAR par: Dialog.Par);
	BEGIN
		par.disabled := ~(dataLoaded IN status)
	END CompileGuard;

	PROCEDURE CompileGuardRO* (VAR par: Dialog.Par);
	BEGIN
		par.readOnly := ~(dataLoaded IN status)
	END CompileGuardRO;

	PROCEDURE CompileGuardRO1* (VAR par: Dialog.Par);
	BEGIN
		par.readOnly := {dataLoaded, modelCompiled} * status = {}
	END CompileGuardRO1;

	(* Notifiers *)

	PROCEDURE ChangeCompNotifier* (op, from, to: INTEGER);
	BEGIN
		IF op = Dialog.changed THEN
			BuildLists
		END
	END ChangeCompNotifier;

	PROCEDURE ChangeCovtsNotifier* (op, from, to: INTEGER);
	BEGIN
		IF op = Dialog.included THEN
			PKBugsCovts.IncludeCovts(specificationDialog.parameters.index, from, to)
		ELSIF op = Dialog.excluded THEN
			PKBugsCovts.ExcludeCovts(specificationDialog.parameters.index, from, to)
		ELSIF op = Dialog.set THEN
			HALT(0)
		END
	END ChangeCovtsNotifier;

	PROCEDURE ChangePriorNotifier* (op, from, to: INTEGER);
	BEGIN
		IF op = Dialog.changed THEN
			PKBugsPriors.StoreSummary(specificationDialog.parameters.index,
			specificationDialog.mean, specificationDialog.cv)
		END
	END ChangePriorNotifier;

	PROCEDURE GetCovts (parameter: INTEGER; VAR selection: Dialog.Selection);
		VAR i: INTEGER;
	BEGIN
		ASSERT(parameter < LEN(PKBugsCovts.covariates), 25);
		selection.Excl(0, selection.len - 1);
		i := 0;
		WHILE i < selection.len DO
			IF i IN PKBugsCovts.covariates[parameter] THEN selection.Incl(i, i) END;
			INC(i)
		END
	END GetCovts;

	PROCEDURE SelectParamNotifier* (op, from, to: INTEGER);
	BEGIN
		IF op = Dialog.changed THEN
			PKBugsPriors.GetSummary(specificationDialog.parameters.index,
			specificationDialog.mean, specificationDialog.cv);
			GetCovts(specificationDialog.parameters.index, specificationDialog.covariates);
			Dialog.Update(specificationDialog)
		END
	END SelectParamNotifier;

	(* Commands *)

	PROCEDURE ReadNames*;
		VAR
			pos, res: INTEGER;
			ok: BOOLEAN;
			m: Models.Model;
			s: PKBugsScanners.Scanner;
			msg: Dialog.String;
	BEGIN
		BugsCmds.NewModel(ok);
		IF ~ok THEN RETURN END;
		Reset;
		BugsCmds.Clear;
		m := Controllers.FocusModel();
		IF m # NIL THEN
			WITH m: TextModels.Model DO
				ConnectScanner(s, m);
				PKBugsNames.ReadNames(s, pos, res);
				IF res # 0 THEN
					PKBugsNames.Reset; Error(m, pos, res, ""); RETURN
				ELSE
					PKBugsNames.CheckNames(res);
					IF res # 0 THEN
						PKBugsNames.Reset; Error(NIL, 0, res, ""); RETURN
					END
				END
			END
		END;
		BugsMsg.Lookup("PKBugs:namesLoaded", msg);
		BugsFiles.ShowStatus(msg);
		status := {namesLoaded};
	END ReadNames;

	PROCEDURE ReadData*;
		VAR
			pos, res, nData, nRecords, ind, cov: INTEGER;
			m: Models.Model;
			string, inp, error, individual, msg: Dialog.String;
			s: PKBugsScanners.Scanner;
	BEGIN
		PKBugsData.Reset; PKBugsParse.Reset;
		PKBugsNodes.Reset; PKBugsPriors.Reset; PKBugsCovts.Reset;
		m := Controllers.FocusModel();
		pos := 0; res := 0;
		IF m # NIL THEN
			WITH m: TextModels.Model DO
				ConnectScanner(s, m);
				PKBugsData.CountData(s, nData, pos, res);
				IF res # 0 THEN Error(m, pos, res, ""); RETURN END;
				IF (nData MOD PKBugsNames.nItems # 0) THEN
					Error(NIL, 0, 5105, ""); RETURN	(* no. of data inconsistent with no. of data items *)
				END;
				nRecords := nData DIV PKBugsNames.nItems;
				PKBugsData.InitializeData(nRecords);
				ConnectScanner(s, m);
				PKBugsData.LoadData(s, pos, res);
				IF res # 0 THEN Error(m, pos, res, ""); RETURN END;
				PKBugsParse.BuildHistory(res, ind, inp);
				IF res # 0 THEN
					Strings.IntToString(ind, string);
					IF res < 0 THEN
						res := - res;
						Strings.IntToString(res, error); error := "#PKBugs:" + error;
						Dialog.MapParamString(error, inp, string, "", msg);
						BugsFiles.ShowStatus(msg);
						Dialog.Beep
					ELSE
						Error(NIL, 0, res, string)
					END;
					RETURN
				END;
				PKBugsCovts.GetData(res, ind, cov);
				IF res # 0 THEN
					Strings.IntToString(res, error); error := "#PKBugs:" + error;
					Strings.IntToString(ind, individual); specificationDialog.covariates.GetItem(cov, string);
					Dialog.ShowParamMsg(error, string, individual, ""); Dialog.Beep; RETURN
				END;
			END;
			res := 0;
			PKBugsParse.CheckInputs(res, ind);
			IF res # 0 THEN
				Strings.IntToString(ind, string); Error(NIL, 0, res, string); RETURN
			END;
			PKBugsParse.StoreHist;
		END;
		BugsMsg.Lookup("PKBugs:dataLoaded", msg);
		BugsFiles.ShowStatus(msg);
		status := {dataLoaded};
		BuildLists;
	END ReadData;

	PROCEDURE Compile*;
		VAR
			chain, ind, numChains, par, res: INTEGER;
			msg, s: Dialog.String;
			log, updaterByMethod, ok: BOOLEAN;
			u: REAL;
	BEGIN
		res := 0;
		ind := 0;
		updaterByMethod := BugsCmds.compileDialog.updaterByMethod;
		log := specificationDialog.residuals = 1;
		numChains := specificationDialog.numChains;
		BugsCmds.specificationDialog.numChains := numChains;
		PKBugsNodes.CreateDataNodes(log, res, ind);
		IF res # 0 THEN
			Strings.IntToString(ind, s); Error(NIL, 0, res, s); SetStatus({namesLoaded}); RETURN
		END;
		PKBugsPriors.CheckPriors(res, par);
		IF res # 0 THEN
			specificationDialog.parameters.GetItem(par, s); Error(NIL, 0, res, s); RETURN
		END;
		PKBugsNodes.StoreData;
		PKBugsNodes.StoreNodes(specificationDialog.nComp);
		PKBugsNodes.SetGraph;
		PKBugsTree.Build(specificationDialog.nComp, log);
		BugsGraph.Compile(numChains, updaterByMethod, ok);
		ASSERT(ok, 77);
		chain := 0;
		WHILE chain < numChains DO
			IF chain = 0 THEN u := 1.0 ELSE u := MathRandnum.Uniform(0.75, 1.25) END;
			PKBugsPriors.GenerateInitsForChain(chain, u);
			INC(chain)
		END;
		BugsMsg.Lookup("PKBugs:modelCompiled", msg);
		Dialog.ShowStatus(msg);
		SetStatus({modelCompiled});
		BugsDialog.UpdateDialogs
	END Compile;

	(*	for use in scripting language	*)

	PROCEDURE SetParam* (param: INTEGER);
		VAR
			len: INTEGER;
	BEGIN
		len := specificationDialog.parameters.len;
		IF (param > 0) & (param <= len) THEN
			specificationDialog.parameters.index := param - 1;
		END
	END SetParam;

	PROCEDURE DefineRegression* (IN covariates: ARRAY OF CHAR);
		VAR
			s: BugsMappers.Scanner;
			i, len: INTEGER;
	BEGIN
		IF PKBugsCovts.covariateNames # NIL THEN
			s.ConnectToString(covariates);
			len := LEN(PKBugsCovts.covariateNames);
			s.SetPos(0);
			WHILE ~s.eot DO
				s.Scan;
				IF s.type = BugsMappers.string THEN
					i := 0;
					WHILE (i < len) & (s.string # PKBugsCovts.covariateNames[i]) DO INC(i) END;
					IF i < len THEN
						PKBugsCovts.IncludeCovts(specificationDialog.parameters.index, i, i)
					END
				END
			END
		END
	END DefineRegression;

	PROCEDURE Notifier*;
		VAR
			i, parameter: INTEGER;
			selection: Dialog.Selection;
	BEGIN
		parameter := specificationDialog.parameters.index;
		PKBugsPriors.StoreSummary(parameter, specificationDialog.mean, specificationDialog.cv);
		selection := specificationDialog.covariates;
		selection.Excl(0, selection.len - 1);
		i := 0;
		WHILE i < selection.len DO
			IF i IN PKBugsCovts.covariates[parameter] THEN selection.Incl(i, i) END;
			INC(i)
		END;
		Dialog.UpdateList(specificationDialog);
		Dialog.Update(specificationDialog)
	END Notifier;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
		NEW(specificationDialog);
		InitDialog(specificationDialog);
		BugsDialog.AddDialog(specificationDialog);
		status := {}
	END Init;

BEGIN
	Init
END PKBugsCmds.
