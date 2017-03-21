(*		

	license:	"Docu/OpenBUGS-License"
	copyright:	"Rsrc/About"



	*)

MODULE BugsDialog;


	

	TYPE
		DialogBox* = POINTER TO ABSTRACT RECORD
			next: DialogBox
		END;

	VAR
		version-: INTEGER;
		maintainer-: ARRAY 40 OF CHAR;
		dialogBoxes: DialogBox;

	PROCEDURE (dialogBox: DialogBox) Init-, NEW, ABSTRACT;

	PROCEDURE (dialogBox: DialogBox) Update-, NEW, ABSTRACT;

	PROCEDURE InitDialogs*;
		VAR
			cursor: DialogBox;
	BEGIN
		cursor := dialogBoxes;
		WHILE cursor # NIL DO
			cursor.Init;
			cursor := cursor.next
		END
	END InitDialogs;

	PROCEDURE UpdateDialogs*;
		VAR
			cursor: DialogBox;
	BEGIN
		cursor := dialogBoxes;
		WHILE cursor # NIL DO
			cursor.Update;
			cursor := cursor.next
		END
	END UpdateDialogs;

	PROCEDURE AddDialog* (dialog: DialogBox);
	BEGIN
		dialog.next := dialogBoxes;
		dialogBoxes := dialog
	END AddDialog;

	PROCEDURE Maintainer;
	BEGIN
		version := 500;
		maintainer := "A.Thomas"
	END Maintainer;

	PROCEDURE Init;
	BEGIN
		Maintainer;
	END Init;

BEGIN
	Init
END BugsDialog.
