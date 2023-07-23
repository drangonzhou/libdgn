# Makefile for test
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

HOST_TYPE ?= linux
OPT_LIB_DIR ?= /home/drangon/opt

SRCS += $(wildcard *.cpp)

OBJS = $(foreach src, $(SRCS), $(basename $(src)).o)
DEPS = $(foreach src, $(SRCS), $(basename $(src)).zzdep)

PCH = catch.hpp
DEPS += $(PCH).catch.zzdep

TARGET = test.exe

GCC = gcc
GXX = g++
CFLAGS += -g -Wall -pipe -I../ 
CFLAGS += -fno-rtti -fno-exceptions
LDFLAGS += -Wl,-rpath,./ -L../ -ldgnbase 

#CFLAGS += -I$(OPT_LIB_DIR)/include
#LDFLAGS += -L$(OPT_LIB_DIR)/lib -lcrypto

ifeq ($(strip $(OS)),Windows_NT)
# CFLAGS += -DWIN32_LEAN_AND_MEAN=1
LDFLAGS += -lwinmm -lws2_32
else
LDFLAGS += 
endif

.PHONY : all clean

all : $(TARGET)

clean :
	rm -f $(TARGET) *.so *.o *.zzdep 


$(TARGET) : $(OBJS) ./libdgnbase.so 
	$(GXX) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

./libdgnbase.so : ../libdgnbase.so
	cp ../libdgnbase.so .

$(OBJS) : $(PCH).gch

$(PCH).gch : $(PCH)
	rm -f  $(PCH).gch
	$(GXX) $(CFLAGS) -x c++-header $(PCH) -MMD -MT $(PCH).gch -MF $(PCH).gch.zzdep

.SUFFIXES : .cpp

.cpp.o :
	$(GXX) $(CFLAGS) -MMD -MF $*.zzdep -c -o $@ $<


-include $(DEPS)

