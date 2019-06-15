# Makefile for libdgn
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

.PHONY : all clean test test_clean

all : 
	make -C dgnbase
#	make -C dgncxx
#	make -C dgnext

clean :
	make -C dgnbase clean
#	make -C dgncxx clean
#	make -C dgnext clean
	rm -f *.a *.so

test : all
	make -C test
#	make -C test_cxx
#	make -C test_ext

test_clean :
	make -C test clean
#	make -C test_cxx clean
#	make -C test_ext clean

dist : all
	rm -rf libdgn
	mkdir libdgn
	cp *.a *.so libdgn/
	mkdir libdgn/dgn
	cp dgnbase/*.h libdgn/dgn
	#cp dgncxx/*.h libdgn/dgn
	#cp -R dgncxx/stl libdgn/dgn
	#cp dgnext/*.h libdgn/dgn
	tar czf libdgn.bin.$(shell date +%y%m%d).tar.gz libdgn

# rm -rf libdgn

