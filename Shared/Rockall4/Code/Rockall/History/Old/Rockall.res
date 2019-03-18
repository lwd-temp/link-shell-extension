#include <windows.h>
#include <winver.h>

VS_VERSION_INFO VERSIONINFO
FILEVERSION     3,01,3,0
PRODUCTVERSION  3,01,3,0
FILEOS          VOS_NT_WINDOWS32
FILETYPE        VFT_DLL

BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904B0"
        BEGIN
            VALUE "CompanyName",      "Microsoft Corporation"
            VALUE "ProductName",      "Rockall"
            VALUE "ProductVersion",   "3.01.3.0"
            VALUE "FileDescription",  "Rockall Heap Manager DLL"
            VALUE "FileVersion",      "3.01.3.0"
            VALUE "OriginalFilename", "RockallDLL.dll"
            VALUE "InternalName",     "Rockall"
            VALUE "LegalCopyright",   "Microsoft Corporation, All rights reserved."
        END
    END
    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x0409, 0x04B0
    END
END
