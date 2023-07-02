// main.cpp : main for test
// Copyright (C) 2011 ~ 2019 drangon <drangon.zhou (at) gmail.com>
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

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include <dgn/Logger.h>

int main( int argc, char **argv ) 
{
	int ret = 0;

	dgn::DgnLib::Init( argc, argv );

	dgn::DgnLib::GetLogger()->InitLevel( DGN_LOG_LEVEL_DEBUG );
	dgn::DgnLib::GetLogger()->InitLogFile( "log_test.log" );
	dgn::DgnLib::GetLogger()->InitSyslog( 0 );
	dgn::DgnLib::GetLogger()->InitStderr( 1 );

	PR_DEBUG( "begin test : %d, %f, [%s]", 30, 40.2, __func__ );

	Catch::Session session; // There must be exactly one instance
	int returnCode = session.applyCommandLine( argc, argv );
	if( returnCode != 0 ) // Indicates a command line error
		return returnCode;
	ret = session.run();

	PR_DEBUG( "end test : ret = %d, %g, [%s]", ret, 40.2, __func__ );

	dgn::DgnLib::Fini();

	return ret;
}
