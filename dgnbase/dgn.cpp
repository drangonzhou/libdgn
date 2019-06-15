// dgn.cpp : dgn common func
// Copyright (C) 2011 ~ 2019 drangon zhou <drangon.zhou@gmail.com>
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

#include <dgn/dgn.h>
#include <dgn/Logger.h>

#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#endif

BEGIN_NS_DGN
////////////////

static volatile int s_init_num = 0;
static Mutex s_init_mtx;
static Logger * s_logger = NULL;

void DgnLib::Init( int argc, char * argv[] )
{
	// <1> check init guard
	MutexGuard guard( &s_init_mtx );
	if( ++s_init_num != 1 )
		return;

	// <2> network init
#ifdef _WIN32
	WORD version_requested = MAKEWORD( 2, 2 );
	WSADATA wsa_data;
	WSAStartup( version_requested, &wsa_data );
#else
	signal( SIGPIPE, SIG_IGN );
#endif  

	// <3> TODO : handle argc/argv

	s_logger = new Logger();
	s_logger->InitSyslog( 1 );

	return;
}

void DgnLib::Fini()
{
	MutexGuard guard( &s_init_mtx );
	if( --s_init_num != 0 )
		return;

#ifdef _WIN32
	WSACleanup();
#endif

	delete s_logger, s_logger = NULL;

	return;
}

Logger * DgnLib::GetLogger()
{
	return s_logger;
}

const char * DgnLib::GetName()
{
	return DGN_LIB_NAME;
}

const char * DgnLib::GetDesc()
{
	return DGN_LIB_DESC;
}

int DgnLib::GetVersion( int * major, int * minor, int * patch )
{
	if( major != NULL )
		*major = DGN_LIB_VER_MAJOR;
	if( minor != NULL )
		*minor = DGN_LIB_VER_MINOR;
	if( patch != NULL )
		*patch = DGN_LIB_VER_PATCH;
	return DGN_LIB_VER_FULL;
}

////////////////
END_NS_DGN

