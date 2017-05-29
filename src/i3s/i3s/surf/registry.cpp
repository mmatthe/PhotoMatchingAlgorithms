/*******************************************************************************
 *   I3S: Interactive Individual Identification System                         *
 *                                                                             *
 *   Copyright (C) 2004-2013  Jurgen den Hartog & Renate Reijns                *
 *                                                                             *
 *   This program is free software; you can redistribute it and/or modify      *
 *   it under the terms of the GNU General Public License as published by      *
 *   the Free Software Foundation; either version 2 of the License, or         *
 *   (at your option) any later version.                                       *
 *                                                                             *
 *   This program is distributed in the hope that it will be useful,           *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 *   GNU General Public License for more details.                              *
 *                                                                             *
 *   You should have received a copy of the GNU General Public License         *
 *   along with this program; see the file COPYING GPL v2.txt. If not,         *
 *   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, *
 *   Boston, MA 02111-1307, USA.                                               *
 *                                                                             *
 *******************************************************************************/

#include <windows.h>
#include <iostream>
using namespace std;

int isUserAccountControlOn()
{
    HKEY hKey = 0;
    char buf[255] = {0};
    DWORD dwType = 0;
    DWORD dwBufSize = sizeof(buf);

    if( RegOpenKey(HKEY_LOCAL_MACHINE,L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",&hKey) == ERROR_SUCCESS)
    {
        dwType = REG_SZ;
        if( RegQueryValueEx(hKey,L"EnableLUA",0, &dwType, (BYTE*)buf, &dwBufSize) == ERROR_SUCCESS)
        {
			DWORD *b = (DWORD*) buf;
			RegCloseKey(hKey);
			return (int) *b;
        }
		else {
			RegCloseKey(hKey);
            return 0;
		}
    }
    else
        return 0;
}


int getWindowsVersion(char *name)
{
    HKEY hKey = 0;
    char buf[255] = {0};
    DWORD dwType = 0;
    DWORD dwBufSize = sizeof(buf);

    if( RegOpenKey(HKEY_LOCAL_MACHINE,L"Software\\Microsoft\\Windows NT\\CurrentVersion",&hKey) == ERROR_SUCCESS)
    {
        dwType = REG_SZ;
        if( RegQueryValueEx(hKey,L"ProductName",0, &dwType, (BYTE*)buf, &dwBufSize) == ERROR_SUCCESS)
        {
			strcpy(name, buf);
			RegCloseKey(hKey);
			return 1;
        }
		else {
			RegCloseKey(hKey);
            return 5;
		}
    }
    else
        return 9;
}
