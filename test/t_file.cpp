// t_file.cpp : test dgn File
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

#include <dgn/File.h>
#include <dgn/CStr.h>

#include "catch.hpp"

#include <stdio.h>
#include <string.h>

using namespace dgn;

TEST_CASE( "dgn file", "[file]")
{
	File fp;
	int ret = 0;

	ret = fp.Open( "abcd.txt", DGN_OPEN_CREATE );
	CHECK( ret == 0 );
	ret = fp.Close();
	CHECK( ret == 0 );
	ret = fp.Open( "abcd.txt", DGN_OPEN_CREATE_EXCL );
	CHECK( ret < 0 );
	ret = File::Unlink( "abcd.txt" );
	CHECK( ret == 0 );
	ret = fp.Open( "abcd.txt", DGN_OPEN_WRITE );
	CHECK( ret < 0 );

	ret = fp.Open( "abcd.txt", DGN_OPEN_CREATE );
	ret = fp.Write( "12345678", 8 );
	CHECK( ret == 8 );
	fp.Truncate( 4 );
	fp.Close();
	char buf[32];
	ret = fp.Open( "abcd.txt", DGN_OPEN_CREATE );
	ret = (int)fp.Size();
	CHECK( ret == 4 );
	ret = fp.Read( buf, 32 );
	CHECK( ret == 4 );
	buf[4] = '\0';
	CHECK( CStr().AttachConst( buf ).Cmp( "1234" ) == 0 );
}
