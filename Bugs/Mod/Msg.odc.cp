(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsMsg;


	

	IMPORT
		Strings;

	TYPE
		MsgList = POINTER TO RECORD
			key, msg: POINTER TO ARRAY OF CHAR;
			next: MsgList
		END;

	VAR
		message-: ARRAY 1024 OF CHAR;
		error-: BOOLEAN;
		debug*: BOOLEAN; 
		
		msgList: MsgList;

		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE Clear*;
	BEGIN
		message := ""; error := FALSE
	END Clear;

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
		message := mes$
	END Lookup;

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
				WHILE pos # - 1 DO
					Strings.Find(msg, pat, 0, pos);
					IF pos # - 1 THEN
						Strings.Replace(msg, pos, LEN(pat$), p[i])
					END
				END;
				INC(i)
			END
		ELSE
			msg := key$
		END;
		message := msg$
	END LookupParam;

	PROCEDURE StoreError* (IN msg: ARRAY OF CHAR);
	BEGIN
		message := msg$;
		error := TRUE;
		IF debug THEN HALT(0) END
	END StoreError;

	PROCEDURE StoreMsg* (IN msg: ARRAY OF CHAR);
	BEGIN
		message := msg$
	END StoreMsg;

	PROCEDURE RegisterKey* (IN key, mes: ARRAY OF CHAR);
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
	END RegisterKey;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		msgList := NIL;
		message := "";
		error := FALSE;
		debug := FALSE;
		Maintainer
	END Init;

BEGIN
	Init;
END BugsMsg.

