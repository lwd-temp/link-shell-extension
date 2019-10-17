
@set GIT_SRC_INDEX=%~dp0\..\tools\git_source_index.exe
@set GIT_SOURCE_SERVER=SOURCE_SERVER

@%GIT_SRC_INDEX% "%~dp0\..\Bin\x64\Release" /svc git /ev %GIT_SOURCE_SERVER%
@%GIT_SRC_INDEX% "%~dp0\..\Bin\win32\Release" /svc git /ev %GIT_SOURCE_SERVER%
