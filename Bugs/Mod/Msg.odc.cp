(*		

license:	"Docu/OpenBUGS-License"
copyright:	"Rsrc/About"



*)

MODULE BugsMsg;


	

	IMPORT
		Files, Strings,
		BugsFiles, BugsMappers;

	TYPE
		MsgList = POINTER TO RECORD
			key, msg: POINTER TO ARRAY OF CHAR;
			next: MsgList
		END;

	VAR
		message: ARRAY 1024 OF CHAR;
		msgList: MsgList;
		debug*: BOOLEAN;

		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;

	PROCEDURE GetError* (OUT msg: ARRAY OF CHAR);
	BEGIN
		msg := message$
	END GetError;

	PROCEDURE GetMsg* (OUT msg: ARRAY OF CHAR);
	BEGIN
		msg := message$
	END GetMsg;

	PROCEDURE LenMsg* (): INTEGER;
	BEGIN
		RETURN LEN(message)
	END LenMsg;

	PROCEDURE Map* (IN key, mes: ARRAY OF CHAR);
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
	END Map;

	PROCEDURE MapMsg* (IN key: ARRAY OF CHAR; OUT mes: ARRAY OF CHAR);
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
		END
	END MapMsg;

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

	PROCEDURE MapParamMsg* (IN key: ARRAY OF CHAR; IN p: ARRAY OF ARRAY OF CHAR;
	OUT mes: ARRAY OF CHAR);
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
			mes := cursor.msg$;
			i := 0;
			len := LEN(p);
			WHILE i < len DO
				Strings.IntToString(i, pat);
				pat := "^" + pat;
				pos := 0;
				WHILE pos #  - 1 DO
					Strings.Find(mes, pat, 0, pos);
					IF pos #  - 1 THEN
						Strings.Replace(mes, pos, LEN(pat$), p[i])
					END
				END;
				INC(i)
			END
		ELSE
			mes := key$
		END
	END MapParamMsg;

	PROCEDURE ParamMsg* (IN in: ARRAY OF CHAR; IN p: ARRAY OF ARRAY OF CHAR;
	OUT mes: ARRAY OF CHAR);
		VAR
			i, len, pos: INTEGER;
			pat: ARRAY 128 OF CHAR;
	BEGIN
		mes := in$;
		i := 0;
		len := LEN(p);
		WHILE i < len DO
			Strings.IntToString(i, pat);
			pat := "^" + pat;
			pos := 0;
			WHILE pos #  - 1 DO
				Strings.Find(mes, pat, 0, pos);
				IF pos #  - 1 THEN
					Strings.Replace(mes, pos, LEN(pat$), p[i])
				END
			END;
			INC(i)
		END
	END ParamMsg;

	PROCEDURE StoreError* (IN msg: ARRAY OF CHAR);
	BEGIN
		message := msg$;
		IF debug THEN HALT(0) END
	END StoreError;

	PROCEDURE StoreMsg* (IN msg: ARRAY OF CHAR);
	BEGIN
		message := msg$
	END StoreMsg;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		msgList := NIL;
		debug := FALSE;
		message := "";
		Maintainer
	END Init;

BEGIN
	Init;
END BugsMsg.

