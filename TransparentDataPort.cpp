/*
 *   Copyright (C) 2016 by Jonathan Naylor G4KLX
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "Conf.h"
#include "TransparentDataPort.h"

#include <string.h>

#include <cstdio>

CTransparentDataPort::CTransparentDataPort(bool enabled, const std::string& remoteaddress, unsigned int remoteport,
                                           const std::string& localaddress, unsigned int localport, unsigned int frametype) :
    m_socket(),
    m_enabled(enabled),
    m_remoteaddress(remoteaddress),
    m_remoteport(remoteport),
    m_localaddress(localaddress),
    m_localport(localport),
    m_addr(),
    m_addrLen(0U),
    m_frametype(frametype)
{
}

CTransparentDataPort::~CTransparentDataPort()
{
}

bool CTransparentDataPort::open()
{
    if (!m_enabled) {
        LogInfo("Display, Transparent data not enabled");
        return false;
    }

    if (m_frametype != 1) {
        LogInfo("Display, SendFrameType should be 1");
        return false;
    }

    if (CUDPSocket::lookup(m_localaddress, m_localport, m_addr, m_addrLen) != 0) {
        LogError("Display, Unable to resolve the address of the Transparent data source");
        return false;
    }

    LogInfo("Display, opening Transparent data socket");

    m_socket = new CUDPSocket(m_remoteport);
    return m_socket->open(0, m_addr.ss_family, m_remoteaddress, m_remoteport);
}

int CTransparentDataPort::write(const unsigned char* data, unsigned int length)
{
    if (!m_enabled) {
        return 0;
    }

    // thanks on7lds
    char content[length + 4U];
    content[0] = 0x80;

    strcpy(&content[1], (char*)data);
    strcat(content, "\xff\xff\xff");

    return m_socket->write((unsigned char*)content, length + 4, m_addr, m_addrLen);
}

int CTransparentDataPort::read(unsigned char* data, unsigned int length)
{
    return true;
}

void CTransparentDataPort::close()
{
    m_socket->close();
}
