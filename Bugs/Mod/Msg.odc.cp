(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsMsg;


	

	IMPORT
		Log, Dialog, Files, Strings; 

	TYPE
		MsgList = POINTER TO RECORD
			key, msg: POINTER TO ARRAY OF CHAR;
			next: MsgList
		END;

	VAR
		message-: ARRAY 1024 OF CHAR;
		msgList: MsgList;

		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Lookup* (IN key: ARRAY OF CHAR; OUT mes: ARRAY OF CHAR);
		VAR
			cursor: MsgList;
	BEGIN
		cursor := msgList;
		WHILE (cursor # NIL) & (key # cursor.key^) DO
			cursor := cursor.next
		END;
		IF cursor # NIL THEN
			mes := cursor.msg$
		ELSE
			mes := key$
		END;
	END Lookup;
	
	PROCEDURE InverseMapMsg* (IN msg: ARRAY OF CHAR; OUT key: ARRAY OF CHAR);
		VAR
			cursor: MsgList;
	BEGIN
		cursor := msgList;
		WHILE (cursor # NIL) & (msg # cursor.msg^) DO
			cursor := cursor.next
		END;
		IF cursor # NIL THEN
			key := cursor.key$
		ELSE
			key := ""
		END
	END InverseMapMsg;

	PROCEDURE LookupParam* (IN key: ARRAY OF CHAR; IN p: ARRAY OF ARRAY OF CHAR; 
	OUT msg: ARRAY OF CHAR);
		VAR
			i, len, pos: INTEGER;
			pat: ARRAY 128 OF CHAR;
			cursor: MsgList;
	BEGIN
		cursor := msgList;
		WHILE (cursor # NIL) & (key # cursor.key^) DO
			cursor := cursor.next
		END;
		IF cursor # NIL THEN
			msg := cursor.msg$;
			i := 0;
			len := LEN(p);
			WHILE i < len DO
				Strings.IntToString(i, pat);
				pat := "^" + pat;
				pos := 0;
				WHILE pos #  - 1 DO
					Strings.Find(msg, pat, 0, pos);
					IF pos #  - 1 THEN
						Strings.Replace(msg, pos, LEN(pat$), p[i])
					END
				END;
				INC(i)
			END
		ELSE
			msg := key$
		END
	END LookupParam;

	PROCEDURE Show* (IN key: ARRAY OF CHAR);
		VAR
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		Lookup(key, msg);
		Dialog.ShowStatus(msg);
		Log.String(msg); Log.Ln
	END Show;
	
	PROCEDURE ShowParam* (IN key: ARRAY OF CHAR; IN p: ARRAY OF ARRAY OF CHAR);
		VAR
			msg: ARRAY 1024 OF CHAR;
	BEGIN
		LookupParam(key, p, msg);
		Dialog.ShowStatus(msg);
		Log.String(msg); Log.Ln
	END ShowParam;

	PROCEDURE Store* (IN msg: ARRAY OF CHAR);
	BEGIN
		message := msg$;
	END Store;

	PROCEDURE StoreKey* (IN key, mes: ARRAY OF CHAR);
		VAR
			len: INTEGER;
			element: MsgList;
	BEGIN
		NEW(element);
		element.next := msgList;
		msgList := element;
		len := LEN(key);
		NEW(element.key, len + 1);
		element.key^ := key$;
		len := LEN(mes);
		NEW(element.msg, len + 1);
		element.msg^ := mes$;
	END StoreKey;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		msgList := NIL;
		message := "";
		Maintainer
	END Init;

BEGIN
	Init;
END BugsMsg.

