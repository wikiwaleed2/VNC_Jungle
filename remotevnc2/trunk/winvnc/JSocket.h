//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
//  Copyright (C) 2001 HorizonLive.com, Inc. All Rights Reserved.
//
//  This file is part of the VNC system.
//
//  The VNC system is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// TightVNC distribution homepage on the Web: http://www.tightvnc.com/
//
// If the source code for the VNC system is not available from the place 
// whence you received this file, check http://www.uk.research.att.com/vnc or contact
// the authors on vnc@uk.research.att.com for information on obtaining it.


// JSocket.h

// Wrapper for Jingle sockets.

class JSocket;

#if (!defined(_JSOCKET_DEFINED))
#define _JSOCKET_DEFINED

#include "VSocketBase.h"
#include "vncjingle.h"

// The main socket class
class JSocket : public VSocketBase
{
public:
  // Constructor/Destructor
  JSocket(VncJingleSocket *sock);
  virtual ~JSocket();

  ////////////////////////////
  // Socket implementation

  // Create
  //        Create a socket and attach it to this VSocket object
  VBool Create();

  // Shutdown
  //        Shutdown the currently attached socket
  VBool Shutdown();

  // Close
  //        Close the currently attached socket
  VBool Close();

  // GetPeerName
  //        If the socket is connected then this returns the name
  //        of the machine to which it is connected.
  //        This string MUST be copied before the next socket call...
  VString GetPeerName();

  // GetSockName
  //		If the socket exists then the name of the local machine
  //		is returned.  This string MUST be copied before the next
  //		socket call!
  VString GetSockName();

  // SetTimeout
  //        Sets the socket timeout on reads and writes.
  VBool SetTimeout(VCard32 secs);

  // I/O routines

  // Send and Read return the number of bytes sent or recieved.
  VInt Send(const char *buff, const VCard bufflen);
  VInt Read(char *buff, const VCard bufflen);

  // SendExact and ReadExact attempt to send and recieve exactly
  // the specified number of bytes.
  VBool SendExact(const char *buff, const VCard bufflen);
  VBool ReadExact(char *buff, const VCard bufflen);

  ////////////////////////////
  // Internal structures
protected:
  // The jingle socket.
	VncJingleSocket *m_sock;
};

#endif // _JSOCKET_DEFINED
