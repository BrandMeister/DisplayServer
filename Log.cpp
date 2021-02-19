/*
 *   Copyright (C) 2015,2016,2020 by Jonathan Naylor G4KLX
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

#include "Log.h"

#include <sys/time.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <ctime>
#include <cassert>
#include <cstring>
#include <syslog.h>

bool m_syslog = false;
static unsigned int m_displayLevel = 2U;

static char LEVELS[] = " DMIWEF";

void LogInitialise(unsigned int displayLevel, bool syslog)
{
	m_displayLevel = displayLevel;
	m_syslog = syslog;
}

void LogFinalise()
{
}

void Log(unsigned int level, const char* fmt, ...)
{
	assert(fmt != NULL);

	char buffer[501U];
	struct timeval now;
	::gettimeofday(&now, NULL);

	struct tm* tm = ::gmtime(&now.tv_sec);

	::sprintf(buffer, "%c: %04d-%02d-%02d %02d:%02d:%02d ", LEVELS[level], tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	va_list vl;
	va_start(vl, fmt);

	::vsnprintf(buffer + ::strlen(buffer), 500, fmt, vl);

	va_end(vl);

	if (level >= m_displayLevel && m_displayLevel != 0U) {
		if (m_syslog)
			syslog(LOG_INFO, "DisplayServer: %s\n", buffer);
		::fprintf(stdout, "%s\n", buffer);
		::fflush(stdout);
	}

	if (level == 6U) {		// Fatal
		exit(1);
	}
}
