
@set GIT_SRC_INDEX=..\..\tools\git_source_index.exe
@set GIT_SOURCE_SERVER=SOURCE_SERVER

@%GIT_SRC_INDEX% "..\..\Bin\x64\Release" /svc git /ev %GIT_SOURCE_SERVER%
@%GIT_SRC_INDEX% "..\..\Bin\win32\Release" /svc git /ev %GIT_SOURCE_SERVER%







