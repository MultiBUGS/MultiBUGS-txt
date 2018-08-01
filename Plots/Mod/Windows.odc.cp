(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE PlotsWindows;

	

	IMPORT
		SYSTEM,
		Fonts, Views,
		HostPorts,
		WinApi,
		PlotsAxis;

	TYPE
		VertString = POINTER TO RECORD(PlotsAxis.VertString) END;

	VAR
		maintainer-: ARRAY 20 OF CHAR;
		version-: INTEGER;

	PROCEDURE SetClipRegion (frame: Views.Frame; OUT noWindow: BOOLEAN): INTEGER;
		VAR
			dc, res, rl, rt, rr, rb: INTEGER;
			port: HostPorts.Port;
	BEGIN
		port := frame.rider(HostPorts.Rider).port; dc := port.dc;
		noWindow := port.wnd = 0;
		IF noWindow THEN res := WinApi.SaveDC(dc)
		ELSE res := WinApi.SelectClipRgn(dc, 0) END;
		frame.rider.GetRect(rl, rt, rr, rb);
		res := WinApi.IntersectClipRect(dc, rl, rt, rr, rb);
		RETURN dc
	END SetClipRegion;

	PROCEDURE (d: VertString) Draw (f: Views.Frame; txt: ARRAY OF CHAR; xl, yl, colour: INTEGER;
	font: Fonts.Font);
		VAR
			dc, res, i, hgt, winFont, ang, oldFont, italic, under, strike, len: INTEGER;
			winTxt, face: ARRAY[untagged] 80 OF SHORTCHAR;
			oldAlign: SET;
			noWindow: BOOLEAN;
	BEGIN
		dc := SetClipRegion(f, noWindow);
		xl := (xl + f.gx) DIV f.unit;
		yl := (yl + f.gy) DIV f.unit;
		len := LEN(txt$);
		i := 0; WHILE i < len DO winTxt[i] := SHORT(txt[i]); INC(i) END;
		len := LEN(font.typeface$);
		i := 0; WHILE i < len DO face[i] := SHORT(font.typeface[i]); INC(i) END;
		res := WinApi.SetTextColor(dc, colour);
		hgt := font.size * WinApi.GetDeviceCaps(dc, WinApi.LOGPIXELSY) DIV (Fonts.point * 60);
		ang := 900;
		IF Fonts.italic IN font.style THEN italic := WinApi.TRUE
		ELSE italic := WinApi.FALSE
		END;
		IF Fonts.underline IN font.style THEN under := WinApi.TRUE
		ELSE under := WinApi.FALSE
		END;
		IF Fonts.strikeout IN font.style THEN strike := WinApi.TRUE
		ELSE strike := WinApi.FALSE
		END;
		winFont := WinApi.CreateFont(hgt, 0, ang, ang, font.weight, italic, under, strike,
		WinApi.ANSI_CHARSET, WinApi.OUT_DEFAULT_PRECIS, WinApi.CLIP_DEFAULT_PRECIS,
		WinApi.DEFAULT_QUALITY, WinApi.DEFAULT_PITCH, face);
		oldFont := WinApi.SelectObject(dc, winFont);
		oldAlign := WinApi.SetTextAlign(dc, WinApi.TA_BASELINE + WinApi.TA_CENTER);
		res := WinApi.TextOut(dc, xl, yl, winTxt, LEN(txt$));
		res := WinApi.SelectObject(dc, oldFont);
		res := WinApi.DeleteObject(winFont);
		oldAlign := WinApi.SetTextAlign(dc, oldAlign);
		IF noWindow THEN res := WinApi.RestoreDC(dc, - 1) END
	END Draw;

	PROCEDURE Maintainer;
	BEGIN
		maintainer := "A.Thomas";
		version := 500
	END Maintainer;

	PROCEDURE Install*;
		VAR
			d: VertString;
	BEGIN
		NEW(d);
		PlotsAxis.SetVertString(d);
	END Install;

BEGIN
	Maintainer
END PlotsWindows.
