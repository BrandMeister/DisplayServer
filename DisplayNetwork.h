/*
 *   Copyright (C) 2009-2014,2016,2020 by Jonathan Naylor G4KLX
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

#pragma once

#include "UDPSocket.h"
#include "Timer.h"

#include <cstdint>
#include <string>

class CDisplayNetwork {
  public:
    CDisplayNetwork(const std::string& address, unsigned int port, bool trace);
    ~CDisplayNetwork();

    bool open();

    unsigned int readData(unsigned char* data, unsigned int length, sockaddr_storage& addr, unsigned int& addrLen);

    void close();

  private:
    CUDPSocket       m_socket;
    std::string      m_addressStr;
    sockaddr_storage m_addr;
    unsigned int     m_addrLen;
    unsigned short   m_port;
    bool             m_trace;
};
