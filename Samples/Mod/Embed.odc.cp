(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE SamplesEmbed;

	

	IMPORT
		Files, Strings,
		BugsEmbed, BugsFiles, BugsIndex, BugsInterface, BugsMappers, BugsMsg,
		SamplesFormatted, SamplesIndex, SamplesInterface,
		UpdaterActions;

	TYPE
		Globals = POINTER TO RECORD(BugsEmbed.Globals) END;

	VAR
		beg*, end*, thin*, firstChain*, lastChain*: INTEGER;
		variable: ARRAY 1024 OF CHAR;
		fractions: ARRAY 10 OF REAL;

		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE InitGlobals;
	BEGIN
		variable := "";
		beg := 1;
		end := 10000000;
		thin := 1;
		firstChain := 1;
		lastChain := BugsEmbed.numChains
	END InitGlobals;

	PROCEDURE (g: Globals) Init;
	BEGIN
		InitGlobals
	END Init;

	PROCEDURE (g: Globals) Update;
	BEGIN
		lastChain := BugsEmbed.numChains;
	END Update;

	PROCEDURE SetVariable* (var: ARRAY OF CHAR);
	BEGIN
		variable := var$
	END SetVariable;

	PROCEDURE Clear*;
		VAR
			ok: BOOLEAN;
			msg: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		SamplesInterface.Clear(variable, ok);
		IF ~ok THEN
			p[0] := variable$;
			BugsMsg.GetError(msg);
			BugsMsg.ParamMsg(msg, p, msg);
			BugsFiles.ShowMsg(msg);
			RETURN
		END;
		BugsMsg.MapMsg("SamplesEmbed:MonitorCleared", msg);
		BugsFiles.ShowMsg(msg)
	END Clear;

	PROCEDURE CODA* (stemName: ARRAY OF CHAR);
		VAR
			i, numChains: INTEGER;
			msg: ARRAY 1024 OF CHAR;
			name: Files.Name;
			numAsString: ARRAY 8 OF CHAR;
			f: POINTER TO ARRAY OF BugsMappers.Formatter;
			file: Files.File;
			loc: Files.Locator;
	BEGIN
		numChains := lastChain - firstChain + 1;
		NEW(f, numChains + 1);
		i := 0;
		BugsFiles.PathToFileSpec(stemName, loc, name);
		WHILE i <= numChains DO
			file := Files.dir.New(loc, Files.dontAsk);
			BugsFiles.ConnectFormatter(f[i], file);
			f[i].SetPos(0);
			INC(i)
		END;
		SamplesFormatted.CODA(variable, beg - 1, end, thin, firstChain, lastChain, f);
		f[0].Register(name + "CODAindex");
		i := 1;
		WHILE i <= numChains DO
			Strings.IntToString(i, numAsString);
			f[i].Register(name + "CODAchain" + numAsString);
			INC(i)
		END;
		BugsMsg.MapMsg("SamplesEmbed:CODAFilesWritten", msg);
		BugsFiles.ShowMsg(msg)
	END CODA;

	PROCEDURE Labels*;
		VAR
			f: BugsMappers.Formatter;
	BEGIN
		BugsFiles.StdConnect(f);
		f.SetPos(0);
		SamplesFormatted.Labels(variable, beg - 1, end, thin, f);
		f.StdRegister
	END Labels;

	PROCEDURE Set*;
		VAR
			begIt, numChains: INTEGER;
			ok: BOOLEAN;
			msg: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		numChains := BugsEmbed.numChains;
		begIt := UpdaterActions.iteration;
		SamplesInterface.Set(variable, begIt, numChains, ok);
		IF ~ok THEN
			p[0] := variable$;
			BugsMsg.GetError(msg);
			BugsMsg.ParamMsg(msg, p, msg);
			BugsFiles.ShowMsg(msg);
			RETURN
		END;
		BugsMsg.MapMsg("SamplesEmbed:MonitorSet", msg);
		BugsFiles.ShowMsg(msg)
	END Set;

	PROCEDURE Stats*;
		CONST
			options = {SamplesFormatted.medianOpt, SamplesFormatted.quant0Opt,
			SamplesFormatted.quant7Opt};
		VAR
			begIt, i, numTabs: INTEGER;
			f: BugsMappers.Formatter;
			tabs: POINTER TO ARRAY OF INTEGER;
	BEGIN
		BugsFiles.StdConnect(f);
		f.SetPos(0);
		IF BugsEmbed.isScript THEN
			f.WriteLn; f.WriteString("Node statistics"); f.WriteLn
		END;
		numTabs := 10;
		NEW(tabs, numTabs);
		tabs[0] := 0;
		tabs[1] := 25;
		i := 2;
		WHILE i < numTabs DO
			tabs[i] := tabs[1] + 10 * (i - 1);
			INC(i)
		END;
		f.WriteRuler(tabs);
		(*	only include values stored after end of adapting period	*)
		begIt := MAX(beg, UpdaterActions.endOfAdapting) - 1;
		SamplesFormatted.StatsSummary(variable, begIt, end, thin, firstChain, lastChain, options, fractions, f);
		f.StdRegister
	END Stats;

	PROCEDURE SampleSize* (): INTEGER;
	BEGIN
		RETURN SamplesInterface.SampleSize(variable, beg - 1, end, thin, firstChain, lastChain)
	END SampleSize;

	PROCEDURE SampleStart* (): INTEGER;
	BEGIN
		RETURN SamplesInterface.SampleStart(variable, beg - 1, end, thin)
	END SampleStart;

	PROCEDURE SampleSummary* (IN fractions: ARRAY OF REAL;
	OUT mean, median, mode, sd, skew, exKur, error: REAL;
	OUT percentiles, lower, upper: ARRAY OF REAL;
	OUT start, sampleSize, ess: INTEGER);
	BEGIN
		SamplesInterface.Statistics(variable, beg - 1, end, thin, firstChain, lastChain,
		fractions, mean, median, mode, sd, skew, exKur, error, percentiles, lower, upper,
		start, sampleSize, ess);
	END SampleSummary;

	PROCEDURE SampleValues* (OUT buffer: ARRAY OF REAL);
		VAR
			i, j, numChains, sampleSize: INTEGER;
			sample: POINTER TO ARRAY OF ARRAY OF REAL;
	BEGIN
		SamplesInterface.SampleValues(variable, beg - 1, end, thin, firstChain, lastChain, sample);
		numChains := LEN(sample, 0);
		sampleSize := LEN(sample, 1);
		i := 0;
		WHILE i < numChains DO
			j := 0;
			WHILE j < sampleSize DO
				buffer[i * sampleSize + j] := sample[i, j];
				INC(j)
			END;
			INC(i)
		END
	END SampleValues;

	PROCEDURE SetGuard* (OUT ok: BOOLEAN);
		VAR
			i: INTEGER;
			msg, var: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		ok := BugsInterface.IsInitialized();
		IF ~ok THEN
			BugsMsg.MapMsg("SamplesEmbed:NotInitialized", msg);
			BugsFiles.ShowMsg(msg)
		ELSE
			var := variable$;
			i := 0;
			WHILE (var[i] # 0X) & (var[i] # "[") DO
				INC(i)
			END;
			var[i] := 0X;
			ok := BugsIndex.Find(var) # NIL;
			IF ~ok THEN
				p[0] := var$;
				BugsMsg.MapParamMsg("SamplesEmbed:NotVariable", p, msg);
				BugsFiles.ShowMsg(msg)
			END
		END
	END SetGuard;

	PROCEDURE StatsGuard* (OUT ok: BOOLEAN);
		VAR
			i, numMonitors: INTEGER;
			msg, var: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		ok := BugsInterface.IsInitialized();
		IF ~ok THEN
			BugsMsg.MapMsg("SamplesEmbed:NotInitialized", msg);
			BugsFiles.ShowMsg(msg)
		ELSE
			ok := ~BugsInterface.IsAdapting();
			IF ~ok THEN
				BugsMsg.MapMsg("SamplesEmbed:Adapting", msg);
				BugsFiles.ShowMsg(msg)
			ELSE
				var := variable$;
				IF~SamplesInterface.IsStar(var) THEN
					i := 0;
					WHILE (var[i] # 0X) & (var[i] # "[") DO
						INC(i)
					END;
					var[i] := 0X;
					ok := SamplesIndex.Find(var) # NIL;
					IF ~ok THEN
						p[0] := var$;
						BugsMsg.MapParamMsg("SamplesEmbed:NotSet", p, msg);
						BugsFiles.ShowMsg(msg)
					END
				ELSE
					numMonitors := SamplesIndex.NumberOfMonitors();
					ok := numMonitors # 0;
					IF ~ok THEN
						BugsMsg.MapMsg("SamplesEmbed:NotSet", msg);
						BugsFiles.ShowMsg(msg)
					END
				END
			END
		END
	END StatsGuard;

	PROCEDURE BGRGuard* (OUT ok: BOOLEAN);
		VAR
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		IF BugsEmbed.numChains = 1 THEN
			ok := FALSE;
			BugsMsg.MapMsg("SamplesEmbed:OnlyOneChain", msg);
			BugsFiles.ShowMsg(msg)
		ELSE
			StatsGuard(ok)
		END
	END BGRGuard;

	PROCEDURE HistoryGuard* (OUT ok: BOOLEAN);
		VAR
			i, numMonitors: INTEGER;
			msg, var: ARRAY 1024 OF CHAR;
			p: ARRAY 1 OF ARRAY 1024 OF CHAR;
	BEGIN
		ok := BugsInterface.IsInitialized();
		IF ~ok THEN
			BugsMsg.MapMsg("SamplesEmbed:NotInitialized", msg);
			BugsFiles.ShowMsg(msg)
		ELSE
			var := variable$;
			IF~SamplesInterface.IsStar(var) THEN
				i := 0;
				WHILE (var[i] # 0X) & (var[i] # "[") DO
					INC(i)
				END;
				var[i] := 0X;
				ok := SamplesIndex.Find(var) # NIL;
				IF ~ok THEN
					p[0] := var$;
					BugsMsg.MapParamMsg("SamplesEmbed:NotSet", p, msg);
					BugsFiles.ShowMsg(msg)
				END
			ELSE
				numMonitors := SamplesIndex.NumberOfMonitors();
				ok := numMonitors # 0;
				IF ~ok THEN
					BugsMsg.MapMsg("SamplesEmbed:NoMonitors", msg);
					BugsFiles.ShowMsg(msg)
				END
			END
		END
	END HistoryGuard;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
		VAR
			g: Globals;
	BEGIN
		InitGlobals;
		NEW(g);
		BugsEmbed.AddGlobals(g);
		fractions[0] := 2.5;
		fractions[1] := 5;
		fractions[2] := 10;
		fractions[3] := 25;
		fractions[4] := 75;
		fractions[5] := 90;
		fractions[6] := 95;
		fractions[7] := 97.5;
		fractions[8] := 90;
		fractions[9] := 95;
		Maintainer
	END Init;

BEGIN
	Init
END SamplesEmbed.
