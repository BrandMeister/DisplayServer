/*
 *   Copyright (C) 2015-2020 by Jonathan Naylor G4KLX
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

#include "Defines.h"
#include "Conf.h"
#include "Log.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

const int BUFFER_SIZE = 500;

enum SECTION {
  SECTION_NONE,
  SECTION_GENERAL,
  SECTION_INFO,
  SECTION_TRANSPARENT,
  SECTION_DMRID_LOOKUP,
  SECTION_DMR,
  SECTION_DMR_NETWORK,
  SECTION_DISPLAY,
  SECTION_TFTSERIAL,
  SECTION_NEXTION,
  SECTION_OLED,
  SECTION_LCDPROC
};

CConf::CConf(const std::string& file) :
m_file(file),
m_callsign(),
m_duplex(true),
m_displayServerAddress("127.0.0.1"),
m_displayServerPort(62001),
m_displayServerType(),
m_displayServerDebug(false),
m_displayServerTrace(false),
m_rxFrequency(0U),
m_txFrequency(0U),
m_transparentEnabled(false),
m_transparentRemoteAddress("127.0.0.1"),
m_transparentRemotePort(40094U),
m_transparentLocalAddress("127.0.0.1"),
m_transparentLocalPort(40095U),
m_transparentSendFrameType(0U),
m_dmrIdLookupFile(),
m_dmrIdLookupTime(0U),
m_logLevel(),
m_syslog(false),
m_dmrId(0U),
m_dmrNetworkSlot1(true),
m_dmrNetworkSlot2(true),
m_tftSerialPort("/dev/ttyAMA0"),
m_tftSerialBrightness(50U),
m_nextionPort("/dev/ttyAMA0"),
m_nextionBrightness(50U),
m_nextionDisplayClock(false),
m_nextionUTC(false),
m_nextionIdleBrightness(20U),
m_nextionScreenLayout(0U),
m_nextionTempInFahrenheit(false),
m_oledType(3U),
m_oledBrightness(0U),
m_oledInvert(false),
m_oledScroll(false),
m_oledRotate(false),
m_oledLogoScreensaver(true),
m_lcdprocAddress("127.0.0.1"),
m_lcdprocPort(13666U),
m_lcdprocLocalPort(0U),
m_lcdprocDisplayClock(false),
m_lcdprocUTC(false),
m_lcdprocDimOnIdle(false)
{
}

CConf::~CConf()
{
}

bool CConf::read()
{
  FILE* fp = ::fopen(m_file.c_str(), "rt");
  if (fp == NULL) {
    ::fprintf(stderr, "Couldn't open the .ini file - %s\n", m_file.c_str());
    return false;
  }

  SECTION section = SECTION_NONE;

  char buffer[BUFFER_SIZE];
  while (::fgets(buffer, BUFFER_SIZE, fp) != NULL) {
    if (buffer[0U] == '#')
      continue;

    if (buffer[0U] == '[') {
      if (::strncmp(buffer, "[General]", 9U) == 0)
          section = SECTION_GENERAL;
	  else if (::strncmp(buffer, "[Info]", 6U) == 0)
		  section = SECTION_INFO;
	  else if (::strncmp(buffer, "[Transparent Data]", 18U) == 0)
		  section = SECTION_TRANSPARENT;
	  else if (::strncmp(buffer, "[DMR Id Lookup]", 15U) == 0)
		  section = SECTION_DMRID_LOOKUP;
	  else if (::strncmp(buffer, "[DMR]", 5U) == 0)
		  section = SECTION_DMR;
	  else if (::strncmp(buffer, "[DMR Network]", 13U) == 0)
		  section = SECTION_DMR_NETWORK;
	  else if (::strncmp(buffer, "[Display]", 9U) == 0)
		  section = SECTION_DISPLAY;
	  else if (::strncmp(buffer, "[TFT Serial]", 12U) == 0)
		  section = SECTION_TFTSERIAL;
	  else if (::strncmp(buffer, "[Nextion]", 9U) == 0)
		  section = SECTION_NEXTION;
	  else if (::strncmp(buffer, "[OLED]", 6U) == 0)
		  section = SECTION_OLED;
	  else if (::strncmp(buffer, "[LCDproc]", 9U) == 0)
		  section = SECTION_LCDPROC;
	  else
		  section = SECTION_NONE;

	  continue;
  }

  char* key   = ::strtok(buffer, " \t=\r\n");
  if (key == NULL)
    continue;

  char* value = ::strtok(NULL, "\r\n");
  if (value == NULL)
    continue;

  // Remove quotes from the value
  size_t len = ::strlen(value);
  if (len > 1U && *value == '"' && value[len - 1U] == '"') {
	  value[len - 1U] = '\0';
	  value++;
  } else {
	  char *p;

	  // if value is not quoted, remove after # (to make comment)
	  if ((p = strchr(value, '#')) != NULL)
		*p = '\0';

	  // remove trailing tab/space
	  for (p = value + strlen(value) - 1;
	       p >= value && (*p == '\t' || *p == ' '); p--)
		*p = '\0';
  }

  if (section == SECTION_GENERAL) {
	if (::strcmp(key, "Callsign") == 0) {
		// Convert the callsign to upper case
		for (unsigned int i = 0U; value[i] != 0; i++)
			value[i] = ::toupper(value[i]);
		m_callsign = value;
		} else if (::strcmp(key, "Id") == 0)
			m_dmrId = (unsigned int)::atoi(value);
		else if (::strcmp(key, "Duplex") == 0)
			m_duplex = ::atoi(value) == 1;
	} else if (section == SECTION_INFO) {
		if (::strcmp(key, "TXFrequency") == 0)
			m_txFrequency = (unsigned int)::atoi(value);
		else if (::strcmp(key, "RXFrequency") == 0)
			m_rxFrequency = (unsigned int)::atoi(value);
	} else if (section == SECTION_DMRID_LOOKUP) {
		if (::strcmp(key, "File") == 0)
			m_dmrIdLookupFile = value;
		else if (::strcmp(key, "Time") == 0)
			m_dmrIdLookupTime = (unsigned int)::atoi(value);
	} else if (section == SECTION_TRANSPARENT) {
		if (::strcmp(key, "Enable") == 0)
			m_transparentEnabled = ::atoi(value) == 1;
		else if (::strcmp(key, "RemoteAddress") == 0)
			m_transparentRemoteAddress = value;
		else if (::strcmp(key, "RemotePort") == 0)
			m_transparentRemotePort = (unsigned int)::atoi(value);
		else if (::strcmp(key, "LocalAddress") == 0)
			m_transparentLocalAddress = value;
		else if (::strcmp(key, "LocalPort") == 0)
			m_transparentLocalPort = (unsigned int)::atoi(value);
		else if (::strcmp(key, "SendFrameType") == 0)
			m_transparentSendFrameType = (unsigned int)::atoi(value);
	} else if (section == SECTION_DMR) {
		if (::strcmp(key, "Id") == 0)
			m_dmrId = (unsigned int)::atoi(value);
	} else if (section == SECTION_DMR_NETWORK) {
		if (::strcmp(key, "Slot1") == 0)
			m_dmrNetworkSlot1 = ::atoi(value) == 1;
		else if (::strcmp(key, "Slot2") == 0)
			m_dmrNetworkSlot2 = ::atoi(value) == 1;
	} else if (section == SECTION_DISPLAY) {
		if (::strcmp(key, "ListenAddress") == 0)
			m_displayServerAddress = value;
		else if (::strcmp(key, "ListenPort") == 0)
			m_displayServerPort = (unsigned int)::atoi(value);
		else if (::strcmp(key, "DisplayType") == 0)
			m_displayServerType = value;
		else if (::strcmp(key, "LogLevel") == 0)
			m_logLevel = (unsigned int)::atoi(value);
		else if (::strcmp(key, "Syslog") == 0)
			m_syslog = ::atoi(value) == 1;
		else if (::strcmp(key, "Debug") == 0)
			m_displayServerDebug = ::atoi(value) == 1;
		else if (::strcmp(key, "Trace") == 0)
			m_displayServerTrace = ::atoi(value) == 1;
	} else if (section == SECTION_TFTSERIAL) {
		if (::strcmp(key, "Port") == 0)
			m_tftSerialPort = value;
		else if (::strcmp(key, "Brightness") == 0)
			m_tftSerialBrightness = (unsigned int)::atoi(value);
	} else if (section == SECTION_NEXTION) {
		if (::strcmp(key, "Port") == 0)
			m_nextionPort = value;
		else if (::strcmp(key, "Brightness") == 0)
			m_nextionIdleBrightness = m_nextionBrightness = (unsigned int)::atoi(value);
		else if (::strcmp(key, "DisplayClock") == 0)
			m_nextionDisplayClock = ::atoi(value) == 1;
		else if (::strcmp(key, "UTC") == 0)
			m_nextionUTC = ::atoi(value) == 1;
		else if (::strcmp(key, "IdleBrightness") == 0)
			m_nextionIdleBrightness = (unsigned int)::atoi(value);
		else if (::strcmp(key, "ScreenLayout") == 0)
			m_nextionScreenLayout = (unsigned int)::strtoul(value, NULL, 0);
		else if (::strcmp(key, "DisplayTempInFahrenheit") == 0)
			m_nextionTempInFahrenheit = ::atoi(value) == 1;
	} else if (section == SECTION_OLED) {
		if (::strcmp(key, "Type") == 0)
			m_oledType = (unsigned char)::atoi(value);
		else if (::strcmp(key, "Brightness") == 0)
			m_oledBrightness = (unsigned char)::atoi(value);
		else if (::strcmp(key, "Invert") == 0)
			m_oledInvert = ::atoi(value) == 1;
		else if (::strcmp(key, "Scroll") == 0)
			m_oledScroll = ::atoi(value) == 1;
		else if (::strcmp(key, "Rotate") == 0)
			m_oledRotate = ::atoi(value) == 1;
		else if (::strcmp(key, "LogoScreensaver") == 0)
			m_oledLogoScreensaver = ::atoi(value) == 1;
	} else if (section == SECTION_LCDPROC) {
		if (::strcmp(key, "Address") == 0)
			m_lcdprocAddress = value;
		else if (::strcmp(key, "Port") == 0)
			m_lcdprocPort = (unsigned int)::atoi(value);
		else if (::strcmp(key, "LocalPort") == 0)
			m_lcdprocLocalPort = (unsigned int)::atoi(value);
		else if (::strcmp(key, "DisplayClock") == 0)
			m_lcdprocDisplayClock = ::atoi(value) == 1;
		else if (::strcmp(key, "UTC") == 0)
			m_lcdprocUTC = ::atoi(value) == 1;
		else if (::strcmp(key, "DimOnIdle") == 0)
                       m_lcdprocDimOnIdle = ::atoi(value) == 1;
	}
  }

  ::fclose(fp);

  return true;
}

std::string CConf::getCallsign() const
{
	return m_callsign;
}

bool CConf::getDuplex() const
{
	return m_duplex;
}

unsigned int CConf::getRXFrequency() const
{
	return m_rxFrequency;
}

unsigned int CConf::getTXFrequency() const
{
	return m_txFrequency;
}

unsigned int CConf::getLogLevel() const
{
	return m_logLevel;
}

bool CConf::getSyslog() const
{
	return m_syslog;
}

bool CConf::getTransparentEnabled() const
{
	return m_transparentEnabled;
}

std::string CConf::getTransparentRemoteAddress() const
{
	return m_transparentRemoteAddress;
}

unsigned int CConf::getTransparentRemotePort() const
{
	return m_transparentRemotePort;
}

std::string CConf::getTransparentLocalAddress() const
{
	return m_transparentLocalAddress;
}

unsigned int CConf::getTransparentLocalPort() const
{
	return m_transparentLocalPort;
}

unsigned int CConf::getTransparentSendFrameType() const
{
	return m_transparentSendFrameType;
}

std::string CConf::getDMRIdLookupFile() const
{
	return m_dmrIdLookupFile;
}

unsigned int CConf::getDMRIdLookupTime() const
{
	return m_dmrIdLookupTime;
}

unsigned int CConf::getDMRId() const
{
	return m_dmrId;
}

bool CConf::getDMRNetworkSlot1() const
{
	return m_dmrNetworkSlot1;
}

bool CConf::getDMRNetworkSlot2() const
{
	return m_dmrNetworkSlot2;
}

std::string CConf::getTFTSerialPort() const
{
	return m_tftSerialPort;
}

unsigned int CConf::getTFTSerialBrightness() const
{
	return m_tftSerialBrightness;
}

std::string CConf::getNextionPort() const
{
	return m_nextionPort;
}

unsigned int CConf::getNextionBrightness() const
{
	return m_nextionBrightness;
}

bool CConf::getNextionDisplayClock() const
{
	return m_nextionDisplayClock;
}

bool CConf::getNextionUTC() const
{
	return m_nextionUTC;
}

unsigned int CConf::getNextionIdleBrightness() const
{
	return m_nextionIdleBrightness;
}

unsigned int CConf::getNextionScreenLayout() const
{
	return m_nextionScreenLayout;
}

unsigned char CConf::getOLEDType() const
{
	return m_oledType;
}

unsigned char CConf::getOLEDBrightness() const
{
	return m_oledBrightness;
}

bool CConf::getOLEDInvert() const
{
	return m_oledInvert;
}

bool CConf::getOLEDScroll() const
{
	return m_oledScroll;
}

bool CConf::getOLEDRotate() const
{
	return m_oledRotate;
}

bool CConf::getOLEDLogoScreensaver() const
{
	return m_oledLogoScreensaver;
}

std::string CConf::getLCDprocAddress() const
{
	return m_lcdprocAddress;
}

unsigned int CConf::getLCDprocPort() const
{
	return m_lcdprocPort;
}

unsigned int CConf::getLCDprocLocalPort() const
{
	return m_lcdprocLocalPort;
}

bool CConf::getLCDprocDisplayClock() const
{
	return m_lcdprocDisplayClock;
}

bool CConf::getLCDprocUTC() const
{
	return m_lcdprocUTC;
}

bool CConf::getLCDprocDimOnIdle() const
{
	return m_lcdprocDimOnIdle;
}

bool CConf::getNextionTempInFahrenheit() const
{
	return m_nextionTempInFahrenheit;
}

std::string CConf::getDisplayServerAddress() const
{
	return m_displayServerAddress;
}

unsigned int CConf::getDisplayServerPort() const
{
	return m_displayServerPort;
}

std::string CConf::getDisplayServerType() const
{
	return m_displayServerType;
}

bool CConf::getDisplayServerDebug() const
{
	return m_displayServerDebug;
}

bool CConf::getDisplayServerTrace() const
{
	return m_displayServerTrace;
}
