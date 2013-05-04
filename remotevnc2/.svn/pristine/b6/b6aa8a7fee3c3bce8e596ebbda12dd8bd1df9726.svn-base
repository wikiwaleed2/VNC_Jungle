//  Copyright (C) 2003-2006 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
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


// LoginAuthDialog.cpp: implementation of the dialog box for authentication
// with a username/password pair.

#include "stdhdrs.h"
#include "vncUtils.h"

typedef std::basic_string<TCHAR> tstring;  
 
void StringToTCHAR(std::string& s, wchar_t *t, size_t max_len)
{
  tstring tstr;
  const char* all = s.c_str();
  mbstowcs(t, all, max_len);
	t[max_len-1] = '\0';
}
void StringToTCHAR(std::string& s, char *t, size_t max_len)
{
  tstring tstr;
  const char* all = s.c_str();
  strncpy(t, all, max_len);
	t[max_len-1] = '\0';
}
 
std::string TCHARToString(const wchar_t *ptsz)
{
   int len = wcslen((wchar_t*)ptsz);
   char* psz = new char[2*len + 1];
   wcstombs(psz, (wchar_t*)ptsz, 2*len + 1);
   std::string s = psz;
   delete [] psz;
   return s;
}
std::string TCHARToString(const char *ptsz)
{
	return std::string(ptsz);
}
