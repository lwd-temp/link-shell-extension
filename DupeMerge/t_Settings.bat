@echo off
call ..\bat\settings.bat
set CONFIG=Release
REM set CONFIG=DEBUG

set TEST_OPTIONS=--automated_test
REM set TEST_OPTIONS=--traditional --automated_test

REM set PLATT=win32
set LN=..\bin\%PLATT%\%CONFIG%\ln.exe %TEST_OPTIONS%
set DUPEMERGE=..\bin\%PLATT%\%CONFIG%\dupemerge.exe %TEST_OPTIONS%
REM set DUPEMERGE=..\..\hardlinks.3720\bin\%PLATT%\%CONFIG%\dupemerge.exe %TEST_OPTIONS%
REM set LN=E:\iBackup20110205\data\cpp\hardlinks\Bin\win32\release\ln.exe %LN_TEST_OPTIONS%

set TOUCH=..\Tools\touch.exe
set TIMESTAMP=..\Tools\timestamp.exe
set WHERE=..\Tools\where.exe
set RD=%LN% --quiet --deeppathdelete

@REM set BASEDEEPPATH=\Gallia\est\omnis\divisa\in\partes\tres\quarum\unam\incolunt\Belgae\aliam\Aquitani\tertiam\qui\ipsorum\lingua\Celtae\nostra\Galli\appellantur\Hi\omnes\lingua\institutis\legibus\inter\se\differunt\Gallos\ab\Aquitanis\Garumna\flumen\a\Belgis\Matrona\et\Sequana\dividit\Horum\omnium\fortissimi\sunt\Belgae\propterea\quod\a\cultu\atque\humanitate\provinciae\longissime\absunt\minimeque\ad\eos\mercatores\saepe\commeant\atque\ea\quae\ad\effeminandos\animos\pertinent\important\proximique\sunt\Germanis\qui\trans\Rhenum\incolunt\quibuscum\continenter\bellum\gerunt.\Qua\de\causa\Helvetii\quoque\reliquos\Gallos\virtute\praecedunt\quod\fere\cotidianis\proeliis\cum\Germanis\contendunt\cum\aut\suis\finibus\eos\prohibent\aut\ipsi\in\eorum\finibus\bellum\gerunt
@set BASEDEEPPATH=\Gallia\est\omnis\divisa\in\partes\tres\quarum\unam\incolunt\Belgae\aliam\Aquitani\tertiam\qui\ipsorum\lingua\Celtae\nostra\Galli\appellantur\Hi\omnes\lingua\institutis\legibus\inter\se\differunt\Gallos\ab\Aquitanis\Garumna\flumen\a\Belgis\Matrona\et\Sequana\dividit\Horum\omnium\fortissimi\sunt

set EMPTYTESTDRIVELETTER=t
set EMPTYTESTDRIVE=%EMPTYTESTDRIVELETTER%:

set EMPTYTESTDRIVELETTER_UPPERCASE=T
set EMPTYTESTDRIVE_UPPERCASE=%EMPTYTESTDRIVELETTER_UPPERCASE%:

@echo on
