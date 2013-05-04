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


// VSocketBase.h

// Base class for communication sockets.

class VSocketBase;

#if (!defined(_ATT_VSOCKETBASE_DEFINED))
#define _ATT_VSOCKETBASE_DEFINED

#include <omnithread.h>
#include "VTypes.h"

// This class is used as a part of output queue
class AIOBlock
{
public:
	size_t data_size;		// Data size in this block
	char *data_ptr;			// Beginning of the data buffer
	AIOBlock *next;			// Next block or NULL for the last block

	AIOBlock(int size, const char *data = NULL) {
		next = NULL;
		data_size = size;
		data_ptr = new char[size];
		if (data_ptr && data)
			memcpy(data_ptr, data, size);
	}
	~AIOBlock() {
		if (data_ptr)
			delete[] data_ptr;
	}
};

// The socket base class
class VSocketBase
{
public:
  // Constructor/Destructor
	VSocketBase();
	virtual ~VSocketBase();

  ////////////////////////////
  // Socket implementation

  // Shutdown
  //        Shutdown the currently attached socket. Subclasses should call this metho
	//				after their own.
  virtual VBool Shutdown();

  // Close
  //        Close the currently attached socket. Subclasses should call this metho
	//				after their own.
  virtual VBool Close();

  // GetPeerName
  //        If the socket is connected then this returns the name
  //        of the machine to which it is connected.
  //        This string MUST be copied before the next socket call...
  virtual VString GetPeerName() = 0;

  // GetSockName
  //		If the socket exists then the name of the local machine
  //		is returned.  This string MUST be copied before the next
  //		socket call!
  virtual VString GetSockName() = 0;

	// SetTimeout
  //        Sets the socket timeout on reads and writes.
  virtual VBool SetTimeout(VCard32 secs) = 0;

  // I/O routines

  // Send and Read return the number of bytes sent or recieved.
	// These functions are non-blocking.
  virtual VInt Send(const char *buff, const VCard bufflen) = 0;
  virtual VInt Read(char *buff, const VCard bufflen) = 0;

  // SendExact and ReadExact attempt to send and recieve exactly
  // the specified number of bytes. These functions are blocking.
  virtual VBool SendExact(const char *buff, const VCard bufflen) = 0;
  virtual VBool ReadExact(char *buff, const VCard bufflen) = 0;

  // SendQueued sends as much data as possible immediately,
  // and puts remaining bytes in a queue, to be sent later.
  virtual VBool SendQueued(const char *buff, const VCard bufflen);

  ////////////////////////////
  // Internal structures
protected:
  // Output queue
  size_t bytes_sent;
  AIOBlock *out_queue;
  omni_mutex queue_lock;

  VBool SendFromQueue();
};

#endif // _ATT_VSOCKET_DEFINED
