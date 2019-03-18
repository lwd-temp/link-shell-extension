; GetWindowsVersion
;
; Based on Yazno's function, http://yazno.tripod.com/powerpimpit/
; Updated by Joost Verburg
; Revamped by Hermann
;
; Returns on top of stack
;
; Windows Version (95, 98, ME, NT x.x, 2000, XP, 2003, Vista)
; or
; '' (Unknown Windows Version)
;
; Usage:
;   Call GetWindowsVersion
;   Pop $R0
;   ; at this point $R0 is "NT 4.0" or whatnot

!macro GetWindowsVersion un
Function ${un}GetWindowsVersion

  Processes::GetOsVersion 
  ; MessageBox MB_OK "Version #$R1#"

  StrCmp $R1 '3' lbl_error
  StrCmp $R1 '4' lbl_error

  StrCmp $R1 '4.0' lbl_win32_95
  StrCmp $R1 '4.9' lbl_win32_ME

  StrCmp $R1 '5.0' lbl_winnt_2000
  StrCmp $R1 '5.1' lbl_winnt_XP
  StrCmp $R1 '5.2' lbl_winnt_2003
  StrCmp $R1 '6.0' lbl_winnt_vista
  StrCmp $R1 '6.1' lbl_winnt_7 
  StrCmp $R1 '6.2' lbl_winnt_8
  StrCmp $R1 '6.3' lbl_winnt_8_1
  StrCmp $R1 '6.4' lbl_winnt_10_9826
  StrCmp $R1 '10.0' lbl_winnt_10 lbl_error

  lbl_win32_95:
    StrCpy $R0 '95'
  Goto lbl_done

  lbl_win32_ME:
    StrCpy $R0 'ME'
  Goto lbl_done

  lbl_winnt_2000:
    Strcpy $R0 '2000'
  Goto lbl_done

  lbl_winnt_XP:
    Strcpy $R0 'XP'
  Goto lbl_done

  lbl_winnt_2003:
    Strcpy $R0 '2003'
  Goto lbl_done

  lbl_winnt_vista:
    Strcpy $R0 'Vista'
  Goto lbl_done

  lbl_winnt_7:
    Strcpy $R0 'Windows7'
  Goto lbl_done

  lbl_winnt_8:
    Strcpy $R0 'Windows8'
  Goto lbl_done

  lbl_winnt_8_1:
    Strcpy $R0 'Windows8.1'
  Goto lbl_done

  lbl_winnt_10_9826:
    Strcpy $R0 'Windows10_9826'
  Goto lbl_done

  lbl_winnt_10:
    Strcpy $R0 'Windows10'
  Goto lbl_done

  lbl_error:
    Strcpy $R0 'unknown windows version'
    MessageBox MB_OK "Unknown Windows Version #$R1#"
  lbl_done:

  push $R0

FunctionEnd
!macroend

!insertmacro GetWindowsVersion ""
!insertmacro GetWindowsVersion "un."



