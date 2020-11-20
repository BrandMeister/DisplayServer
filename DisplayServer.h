/*
 *   Copyright (C) 2016,2020 by Jonathan Naylor G4KLX
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

#include "Conf.h"
#include "DMRLookup.h"
#include "Display.h"
#include "Timer.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdio>
#include <string>
#include <vector>

class CDisplayServer {
  public:
    CDisplayServer(const std::string& file);
    ~CDisplayServer();

    void run();

  private:
    CConf           m_conf;
    CDisplay*       m_display;
    CDMRLookup*     m_dmrLookup;
    bool            m_debug;
    bool            m_trace;
};
