@echo off

call ..\bat\settings.bat
set CONFIG=Release
REM set CONFIG=DEBUG

set LN_TEST_OPTIONS=--automated_test
REM
REM The test should be done with everything running under --traditional
REM
REM set LN_TEST_OPTIONS=--automated_traditional --automated_test

set LN=..\bin\%PLATT%\%CONFIG%\ln.exe %LN_TEST_OPTIONS%
set LNPURE=..\bin\%PLATT%\%CONFIG%\ln.exe

set GSAR=..\Tools\gsar.exe
set GREP=..\Tools\grep.exe
set LISTSTREAMS=..\Tools\ListStreams.exe

set TOUCH=..\Tools\touch.exe
set TIMESTAMP=..\Tools\timestamp.exe
set WHERE=..\Tools\where.exe
set RD=%LN% --quiet --deeppathdelete
set RMTSHARE=..\Tools\rmtshare.exe
set JSLINT=..\Tools\jslint.exe
set ATTR=..\Tools\timestamp --attr

set CACLS=cacls.exe

REM if [Win7] == [%OS%] (
set VSHADOW=..\Tools\vshadow64.exe
REM ) else (
REM   set VSHADOW=..\Tools\vshadow32xp.exe
REM )  
set NETSHAREDEL=..\Tools\netsharedel.exe


call t_DeepPathOff.bat

@REM set BASEDEEPPATH=\Gallia\est\omnis\divisa\in\partes\tres\quarum\unam\incolunt\Belgae\aliam\Aquitani\tertiam\qui\ipsorum\lingua\Celtae\nostra\Galli\appellantur\Hi\omnes\lingua\institutis\legibus\inter\se\differunt\Gallos\ab\Aquitanis\Garumna\flumen\a\Belgis\Matrona\et\Sequana\dividit\Horum\omnium\fortissimi\sunt\Belgae\propterea\quod\a\cultu\atque\humanitate\provinciae\longissime\absunt\minimeque\ad\eos\mercatores\saepe\commeant\atque\ea\quae\ad\effeminandos\animos\pertinent\important\proximique\sunt\Germanis\qui\trans\Rhenum\incolunt\quibuscum\continenter\bellum\gerunt.\Qua\de\causa\Helvetii\quoque\reliquos\Gallos\virtute\praecedunt\quod\fere\cotidianis\proeliis\cum\Germanis\contendunt\cum\aut\suis\finibus\eos\prohibent\aut\ipsi\in\eorum\finibus\bellum\gerunt
@set BASEDEEPPATH=\Gallia\est\omnis\divisa\in\partes\tres\quarum\unam\incolunt\Belgae\aliam\Aquitani\tertiam\qui\ipsorum\lingua\Celtae\nostra\Galli\appellantur\Hi\omnes\lingua\institutis\legibus\inter\se\differunt\Gallos\ab\Aquitanis\Garumna\flumen\a\Belgis\Matrona\et\Sequana\dividit\Horum\omnium\fortissimi\sunt

set EMPTYTESTDRIVELETTER=t
set EMPTYTESTDRIVE=%EMPTYTESTDRIVELETTER%:

set EMPTYTESTDRIVELETTER_UPPERCASE=T
set EMPTYTESTDRIVE_UPPERCASE=%EMPTYTESTDRIVELETTER_UPPERCASE%:

set EMPTYTESTDRIVELETTER1=q
set EMPTYTESTDRIVE1=%EMPTYTESTDRIVELETTER1%:

set EMPTYTESTDRIVELETTER2=p
set EMPTYTESTDRIVE2=%EMPTYTESTDRIVELETTER2%:


@echo on
