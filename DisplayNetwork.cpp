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

#include "DisplayNetwork.h"
#include "Utils.h"
#include "Log.h"

#include <cstdio>
#include <cassert>
#include <cstring>

CDisplayNetwork::CDisplayNetwork(const std::string& address, unsigned int port, bool trace) :
m_addressStr(address),
m_addr(),
m_addrLen(0U),
m_port(port),
m_trace(trace)
{
}

CDisplayNetwork::~CDisplayNetwork()
{
}

bool CDisplayNetwork::open()
{
	LogInfo("Opening Display network connection");

	CUDPSocket::lookup(m_addressStr, m_port, m_addr, m_addrLen);

	return m_socket.open(0, m_addr.ss_family, m_addressStr, m_port);
}

unsigned int CDisplayNetwork::readData(unsigned char* data, unsigned int length, sockaddr_storage& addr, unsigned int& addrLen)
{
	assert(data != NULL);
	assert(length > 0U);

	int len = m_socket.read(data, length, addr, addrLen);
	if (len <= 0)
		return 0U;

	if (m_trace)
		CUtils::dump(1U, "Network Received", data, len);

	return len;
}

void CDisplayNetwork::close()
{
	m_socket.close();

	LogInfo("Closing Display network connection");
}
