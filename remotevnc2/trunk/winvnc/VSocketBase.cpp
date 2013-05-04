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


// VSocketBaseBase.cpp

////////////////////////////////////////////////////////
// System includes

#include "stdhdrs.h"

// Visual C++ .NET 2003 compatibility
#if (_MSC_VER>= 1300)
#include <iostream>
#else
#include <iostream.h>
#endif

#include <stdio.h>
#ifdef __WIN32__
#include <io.h>
#include <winsock.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#endif
#include <sys/types.h>

////////////////////////////////////////////////////////
// Custom includes

#include "VTypes.h"

////////////////////////////////////////////////////////
// *** Lovely hacks to make Win32 work.  Hurrah!

#ifdef __WIN32__
#define EWOULDBLOCK WSAEWOULDBLOCK
#endif

////////////////////////////////////////////////////////
// Socket implementation

#include "VSocketBase.h"

VSocketBase::VSocketBase()
{
  // Clear out the internal socket fields
  out_queue = NULL;
}

////////////////////////////

VSocketBase::~VSocketBase()
{
  // Close the socket
  Close();
}

////////////////////////////
VBool
VSocketBase::Close()
{
  while (out_queue)
	{
	  AIOBlock *next = out_queue->next;
	  delete out_queue;
	  out_queue = next;
	}

  return VTrue;
}

////////////////////////////

VBool
VSocketBase::Shutdown()
{
  while (out_queue)
	{
	  AIOBlock *next = out_queue->next;
	  delete out_queue;
	  out_queue = next;
	}

  return VTrue;
}

VBool
VSocketBase::SendQueued(const char *buff, const VCard bufflen)
{
	omni_mutex_lock l(queue_lock);

	// Just append new bytes to the output queue
	if (!out_queue) {
		out_queue = new AIOBlock(bufflen, buff);
		bytes_sent = 0;
	} else {
		AIOBlock *last = out_queue;
		while (last->next)
			last = last->next;
		last->next = new AIOBlock(bufflen, buff);
	}

	return VTrue;
}

////////////////////////////

VBool
VSocketBase::SendFromQueue()
{
	omni_mutex_lock l(queue_lock);

	// Is there something to send?
	if (!out_queue)
		return VTrue;

	// Maximum data size to send at once
	size_t portion_size = out_queue->data_size - bytes_sent;
	if (portion_size > 32768)
		portion_size = 32768;

	// Try to send some data
	int bytes = Send(out_queue->data_ptr + bytes_sent, portion_size);
	if (bytes > 0) {
		bytes_sent += bytes;
	} else if (bytes < 0 && errno != EWOULDBLOCK) {
		vnclog.Print(LL_SOCKERR, VNCLOG("socket error\n"));
		return VFalse;
	}

	// Remove block if all its data has been sent
	if (bytes_sent == out_queue->data_size) {
		AIOBlock *sent = out_queue;
		out_queue = sent->next;
		bytes_sent = 0;
		delete sent;
	}

	return VTrue;
}

