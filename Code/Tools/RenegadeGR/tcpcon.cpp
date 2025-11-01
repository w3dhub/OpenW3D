/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "tcpcon.h"
#include <time.h>

#define DEFAULT_TCP_DELAY 15

TCPCon::TCPCon(SOCKET sock) :
  BufferedWrites_(false),
  TCPMgrPtr_(NULL)
{
  Socket_=sock;
  isConnected();  // This will set the internal connect status
  setInputDelay(DEFAULT_TCP_DELAY);  
  setOutputDelay(DEFAULT_TCP_DELAY);  
}

TCPCon::~TCPCon()
{
  close();  // will try and flush the write queue
  ReadQueue_.clear();
  WriteQueue_.clear();

  if ((BufferedWrites_) && (TCPMgrPtr_))
    TCPMgrPtr_->setBufferedWrites(this, false);
}

SOCKET TCPCon::getFD(void)
{  return(Socket_); }


void TCPCon::close(void)
{
  if (BufferedWrites_)
  {
    while(WriteQueue_.length())
      pumpWrites();
  }
  ::closesocket(Socket_);
  State_=TCPMgr::CLOSED;
}


//
// Write data
//
// Returns 'n' bytes written, 0 if closed, or -1 for error.
//
sint32 TCPCon::write(IN uint8 *msg,uint32 len, sint32 wait_secs)
{
  if (State_==TCPMgr::CLOSED)
    return(0);

  if (BufferedWrites_)
  {
    WriteQueue_.addMany(msg, WriteQueue_.length(), len);  // add to tail
    pumpWrites();  // send what I can
    return(len);
  }
  else
  {
    return (normalWrite(msg, len, wait_secs));
  }
}


//
// set buffered status 
//
void TCPCon::setBufferedWrites(TCPMgr *mgrptr, bit8 enabled)
{
  if (enabled)
    BufferedWrites_=true;  
  else
  {
    while(WriteQueue_.length())
      pumpWrites();
    BufferedWrites_=false;
  }
  TCPMgrPtr_=mgrptr;
}


//
// Try and send queued data (PRIVATE)
//
void TCPCon::pumpWrites(void)
{
  if (State_==TCPMgr::CLOSED)
    WriteQueue_.clear();

  if (WriteQueue_.length()==0)
    return;

  int   sendlen;
  int   retval;
  uint8 *bufptr=NULL;

  while(1)
  {
    sendlen=WriteQueue_.length();
    if (sendlen == 0)
      break;

    WriteQueue_.getPointer(&bufptr,0);  // pointer to first byte

    retval=normalWrite((uint8 *)bufptr, sendlen, 0);
    if (retval <= 0)
      break;

    WriteQueue_.removeMany(NULL, 0, retval);  // successful send
  }
  return;
}


//
// Non-buffered write (PRIVATE METHOD) 
//
// Returns 'n' bytes written, 0 if closed, or -1 for error.
//
sint32 TCPCon::normalWrite(IN uint8 *msg,uint32 len, sint32 wait_secs)
{
  if (State_==TCPMgr::CLOSED)
    return(0);

  if (wait_secs < 0)
    wait_secs=OutputDelay_;

  sint32 retval=0;
  sint32 sendCount=0;
  time_t start=time(NULL);

  TCPMgr::STATUS status;
 
  while(1)
  {
    retval=send(Socket_,(const char *)(msg+sendCount),(len-sendCount),0);
    if ((retval > 0) && (retval+sendCount)==int(len))
      break;
    if (retval==SOCKET_ERROR)
    {
      status=TCPMgr::getStatus(); 
      if ((status != TCPMgr::INTR) && (status != TCPMgr::WOULDBLOCK) && (status != TCPMgr::INPROGRESS))
      {
        if (sendCount)
          return(sendCount);
        else
          return(-1);
      }
    }
    else if (retval > 0)
      sendCount+=retval;

    sint32 remaining_wait=wait_secs - (time(NULL)-start);
    if ((remaining_wait > 0) && (TCPMgr::wait(remaining_wait,0,&Socket_,1,false) > 0))
      continue;  // I can write now.... 

    if (remaining_wait <= 0)
      break;
  }
  return(retval);
}

//
// Read data
//
// Returns 'n' bytes read, 0 for close, or -1 for error.
// This may return less than we asked for
//
sint32 TCPCon::read(OUT uint8 *msg,uint32 maxlen, sint32 wait_secs)
{
  sint32 retval=0;
  sint32 recvCount=0;
  time_t start=time(NULL);
  char   readBuffer[257];

  if (wait_secs < 0)
    wait_secs=InputDelay_;

  if (State_==TCPMgr::CLOSED)
    return(0);
 
  TCPMgr::STATUS status;
 
  while(1)
  {
    // Do we even nead to read from the net?
    if (ReadQueue_.length() >= int(maxlen))
    {
      ReadQueue_.removeMany(msg, 0, maxlen);
      return(maxlen);
    }

    do
    {
      retval=recv(Socket_,readBuffer, 256,0);
      if (retval > 0)   // add to the tail of the list
      {
        readBuffer[retval]=0;
        DBGMSG("RECV: "<<readBuffer);

        // Add to tail
        ReadQueue_.addMany((uint8 *)readBuffer, ReadQueue_.length(), retval);
      }
    } while ((retval > 0)&&(ReadQueue_.length() < int(maxlen)));

    if (ReadQueue_.length())  // OK, we'll take what we've got
    {
      uint8 *cptr;
      ReadQueue_.getPointer(&cptr,0);

/*******
      fprintf(stderr,"ReadQueue(%d): '",ReadQueue_.length()); 
      for (int i=0; i<ReadQueue_.length(); i++)
        fprintf(stderr,"%c",cptr[i]);
      fprintf(stderr,"'\n"); 
********/

      int retcount=MIN(ReadQueue_.length(), int(maxlen));
      ReadQueue_.removeMany(msg, 0, retcount);
      return(retcount);
    }
    else if (retval==0)
    {
      close();
      return(0);
    }
    else if (retval==SOCKET_ERROR)
    {
      status=TCPMgr::getStatus(); 
      if ((status != TCPMgr::INTR) && (status != TCPMgr::WOULDBLOCK) && 
          (status != TCPMgr::INPROGRESS))
        return(-1);
    }
 
    sint32 remaining_wait=wait_secs - (time(NULL)-start);
    if ((remaining_wait > 0) && (TCPMgr::wait(remaining_wait,0,&Socket_,1,true) > 0))
      continue;  // I can read now.... 

    if (remaining_wait <= 0)
      break;
  }
  return(retval);
}

// Push data back onto the read queue
bit8 TCPCon::unread(uint8 *data, int length)
{
  ReadQueue_.addMany(data, 0, length);
  return(true);
}


// Returns 0 on failure
// Returns IP in host byte order!
bit8 TCPCon::getRemoteAddr(uint32 *ip, uint16 *port)
{
  struct sockaddr_in sin;
  int    sinSize=sizeof(sin);
 
  if(getpeername(Socket_,(sockaddr *)&sin,&sinSize)==0)
  {
    if (ip)
      *ip=ntohl(sin.sin_addr.s_addr);
    if (port)
      *port=ntohs(sin.sin_port);
    return(true);
  }
  return(false);
}

//
// only use for strings up to 4096 chars!
//
sint32 TCPCon::printf(const char *format, ...)
{
  va_list arg;
  char string[4097];
  sint32 retval;
  va_start(arg,format);
  vsprintf(string,format,arg);
  va_end(arg);
  string[4096]=0;
 
  retval=write((IN uint8 *)string,strlen(string), OutputDelay_);
  return(retval);
}
 

bit8 TCPCon::isConnected(void)
{
  uint32 remoteIp;
  uint16 remotePort;

  if (getRemoteAddr(&remoteIp,&remotePort)==true)
  {
    State_=TCPMgr::CONNECTED;
    return(true);
  }
  else
  {
    State_=TCPMgr::CLOSED;
    return(false);
  }
}

/*********
// For the OutputDevice interface
int TCPCon::print(IN char *str, int len)
{
  return(write((IN uint8 *)str,len,0));
}
********/


