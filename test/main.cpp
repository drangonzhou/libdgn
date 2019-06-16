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

#include <gtest/gtest.h>

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

	testing::InitGoogleTest(&argc, argv);
	ret = RUN_ALL_TESTS();

	PR_DEBUG( "end test : ret = %d, %g, [%s]", ret, 40.2, __func__ );

	dgn::DgnLib::Fini();

	return ret;
}

