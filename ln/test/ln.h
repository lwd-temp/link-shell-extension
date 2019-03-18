

#define _A_SUBDIR 	0x10

void 
MakeAnsiString (
	WORD *unistring, 
	char *ansistring
);

int 
enableprivs(
);

void
ProbeCreateHardLink( 
);

DWORD 
CreateHardLinkIndep( 
	const PTCHAR	fromFile, 
	const PTCHAR	toFile 
);

DWORD 
CreateHardLinkNt4( 
	const TCHAR *fromFile, 
	const TCHAR *toFile 
);

int 
xln (
	PTCHAR aSrcPath, 
	PTCHAR aDestPath
);

PTCHAR
ResolveUNC (
	PTCHAR	src,
	PTCHAR	dest
);

int 
Usage(
);
