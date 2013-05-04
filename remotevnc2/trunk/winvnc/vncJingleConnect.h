//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
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


// vncJingleConnect.h

// Support connections over XMPP.

class vncJingleConnect;

#if (!defined(_WINVNC_VNCJINGLEONNECT))
#define _WINVNC_VNCJINGLEONNECT

// Includes
#include "stdhdrs.h"
#include "JSocket.h"
#include "vncServer.h"
#include <vncjingle.h>
#include <omnithread.h>

// The vncJingleConnect class itself
class vncJingleConnect : public sigslot::has_slots<> {
public:
	// Constructor/destructor
	vncJingleConnect();
	~vncJingleConnect();

	// Init
	virtual VBool Init(vncServer *server, std::string username,
		std::string password);

	void OnAcceptConnection(VncJingleSocket *socket);
	void OnLoginComplete(bool Connected);
	static void LoadXmppLogin(std::string& username, std::string& password);

	// Implementation
protected:

	// The Jingle connection.
	VncJingleConnection *m_connection;

	// Server that will handle requests.
	vncServer *m_server;
};

#endif // _WINVNC_VNCJINGLEONNECT
