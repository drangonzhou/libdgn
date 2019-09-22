// Logger.cpp : drangon logger
// Copyright (C) 2012 ~ 2019 drangon zhou <drangon.zhou (at) gmail.com>
//
// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.

#include <dgn/Logger.h>
#include <dgn/Time.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h> // abort()

#ifdef _WIN32
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#define open _open
#define close _close
#define write _write
#define snprintf _snprintf
#else
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#endif

BEGIN_NS_DGN
////////////////

Logger::Logger() 
	: m_level( DGN_LOG_LEVEL_INFO ), m_syslog_enable(0), m_stderr_enable(0)
	, m_fd( -1 ), m_year(0), m_month(0), m_day(0)
{
	m_fname[0] = '\0';
}

Logger::~Logger()
{
	m_fname[0] = '\0';
	if( m_fd >= 0 ) {
		close( m_fd ), m_fd = -1;
	}
}

int Logger::InitLevel( int level )
{
	if( level < DGN_LOG_LEVEL_DEBUG || level > DGN_LOG_LEVEL_ERR )
		return -1;
	m_level = level;
	return 0;
}

int Logger::InitLogFile( const char * logfile )
{
	if( m_fd >= 0 ) {
		close( m_fd ), m_fd = -1;
	}
	if( logfile == NULL || logfile[0] == '\0' ) {
		m_fname[0] = '\0';
		return 0;
	}

	strncpy( m_fname, logfile, sizeof(m_fname) - 1 );
	m_fname[sizeof(m_fname) - 1] = '\0';
	m_year = m_month = m_day = 0;
	return 0;
}

int Logger::InitSyslog( int enable )
{
	m_syslog_enable = enable;
	return 0;
}

int Logger::InitStderr( int enable )
{
	m_stderr_enable = enable;
	return 0;
}

void Logger::Log( int level, const char * file, int line, const char * fmt, ... )
{
	if( level < m_level )
		return;
	if( level > 7 )
		level = 7;
	if( m_fname[0] == '\0' && m_syslog_enable == 0 && m_stderr_enable == 0 )
		return;

	// TODO : get time improve
	Time tm;
	tm.SetNow();

	const char * sf = file;
	const char * p;
	for( p = file; *p != '\0'; p++ ) {
		if( *p == '/' || *p == '\\' )
			sf = p + 1;
	}

	static const char * s_level_str[8] = { "0", "1", "DBG", "3", "INFO", "5", "ERR", "7" };
	va_list ap;
	int len;
	int ret;
	char buf[2048];
	len = snprintf( buf, sizeof(buf) - 1, "%02d%02d %02d:%02d:%02d.%06d|%s:%d|%s ",
			tm.m_month, tm.m_day, tm.m_hour, tm.m_minute, tm.m_sec, tm.m_usec, sf, line, s_level_str[level] );
	va_start( ap, fmt );
	ret = vsnprintf( buf + len, sizeof(buf) - len - 1, fmt, ap );
	va_end( ap );
	buf[sizeof(buf) - 1] = '\0';
	if( ret >= 0 && ret <= (int)sizeof(buf) - len - 1 )
		len += ret;
	else
		len = strlen( buf );

	if( m_fname[0] != '\0' ) {
		if( check_log_file( tm.m_year, tm.m_month, tm.m_day ) >= 0 )
			write( m_fd, buf, len );
	}
	
	if( m_stderr_enable != 0 ) {
		fprintf( stderr, "%s", buf );
	}
	
	if( m_syslog_enable != 0 ) {
#ifdef _WIN32
		OutputDebugStringA( buf );
#else
		static const int s_syslog_level[8] = { LOG_DEBUG, LOG_DEBUG, LOG_DEBUG, LOG_INFO, LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_CRIT };
		syslog( LOG_LOCAL3 | s_syslog_level[level], "%s", buf + 14 );
#endif
	}

	return;
}

void Logger::DoAssert( const char * file, int line, const char * msg )
{
	Log( DGN_LOG_LEVEL_ERR, file, line, "Assert [%s] failed", msg );
	abort();
	return; // never come here
}

int Logger::check_log_file( int year, int month, int day )
{
	if( m_fd >= 0 && year == m_year && month == m_month && day == m_day )
		return 0;

	m_lock.Lock();

	if( m_fd >= 0 && year == m_year && month == m_month && day == m_day ) {
		m_lock.UnLock();
		return 0;
	}

	if( m_fd >= 0 ) {
		close( m_fd ), m_fd = -1;
	}

	// TODO : check dir exist
	char fname[512];
	snprintf( fname, sizeof(fname) - 1, "%s.%04d-%02d-%02d.txt", m_fname, year, month, day );
#ifdef _WIN32
	int flag = _O_CREAT | _O_APPEND | _O_WRONLY | _O_BINARY;
#else
	int flag = O_CREAT | O_APPEND | O_WRONLY;
#endif
	int fd = open( fname, flag, 0664 );
	if( fd < 0 ) {
		m_lock.UnLock();
		return -1;
	}

	m_fd = fd;
	m_year = year;
	m_month = month;
	m_day = day;

	m_lock.UnLock();

	return 0;
}

////////////////
END_NS_DGN

