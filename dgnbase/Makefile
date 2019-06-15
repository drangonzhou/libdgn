# Makefile for dgnbase
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

# HOST_TYPE : linux/mingw32/mingw64
# OPT_LIB_DIR : optional library

HOST_TYPE ?= linux
OPT_LIB_DIR ?= /home/drangon/opt

DSO_TARGET = libdgnbase.so

SRCS += $(wildcard *.cpp)

OBJS = $(foreach src, $(SRCS), $(basename $(src)).o)
DEPS = $(foreach src, $(SRCS), $(basename $(src)).zzdep)

GCC ?= gcc
GXX ?= g++
GAR ?= ar

CFLAGS += -g -Wall -pipe -I.. 
CFLAGS += -fno-rtti -fno-exceptions 
LDFLAGS +=

mingw32_IMPLIB = $(DSO_TARGET:.so=.dll).a
mingw64_IMPLIB = $(mingw32_IMPLIB)
# linux_IMPLIB = 
IMPLIB = $($(HOST_TYPE)_IMPLIB)

# mingw32_CFLAGS += -flto
# mingw64_CFLAGS += $(mingw32_CFLAGS)
linux_CFLAGS += -DPIC -fPIC
mingw32_LDFLAGS += -Wl,--out-implib,$(IMPLIB)
mingw32_LDFLAGS += -lwinmm -lws2_32
mingw64_LDFLAGS = $(mingw32_LDFLAGS)
linux_LDFLAGS += -lpthread -ldl -lrt

CFLAGS += $($(HOST_TYPE)_CFLAGS)
LDFLAGS += $($(HOST_TYPE)_LDFLAGS)


.PHONY : all clean 

all : $(DSO_TARGET)
	cp $(DSO_TARGET) $(IMPLIB) ../

# $(GXX) $(CFLAGS) -x c++-header libtestfc.h

clean :
	rm -f $(DSO_TARGET) $(IMPLIB) *.o *.zzdep

# $(LIB_TARGET) : $(OBJS)
# 	$(GAR) crv $(LIB_TARGET) $(OBJS)

$(DSO_TARGET) : $(OBJS)
	$(GXX) -shared $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)


.SUFFIXES : .cpp

.cpp.o :
	$(GXX) $(CFLAGS) -MMD -MF $*.zzdep -c -o $@ $<

-include $(DEPS)

