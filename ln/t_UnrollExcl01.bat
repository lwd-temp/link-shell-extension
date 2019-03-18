@echo off
REM
REM 1: directory

@set TESTROOT=%1
@set TESTROOTSRC=%TESTROOT%%DEEPPATH%\source
@set TESTROOTDST=%TESTROOT%%DEEPPATH%\dest
@set TESTROOTBKP=%TESTROOT%%DEEPPATH%\bk1

%MKDIR% %TESTROOTSRC%\FI
%MKDIR% %TESTROOTSRC%\FE
%MKDIR% %TESTROOTDST%
%MKDIR% %TESTROOTBKP%

REM 
REM Ich mache eine DeLorean Kopie meines User-Ordners "C:\Users\mad" mit der Option --unroll.
REM Dabei schließe ich bestimmte Unterordner aus mit -X "\AppData" usw.
REM Jetzt habe ich aber in einem Unterordner, der auch mitgesichert wird, einen SymLink auf 
REM einen Unterordner in "C:\Users\mad\AppData".
REM 
REM Wegen der --unroll Option hätte ich erwartet, dass der Inhalt hinter dem SymLink 
REM wieder angelegt wird, weil es sich ja um einen "Outer SymLink" handelt, weil ja \AppData 
REM ausgeschlossen wurde. Tatsächlich wird aber in der DeLorean Kopie ein SymLink erzeugt 
REM (vermutlich weil das Ziel unterhalb "C:\Users\mad" ist), der aber ungültig ist, weil 
REM er auf den Unterordner ....\AppData\... innerhalb der DeLorean Kopie verweist, 
REM der aber ja richtigerweise ausgeschlossen wurde.
REM
%COPY% test\ln.h %TESTROOTSRC%\FE\FI_D0 > nul
%LN% --symbolic --absolute %TESTROOTSRC%\FE\FI_D0 %TESTROOTSRC%\FI\FI_D0 > nul

REM
REM --unroll should override --exclude ( but currently does not )
REM
@echo on
%LN% --unroll -X FE --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
@sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOT% > nul
@echo on

