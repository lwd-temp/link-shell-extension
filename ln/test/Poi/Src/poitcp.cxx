/**********************************************************************
*                                                                     *
*    A V L - Puma Operator Interface C++ Program                      *
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
#include <sys/types.h>

////#include <stdlib.h>  REMOVED !!!!!!!
////#include <stdio.h>  REMOVED !!!!!!!
////#include <string.h>  REMOVED !!!!!!!
#include <signal.h>
#include <ctype.h>
//#include <iostream.h>
//#include <iomanip.h>
////#include <io.h>                 // for open()  REMOVED !!!!!!!
#include <fcntl.h>              // for open()

#ifdef unix
#  include <unistd.h>
#  include <netinet/in.h>
#  include <sys/times.h>
#endif

#include "event.h"
////#include "POI_DEBUG.hxx"  REMOVED !!!!!!!
#include "poitcp.hxx"
#include "exttcp.hxx"
#include "poi.hrc"

#define ARGUS_FEATURE	"ArgusServer"
#define ARGUS_VERSION	"1.0"	// for license manager only
#define PROTOCOL_VERSION 104	// Argus <-> Poi protocol version

extern int gDebugLevel;
extern int DebugLevel;
extern Debug *gDebug;

#define POI_DEBUG(level) if (gDebug && DebugLevel&0x01 && level<=gDebugLevel) \
*gDebug<<

#undef SWAPPED
#if defined(MS_WINDOWS) || defined(WIN32)
# define SWAPPED 1
#endif

extern char* gWantPumaPort;
extern char* gSnifOptions;

extern String & IntString(ULONG aId);
extern void PoiInfoBox(const String aText);

#define TEXTID(id) IntString(id)
#define INFO(id,text) PoiInfoBox(String(id) + String(": ") + text)

extern short LmCheckOut(char* aFeature, char* aVersion);
extern void LmCheckIn(char* aFeature);

#define NeedCM (iPumaPort > 0 || iArgusPort || iConnectPort > 0)



//  global variables
//  --------------------------------------------------------------------------

PoiTcp *gTcp = 0;       // for CM callback functions


//  local functions
//  --------------------------------------------------------------------------

long elapsedTime()
{
# ifdef unix
  static unsigned long start = 0;
  
  struct timeval tm;
  
  gettimeofday(&tm, 0);
  unsigned long t = 1000 * tm.tv_sec + tm.tv_usec/1000;
  if (! start)			// first call
    start = t;
  //Message(1, "time: %d", (long)(t-start));
  return (long)(t - start);
#endif
# ifdef WIN32
  static DWORD start = 0;
  
  //DWORD t = timeGetTime();
  DWORD t = GetTickCount();
  if (! start)			// first call
    start = t;
  //Message(1, "time: %d", (long)(t-start));
  return (long)(t - start);
#endif
}


#ifdef unix
void alarmCb(int)
{
  POI_DEBUG(12) "Alarm signal arrived" << debl;
  (void) signal(SIGALRM, alarmCb);
}
#endif

// support for byte ordering
short swap_s(char *aVal)
{
  short help;
  char *out = (char *) &help;
  
# ifdef SWAPPED
  out[0] = aVal[1];
  out[1] = aVal[0];
# else
  out[0] = aVal[0];     // a naive person might think its enough to
  out[1] = aVal[1];     // return aVal;  but be aware of alignment
# endif
  
  return help;
}


long swap_l(char *aVal)
{
  long help;
  char *out = (char *) &help;
  
# ifdef SWAPPED
  out[0] = aVal[3];
  out[1] = aVal[2];
  out[2] = aVal[1];
  out[3] = aVal[0];
# else
  out[0] = aVal[0];
  out[1] = aVal[1];
  out[2] = aVal[2];
  out[3] = aVal[3];
# endif
  
  return help;
}


//  CurrentTime returns the elapsed number of seconds up to now from some
//  fixed point in the past.
long CurrentTime()
{
# ifdef MS_WINDOWS
  return GetTickCount() / 1000;
#endif
# ifdef unix
  long tickspersec = sysconf(_SC_CLK_TCK);
  tms b;
  long ticks = times(&b);
  
  return ticks / tickspersec;
# endif
}


//  BsliceRead returns the total size of the read bslice, or a negative
//  error code.  aTag is expected to be 1 for all bslices that come from
//  Puma.
long BsliceRead(SliceBuff & aBuf, ConnectionManager & aCM, short aConnID,
                BOOL aTag = 0)
{
  static BYTE hbuf[2];
  
  aBuf.SwitchTag(aTag);
  
  // determine bslice size
  int rc = aCM.Read(aConnID, (char *) hbuf, 2);
  
  if (rc != 2)
  {
    POI_DEBUG(2) "BsliceRead: could not read bslice size" << debl;
    return -1;
  }
  
  USHORT size = hbuf[0] * 0x100 + hbuf[1];
  
  aBuf.MakePlace(size);
  char *buf = (char *) aBuf.GetWriteBuffer();
  
  rc = 0;
  int left = size;
  
  while (left > 0 && (rc = aCM.Read(aConnID, buf, left)) > 0)
  {
    buf += rc;
    left -= rc;
  }
  
  if (rc < 0 || left != 0)
  {
    POI_DEBUG(1) "BsliceRead: requested " << size << " bytes, got " <<
      (size - left) << " (rc=" << rc << ")" << debl;
    return -2;
  }
  return (long) size + 2;
}


//  BslyteWrite returns 0 on success, or a negative error code.
short BslyteWrite(BYTE * aBslice, int aLen, ConnectionManager & aCM,
                  short aConnID)
{
  // send bslice
  POI_DEBUG(50) "BslyteWrite(" << aConnID << "): sending bslice of size "
    << aLen << debl;
  
  if (aCM.Write(aConnID, (char *) aBslice, aLen) < 0)
    return -1;
  return 0;
}


//  CM Callback Functions
//  --------------------------------------------------------------------------

void TcpException(int aConnID, ConnectionManager *)
{
  POI_DEBUG(8) "!! Tcp exception on connection " << aConnID << debl;
}


void ArgusAccept(int aConnID, ConnectionManager *)
{
  gTcp->ArgusAccept(aConnID);
}


void ArgusRead(int aConnID, ConnectionManager *)
{
  gTcp->ArgusRead(aConnID);
}


void PumaAccept(int aConnID, ConnectionManager *)
{
  gTcp->PumaAccept(aConnID);
}


void PumaRead(int, ConnectionManager *)
{
  gTcp->PumaRead();
}


void PoiRead(int aConnID, ConnectionManager *)
{
  gTcp->PoiRead(aConnID);
}


//  BufQueue methods
//  --------------------------------------------------------------------------

BufQueueElem::BufQueueElem(SliceBuff & aBuf, short aDelay, BYTE aMachID)
{
  iLen = aBuf.GetLength() + 2;
  iBslice = new BYTE[iLen];
  memcpy(iBslice, aBuf.GetReadBuffer(), iLen);
  iDelay = aDelay;
  iMachID = aMachID;
}


void BufQueue::Clear()
{
  for (BufQueueElem * elem = iQueue.First(); elem; elem = iQueue.Next())
    delete elem;
  
  iQueue.Clear();
}


BufQueue::~BufQueue()
{
  Clear();
}


void BufQueue::Insert(SliceBuff & aBuf, short aDelay, BYTE aMachID)
{
  // aDelay++;			// delay is immediately decremented
  BufQueueElem *elem = new BufQueueElem(aBuf, aDelay, aMachID);
  
  iQueue.Insert(elem, LIST_APPEND);
  POI_DEBUG(20) "BufQueue::Insert: delay = " << aDelay << " queue length = "
    << iQueue.Count() << debl;
}


void BufQueue::Remove(BYTE aMachID)
{
  for (BufQueueElem * elem = iQueue.First(); elem; elem = iQueue.Next())
    if (elem->iMachID == aMachID)
    {
      POI_DEBUG(8) "BufQueue: removing request for machine " << (int) aMachID << debl;
      delete elem;
      
      iQueue.Remove();
      elem = iQueue.GetCurObject();
    }
}


short BufQueue::ContainsRequest(BYTE aMachID)
{
  for (BufQueueElem * elem = iQueue.First(); elem; elem = iQueue.Next())
  {
    if (elem->iMachID == aMachID)
      return 1;
  }
  return 0;
}


//  Top returns the oldest bslice in BufQueue with delay == 0, or the zero
//  pointer if no entry with zero delay exists.
//  If aForce != 0, then the top of the queue is returned regardless of the
//  delay.
//  The returned entry must be deleted by the caller.
BufQueueElem *BufQueue::Top(short aForce)
{
  for (BufQueueElem * elem = iQueue.First(); elem; elem = iQueue.Next())
    if (aForce || elem->iDelay <= 0)
      return iQueue.Remove();
    return 0;
}


//  DecrDelay decrements the delay value of all enqueued bslices with
//  positive delay.
void BufQueue::DecrDelay()
{
  for (BufQueueElem * elem = iQueue.First(); elem; elem = iQueue.Next())
    if (elem->iDelay > 0)
      elem->iDelay--;
}


//  PoiTcp methods
//  --------------------------------------------------------------------------

PoiTcp::PoiTcp()
{
  iProtocolVersion = PROTOCOL_VERSION;
  iStatistics.iValue[PROTOC_VERSION] = iProtocolVersion;
  
  iBeds = 0;
  iPtl = 0;
  iCM = 0;
  iEOMslice = 0;
  iPumaInit = 255;		// no connection
  iSnifFile = -1;
  iUrgent = 0;
  iSysvalid = 1;

  iMachID = 0;
  //iProtocolVersion=0;
  iConnectTimeout=0;
  iArgusLicenseOK=TRUE;

  iPollFreq = 0;
  iPollDelay = 100;
  iUpdateDelay = 100;
  iArgusDelay = 100;	// nr. of tics Argus requests are kept in queue
  iPumaPort = 0;	// listen port for Puma
  iArgusPort = 0;	// listen port for Argus
  iConnectPort = 0;	// connect port to remote Pois
  iMaxNumBslices = 30;
  m_NrOfPSVSlices = 100;

  iPumaConn = -1;
  iPumaWriteOK = 0;     	// Puma is expected to send first bslice
  iIsDefault = 0;
}


PoiTcp::~PoiTcp()
{
  gTcp = 0;
  
  if (iBeds)
    DeleteBeds();
  
  StopSnif();
  
  Close();
  
  delete iCM;   // delete connection manager
  delete iEOMslice;
}


//  InitBeds reads the list of available test beds from the file
//  beds.lst and initializes the iBeds member.
//  InitBeds may be called more than once.
short PoiTcp::InitBeds()
{
  const int maxLen = 128;
  char inLine[maxLen];
  
  if (iBeds)
    iBeds->Clear();
  else
    iBeds = new BedList;
  
  FILE *bedf = fopen(iBedsFile, "r");
  
  if (!bedf)
  {
    POI_DEBUG(2) "List of Test Beds (" << iBedsFile.GetStr() << ") not found." << debl;
    return -1;
  }
  
  int nr = 0;
  char *sep = " \t\n";
  String options;
  
  while (fgets(inLine, maxLen, bedf))
  {
    nr++;
    inLine[maxLen - 1] = '\0';
    
    if (inLine[0] == '#' || strspn(inLine, " \t\n") == strlen(inLine))
      continue; // skip comments and empty lines
    
    if (inLine[0] == '[')	// option string
    {
      char* end = strchr(inLine, ']');
      if (! end)
      {
        POI_DEBUG(2) "InitBeds: " << iBedsFile.GetStr() << " line " << nr
          << ": Option line does not contain ']'" << debl;
        continue;
      }
      *end = '\0';
      options = inLine + 1;
      continue;
    }
    
    char *tok1 = strtok(inLine, sep);
    char *tok2 = strtok(0, sep);
    char *tok3 = strtok(0, "\n");
    
    if (!tok3)
    {
      POI_DEBUG(2) "InitBeds: " << iBedsFile.GetStr() << " line " << nr
        << " is incomplete." << debl;
      continue;
    }
    
    tok3 += strspn(tok3, " \t");        // skip leading white space in bed name
    if (*tok3 == '\0')
    {
      POI_DEBUG(2) "InitBeds: " << iBedsFile.GetStr() << " line " << nr
        << ": Bed name is missing." << debl;
      continue;
    }
    
    int val1 = atoi(tok1);
    
    if (!ArgusMachID(val1))
    {
      POI_DEBUG(2) "InitBeds: " << iBedsFile.GetStr() << " line " << nr
        << ": invalid machine id " << val1
        << " (must lie in range 1..249)." << debl;
      continue;
    }
    
    BYTE machID = (BYTE) val1;
    if (GetBed(machID))
    {
      POI_DEBUG(2) "InitBeds: " << iBedsFile.GetStr() << " line " << nr
        << ": machine id " << val1 << " multiply defined ." << debl;
      continue;
    }
    
    String host(tok2);
    String name(tok3);
    iBeds->Insert(new Bed(machID, host, options, name), LIST_APPEND);
  }
  
  fclose(bedf);
  
  return 0;
}


void PoiTcp::DeleteBeds()
{
  if (!iBeds)
    return;
  
  POI_DEBUG(20) "deleting beds ..." << debl;
  for (Bed * bed = iBeds->First(); bed; bed = iBeds->Next())
    delete bed;
  delete iBeds;
  
  iBeds = 0;
}


//  PoiClose closes the monitor connection to the specified POI.
void PoiTcp::PoiClose(short aConnID)
{
  POI_DEBUG(8) "closing POI connection " << aConnID << debl;
  
  BYTE machID = MachID(iPoiConn, aConnID);
  
  iPoiQueue.Remove(machID);
  DeleteAssoc(iPoiConn, aConnID);
  iPtl->TcpPoiDisconnected(machID);
  if (iCM)      // only possible if called before Init
    iCM->Close(aConnID);
}


//  ArgusClose closes the connection to the specified Argus.
void PoiTcp::ArgusClose(short aConnID)
{
  BYTE machID = MachID(iArgusConn, aConnID);
  
  POI_DEBUG(8) "closing ARGUS " << (int) machID << debl;
  
  DeleteAssoc(iArgusConn, aConnID);
  iPumaQueue.Remove(machID);
  
  if (iLastMachID == machID)    // we have a pending response
    iLastMachID = InvalidMachID;
  
  if (iCM)      // only possible if called before Init
    iCM->Close(aConnID);
}


//  PumaClose closes the Tcp connection to PUMA.
void PoiTcp::PumaClose()
{
  // close Arguses
  for (Assoc * assoc = iArgusConn.First(); assoc; assoc = iArgusConn.Next())
    ArgusClose(assoc->iConnID);
  
  // delete buffers
  iPumaQueue.Clear();
  
  if (iPumaConn < 0)
    return;
  
  POI_DEBUG(8) "closing PUMA connection." << debl;
  
  if (iCM)
    iCM->Close(iPumaConn);
  iPumaConn = -1;
}


//  ArgusGetLicense checks out an Argus license from FlexLM
short PoiTcp::ArgusGetLicense()
{
  POI_DEBUG(10)"ArgusGetLicense: trying to contact FlexLM ..."<<debl;
  short rc = LmCheckOut(ARGUS_FEATURE, ARGUS_VERSION);
  if (rc < 0)
  {
    POI_DEBUG(10)"  FAILED"<<debl;
    return -1;
  }
  POI_DEBUG(10)"  OK"<<debl;
  return 1;
}


//  ArgusReturnLicense returns an Argus license to FlexLM
void PoiTcp::ArgusReturnLicense()
{
  POI_DEBUG(10)"ArgusReturnLicense: returning license to FlexLM"<<debl;
  LmCheckIn(ARGUS_FEATURE);
}


//  CreateAssoc  relates an Argus or remote Poi machine ID with a
//  connection ID.
void PoiTcp::CreateAssoc(ConnTable & aTable, BYTE aMachID, short aConnID,
                         ConnInfo * aInfo)
{
  POI_DEBUG(50) "CreateAssoc(" << (int) aMachID << ':' << aConnID << ")." << debl;
  
  Assoc *assoc = new Assoc(aMachID, aConnID, aInfo);
  
  aTable.Insert(assoc);
}


short PoiTcp::ConnID(ConnTable & aTable, BYTE aMachID)
{
  for (Assoc * assoc = aTable.First(); assoc; assoc = aTable.Next())
    if (assoc->iMachID == aMachID)
      return assoc->iConnID;
    return -1;
}


BYTE PoiTcp::MachID(ConnTable & aTable, short aConnID)
{
  for (Assoc * assoc = aTable.First(); assoc; assoc = aTable.Next())
    if (assoc->iConnID == aConnID)
      return assoc->iMachID;
    return 0;
}


ConnInfo *PoiTcp::Info(ConnTable & aTable, short aMachID)
{
  for (Assoc * assoc = aTable.First(); assoc; assoc = aTable.Next())
    if (assoc->iMachID == aMachID)
      return assoc->iInfo;
    return 0;
}

RequestList *PoiTcp::GetSlowRequestList(BYTE aMachID)
{
  if (aMachID)  // find the remote list
  {
    return NULL;  //Not supported
  }
  if (iPumaInit == 0)		// we have or had a Puma connection
    return &iSlowWriteRequests;
  POI_DEBUG(40)"PoiTcp::GetSlowRequestList(0): Puma is not connected."<<debl;
  return 0;
}

RequestList *PoiTcp::GetRequestList(BYTE aMachID)
{
  if (aMachID)  // find the remote list
  {
    PoiInfo *info = (PoiInfo *) Info(iPoiConn, aMachID);
    
    if (!info)  // this Poi is not connected
      return 0;
    return &info->iWriteRequests;
  }
  else  // use local request list
  {
    if (iPumaInit == 0)		// we have or had a Puma connection
      return &iWriteRequests;
    
    //  we either have no Puma, or the connection is not established yet
    POI_DEBUG(40)"PoiTcp::GetRequestList(0): Puma is not connected."<<debl;
    return 0;
  }
}


void PoiTcp::DeleteAssoc(ConnTable & aTable, short aConnID)
{
  for (Assoc * assoc = aTable.First(); assoc; assoc = aTable.Next())
    if (assoc->iConnID == aConnID)
    {
      aTable.Remove(assoc);
      delete assoc;
    }
}


PoiInfo::PoiInfo()
{
  iPending = 0;
  iProtocolVersion = -1;
  
  Init();
}


PoiInfo::~PoiInfo()
{
  Slice *slice;
  
  POI_DEBUG(12) "Deleting PoiInfo struct ..." << debl;
  
  for (slice = iWriteRequests.First(); slice; slice = iWriteRequests.Next())
    slice->iRequestRegistered = FALSE;
}


void PoiInfo::Init()
{
  iInitTime = CurrentTime();
  iToPoiBslices = 0;
  iFromPoiBslices = 0;
}


ArgusInfo::ArgusInfo()
{
  iProtocolVersion = -1;
  iPendingCommand = 0;
  iNumBslices = 0;
  
  Init();
}


ArgusInfo::~ArgusInfo()
{
  POI_DEBUG(12) "Deleting ArgusInfo struct ..." << debl;
}


void ArgusInfo::Init()
{
  iInitTime = CurrentTime();
  iToArgusBslices = 0;
  iFromArgusBslices = 0;
  iSendMessages = 0;
}


//  Configure loads configuration parameters relevant to Argus from
//  poi.ini.
void PoiTcp::Configure(const String & aInitPath, Config * aConfig)
{
  // explicitely specified machine id overrides that in testbed list
  iMachID = (BYTE) (int) iPtl->ConfigGet(aConfig, "ARGUS_MACHID", "0");
  
  // get timeout for connect() call
  iConnectTimeout = atol(iPtl->ConfigGet(aConfig, "CONNECT_TIMEOUT", "6"));
  iStatistics.iValue[CONNECT_TIMEOUT] = iConnectTimeout;
  
  // get polling and update frequencies
  double pfreq = atof(iPtl->ConfigGet(aConfig, "POLL_FREQ", "4"));
  
  if (pfreq < 0.1)
    pfreq = 0.1;
  if (pfreq > 1000)
    pfreq = 1000;
  
  double ufreq = atof(iPtl->ConfigGet(aConfig, "LOCAL_UPDATE_FREQ", "2"));
  
  if (ufreq < 0.1)
    ufreq = 0.1;
  if (ufreq > pfreq)
    ufreq = pfreq;
  
  double afreq = atof(iPtl->ConfigGet(aConfig, "ARGUS_UPDATE_FREQ", "1"));
  
  if (afreq < 0.01)
    afreq = 0.01;
  if (afreq > pfreq)
    afreq = pfreq;
  
  String help;
  
  if (gWantPumaPort == 0)	// PumaPort not specified via command line
  {
# ifdef MS_WINDOWS
    help = iPtl->ConfigGet(aConfig, "PUMA_PORT", "PC_LINK");
# else
    help = iPtl->ConfigGet(aConfig, "PUMA_PORT", "5050");
# endif
  }
  else
  {
    help = gWantPumaPort;
    POI_DEBUG(10)"PumaPort is set to "<<gWantPumaPort<<" by command line arg"
      <<debl;
  }
  
  if (help.ICompare("PC_LINK") == COMPARE_EQUAL)
    iPumaPort = PC_LINK;
  else if (help.ICompare("EXT_TCP") == COMPARE_EQUAL)
    iPumaPort = EXT_TCP;
  else
    iPumaPort = atoi(help);
  
  iArgusPort = iPtl->ConfigGet(aConfig, "ARGUS_LISTEN_PORT", "0");
  iConnectPort = iPtl->ConfigGet(aConfig, "ARGUS_CONNECT_PORT", "0");
  iBedsFile = iPtl->ConfigGet(aConfig, "BED_LIST", "beds.lst");
  iArgusRBS = iPtl->ConfigGet(aConfig, "ARGUS_RBS", "0");
  
  iArgusLicenseOK = 0;
  if (iConnectPort > 0)		// try to get an Argus license
  {
    String argusLicense = iPtl->ConfigGet(aConfig, "ARGUS_LICENSE", "0");
    iArgusLicenseOK = (argusLicense == String("rose"));
    
    if (! iArgusLicenseOK)
      POI_DEBUG(1)"Argus License in poi.ini file is invalid!"<<debl;
    
    if (! iArgusLicenseOK)
    {
      if (ArgusGetLicense() < 0)
      {
        POI_DEBUG(1) "PoiTcp::Configure: No Argus License available"<<debl;
      }
      else
        iArgusLicenseOK = 2;	// got license from FlexLM
    }
  }
  m_NrOfPSVSlices =  atol(iPtl->ConfigGet(aConfig, "NR_OF_PSV_SLICES", "100"));
  
  if(m_NrOfPSVSlices<=0 && m_NrOfPSVSlices>1000)
    m_NrOfPSVSlices = 100;
  
  iMaxNumBslices =  atol(iPtl->ConfigGet(aConfig, "MAX_MESSAGE_CNT", "10"));
  
# ifdef MS_WINDOWS
  char sep = '\\';
  
# else
  char sep = '/';
  
# endif
  
  if (iBedsFile.Search(sep) == STRING_NOTFOUND)
  {	const char *buff = aInitPath.GetStr();
  if (buff[aInitPath.Len() - 1] == sep)
    iBedsFile = aInitPath + iBedsFile;
  else
    iBedsFile = aInitPath + String(sep) + iBedsFile;
  }
  
  iPollFreq = pfreq;
  SetPollDelay();
  iUpdateDelay = (short) (pfreq / ufreq);
  iArgusDelay = (short) (pfreq / afreq);
  
  // fetch list of beds
  
  if (iConnectPort > 0)
  {
    if (InitBeds() < 0)
    {
      POI_DEBUG(10) "no test bed list found; remote monitoring disabled." << debl;
      iConnectPort = -1;
      //iStatistics.iValue[ARGUS_CONNECT_PORT] = iConnectPort;
    }
  }
  
  POI_DEBUG(10) "Poll Freq       " << pfreq << debl;
  POI_DEBUG(10) "Update Freq     " << ufreq << debl;
  POI_DEBUG(10) "Argus Freq      " << afreq << debl;
  POI_DEBUG(10) "Poll delay      " << iPollDelay << debl;
  POI_DEBUG(10) "Update Delay    " << iUpdateDelay << debl;
  POI_DEBUG(10) "Argus Delay     " << iArgusDelay << debl;
  POI_DEBUG(10) "Puma Port       " << iPumaPort << debl;
  POI_DEBUG(10) "Argus Port      " << iArgusPort << debl;
  POI_DEBUG(10) "Connect Port    " << iConnectPort << debl;
  POI_DEBUG(10) "Bed List        " << (const char *) iBedsFile << debl;
# ifndef MS_WINDOWS
  POI_DEBUG(10) "Default Request " << (const char *) defItem << debl;
# endif
  POI_DEBUG(10) "ArgusLicenseOK  " << iArgusLicenseOK << debl;
  
  iStatistics.iValue[POLL_DELAY] = iPollDelay;
  iStatistics.iValue[UPDATE_DELAY] = iUpdateDelay;
  iStatistics.iValue[PUMA_PORT] = iPumaPort;
  iStatistics.iValue[ARGUS_LISTEN_PORT] = iArgusPort;
  iStatistics.iValue[ARGUS_CONNECT_PORT] = iConnectPort;
}


void PoiTcp::SetPollDelay()
{
  iPollDelay = 0;
  
  if (iPollFreq < 0.1)
    return;
  
  if (NeedCM || iPumaPort == EXT_TCP || iPumaPort == ASYNC_PC_LINK)
    iPollDelay = (short) (1000 / iPollFreq);
}  

//  Init opens a listening port on aListenPort, through which Arguses may
//  connect to us.  aPtl is kept so that we can access PoiLinkWindow
//  services through it.  Also, a listening port aPumaPort is opened, in
//  case a PUMA wants to be controlled by us.
//  Init can be called more than once.  If called repeatedly, it closes
//  all open connections first.
short PoiTcp::Init(PoiTcpLink * aPtl)
{
  iPtl = aPtl;
  iPumaConn = -1;
  iPumaInit = 1;        	// if first bslice arrives, answer with init
  iPumaWriteOK = 0;     	// Puma is expected to send first bslice
  iIsDefault = 0;
  
  if (iPumaPort == PC_LINK || iPumaPort == ASYNC_PC_LINK)
    PumaInit();			// reset connection-specific variables
  
# ifdef unix
  // SIGPIPE may occur when Argus is killed; ignore it
  (void) signal(SIGPIPE, SIG_IGN);
  
  // alarm is used to interupt connect calls
  (void) signal(SIGALRM, alarmCb);
# endif
  
  // init end-of-message slice
  if (!iEOMslice)
    iEOMslice = new Slice(0, GENERAL_EVENT, END_OF_MESSAGE, 8);
  
  if (iConnectPort < 0)		// error in reading beds.lst has occured
    INFO(STR_NO_BEDS, TEXTID(STR_NO_BEDS));
  
  if (iCM)      // dump old connections
    Close();
  else
  {     // create CM only if any Tcp service is used
    if (iPollDelay > 0 && NeedCM)
    {
      int ports = (iConnectPort > 0) ? 250 : 10; // WO-3579
      POI_DEBUG(10) "creating ConnectionManager for "
        << ports << " connections" << debl;
      iCM = new ConnectionManager(ports);
      if (!iCM)
      {
        POI_DEBUG(1) "can't create ConnectionManager." << debl;
        return -1;
      }
    }
  }
   
  POI_DEBUG(10) "Local machine ID: " << (int) iMachID << debl;
  
  // listen on iArgusPort
  if (iCM && iArgusPort && iPumaPort != 0)      // if no Puma then no Argus
  {
    POI_DEBUG(10) "Listen on Argus port " << iArgusPort << " ..." << debl;
    if (iCM->AddConnection((int) iArgusPort, &::ArgusAccept) < 0)
    {
      POI_DEBUG(10) "AddConnection on port " << iArgusPort << " failed." << debl;
      POI_DEBUG(1) "Argus requests will fail." << debl;
      INFO(STR_NO_ARGUS_PORT, TEXTID(STR_NO_ARGUS_PORT));
    }
  }
  
  // listen on iPumaPort
  if (iCM && iPumaPort > 0)
  {
    POI_DEBUG(10) "Listen on Puma port " << iPumaPort << " ..." << debl;
    if (iCM->AddConnection((int) iPumaPort, &::PumaAccept) < 0)
    {
      POI_DEBUG(1) "AddConnection on port " << iPumaPort << " failed." << debl;
      INFO(STR_NO_PUMA_PORT, TEXTID(STR_NO_PUMA_PORT));
    }
  }
  
  // if there is an external Puma connection, initialize it and read our
  // machine id
  if (iPumaPort == EXT_TCP)
  {
    ExtTcpInit(&iMachID);
    POI_DEBUG(10) "ExtTcp assigned us machID " << (int) iMachID << debl;
    PumaInit();
  }
  
  iPtl->SetTimer(iPollDelay);
  
  if (gSnifOptions)             // snif Puma connection
    (void) Command("set snif snif.0");
  
  gTcp = this;
  return 0;
}


//  Close closes all open Tcp connections.
void PoiTcp::Close()
{
  // send any pending bslice from local Poi to Puma
  BufQueueElem *elem;
  
  while ((elem = iPumaQueue.Top(1)) != 0)
  {
    if (elem->iMachID == 0)
      (void) PumaWrite(elem->iBslice, elem->iLen, elem->iMachID);
    delete elem;
  }
  
  // if there is an external Puma connection, close it
  if (iPumaPort == EXT_TCP)
    ExtTcpClose();
  
  POI_DEBUG(10) "closing all Tcp connections ..." << debl;
  
  if (!iCM)
    return;
  
  for (Assoc * assoc = iPoiConn.First(); assoc; assoc = iPoiConn.Next())
    PoiClose(assoc->iConnID);
  
  if (iArgusLicenseOK == 2)	// got license from FlexLM
    ArgusReturnLicense();
  
  PumaClose();  // also closes Arguses
}


Bed *PoiTcp::GetBed(BYTE aMachID)
{
  if (! iBeds)
    return 0;
  
  for (Bed * bed = iBeds->First(); bed; bed = iBeds->Next())
    if (bed->MachID() == aMachID)
      return bed;
    return 0;
}


Bed *PoiTcp::GetBed(const char *aHost)
{
  if (! iBeds)
    return 0;
  
  for (Bed * bed = iBeds->First(); bed; bed = iBeds->Next())
    if (bed->HostName() == aHost)
      return bed;
    return 0;
}


//  WakeUp  checks all active sockets to see if any bslices arrived.  The
//  appropriate callbacks are invoked (ArgusRead, PoiRead, or PumaRead).
//  These callbacks route Argus slices through the local POI (ourselves),
//  and call TcpIn for bslices from Puma or remote Pois to the local Poi.
void PoiTcp::WakeUp()
{
  POI_DEBUG(50) "WakeUp ..." << debl;
  
  iStatistics.iValue[POLLS]++;
  
  if (iPumaPort == EXT_TCP)
    ExtTcpWakeUp();
  
  if (iCM)
    (void) iCM->Check(0);       // handle input, queue output
  
  // in asyncronous ServWin communication, we don't enqueue empty request.
  // if we don't have a request blice enqueued, and POI has some new request
  // slices, then we enqueue them now.
  if (iPumaPort == ASYNC_PC_LINK && iPumaWriteOK)
  {
    RequestList* list  = GetRequestList(0);
    RequestList* list1 = GetSlowRequestList(0);
    
    if (iPumaQueue.ContainsRequest(0) == 0 && ((list && list->Count() > 0) || (list1 && list1->Count() > 0))) // WO-3579
    {
      POI_DEBUG(12) "PoiTcp::WakeUp enqueues new requests" << debl;
      iPtl->PoiCommand(WRITE_REQUESTS, 0);
      //WriteRequests(0, 1);	// collect new request and enqueue
      return;
    }
  }
  
  TryPumaWrite();		// send bslice to Puma, if possible
  CheckPois();			// enqueue initial Poi requests
  TryPoisWrite();		// send ripe requests to remote Pois
}


//  BsliceStart  resets iOutBuf to zero, and sets iPoiMachID to aMachID.
//  subsequent calls to BsliceAdd append slices to iOutBuf.  A call to
//  BsliceWrite sends the iOutBuf to Poi iPoiMachID, if iPoiMachID > 0,
//  or to the Puma.
short PoiTcp::BsliceStart(BYTE aMachID)
{
  iOutBuf.SwitchTag(0);
  iPoiMachID = aMachID;
  iArgusMachID = InvalidMachID;
  
  if (ArgusMachID(aMachID))     // remember the conn. specific return address
  {
    PoiInfo *info = (PoiInfo *) Info(iPoiConn, aMachID);
    
    if (!info)
      return -1;
    
    iArgusMachID = info->iMachID;
  }
  else
    iArgusMachID = iMachID;     // 0, except when overwritten by EXT_TCP
  
  return 0;
}


//  BsliceAdd  appends the specified bslice to iOutBuf, provided it is not
//  a CHANGE_VALUE or KBIN slice, or disallowed for some other reason.
//  The MachID byte of the slice is replaced by iMachID, so Poi/Puma knows
//  where the response slice belongs.
void PoiTcp::BsliceAdd(Slice * aSlice)
{
  // make sure bslice length does not exceed the max number of a short.  we
  // can not use unsigned short because roberts connection manager uses a
  // int parameter for the bslice length, which is a short under MS_WINDOWS.
  // 960528: under windows, pack.dll restricts bslice size to 16000 bytes.
  if(!aSlice)
      return;
  if ((long) iOutBuf.GetLength() +
    (long) aSlice->GetCurrentWritePos() + 16 >
#ifdef MS_WINDOWS
    16000
#else
    0x7FFFL
#endif
    )
  {
    POI_DEBUG(2) "PoiTcp::BsliceAdd: max bslice length reached; "
      << "additional request ignored." << debl;
    POI_DEBUG(11) "  Len=" << aSlice->GetCurrentWritePos()
      << ", tag=" << aSlice->GetEventId()
      << ", stag=" << aSlice->GetSubEventId()
      << debl;
    return;
  }
  
  if (iPoiMachID && IsSliceOk(aSlice) < 0)      // to remote POI: check slice
    return;
  
  BYTE sendMach = aSlice->GetOITag();
  
  aSlice->PutOITag(iArgusMachID);
  iOutBuf.PutSlice(*aSlice);
  aSlice->PutOITag(sendMach);   // restore machine ID
}


//  BsliceWrite is called by the method CollectRequests to close the request
//  bslice in iOutBuf.
short PoiTcp::BsliceWrite()
{
  BOOL isEmpty = iOutBuf.IsEom();
  if (isEmpty)
    return 0;
  
  iEOMslice->PutOITag(iArgusMachID);
  iOutBuf.PutSlice(*iEOMslice); // append end-of-message slice
  iOutBuf.LengthToHeader();     // write bslice size into first two bytes
  
  short len = iOutBuf.GetLength() + 2;
  POI_DEBUG(30)"BsliceWrite: len="<<len<<", mach="<<iPoiMachID<<debl;
  
  return len;
}

void PoiTcp::RegisterSlowRequest(Slice * aSlice, short aMode)
{
  BYTE machID = aSlice->GetOITag();
  long len = aSlice->GetCurrentWritePos();
  BYTE tag = aSlice->GetEventId();
  BYTE subtag = aSlice->GetSubEventId();
  
  RequestList *list = GetSlowRequestList(machID);
  
  if (!list)  // this Poi is not connected
  {
    POI_DEBUG(45) "request slice dumped; machID = " << (int) machID << debl;
    iUrgent = 0;
    return;
  }
    
  if (!aSlice->iRequestRegistered)
  {
    if (len > 1000)
    {
      POI_DEBUG(11) "PoiTcp::RegisterSlowRequest: very long slice." << debl;
      POI_DEBUG(12) "  Len=" << len << ", tag=" << (int)tag
        << ", stag=" << (int)subtag << debl;
    }
    
    if (tag == PSV_EVENT && subtag == INIT_NORMVALBUFFER)
    {
      POI_DEBUG(25) "InitRingbuffer slice, len=" << len  << debl;
    }
    
    if (aMode == 1)		// insert in front
      list->Insert(aSlice);
    else
      list->Insert(aSlice, LIST_APPEND);
    
    aSlice->iRequestRegistered = TRUE;
  }
  
  if (tag == KBIN_EVENT ||
    (tag == PSV_EVENT &&
    subtag == CHANGE_VALUE
    || subtag == CHANGE_ARRAY))
  {
    iUrgent = list->Count() + 1;  // express service for pending bslices // WO-3579
    iPtl->PoiCommand(WAKEUP, 0);// try to send request immediately
  }
}

//  RegisterRequest is called (directly or via PoiLinkWindow::RegisterRequest)
//  when TcpIn distributes data from Puma or some remote Poi to the data
//  sources.  The request slices are kept in connection-specific lists (one
//  for each remote Poi, and one for the local Puma).  At the end of the
//  update cycle, TcpIn calls WriteRequests, which then collects all the
//  requests in the specified list and sends them to Puma or the remote Poi.
void PoiTcp::RegisterRequest(Slice * aSlice, short aMode)
{
  BYTE machID = aSlice->GetOITag();
  long len = aSlice->GetCurrentWritePos();
  BYTE tag = aSlice->GetEventId();
  BYTE subtag = aSlice->GetSubEventId();
  
  RequestList *list = GetRequestList(machID);
  
  if (!list)  // this Poi is not connected
  {
    POI_DEBUG(45) "request slice dumped; machID = " << (int) machID << debl;
    iUrgent = 0;
    return;
  }
    
  if (!aSlice->iRequestRegistered)
  {
    if (len > 1000)
    {
      POI_DEBUG(11) "PoiTcp::RegisterRequest: very long slice." << debl;
      POI_DEBUG(12) "  Len=" << len << ", tag=" << (int)tag
        << ", stag=" << (int)subtag << debl;
    }
    
    if (tag == PSV_EVENT && subtag == INIT_NORMVALBUFFER)
    {
      POI_DEBUG(25) "InitRingbuffer slice, len=" << len  << debl;
    }
    
    if (aMode == 1)		// insert in front
      list->Insert(aSlice);
    else
      list->Insert(aSlice, LIST_APPEND);
    
    aSlice->iRequestRegistered = TRUE;
  }
  
  if (tag == KBIN_EVENT ||
    (tag == PSV_EVENT &&
    subtag == CHANGE_VALUE
    || subtag == CHANGE_ARRAY))
  {
    iUrgent = list->Count() + 1; // express service for pending bslices // WO-3579
    iPtl->PoiCommand(WAKEUP, 0);// try to send request immediately
  }
}


//  RemoveRequest removes the specified request from it's request list
void PoiTcp::RemoveRequest(Slice * aSlice)
{
  BYTE machID = aSlice->GetOITag();
  RequestList *list = GetRequestList(machID);
  
  if (!list) // this Poi is not connected
  {
    if (aSlice->iRequestRegistered)
    {
      aSlice->iRequestRegistered = FALSE;
      POI_DEBUG(14) "can't remove request (Poi not connected); machID = "
        << (int) machID << debl;
    }
    return;
  }
  
  aSlice->iRequestRegistered = FALSE;
  (void) list->Remove(aSlice);
  /*********same for slow requests**************/
  
  RequestList *list1 = GetSlowRequestList(machID);
  if (!list1) // this Poi is not connected
  {
    if (aSlice->iRequestRegistered)
    {
      aSlice->iRequestRegistered = FALSE;
      POI_DEBUG(14) "can't remove request (Poi not connected); machID = "
        << (int) machID << debl;
    }
    return;
  }
  
  aSlice->iRequestRegistered = FALSE;
  (void) list1->Remove(aSlice);
}


//  WriteRequests is called by PoiLinkWindow::WriteRequests, which is turn is
//  called by TcpIn.  It sends the request in the specified list to the
//  appropriate receiver.  It returns the number of requests from the
//  DataSourceTable.
short PoiTcp::WriteRequests(BYTE aMachID, short aDelay)
{
  // get number of DS requests
  //short count = 0;
  //for (Slice * slice = list->First(); slice; slice = list->Next())
  //if (slice->GetReceiver() == DATASOURCE_ID)
  //count++;
  
  // Enqueue a token
  POI_DEBUG(30) "PoiTcp::WriteRequests enqueues token for machID "
    << (int) aMachID << debl;
  
  short delay = aDelay ? aDelay : iUpdateDelay;
  SliceBuff token(2, 0, FALSE);
  
  if (PumaMachID(aMachID))
    (void)PumaEnqueue(&token, delay, aMachID);
  else
    (void)PoiEnqueue(&token, delay, aMachID);
  
  if (iUrgent > 0)              // if urgent data is pending
    iPtl->PoiCommand(WAKEUP, 0);// try to send request immediately
  
  return 1;
}

BOOL  PoiIsValidAddress(const void* lp, UINT nBytes,
	BOOL bReadWrite /* = TRUE */)
{
	// simple version using Win-32 APIs for pointer validation.
	return (lp != NULL && !IsBadReadPtr(lp, nBytes) &&
		(!bReadWrite || !IsBadWritePtr((LPVOID)lp, nBytes)));
}
//////////////////////////////////////////////////////////////////////////////
// Pack all requests in the queue for machId into iOutBuf.
//
short PoiTcp::CollectRequests(BYTE aMachID)
{
  RequestList *list = GetRequestList(aMachID);
  
  if (!list)    // this Poi is not connected
  {
    POI_DEBUG(3) "can't write requests (Poi not connected); machID = "
      << (int) aMachID << debl;
    return 0;
  }
  
  BsliceStart(aMachID);
  int cnt = 0;
  for (Slice * slice = list->First(); slice; slice = list->Next())
  {
    BsliceAdd(slice);
    slice->iRequestRegistered = FALSE;
    cnt++;
  }
  list->Clear();        // kick out all requests
  
  POI_DEBUG(15) "Nr. of Requests sent: " << (int) cnt << debl;
  
  cnt = 0;
  RequestList *list1 = GetSlowRequestList(aMachID);
  if(list1)
  {
    int nrEntry = list1->Count();
    if(nrEntry < m_NrOfPSVSlices)
    {
      for (Slice * slice = list1->First(); slice; slice = list1->Next())
      {
        BsliceAdd(slice);
        slice->iRequestRegistered = FALSE;
        cnt++;
      }
      list1->Clear();        // kick out all requests
    } else
    {
      while(cnt < m_NrOfPSVSlices)
      {
        Slice * slice = list1->First();
        if(slice)
        {
          if(PoiIsValidAddress(slice,sizeof(Slice),TRUE))
          {
            slice->iRequestRegistered = FALSE;
            BsliceAdd(slice);         
          } else
          {
            POI_DEBUG(15) "!!!Error address of slice not valid->discard " << debl;
          }
          list1->Remove(slice);
  	  	  cnt++;
        } else
        {
          list1->Clear();        // kick out all requests
        }
      }
    }
  }
  
  POI_DEBUG(15) "Nr. of SLOW Requests sent: " << (int) cnt << debl;
  POI_DEBUG(15) "Nr. of SLOW Requests waiting in queue: " << (int) list1->Count() <<" Max.Req."<<m_NrOfPSVSlices<<debl;
  
  
  short len = BsliceWrite();
  
  return len;
}


//  ForceExtWrite makes sure that a request bslice is sent to Puma via
//  PumaExtWrite.  ForceExtWrite is called by PumaBsliceHandle or
//  TcpBsliceWrite if we are connected to Puma via ServWin.  This is
//  neccessary, since ServWin would stop talking to us if we don't pass it
//  a new request.
void PoiTcp::ForceExtWrite()
{
  BufQueueElem *elem = iPumaQueue.Top(1);       // force elem to be popped
  if (!elem)    // send EOM
  {
    POI_DEBUG(5) "ForceExtWrite: have to send EOM" << debl;
    iOutBuf.SwitchTag(0);
    iEOMslice->PutOITag(iMachID);
    iOutBuf.PutSlice(*iEOMslice);       // append end-of-message slice
    iOutBuf.LengthToHeader();   // write bslice size into first two bytes
    (void) PumaWrite(iOutBuf.GetReadBuffer(), iOutBuf.GetLength() + 2, 0);
  }
  else
  {
    (void) PumaWrite(elem->iBslice, elem->iLen, elem->iMachID);
    delete elem;
  }
}


short PoiTcp::ArgusAccept(short aConnID)
{
  POI_DEBUG(10) "Accepting Argus connection " << aConnID << debl;
  
  if (aConnID < 1 || aConnID > 255)
  {
    POI_DEBUG(1) "ArgusAccept: invalid connection index not accepted: "
      << aConnID << debl;
    iCM->Close(aConnID);
    return -1;
  }

  // WO-3579
  if (iCM->NumFreeEntries() < 1)  // need at least one spare
  {
    POI_DEBUG(1) "ArgusAccept: Max. number of ARGUS connections reached - connection denied" 
      << debl;
    iCM->Close(aConnID);
    return -1;
  }
  
  // read remote Tcp version
  char head[2];
  
  if (iCM->Read(aConnID, head, 2) != 2)
  {
    POI_DEBUG(1) "PoiTcp::ArgusAccept: Argus does not send TCP version." << debl;
    iCM->Close(aConnID);
    return -3;
  }
  
  int len = SLICE_SIZE(head);
  
  if (len > 32)
  {
    POI_DEBUG(1) "PoiTcp::ArgusAccept: FATAL: insufficient buffer size." << debl;
    iCM->Close(aConnID);
    return -100;
  }
  
  char mess[32];
  
  if (iCM->Read(aConnID, mess, len) != len)
  {
    POI_DEBUG(1) "PoiTcp::ArgusAccept: Error reading  TCP version." << debl;
    iCM->Close(aConnID);
    return -3;
  }
  
  if (len < 4)  // long
  {
    POI_DEBUG(10) "PoiTcp::ArgusAccept: Argus failed to send TCP version." << debl;
    iCM->Close(aConnID);
    return -3;
  }
  long argusVersion = swap_l((char*)mess);
  
  BYTE argusMachID = aConnID + 1;
  
  if (!ArgusMachID(argusMachID))
  {
    POI_DEBUG(10) "Can't accept Argus; assigned machID " << (int) argusMachID
      << " is invalid" << debl;
    iCM->Close(aConnID);
    return -4;
  }
  
  POI_DEBUG(10) "Accepting Argus version " << argusVersion
    << "; assigned MachID is " << (int) argusMachID << debl;
  
  // send up version and assigned machID
  // length  tag version         machid
  BYTE ack[8];
  
  memcpy(ack, "\000\006\000\000\000\000\000\000", 8);
  long hvers = swap_l((char*)&iProtocolVersion);
  memcpy(ack + 3, &hvers, 4);
  ack[7] = argusMachID;
  
  if (BslyteWrite(ack, 8, *iCM, aConnID) < 0)
  {
    POI_DEBUG(15) "PoiTcp::ArgusAccept: send acknowledge for connect failed."
      << debl;
    iCM->Close(aConnID);
    return -4;
  }
  
  iCM->SetCallback(aConnID, ConnectionManager::CM_READ, &::ArgusRead);
  iCM->SetCallback(aConnID, ConnectionManager::CM_EXCEPTION, &TcpException);
  
  ArgusInfo *info = new ArgusInfo;
  info->Init();
  info->iProtocolVersion = argusVersion; 
  CreateAssoc(iArgusConn, argusMachID, aConnID, info);
  
  return 0;
}


//  ArgusRead reads a bslice from Argus.  If the bslice contains an internal
//  command (like a request to send the NormNames), it is processed locally.
//  Otherwise the bslice is forwarded to Puma.  If the Puma is offline, the
//  connection to the Argus is closed.  It is the reponsibility of the Argus
//  to try to reconnect with a reasonable frequency.
short PoiTcp::ArgusRead(short aConnID)
{
  POI_DEBUG(50) "reading Argus connection " << aConnID << debl;
  
  // read bslice from Argus
  if (BsliceRead(iBuf, *iCM, aConnID) < 0)      // Argus has failed us
  {
    ArgusClose(aConnID);
    return -2;
  }
  
  BYTE mach = MachID(iArgusConn, aConnID);
  long len = iBuf.GetLength() + 2;
  POI_DEBUG(15) "      Argus " << (int) mach << " read ["
    << len << "]" << debl;
  
  if (len < 3)
  {
    POI_DEBUG(3) "PoiTcp::ArgusRead: Invalid bslice;  len = " << len << debl;
    return -4;
  }
  
  if (iBuf.IsEom())		// internal command
  {
    BYTE command = *(iBuf.GetReadBuffer() + 2);
    return ArgusInternalCommand(mach, command);
  }
  
  ArgusInfo* info = (ArgusInfo*) Info(iArgusConn, mach);
  if (!info)			// this Argus is not connected
    return -99;
  
  if (info->iPendingCommand)	// this is the argument to a previous command
    return ArgusInternalCommand(mach, ARG_COMMAND);
  
  // check that Puma is available
  if (!IsPumaConnected())       // close the connection; let Argus retry
  {
    POI_DEBUG(10) "PoiTcp::ArgusRead: Puma is offline." << debl;
    ArgusClose(aConnID);
    return -3;
  }
  
  // forward bslice to Puma
  POI_DEBUG(15) "Puma enqueue for Argus " << (int) mach << debl;
  
  iStatistics.iValue[FROM_ARGUS_BSLICES]++;
  iStatistics.iValue[FROM_ARGUS_BYTES] += len;
  
  info->iNumBslices = 0; // reset max pending bslice count
  
  return PumaEnqueue(&iBuf, iArgusDelay, mach);
}


//  ArgusWrite delivers the specified bslice to Argus aMachID.
short PoiTcp::ArgusWrite(BYTE aMachID, SliceBuff * aBuf)
{
  short conn = ConnID(iArgusConn, aMachID);
  
  if (conn < 0)
  {
    POI_DEBUG(1) "PoiTcp::ArgusWrite: no connection to Argus "
      << (int) aMachID << debl;
    return -1;
  }
  
  long len = aBuf->GetLength() + 2;
  POI_DEBUG(15) "Argus " << (int) aMachID << " write [" << len << ']' << debl;
  
  iStatistics.iValue[TO_ARGUS_BSLICES]++;
  iStatistics.iValue[TO_ARGUS_BYTES] += len;
  
  if (BslyteWrite(aBuf->GetReadBuffer(), (int) len, *iCM, conn) < 0)
  {
    POI_DEBUG(10) "PoiTcp::ArgusWrite: write to Argus failed." << debl;
    ArgusClose(conn);
    return -4;
  }
  return 0;
}


short PoiTcp::PumaAccept(short aConnID)
{
  if (iPumaConn >= 0)   // is this another Puma, or a retry ?
  {
    if (iPtl->IsHostConnected()) // looks like a second Puma
    {
      POI_DEBUG(10) "Ignoring Puma connect attempt (connection already exists)" << debl;
      iCM->Close(aConnID);
      return -1;
    }
    else			// we lost our connection, but Tcp is still
    {				// trying;  kill old connection, accept new
      POI_DEBUG(10) "Closing old Puma connection (seems dead)" << debl;
      PumaClose();
    }
  }
  
  POI_DEBUG(10) "Accepting Puma connection " << aConnID << debl;
  
  iCM->SetCallback(aConnID, ConnectionManager::CM_READ, &::PumaRead);
  iCM->SetCallback(aConnID, ConnectionManager::CM_EXCEPTION, &TcpException);
  
  iPumaConn = aConnID;  // remember connection
  PumaInit();
  
  return 0;
}


void PoiTcp::PumaInit()
{
  iPumaInit = 1;        // if first bslice arrives, answer with init
  iPumaWriteOK = 0;     // Puma is expected to send first bslice
  
  if (iPumaPort == ASYNC_PC_LINK)
  {
    iPumaInit = 0;        // no need to init at first slice
    iPumaWriteOK = 1;     // we need to send first bslice
  }
  
  if (iWriteRequests.Count() > 0)
  {
    POI_DEBUG(3) "PumaInit: Warning: non-zero request count ("
      << iWriteRequests.Count() << ")" << debl;
    
      /*  don't use slice pointers;  they may be invalid
      for (Slice * slice = iWriteRequests.First(); slice;
      slice = iWriteRequests.Next())
      slice->iRequestRegistered = FALSE;
    */
    
    iWriteRequests.Clear();
  }
  
  iLastMachID = 0;      // The first Puma slice belongs to us
  
  iStatistics.Init();
}


//  PumaRead is invoked by the Connection Manager when input from Puma is
//  available.  The bslice is read into the iBuf buffer, and analysed via
//  PumaBsliceHandle().
short PoiTcp::PumaRead()
{
  long len;
  
  if ((len = BsliceRead(iBuf, *iCM, iPumaConn, 1)) < 0)
  {
    POI_DEBUG(10) "PoiTcp::PumaRead: read from Puma failed." << debl;
    PumaClose();
    return -1;
  }
  
  iStatistics.iValue[FROM_PUMA_BSLICES]++;
  iStatistics.iValue[FROM_PUMA_BYTES] += len;
  
  return PumaBsliceHandle(&iBuf);
}


//  PumaBsliceHandle is called by PumaRead, when a bslice arrives from Puma
//  via Tcp, or by the PoiLinkWindow method PerformLinkWndProc, when a bslice
//  arrives from ServWin.
//  The MachID byte of the first PSV slice contains the machine id of the
//  receiver of the bslice.  The machine id is zero, if the bslice belongs to
//  us, and nonzero, if it has to be handed to an Argus.  A special case are
//  asynchronous events, which are always handed to the local POI.  From
//  synchronous bslices, all slices of general interest (like date/time) are
//  extracted and sent to all connected Arguses.
short PoiTcp::PumaBsliceHandle(SliceBuff * aBuf)
{
  //  prepare bslice for general interest slices
  if(!iPtl)
    return 0;
  short len = aBuf->GetLength() + 2;
  
  if (iSnifFile >= 0)
    LogBslice(aBuf->GetReadBuffer(), len - 2, "C");
  
  SliceBuff general(len);
  general.SwitchTag(1);
  general.PutTag(TAG_ASYNC);
  
  // async bslices are always local
  if (aBuf->Async())
  {
    BYTE ack[3];
    memcpy(ack, "\000\001\000", 3);
    
    POI_DEBUG(15) "      Puma ASYNC read [" << len << "]" << debl;
    
    // distribute general interest slices to Arguses
    ExtractAndSendGeneral(aBuf, &general); 
    
    // pass async bslice to Poi
    iPtl->TcpIn(aBuf, 0);
    
    if (iPumaConn >= 0) // Tcp Puma: send acknowledge
    {
      if (BslyteWrite(ack, 3, *iCM, iPumaConn) < 0)
      {
        POI_DEBUG(15) "PoiTcp::PumaRead: send acknowledge for async bslice failed."
          << debl;
        PumaClose();
        return -1;
      }
      iPumaWriteOK = 0;
    }
    else if (iPumaPort == EXT_TCP) // also reply
    {
      iPumaWriteOK = 0;
      return ExtTcpWrite(ack);
    }
    
    return 0;
  }
  
  // below this point, we know its a synchronous slice
  iPumaWriteOK = 1;
  BYTE machID = iLastMachID;
  
  POI_DEBUG(15) "      Puma read for " << (int) machID << " [" << len << "]" <<
    (iIsDefault ? " (defaultResponse)" : "") << debl;
  
  if (machID == InvalidMachID)  // this is for an Argus that is already closed
  {
    POI_DEBUG(10) "Dumping bslice for invalid machID (Argus closed)" << debl;
    return 0;
  }
  
  // initialize Poi when first bslice arrives for it
  if (iPumaInit == 1 &&	// dump first slice; it's only date/time
    PumaMachID(machID))
  {
    iPumaInit = 0;
    iPtl->TcpStart(0);
    return 0;
  }
  
  // take shortcut if no Arguses are connected, and bslice is for Puma.
  // If an Argus has just disconnected, this could be the answer to his last
  // request.  If so, don't take the shortcut.
  // If bslice is response to defaultRequest, we must extract date/time
  if (iArgusConn.Count() == 0 && PumaMachID(machID) && !iIsDefault)
  {
    iPtl->TcpIn(aBuf, 0);
    return 0;
  }
  
  Clice clice;
  
  aBuf->ResetRead();
  
  // find out receiver of bslice and extract slices for the rest
  if (ArgusMachID(machID))      // for Argus:
  {     // aBuf to Argus, general to local and other Arguses
    POI_DEBUG(20) "SYNC bslice for Argus " << (int) machID << debl;
    ExtractAndSendGeneral(aBuf, &general); 
    
    // all slices with machID = 0 or 255 go to local Poi
    general.ResetWrite();
    for (aBuf->First(clice); clice.Start(); aBuf->Next(clice))
      if (!ArgusMachID(clice.MachID()))
        general.PutClice(clice);
      
      general.LengthToHeader();   // write bslice size into first two bytes
      general.PutTag(TAG_ASYNC);
      if (!general.IsEom())
      {
        iPtl->TcpIn(&general, 0);
      }
  }
  
  if (PumaMachID(machID))       // for local Poi:
  {     // aBuf to local, general to Arguses
    
    // all slices of general interest go to Arguses
    ExtractAndSendGeneral(aBuf, &general);
    
    if (iIsDefault)     // send aBuf as async slice
      aBuf->PutTag(TAG_ASYNC);  // fake async; we need not generate requests
    
    iPtl->TcpIn(aBuf, 0);
  }
  
  // If we talk to ServWin, and no synchronous slice has been sent to Puma
  // in the above code, we must do it now.
  if (iPumaPort == PC_LINK && iPumaWriteOK)
    ForceExtWrite();
  
  return 0;
}


short PoiTcp::PumaBslyteHandle(BYTE * aBslyte)
{
  USHORT len = SLICE_SIZE(aBslyte);
  
  iBuf.SwitchTag(TRUE);
  iBuf.MakePlace(len, aBslyte + 2);
  return PumaBsliceHandle(&iBuf);
}



//////////////////////////////////////////////////////////////////////////////
//  ExtractAndSendGeneral is called from PumaBsliceHandle.  It parses the
//  incoming bslice aBuf for slices of interest for each connected ARGUS,
//  puts those slices into the output bslice aOut, and sends aOut to the
//  Argus. 
void PoiTcp::ExtractAndSendGeneral(SliceBuff* aBuf, SliceBuff* aOut)
{
  Clice clice;
  
  for (Assoc* assoc = iArgusConn.First(); assoc; assoc = iArgusConn.Next())
  {
    BYTE machID = assoc->iMachID;
    ArgusInfo* info = (ArgusInfo*)(assoc->iInfo);
    
    if (iMaxNumBslices > 0 && info->iNumBslices > iMaxNumBslices)
    {
      if (info->iNumBslices == iMaxNumBslices + 1)
      {
        POI_DEBUG(10) "Stopping to forward messages to ARGUS " 
          << (int)(assoc->iMachID) << " (MAX_MESSAGE_CNT reached)" << debl;
      }
      info->iNumBslices++;
      continue;
    }
    
    BOOL needMess = (info && info->iSendMessages);
    
    aOut->ResetWrite();
    
    for (aBuf->First(clice); clice.Start(); aBuf->Next(clice))
      if (clice.MachID() == machID
        || (needMess && clice.Tag() == MESSAGE_EVENT)
        || (clice.MachID() == BROADCAST && clice.Tag() != MESSAGE_EVENT)
        || (clice.Tag() == GENERAL_EVENT && clice.SubTag() == PUMA_STATE) )
        aOut->PutClice(clice);
      
      aOut->LengthToHeader();	// write bslice size into first two bytes
      
      if (machID == iLastMachID && ! aBuf->Async())
      {
        aOut->PutTag(TAG_SYNC);	// this is the response for our last request
        (void) ArgusWrite(machID, aOut);
        info->iNumBslices++; 
      }
      else			// just send it asynchronously
      {
        if (!aOut->IsEom())
        {
          aOut->PutTag(TAG_ASYNC);
          (void) ArgusWrite(machID, aOut);
          info->iNumBslices++; 
        }
      }
  }
}


void PoiTcp::SendToArguses(SliceBuff* aBuf)
{
  aBuf->LengthToHeader();	// write bslice size into first two bytes
  
  if (!aBuf->IsEom())
  {
    POI_DEBUG(40) "SendToArguses" << debl;
    for (Assoc* assoc = iArgusConn.First(); assoc; assoc = iArgusConn.Next())
    {
      (void) ArgusWrite(assoc->iMachID, aBuf);
    }
  }
}


//  PumaEnqueue puts aBuf into the output queue iPumaQueue.  Every time
//  Puma is ready to accept a bslice, the first element of the queue is
//  sent to it.
//  The maximal length of the queue is  1+ConnectedArgusses.
short PoiTcp::PumaEnqueue(SliceBuff * aBuf, short aDelay, BYTE aMachID)
{
  // check that Puma is available
  if (!IsPumaConnected())
  {
    POI_DEBUG(10) "PoiTcp::PumaEnqueue: Puma is offline." << debl;
    return -1;
  }
  
  iPumaQueue.Remove(aMachID);	// if bslice is already enqueued, overwrite it
  
  // The Puma queue may have at most one entry for each connected Argus,
  // and one for the local Poi.
  if (iPumaQueue.Count() > iArgusConn.Count())
  {
    POI_DEBUG(1) "PumaEnqueue: ERROR: Queue Length is " << iPumaQueue.Count()
      << " (connected Arguses: " << iArgusConn.Count() << ")" << debl;
    return -1;
  }
  
  iPumaQueue.Insert(*aBuf, aDelay, aMachID);
  return 0;
}


//  PoiEnqueue puts aBuf into the output queue iPoiQueue.  Each queue entry
//  has a delay couter, that is decremented during wakeup.  when the counter
//  reaches 0, the bslice is sent to its Poi.
short PoiTcp::PoiEnqueue(SliceBuff * aBuf, short aDelay, BYTE aMachID)
{
  PoiInfo *info = (PoiInfo *) Info(iPoiConn, aMachID);
  
  if (!info)
  {
    POI_DEBUG(10) "PoiEnqueue: Poi " << (int) aMachID << " is not connected" << debl;
    return -2;
  }
  
  // The Poi queue may have at most one entry for each connected Poi
  if (iPoiQueue.Count() >= iPoiConn.Count())
  {
    POI_DEBUG(3) "PoiEnqueue: ERROR: Queue Length is " << iPoiQueue.Count()
      << " (connected Pois: " << iPoiConn.Count() << ")" << debl;
    return -1;
  }
  
  iPoiQueue.Insert(*aBuf, aDelay, aMachID);
  info->iPending = 1;
  return 0;
}


//  QueueClear empties the output queue for Puma.  It is called by
//  PoiLinkWindow::TerminateCommunication before Poi registers it's last
//  request slice.  The last request slice contains clean-up slices.  By
//  emptying the queue, Poi makes sure that this bslice is actually
//  transmitted to Puma when Tcp is closed.
void PoiTcp::QueueClear()
{
  iPumaQueue.Clear();
  iPoiQueue.Clear();
}


//  TryPumaWrite  is called during WakeUp.  It decrements the delay
//  counter of all enqueued bslices, and sends the first available bslice
//  to the Puma, provided Puma is ready to accept input.
//  The purpose of the delay counter is to keep the frequency of data
//  request to Puma low, while running a high polling rate.  A high
//  polling rate is necessary to display messages from Puma with low
//  delay.  For this a polling rate of at least 5 Hertz is necessary.  On
//  the other hand, for data update it is frequently sufficient to use a
//  lower polling rate.  A bslice with a delay count of 3 is kept is the
//  output queue for at least 3 tics (calls to WakeUp), before it is
//  released to Puma.
void PoiTcp::TryPumaWrite()
{
  iPumaQueue.DecrDelay();
  
  if (! (iPumaWriteOK && (iSysvalid || iUrgent > 0)))
    return;
  
  short force = 0;
  if (iUrgent > 0)		// this or next bslice needs fast delivery
  {
    force = 1;
  }
  
  BufQueueElem *elem = iPumaQueue.Top(force);
  
  if (! elem)
    return;
  
  BYTE machId = elem->iMachID;
  short len = elem->iLen;
  BYTE* bslice = elem->iBslice;
  
  if (len == 2)                 // this is a token; collect requests first
  {
    len = CollectRequests(machId);
    bslice = iOutBuf.GetReadBuffer();
  }
  
  if (len > 2)                  // write bslice if non-empty
  {
    (void) PumaWrite(bslice, len, machId);
    
    if (PumaMachID(machId))
    {
      if (iUrgent > 0)
        iUrgent--;
      iPtl->PoiCommand(CLEAR_REQUESTS, (Slice*)1); // clear special requests
    }
  }
  
  delete elem;
}


//  TryPoisWrite  is called during WakeUp.  It decrements the delay
//  counter of all enqueued bslices, and sends all available bslices
//  to the remote Pois.
void PoiTcp::TryPoisWrite()
{
  iPoiQueue.DecrDelay();
  
  BufQueueElem *elem;
  
  while ((elem = iPoiQueue.Top()) != 0)
  {
    // this is a token; collect requests first
    (void)CollectRequests(elem->iMachID);
    BYTE* bslice = iOutBuf.GetReadBuffer();
    short len = iOutBuf.GetLength() + 2;
    
    (void) PumaWrite(bslice, len, elem->iMachID);
    delete elem;
  }
}


//  CheckPois is called during WakeUp.  For every connected remote Poi, it
//  performs the following operations:
//  o  If there is an outstanding request, increment the iPending counter in
//     the PoiInfo struct, to keep track of the time it takes the Poi to
//     answer a request.
//  o  If there is no outstanding request, and the request list is non-empty,
//     then the request list is packed into a bslice and sent to the Poi.
//     This can only happen if the first DS associated with the Poi was
//     created right now.
//     If there is an outstanding request, we wait for the response, and send
//     the requests in the list with the bslice that is generated in reaction
//     to the response.
void PoiTcp::CheckPois()
{
  for (Assoc * assoc = iPoiConn.First(); assoc; assoc = iPoiConn.Next())
  {
    PoiInfo *info = (PoiInfo *) assoc->iInfo;
    BYTE mach = assoc->iMachID;
    
    if (!info)			// should never happen
      continue;
    
    if (info->iPending)		// there is an outstanding request
      info->iPending++;
    
    if (info->iWriteRequests.Count() > 0 && info->iPending == 0)
    {
      POI_DEBUG(10) "Sending initial request to Poi " << (int) mach << debl;
      info->Init();
      WriteRequests(mach, 1);
    }
  }
}


//  PumaWrite sends the specified buffer to the Puma.  PumaWrite is called
//  from TryPumaWrite, which is again called from WakeUp.
//  The caller must make sure that Puma is connected and ready to receive
//  a bslice.
//  The Puma may be connected via Tcp or by an external connection (like
//  the PC-Link card.  In the latter case an external writer is called.
short PoiTcp::PumaWrite(BYTE * aBslice, short aLen, BYTE aMachID,
                        short aIsDefault)
{
  POI_DEBUG(15) "Puma write for " << (int) aMachID << " [" << aLen << "]" << debl;
  
  if (iSnifFile >= 0)
    LogBslice(aBslice, aLen-2, "S");
  
  iStatistics.iValue[TO_PUMA_BSLICES]++;
  iStatistics.iValue[TO_PUMA_BYTES] += aLen;
  if (PumaMachID(aMachID))
    iStatistics.iValue[UPDATES]++;
  
  // iPumaWriteOK = 0;
  PumaWriteRegister(aMachID, aIsDefault);
  
  if (iPumaPort == PC_LINK || iPumaPort == ASYNC_PC_LINK) // ServWin
    return iPtl->PumaExtWrite(aBslice);
  
  if (iPumaPort == EXT_TCP)     // Fwing
    return ExtTcpWrite(aBslice);
  
  if (iPumaConn < 0)
    return -1;
  
  if (BslyteWrite(aBslice, aLen, *iCM, iPumaConn) < 0)
  {
    POI_DEBUG(10) "PoiTcp::PumaWrite: write to Puma failed." << debl;
    PumaClose();
    return -1;
  }
  
  return 0;
}



//  PumaWriteRegister is called before a request is sent to Puma, no matter
//  what Puma connection is used.  It sets iLastMachID and iIsDefault so when
//  the response arrives, we know where to send it, and whether it is caused
//  by a default request.
void PoiTcp::PumaWriteRegister(BYTE aMachID, short aIsDefault)
{
  iPumaWriteOK = 0;
  iLastMachID = aMachID;
  iIsDefault = aIsDefault;
}


//  PoiConnect  finds out the host name of the specified bed and initiates a
//  connection to the POI on that host.
short PoiTcp::PoiConnect(BYTE aMachID)
{
  if (! (iConnectPort > 0) || !iCM)
  {
    POI_DEBUG(10) "remote monitoring is disabled." << debl;
    return -5;
  }
  
  /*
  this needs to be fixed !!!
  // don't allow to connect to myself
  if (aMachID == iMachID)
  {
  POI_DEBUG(10) "can't connect to myself." << debl;
  return -6;
  }
  */
  
  //  check the license
  if (! iArgusLicenseOK)
  {
    POI_DEBUG(1) "PoiTcp::PoiConnect: No Argus License available"<<debl;
    INFO(STR_NO_ARGUS_LICENSE, TEXTID(STR_NO_ARGUS_LICENSE));
    return -5;
  }
  
  // find the bed with aMachID
  Bed *bed = GetBed(aMachID);
  
  if (!bed)
  {
    POI_DEBUG(1) "PoiTcp::PoiConnect: No bed for MachID " << (short) aMachID << debl;
    return -1;
  }
  
  const char *host = bed->HostName();
  
  // if bed is already connected, disconnect first
  short connID = ConnID(iPoiConn, aMachID);
  
  if (connID >= 0)
  {
    POI_DEBUG(20) "Reconnecting to POI on host " << host << debl;
    PoiClose(connID);
  }
  
  POI_DEBUG(10) (int) iMachID << " is trying to monitor "
    << host << ':' << iConnectPort << debl;
  int loc = iCM->AddConnection(host, (int) iConnectPort);
  
  if (loc < 0)
  {
    POI_DEBUG(10) "AddConnection for " << host << " failed." << debl;
    return -2;
  }
  
# ifdef unix
  (void) alarm((int)iConnectTimeout); // time out connect() call
# endif
  
  short rc = iCM->Connect(loc);
  
# ifdef unix
  (void) alarm(0);		// cancel alarm clock
# endif
  
  if (rc < 0)
  {
    POI_DEBUG(10) "Connect to POI on " << host << " failed." << debl;
    iCM->Close(loc);
    return -3;
  }
  
  // send our version number to Poi
  BYTE vers[6];
  
  memcpy(vers, "\000\004", 2);
  long hvers = swap_l((char*)&iProtocolVersion);
  memcpy(vers + 2, &hvers, 4);
  
  if (iCM->Write(loc, (char*)vers, 6) < 0)
  {
    POI_DEBUG(1) "PoiTcp::PoiConnect: can't send TCP version to POI " <<
      iProtocolVersion << endl;
    iCM->Close(loc);
    return -4;
  }
  
  // read Poi version and assigned machine id
  char head[2];
  
  if (iCM->Read(loc, head, 2) != 2)
  {
    POI_DEBUG(1) "PoiTcp::PoiConnect: Poi does not send TCP version." << debl;
    iCM->Close(loc);
    return -5;
  }
  
  int len = SLICE_SIZE(head);
  
  if (len > 32)
  {
    POI_DEBUG(1) "PoiTcp::PoiConnect: FATAL: insufficient buffer size." << debl;
    iCM->Close(loc);
    return -100;
  }
  
  char mess[32];
  
  if (iCM->Read(loc, mess, len) != len)
  {
    POI_DEBUG(1) "PoiTcp::PoiConnect: Error reading TCP version, machID." << debl;
    iCM->Close(loc);
    return -5;
  }
  
  if (len < 6)			// 1 * tag + 4 * version + 1 * machID
  {
    POI_DEBUG(10) "PoiTcp::PoiConnect: Poi failed to send machID." << debl;
    iCM->Close(loc);
    return -6;
  }
  
  long poiVersion = swap_l((char*)(mess+1));
  POI_DEBUG(13) "connected to Poi (version " << poiVersion << ")" << debl;
  
  BYTE machID = mess[5];
  POI_DEBUG(14) "Poi assigned us machID " << (int) machID << debl;
  
  if (!ArgusMachID(machID))
  {
    POI_DEBUG(10) "PoiTcp::PoiConnect: Poi assigned invalid machID "
      << (int) machID << debl;
    iCM->Close(loc);
    return -7;
  }
  
  iCM->SetCallback(loc, ConnectionManager::CM_READ, &::PoiRead);
  iCM->SetCallback(loc, ConnectionManager::CM_EXCEPTION, &TcpException);
  
  PoiInfo *info = new PoiInfo;
  
  info->iMachID = machID;
  info->iProtocolVersion = poiVersion;
  CreateAssoc(iPoiConn, aMachID, (short) loc, info);
  iPtl->TcpPoiConnected(aMachID);
  iPtl->TcpStart(aMachID);      // create and send INIT_DATA events
  
  return 0;
}


//  PoiDisconnect releases the specified remote Poi and cleans up.
short PoiTcp::PoiDisconnect(BYTE aMachID)
{
  // check that bed is connected
  short connID = ConnID(iPoiConn, aMachID);
  
  if (connID < 0)
  {
    POI_DEBUG(10) "PoiDisconnect: Machine " << (int) aMachID << " not connected." << debl;
    return -1;
  }
  
  PoiClose(connID);
  return 0;
}


long PoiTcp::Info(short aWhat)
{
  switch (aWhat)
  {
  case PUMA_CONFIGURED:
    return (iPumaPort != 0);
  case ARGUS_CONFIGURED:
    return (iConnectPort > 0);
  default:
    POI_DEBUG(3) "PoiTcp::Info: unknown info category " << aWhat << debl;
  }
  
  return 0;
}


//  ArgusCommand sends the Argus command encoded in aComm to the specified;
//  Poi, provided it is connected and has at least protocol version aVersion. 
short PoiTcp::ArgusCommand(BYTE aMachID, BYTE* aComm, long aLen, long aVersion)
{
  if (aLen > 5)
  {
    POI_DEBUG(3) "Attempt to send command " << (int)*aComm << " to machID "
      << (int)aMachID << " failed; command longer then 5 bytes: "
      << aLen << debl;
    return -13;
  }
  
  PoiInfo *info = (PoiInfo *) Info(iPoiConn, aMachID);
  if (! info)
  {
    POI_DEBUG(3) "Attempt to send command " << (int)*aComm << " to machID "
      << (int)aMachID << " failed; testbed not connected." << debl;
    return -11;
  }
  
  if (info->iProtocolVersion < aVersion)
  {
    POI_DEBUG(3) "Attempt to send command " << (int)*aComm << " to machID "
      << (int)aMachID << " failed; testbed version is "
      << info->iProtocolVersion
      << " (" << aVersion << " is required)." << debl;
    return -12;
  }
  
  //  build request slice
  BYTE* bslyte = new BYTE[2 + aLen];
  bslyte[0] = 0;
  bslyte[1] = (BYTE)aLen;	// length
  for (int i = 0; i < aLen; i++)
    bslyte[2 + i] = aComm[i];
  (void) PoiWrite(bslyte, (short)(2 + aLen), aMachID);
  delete[] bslyte;
  
  return 0;
}


//  ArgPoiCommand sends the specified Argus command with the argument to Poi
//  via 'PoiCommand'.  An acknowledge bslice is returned to the argus upon
//  command completion.  The tag field of the acknowledge bslice is 0, if all
//  went well.
short PoiTcp::ArgPoiCommand(BYTE aMachID, BYTE aComm, const char* aArg)
{
  Slice slice(strlen(aArg) + 7);
  slice.AddString(aArg);
  short rc = iPtl->PoiCommand(aComm, &slice);
  SliceBuff ack(3);
  ack.SwitchTag(1);
  ack.PutTag((rc < 0) ? 255 : 0);
  ack.LengthToHeader();		// write bslice size into first two bytes
  (void) ArgusWrite(aMachID, &ack); // send acknowledgement
  return rc;
}


//  PoiRead is called by the ConnectionManager when a remote Poi has
//  delivered a bslice.  It passes the slice to the appropriate data
//  sources.
short PoiTcp::PoiRead(short aConnID)
{
  BYTE mach = MachID(iPoiConn, aConnID);
  PoiInfo *info = (PoiInfo *) Info(iPoiConn, mach);
  
  POI_DEBUG(15) "      Poi " << (int) mach << " read" << debl;
  
  if (!info || BsliceRead(iBuf, *iCM, aConnID, 1) < 0)
  {
    POI_DEBUG(10) "Closing Poi " << (int) mach << debl;
    PoiClose(aConnID);
    return -1;
  }
  
  iStatistics.iValue[FROM_POI_BSLICES]++;
  iStatistics.iValue[FROM_POI_BYTES] += (iBuf.GetLength() + 2);
  info->iFromPoiBslices++;
  
  if (!iBuf.Async())
    info->iPending = 0;
  
  iPtl->TcpIn(&iBuf, mach);     // handle data and send new requests
  
  return 0;
}


//  PoiWrite sends aBslice to the specified remote Poi.  PoiWrite is called
//  by TryPoisWrite, which in turn is called by WakeUp.
short PoiTcp::PoiWrite(BYTE * aBslice, short aLen, BYTE aMachID)
{
  short conn = ConnID(iPoiConn, aMachID);
  PoiInfo *info = (PoiInfo *) Info(iPoiConn, aMachID);
  
  if (conn < 0 || !info)
  {
    POI_DEBUG(1) "PoiTcp::PoiWrite: Poi " << aMachID << " is not connected." << debl;
    return -1;
  }
  
  POI_DEBUG(15) "Poi " << (int) aMachID << " write" << debl;
  
  iStatistics.iValue[TO_POI_BSLICES]++;
  iStatistics.iValue[TO_POI_BYTES] += aLen;
  info->iToPoiBslices++;
  
  if (BslyteWrite(aBslice, aLen, *iCM, conn) < 0)
  {
    POI_DEBUG(10) "Closing Poi " << (int) aMachID << debl;
    PoiClose(conn);
    return -1;
  }
  
  return 0;
}


//  IsSliceOk checks whether the specified slice may be passed to a remote
//  Poi.  At this time, any slice that influences the operation of the
//  realtime is disallowed.
short PoiTcp::IsSliceOk(Slice * aSlice)
{
  BYTE tag = aSlice->GetEventId();
  BYTE subtag = aSlice->GetSubEventId();
  
  if (tag == KBIN_EVENT)
    goto nogood;
  if (tag == PSV_EVENT)
  {
    if (subtag == CHANGE_VALUE)
      goto nogood;
    if (! iArgusRBS)
    {
      if (subtag == START_RBS || subtag == STOP_RBS || subtag == REMOVE_RBS)
        goto nogood;
    }
  }
  return 0;
  
nogood:
  POI_DEBUG(3) "IsSliceOk: dumping slice event " << (int) tag << "." << (int) subtag << debl;
  return -1;
}


//  Command  executes the specified command.
short PoiTcp::Command(char *aCommand)
{
  char command[128];
  char response[128];
  
  if (!aCommand)
    return -1;
  
  POI_DEBUG(20) "PoiTcp::Command(" << aCommand << ")" << debl;
  
  strncpy(command, aCommand, 127);
  char *sep = " \t\n";
  char *tok1 = strtok(command, sep);
  char *tok2 = strtok(0, sep);
  char *tok3 = strtok(0, sep);
  
  if (!tok1)
    return -1;
  
  if (!strcmp(tok1, "print"))   // print a statistics variable
  {
    int i;
    
    if (!tok2)			// print all values
    {
      for (i = 0; i < iStatistics.iNum; i++)
      {
        sprintf(response, "%-20s: %ld",
          iStatistics.Name(i), iStatistics.Value(i));
        POI_DEBUG(1) response << debl;
      }
      return 0;
    }
    if (isdigit(*tok2))		// access via index
    {
      i = atoi(tok2);
      char *name = iStatistics.Name(i);
      
      if (!name)
        return -1;
      long value = iStatistics.Value(i);
      
      sprintf(response, "%-20s: %ld", name, value);
      POI_DEBUG(1) response << debl;
      return 0;
    }
    else
    {				// access via name;  substrings are ok
      for (i = 0; i < iStatistics.iNum; i++)
      {
        char *name = iStatistics.iName[i];
        
        if (strstr(name, tok2))
        {
          long value = iStatistics.Value(i);
          
          sprintf(response, "%-20s: %ld", name, value);
          POI_DEBUG(1) response << debl;
        }
      }
      return 0;
    }
  }
  
  if (!strcmp(tok1, "show"))    // determine some information
  {
    if (!strcmp(tok2, "freq"))	// compute actual polling and update freq
    {
      long now = CurrentTime();
      double elapsed = (now - iStatistics.iValue[PUMA_CONNECT_TIME]);
      
      if (elapsed < 1)
        elapsed = 1;
      double polls = iStatistics.iValue[POLLS];
      double updates = iStatistics.iValue[UPDATES];
      
      POI_DEBUG(1) "elapsed time: " << elapsed << " secs" << debl;
      POI_DEBUG(1) "poll freq   : " << polls / elapsed << debl;
      POI_DEBUG(1) "update freq : " << updates / elapsed << debl;
      return 0;
    }
    if (!strcmp(tok2, "reqs"))	// show contents of request queues
    {
      POI_DEBUG(1) "Receiver     Slices  Pending" << debl;
      POI_DEBUG(1) "----------------------------" << debl;
      sprintf(response, "Puma          %5d", iWriteRequests.Count());
      POI_DEBUG(1) response << debl;
      for (Assoc * assoc = iPoiConn.First(); assoc; assoc = iPoiConn.Next())
      {
        PoiInfo *info = (PoiInfo *) assoc->iInfo;
        long len = info ? (long) info->iWriteRequests.Count() : -1;
        long pend = info ? info->iPending : -1;
        
        sprintf(response, "Poi %3d       %5d %8d",
          assoc->iMachID, len, pend);
        POI_DEBUG(1) response << debl;
        // POI_DEBUG(1)"Poi "<<setw(3)<<(int)assoc->iMachID<<"      "
        // <<setw(3)<<len<<"     "<<setw(6)<<pend<<debl;
      }
      return 0;
    }
    if (!strcmp(tok2, "pois"))	// show statistics on Poi connections
    {
      long now = CurrentTime();
      
      POI_DEBUG(1) "MId vers AId toBsls fromBs    time updHz" << debl;
      POI_DEBUG(1) "----------------------------------------" << debl;
      for (Assoc * assoc = iPoiConn.First(); assoc; assoc = iPoiConn.Next())
      {
        PoiInfo *info = (PoiInfo *) assoc->iInfo;
        
        if (!info)
          continue;
        long elapsed = now - info->iInitTime;
        
        if (elapsed <= 0)
          elapsed = 1;
        sprintf(response, "%3d %4d %3d %6d %6d %7d %5.2lf",
          assoc->iMachID, info->iProtocolVersion, info->iMachID,
          info->iToPoiBslices, info->iFromPoiBslices,
          elapsed, info->iToPoiBslices / (double) elapsed);
        POI_DEBUG(1) response << debl;
      }
      return 0;
    }
    if (!strcmp(tok2, "arguses"))	// show statistics on Argus connections
    {
      long now = CurrentTime();
      
      POI_DEBUG(1) "MId vers Msg toBsls fromBs    time updHz" << debl;
      POI_DEBUG(1) "----------------------------------------" << debl;
      POI_DEBUG(1) "CURRENTLY NOT SUPPORTED: performance info" << debl;
      POI_DEBUG(1) "----------------------------------------" << debl;
      for (Assoc * assoc = iArgusConn.First(); assoc; assoc = iArgusConn.Next())
      {
        ArgusInfo *info = (ArgusInfo *) assoc->iInfo;
        
        if (!info)
          continue;
        long elapsed = now - info->iInitTime;
        
        if (elapsed <= 0)
          elapsed = 1;
        sprintf(response, "%3d %4d %3d %6d %6d %7d %5.2lf",
          assoc->iMachID, info->iProtocolVersion, info->iSendMessages,
          info->iToArgusBslices, info->iFromArgusBslices,
          elapsed, info->iToArgusBslices / (double) elapsed);
        POI_DEBUG(1) response << debl;
      }
      return 0;
    }
    return -1;
  }
  
  if (!strcmp(tok1, "init"))    // initialize statistics
  {
    iStatistics.Init();
    return 0;
  }
  
  if (!strcmp(tok1, "set"))     // set some TCP config variables
  {
    if (! tok3)
      return -1;
    if (!strcmp(tok2, "maxmess")) // max number of pending Argus mess
    {
      POI_DEBUG(10) "PoiTcp::Command: maxmess = " << tok3 << debl;
      iMaxNumBslices = atol(tok3);
      return 0;
    }
    if (!strcmp(tok2, "urgent")) // next tok3 requests are urgent
    {
      POI_DEBUG(10) "PoiTcp::Command: urgent = " << tok3 << debl;
      iUrgent = atoi(tok3);
      return 0;
    }
    if (!strcmp(tok2, "sysvalid"))	// allow/disallow requests to PUMA
    {
      POI_DEBUG(10) "PoiTcp::Command: SYSVALID = " << tok3 << debl;
      iSysvalid = atoi(tok3);
      return 0;
    }
    if (!strcmp(tok2, "poll"))	// set polling delay
    {
      iPollDelay = atoi(tok3);
      iStatistics.iValue[POLL_DELAY] = iPollDelay;
      iPtl->SetTimer(iPollDelay);
      return 0;
    }
    if (!strcmp(tok2, "update"))	// set update delay counter
    {
      iUpdateDelay = atoi(tok3);
      iStatistics.iValue[UPDATE_DELAY] = iUpdateDelay;
      return 0;
    }
    if (!strcmp(tok2, "cmdeb"))	// set connection manager POI_DEBUG level
    {
      if (!iCM)
        return -1;
      int level = atoi(tok3);
      
      iCM->Trace(level);
      return 0;
    }
    if (!strcmp(tok2, "protoc"))	// set Poi/Argus protocol version
    {
      long version = atol(tok3);
      iProtocolVersion = version;
      iStatistics.iValue[PROTOC_VERSION] = iProtocolVersion;
      return 0;
    }
    if (!strcmp(tok2, "conntime")) // set max time for connect() call
    {
      iConnectTimeout = atol(tok3);
      iStatistics.iValue[CONNECT_TIMEOUT] = iConnectTimeout;
      return 0;
    }
    if (!strcmp(tok2, "snif")) // set max time for connect() call
    {
      if (*tok3 == '-')          // stop snifing
        return StopSnif();
      return StartSnif(tok3);
    }
    if (!strcmp(tok2, "writeok")) // allow/disallow puma write (ASYNC_PC_LINK)
    {
      if (*tok3 == '0')		// wait for puma bslice
        iPumaWriteOK = 0;
      else
        iPumaWriteOK = 1;
      return 0;
    }
    if (!strcmp(tok2, "pumaport")) // set SYNC or ASYNC PC_LINK mode
    {
      if (!strcmp(tok3, "sync"))
        iPumaPort = PC_LINK;
      if (!strcmp(tok3, "async"))
      {
        iPumaPort = ASYNC_PC_LINK;
        SetPollDelay();
      }
      return 0;
    }
    return -1;
  }
  
  if (!strcmp(tok1, "nnload"))	// reload NormNames
  {
    if (!iCM)
      return -1;
    
    return PoiNormnamesLoad(tok2);
  }
  
  // explain the available commands
  POI_DEBUG(1) "Available TCP commands:" << debl;
  POI_DEBUG(1) "  help		// print this help" << debl;
  POI_DEBUG(1) "  init		// reset the statistics variables" << debl;
  POI_DEBUG(1) "  show ...		// display various special statistics" << debl;
  POI_DEBUG(1) "    freq		//   current poll and update frequency" << debl;
  POI_DEBUG(1) "    reqs		//   contents of request queues" << debl;
  POI_DEBUG(1) "    pois		//   connected Pois" << debl;
  POI_DEBUG(1) "    arguses		//   connected Arguses" << debl;
  POI_DEBUG(1) "  print		// print all TCP statistics variables" << debl;
  POI_DEBUG(1) "  print byte	// print variables with name '*byte*'" << debl;
  POI_DEBUG(1) "  print 3		// print the 3rd variable (if it exists)" << debl;
  POI_DEBUG(1) "  set ...		// set various communication parameters" << debl;
  POI_DEBUG(1) "    poll 100	//   polling delay to 100 msecs" << debl;
  POI_DEBUG(1) "    update 2	//   update delay counter to 2" << debl;
  POI_DEBUG(1) "    cmdeb 1		//   connection manager POI_DEBUG level" << debl;
  POI_DEBUG(1) "    protoc 100	//   Poi/Argus protocol version" << debl;
  POI_DEBUG(1) "    conntime 5	//   timeout for Argus connect() call" << debl;
  POI_DEBUG(1) "    snif snif.0	//   start logging slices to snif.0" << debl;
  POI_DEBUG(1) "    snif -    	//   stop logging slices" << debl;
  POI_DEBUG(1) "    writeok 1|0    	//   allow/disallow PING to Puma" << debl;
  POI_DEBUG(1) "    pumaport 	//   select 'old style' sychronous" << debl;
  POI_DEBUG(1) "      sync|async 	//     or new style ServWin communic." << debl;
  POI_DEBUG(1) "    sysvalid 1|0 	//   allow/disallow requests to PUMA" << debl;
  POI_DEBUG(1) "    urgent num	  //   next num requests are urgent" << debl;
  POI_DEBUG(1) "    maxmess num	  //   max number of pending Argus mess " << debl;
  POI_DEBUG(1) "  nnload [pc1]	  // reload NormNames from Poi on pc1" << debl;
  
  return -1;
}


//  ArgusInternalCommand  is called when Poi receives a bslice with an
//  internal command from Argus.  An example for such a command is the request
//  to send the list of NormNames.
short PoiTcp::ArgusInternalCommand(BYTE aMachID, BYTE aCommand)
{
  static char* arg = 0;
  Slice slice;
  
  POI_DEBUG(12) "ArgusInternalCommand: machID: " << (int)aMachID
    << ", command: " << (int)aCommand << debl;
  //<< '(' << (aArg ? aArg : "void") << ')' << debl;
  
  ArgusInfo* info = (ArgusInfo*) Info(iArgusConn, aMachID);
  if (!info)			// this Argus is not connected
    return -1;
  
  if (info->iPendingCommand)	// the argument is now in iBuf
  {
    info->iPendingCommand = 0;
    aCommand = 0;
    iBuf.ResetRead();
    if (!iBuf.IsEom())
    {
      iBuf.GetSlice(slice);
      aCommand = slice.GetSubEventId();
      if (arg)
        free(arg);
      arg = strdup(slice.GetString().GetStr());
      POI_DEBUG(12) "  Argument=<" << arg << ">" << debl;
    }
  }
  
  switch (aCommand)
  {
  case ARG_COMMAND:		// wait for the argument
    info->iPendingCommand = 1;
    break;
    
  case LOAD_WND:
    {
      const char* name = arg ? arg : "";
      POI_DEBUG(12) "LOAD_WND(" << name << ") for Argus " << (int)aMachID << debl;
      (void) ArgPoiCommand(aMachID, ARGUS_LOAD_WND, name);
    }
    break;
    
  case SAVE_WND:
    {
      const char* name = arg ? arg : "";
      POI_DEBUG(12) "SAVE_WND(" << name << ") for Argus " << (int)aMachID << debl;
      (void) ArgPoiCommand(aMachID, ARGUS_SAVE_WND, name);
    }
    break;
    
  case CLOSE_WND:
    {
      const char* name = arg ? arg : "";
      POI_DEBUG(12) "CLOSE_WND(" << name << ") for Argus " << (int)aMachID << debl;
      (void) ArgPoiCommand(aMachID, ARGUS_CLOSE_WND, name);
    }
    break;
    
  case LOAD_NORMNAMES:		// send our NN list up
    return NormnamesSend(aMachID);
    break;
    
  case MESSAGES_ON:		// forward Puma messages to that Argus
    POI_DEBUG(12) "Messages for Argus " << (int)aMachID << " are ON" << debl;
    info->iSendMessages = 1;
    break;
    
  case MESSAGES_OFF:		// stop forwarding Puma messages
    POI_DEBUG(12) "Messages for Argus " << (int)aMachID << " are OFF" << debl;
    info->iSendMessages = 0;
    break;
    
  default:
    POI_DEBUG(3) "ArgusInternalCommand: unknown command " << ", command: "
      << (int)aCommand << " from machID " << (int)aMachID << debl;
    return -1;
  }
  
  if (arg)
  {
    free(arg);
    arg = 0;
  }
  
  return 0;
}


//  NormnamesSend  is called when an Argus requests the list of local
//  Normnames from a Poi.  For each Normname, a slice with relevant
//  information is sent to the Argus.
short PoiTcp::NormnamesSend(BYTE aMachID)
{
  POI_DEBUG(10) "NormnamesSend for Argus " << (int)aMachID << debl;
  
  SliceBuff nnbuf(16000);
  nnbuf.SwitchTag(1);
  nnbuf.PutTag(TAG_ASYNC);
  
  // create header
  Slice slice(ARGUS_WINDOW_ID, ARGUS_REQUEST, SAVE_SLICES, 128);
  
  nnbuf.PutSlice(slice);	// push header
  
  // read normname info from datasrct and put in slice
  short rc;
  short nns = 0;
  slice.PutSubEventId(NORMNAME_INFO);
  
  POI_DEBUG(12) "Sending Normnames to Argus" << debl;
  for (rc = iPtl->PoiCommand(GET_FIRST_NORMNAME, &slice);
  rc >= 0;
  rc = iPtl->PoiCommand(GET_NEXT_NORMNAME, &slice), nns++)
  {
    if ((long) nnbuf.GetLength() +
      (long) slice.GetCurrentWritePos() + 4 > 16000) // start new bslice
    {
      nnbuf.LengthToHeader();	// write bslice size into first two bytes
      POI_DEBUG(12) "  " << nns << " ..." << debl;
      if (ArgusWrite(aMachID, &nnbuf) < 0)
      {
        POI_DEBUG(3) "NormnamesSend: failed to send bslice of length "
          << nnbuf.GetLength() << " to Argus" << debl;
      }
      nnbuf.ResetWrite();
    }
    nnbuf.PutSlice(slice);	// push NN info
  }
  
  // create trailer
  slice.ResetWrite();
  slice.PutSubEventId(LOAD_NORMNAMES);
  nnbuf.PutSlice(slice);	// push trailer
  
  nnbuf.LengthToHeader();	// write bslice size into first two bytes
  
  POI_DEBUG(12) "Sending total of " << nns << " Normnames to Argus" << debl;
  return ArgusWrite(aMachID, &nnbuf);
}


//  PoiNormnamesLoad sends a request for the list of normnames to the
//  specified Poi.
short PoiTcp::PoiNormnamesLoad(const char* aHost)
{
  //long needVersion = 102;	// minimum Protocol version of Poi
  long needVersion = 104; // rose970404: NT version only
  Bed* bed = 0;
  
  if (! aHost)			// use Poi that was connected most recently
  {
    Assoc* assoc;
    Assoc* nassoc = 0;
    long ntime = 0;
    for (assoc = iPoiConn.First(); assoc; assoc = iPoiConn.Next())
    {
      long htime = ((PoiInfo*)(assoc->iInfo))->iInitTime;
      if (htime > ntime)
      {
        ntime = htime;
        nassoc = assoc;
      }
    }
    if (nassoc)
      bed = GetBed(nassoc->iMachID);
  }
  else				// look up bed through provided host name
    bed = GetBed(aHost);
  
  if (! bed && ! aHost)
  {
    POI_DEBUG(5) "PoiTcp::PoiNormnamesLoad: no testbed connected." << debl;
    return -10;
  }
  
  if (! bed)
  {
    POI_DEBUG(3) "Attempt to request normnames from " << aHost
      << " failed; testbed unknown." << debl;
    return -10;
  }
  
  aHost = bed->HostName().GetStr();
  POI_DEBUG(10) "Try to load normnames from testbed " << aHost << debl;
  
  BYTE machID = bed->MachID();
  BYTE command = LOAD_NORMNAMES;
  return ArgusCommand(machID, &command, 1, needVersion);
}


short PoiTcp::StartSnif(const char* aFileName)
{
  (void) StopSnif();
  iSliceNr = 0;
  
  iSnifFile = open(aFileName, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666);
  if (iSnifFile < 0)
  {
    POI_DEBUG(3) "Can't open Snif file " << aFileName << " for writing" << debl;
    return -1;
  }
  
  POI_DEBUG(10) "Start logging slices to file " << aFileName << debl;
  
  return 0;
}


short PoiTcp::StopSnif()
{
  if (iSnifFile >= 0)
  {
    POI_DEBUG(10) "Stop logging slices" << debl;
    close(iSnifFile);
    iSnifFile = -1;
  }
  return 0;
}


//  PoiTcpLink methods
//  --------------------------------------------------------------------------

PoiTcpLink::~PoiTcpLink()
{
  delete iTcp;
}


void PoiTcpLink::TcpConfigure(const String & aInitPath, Config * aConfig)
{
  if (!iTcp)
    iTcp = new PoiTcp();
  
  iTcp->Configure(aInitPath, aConfig);
}


//  Init creates a PoiTcp object and initializes it.  After
//  initialization, POI listens for ARGUS requests on aListenPort, and for
//  PUMA requests on iPumaPort.
//  Init can be called more then once.
short PoiTcpLink::TcpInit(PoiLinkWindow * aPlw)
{
  iPlw = aPlw;
  
  if (!iPlw || !iTcp)
    return -1;
  
  return iTcp->Init(this);
}


//  Close closes all open Tcp connections.
void PoiTcpLink::TcpClose()
{
  if (iTcp)
    iTcp->Close();
}


void PoiTcpLink::TcpWakeUp()
{
  if (iTcp)
    iTcp->WakeUp();
}


void PoiTcpLink::TcpRegisterRequest(Slice* aSlice, short aMode)
{
  if (iTcp)
    iTcp->RegisterRequest(aSlice, aMode);
}

void PoiTcpLink::TcpRegisterSlowRequest(Slice* aSlice, short aMode)
{
  if (iTcp)
    iTcp->RegisterSlowRequest(aSlice, aMode);
}

void PoiTcpLink::TcpRemoveRequest(Slice * aSlice)
{
  if (iTcp)
    iTcp->RemoveRequest(aSlice);
}


short PoiTcpLink::TcpWriteRequests(BYTE aMachID)
{
  if (iTcp)
    return iTcp->WriteRequests(aMachID);
  return -1;
}


void PoiTcpLink::TcpPumaBsliceHandle(BYTE * aBslyte)
{
  if (iTcp)
    (void) iTcp->PumaBslyteHandle(aBslyte);
}


void PoiTcpLink::TcpQueueClear()
{
  if (iTcp)
    iTcp->QueueClear();
}


BedList *PoiTcpLink::TcpBeds()
{
  return iTcp ? iTcp->Beds() : 0;
}


short PoiTcpLink::TcpPoiConnect(BYTE aMachID)
{
  if (iTcp)
    return iTcp->PoiConnect(aMachID);
  return -1;
}


short PoiTcpLink::TcpPoiDisconnect(BYTE aMachID)
{
  if (iTcp)
    return iTcp->PoiDisconnect(aMachID);
  return -1;
}


long PoiTcpLink::TcpInfo(short aWhat)
{
  return iTcp ? iTcp->Info(aWhat) : 0;
}


short PoiTcpLink::TcpArgusCommand(BYTE aMachID, BYTE* aComm, long aLen,
                                  long aVersion)
{
  return iTcp ? iTcp->ArgusCommand(aMachID, aComm, aLen, aVersion) : -1;
}


short PoiTcpLink::TcpPumaPort()
{
  return iTcp ? iTcp->iPumaPort : 0;
}


short PoiTcpLink::TcpCommand(char *aCommand)
{
  return iTcp ? iTcp->Command(aCommand) : -1;
}


//  Bed methods
//  --------------------------------------------------------------------------

short Bed::Connected()
{
  if (!gTcp)
    return 0;
  
  return gTcp->Connected(iMachID);
}


//  Statistics methods
//  --------------------------------------------------------------------------

Statistics::Statistics()
{
  iNum = sizeof(iName) / sizeof(char *);
  iValue = new long[iNum];
  
  Init();
}


Statistics::~Statistics()
{
  delete iValue;
}


void Statistics::Init()
{
  for (int i = 0; i < iNum; i++)
    iValue[i] = 0;
  
  if (! gTcp)
    return;
  
  iValue[PUMA_CONNECT_TIME] = CurrentTime();
  iValue[POLL_DELAY] = gTcp->iPollDelay;
  iValue[UPDATE_DELAY] = gTcp->iUpdateDelay;
  iValue[PUMA_PORT] = gTcp->iPumaPort;
  iValue[ARGUS_LISTEN_PORT] = gTcp->iArgusPort;
  iValue[ARGUS_CONNECT_PORT] = gTcp->iConnectPort;
}


int Statistics::Index(char *aName)
{
  for (int i = 0; i < iNum; i++)
    if (!strncmp(aName, iName[i], strlen(aName)))
      return i;
    return -1;
}


char *Statistics::Name(int aIndex)
{
  if (aIndex < 0 || aIndex >= iNum)
    return 0;
  return iName[aIndex];
}


long Statistics::Value(int aIndex)
{
  if (aIndex < 0 || aIndex >= iNum)
    return -1;
  return iValue[aIndex];
}


//////////////////////////////////////////////////////////////////////////////
// Snifing
//


void PoiTcp::LogBslice(BYTE* aBslice, long aSize, char* aLabel)
{
  const char headsize = 1 + 3*sizeof(long);
  
  aSize += 2;			// add 2 bytes for header
  
  long t = elapsedTime();
  
  long hsize = swap_l((char*)&aSize);
  long hnr = swap_l((char*)&iSliceNr);
  long ht = swap_l((char*)&t);
  
  if (write(iSnifFile, aLabel, 1) != 1
    || write(iSnifFile, &hsize, sizeof(long)) != sizeof(long)
    || write(iSnifFile, &hnr, sizeof(long)) != sizeof(long)
    || write(iSnifFile, &ht, sizeof(long)) != sizeof(long)
    || write(iSnifFile, aBslice, aSize) != aSize
    )
  {
    StopSnif();
    return;
  }
  
  POI_DEBUG(20) "SNIF: " << *aLabel << " " << iSliceNr << " len=" << aSize << debl;
  
  if (aLabel && *aLabel == 'C')
    iSliceNr++;
}
