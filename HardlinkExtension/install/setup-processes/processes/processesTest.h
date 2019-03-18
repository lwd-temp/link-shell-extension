#pragma once




//-------------------------------------------------------------------------------------------
// Exported routines
#ifdef FINDPROCDLL_EXPORTS
#define PROCESS_API __declspec(dllexport)
#else
#define PROCESS_API __declspec(dllimport)
#endif

extern "C"

#if defined _DEBUG
PROCESS_API
void	
KillProcessTest( char* aPlattform );

PROCESS_API
void
VcRedistLevelTest( char* aPlattform );

PROCESS_API
void
GetOsVersionTest();

#endif

