// t_inidoc.cpp : test dgn ini document
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

#include <dgn/IniDoc.h>

#include "catch.hpp"

using namespace dgn;

const char * s_ini_s1 = R"ini(# abc
 ; edf
a = b

[]
a = c

# [ sss ] sss
#   	 = aa

 [ sss ddd ]   	 
 dsss fff = fsss dddef 	  

 [	 ]  	  
a2= d
a3 =e
a4 = fdfd
a5 =
a6 =   	  

[ad]
bb=dc
	 	 dd 	 = ss		 )ini";

TEST_CASE( "inidoc load/save", "[ini]")
{
	IniDoc doc;
	int ret;
	ret = doc.LoadBuf( s_ini_s1 );
	CHECK( ret >= 0 );
	ret = doc.SaveFile( "out_ini.ini" );
	CHECK( ret >= 0 );
	doc.Reset();
	doc.AddSection( "abc" );
	doc.Set( "a2", "b2", "123" );
	doc.Set( "a2", "b3", "123" );
	CHECK( doc.HasKey( "a2", "b2" ) );
	doc.Del( "a2", "b3" );
	CHECK( ! doc.HasKey( "a2", "b3" ) );
	doc.DelSection( "a2" );
	CHECK( ! doc.HasKey( "a2", "b2" ) );
	CHECK( doc.Size() == 1 );

	IniSection & sect = doc.GetSection( "abc" );
	sect.Set( "a4", "123" );
	CHECK( sect.Size() == 1 );
	CHECK( sect.HasKey( "a4" ) );
	CHECK( ! sect.HasKey( "a5" ) );
}
