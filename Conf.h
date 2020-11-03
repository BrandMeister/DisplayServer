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

#pragma once

#include <string>
#include <vector>

class CConf
{
public:
  CConf(const std::string& file);
  ~CConf();

  bool read();

  // The General section
  std::string  getCallsign() const;
  bool         getDuplex() const;

  // The Info section
  unsigned int getRXFrequency() const;
  unsigned int getTXFrequency() const;

  // The DMR Id section
  std::string  getDMRIdLookupFile() const;
  unsigned int getDMRIdLookupTime() const;

  // The Network section
  std::string  getDisplayServerAddress() const;
  unsigned int getDisplayServerPort() const;
  std::string  getDisplayServerType() const;
  bool         getDisplayServerDebug() const;

  // The Log section
  unsigned int getLogDisplayLevel() const;

  // The DMR section
  unsigned int getDMRId() const;

  // The DMR Network section
  bool         getDMRNetworkSlot1() const;
  bool         getDMRNetworkSlot2() const;

  // The TFTSERIAL section
  std::string  getTFTSerialPort() const;
  unsigned int getTFTSerialBrightness() const;

  // The Nextion section
  std::string  getNextionPort() const;
  unsigned int getNextionBrightness() const;
  bool         getNextionDisplayClock() const;
  bool         getNextionUTC() const;
  unsigned int getNextionIdleBrightness() const;
  unsigned int getNextionScreenLayout() const;
  bool         getNextionTempInFahrenheit() const;

  // The OLED section
  unsigned char  getOLEDType() const;
  unsigned char  getOLEDBrightness() const;
  bool           getOLEDInvert() const;
  bool           getOLEDScroll() const;
  bool           getOLEDRotate() const;
  bool           getOLEDLogoScreensaver() const;

  // The LCDproc section
  std::string  getLCDprocAddress() const;
  unsigned int getLCDprocPort() const;
  unsigned int getLCDprocLocalPort() const;
  bool         getLCDprocDisplayClock() const;
  bool         getLCDprocUTC() const;
  bool         getLCDprocDimOnIdle() const;

private:
  std::string  m_file;
  std::string  m_callsign;
  bool         m_duplex;
  std::string  m_display;

  std::string  m_displayServerAddress;
  unsigned int m_displayServerPort;
  std::string  m_displayServerType;
  bool         m_displayServerDebug;

  unsigned int m_rxFrequency;
  unsigned int m_txFrequency;

  std::string  m_dmrIdLookupFile;
  unsigned int m_dmrIdLookupTime;

  unsigned int m_logDisplayLevel;

  unsigned int m_dmrId;

  bool         m_dmrNetworkSlot1;
  bool         m_dmrNetworkSlot2;

  std::string  m_tftSerialPort;
  unsigned int m_tftSerialBrightness;

  std::string  m_nextionPort;
  unsigned int m_nextionBrightness;
  bool         m_nextionDisplayClock;
  bool         m_nextionUTC;
  unsigned int m_nextionIdleBrightness;
  unsigned int m_nextionScreenLayout;
  bool         m_nextionTempInFahrenheit;
  
  unsigned char m_oledType;
  unsigned char m_oledBrightness;
  bool          m_oledInvert;
  bool          m_oledScroll;
  bool          m_oledRotate;
  bool          m_oledLogoScreensaver;

  std::string  m_lcdprocAddress;
  unsigned int m_lcdprocPort;
  unsigned int m_lcdprocLocalPort;
  bool         m_lcdprocDisplayClock;
  bool         m_lcdprocUTC;
  bool         m_lcdprocDimOnIdle;
};
