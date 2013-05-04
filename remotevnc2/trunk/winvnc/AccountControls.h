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

// AdministrationControls.h: interface for the AdministrationControls class.

#ifndef _WINVNC_ACCOUNTCONTROLS_H
#define _WINVNC_ACCOUNTCONTROLS_H
#pragma once

#include "resource.h"
#include "vncServer.h"

class AccountControls  
{
public:
	AccountControls(HWND hwnd, vncServer * server);
	void Validate();
	void Apply();
	void Init();
	virtual ~AccountControls();
private:
	std::string m_original_username;
	std::string  m_original_password;

	vncServer * m_server;
	HWND m_hwnd;
};

#endif
