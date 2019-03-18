#pragma once

//-------------------------------------------------------------------------------------------
// Internal use routines

bool	FindProc( char *szProcess );
bool	KillProc( char *szProcess );
bool	Vcredist( char* aPlattform );
bool	CheckPlatt( char* aPlattform );
bool	ProcEnumModules( char* aPlattform, int ProcessListSize, char* ProcessList );
bool	_GetOsVersion( char* aOsVersion);


//-------------------------------------------------------------------------------------------
// Exported routines
extern "C" __declspec(dllexport) void	FindProcess( HWND		hwndParent, 
													 int		string_size,
													 char		*variables, 
													 stack_t	**stacktop );

extern "C" __declspec(dllexport) void	KillProcess( HWND		hwndParent, 
													 int		string_size,
													 char		*variables, 
													 stack_t	**stacktop );

extern "C" __declspec(dllexport) void	VcRedistLevel( HWND		hwndParent, 
													 int		string_size,
													 char		*variables, 
													 stack_t	**stacktop );

extern "C" __declspec(dllexport) void	CheckPlattform( HWND		hwndParent, 
													 int		string_size,
													 char		*variables, 
													 stack_t	**stacktop );

extern "C" __declspec(dllexport) void	ProcessEnumModules( HWND		hwndParent, 
													 int		string_size,
													 char		*variables, 
													 stack_t	**stacktop );

extern "C" __declspec(dllexport) void	GetOsVersion( HWND		hwndParent, 
													 int		string_size,
													 char		*variables, 
													 stack_t	**stacktop );

