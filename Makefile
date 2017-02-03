#-----------------------------------------------------------------------------
# Description: Makefile to build libjetnet.so
# Created:     Oct 2014
# Author:      Shakepeare's ghost
#-----------------------------------------------------------------------------
ifndef ROOTSYS
$(error *** Please set up Root)
endif

name    := jetnet
arch	:= -m64

# Sub-directories
srcdir	:= src
tmpdir	:= tmp
libdir	:= lib
incdir	:= include

$(shell mkdir -p tmp)
$(shell mkdir -p lib)

# Set this equal to the @ symbol to suppress display of instructions
# while make executes
ifdef verbose
AT 	:=
else
AT	:= @
endif

fsrcs	:= $(wildcard $(srcdir)/*.f)
fobjs	:= $(subst $(srcdir)/,$(tmpdir)/,$(fsrcs:.f=.o))
 
ccsrcs	:= $(wildcard $(srcdir)/*.cc) 
ccobjs	:= $(subst $(srcdir)/,$(tmpdir)/,$(ccsrcs:.cc=.o))

# Dictionaries
SRCS	:= 	$(srcdir)/Jetnet.cc 
dictsrcs:= $(subst $(srcdir)/,$(tmpdir)/,$(SRCS:.cc=_dict.cc))
dictobjs:= $(dictsrcs:.cc=.o)

objects	:= $(fobjs) $(ccobjs) $(dictobjs) 

sharedlib := $(libdir)/lib$(name).so

# Display list of applications to be built
#say	:= $(shell echo -e "Apps: $(applications)" >& 2)
#say	:= $(shell echo -e "AppObjs: $(appobjs)" >& 2)
#say	:= $(shell echo -e "Objects: $(objects)" >& 2)
#$(error bye!) 

#-----------------------------------------------------------------------
# 	Define which compilers and linkers to use

# 	C++ Compiler
CXX	:= clang++

CINT    := rootcint

F77	:= gfortran
F77FLAGS:= -c -g -O2 -fPIC $(arch)

# 	Define paths to be searched for C++ header files (#include ....)
CPPFLAGS:= -I. -I$(incdir) -I$(srcdir) $(shell root-config --cflags)

# 	Define compiler flags to be used
#	-c		perform compilation step only 
#	-g		include debug information in the executable file
#	-O2		optimize
#	-ansi	require strict adherance to C++ standard
#	-Wall	warn if source uses any non-standard C++
#	-pipe	communicate via different stages of compilation
#			using pipes rather than temporary files

CXXFLAGS:= -c -g -O2 -ansi -Wall -pipe -fPIC $(arch)

#	C++ Linker
#   set default path to shared library

LD	:= $(CXX) -Wl,-rpath,$(PWD)/$(libdir)

OS	:= $(shell uname -s)
ifeq ($(OS),Darwin)
	LDSHARED	:= $(LD) -dynamiclib
else
	LDSHARED	:= $(LD) -shared
endif

#	Linker flags
LDFLAGS := -g $(arch)

# 	Libraries

LIBS	:=  $(shell root-config --libs) -lgfortran


#	Rules
#	The structure of a rule is
#	target : source
#		command
#	The command makes a target from the source. 
#	$@ refers to the target
#	$< refers to the source

lib:	$(sharedlib)

# Syntax:
# list of targets : target pattern : source pattern

$(sharedlib)	: $(objects)
	@echo "---> Linking `basename $@`"
	$(AT)$(LDSHARED) $(LDFLAGS) -fPIC $(objects) $(LIBS) -o $@
	$(AT)mv $(tmpdir)/*.pcm $(libdir)

$(dictobjs)	: %.o	: %.cc
	@echo "---> Compiling `basename $<`" 
	$(AT)$(CXX) $(CXXFLAGS) $(CPPFLAGS)  $< -o $@

$(dictsrcs)	: $(tmpdir)/%_dict.cc : $(incdir)/%.h
	@echo "---> Building dictionary from `basename $^`" 
	$(AT)$(CINT) -f $@ -c $^

$(ccobjs)	: $(tmpdir)/%.o	: $(srcdir)/%.cc
	@echo "---> Compiling `basename $<`" 
	$(AT)$(CXX) $(CXXFLAGS) $(CPPFLAGS)  $< -o $@

$(fobjs)	: $(tmpdir)/%.o	: 	$(srcdir)/%.f
	@echo "---> Compiling `basename $<`"
	$(AT)$(F77) $(F77FLAGS) $< -o $@

# 	Define clean up rules
clean   :
	rm -rf $(tmpdir)/* $(libdir)/* $(srcdir)/*.so $(srcdir)/*.d
