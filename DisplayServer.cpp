/*
*   Copyright (C) 2016,2018,2020 by Jonathan Naylor G4KLX
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

#include "DisplayServer.h"
#include "StopWatch.h"
#include "DisplayNetwork.h"
#include "Version.h"
#include "Log.h"
#include "Thread.h"
#include "GitVersion.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <pwd.h>

const char* DEFAULT_INI_FILE = "/etc/MMDVM.ini";

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <ctime>
#include <cstring>

int main(int argc, char** argv)
{
	const char* iniFile = DEFAULT_INI_FILE;
	if (argc > 1) {
		for (int currentArg = 1; currentArg < argc; ++currentArg) {
			std::string arg = argv[currentArg];
			if ((arg == "-v") || (arg == "--version")) {
				::fprintf(stdout, "DisplayServer version %s git #%.10s\n", VERSION, gitversion);
				return 0;
			} else if (arg.substr(0, 1) == "-") {
				::fprintf(stderr, "Usage: DisplayServer [-v|--version] [filename]\n");
				return 1;
			} else {
				iniFile = argv[currentArg];
			}
		}
	}

	CDisplayServer* reflector = new CDisplayServer(std::string(iniFile));
	reflector->run();
	delete reflector;

	return 0;
}

CDisplayServer::CDisplayServer(const std::string& file) :
m_conf(file),
m_display(NULL),
m_dmrLookup(NULL)
{
	CUDPSocket::startup();
}

CDisplayServer::~CDisplayServer()
{
	CUDPSocket::shutdown();
}

void CDisplayServer::run()
{
	bool ret = m_conf.read();
	if (!ret) {
		::fprintf(stderr, "DisplayServer: cannot read the .ini file\n");
		return;
	}

        ::LogInitialise(m_conf.getLogDisplayLevel());

	CStopWatch stopWatch;
	stopWatch.start();

	CTimer pollTimer(1000U, 5U);
	pollTimer.start();

	LogMessage("Starting DisplayServer-%s git #%.10s", VERSION, gitversion);

	CTimer watchdogTimer(1000U, 0U, 1500U);

	m_display = CDisplay::createDisplay(m_conf);

	std::string lookupFile  = m_conf.getDMRIdLookupFile();
	unsigned int reloadTime = m_conf.getDMRIdLookupTime();

	LogInfo("DMR Id Lookups");
	LogInfo("    File: %s", lookupFile.length() > 0U ? lookupFile.c_str() : "None");
	if (reloadTime > 0U)
		LogInfo("    Reload: %u hours", reloadTime);

	m_dmrLookup = new CDMRLookup(lookupFile, reloadTime);
	m_dmrLookup->read();

	m_display->setIdle();

	CDisplayNetwork network(m_conf.getDisplayServerAddress(), m_conf.getDisplayServerPort(), m_conf.getDisplayServerDebug());

	ret = network.open();
	if (!ret) {
		::LogFinalise();
		return;
	}

	for (;;) {
		char buffer[200U];
		sockaddr_storage addr;
		unsigned int addrLen;

		unsigned int len = network.readData((unsigned char*)buffer, 200U, addr, addrLen);
		if (len > 0U) {
			if (::memcmp(buffer, "setIdle", 6U) == 0) {
				m_display->setIdle();
			}
			else if (::memcmp(buffer, "setError ", 9U) == 0) {
				char* argv[256];
				int cnt = 0;
				int newtoken = 1;
				size_t len = strlen(buffer);
				for (size_t i = 0U; i < len; i++) {
					switch (buffer[i]) {
						case ' ':
							newtoken = 1;
							buffer[i] = 0;
							break;
						default:
							if (newtoken)
								argv[cnt++] = buffer + i;
							newtoken = 0;
							break;
					}
					if (cnt >= 2)
						break;
				}

				const char* text = argv[1];
				if (text) {
					m_display->setError(text);
				}
			}
			else if (::memcmp(buffer, "setQuit", 7U) == 0) {
				m_display->setQuit();
			}
			else if (::memcmp(buffer, "writeDMR ", 9U) == 0) {
				char* argv[256];
				int cnt = 0;
				int newtoken = 1;
				size_t len = strlen(buffer);
				for (size_t i = 0U; i < len; i++) {
					switch (buffer[i]) {
						case ' ':
							newtoken = 1;
							buffer[i] = 0;
							break;
						default:
							if (newtoken)
								argv[cnt++] = buffer + i;
							newtoken = 0;
							break;
					}
				}

				unsigned int slot = atoi(argv[1]);
				std::string src = m_dmrLookup->find(atoi(argv[2]));
				bool group = atoi(argv[3]);
				std::string dst = m_dmrLookup->find(atoi(argv[4]));
				char* type = argv[5];
				if (type) {
					m_display->writeDMR(slot, src, group, dst, type);
					//LogMessage(".... writeDMR %s %s", src.c_str(), dst.c_str());
				}
			}
			else if (::memcmp(buffer, "writeDMRRSSI ", 13U) == 0) {
				char* argv[256];
				int cnt = 0;
				int newtoken = 1;
				size_t len = strlen(buffer);
				for (size_t i = 0U; i < len; i++) {
					switch (buffer[i]) {
						case ' ':
							newtoken = 1;
							buffer[i] = 0;
							break;
						default:
							if (newtoken)
								argv[cnt++] = buffer + i;
							newtoken = 0;
							break;
					}
					if (cnt >= 3)
						break;
				}

				unsigned int slot = atoi(argv[1]);
				char* rssi = argv[2];
				if (rssi) {
					//m_display->writeDMRRSSI(slot, rssi);
					//LogMessage(".... writeDMRRSSI %u %s", slot, rssi);
				}
			}
			else if (::memcmp(buffer, "writeDMRTA ", 11U) == 0) {
				char* argv[256];
				int cnt = 0;
				int newtoken = 1;
				size_t len = strlen(buffer);
				for (size_t i = 0U; i < len; i++) {
					switch (buffer[i]) {
						case ' ':
							newtoken = 1;
							buffer[i] = 0;
							break;
						default:
							if (newtoken)
								argv[cnt++] = buffer + i;
							newtoken = 0;
							break;
					}
					if (cnt >= 4)
						break;
				}

				unsigned int slot = atoi(argv[1]);
				const char* type = argv[2];
				char* ta = argv[3];
				if (ta) {
					m_display->writeDMRTA(slot, (unsigned char*)ta, type);
				}
			}
			else if (::memcmp(buffer, "writeDMRBER ", 12U) == 0) {
				char* argv[256];
				int cnt = 0;
				int newtoken = 1;
				size_t len = strlen(buffer);
				for (size_t i = 0U; i < len; i++) {
					switch (buffer[i]) {
						case ' ':
							newtoken = 1;
							buffer[i] = 0;
							break;
						default:
							if (newtoken)
								argv[cnt++] = buffer + i;
							newtoken = 0;
							break;
					}
					if (cnt >= 3)
						break;
				}

				unsigned int slot = atoi(argv[1]);
				float ber = atof(argv[2]);
				if (ber) {
					m_display->writeDMRBER(slot, ber);
				}
			}
			else if (::memcmp(buffer, "clearDMR ", 9U) == 0) {
				char* argv[256];
				int cnt = 0;
				int newtoken = 1;
				size_t len = strlen(buffer);
				for (size_t i = 0U; i < len; i++) {
					switch (buffer[i]) {
						case ' ':
							newtoken = 1;
							buffer[i] = 0;
							break;
						default:
							if (newtoken)
								argv[cnt++] = buffer + i;
							newtoken = 0;
							break;
					}
					if (cnt >= 2)
						break;
				}

				unsigned int slot = atoi(argv[1]);

				m_display->clearDMR(slot);
			}
			else if (::memcmp(buffer, "writePOCSAG ", 12U) == 0) {
				char* argv[256];
				int cnt = 0;
				int newtoken = 1;
				size_t len = strlen(buffer);
				for (size_t i = 0U; i < len; i++) {
					switch (buffer[i]) {
						case ' ':
							newtoken = 1;
							buffer[i] = 0;
							break;
						default:
							if (newtoken)
								argv[cnt++] = buffer + i;
							newtoken = 0;
							break;
					}
					if (cnt >= 3)
						break;
				}

				uint32_t ric = atoi(argv[1]);
				std::string message = argv[2];
				m_display->writePOCSAG(ric, message.c_str());
			}
			else if (::memcmp(buffer, "clearPOCSAG", 11U) == 0) {
				m_display->clearPOCSAG();
			}
			else if (::memcmp(buffer, "writeCW", 7U) == 0) {
				m_display->writeCW();
			}
			else if (::memcmp(buffer, "clearCW", 7U) == 0) {
				m_display->setIdle();
			}
			else if (::memcmp(buffer, "close", 5U) == 0) {
				// do nothing here for now
				//m_display->close();
			}
		}

		unsigned int ms = stopWatch.elapsed();
		stopWatch.start();

		pollTimer.clock(ms);
		if (pollTimer.hasExpired()) {
			pollTimer.start();
		}

		if (ms < 5U)
			CThread::sleep(5U);
	}

	m_display->close();
	delete m_display;

	if (m_dmrLookup != NULL)
		m_dmrLookup->stop();

	network.close();

	::LogFinalise();
}
