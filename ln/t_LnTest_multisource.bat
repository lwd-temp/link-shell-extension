call t_Settings.bat

call t_IpAdress.bat
for /f %%f in ('type .ipadress') do set IP_ADRESS=%%f


REM
REM Unroll
REM
@echo off
call t_MultiSource01.bat test\MultiSourceUnroll unroll noitcnuj relative
call t_MultiSource01.bat test\MultiSourceUnroll unroll symbolic relative
call t_MultiSource01.bat test\MultiSourceUnroll unroll symbolic relative absolute
call t_MultiSource01.bat test\MultiSourceUnroll unroll symbolic absolute
call t_MultiSource01.bat test\MultiSourceUnroll unroll symbolic absolute absolute

REM
REM DeepPath Unroll
REM
call t_DeepPathOn.bat

call t_MultiSource01.bat test\MultiSourceUnroll unroll noitcnuj relative
call t_MultiSource01.bat test\MultiSourceUnroll unroll symbolic relative
call t_MultiSource01.bat test\MultiSourceUnroll unroll symbolic relative absolute
call t_MultiSource01.bat test\MultiSourceUnroll unroll symbolic absolute
call t_MultiSource01.bat test\MultiSourceUnroll unroll symbolic absolute absolute

call t_DeepPathOff.bat

REM
REM Splice
REM
@echo off
call t_MultiSource01.bat test\MultiSourceSplice splice noitcnuj relative
call t_MultiSource01.bat test\MultiSourceSplice splice symbolic relative
call t_MultiSource01.bat test\MultiSourceSplice splice symbolic relative absolute
call t_MultiSource01.bat test\MultiSourceSplice splice symbolic absolute
call t_MultiSource01.bat test\MultiSourceSplice splice symbolic absolute absolute

REM
REM DeepPath Splice
REM
call t_DeepPathOn.bat

call t_MultiSource01.bat test\DeepMultiSourceSplice splice noitcnuj relative
call t_MultiSource01.bat test\DeepMultiSourceSplice splice symbolic relative
call t_MultiSource01.bat test\DeepMultiSourceSplice splice symbolic relative absolute
call t_MultiSource01.bat test\DeepMultiSourceSplice splice symbolic absolute
call t_MultiSource01.bat test\DeepMultiSourceSplice splice symbolic absolute absolute

call t_DeepPathOff.bat


REM
REM SUCC: Command line tests
REM
REM mkdir c:\1
REM %LN%  -e c:\1\
REM %LN%  -e "c:\1"
REM %LN%  -e c:\1
REM %LN%  -e "c:\1\"

:ausmausraus

:nodel
@echo on

