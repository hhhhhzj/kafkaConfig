
#############################################################################
# Makefile for building: rdp-tool
# Template: app
#############################################################################


####### Compiler, tools and options
TRUNKDIR=$(shell pwd -P|sed -e 's/\(\/rdp-proj\).*/\1/')
RDP_DEPS:= $(TRUNKDIR)/deps

CC            = gcc
CXX           = g++
CFLAGS        = -O2 -Wall -W -fPIC $(DEFINES)
CXXFLAGS      = -pipe -O2 -std=gnu++0x -Wall -W -fPIC $(DEFINES)
INCPATH       = -I. -I $(RDP_DEPS)/jansson/include -I $(RDP_DEPS)/zklib/include/zookeeper
LINK          = g++
LFLAGS        = -Wl,-O1
LIBS          = -L $(RDP_DEPS)/zklib/lib/ -lzookeeper_mt  -L $(RDP_DEPS)/jansson/lib -ljansson

SOURCES       = command_parser.cc \
		main.cc \
		zookeeper_handle.cc 
OBJECTS       = command_parser.o \
		main.o \
		zookeeper_handle.o

.cc.o:
	$(CXX) $(CFLAGS) -c $^ $(INCPATH)
 
all:zookeeper_configuration

zookeeper_configuration:$(OBJECTS)
	$(CXX) $(CFLAGS) -o $@ $^ $(LIBS)

.PHONY:clean

clean:
	rm -f $(OBJECTS)
