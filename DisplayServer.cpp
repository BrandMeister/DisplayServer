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

        ::LogInitialise(m_conf.getLogLevel());

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
			switch (buffer[0]) {
			case 0x01U: // idle
				m_display->setIdle();
				break;
			case 0x02U: { // error
				unsigned int count = buffer[1U];

				char text[count];
				text[0] = 0U;
				for (uint8_t i = 0U; i < count; i++)
					text[i] = buffer[2 + i];

				m_display->setError(text);
				//LogMessage(".... setError text %s", text);
				}
				break;
			case 0x03U: // quit
				m_display->setQuit();
				break;
			case 0x04U: { // write dmr
				unsigned int slotNo = buffer[1];

				unsigned int srcId = (buffer[2] << 24) | ((buffer[3] & 0xFF) << 16) | ((buffer[4] & 0xFF) << 8) | (buffer[5] & 0xFF);
				std::string src = m_dmrLookup->find(srcId);

				bool group = buffer[6] != 0;

				unsigned int dstId = (buffer[7] << 24) | ((buffer[8] & 0xFF) << 16) | ((buffer[9] & 0xFF) << 8) | (buffer[10] & 0xFF);
				std::string dst = m_dmrLookup->find(dstId);

				char type[2U];
				type[0] = buffer[11];
				type[1] = 0U;

				m_display->writeDMR(slotNo, src, group, dst, type);
				//LogMessage(".... writeDMR src %s dst %s group %d type %s", src.c_str(), dst.c_str(), group, type);
				}
				break;
			case 0x05U: { // dmr rssi
				unsigned int slotNo = buffer[1U];
				unsigned int rssi = buffer[2U];

				m_display->writeDMRRSSI(slotNo, rssi);
				//LogMessage(".... writeDMRRSSI slotNo %u rssi %u", slotNo, rssi);
				}
				break;
			case 0x06U: { // dmr ta
				unsigned int slotNo = buffer[1U];

				char type[2U];
				type[0] = buffer[2U];
				type[1] = 0U;

				unsigned int count = buffer[3U];

				unsigned char talkerAlias[count];
				talkerAlias[0] = 0U;
				for (uint8_t i = 0U; i < count; i++)
					talkerAlias[i] = buffer[4 + i];

				m_display->writeDMRTA(slotNo, talkerAlias, type);
				//LogMessage(".... writeDMRTA slotNo %u type %s ta %s", slotNo, type, talkerAlias);
				}
				break;
			case 0x07U: { // dmr ber
				unsigned int slotNo = buffer[1U];

				unsigned int count = buffer[2U];

				char _ber[count];
				_ber[0] = 0U;
				for (uint8_t i = 0U; i < count; i++)
					_ber[i] = buffer[3 + i];

				float ber = atof(_ber);
				m_display->writeDMRBER(slotNo, ber);
				//LogMessage(".... writeDMRBER slotNo %u %f", slotNo, ber);
				}
				break;
			case 0x08U: { // clear dmr
				unsigned int slotNo = buffer[1U];
				m_display->clearDMR(slotNo);
				//LogMessage(".... clearDMR slotNo %u", slotNo);
				}
				break;
			case 0x09U: { // write pocsag
				uint32_t ric = (buffer[1] << 24) | ((buffer[2] & 0xFF) << 16) | ((buffer[3] & 0xFF) << 8) | (buffer[4] & 0xFF);

				unsigned int count = buffer[5U];

				char buf[count];
				buf[0] = 0;
				for (uint8_t i = 0U; i < count; i++)
					buf[i] = buffer[6 + i];

				std::string message(buf, count);

				m_display->writePOCSAG(ric, message.c_str());
				//LogMessage(".... writePOCSSAG ric %u message %s", ric, message.c_str());
				}
				break;
			case 0x0AU: // clear pocsag
				m_display->clearPOCSAG();
				break;
			case 0x0BU: // write cw
				m_display->writeCW();
				break;
			case 0x0CU: // clear cw
				m_display->setIdle();
				break;
			case 0x0DU: // close
				// do nothing here for now
				//m_display->close();
				break;
			default:
				break;
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
