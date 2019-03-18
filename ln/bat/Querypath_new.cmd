:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::                                                                           ::
:: Querypath.cmd Rev. 2.0                                                    ::
::                                                                           ::
:: Builded by Archimede                                                      ::
::                                                                           ::
:: Determines if a path is a File path, Directory Path, Link path and, in    ::
:: this case, the type of the link.                                          ::
::                                                                           ::
:: This program is based on ln.exe builded by Hermann Schinagl               ::
::                                                                           ::
:: Rev. 2                                                                    ::
:: - Add difference between File path and Hard Link path                     ::
:: - Add ERRORLEVEL code return                                              ::
:: - Add fine control if the path is between "                               ::
::                                                                           ::
:: Rev. 1                                                                    ::
:: Base version                                                              ::
::                                                                           ::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

@ECHO OFF

IF @%1@==@@ GOTO :HELP
IF @%1@==@/?@ GOTO :HELP

ECHO.

SET ARG1=%1

IF NOT EXIST %ARG1% GOTO :FILENOTFOUND

IF %ARG1:~0,1%%ARG1:~-1,1%=="" SET ARG1=%ARG1:~1,-1%

(ln -j "%ARG1%" | Find "->" && GOTO :JUNCTION) || (ln -s "%ARG1%" | Find "->" && GOTO :SYMBOLIC) || (ln -l "%ARG1%" | Find /I /V "%ARG1%" | Find "\">NUL && GOTO :HARDLINK) || (ln -l "%ARG1%" | Find "\" && GOTO :STANDARD) || GOTO :ERROR
 
:JUNCTION
ECHO.
ECHO Junction - Directory && EXIT /B 6

:SYMBOLIC
ECHO.
((DIR /AD "%ARG1%" | Find "<DIR>")>NUL && ECHO Symbolic link - Directory && EXIT /B 5)2>NUL
((DIR /A-D /B "%ARG1%")>NUL && ECHO Symbolic link - File && EXIT /B 4)2>NUL

:HARDLINK
ln -l "%ARG1%" | find "\"
ECHO.
ECHO Hardlink (multiple file) && EXIT /B 3)2>NUL

:STANDARD
ECHO.
((DIR /AD "%ARG1%" | Find "<DIR>")>NUL && ECHO Directory && EXIT /B 2)2>NUL
((DIR /A-D /B "%ARG1%")>NUL && ECHO File && EXIT /B 1)2>NUL

:FILENOTFOUND
ECHO %ARG1%
ECHO Path not found
EXIT /B -2

:ERROR
ECHO Error
EXIT /B -1

:HELP
ECHO.
ECHO Detect the type of the path passed as argument.
ECHO Requires ln.exe installed.
ECHO This command determines if the type of a Path is:
ECHO - File Name
ECHO - Directory Name
ECHO - Hardlink to much than 1 File
ECHO - Symbolic link to File
ECHO - Symbolic link to Directory
ECHO - Junction (only to Directory)
ECHO.
ECHO Return ERRORLEVEL:
ECHO -2 Path not found
ECHO -1 Error
ECHO 1 Path of File
ECHO 2 Path of Directory
ECHO 3 Path of Hard Link to much than 1 File
ECHO 4 Path of Symbolic Link to File
ECHO 5 Path of Symbolic Link to Directory
ECHO 6 Path of Junction
ECHO.
ECHO Syntax:
ECHO [Path]QueryPath ["]PathTest["]
ECHO [Path]QueryPath /?
ECHO.
ECHO PathTest: the path to know the type.
ECHO./?        Help
GOTO :EOF

