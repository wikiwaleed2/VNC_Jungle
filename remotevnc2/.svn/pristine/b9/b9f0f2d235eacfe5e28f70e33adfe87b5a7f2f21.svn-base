// Copyright (C) 2004 TightVNC Development Team. All Rights Reserved.
//
//  TightVNC is free software; you can redistribute it and/or modify
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
// TightVNC homepage on the Web: http://www.tightvnc.com/

// AccountControls.cpp: implementation of the AccountControls class.

#include "AccountControls.h"
#include <windowsx.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

AccountControls::AccountControls(HWND hwnd, vncServer * server)
{
	m_server = server;
	m_hwnd = hwnd;
	Init();
}

void AccountControls::Validate()
{
	
}

void AccountControls::Apply()
{
	char buf[256];
	int len;
	
	if (Edit_GetTextLength(GetDlgItem(m_hwnd, IDC_XMPP_USERNAME)) == 0
		|| Edit_GetTextLength(GetDlgItem(m_hwnd, IDC_XMPP_PASSWORD)) == 0) {
		MessageBox(NULL, 
	   "You must enter a Gmail account to use RemoteVNC.",
	   "RemoteVNC Error", MB_ICONEXCLAMATION | MB_OK);
	}

	len = GetDlgItemText(m_hwnd, IDC_XMPP_USERNAME, (LPSTR)&buf, 256);
	m_server->SetXmppUsername(buf);
	len = GetDlgItemText(m_hwnd, IDC_XMPP_PASSWORD, (LPSTR)&buf, 256);
	m_server->SetXmppPassword(buf);

	/*
	 * Reconnect with the new authentication.
	 */
	if (m_server->GetXmppUsername() != m_original_username
		|| m_server->GetXmppPassword() != m_original_password) {
		m_server->SockConnect(false);
		m_server->SockConnect(true);
	}
}

void AccountControls::Init()
{
	m_original_username = m_server->GetXmppUsername();
	m_original_password = m_server->GetXmppPassword();

	Edit_SetText(GetDlgItem(m_hwnd, IDC_XMPP_USERNAME), 
		m_original_username.c_str());
	Edit_SetText(GetDlgItem(m_hwnd, IDC_XMPP_PASSWORD), 
		m_original_password.c_str());
}

AccountControls::~AccountControls()
{

}
