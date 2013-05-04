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


// JSocket.cpp

////////////////////////////////////////////////////////
// System includes

#include "stdhdrs.h"

////////////////////////////////////////////////////////
// *** Lovely hacks to make Win32 work.  Hurrah!

#ifdef __WIN32__
#define EWOULDBLOCK WSAEWOULDBLOCK
#endif

////////////////////////////////////////////////////////
// Socket implementation

#include "JSocket.h"

JSocket::JSocket(VncJingleSocket *sock) : VSocketBase(), m_sock(sock)
{
}

////////////////////////////

JSocket::~JSocket()
{
}

////////////////////////////

VBool
JSocket::Close()
{
	if (m_sock) {
		m_sock->Close();
		m_sock = NULL;
	}

	return VSocketBase::Close();
}

////////////////////////////

VBool
JSocket::Shutdown()
{
	if (m_sock) {
		delete m_sock;
		m_sock = NULL;
	}

	return VSocketBase::Shutdown();
}

////////////////////////////

VString
JSocket::GetPeerName()
{
	return "<unavailable1>";
}

////////////////////////////

VString
JSocket::GetSockName()
{
	return "<unavailable2>";
}

////////////////////////////

VBool
JSocket::SetTimeout(VCard32 secs)
{
	return VTrue;
}

////////////////////////////

VInt JSocket::Send(const char *buff, const VCard bufflen)
{
	errno = 0;

	int result = m_sock->Send(buff, bufflen, MSG_NBLOCK);
	if (result < 0) {
		return -1;
	}
	if (result != bufflen) {
		errno = EWOULDBLOCK;
		return -1;
	}

	return result;
}

////////////////////////////

VBool
JSocket::SendExact(const char *buff, const VCard bufflen)
{
	// Put the data into the queue
	SendQueued(buff, bufflen);

	while (out_queue) {
		if (!SendFromQueue()) {
			return VFalse;
		}
  }

	return VTrue;
}

////////////////////////////

VInt
JSocket::Read(char *buff, const VCard bufflen)
{
	errno = 0;

	int result = m_sock->Recv(buff, bufflen, MSG_NBLOCK);
	if (result < 0) {
		return -1;
	}
	if (result != bufflen) {
		errno = EWOULDBLOCK;
		return -1;
	}

	return result;
}

////////////////////////////

VBool
JSocket::ReadExact(char *buff, const VCard bufflen)
{
	VCard currlen = bufflen;

	while (currlen > 0) {
		if (!m_sock || m_sock->ConnectionClosed()) {
			return VFalse;
		}

		int result = m_sock->Recv(buff, currlen, MSG_NBLOCK);
		if (result < 0 && errno != EWOULDBLOCK) {
			return VFalse;
		} else if (result > 0) {
			currlen -= result;
			buff += result;
		} else {
			if (!SendFromQueue()) {
				return VFalse;
			}
			::Sleep(10); // Sleep a bit if we would have blocked.
		}
	}

	return bufflen;
}

