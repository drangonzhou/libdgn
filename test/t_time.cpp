// t_time.cpp : test dgn time
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

#include <dgn/CStr.h>
#include <dgn/Time.h>

#include "catch.hpp"

#include <stdio.h>
#include <string.h>

using namespace dgn;

TEST_CASE( "gmt time", "[time]")
{
	Time t;
	int64_t t1, t2;
	t.SetNow();
	t1 = t.ToTUS();
	printf( "time : %02d:%02d:%02d, t1 %d\n", t.m_hour, t.m_minute, t.m_sec, 
			(int)(t1 / 1000000) );
	t.SetGMT();
	t2 = t.ToTUS();
	printf( "gmt time : %02d:%02d:%02d, t2 %d\n", t.m_hour, t.m_minute, t.m_sec,
		 	(int)(t2 / 1000000) );

	CStr str;
	t.ToRfc( &str );
	printf( "rfc time : [%s]\n", str.Str() );

	int ret = 0;
	ret = t.FromRfc( "Tue, 15 Jan 2013 21:47:38 GMT" );
	printf( "<1> : ret %d, [%s]\n", ret, t.ToRfc().Str() );
	ret = t.FromRfc( "Tue, 16 Jan 13 21:47:38 GMT" );
	printf( "<2> : ret %d, [%s]\n", ret, t.ToRfc().Str() );
	ret = t.FromRfc( "Tuesday, 17-Jan-13 21:47:38 GMT" );
	printf( "<3> : ret %d, [%s]\n", ret, t.ToRfc().Str() );
	ret = t.FromRfc( "Tue Jan 18 21:47:38 2013" );
	printf( "<4> : ret %d, [%s]\n", ret, t.ToRfc().Str() );

	CHECK( ret >= 0 );
}

TEST_CASE( "sleep some time", "[time]")
{
	int64_t t1, t2;
	t1 = Time::Now();
	Time::SleepMs( 1 );
	t2 = Time::Now();
	printf( "sleep 1, diff %d\n", (int)(t2 - t1) );

	t1 = Time::Now();
	Time::SleepMs( 10 );
	t2 = Time::Now();
	printf( "sleep 10, diff %d\n", (int)(t2 - t1) );

	t1 = Time::Now();
	Time::SleepMs( 100 );
	t2 = Time::Now();
	printf( "sleep 100, diff %d\n", (int)(t2 - t1) );

	CHECK( t2 >= t1 );
}

