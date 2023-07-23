// t_atomic.cpp : test dgn atomic
// Copyright (C) 2011 ~ 2023 drangon <drangon.zhou (at) gmail.com>
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

#include <dgn/Atomic.h>
#include <dgn/Thread.h>

#include "catch.hpp"

#include <stdio.h>
#include <string.h>

using namespace dgn;

struct sum_data
{
	int m_v1;
	Atomic m_v2;
};

int thread_do_sum( Thread * th, void * arg )
{
	struct sum_data * sd = (struct sum_data *)arg;
	int i;
	for( i = 0; i < 10000; ++i )
	{
		++sd->m_v1;
		sd->m_v2.Inc();
	}
	return 0;
}

TEST_CASE( "atomic test", "[atomic]")
{
	Atomic a2{ 20 };
	int ret = 0;
	ret = a2.Set( 30 );
	CHECK( ( ret == 30 && a2.Get() == 30 ) );
	ret = a2.Inc();
	CHECK( ( ret == 31 && a2.Get() == 31 ) );
	ret = a2.Add( 20 );
	CHECK( ( ret == 51 && a2.Get() == 51 ) );
	ret = a2.Sub( 16 );
	CHECK( ( ret == 35 && a2.Get() == 35 ) );
	ret = a2.Dec();
	CHECK( ( ret == 34 && a2.Get() == 34 ) );

	struct sum_data sd { 0, 0 };
	ThreadObj tho[3];
	int i = 0;
	for( i = 0; i < 3; ++i )
	{
		tho[i].SetFunc( thread_do_sum, &sd );
		tho[i].Start();
	}
	for( i = 0; i < 3; ++i )
	{
		tho[i].WaitStop();
	}
	printf( "atomic test : %d, %d\n", sd.m_v1, sd.m_v2.Get() );
	CHECK( sd.m_v2.Get() == 30000 );
}
