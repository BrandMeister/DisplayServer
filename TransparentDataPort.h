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

#pragma once

#include "SerialPort.h"
#include "UDPSocket.h"
#include "Log.h"

#include <string>

class CTransparentDataPort : public ISerialPort {
public:
	CTransparentDataPort(bool enabled, const std::string& remoteaddress, unsigned int remoteport, const std::string& localaddress, unsigned int localport, unsigned int frametype);
	virtual ~CTransparentDataPort();

	virtual bool open() override;

	virtual int read(unsigned char* buffer, unsigned int length) override;

	virtual int write(const unsigned char* buffer, unsigned int length) override;

	virtual void close() override;

private:
	CUDPSocket*      m_socket;
	bool             m_enabled;
	std::string      m_remoteaddress;
	unsigned int     m_remoteport;
	std::string      m_localaddress;
	unsigned int     m_localport;
	sockaddr_storage m_addr;
	unsigned int     m_addrLen;
	unsigned int     m_frametype;
};
