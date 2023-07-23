// t_cstr.cpp : test dgn CStr
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

#include <dgn/CStr.h>

#include "catch.hpp"

#include <stdio.h>
#include <string.h>

using namespace dgn;

TEST_CASE( "CStr move", "[cstr]")
{
	CStr str1, str2;
	str1 = "500";
	str2 = "600";
	void * p1 = (void *)str1.Str();
	str2 = std::move( str1 );

	CHECK( str2.Str() == p1 );
}

TEST_CASE( "CStr attach buf", "[cstr]" )
{
	char buf[8] = "abc";
	CStr a1;
	a1.AttachBuffer( buf, 8 );
	a1 += "def";
	CHECK( a1.Len() == 6 );
	a1 += ( "123" );
	CHECK( a1.Len() == 7 );
	a1.Reserve( 32 );
	CHECK( a1.Cap() == 8 );
}

TEST_CASE( "CStr attach const", "[cstr]" )
{
	CStr a1;
	a1.AttachConst( "abc" );
	a1 += "def";
	CHECK( a1.Len() == 3 );
	CHECK( a1.Cap() == 4 );
	a1.Reserve( 32 );
	CHECK( a1.Cap() == 4 );
}

TEST_CASE( "CStr basic", "[cstr]" )
{
	CStr a1;
	a1.Reserve( 32 );
	a1 = "abc";
	a1 += "def";
	CHECK( a1.Len() == 6 );
	a1.AppendFmt( "%d, %s, %f", a1.Len(), __DATE__, __LINE__ / 100.0 );
	printf( "a1 AppendFmt is : %s, cap %d\n", a1.Str(), a1.Cap() );
	a1 += "12345678901234567890";
	CHECK( a1.Cap() > 32 );
	a1 = "123";
	int b1 = a1.ToInt();
	CHECK( b1 == 123 );
	a1 = "abc123EFG";
	a1.ToUpper();
	CHECK( a1 == "ABC123EFG" );
}
