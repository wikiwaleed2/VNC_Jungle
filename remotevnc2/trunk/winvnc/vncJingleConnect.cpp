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


// vncSockConnect.cpp

// Implementation of the jingle connection listener.

#include "stdhdrs.h"
#include "vncJingleConnect.h"
#include "vncServer.h"
#include "WinVNC.h"

vncJingleConnect::vncJingleConnect()
{
	m_connection = NULL;
}

vncJingleConnect::~vncJingleConnect()
{
	delete m_connection;
}

void vncJingleConnect::LoadXmppLogin(std::string& username, std::string& password)
{
	HKEY hRegKey;
		
	if (RegOpenKey(HKEY_LOCAL_MACHINE, WINVNC_REGISTRY_KEY, &hRegKey) != ERROR_SUCCESS ) {
		hRegKey = NULL;
	} else {
		TCHAR buf[256];
		DWORD buffersize = 256;
		DWORD valtype;
		if (RegQueryValueEx( hRegKey, "XmppUsername", NULL, &valtype, 
				(LPBYTE) &buf, &buffersize) == ERROR_SUCCESS) {
			username = buf;
		}
		if (RegQueryValueEx( hRegKey, "XmppPassword", NULL, &valtype, 
				(LPBYTE) &buf, &buffersize) == ERROR_SUCCESS) {
			password = buf;
		}

		RegCloseKey(hRegKey);
	}
}

BOOL vncJingleConnect::Init(vncServer *server, std::string username,
	std::string password)
{
	m_server = server;

	/*
	 * Get the name of this computer.
	 */
	char desktop_name[MAX_COMPUTERNAME_LENGTH+1];
	DWORD desktop_name_len = MAX_COMPUTERNAME_LENGTH + 1;
	if (!GetComputerName(desktop_name, &desktop_name_len)) {
		desktop_name[0] = '\0';
	}

	m_connection = new VncJingleConnection(true, desktop_name);
	m_connection->SignalLoginComplete.connect(this, &vncJingleConnect::OnLoginComplete);
	m_connection->Login(username, password);

	return TRUE;
}

void vncJingleConnect::OnLoginComplete(bool Connected)
{
	if (Connected) {
		m_connection->SignalAcceptConnection.connect(this, &vncJingleConnect::OnAcceptConnection);
	} else {
		/* Login failed or disconnected. */
	}
}

void vncJingleConnect::OnAcceptConnection(VncJingleSocket *socket)
{
	vnclog.Print(LL_CLIENTS, VNCLOG("accepted connection\n"));

	JSocket *new_sock = new JSocket(socket);
	m_server->AddClient(new_sock, FALSE, FALSE);
}
