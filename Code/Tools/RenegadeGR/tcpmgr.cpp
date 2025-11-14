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

#include "tcpmgr.h"
#include "tcpcon.h"
#include "wlib/wtime.h"


TCPMgr::TCPMgr()
{
	HandleSequence_=0;
}

TCPMgr::~TCPMgr()
{
}


//
// Add a listener socket to accept connections on a given port
//
bit8 TCPMgr::addListener(uint32 ip, uint16 port, bit8 reuseAddr)
{
  SOCKET fd=createSocket(ip,port,reuseAddr);
  if (fd == INVALID_SOCKET)
    return(false);

  listen(fd,64);  // Solaris needs lots of listen slots

  ListenSocket lsock;
  lsock.fd=fd;
  lsock.ip=ip;
  lsock.port=port;

  ListenArray_.addTail(lsock);

  return(true);
}

//
// Remove listener on a given ip/port 
//
bit8 TCPMgr::removeListener(uint32 ip, uint16 port)
{
  ListenSocket *lptr;
  for (int i=0; i<ListenArray_.length(); i++)
  {
    ListenArray_.getPointer(&lptr,i);
    if ((lptr->ip == ip) && (lptr->port == port))
    {
      closesocket(lptr->fd);
      ListenArray_.remove(i);
      return(true);
    }
  }
  return(false);
}

//
// Get the socket for a given listener
//
bit8 TCPMgr::getListener(uint32 ip, uint16 port, OUT SOCKET &outsock)
{
  ListenSocket *lptr;
  for (int i=0; i<ListenArray_.length(); i++)
  {
    ListenArray_.getPointer(&lptr,i);
    if ((lptr->ip == ip) && (lptr->port == port))
    {
      outsock=lptr->fd;
      return(true);
    }
  }
  return(false);
}


//
// Enable/Disable buffered writes on a socket
//
bit8 TCPMgr::setBufferedWrites(TCPCon *con, bit8 enabled)
{
  TCPCon *tempptr=NULL;

  // Check to see if this connection is already in our list
  for (int i=0; i<BufferedWriters_.length(); i++)
  {
    BufferedWriters_.get(tempptr, i);
    if (tempptr==con)
    {
      con->setBufferedWrites(this, false);
      BufferedWriters_.remove(i);
      break;
    }
  }
  if (enabled)  // OK, now add to the list
  {
    con->setBufferedWrites(this, true);
    BufferedWriters_.addTail(con);
  }
  return(true);
}


//
// Let all the buffered writers send data
//
void TCPMgr::pumpWriters(void)  // pump the buffered writer connections
{
  TCPCon *conptr=NULL;
 
  // Check to see if this connection is already in our list
  for (int i=0; i<BufferedWriters_.length(); i++)
  {
    BufferedWriters_.get(conptr, i);
    conptr->pumpWrites();
  }
}



//
// Connect by hostname rather than IP
//
bit8 TCPMgr::connect(char *host, uint16 port, OUT uint32 *handle)
{
  char           hostName[129];
  struct hostent *hostStruct;
  struct in_addr *hostNode;
 
  if (isdigit(host[0]))
    return ( connect(ntohl(inet_addr(host)), port, handle));
 
  strcpy(hostName, host);
  hostStruct = gethostbyname(host);
  if (hostStruct == NULL)
    return (0);
  hostNode = (struct in_addr *) hostStruct->h_addr;
  return ( connect(ntohl(hostNode->s_addr),port,handle) );
}



//
// Request a connection to a given address (all values in host byte order)
//
bit8 TCPMgr::connect(uint32 ip, uint16 port,OUT uint32 *handle)
{
  PendingConn pConn;
  if ((pConn.fd=createSocket((uint32) 0,(uint16) 0,false)) == INVALID_SOCKET)
    return(false);
  pConn.ip=0;
  pConn.port=0;
  pConn.remoteIp=ip;
  pConn.remotePort=port;
  pConn.startTime=time(NULL);
  pConn.handle=HandleSequence_++;
  pConn.state=CLOSED;
  pConn.incoming=false;   // outgoing connection

  ConnectArray_.addTail(pConn);

  *handle=pConn.handle;
  return(true);
}

//
// Get an incoming connection (specify handle or 0 for any)
//
// Wait for upto 'wait_secs' seconds for the connection.
//
bit8 TCPMgr::getOutgoingConnection(TCPCon **conn, uint32 handle, sint32 wait_secs)
{
  return(getConnection(conn,handle,0,wait_secs,OUTGOING));
}


//
// Get an incoming connection (specify listen port or 0 for any) 
//
// Wait for upto 'wait_secs' seconds for the connection.
//
bit8 TCPMgr::getIncomingConnection(TCPCon **conn, uint16 port, sint32 wait_secs)
{
  return(getConnection(conn,INVALID_HANDLE,port,wait_secs,INCOMING));
}


//
// Return after there is data to read, or we've timed out.
//
int TCPMgr::wait(uint32 sec, uint32 usec, SOCKET *sockets, int count, bit8 readMode)
{
  Wtime  timeout,timenow,timethen;
  fd_set givenSet;
  fd_set returnSet;
  fd_set backupSet;
  int    givenMax=0;
  bit8   noTimeout=false;
  int    retval;
  uint32 i;

  DBGMSG("Waiting on "<<count<<" sockets");

  FD_ZERO(&givenSet);
  for (i=0; i<(uint32)count; i++)
  {
    FD_SET(sockets[i],&givenSet);
  }

  timeval      tv;
  timeval     *tvPtr=NULL;
  returnSet=givenSet;
  backupSet=givenSet;
 
  if ((sec==-1)||(usec==-1))
    noTimeout=true;
 
  timeout.SetSec(sec);
  timeout.SetUsec(usec);
  timethen+=timeout;
 
  for (i=0; i<(uint32)count; i++)
  {
    if (sockets[i] > (SOCKET)givenMax)
      givenMax=sockets[i];
  }
 
  bit8 done=false;
  while( ! done)
  {
    tvPtr=&tv;
    if (noTimeout)
      tvPtr=NULL;
    else
      timeout.GetTimevalMT(tv);

    if (readMode)   // can we read?
      retval=select(givenMax+1,&returnSet,0,0,tvPtr);
    else            // can we write?
      retval=select(givenMax+1,0,&returnSet,0,tvPtr);

    DBGMSG("Select wake");

    if (retval>=0)
      done=true;
    else if ((retval==SOCKET_ERROR)&&(getStatus()==INTR))  // in case of signal
    {
      if (noTimeout==false)
      {
        timenow.Update();
        timeout=timethen-timenow;
      }
      if ((noTimeout==false)&&(timenow.GetSec()==0)&&(timenow.GetUsec()==0))
        done=true;
      else
        returnSet=backupSet;
    }
    else  // maybe out of memory?
    {
      done=true;
    }
  }
  return(retval);
}


TCPMgr::STATUS TCPMgr::getStatus(void)
{
 #ifdef _WINDOWS
  int status=WSAGetLastError();
  if (status==0) return(OK);
  else if (status==WSAEINTR) return(INTR);
  else if (status==WSAEINPROGRESS) return(INPROGRESS);
  else if (status==WSAECONNREFUSED) return(CONNREFUSED);
  else if (status==WSAEINVAL) return(INVAL);
  else if (status==WSAEISCONN) return(ISCONN);
  else if (status==WSAENOTSOCK) return(NOTSOCK);
  else if (status==WSAETIMEDOUT) return(TIMEDOUT);
  else if (status==WSAEALREADY) return(ALREADY);
  else if (status==WSAEWOULDBLOCK) return(WOULDBLOCK);
  else if (status==WSAEBADF) return(BADF);
  else     return(UNKNOWN);
 #else
  int status=errno;
  if (status==0) return(OK);
  else if (status==EINTR) return(INTR);
  else if (status==EINPROGRESS) return(INPROGRESS);
  else if (status==ECONNREFUSED) return(CONNREFUSED);
  else if (status==EINVAL) return(INVAL);
  else if (status==EISCONN) return(ISCONN);
  else if (status==ENOTSOCK) return(NOTSOCK);
  else if (status==ETIMEDOUT) return(TIMEDOUT);
  else if (status==EALREADY) return(ALREADY);
  else if (status==EAGAIN) return(AGAIN);
  else if (status==EWOULDBLOCK) return(WOULDBLOCK);
  else if (status==EBADF) return(BADF);
  else     return(UNKNOWN);
 #endif
}


/********************* BEGIN PRIVATE METHODS *********************************/

//
// Create a bound socket
//
SOCKET TCPMgr::createSocket(uint32 ip, uint16 port, bit8 reuseAddr)
{
  struct sockaddr_in addr;
  addr.sin_family=AF_INET;
  addr.sin_port=htons(port);
  addr.sin_addr.s_addr=htonl(ip);
 
  SOCKET fd=socket(AF_INET,SOCK_STREAM,DEFAULT_PROTOCOL);
  if (fd==-1)
    return(INVALID_SOCKET);

  if (setBlocking(fd,false)==false)
    return(INVALID_SOCKET);
 
  if (reuseAddr)
  {
    uint32 opval=1;
    if (setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char *)&opval,sizeof(opval)) != 0)
    {
      closesocket(fd);
      return(INVALID_SOCKET);
    }
  }
  if (bind(fd,(struct sockaddr *)&addr, sizeof(addr))==SOCKET_ERROR)
  {
    closesocket(fd);
    return(INVALID_SOCKET);
  }
  return(fd);
}


//
// Set the blocking mode of the socket 
//
bit8 TCPMgr::setBlocking(SOCKET fd, bit8 block)
{
 #ifdef _WINDOWS
  unsigned int flag=1;
  if (block)
    flag=0;
  int retval;
  retval=ioctlsocket(fd,FIONBIO,&flag);
  if (retval==SOCKET_ERROR)
    return(false);
  else
    return(true);
 #else	// UNIX
  int flags = fcntl(fd, F_GETFL, 0);
  if (block==false)					// set nonblocking
    flags |= O_NONBLOCK;
  else											 // set blocking
    flags &= ~(O_NONBLOCK);

  if (fcntl(fd, F_SETFL, flags) < 0)
  {
    return(false);
  }
  return(true);
 #endif
}


bit8 TCPMgr::getConnection(TCPCon **conn, uint32 handle, uint16 port, sint32 wait_secs, DIRECTION dir)
{
  PendingConn *connPtr=NULL;
  time_t      start=time(NULL);
  SOCKET      fdArray[1024];
 
  for (int i=0; i<ConnectArray_.length(); i++)
  {
    ConnectArray_.getPointer(&connPtr,i);
    fdArray[i]=connPtr->fd;
  }
 
  while(1)
  {
    pumpConnections();

    for (int i=0; i<ConnectArray_.length(); i++)
    {
      ConnectArray_.getPointer(&connPtr,i);
      if (connPtr->state != CONNECTED)
        continue;
      if (( ! ((int)dir & (int)INCOMING)) && (connPtr->incoming == true))
        continue;
      if (( ! ((int)dir & (int)OUTGOING)) && (connPtr->incoming == false))
        continue;
      if ((handle != INVALID_HANDLE) && (handle != connPtr->handle))
        continue;
      if ((port != 0) && (port != connPtr->port))
        continue;
      *conn=new TCPCon(connPtr->fd);
      ConnectArray_.remove(i);
      return(true);
    }
    // Wait for socket activity for a bit
	 #ifdef _WINDOWS
    Sleep(100);  // windows may be getting flooded with conn msgs, test this
	 #endif
    sint32 remaining_wait=wait_secs - (time(NULL)-start);
    if ((remaining_wait > 0) && (wait(remaining_wait,0,fdArray,ConnectArray_.length(),false) > 0))
      continue;  // got something!
 
    if (remaining_wait <= 0)
      break;
  }
  return(false);
}





// 
// TOFIX: don't forget about connect flooding on Win32
//
void TCPMgr::pumpConnections(void)
{
  PendingConn *connPtr=NULL;
  STATUS      status;
  int         i;
  int         retval;

  // Outgoing connections
  for (i=0; i<ConnectArray_.length(); i++)
  {
    ConnectArray_.getPointer(&connPtr,i);

    if ((connPtr->state == CLOSED) || (connPtr->state == CONNECTING))
    {
      struct sockaddr_in addr;
		int                addrSize=sizeof(addr);
      addr.sin_family=AF_INET;
      addr.sin_port=htons(connPtr->remotePort);
      addr.sin_addr.s_addr=htonl(connPtr->remoteIp);

		if (connPtr->state == CONNECTING)
		{
        if (getpeername(connPtr->fd,(sockaddr *)&addr,&addrSize)==0)  // if get endpoint, then success
		    connPtr->state=CONNECTED;
      }
      else if ((retval=::connect(connPtr->fd, (struct sockaddr *)&addr, sizeof(addr)))==SOCKET_ERROR)
      {
        status=getStatus();
		  if (status==ISCONN)
          retval=0;
        else if ((status==INPROGRESS)||(status==ALREADY)||(status==WOULDBLOCK))
        {
          connPtr->state=CONNECTING;
          continue;   // Move on to next pending conn...
        }
        else // doh, real problem
        {
           assert(0);
           closesocket(connPtr->fd);
           connPtr->fd=createSocket(connPtr->ip, connPtr->port, false);
        }
      }
      if (retval==0)
      {
        connPtr->state=CONNECTED;
      }
    }
  } // for ConnectArray_; 

  // Incoming connections
  ListenSocket        *listenPtr;
  struct sockaddr_in  clientAddr;
  int                 addrlen;
  for (i=0; i<ListenArray_.length(); i++)
  {
    ListenArray_.getPointer(&listenPtr,i);

    while(1)  // accept all incoming on each socket
    {
      addrlen=sizeof(clientAddr);
      SOCKET newFD=accept(listenPtr->fd, (struct sockaddr *)&clientAddr, &addrlen);
      if (newFD != INVALID_SOCKET)
      {
        setBlocking(newFD, false);

        DBGMSG("Connection accepted");

        PendingConn newConn;
        newConn.fd=newFD;
        newConn.ip=0;
        newConn.port=0;
        newConn.remoteIp=ntohl(clientAddr.sin_addr.s_addr);
        newConn.remotePort=ntohs(clientAddr.sin_port);
        newConn.handle=HandleSequence_++;
        newConn.state=CONNECTED;
        newConn.incoming=true;
        newConn.remoteIp=ntohl(clientAddr.sin_addr.s_addr);
        newConn.remotePort=ntohs(clientAddr.sin_port);

        ConnectArray_.addTail(newConn);
      }
      else
        break;
    }
  }
  return;
}






