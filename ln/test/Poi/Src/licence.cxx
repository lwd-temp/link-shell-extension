/**********************************************************************
*                                                                     *
*    A V L - Puma Operator Interface C++ Program                      *
*    Source file for Class : ActionArgument                           *
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

#include "PoiStdAfx.h"
#include "Licence.hxx"
#include "FLEXLmAPI\Include\flexlmclient.h"
#include "..\DeploymentTools\Licensing\LicCheckSum\LicCheckSum.h"


extern String GetSettingsPath(const char* aDirKey, const char* aFileName);
extern String & IntString(ULONG aId); //defined in poimainw.cxx
extern void AddMessageLine(const String & aMess,short type );

Licencemgr gLicenceMgr;

String GetOptionTx()
{
  // String aFilename = "f:\\prtv562\\option.tx";//GetSettingsPath("POITXTPATH", "Option.tx");
  String aFilename = GetSettingsPath("POITXTPATH", "Option.tx");
  return aFilename;
}

String GetLicenseFile()
{
  //  String aFilename = "f:\\prtv562\\license.dat";//GetSettingsPath("LICENSEPATH", "license.dat");
  String aFilename = GetSettingsPath("LICENSEPATH", "license.dat");
  
  return aFilename;
}
// input is a timestring like 01-jul-2001 
//output is the corresponding SYSTEMTIME
void Licencemgr::ConvertLicTimeToSystemTime(const String & aStr, SYSTEMTIME *pTime)
{
  ZeroMemory(pTime,sizeof(SYSTEMTIME));
  
  String aTime = aStr;
  if(aTime.GetTokenCount('-') != 3)
  { 
    theDebugWin->AddString("Licencemgr::ConvertLicTimeToSystemTime: The Timeformat is not correct! Don't have 3 token!");
    return;
  }
  
  WORD day = atoi(aTime.GetToken(0,'-'));
  String tmonth = aTime.GetToken(1,'-');
  WORD month=0;
  if(stricmp(tmonth.GetStr(),"Jan") == 0)
    month=1;
  else if(stricmp(tmonth.GetStr(),"Feb") == 0)
    month=2;
  else if(stricmp(tmonth.GetStr(),"Mar") == 0)
    month=3;
  else if(stricmp(tmonth.GetStr(),"Apr") == 0)
    month=4;
  else if(stricmp(tmonth.GetStr(),"May") == 0)
    month=5;
  else if(stricmp(tmonth.GetStr(),"Jun") == 0)
    month=6;
  else if(stricmp(tmonth.GetStr(),"Jul") == 0)
    month=7;
  else if(stricmp(tmonth.GetStr(),"Aug") == 0)
    month=8;
  else if(stricmp(tmonth.GetStr(),"Sep") == 0)
    month=9;
  else if(stricmp(tmonth.GetStr(),"Oct") == 0)
    month=10;
  else if(stricmp(tmonth.GetStr(),"Nov") == 0)
    month=11;
  else if(stricmp(tmonth.GetStr(),"Dec") == 0)
    month=12;
  WORD year = atoi(aTime.GetToken(2,'-'));
  pTime->wYear =year;
  pTime->wMonth = month;
  pTime->wDay=day;
}

void Licencemgr::RemoveAll()
{
  LicenceFiles *pLic = m_LicList.First();
  while(pLic)
  {
    delete pLic;
    pLic = m_LicList.Next();
  }
  m_LicList.Clear();
}
Licencemgr::~Licencemgr()
{
  RemoveAll();
}

BOOL Licencemgr::GetEntriesFromLicenseDat()
{
  RemoveAll();
  String aFilename = GetLicenseFile();
  //++++++++++++++++++++++++++++++++
  //+ Read licence file            +
  //++++++++++++++++++++++++++++++++
  //wsprintf(iniLicence, _T("%s\\%s"), iniIniPath, aLicenceFile);
  FILE *aFileId = fopen(aFilename.GetStr(), "rt");
  char aStr[1024];
  
  if (!aFileId)
  {
    ErrorBox aErrorBox(pPoiMain, WinBits(WB_OK | WB_DEF_OK),
      gAvlTextResCont.GetResString(STR_ERROR_OPEN_FILE) + String(" ") + aFilename);
    
    aErrorBox.SetText(gAvlTextResCont.GetResString(STR_D_ERROR_BOX));
    aErrorBox.Execute();
    return FALSE;
  }
  while (fgets(aStr, 512, aFileId))
  {
    
    char *test = 0;
    if ((test = strrchr(aStr, '\n')) != 0)
      *test = 0;
	   
    String tmpStr(aStr);
    TrimBlanks(tmpStr);
    if(tmpStr.GetStr()[0] =='#')
      continue;
    if(tmpStr.GetStr()[tmpStr.Len()-1] =='\\')
    {
      tmpStr.Replace(String(" "),tmpStr.Len()-1);
      fgets(aStr, 512, aFileId);
      if ((test = strrchr(aStr, '\n')) != 0)
        *test = 0;
      
      tmpStr += aStr;
    }
    
    //	FEATURE FLXParameters puma 1.000 01-jul-2001 0 8C19533F7DCBD8A86870 "MaxInstances=3" INTERNET=157.247.*.*
    if (tmpStr.GetTokenCount(' ') >= 5)
    {  
      if(tmpStr.GetToken(0, ' ') != "FEATURE")
        continue;
      String aCompStr = tmpStr.GetToken(1, ' ');
      String aDateStr = tmpStr.GetToken(4, ' ');
      SYSTEMTIME aSysTime;
      ConvertLicTimeToSystemTime(aDateStr,&aSysTime);
      LicenceFiles *pLic = new LicenceFiles(aCompStr,aSysTime);
      m_LicList.Insert(pLic,LIST_APPEND);
    }   
  }
  fclose(aFileId);
  return TRUE;
}


#define _SECOND ((__int64) 10000000)
#define _MINUTE (60 * _SECOND)
#define _HOUR   (60 * _MINUTE)
#define _DAY    (24 * _HOUR) 

long Licencemgr::TimeToExpire(SYSTEMTIME & aTime,long & days, long & hours, long & min)
{
  SYSTEMTIME currTime;
  GetLocalTime(&currTime);
  
  FILETIME currFileTime;
  SystemTimeToFileTime(&currTime,&currFileTime);
  ULONGLONG qwcurrFileTime = (((ULONGLONG) currFileTime.dwHighDateTime) << 32) + currFileTime.dwLowDateTime;
  FILETIME expireFileTime;
  SystemTimeToFileTime(&aTime,&expireFileTime);
  ULONGLONG qwexpireFileTime = (((ULONGLONG) expireFileTime.dwHighDateTime) << 32) + expireFileTime.dwLowDateTime;
  ULONGLONG diff = 0;
  int neg = 1;
  if(qwcurrFileTime < qwexpireFileTime)
  {
    diff = qwexpireFileTime - qwcurrFileTime;
  }
  else
  {
    neg = -1;
    diff = qwcurrFileTime - qwexpireFileTime;
  }
  
  days =(long)( diff /_DAY);
  hours = (long)((diff - days*_DAY)/_HOUR);
  min =(long)((diff  - hours*_HOUR - days*_DAY)/_MINUTE);
  
  return (long)neg;
}

void Licencemgr::CheckAllLicenses()
{
  GetEntriesFromLicenseDat();
  LicenceFiles *pLic = m_LicList.First();
  while(pLic)
  {
    long days=0;
    long hours=0;
    long min=0;
    int neg = TimeToExpire(pLic->m_ExpireTime,days,hours,min);
    if(neg < 0)
    {
   		 String reason = IntString(STR_LICENSE_HAS_EXPIRED);
       char text[255];
       //		 sprintf(text,reason.GetStr(),days);
       sprintf(text,reason,pLic->m_Name.GetStr(),days);
       reason = text;
       AddMessageLine(reason,2);
    } else
      if(days <= 14)
      {
        String reason = IntString(STR_LICENSE_WILL_EXPIRE);
        char text[255];
        sprintf(text,reason,pLic->m_Name.GetStr(),days);
        reason = text;
        AddMessageLine(reason,2);
      }
      pLic = m_LicList.Next();
  }
}

BOOL Licencemgr::IsAdeModeAllowed()
{
  String aFilename = GetLicenseFile();
  String aOptionTx = GetOptionTx();
  
  FLEXlmClient TheLicenceManager("poiade", "1.2", false, aFilename.GetStr());
  BOOL iniLicenceOK = TheLicenceManager.LicenseCheckedOut();
  
  if (iniLicenceOK)
  {
    return TRUE;
  }
  return FALSE;
}

BOOL Licencemgr::CheckLicence(String &reason)
{
  String aFilename = GetLicenseFile();
  String aOptionTx = GetOptionTx();
  
  FLEXlmClient TheLicenceManager("pr68k", "1.2", false, aFilename.GetStr());
  BOOL iniLicenceOK = TheLicenceManager.LicenseCheckedOut();
  
  if (iniLicenceOK)
  {
    String authorize = TheLicenceManager.Value("AUTHORIZ");
    if(authorize.Len() == 0) // no check if no checksum assume it is ok
      return TRUE;
    unsigned char checksum[16]; 
    
    if(OptionTxFile((char*)aOptionTx.GetStr(),checksum)==true)
    { 
      char checksumStrg[64];
      checksumStrg[0]=0; 
      unsigned int i;
      for (i = 0; i < 16; i++)
      {
        char byteck[16];
        sprintf(byteck,"%02x", checksum[i]);
        strcat(checksumStrg,byteck);
      }
      if(strcmp(authorize.GetStr(),checksumStrg)!=0)
      {
        String reason = IntString(STR_CHECKSUM_NOT_OK);
        char text[255];
        sprintf(text,reason,aOptionTx.GetStr());
        theDebugWin->AddString(String("License checksum NOK: AUTOHORIZ=") +authorize.GetStr()+String("=")+checksumStrg);
        reason=text;
        return FALSE;
      }
    } else
    {
 	    reason=IntString(STR_NO_VALID_CHECKSUMFILE);
      char text[255];
      sprintf(text,reason,aOptionTx.GetStr());
      reason=text;
      return FALSE;
    }
  } else
  {
    reason=IntString(STR_NO_VALID_LICENSE);
    return FALSE;
  }
  return TRUE;
}

BOOL Licencemgr::RetrieveLocalIpAddress()
{
  WSADATA  wsadata;
  HOSTENT *lpHost=NULL;
  char    hostname[_MAX_PATH];
  TCHAR    msg[128];
  struct   sockaddr_in   dest;
  BOOL bReturn = FALSE;
  
  int err = WSAStartup(0x0101, &wsadata);
  if(err != 0)
    return FALSE;
  int ret;
  if ((ret = gethostname(hostname, _MAX_PATH)) != 0)
  {
    sprintf(msg, "Error calling gethostname: %d", ret);
    theDebugWin->AddString(msg);
  }
  else
  {
    sprintf(msg, "Hostname is: %s", hostname);
    theDebugWin->AddString(String(msg));
    
  }
  
  lpHost = gethostbyname(hostname);
  if (lpHost == NULL)
  {
    wsprintf(msg, "gethostbyname failed: %d", WSAGetLastError());
    theDebugWin->AddString(String(msg));
  }
  else
  {
    for(int i=0; lpHost->h_addr_list[i] != NULL ;i++)
    {
      memcpy(&(dest.sin_addr), lpHost->h_addr_list[i],
        lpHost->h_length);
      //          wsprintf(msg, "IP address is: '%s'",
      //                   inet_ntoa(dest.sin_addr));
      char *add = inet_ntoa(dest.sin_addr);
      if(add != 0)
      {
        m_LocalIpAddress = add;
        bReturn = TRUE; 	
      }
      theDebugWin->AddString(String("Local IpAddress ") + m_LocalIpAddress);
      
    }
    
  }
  WSACleanup();
  return bReturn;
}