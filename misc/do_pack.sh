#!/bin/bash

# Copyright (C) 2011 ~ 2019 drangon zhou <drangon.zhou (at) gmail.com>
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.

# build : linux32 / linux64 / mingw32 / mingw64
export PATH=/opt/cross_linux64/bin:/opt/cross_mingw32/bin:/opt/cross_mingw64/bin:$PATH

# <0> common function

function pack_header
{
	sdir=$1
	hds=$(cd ../$sdir ; ls *.h)
	for hd in $hds ; do
		cp ../$sdir/$hd dgn_lib/dgn/
	done
}

function do_build
{
	tdir=dgn_lib/lib_$1
	make -C ../ clean
	make -C ../
	mkdir -p $tdir
	cp ../*.so ../*.a $tdir
}


# <1> header

rm -rf dgn_lib
mkdir -p dgn_lib/dgn
pack_header dgnbase
#pack_header dgnsup
#pack_header dgnext
#pack_header dgncxx


# <2> linux32 / linux64 / mingw64 / mingw32

unset HOST_TYPE
unset OPT_LIB_DIR
unset CFLAGS
unset GCC
unset GXX
unset GAR

export OPT_LIB_DIR=$HOME/opt_32
do_build linux32
file ../libdgnbase.so

export OPT_LIB_DIR=$HOME/opt_64
export GCC="x86_64-dgn-linux-gcc"
export GXX="x86_64-dgn-linux-g++"
export GAR="x86_64-dgn-linux-ar"
do_build linux64
file ../libdgnbase.so

export HOST_TYPE=mingw32
export OPT_LIB_DIR=$M32D/lib_bin/mingw_dgn_lib
export GCC="i686-w64-mingw32-gcc"
export GXX="i686-w64-mingw32-g++"
export GAR="i686-w64-mingw32-ar"
do_build mingw32
file ../libdgnbase.so


export HOST_TYPE=mingw64
export OPT_LIB_DIR=$M64D/lib_bin/mingw_dgn_lib
export CFLAGS=""
export GCC="x86_64-w64-mingw32-gcc"
export GXX="x86_64-w64-mingw32-g++"
export GAR="x86_64-w64-mingw32-ar"
do_build mingw64
file ../libdgnbase.so


# <3> finish
tar czf dgn_lib.bin.$(date +%y%m%d).tar.gz dgn_lib

make -C ../ clean
echo "finish"

