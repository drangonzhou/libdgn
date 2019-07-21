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

#include <gtest/gtest.h>

using namespace dgn;

TEST( inidoc, loadsave)
{
	IniDoc doc;
	int ret;
	ret = doc.Load( "sample_ini.ini" );
	EXPECT_TRUE( ret >= 0 );
	ret = doc.Save( "out_ini.ini" );
	EXPECT_TRUE( ret >= 0 );
}

