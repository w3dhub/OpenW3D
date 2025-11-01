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

#ifndef TCP_CON_HEADER
#define TCP_CON_HEADER

#include "tcpmgr.h"
#include "wlib/arraylist.h"

class TCPMgr;

class TCPCon /// : OutputDevice
{
 public:
           TCPCon(SOCKET sock);
           ~TCPCon();

   SOCKET  getFD(void);
   void    close(void); 
   sint32  write(IN uint8 *msg, uint32 len, sint32 wait_secs=-1);
   sint32  read(OUT uint8 *msg, uint32 maxlen, sint32 wait_secs=-1);
   bit8    unread(uint8 *data, int length);
   bit8    getRemoteAddr(uint32 *ip, uint16 *port);
   sint32  printf(const char *format, ...);
   bit8    isConnected(void);

   bit8    setInputDelay(sint32 delay) { InputDelay_=delay; return(true); };
   bit8    setOutputDelay(sint32 delay) { OutputDelay_=delay; return(true); };

   // For OutputDevice
   /// virtual int print(IN char *str, int len); 

 private:
   friend  class TCPMgr;
   void    pumpWrites(void);  // for buffered mode
   void    setBufferedWrites(TCPMgr *mgrptr, bit8 enabled);

   sint32  normalWrite(IN uint8 *msg, uint32 len, sint32 wait_secs=-1);

   sint32              InputDelay_;  // default max time for input
   sint32              OutputDelay_; // default max time for output

   TCPMgr::CONN_STATE  State_;
   SOCKET              Socket_;
   ArrayList<uint8>    ReadQueue_;       // reads are buffered

   bit8                BufferedWrites_;  // T/F buffer writes?
   ArrayList<uint8>    WriteQueue_;      // writes _can_ be buffered
   TCPMgr              *TCPMgrPtr_;      // pointer to my manager object
};

#endif
