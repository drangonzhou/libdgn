// Logger.cpp : drangon logger
// Copyright (C) 2012 ~ 2023 drangon zhou <drangon.zhou (at) gmail.com>
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
#define lseek _lseek
#define tell _tell
#define close _close
#define write _write
#define unlink _unlink
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
	, m_fname_len(0), m_max_size_mb(200), m_max_roll(10), m_fd( -1 ), m_file_size(0)
{
	m_fname[0] = '\0';
}

Logger::~Logger()
{
	m_syslog_enable = 0;
	m_stderr_enable = 0;
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

int Logger::InitLogFile( const char * logfile, int max_size_mb, int max_roll )
{
	if( m_fd >= 0 ) {
		close( m_fd ), m_fd = -1;
	}
	if( logfile == NULL || logfile[0] == '\0' ) {
		m_fname[0] = '\0';
		return 0;
	}

	m_fname_len = (int)strlen( logfile );
	if( m_fname_len > MAX_LOG_FILENAME_LEN )
		m_fname_len = MAX_LOG_FILENAME_LEN;
	memcpy( m_fname, logfile, m_fname_len );
	m_fname[m_fname_len] = '\0';
	m_max_size_mb = max_size_mb <= 1 ? 2 : max_size_mb;
	m_max_roll = max_roll <= 1 ? 2 : max_roll;
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
		len = (int)strlen( buf );

	if( m_fname[0] != '\0' ) {
		if( check_log_file() >= 0 ) {
			write( m_fd, buf, len );
			m_file_size += len;
		}
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

int Logger::check_log_file()
{
	if( m_fd >= 0 && m_file_size < (int64_t)m_max_size_mb * 1024 * 1024 ) {
		return 0;
	}

	MutexGuard guard( &m_lock );

	if( m_fd >= 0 && m_file_size < (int64_t)m_max_size_mb * 1024 * 1024 ) {
		return 0;
	}

#ifdef _WIN32
	const int flag = _O_CREAT | _O_APPEND | _O_WRONLY | _O_BINARY;
#else
	const int flag = O_CREAT | O_APPEND | O_WRONLY;
#endif

	if( m_fd >= 0 ) {
		close( m_fd ), m_fd = -1;
	}
	else {
		m_fd = open( m_fname, flag, 0660 );
		if( m_fd < 0 )
			return -1;
		lseek( m_fd, SEEK_END, 0 );
		m_file_size = (int64_t)tell( m_fd );
		if( m_file_size < (int64_t)m_max_size_mb * 1024 * 1024 ) {
			return 0;
		}
		close( m_fd ), m_fd = -1;
	}

	// TODO : check dir exist
	char buf[MAX_LOG_FILENAME_LEN + 16];
	memcpy( buf, m_fname, m_fname_len + 1 );
	int i = 0;
	for( i = m_max_roll - 1; i >= 1; ++i ) {
		snprintf( buf + m_fname_len, 16 - 1, ".%d", i );
		if( i - 1 == 0 )
			m_fname[m_fname_len] = '\0';
		else
			snprintf( m_fname + m_fname_len, 16 - 1, ".%d", i - 1 );
		unlink( buf );
		rename( m_fname, buf );
	}

	m_fname[m_fname_len] = '\0';
	int fd = open( m_fname, flag, 0660 );
	if( fd < 0 ) {
		return -1;
	}

	m_fd = fd;
	m_file_size = 0;

	return 0;
}

////////////////
END_NS_DGN

