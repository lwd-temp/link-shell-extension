!define UNINSTALLER_NAME "uninst-HardlinkShellExt_${PLATTFORM}.exe"
!define startmenu "$SMPROGRAMS\Link Shell Extension"
!include WriteEnvStr.nsh # or the name you chose
!define LANG_ENGLISH "1033"
!define LANG_CHINESE "2052"
!define LANG_CZECH "1029"
!define LANG_FRENCH "1036"
!define LANG_GERMAN "1031"
!define LANG_GREEK "1032"
!define LANG_ITALIAN "1040"
!define LANG_JAPANESE "1041"
!define LANG_KOREAN "1042"
!define LANG_POLISH "1045"
!define LANG_PORTUGUESE_BRAZIL "1046"
!define LANG_RUSSIAN "1049"
!define LANG_SLOVAK "1051"
!define LANG_SPANISH "3082"
!define LANG_SWEDISH "1053"
!define LANG_TURKISH "1055"
!define LANG_UKRAINIAN "1058"

!include "LSE_version.nsh"

Var WINDOWS_VERSION
Var COM_SERVER_PATH
Var REG_PREFIX_
Var NO_REDIST

Name "Link Shell Extension"

InstallDir $PROGRAMFILES\LinkShellExtension
CRCCheck on

SetCompressor /solid LZMA


; Registry key to check for directory (so if you install again, it will
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\LinkShellExtension" "Install_Dir"


!include "GetTime.nsh"
!include "GetWindowsVersion.nsh"

!include "WordFunc.nsh"
!insertmacro WordFind
!insertmacro WordReplace


Function InstPathPlattformCompliant
  StrCmp ${PLATTFORM} 'X64' lbl_ReplaceX86 lbl_preInstallDirEnd
  lbl_ReplaceX86:
	${WordReplace} $INSTDIR " (x86)" "" "+" $R0
	StrCpy $INSTDIR $R0
  lbl_preInstallDirEnd:
FunctionEnd


; ------------ Specify InstallDir -------
Function preInstallDir
	call InstPathPlattformCompliant
FunctionEnd

Function showInstallDir
FunctionEnd

Function leaveInstallDir
  CreateDirectory $INSTDIR
FunctionEnd

PageEx Directory
  DirText "Setup has determined the optimal location to install. If you would like to change the directory, do so now."
  DirVar $INSTDIR
  Caption ": Installation Location"
  PageCallbacks preInstallDir showInstallDir leaveInstallDir
  DirVerify leave
PageExEnd

; ------------ GetParameters ------- 
; GetParameters
; input, none
; output, top of stack (replaces, with e.g. whatever)
; modifies no other variables.
 
Function GetParameters
 
   Push $R0
   Push $R1
   Push $R2
   Push $R3
   
   StrCpy $R2 1
   StrLen $R3 $CMDLINE
   
   ;Check for quote or space
   StrCpy $R0 $CMDLINE $R2
   StrCmp $R0 '"' 0 +3
     StrCpy $R1 '"'
     Goto loop
   StrCpy $R1 " "
   
   loop:
     IntOp $R2 $R2 + 1
     StrCpy $R0 $CMDLINE 1 $R2
     StrCmp $R0 $R1 get
     StrCmp $R2 $R3 get
     Goto loop
   
   get:
     IntOp $R2 $R2 + 1
     StrCpy $R0 $CMDLINE 1 $R2
     StrCmp $R0 " " get
     StrCpy $R0 $CMDLINE "" $R2
   
   Pop $R3
   Pop $R2
   Pop $R1
   Exch $R0
 
FunctionEnd

; ------------ COMServerRegistration ------- 
; COMServerRegistration
Function COMServerRegistration

  ; Context Menue
  ;
  ; ClassID
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568AA}" "" "HardLink Context Menu"
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568AA}\InProcServer32" "" "$COM_SERVER_PATH"
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568AA}\InProcServer32" "ThreadingModel" "Apartment"
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568AA}\ProgID" "" "HardLink.HardLinkMenu.1"
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568AA}\VersionIndependentProgID" "" "HardLink.HardLinkMenu"
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568AA}" "AppID" "{60ADE2AD-3FEE-4eea-A599-311B656C6AA4}"

  ; ProgID
  WriteRegStr HKCR "$REG_PREFIX_HardLink.HardLinkMenu" "" "HardLink Class"
  WriteRegStr HKCR "$REG_PREFIX_HardLink.HardLinkMenu\CLSID" "" "{0A479751-02BC-11d3-A855-0004AC2568AA}"
  WriteRegStr HKCR "$REG_PREFIX_HardLink.HardLinkMenu\CurVer" "" "HardLink.HardLinkMenu.1"

  WriteRegStr HKCR "$REG_PREFIX_HardLink.HardLinkMenu.1" "" "HardLink Class"
  WriteRegStr HKCR "$REG_PREFIX_HardLink.HardLinkMenu.1\CLSID" "" "{0A479751-02BC-11d3-A855-0004AC2568AA}"

  WriteRegStr HKLM "$REG_PREFIX_Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved" "{0A479751-02BC-11d3-A855-0004AC2568AA}" "HardLink"

  ; Copy Hook
  ;
  ; ClassID
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568BB}" "" "HardLink Copyhook"
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568BB}\InProcServer32" "" "$COM_SERVER_PATH"
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568BB}\InProcServer32" "ThreadingModel" "Apartment"
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568BB}\ProgID" "" "HardLink.Copyhook.1"
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568BB}\VersionIndependentProgID" "" "HardLink.Copyhook"
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568BB}" "AppID" "{60ADE2AD-3FEE-4eea-A599-311B656C6AA4}"

  ; ProgID
  WriteRegStr HKCR "$REG_PREFIX_HardLink.Copyhook" "" "HardLink Copyhook Class"
  WriteRegStr HKCR "$REG_PREFIX_HardLink.Copyhook\CLSID" "" "{0A479751-02BC-11d3-A855-0004AC2568BB}"
  WriteRegStr HKCR "$REG_PREFIX_HardLink.Copyhook\CurVer" "" "HardLink.Copyhook.1"

  WriteRegStr HKCR "$REG_PREFIX_HardLink.Copyhook.1" "" "HardLink Copyhook Class"
  WriteRegStr HKCR "$REG_PREFIX_HardLink.Copyhook.1\CLSID" "" "{0A479751-02BC-11d3-A855-0004AC2568BB}"

  WriteRegStr HKLM "$REG_PREFIX_Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved" "{0A479751-02BC-11d3-A855-0004AC2568BB}" "HardLink Copyhook"

  ; PropertySheetPage
  ;
  ; ClassID
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568CC}" "" "HardLink PropertySheetPage"
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568CC}\InProcServer32" "" "$COM_SERVER_PATH"
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568CC}\InProcServer32" "ThreadingModel" "Apartment"
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568CC}\ProgID" "" "HardLink.PropertySheetPage.1"
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568CC}\VersionIndependentProgID" "" "HardLink.PropertySheetPage"
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568CC}" "AppID" "{60ADE2AD-3FEE-4eea-A599-311B656C6AA4}"

  ; ProgID
  WriteRegStr HKCR "$REG_PREFIX_HardLink.PropertySheetPage" "" "HardLink PropertySheetPage Class"
  WriteRegStr HKCR "$REG_PREFIX_HardLink.PropertySheetPage\CLSID" "" "{0A479751-02BC-11d3-A855-0004AC2568CC}"
  WriteRegStr HKCR "$REG_PREFIX_HardLink.PropertySheetPage\CurVer" "" "HardLink.PropertySheetPage.1"

  WriteRegStr HKCR "$REG_PREFIX_HardLink.PropertySheetPage.1" "" "HardLink PropertySheetPage Class"
  WriteRegStr HKCR "$REG_PREFIX_HardLink.PropertySheetPage.1\CLSID" "" "{0A479751-02BC-11d3-A855-0004AC2568CC}"

  WriteRegStr HKLM "$REG_PREFIX_Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved" "{0A479751-02BC-11d3-A855-0004AC2568CC}" "HardLink PropertySheetPage"

  ; Hardlink Overlay Icon
  ;
  ; ClassID
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568DD}" "" "HardLink IconOverlay Hardlink"
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568DD}\InProcServer32" "" "$COM_SERVER_PATH"
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568DD}\InProcServer32" "ThreadingModel" "Apartment"
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568DD}\ProgID" "" "HardLink.IconOverlayHardlink.1"
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568DD}\VersionIndependentProgID" "" "HardLink.IconOverlayHardlink"
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568DD}" "AppID" "{60ADE2AD-3FEE-4eea-A599-311B656C6AA4}"

  ; ProgID
  WriteRegStr HKCR "$REG_PREFIX_HardLink.IconOverlayHardlink" "" "HardLink IconOverlay Hardlink Class"
  WriteRegStr HKCR "$REG_PREFIX_HardLink.IconOverlayHardlink\CLSID" "" "{0A479751-02BC-11d3-A855-0004AC2568DD}"
  WriteRegStr HKCR "$REG_PREFIX_HardLink.IconOverlayHardlink\CurVer" "" "HardLink.IconOverlayHardlink.1"

  WriteRegStr HKCR "$REG_PREFIX_HardLink.IconOverlayHardlink.1" "" "HardLink IconOverlay Hardlink Class"
  WriteRegStr HKCR "$REG_PREFIX_HardLink.IconOverlayHardlink.1\CLSID" "" "{0A479751-02BC-11d3-A855-0004AC2568DD}"

  WriteRegStr HKLM "$REG_PREFIX_Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved" "{0A479751-02BC-11d3-A855-0004AC2568DD}" "HardLink IconOverlay Hardlink"
  WriteRegStr HKLM "$REG_PREFIX_Software\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers\IconOverlayHardLink" "" "{0A479751-02BC-11d3-A855-0004AC2568DD}"

  ; Junction Overlay Icon
  ;
  ; ClassID
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568FF}" "" "HardLink IconOverlay Junction"
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568FF}\InProcServer32" "" "$COM_SERVER_PATH"
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568FF}\InProcServer32" "ThreadingModel" "Apartment"
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568FF}\ProgID" "" "HardLink.IconOverlayJunction.1"
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568FF}\VersionIndependentProgID" "" "HardLink.IconOverlayJunction"
  WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568FF}" "AppID" "{60ADE2AD-3FEE-4eea-A599-311B656C6AA4}"

  ; ProgID
  WriteRegStr HKCR "$REG_PREFIX_HardLink.IconOverlayJunction" "" "HardLink IconOverlay Junction Class"
  WriteRegStr HKCR "$REG_PREFIX_HardLink.IconOverlayJunction\CLSID" "" "{0A479751-02BC-11d3-A855-0004AC2568FF}"
  WriteRegStr HKCR "$REG_PREFIX_HardLink.IconOverlayJunction\CurVer" "" "HardLink.IconOverlayJunction.1"

  WriteRegStr HKCR "$REG_PREFIX_HardLink.IconOverlayJunction.1" "" "HardLink IconOverlay Junction Class"
  WriteRegStr HKCR "$REG_PREFIX_HardLink.IconOverlayJunction.1\CLSID" "" "{0A479751-02BC-11d3-A855-0004AC2568FF}"

  WriteRegStr HKLM "$REG_PREFIX_Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved" "{0A479751-02BC-11d3-A855-0004AC2568FF}" "HardLink IconOverlay Junction"
  WriteRegStr HKLM "$REG_PREFIX_Software\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers\IconOverlayJunction" "" "{0A479751-02BC-11d3-A855-0004AC2568FF}"

  ; AppID
  ;
  WriteRegStr HKCR "$REG_PREFIX_AppID\{60ADE2AD-3FEE-4eea-A599-311B656C6AA4}" "" "HardLinkMenu"
  WriteRegStr HKCR "$REG_PREFIX_AppID\HardlinkMenu.DLL" "" ""
  WriteRegStr HKCR "$REG_PREFIX_AppID\HardlinkMenu.DLL" "AppID" "{60ADE2AD-3FEE-4eea-A599-311B656C6AA4}"


  ;
  ; Shell Epxlorer Registration
  ;
  WriteRegStr HKCR "$REG_PREFIX_*\shellex\ContextMenuHandlers\HardLinkMenu" "" "{0A479751-02BC-11d3-A855-0004AC2568AA}"
  WriteRegStr HKCR "$REG_PREFIX_Folder\shellex\ContextMenuHandlers\HardLinkMenu" "" "{0A479751-02BC-11d3-A855-0004AC2568AA}"
  WriteRegStr HKCR "$REG_PREFIX_Directory\shellex\DragDropHandlers\HardLinkMenu" "" "{0A479751-02BC-11d3-A855-0004AC2568AA}"
  WriteRegStr HKCR "$REG_PREFIX_Drive\shellex\DragDropHandlers\HardLinkMenu" "" "{0A479751-02BC-11d3-A855-0004AC2568AA}"
  WriteRegStr HKCR "$REG_PREFIX_Directory\Background\ShellEx\ContextMenuHandlers\HardLinkMenu" "" "{0A479751-02BC-11d3-A855-0004AC2568AA}"
  WriteRegStr HKCR "$REG_PREFIX_LibraryFolder\Background\ShellEx\ContextMenuHandlers\HardLinkMenu" "" "{0A479751-02BC-11d3-A855-0004AC2568AA}"

  ; lnkfile
  WriteRegStr HKCR "$REG_PREFIX_lnkfile\ShellEx\ContextMenuHandlers\HardLinkMenu" "" "{0A479751-02BC-11d3-A855-0004AC2568AA}"

  StrCmp $WINDOWS_VERSION 'Vista' lbl_FolderVista_inst
  StrCmp $WINDOWS_VERSION 'Windows7' lbl_FolderVista_inst
  StrCmp $WINDOWS_VERSION 'Windows8' lbl_FolderVista_inst
  StrCmp $WINDOWS_VERSION 'Windows8.1' lbl_FolderVista_inst
  StrCmp $WINDOWS_VERSION 'Windows10_9826' lbl_FolderVista_inst
  StrCmp $WINDOWS_VERSION 'Windows10' lbl_FolderVista_inst

   ;
   ; Special Folders for XP, W2K ...
   ;
   ; My Documents 
   WriteRegStr HKCR "$REG_PREFIX_CLSID\{450d8fba-ad25-11d0-98a8-0800361b1103}\ShellEx\DragDropHandlers\HardLinkMenu" "" "{0A479751-02BC-11d3-A855-0004AC2568AA}"
   goto lbl_Common01_inst

  lbl_FolderVista_inst:
   ;
   ; Special Folders for Vista, Windows7, Windows8, Windows10 only
   ;
   ; Public Folder 
   WriteRegStr HKCR "$REG_PREFIX_CLSID\{4336a54d-038b-4685-ab02-99bb52d3fb8b}\ShellEx\DragDropHandlers\HardLinkMenu" "" "{0A479751-02BC-11d3-A855-0004AC2568AA}"

   ; Users Folder
   WriteRegStr HKCR "$REG_PREFIX_CLSID\{59031a47-3f72-44a7-89c5-5595fe6b30ee}\ShellEx\DragDropHandlers\HardLinkMenu" "" "{0A479751-02BC-11d3-A855-0004AC2568AA}"


  lbl_Common01_inst:

   ; SymbolicLink Overlay are also installed on XP, because there is a driver which enables XP to create symlinks
   ;
	 ; ClassID
	 WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568EE}" "" "HardLink IconOverlay SymbolicLink"
	 WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568EE}\InProcServer32" "" "$COM_SERVER_PATH"
	 WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568EE}\InProcServer32" "ThreadingModel" "Apartment"
	 WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568EE}\ProgID" "" "HardLink.IconOverlaySymbolicLink.1"
	 WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568EE}\VersionIndependentProgID" "" "HardLink.IconOverlaySymbolicLink"
	 WriteRegStr HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568EE}" "AppID" "{60ADE2AD-3FEE-4eea-A599-311B656C6AA4}"

	 ; ProgID
	 WriteRegStr HKCR "$REG_PREFIX_HardLink.IconOverlaySymbolicLink" "" "HardLink IconOverlay SymbolicLink Class"
	 WriteRegStr HKCR "$REG_PREFIX_HardLink.IconOverlaySymbolicLink\CLSID" "" "{0A479751-02BC-11d3-A855-0004AC2568EE}"
	 WriteRegStr HKCR "$REG_PREFIX_HardLink.IconOverlaySymbolicLink\CurVer" "" "HardLink.IconOverlaySymbolicLink.1" 

 	 WriteRegStr HKCR "$REG_PREFIX_HardLink.IconOverlaySymbolicLink.1" "" "HardLink IconOverlay SymbolicLink Class" 
	 WriteRegStr HKCR "$REG_PREFIX_HardLink.IconOverlaySymbolicLink.1\CLSID" "" "{0A479751-02BC-11d3-A855-0004AC2568EE}"

	 WriteRegStr HKLM "$REG_PREFIX_Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved" "{0A479751-02BC-11d3-A855-0004AC2568EE}" "HardLink IconOverlay SymbolicLink"
   WriteRegStr HKLM "$REG_PREFIX_Software\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers\IconOverlaySymbolicLink" "" "{0A479751-02BC-11d3-A855-0004AC2568EE}"



  WriteRegStr HKCR "$REG_PREFIX_Folder\shellex\ColumnHandlers\{0A479751-02BC-11d3-A855-0004AC2568AA}" "" "HardLink Reference Count"

  WriteRegStr HKCR "$REG_PREFIX_Directory\shellex\CopyHookHandlers\HardLinkMenu" "" "{0A479751-02BC-11d3-A855-0004AC2568BB}"
  
  WriteRegStr HKCR "$REG_PREFIX_*\shellex\PropertySheetHandlers\HardLinkMenu" "" "{0A479751-02BC-11d3-A855-0004AC2568CC}"
  WriteRegStr HKCR "$REG_PREFIX_Folder\shellex\PropertySheetHandlers\HardLinkMenu" "" "{0A479751-02BC-11d3-A855-0004AC2568CC}"

  ; Write Overlay Icon Priority
  ;
  WriteRegDWORD HKLM "$REG_PREFIX_Software\LinkShellExtension" "HardlinkOverlayPrio" 0
  WriteRegDWORD HKLM "$REG_PREFIX_Software\LinkShellExtension" "JunctionOverlayPrio" 0
  WriteRegDWORD HKLM "$REG_PREFIX_Software\LinkShellExtension" "SymboliclinkOverlayPrio" 0

  ; Check which icon to install
  ;
  StrCmp $WINDOWS_VERSION 'Windows10' lbl_Win10Icon 
  
  lbl_VistaIcon:
  ;
  ; WindowsXP, Vista, Windows7, Windows8
  ;
  WriteRegStr HKLM "$REG_PREFIX_SOFTWARE\LinkShellExtension" "Hardlink Icon" "$INSTDIR\Hardlink_Vista.ico"
  WriteRegStr HKLM "$REG_PREFIX_SOFTWARE\LinkShellExtension" "Junction Icon" "$INSTDIR\JunctionIcon_Vista.ico"
  WriteRegStr HKLM "$REG_PREFIX_SOFTWARE\LinkShellExtension" "SymbolicLink Icon" "$INSTDIR\SymbolicLink_Vista.ico"
  goto lbl_IconEnd
  
  lbl_Win10Icon:
  ;
  ; Windows10
  ;
  WriteRegStr HKLM "$REG_PREFIX_SOFTWARE\LinkShellExtension" "Hardlink Icon" "$INSTDIR\Hardlink_Win10.ico"
  WriteRegStr HKLM "$REG_PREFIX_SOFTWARE\LinkShellExtension" "Junction Icon" "$INSTDIR\JunctionIcon_Win10.ico"
  WriteRegStr HKLM "$REG_PREFIX_SOFTWARE\LinkShellExtension" "SymbolicLink Icon" "$INSTDIR\SymbolicLink_Win10.ico"

  lbl_IconEnd:  
  ;  0x0400 = 1024. Junctions on, Mirror on, Delorean on, Relative Links
  ;
  WriteRegDWORD HKLM "$REG_PREFIX_Software\LinkShellExtension" "gFlags" 1024

  ;  Add third party filesystems
  ;
  WriteRegStr HKLM "$REG_PREFIX_Software\LinkShellExtension" "ThirdPartyFileSystems" "btrfs;"

FunctionEnd

Function un.COMServerCleanup
  ;
  ; Shell Epxlorer Deregistration
  ;
  DeleteRegKey HKCR "$REG_PREFIX_*\shellex\ContextMenuHandlers\HardLinkMenu"
  DeleteRegKey HKCR "$REG_PREFIX_Folder\shellex\ContextMenuHandlers\HardLinkMenu"
  DeleteRegKey HKCR "$REG_PREFIX_Directory\shellex\DragDropHandlers\HardLinkMenu"
  DeleteRegKey HKCR "$REG_PREFIX_Drive\shellex\DragDropHandlers\HardLinkMenu"
  DeleteRegKey HKCR "$REG_PREFIX_Directory\Background\ShellEx\ContextMenuHandlers\HardLinkMenu"
  DeleteRegKey HKCR "$REG_PREFIX_LibraryFolder\Background\ShellEx\ContextMenuHandlers\HardLinkMenu"

  ; lnkfile
  DeleteRegKey HKCR "$REG_PREFIX_lnkfile\ShellEx\ContextMenuHandlers\HardLinkMenu"

  ; My Documents
  StrCmp $WINDOWS_VERSION 'Vista' lbl_FolderVista_deinst
  StrCmp $WINDOWS_VERSION 'Windows7' lbl_FolderVista_deinst
  StrCmp $WINDOWS_VERSION 'Windows8' lbl_FolderVista_deinst
  StrCmp $WINDOWS_VERSION 'Windows8.1' lbl_FolderVista_deinst
  StrCmp $WINDOWS_VERSION 'Windows10_9826' lbl_FolderVista_deinst
  StrCmp $WINDOWS_VERSION 'Windows10' lbl_FolderVista_deinst
     ;
     ; (XP, W2K ...)
     ;
     ; My Documents 
     DeleteRegKey HKCR "$REG_PREFIX_CLSID\{450d8fba-ad25-11d0-98a8-0800361b1103}\ShellEx\DragDropHandlers\HardLinkMenu"
     goto lbl_Common01_deinst

  lbl_FolderVista_deinst:
     ;
     ; Vista, Windows7, Windows8, Windows10 only
     ;
     ; Public Folder 
     DeleteRegKey HKCR "$REG_PREFIX_CLSID\{4336a54d-038b-4685-ab02-99bb52d3fb8b}\ShellEx\DragDropHandlers\HardLinkMenu"

     ; Users Folder
     DeleteRegKey HKCR "$REG_PREFIX_CLSID\{59031a47-3f72-44a7-89c5-5595fe6b30ee}\ShellEx\DragDropHandlers\HardLinkMenu"

     ; SymbolicLink Overlay
     DeleteRegKey HKLM "$REG_PREFIX_Software\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers\IconOverlaySymbolicLink" 

	   ; Icon Overlay SymbolicLink
	   ;
	   DeleteRegKey HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568EE}"
	   DeleteRegKey HKCR "$REG_PREFIX_HardLink.IconOverlaySymbolicLink"
	   DeleteRegKey HKCR "$REG_PREFIX_HardLink.IconOverlaySymbolicLink.1"
	   DeleteRegValue HKLM "$REG_PREFIX_Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved" "{0A479751-02BC-11d3-A855-0004AC2568EE}"

  lbl_Common01_deinst:

  DeleteRegKey HKCR "$REG_PREFIX_Folder\shellex\ColumnHandlers\{0A479751-02BC-11d3-A855-0004AC2568AA}"
  DeleteRegKey HKCR "$REG_PREFIX_Directory\shellex\CopyHookHandlers\HardLinkMenu"
  DeleteRegKey HKLM "$REG_PREFIX_Software\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers\IconOverlayJunction"
  DeleteRegKey HKLM "$REG_PREFIX_Software\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers\IconOverlayHardLink"
  DeleteRegKey HKCR "$REG_PREFIX_*\shellex\PropertySheetHandlers\HardLinkMenu"
  DeleteRegKey HKCR "$REG_PREFIX_Folder\shellex\PropertySheetHandlers\HardLinkMenu"


  ; Context Menue
  ;
  DeleteRegKey HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568AA}"
  DeleteRegKey HKCR "$REG_PREFIX_HardLink.HardLinkMenu"
  DeleteRegKey HKCR "$REG_PREFIX_HardLink.HardLinkMenu.1"
  DeleteRegValue HKLM "$REG_PREFIX_Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved" "{0A479751-02BC-11d3-A855-0004AC2568AA}"

  ; Copy Hook
  ;
  DeleteRegKey HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568BB}"
  DeleteRegKey HKCR "$REG_PREFIX_HardLink.Copyhook"
  DeleteRegKey HKCR "$REG_PREFIX_HardLink.Copyhook.1"
  DeleteRegValue HKLM "$REG_PREFIX_Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved" "{0A479751-02BC-11d3-A855-0004AC2568BB}"

  ; PropertySheetPage
  ;
  DeleteRegKey HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568CC}"
  DeleteRegKey HKCR "$REG_PREFIX_HardLink.PropertySheetPage"
  DeleteRegKey HKCR "$REG_PREFIX_HardLink.PropertySheetPage.1"
  DeleteRegValue HKLM "$REG_PREFIX_Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved" "{0A479751-02BC-11d3-A855-0004AC2568CC}"

  ; Icon Overlay Hardlink
  ;
  DeleteRegKey HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568DD}"
  DeleteRegKey HKCR "$REG_PREFIX_HardLink.IconOverlayHardlink"
  DeleteRegKey HKCR "$REG_PREFIX_HardLink.IconOverlayHardlink.1"
  DeleteRegValue HKLM "$REG_PREFIX_Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved" "{0A479751-02BC-11d3-A855-0004AC2568DD}"

  ; Icon Overlay Junction
  ;
  DeleteRegKey HKCR "$REG_PREFIX_CLSID\{0A479751-02BC-11d3-A855-0004AC2568FF}"
  DeleteRegKey HKCR "$REG_PREFIX_HardLink.IconOverlayJunction"
  DeleteRegKey HKCR "$REG_PREFIX_HardLink.IconOverlayJunction.1"
  DeleteRegValue HKLM "$REG_PREFIX_Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved" "{0A479751-02BC-11d3-A855-0004AC2568FF}"

  ; AppID
  ;
  DeleteRegKey HKCR "$REG_PREFIX_AppID\{60ADE2AD-3FEE-4eea-A599-311B656C6AA4}"
  DeleteRegKey HKCR "$REG_PREFIX_AppID\HardlinkMenu.DLL"

  ; Delete all LSE specifica settings at once
  ;
  DeleteRegKey HKLM "$REG_PREFIX_Software\LinkShellExtension" 

FunctionEnd


; ------------ .onInit ------- 
; .onInit
Function .onInit
    IfSilent 0 +2
	  call InstPathPlattformCompliant
    
	Call GetParameters
	Pop $2
	
	# Check /LANGUAGE
	#
  ; MessageBox MB_OK "Arguments '$2', "
	StrCpy $1 '"'
	Push $2
	Push '"/LANGUAGE='
	Call StrStr
	Pop $0
	StrCpy $0 $0 "" 1 # skip quote
	StrCmp $0 "" "" cmdpar_lang_next
	# search for non quoted /LANGUAGE
	StrCpy $1 ' '             
	Push $2
	Push '/LANGUAGE='
	Call StrStr
	Pop $0
  cmdpar_lang_next:
	StrCmp $0 "" cmdpar_lang_done
	# copy the value after /LANGUAGE=
	StrCpy $0 $0 "" 10
	# search for the next parameter
	Push $0
	Push $1
	Call StrStr
	Pop $1
	StrCmp $1 "" cmdpar_lang_done
	StrLen $1 $1
	StrCpy $0 $0 -$1
  cmdpar_lang_done:
	StrCpy $LANGUAGE $0

  # Check for /noredist
  #
  StrCpy $NO_REDIST "0"
	Push $2
	Push '/noredist'
	Call StrStr
	Pop $0
	StrCmp $0 '' lbl_redist_check 0
  ; MessageBox MB_OK "Redist: '$0'"
  StrCpy $NO_REDIST "1"
	lbl_redist_check:
	

  # Perform redist check
  #
	StrCmp ${PLATTFORM} 'vc6' lbl_NoPlatRedistCheck
	StrCmp ${PLATTFORM} 'win32x64' lbl_NoPlatRedistCheck

	# Check for the correct plattform
	#
	Processes::CheckPlattform ${PLATTFORM};
	Pop $R0
	StrCmp $R0 "1" lbl_plattform_ok
	  MessageBox MB_OK "This version of Link Shell Extension is not compatible with this plattform. $\r$\nPlease download the properversion from$\r$\n https://schinagl.priv.at/nt/hardlinkshellext/linkshellextension.html#download";
	  Abort;
	lbl_plattform_ok:


	# Check for the Servicpack	
	#
	lbl_NoRedistCheck:

	Call GetWindowsVersion
	Pop $R0
	StrCpy $WINDOWS_VERSION $R0
	; MessageBox MB_OK "Windows Version '$WINDOWS_VERSION'"

	# Check if prerequisites are available
	lbl_redistlevel:
	Processes::VcRedistLevel ${PLATTFORM};
	Pop $R0
	StrCmp $R0 "1" lbl_redistlevel_ok
	StrCmp $NO_REDIST "1" lbl_redistlevel_ok
	  MessageBox MB_OK "The VS2017 Sp1 Redistributable Package is a prerequiste for Link Shell Extension to work on this plattform.$\r$\n ${SP1_DOWNLOAD}";
	  Abort;
	lbl_redistlevel_ok:
	
	lbl_NoPlatRedistCheck:
	
	IfSilent NoLangSelection

	;Language selection dialog
	Push ""
	Push ${LANG_ENGLISH}
	Push English
	Push ${LANG_CHINESE}
	Push Chinese
	Push ${LANG_CZECH}
	Push Czech
	Push ${LANG_FRENCH}
	Push French
	Push ${LANG_GERMAN}
	Push German
	Push ${LANG_GREEK}
	Push Greek
	Push ${LANG_ITALIAN}
	Push Italian
	Push ${LANG_JAPANESE}
	Push Japanese
	Push ${LANG_KOREAN}
	Push Korean
	Push ${LANG_POLISH}
	Push Polish
	Push ${LANG_PORTUGUESE_BRAZIL}
	Push "Portuguese Brazil"
	Push ${LANG_RUSSIAN}
	Push Russian
	Push ${LANG_SLOVAK}
	Push Slovak
	Push ${LANG_SPANISH}
	Push Spanish
	Push ${LANG_SWEDISH}
	Push Swedish
	Push ${LANG_TURKISH}
	Push Turkish
	Push ${LANG_UKRAINIAN}
	Push Ukrainian
	

	Push A ; A means auto count languages
	       ; for the auto count to work the first empty push (Push "") must remain
	LangDLL::LangDialog "Link Shell Extension Language" "Please select the language of Link Shell Extension ${LSE_VERSION}"

	Pop $LANGUAGE

  NoLangSelection:

	StrCmp $LANGUAGE ${LANG_ENGLISH} 0 +2
	WriteRegDWORD HKEY_CURRENT_USER "Software\LinkShellExtension" "Language" 1033

	StrCmp $LANGUAGE ${LANG_FRENCH} 0 +2
	WriteRegDWORD HKEY_CURRENT_USER "Software\LinkShellExtension" "Language" 1036

	StrCmp $LANGUAGE ${LANG_GERMAN} 0 +2
	WriteRegDWORD HKEY_CURRENT_USER "Software\LinkShellExtension" "Language" 1031

	StrCmp $LANGUAGE ${LANG_GREEK} 0 +2
	WriteRegDWORD HKEY_CURRENT_USER "Software\LinkShellExtension" "Language" 1032

	StrCmp $LANGUAGE ${LANG_ITALIAN} 0 +2
	WriteRegDWORD HKEY_CURRENT_USER "Software\LinkShellExtension" "Language" 1040

	StrCmp $LANGUAGE ${LANG_SPANISH} 0 +2
	WriteRegDWORD HKEY_CURRENT_USER "Software\LinkShellExtension" "Language" 3082

	StrCmp $LANGUAGE ${LANG_SWEDISH} 0 +2
	WriteRegDWORD HKEY_CURRENT_USER "Software\LinkShellExtension" "Language" 1053

	StrCmp $LANGUAGE ${LANG_CHINESE} 0 +2
	WriteRegDWORD HKEY_CURRENT_USER "Software\LinkShellExtension" "Language" 2052

	StrCmp $LANGUAGE ${LANG_RUSSIAN} 0 +2
	WriteRegDWORD HKEY_CURRENT_USER "Software\LinkShellExtension" "Language" 1049

	StrCmp $LANGUAGE ${LANG_JAPANESE} 0 +2
	WriteRegDWORD HKEY_CURRENT_USER "Software\LinkShellExtension" "Language" 1041

	StrCmp $LANGUAGE ${LANG_KOREAN} 0 +2
	WriteRegDWORD HKEY_CURRENT_USER "Software\LinkShellExtension" "Language" 1042

	StrCmp $LANGUAGE ${LANG_PORTUGUESE_BRAZIL} 0 +2
	WriteRegDWORD HKEY_CURRENT_USER "Software\LinkShellExtension" "Language" 1046

	StrCmp $LANGUAGE ${LANG_POLISH} 0 +2
	WriteRegDWORD HKEY_CURRENT_USER "Software\LinkShellExtension" "Language" 1045

	StrCmp $LANGUAGE ${LANG_TURKISH} 0 +2
	WriteRegDWORD HKEY_CURRENT_USER "Software\LinkShellExtension" "Language" 1055

	StrCmp $LANGUAGE ${LANG_CZECH} 0 +2
	WriteRegDWORD HKEY_CURRENT_USER "Software\LinkShellExtension" "Language" 1029

	StrCmp $LANGUAGE ${LANG_SLOVAK} 0 +2
	WriteRegDWORD HKEY_CURRENT_USER "Software\LinkShellExtension" "Language" 1051

	StrCmp $LANGUAGE ${LANG_UKRAINIAN} 0 +2
	WriteRegDWORD HKEY_CURRENT_USER "Software\LinkShellExtension" "Language" 1058

	StrCmp $LANGUAGE "cancel" 0 +2
		Abort

	; Check for running processes
	;
	lbl_ProcEnumModules_retry:
	Processes::ProcessEnumModules ${PLATTFORM};
	Pop $R0
	Pop $R1
	StrCmp $R0 "0" lbl_ProcEnumModules_ok
	  MessageBox MB_ABORTRETRYIGNORE|MB_ICONEXCLAMATION "Please close the following processes before installation:$\r$\n$\r$\n$R1" /SD IDIGNORE IDRETRY lbl_ProcEnumModules_retry IDIGNORE lbl_ProcEnumModules_ok 
	  Abort
	  
	lbl_ProcEnumModules_ok:

FunctionEnd


UninstPage uninstConfirm
UninstPage instfiles


Page Instfiles

;--------------------------------

; The stuff to install
Section

  ;
  ; Get the windows version
  ;
  Call GetWindowsVersion
  Pop $R0
  StrCpy $WINDOWS_VERSION $R0
  ; MessageBox MB_OK "Windows Version '$WINDOWS_VERSION'"

  ;
  ; <COM_Server_Registration>
  ;

  StrCpy $COM_SERVER_PATH $INSTDIR\HardlinkShellExt.dll
  StrCpy $REG_PREFIX_ ""
  Call COMServerRegistration
  
  !ifdef WIN32_REG_PREFIX_
    StrCpy $COM_SERVER_PATH $INSTDIR\32\HardlinkShellExt.dll
    StrCpy $REG_PREFIX_ ${WIN32_REG_PREFIX_}
    Call COMServerRegistration
  !endif
    
  ;
  ; Write the installation path into the registry
  ;
  WriteRegStr HKLM SOFTWARE\LinkShellExtension "Install_Dir" "$INSTDIR"

  CreateDirectory "${startmenu}"
  CreateShortCut "${startmenu}\Documentation.lnk" "$INSTDIR\Doc\linkshellextension.html"
  CreateShortCut "${startmenu}\LSEConfig.lnk" "$INSTDIR\LSEConfig.exe"
  CreateShortCut "${startmenu}\Donate.lnk" "http://schinagl.priv.at/nt/hardlinkshellext/linkshellextension.html#contact"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HardlinkShellExt" "DisplayName" "Link Shell Extension"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HardlinkShellExt" "DisplayIcon" "$INSTDIR\Hardlink_Vista.ico"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HardlinkShellExt" "Publisher" "Hermann Schinagl"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HardlinkShellExt" "DisplayVersion" "${LSE_VERSION}"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HardlinkShellExt" "EstimatedSize" 15000
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HardlinkShellExt" "HelpLink" "http://schinagl.priv.at/nt/hardlinkshellext/linkshellextension.html"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HardlinkShellExt" "UrlInfoAbout" "http://schinagl.priv.at/nt/hardlinkshellext/linkshellextension.html"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HardlinkShellExt" "UrlUpdateInfo" "http://schinagl.priv.at/nt/hardlinkshellext/linkshellextension.html"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\HardlinkShellExt" "UninstallString" '"$INSTDIR\${UNINSTALLER_NAME}"'
  WriteUninstaller "${UNINSTALLER_NAME}"

  IfSilent lbl_killitsilent

  MessageBox MB_YESNO "To make installation active Explorer.exe must be restarted.$\r$\n$\r$\n\
				       Press Yes to restart or No.$\r$\n" \
                       IDNO lbl_dontkill

  lbl_killitsilent:
  Processes::KillProcess "explorer.exe"
  Pop $R0
  
  lbl_dontkill:
  SetOverwrite on

  ; Rename a already existing .dll to something different
  ;
  ${GetTime} "" "L" $0 $1 $2 $3 $4 $5 $6
  Rename $INSTDIR\HardlinkShellExt.dll $INSTDIR\HardlinkShellExt.dll.$2$1$0$4$5$6
  
  ; If in 64bit mode add the 32bit dll too
  ;
  !ifdef WIN32_REG_PREFIX_
    Rename $INSTDIR\32\HardlinkShellExt.dll $INSTDIR\32\HardlinkShellExt.dll.$2$1$0$4$5$6

    SetOutPath $INSTDIR\32
    File ..\..\bin\${PLATTFORM_ALTERNATIVE}\${CONFIGURATION}\HardlinkShellExt.dll
    File ..\..\bin\${PLATTFORM_ALTERNATIVE}\${CONFIGURATION}\LSEUacHelper.exe
  !endif

  ; go on with the normal part
  ;
  SetOutPath $INSTDIR
  File ..\..\bin\${PLATTFORM}\${CONFIGURATION}\HardlinkShellExt.dll
  File ..\..\bin\${PLATTFORM}\${CONFIGURATION}\LSEConfig.exe

  ; We need an extra LSEUacHelper.exe with Windows7 to perform the symlinks
    File ..\..\bin\${PLATTFORM}\${CONFIGURATION}\LSEUacHelper.exe
    File ..\res\Hardlink_Vista.ico
    File ..\res\JunctionIcon_Vista.ico
    File ..\res\SymbolicLink_Vista.ico
    File ..\res\Hardlink_Win10.ico
    File ..\res\JunctionIcon_Win10.ico
    File ..\res\SymbolicLink_Win10.ico


  File ..\License.txt

  SetOutPath $INSTDIR\Doc
  File ..\Doc\autorename.png
  File ..\Doc\autorenamemultpiple.png
  File ..\Doc\columnsel.png
  File ..\Doc\draghardlink.png
  File ..\Doc\hardlinkclonedrag.png
  File ..\Doc\junctiondrag.png
  File ..\Doc\drophardlinkbackground.png
  File ..\Doc\hardlinkclonefolderdrop.png
  File ..\Doc\hardlinkcloneshowto.png
  File ..\Doc\drophardlinkfolder.png
  File ..\Doc\junctiondrop.png
  File ..\Doc\linkshellext.png
  File ..\Doc\junctionorigin.png
  File ..\Doc\junctionoverlay.png
  File ..\Doc\linkproperties.png
  File ..\Doc\picklinksource.png
  File ..\Doc\refcount.png
  File ..\Doc\submenue.png

  File ..\Doc\mountpointpick.png
  File ..\Doc\mountpointdrop.png
  File ..\Doc\mountpointdrag.png
  File ..\Doc\mountpointdelete.png
  File ..\Doc\mountpointssplice.png

  File ..\Doc\symboliclinkclonefolderdrop.png
  File ..\Doc\symboliclinkfolderdrop.png
  File ..\Doc\uacunsigned.png
  File ..\Doc\cancelpickbackground.png
  File ..\Doc\linkshellextension.html
  File ..\Doc\linkshellextension_fr.html
  
  File ..\Doc\remotecapabilities.png
  File ..\Doc\junctionreplacementdrop.png
  File ..\Doc\mappednetworkdrive.png

  File ..\Doc\smartcopydrop.png
  File ..\Doc\smartcopyhowto.png
  File ..\Doc\junctionshowto.png
  File ..\Doc\symboliclinkhowto.png
  File ..\Doc\smartmovehowto.png
  File ..\Doc\smartmoveprogress.png
  File ..\Doc\symlinkssmart.png
  File ..\Doc\hardlinkclonessmart.png
  
  File ..\Doc\amazon.de.png
  File ..\Doc\facebook.png
  File ..\Doc\rss.png

  File ..\Doc\french.png
  File ..\Doc\english.png
  
  File ..\Doc\enumeratehardlinks.png
  File ..\Doc\enumeratehardlinksxp.png
  
  File ..\Doc\hardlinkicon_10.png
  File ..\Doc\junctionicon_10.png
  File ..\Doc\symboliclinkicon_10.png

  File ..\Doc\lseconfiggeneral.png
  File ..\Doc\lseconfighardlink.png

  File ..\Doc\zeiteisen.png
  File ..\Doc\deloreanpick.png
  File ..\Doc\deloreandrop.png
  File ..\Doc\deloreantimestamps.png
  File ..\Doc\delorean1024exceed.png

  File ..\Doc\outerjunctionscrop.png
  File ..\Doc\outerjunctionssplice.png
  File ..\Doc\outerjunctionsunroll.png
  File ..\Doc\outerjunctionsunrollinner.png
  File ..\Doc\outerjunctionsunrollinnernested.png
  File ..\Doc\outerjunctionsunrollinnersymlink.png
  File ..\Doc\outerjunctionsunrollcircularity.png
  File ..\Doc\outerjunctionsunrolldiskid.png
  File ..\Doc\outerjunctionsunrollmultitraverse.png

  File ..\Doc\multiplesource.png
  File ..\Doc\multiplesourceselect.png
  File ..\Doc\symlinkpriv.png

  File ..\Doc\symlinkcopydrop.png
  File ..\Doc\symlinkcopy.png

    
  File ..\Doc\smartmirrordrop.png

  File ..\Doc\bitcoinlogo.png
  File ..\Doc\bitcoinlseqr.png

  File ..\driver\symlink-1.06-x86.cab
  File ..\driver\symlink-1.06-x64.zip
  File ..\driver\symlink-1.06-src.zip
  
  Delete $INSTDIR\*.20*
  Delete $INSTDIR\32\*.20*
  
  IfSilent noreadme
    ExecShell "open" "$INSTDIR\Doc\linkshellextension.html"
  noreadme:

SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"

  IfSilent lbl_un_ProcEnumModules_ok

  ; Check for running processes
  ;
  lbl_un_ProcEnumModules_retry:
  Processes::ProcessEnumModules ${PLATTFORM};
  Pop $R0
  Pop $R1
  StrCmp $R0 "0" lbl_un_ProcEnumModules_ok
    MessageBox MB_ABORTRETRYIGNORE|MB_ICONEXCLAMATION "Please close the following processes before de-install:$\r$\n$\r$\n$R1" /SD IDIGNORE IDRETRY lbl_un_ProcEnumModules_retry IDIGNORE lbl_un_ProcEnumModules_ok 
    Abort
  
lbl_un_ProcEnumModules_ok:

  ;
  ; Get the windows version
  ;
  Call un.GetWindowsVersion
  Pop $R0
  StrCpy $WINDOWS_VERSION $R0
  ; MessageBox MB_OK "Windows Version '$WINDOWS_VERSION'"

  StrCpy $COM_SERVER_PATH $INSTDIR\HardlinkShellExt.dll
  StrCpy $REG_PREFIX_ ""
  Call un.COMServerCleanup
  
  !ifdef WIN32_REG_PREFIX_
    StrCpy $COM_SERVER_PATH $INSTDIR\32\HardlinkShellExt.dll
    StrCpy $REG_PREFIX_ ${WIN32_REG_PREFIX_}
    Call un.COMServerCleanup
  !endif

  
  ; Installer Cleanup
  ;
  DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\HardlinkShellExt\"

  ; MessageBox MB_YESNO "StartMenue ${startmenu}"

  Delete "${startmenu}\*.*"
  RMDir "${startmenu}"

  Processes::KillProcess "explorer.exe"
  Pop $R0

  Delete $INSTDIR\32\*.*
  Delete $INSTDIR\Doc\*.*
  Delete $INSTDIR\driver\*.*
  Delete $INSTDIR\*.*
  RMDir $INSTDIR\32
  RMDir $INSTDIR\Doc
  RMDir $INSTDIR\driver
  RMDir $INSTDIR


SectionEnd

