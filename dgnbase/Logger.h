// Logger.h : drangon logger
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

#ifndef INCLUDED_DGN_LOGGER_H
#define INCLUDED_DGN_LOGGER_H

#include <dgn/dgn.h>
#include <dgn/Thread.h>  // Mutex

enum {
	DGN_LOG_LEVEL_DEBUG = 2,
	DGN_LOG_LEVEL_INFO = 4,
	DGN_LOG_LEVEL_ERR = 6,
};

#ifdef _WIN32
#define DGN_LOG_LINE_END "\r\n"
#else
#define DGN_LOG_LINE_END "\n"
#endif

// fmt must be string literals, and should not end with \r\n ( will auto add )
#define PR_DEBUG( fmt, ... ) DGN_PR_LOG( DGN_LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__ )
#define PR_INFO( fmt, ... ) DGN_PR_LOG( DGN_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__ )
#define PR_ERR( fmt, ... ) DGN_PR_LOG( DGN_LOG_LEVEL_ERR, fmt, ##__VA_ARGS__ )
#define DGN_PR_LOG( level, fmt, ... ) dgn::DgnLib::GetLogger()->Log( level, __FILE__, __LINE__, fmt DGN_LOG_LINE_END, ##__VA_ARGS__ )

#define ASSERT(x) \
	do { \
		if( ! (x) ) { \
			dgn::DgnLib::GetLogger()->DoAssert( __FILE__, __LINE__, #x ); \
		} \
	} while( 0 )


BEGIN_NS_DGN
////////////////

#define MAX_LOG_FILENAME_LEN	512

class DGN_LIB_API Logger
{
public:
	Logger();
	~Logger();

	int InitLevel( int level );
	int InitSyslog( int enable );
	int InitStderr( int enable );
	int InitLogFile( const char * logfile, int max_size_mb = 200, int max_roll = 10 );

	void Log( int level, const char * file, int line, const char * fmt, ... ) DGN_ATTR_PRINTF(5,6);
	void DoAssert( const char * file, int line, const char * msg );

protected:
	int check_log_file();

protected:
	int m_level;
	int m_syslog_enable;
	int m_stderr_enable;

	Mutex m_lock;
	char m_fname[MAX_LOG_FILENAME_LEN + 16];
	int m_fname_len;
	int m_max_size_mb;
	int m_max_roll;
	int m_fd;
	int64_t m_file_size;
};

////////////////
END_NS_DGN

#endif // INCLUDED_DGN_LOGGER_H

