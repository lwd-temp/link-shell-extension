/**********************************************************************
*                                                                     *
*    A V L - Puma Operator Interface C++ Program                      *
*    Source file for general C-functions : FUNCTION.C                 *
*                                                                     *
***********************************************************************

********************** COPYRIGHT  (C) 1993 ****************************
*                                                                     *
*    This program is property of AVL/Graz.                            *
*    All rights reserved. No part of this program may be              *
*    used or copied in any way without the prior written              *
*    permission of AVL.                                               *
*                                                                     *
**********************************************************************/

////#include <solar.h>      //Compiler and Machine Dependents  REMOVED !!!!!!!


////#include <sv.hxx>       //for sv.lib  REMOVED !!!!!!!
////#include <tools.hxx>    //for tools.lib  REMOVED !!!!!!!
////#include <sysdep.hxx>  REMOVED !!!!!!!

#include "PoiStdAfx.h"
#include <direct.h>     //chdir
////#include <math.h>  REMOVED !!!!!!!
////#include <float.h>  REMOVED !!!!!!!
////#include <function.h>  REMOVED !!!!!!!
////#include <stdlib.h>  REMOVED !!!!!!!
////#include <stdio.h>  REMOVED !!!!!!!
#include <time.h>
#include "poi.hrc"
#include <avlinfob.hxx>
////#include <dir.h>  REMOVED !!!!!!!
////#include <avlres.hxx>  REMOVED !!!!!!!
////#include <global.hxx>  REMOVED !!!!!!!
////#include <poimainw.hxx>  REMOVED !!!!!!!
////#include <domanna.hxx>  REMOVED !!!!!!!
////#include <winman.hxx>  REMOVED !!!!!!!
////#include "poiapp.hxx"  REMOVED !!!!!!!
////#include "textwind.hxx"  REMOVED !!!!!!!

#ifdef unix
#include <sys/lock.h>
#include <X11/Xlib.h>
#undef FAR
#include "lm_client.h"
#include "lm_code.h"
#endif

extern int DebugLevel;
extern TextWindow *theDebugWin;
extern PoiApplication *pPoiApp;
extern Config *gConfig;
extern char theCurrentDir[];
extern char gWorkDir[];


//////////////////////////////////////////////////////////////////////////////
// GetFullFileName("SETTINGS", "WNDPATH", "poi.dsk") returns something like
// 'c:\avl\poi\wnd\poi.dsk', assuming the poi.ini variable "WNDPATH" in
// section "SETTINGS" is set to 'c:\avl\poi\wnd'.  If the variable is
// undefined, the poi work directory is prepended to the file name.
String GetFullFileName(const String & aSection, const String & aDirKey,
                         const char *aFileName)
{
  String path;

  if (gConfig)
  {
    gConfig->SetGroup(aSection);
    path = gConfig->ReadKey(aDirKey);
  }

  if (path.Len() == 0)
    path = String(gWorkDir);

  if ((char) path[(USHORT) (path.Len() - 1)] != '\\')
    path += '\\';

  if (aFileName)
    path += String(aFileName);

  return path;
}


String GetSettingsPath(const char *aDirKey, const char *aFileName)
{
  return GetFullFileName(String("SETTINGS"), String(aDirKey), aFileName);
}


int ChangeWorkDir(const char *aDir)
{
  return chdir(aDir);
}


char *GetWorkDir(char *aBuffer, int aMaxLen)
{
  return getcwd(aBuffer, aMaxLen);
}


//  FormatTimeFields(time) converts time in seconds in HH:MIN:SEC.
char *FormatTimeFields(double fTime)
{
  long i;
  double f;
  long days;
  long hours;
  long mins;
  long secs;
  long msecs;
  static char buf[64];

  f = fTime / 86400.;

  days = i = (long) f;

  f = f - i;

  hours = i = (long) (f = f * 24.);

  f = f - i;
  mins = i = (long) (f = f * 60.);

  f = f - i;
  secs = i = (long) (f = f * 60. + 0.0000001);

  f = f - i;
  msecs = (long) (f * 1000);
  sprintf(buf, "%2ld.%02ld:%02ld:%02ld", days, hours, mins, secs);
  return buf;
}

//  CharTime(aFormat) returns the current time in the specified format.
//  The  format string 'aFormat' has the syntax that cftime understands.
char *CharTime(const char *aFormat)
{
  static char buf[64];

  time_t now = time(0);
  struct tm *tm = localtime(&now);

  (void) strftime(buf, 64, aFormat, tm);
  return (buf);
}


#ifdef unix
LM_CODE(lmCode, ENCRYPTION_CODE_1, ENCRYPTION_CODE_2, VENDOR_KEY1,
        VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);
LM_HANDLE *lm_job = (LM_HANDLE *) 0;

#endif

//  LmCheckOut  fetches a license for the specified feature from the FlexLM
//  license manager.  If it fails, a negative error code is returned.
short LmCheckOut(char *aFeature, char *aVersion)
{
# ifdef unix
  static short initStatus = 0;
  int rc;

  if (initStatus < 0)   //init has failed; just say NO
    return -1;

  if (initStatus == 0)  //first call; init lm connection
  {
    initStatus = 1;
    rc = lm_init(VENDOR_NAME, &lmCode, &lm_job);
    if (rc)
    {
      switch (rc)
      {
        case LM_DEMOKIT:
          lm_perror("LM: LM_DEMOKIT: Init OK");
          break;
        default:
          lm_perror("LM: lm_init failed");
          initStatus = -1;
          return -1;
      }
    }
  }

  rc = lc_checkout(lm_job, aFeature, aVersion, 1, LM_CO_WAIT, &lmCode,
                   LM_DUP_NONE);

  if (rc)
  {
    lc_perror(lm_job, "LM: Can't get licence");
    return -2;
  }
  return 1;
# else
  return -1;
# endif
}


//  LmCheckIn  releases a license for the specified feature.
void LmCheckIn(char *aFeature)
{
# ifdef unix
  lc_checkin(lm_job, aFeature, 0);
# endif
}


void LockProcess()
{
# ifdef unix
  plock(PROCLOCK);
# endif
}


//  SetDisplay  sets the current display to the specified X11 display.  If
//  aDisplayName is the empty string, it resets the current display to the
//  original value (the one active before the first call to SetDisplay).
short SetDisplay(String & aDisplayName)
{
# ifdef unix
  static SVDISPLAY *orgDisplay = 0;
  static SVDISPLAY *nowDisplay = 0;

  if (!orgDisplay)      //remember original display
    orgDisplay = Sysdepen::GetCurDisplay();

  if (aDisplayName.Len() == 0)  //try to reset the original display
  {
    if (nowDisplay == orgDisplay)
      return 0;

    if (orgDisplay && nowDisplay)       //if only one != 0, we had an error
    {
      printf("SetDisplay: closing current display...\n");
      Sysdepen::CloseDisplay(nowDisplay);
      printf("SetDisplay: resetting to main display...\n");
      Sysdepen::ChangeCurDisplay(orgDisplay);
      nowDisplay = orgDisplay;
    }
    return 0;
  }

  //below, we switch to a new display
  if (nowDisplay)
  {
    printf("SetDisplay: closing current display...\n");
    Sysdepen::CloseDisplay(nowDisplay);
  }

  nowDisplay = Sysdepen::OpenDisplay(aDisplayName);
  if (!nowDisplay)
    return -1;

  printf("SetDisplay: setting display to %s ...\n", aDisplayName.GetStr());
  Sysdepen::ChangeCurDisplay(nowDisplay);
# endif
  return 0;
}


void SetWindowAttributes(const Window * aWindow)
{
  if (aWindow == 0)
    return;

# ifdef unix
  if (!pPoiApp->WithBackingStore())
    return;

  Widget wTop = Sysdepen::GetTopWidget(aWindow);

  MOT_Window win = XtWindow(wTop);

  if (!win)
    return;

  Display *aDisp = Sysdepen::GetSVDisplay();

  //try to set backing_store on
  if (DebugLevel & 0x10)
    theDebugWin->AddString(aWindow->GetText()
                           + String(": try to use backing store."));
  XSetWindowAttributes setWinAttr;

  setWinAttr.backing_store = WhenMapped;        //Always is too severe
  unsigned long valuemask = CWBackingStore;

  XChangeWindowAttributes(aDisp, win, valuemask, &setWinAttr);

  /*does not work;  maybe need to process event first if (DebugLevel & 0x10) { // check if server accepted request
   * XWindowAttributes winAttr; XGetWindowAttributes(aDisp, win, &winAttr); if (winAttr.backing_store)
   * theDebugWin->AddString(aWindow->GetText() + String(": backing store enabled.")); else
   * theDebugWin->AddString(aWindow->GetText() + String(": backing store DENIED.")); } */
  /*if (DoesBackingStore(Sysdepen::GetSVScreen())) printf("Server supports backing store\n"); else printf("Server does not
   * support backing store\n"); */
# endif
}



void TrimTrailingBlanks(String & theStr)
{
  short i;
  const char *buff = theStr.GetStr();

  for (i = theStr.Len() - 1; i >= 0; i--)
    if (buff[i] != ' ')
      break;
  theStr.Erase(i + 1);
}

void TrimLeadingBlanks(String & theStr)
{
  USHORT i;
  const char *buff = theStr.GetStr();

  for (i = 0; i < theStr.Len(); i++)
    if (buff[i] != ' ')
      break;
  theStr.Erase(0, i);
}

void TrimBlanks(String & theStr)
{
  TrimLeadingBlanks(theStr);
  TrimTrailingBlanks(theStr);
}


void ShowError(USHORT aErrorID, Window * aWnd, float min, float max)
{
  if (aWnd)
  {
    String aStr = gAvlTextResCont.GetResString(aErrorID);

    if (min != max)
    {
      char buf[64];

      sprintf(buf, " ( %1.7g - %1.7g )", (double)min, (double)max);
      aStr += String(buf);

    }
    PoiError aErrorBox(aWnd, WinBits(WB_OK | WB_DEF_OK), aStr);

    aErrorBox.SetText(gAvlTextResCont.GetResString(STR_D_ERROR_BOX));
    aErrorBox.Execute();
  }
}


double CalcDistanceOfPointandLine(Point & aLinePt1, Point & aLinePt2,
                                    Point & aPt, short *direc)
{
  double dist, lamda;
  double x0, x1, y0, y1, xx, yy;

  x0 = aLinePt1.X();
  y0 = aLinePt1.Y();
  x1 = aLinePt2.X();
  y1 = aLinePt2.Y();
  xx = aPt.X();
  yy = aPt.Y();

  /*formula for calculation : find nearest distance between point and a line */
  /*d = [ a/[a] x [r1 - r0]] --> d = (r0 - r1) + ld*a; ld=(r1-r0)*a/a*a; bartsch S282 */

  dist = (x0 - x1) * (x0 - x1) + (y0 - y1) * (y0 - y1); /*a*a */

  if (dist == 0)
    return 0.;
  /*r1 *//*caculate lambda see bartsch page 282 */
  lamda = ((xx - x1) * (x0 - x1) + (yy - y1) * (y0 - y1)) / dist;
  //if (lamda < 0.) l1=0.;
  //if (lamda > 1.) l1=1.;
  /*r0-r1 */
  x1 = (x1 - xx) + lamda * (x0 - x1);
  /*r0-r1 */
  y1 = (y1 - yy) + lamda * (y0 - y1);

  if (y1 < 0)
    *direc = 1; //if point above of the line direc = 1;
  else
    *direc = -1;

  dist = sqrt(x1 * x1 + y1 * y1);
  return dist;
}

short signum(double aCnt)
{
  if (aCnt >= 0)
    return 1;
  else
    return -1;
}

String Splitpath(String aString, USHORT what)
{
  char drive[_MAX_DRIVE];
  char dir[_MAX_DIR];
  char file[_MAX_FNAME];
  char ext[_MAX_EXT];

  String aStr;

  _splitpath(aString.GetStr(), drive, dir, file, ext);
  switch (what)
  {
    case POI_DRIVE:
      aStr = String(drive);
      break;
    case POI_DIRECTORY:
      aStr = String(dir);
      break;
    case POI_FILENAME:
      aStr = String(file);
      break;
    case POI_EXTENSION:
      aStr = String(ext);
      break;

  }

  return (aStr);
}


PoiError::PoiError(Window * aWnd, WinBits aStyle, const String & aMess):
ErrorBox(aWnd, aStyle, aMess)
{
  if (!aWnd || aWnd == pPoiMain)
  {
    CommunicationWindow *aCom = gWinMannager.GetActiveWindow();

    if (aCom)
      aWnd = aCom->GetDoMannager()->GetWindow();
  }

  iParentToCheck = aWnd;
  iLockStatus = FALSE;
  ihandler = gWinMannager.IsWindowInWorkList(iParentToCheck);
  if (ihandler)
  {
    iLockStatus = ihandler->Locked();
    ihandler->IsLockedByDlg() = TRUE;
    ihandler->Locked() = TRUE;
  }
  //CaptureMouse();
}

PoiError::~PoiError()
{
  ReleaseMouse();
  if (iParentToCheck)
  {
    DoMannager *handler = gWinMannager.IsWindowInWorkList(iParentToCheck);

    if (handler != ihandler)
    {
      InfoBox aInfo(0, String("PoiError: Crash will happen!!!!"));

      aInfo.Execute();
    }
    else
    {
      if (ihandler)
      {
        ihandler->Locked() = iLockStatus;
        ihandler->IsLockedByDlg() = FALSE;
      }
    }

  }
}
extern void CenterDialogCoord(int aWidth, int aHeight, int *aPosx, int *aPosy);

/*******************************/
PoiQueryBox::PoiQueryBox(Window * aWnd, WinBits aStyle, const String & aMess):
QueryBox(aWnd, aStyle, aMess)
{
  if (!aWnd || aWnd == pPoiMain)
  {
    CommunicationWindow *aCom = gWinMannager.GetActiveWindow();

    if (aCom)
      aWnd = aCom->GetDoMannager()->GetWindow();

  }
  iParentToCheck = aWnd;
  iLockStatus = FALSE;
  ihandler = gWinMannager.IsWindowInWorkList(iParentToCheck);
  if (ihandler)
  {
    iLockStatus = ihandler->Locked();
    ihandler->Locked() = TRUE;
    ihandler->IsLockedByDlg() = TRUE;
  }
  //Size aSize = GetSizePixel();
  //int aPosx;
  //int aPosy;
  //CenterDialogCoord(aSize.Width(),aSize.Height(),&aPosx,&aPosy);
  //ChangePosPixel(Point(aPosx,aPosy));
}

PoiQueryBox::~PoiQueryBox()
{
  ReleaseMouse();
  if (iParentToCheck)
  {
    DoMannager *handler = gWinMannager.IsWindowInWorkList(iParentToCheck);

    if (handler != ihandler)
    {
      InfoBox aInfo(0, String("PoiError: Crash will happen!!!!"));

      aInfo.Execute();
    }
    else
    {
      if (handler)
      {
        handler->Locked() = iLockStatus;
        handler->IsLockedByDlg() = FALSE;
      }
    }

  }
}
