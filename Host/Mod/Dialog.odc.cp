MODULE HostDialog;
	(*
	DONE:
		Denisov: CloseDialog убрал флаг 2, поместил правильную константу кнопок YES&NO
		ShiryaevAV: UTF-8, преобразование кодировки имён файлов
		ShiryaevAV: Preferences dialog
	TODO:
		Map titles for the dialogs 
		Preferences dialog: true type metric; visual scroll
	*)

	IMPORT
		SYSTEM, Log, LinLibc,
		GLib := Gtk2GLib, Gtk := Gtk2Gtk,
		Dialog, Files, Stores, Views, Ports, Converters,
		Windows, Fonts, Strings, Properties, StdCmds,
		HostFiles, HostFonts, HostWindows,
		HostLang, HostCFrames, HostRegistry, Iconv := LinIconv, Kernel;
		
	CONST
		(** CloseDialog res **)
		save* = 1; cancel* = 2;
		
		dirtyString = "#Host:SaveChanges";

	CONST	sepChar = GLib.G_DIR_SEPARATOR;
	
	TYPE
		ShowHook = POINTER TO RECORD (Dialog.ShowHook) END;
		DialogHook = POINTER TO RECORD (Dialog.GetHook) END;
		GetSpecHook = POINTER TO RECORD (Views.GetSpecHook) END;
		ExtCallHook = POINTER TO RECORD (Dialog.ExtCallHook) END;
		
	VAR	
		hist:	HostFiles.FullName;
		encoder, decoder: Iconv.iconv_t;

		prefs*: RECORD
			useTTMetric*: BOOLEAN;
			visualScroll*: BOOLEAN;
			statusbar*: INTEGER;
			thickCaret*: BOOLEAN;
			caretPeriod*: INTEGER;
			beep*: BOOLEAN;
			serverMode*: BOOLEAN;
			language*: Dialog.Combo;
		END;
		
		prefFName, prefDName: Fonts.Typeface;
		prefFSize, prefDSize, prefDWght: INTEGER;
		prefDStyle: SET;

RunDialog
  
	PROCEDURE [ccall] Response (dialog :Gtk.GtkDialog; id,user_data: INTEGER);
	BEGIN
		response:=id;
	END Response;

	PROCEDURE RunDialog(dialog :Gtk.GtkDialog ):INTEGER;
	VAR res: INTEGER; 
	BEGIN
		res := GtkU.gtk_signal_connect(dialog, "response", SYSTEM.ADR(Response), 0);
		Gtk.gtk_window_set_modal(dialog, 1); (* TODO: *)
		Gtk.gtk_widget_show_now(dialog);
		response:=Gtk.GTK_RESPONSE_NONE;
		WHILE response=Gtk.GTK_RESPONSE_NONE DO 
			res := Gtk.gtk_main_iteration();
			Windows.dir.Update(NIL);
		END;
		RETURN response
	END RunDialog;

	PROCEDURE RunDialog(dialog :Gtk.GtkDialog ):INTEGER;
	VAR res: INTEGER; 
	BEGIN
		HostWindows.dialogIsOpen := TRUE;
		res := Gtk.gtk_dialog_run(dialog);
		HostWindows.dialogIsOpen := FALSE;
		RETURN res;
	END RunDialog;
 
(* Show Hook *)

	PROCEDURE ShowParamMsg* (IN str, p0, p1, p2: ARRAY OF CHAR);
		VAR res: INTEGER; 
			dlg: Gtk.GtkMessageDialog; 
			st: ARRAY 512 OF CHAR;
			us: GLib.PString;
	BEGIN
		ASSERT(str # "", 20);
		(*Dialog.appName ?*);
		Dialog.MapParamString(str, p0, p1, p2, st); 
		us := GLib.g_utf16_to_utf8(st, -1, NIL, NIL, NIL);
		dlg := Gtk.gtk_message_dialog_new (NIL, (* TODO: NIL-> main_application_window*)
			Gtk.GTK_DIALOG_MODAL + Gtk.GTK_DIALOG_DESTROY_WITH_PARENT,
			Gtk.GTK_MESSAGE_INFO,
			Gtk.GTK_BUTTONS_OK,
			us);
		GLib.g_free(SYSTEM.VAL(GLib.gpointer, us));
		res := RunDialog(dlg);
		Gtk.gtk_widget_destroy(dlg) 
	END ShowParamMsg;

	PROCEDURE ShowParamStatus* (IN str, p0, p1, p2: ARRAY OF CHAR);
		VAR res: INTEGER; 
		st: ARRAY 512 OF CHAR; 
	BEGIN
		Dialog.MapParamString(str, p0, p1, p2, st);
		HostWindows.SetStatusText(st)
	END ShowParamStatus;

	PROCEDURE (h: ShowHook) ShowParamMsg (IN str, p0, p1, p2: ARRAY OF CHAR);
	BEGIN
		ShowParamMsg(str, p0, p1, p2)
	END ShowParamMsg;
	
	PROCEDURE (h: ShowHook) ShowParamStatus (IN str, p0, p1, p2: ARRAY OF CHAR);
	BEGIN
		ShowParamStatus(str, p0, p1, p2)
	END ShowParamStatus;
(*=====*)
(** Dialogs  *)

	PROCEDURE (hook: DialogHook) GetOK (IN str, p0, p1, p2: ARRAY OF CHAR; form: SET; OUT res: INTEGER);
		VAR r: INTEGER; 
			dlg: Gtk.GtkMessageDialog; 
			st: ARRAY 512 OF CHAR; 
			us: GLib.PString;
			type:Gtk.GtkMessageType;
			buttons:Gtk.GtkButtonsType;
	BEGIN
		ASSERT(str # "", 20);
		Dialog.MapParamString(str, p0, p1, p2, st);
		IF Dialog.yes IN form THEN
			type:=Gtk.GTK_MESSAGE_QUESTION;
			buttons:= Gtk.GTK_BUTTONS_YES_NO;
			(* IF Dialog.cancel IN form THEN YES_NO_CANCEL	END *)
		ELSE (* ok *)
			type:=Gtk.GTK_MESSAGE_WARNING;
			IF Dialog.cancel IN form THEN buttons:= Gtk.GTK_BUTTONS_OK_CANCEL ELSE buttons := Gtk.GTK_BUTTONS_OK END;
		END;
		us := GLib.g_utf16_to_utf8(st, -1, NIL, NIL, NIL);
		dlg := Gtk.gtk_message_dialog_new(NIL,
			Gtk.GTK_DIALOG_MODAL + Gtk.GTK_DIALOG_DESTROY_WITH_PARENT,
			type, buttons, us);
		GLib.g_free(SYSTEM.VAL(GLib.gpointer, us));
		r := RunDialog(dlg);
		CASE r OF
		| Gtk.GTK_RESPONSE_YES : res := Dialog.yes
		| Gtk.GTK_RESPONSE_CANCEL: res := Dialog.cancel
		| Gtk.GTK_RESPONSE_OK : res := Dialog.ok
		| Gtk.GTK_RESPONSE_NO : res := Dialog.no
		ELSE res := 0
		END;
		Gtk.gtk_widget_destroy(dlg); 
	END GetOK;


	PROCEDURE CloseDialog* (w: Windows.Window; quit: BOOLEAN; VAR res: INTEGER);
		VAR r: INTEGER; 
			dlg: Gtk.GtkMessageDialog; 
			title: Views.Title; 
			text: ARRAY 256 OF CHAR;
			us: GLib.PString;
	BEGIN
		w.GetTitle(title);
		Dialog.MapParamString(dirtyString, title, 0DX, 0DX, text);
		us := GLib.g_utf16_to_utf8(text, -1, NIL, NIL, NIL);
		dlg := Gtk.gtk_message_dialog_new(NIL,
			Gtk.GTK_DIALOG_MODAL + Gtk.GTK_DIALOG_DESTROY_WITH_PARENT,
			Gtk.GTK_MESSAGE_QUESTION,
			Gtk.GTK_BUTTONS_YES_NO, us);
		GLib.g_free(SYSTEM.VAL(GLib.gpointer, us));
		r := RunDialog(dlg);
		Gtk.gtk_widget_destroy(dlg); 

		IF r = Gtk.GTK_RESPONSE_YES THEN res := save
		ELSIF r = Gtk.GTK_RESPONSE_NO THEN res := 0
		ELSE res := cancel
		END;
	END CloseDialog;

	PROCEDURE GetColor(in: Ports.Color; OUT out: Ports.Color; OUT set: BOOLEAN);
		VAR colorDialog: Gtk.GtkColorSelectionDialog; 
				color: Gtk.GtkColors;
				res: INTEGER; 
	BEGIN
		set := FALSE;
		colorDialog := Gtk.gtk_color_selection_dialog_new("Color");
		color[0] := (in MOD 256) / 255.0;
		color[1] := ((in DIV 256) MOD 256) / 255.0;
		color[2] := ((in DIV 65536) MOD 256) / 255.0;
		color[3] := 0; (* opacity *)
		Gtk.gtk_color_selection_set_color(colorDialog.colorsel, color);
		(* Gtk.gtk_widget_hide(colorDialog.help_button); *)
		res := RunDialog(colorDialog);
		CASE res OF
		| Gtk.GTK_RESPONSE_OK:
			Gtk.gtk_color_selection_get_color(colorDialog.colorsel, color);
			out := Ports.RGBColor(
										SHORT(ENTIER((color[0]*255))), 
										SHORT(ENTIER((color[1]*255))),
										SHORT(ENTIER((color[2]*255))));
			set := TRUE;
		| Gtk.GTK_RESPONSE_DELETE_EVENT:
		ELSE	
		END;
		Gtk.gtk_widget_destroy(colorDialog)
	END GetColor;

	PROCEDURE ColorDialog*;
	(* open color dialog and set selection to choosen color *)
		VAR set: BOOLEAN; p: Properties.StdProp; col: Ports.Color;
	BEGIN
		Properties.CollectStdProp(p);
		IF ~(Properties.color IN p.known) THEN p.color.val := Ports.black END;
		GetColor(p.color.val, col, set);
		IF set THEN StdCmds.Color(col) END
	END ColorDialog;
	
	PROCEDURE (hook: DialogHook) GetColor (in: Ports.Color; OUT out: Ports.Color; OUT set: BOOLEAN);
	BEGIN
		GetColor(in, out, set);
	END GetColor;
	
	PROCEDURE GetFont (VAR typeface: Fonts.Typeface; VAR size: INTEGER; VAR weight: INTEGER; VAR style: SET; VAR set: BOOLEAN);
	VAR 
			res: INTEGER; 
			fsDialog: Gtk.GtkFontSelectionDialog; 
			fn: GLib.PString; 
			s: ARRAY 256 OF SHORTCHAR;
	BEGIN
		set := FALSE;
		fsDialog := Gtk.gtk_font_selection_dialog_new("Font");
		HostFonts.MakePangoString(typeface$, size, style, weight, s);
		res := Gtk.gtk_font_selection_dialog_set_font_name(fsDialog, s);
		res := RunDialog(fsDialog);
		CASE res OF
		| Gtk.GTK_RESPONSE_OK:
				fn := Gtk.gtk_font_selection_dialog_get_font_name(fsDialog);
				HostFonts.ParsePangoString(fn$, typeface,  size, style, weight);
				set := TRUE
		| Gtk.GTK_RESPONSE_DELETE_EVENT:
		ELSE	
		END;
		Gtk.gtk_widget_destroy(fsDialog) 
	END GetFont;
	
	PROCEDURE FontDialog*;
	(** open font dialog and set selection to choosen attributes **)
		VAR set: BOOLEAN; p, p0: Properties.StdProp;
	BEGIN
		Properties.CollectStdProp(p0);
		IF Properties.typeface IN p0.known THEN
			NEW(p); 
			p.typeface := p0.typeface$;
			p.size := p0.size; 
			p.weight := p0.weight; 
			p.style := p0.style;
			GetFont(p.typeface, p.size, p.weight, p.style.val, set);
			IF set THEN
				p.valid := {Properties.typeface, Properties.style, Properties.weight, Properties.size};
				p.style.mask := {Fonts.italic, Fonts.underline, Fonts.strikeout};
				Properties.EmitProp(NIL, p)
			END
		END
	END FontDialog;

	PROCEDURE TypefaceDialog*;
	(** open font dialog and set selection to choosen typeface **)
		VAR set: BOOLEAN; p, p0: Properties.StdProp; s, w: INTEGER; st: SET;
	BEGIN
		Properties.CollectStdProp(p0);
		IF Properties.typeface IN p0.known THEN
			NEW(p); 
			p.typeface := p0.typeface$;
			GetFont(p.typeface, s, w, st, set);
			IF set THEN
				p.valid := {Properties.typeface};
				Properties.EmitProp(NIL, p)
			END
		END
	END TypefaceDialog;

	(*** A. V. Shiryaev, 2012.11: filenames encoding translation ***)

	PROCEDURE ConvInit;
	BEGIN
		(* NOTE: In case of Gtk for Windows, use UTF-8 encoding instead of HostLang.enc *)
		IF Kernel.littleEndian THEN
			decoder := Iconv.iconv_open("UCS-2LE", HostLang.enc);
			encoder := Iconv.iconv_open(HostLang.enc, "UCS-2LE");
		ELSE
			decoder := Iconv.iconv_open("UCS-2BE", HostLang.enc);
			encoder := Iconv.iconv_open(HostLang.enc, "UCS-2BE")
		END;
	END ConvInit;

	PROCEDURE ConvClose;
		VAR res: INTEGER;
	BEGIN
		IF decoder # -1 THEN res := Iconv.iconv_close(decoder); decoder := -1 END;
		IF encoder # -1 THEN res := Iconv.iconv_close(encoder); encoder := -1 END
	END ConvClose;

	PROCEDURE ResetCodec (c: Iconv.iconv_t): BOOLEAN;
		VAR res, fLen, tLen: Iconv.size_t;
	BEGIN
		ASSERT(c # -1, 20);
		fLen := 0; tLen := 0;
		res := Iconv.iconv(c, NIL, fLen, NIL, tLen);
		RETURN res # -1
	END ResetCodec;

	(* decode filename from HostLang.enc encoding *)
	PROCEDURE Long (ss: Iconv.PtrSTR; OUT s: ARRAY OF CHAR);
		VAR res: Iconv.size_t;
			fLen, tLen: Iconv.size_t;
			to: Iconv.PtrLSTR;
			i: INTEGER;
	BEGIN
		i := 0;
		WHILE (i < LEN(s) - 1) & (ss[i] >= ' ') & (ss[i] <= '~') DO s[i] := ss[i]; INC(i) END;
		IF ss[i] = 0X THEN
			IF i < LEN(s) THEN s[i] := 0X
			ELSE s[0] := 0X
			END
		ELSIF (decoder # -1) & ResetCodec(decoder) THEN
			fLen := LEN(ss$); to := s; tLen := (LEN(s) - 1) * SIZE(CHAR);
			res := Iconv.iconv_decode(decoder, ss, fLen, to, tLen);
			IF (res >= 0) & (fLen = 0) & (tLen >= 0) THEN to[0] := 0X
			ELSE s[0] := 0X
			END
		ELSE s[0] := 0X
		END
	END Long;

	(* encode filename to HostLang.enc encoding *)
	PROCEDURE Short (IN s: ARRAY OF CHAR; OUT ss: ARRAY OF SHORTCHAR);
		VAR res: Iconv.size_t;
			fLen, tLen: Iconv.size_t;
			from: Iconv.PtrLSTR;
			to: Iconv.PtrSTR;
			i: INTEGER;
	BEGIN
		i := 0;
		WHILE (i < LEN(ss) - 1) & (s[i] >= ' ') & (s[i] <= '~') DO ss[i] := SHORT(s[i]); INC(i) END;
		IF s[i] = 0X THEN
			IF i < LEN(ss) THEN ss[i] := 0X
			ELSE ss[0] := 0X
			END
		ELSIF (encoder # -1) & ResetCodec(encoder) THEN
			from := s; fLen := LEN(s$) * SIZE(CHAR); to := ss; tLen := LEN(ss) - 1;
			res := Iconv.iconv_encode(encoder, from, fLen, to, tLen);
			IF (res >= 0) & (fLen = 0) & (tLen >= 0) THEN to[0] := 0X
			ELSE ss[0] := 0X
			END
		ELSE ss[0] := 0X
		END
	END Short;

	(*** end of filenames encoding translation ***)

	(* s -> [ locName sepChar ] name *)
	PROCEDURE SplitFileName (IN s: ARRAY OF CHAR; OUT name: Files.Name; OUT locName: HostFiles.FullName);
		VAR i, j, sepIdx, len: INTEGER;
	BEGIN
		len := LEN(s$);
		i := len - 1; WHILE (i >= 0) & (s[i] # sepChar) DO DEC(i) END;
		IF (i >= 0) & (s[i] = sepChar) THEN
			sepIdx := i;
			(* s(sepIdx;len) -> name *)
				INC(i);
				j := 0;
				WHILE (i < len) & (j < LEN(name) - 1) DO name[j] := s[i]; INC(j); INC(i) END;
				IF (i = len) & (j < LEN(name)) THEN name[j] := 0X
				ELSE name[0] := 0X
				END;
			(* s[0;sepIdx) -> locName *)
				i := 0;
				WHILE (i < sepIdx) & (i < LEN(locName) - 1) DO locName[i] := s[i]; INC(i) END;
				IF (i = sepIdx) & (i < LEN(locName)) THEN locName[i] := 0X
				ELSE locName[0] := 0X
				END
		ELSIF len > 0 THEN
			name := s$; locName[0] := 0X
		ELSE
			name[0] := 0X; locName[0] := 0X
		END
	END SplitFileName;

	##=>(* old version of GetFileSpec *)

	PROCEDURE GetFileSpec (mode:INTEGER; VAR loc: Files.Locator; VAR name: Files.Name);
		VAR fs: Gtk.GtkFileSelection; 
				res: INTEGER;  
				locName: HostFiles.FullName; 
				ss: GLib.PString;
				ss1: ARRAY LEN(HostFiles.FullName) * 4 OF SHORTCHAR;
				s: HostFiles.FullName;
	BEGIN
		Gtk.gtk_file_chooser_dialog_new
		CASE mode OF
		| 0 : fs := Gtk.gtk_file_selection_new("Open");    (*<--*)	
		| 1 : fs := Gtk.gtk_file_selection_new("Save As"); (*<--*)	
		END;
			IF loc # NIL THEN
				Short(loc(HostFiles.Locator).path + sepChar + name, ss1);
				Gtk.gtk_file_selection_set_filename(fs, ss1);
			ELSIF hist # "" THEN
				Short(hist + sepChar, ss1);
				Gtk.gtk_file_selection_set_filename(fs, ss1);
			END;
		(* Gtk.gtk_file_selection_hide_fileop_buttons(fs); *)
		res := RunDialog(fs);
		CASE res OF
		| Gtk.GTK_RESPONSE_OK:
			ss := Gtk.gtk_file_selection_get_filename(fs);
			Long(ss, s);
			SplitFileName(s,name,locName);
			loc := HostFiles.NewLocator(locName);
			hist:= loc(HostFiles.Locator).path$;
		| Gtk.GTK_RESPONSE_DELETE_EVENT:
		ELSE	
		END;
		Gtk.gtk_widget_destroy(fs) 
	END GetFileSpec;
	##<=

	PROCEDURE GetFileSpec (mode:INTEGER; VAR loc: Files.Locator; VAR name: Files.Name);
		VAR fs: Gtk.GtkFileChooserDialog; 
				res: INTEGER;  
				locName: HostFiles.FullName; 
				ss: GLib.PString;
				ss1: ARRAY LEN(HostFiles.FullName) * 4 OF SHORTCHAR;
				s: HostFiles.FullName;
	BEGIN
	(* gtk_file_chooser_set_current_name
	gtk_file_chooser_add_filter/remove *)
		
		CASE mode OF
		| 0 : fs := Gtk.gtk_file_chooser_dialog_new("Open", NIL,	0,
                  "gtk-open", Gtk.GTK_RESPONSE_OK,
                  "gtk-cancel", Gtk.GTK_RESPONSE_CANCEL, NIL, 0, 0)
		| 1 : fs := Gtk.gtk_file_chooser_dialog_new("Save As", NIL,	1,
                  "gtk-save", Gtk.GTK_RESPONSE_OK,
                  "gtk-cancel", Gtk.GTK_RESPONSE_CANCEL, NIL, 0, 0)
		END;
		
		IF (hist = "") & (loc = NIL) THEN
			IF name = "" THEN
				Short(Files.dir.This("")(HostFiles.Locator).path + sepChar + " ", ss1)
			ELSE
				Short(Files.dir.This("")(HostFiles.Locator).path + sepChar + name, ss1)
			END;
			Gtk.gtk_file_chooser_set_filename(fs, ss1)
		ELSIF loc # NIL THEN
			Short(loc(HostFiles.Locator).path + sepChar + name, ss1);
			Gtk.gtk_file_chooser_set_filename(fs, ss1)
		ELSIF hist # "" THEN
			Short(hist + sepChar + name, ss1);
			Gtk.gtk_file_chooser_set_filename(fs, ss1)
		END;
		res := RunDialog(fs);
		CASE res OF
		| Gtk.GTK_RESPONSE_OK:
			ss := Gtk.gtk_file_chooser_get_filename(fs);
			Long(ss, s);
			SplitFileName(s,name,locName);
			loc := HostFiles.NewLocator(locName);
			hist:= loc(HostFiles.Locator).path$;
		| Gtk.GTK_RESPONSE_DELETE_EVENT:
		ELSE
		END;
		Gtk.gtk_widget_destroy(fs) 
		
	END GetFileSpec;

	PROCEDURE FindConverter (IN name: Files.Name; VAR conv: Converters.Converter);
	VAR i, l: INTEGER; 
			type: Files.Type;
	BEGIN
		l := LEN(name$); 
		type:="";
		i := l;	WHILE (i > 0) & (name[i] # ".") DO DEC(i) END;
		IF i > 0 THEN
			Strings.Extract(name,i+1,l-i,type);
			conv := Converters.list; 
			WHILE (conv # NIL) & (conv.fileType # type) DO conv := conv.next END;
		ELSE 	
			conv := NIL; 
		END;
	END FindConverter;
	
	PROCEDURE GetIntSpec* (VAR loc: Files.Locator; VAR name: Files.Name; VAR conv: Converters.Converter);
	BEGIN
		GetFileSpec(0, loc, name);
		FindConverter(name,conv);
	END GetIntSpec;
	
	PROCEDURE GetExtSpec* (s: Stores.Store; VAR loc: Files.Locator; VAR name: Files.Name; VAR conv: Converters.Converter);
	BEGIN
		GetFileSpec(1, loc, name);
		FindConverter(name,conv);
	END GetExtSpec;
	
	PROCEDURE (hook: DialogHook) GetIntSpec (IN defType: Files.Type; VAR loc: Files.Locator; OUT name: Files.Name);
	BEGIN
		(* defType *)
		GetFileSpec(0, loc, name);
	END GetIntSpec;
	
	PROCEDURE (hook: DialogHook) GetExtSpec (IN default: Files.Name; IN defType: Files.Type; VAR loc: Files.Locator; OUT name: Files.Name);
	BEGIN
		(* defType *)
		name:=default;
		GetFileSpec(1, loc, name);
	END GetExtSpec;

	
	PROCEDURE (h: GetSpecHook) GetIntSpec (VAR loc: Files.Locator; VAR name: Files.Name; VAR conv: Converters.Converter);
	BEGIN
		GetIntSpec( loc, name, conv);
	END GetIntSpec;
	
	PROCEDURE (h: GetSpecHook) GetExtSpec (s: Stores.Store; VAR loc: Files.Locator; VAR name: Files.Name; VAR conv: Converters.Converter);
	BEGIN
		GetExtSpec( s, loc, name, conv);
	END GetExtSpec;


	(* preferences dialog *)

	PROCEDURE DefFont*;
		VAR tf: Fonts.Typeface; size: INTEGER; w: INTEGER; style: SET; set: BOOLEAN;
	BEGIN
		tf := prefFName;
		size := prefFSize;
		w := Fonts.normal;
		style := {};
		GetFont(tf, size, w, style, set);
		IF set THEN
			prefFName := tf; prefFSize := size
		END
	END DefFont;

	PROCEDURE DlgFont*;
		VAR tf: Fonts.Typeface; size: INTEGER; w: INTEGER; style: SET; set: BOOLEAN;
	BEGIN
		tf := prefDName;
		size := prefDSize;
		w := prefDWght;
		style := prefDStyle;
		GetFont(tf, size, w, style, set);
		IF set THEN
			prefDName := tf; prefDSize := size; prefDStyle := style; prefDWght := w
		END
	END DlgFont;

	PROCEDURE PrefOk*;
		VAR res: INTEGER;
	BEGIN
		HostFonts.SetDefaultFont(prefFName, prefFSize);
		HostFonts.SetDialogFont(prefDName, prefDSize, prefDStyle, prefDWght);

(*
		HostFonts.SetTTMetric(prefs.useTTMetric);
		HostWindows.SetVisualScroll(prefs.visualScroll);
		IF prefs.statusbar = 1 THEN Dialog.showsStatus := TRUE; HostWindows.memInStatus := FALSE
		ELSIF prefs.statusbar = 2 THEN Dialog.showsStatus := TRUE; HostWindows.memInStatus := TRUE
		ELSE Dialog.showsStatus := FALSE
		END;
*)
		Dialog.showsStatus := TRUE;

		Dialog.Call("StdCmds.UpdateAll", "", res);
		Dialog.Call("StdCmds.RecalcAllSizes", "", res);
		Dialog.Call("TextCmds.UpdateDefaultAttr", "", res);
		HostCFrames.SetDefFonts;
		HostRegistry.WriteBool("noStatus", ~Dialog.showsStatus);
(*
		HostRegistry.WriteBool("memStatus", HostWindows.memInStatus);
*)
(*
		res := WinApi.GetClientRect(HostWindows.main, rect);
		HostWindows.ResizeMainWindow(0, rect.right, rect.bottom);
*)
		Dialog.thickCaret := prefs.thickCaret;
		Dialog.caretPeriod := prefs.caretPeriod;
		HostRegistry.WriteBool("thickCaret", Dialog.thickCaret);
		HostRegistry.WriteInt("caretPeriod", Dialog.caretPeriod)
	END PrefOk;

	PROCEDURE InitPrefDialog*;
	BEGIN
		prefFName := HostFonts.defFont.alias;
		prefFSize := HostFonts.defFont.size;
		prefDName := HostFonts.dlgFont.typeface;
		prefDSize := HostFonts.dlgFont.size;
		prefDStyle := HostFonts.dlgFont.style;
		prefDWght := HostFonts.dlgFont.weight;

(*
		prefs.useTTMetric := HostFonts.useTTMetric;
		prefs.visualScroll := HostWindows.visualScroll;
*)
		prefs.visualScroll := TRUE;

(*
		IF ~Dialog.showsStatus THEN prefs.statusbar := 0
		ELSIF HostWindows.memInStatus THEN prefs.statusbar := 2
		ELSE prefs.statusbar := 1
		END;
*)
		prefs.statusbar := 2;

		prefs.thickCaret := Dialog.thickCaret;
		prefs.caretPeriod := Dialog.caretPeriod
	END InitPrefDialog;

	PROCEDURE (h: ExtCallHook) OpenExternal* (IN fileName: ARRAY OF CHAR);
	VAR  i, process: INTEGER; cmd: ARRAY [untagged] 128 OF SHORTCHAR;
	BEGIN
		FOR i  := 0 TO LEN(cmd) - 1 DO cmd[i] := 0X END;
		cmd := "xdg-open " + SHORT(fileName$);
		process := LinLibc.popen(cmd, "r");
	END OpenExternal;

	PROCEDURE (h: ExtCallHook) RunExternal* (IN exeName: ARRAY OF CHAR);
	BEGIN
		
	END RunExternal;

	PROCEDURE Init;
		VAR 
			showHook: ShowHook; 
			dialogHook: DialogHook; 
			getSpecHook: GetSpecHook;
			extCallHook: ExtCallHook;
	BEGIN
		ConvInit;

		Dialog.platform := Dialog.linux;
		Dialog.showsStatus := TRUE; (* TODO: Should be read from HostRegistry *)
		
		NEW(showHook); Dialog.SetShowHook(showHook);
		NEW(dialogHook); Dialog.SetGetHook(dialogHook);
		NEW(getSpecHook); Views.SetGetSpecHook(getSpecHook);
		NEW(extCallHook); Dialog.SetExtCallHook(extCallHook);
		HostFiles.MapParamString := Dialog.MapParamString
	END Init;

BEGIN
	Init
CLOSE
	ConvClose
END HostDialog.

