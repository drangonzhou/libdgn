/* t_json.cpp : test dgn JsonVal
 * Copyright (C) 2011 drangon <drangon.zhou@gmail.com>
 * 2013-05
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <dgn/JsonVal.h>

#include "catch.hpp"

using namespace dgn;

TEST_CASE( "json test", "[json]")
{
	JsonVal jv;
	jv.SetTrue();
	jv.SetFalse();

	jv.GetInt();
	jv.SetInt( 32 );
	jv.GetInt64();
	jv.SetInt64( 0x10FFFFFFFFLL );

	jv.GetDouble();
	jv.SetDouble( 3.4 );

	jv.GetString();
	jv.SetString( "abcd" );

	jv.SetArray( 20 );
	jv[3] = "abcd";

	jv.SetObject();
	jv["ss"] = 34.34;

	CStr str = jv.ToBuf();
	jv = 20;

	int ret = jv.FromBuf( str );
	printf( "test %g\n", jv["ss"].GetDouble() );

	CHECK( ret >= 0 );
}

