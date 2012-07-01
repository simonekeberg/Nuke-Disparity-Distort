# Compiler Info ('g++-4.0 --version')
# i686-apple-darwin9-g++-4.0.1 (GCC) 4.0.1 (Apple Inc. build 5488)
# Copyright (C) 2005 Free Software Foundation, Inc.
# This is free software; see the source for copying conditions.  There is NO
# warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# End Compiler Info Output
NDKDIR ?= /Applications/Nuke6.3v2/Nuke6.3v2.app/Contents/MacOS
MYCXX ?= g++-4.0
LINK ?= g++-4.0
CXXFLAGS ?= -g -c -DUSE_GLEW -I$(NDKDIR)/include -isysroot /Developer/SDKs/MacOSX10.5.sdk -arch x86_64
LINKFLAGS ?= -L$(NDKDIR) -Wl,-syslibroot,/Developer/SDKs/MacOSX10.5.sdk -arch x86_64
LIBS ?= -lDDImage -lGLEW
LINKFLAGS += -bundle
FRAMEWORKS ?= -framework QuartzCore -framework IOKit -framework CoreFoundation -framework Carbon -framework ApplicationServices -framework OpenGL -framework AGL 

all: IForwardDistort.dylib

.PRECIOUS : %.os
%.os: src/%.cpp
	$(MYCXX) $(CXXFLAGS) -o build/$(@) $<
%.dylib: %.os
	$(LINK) $(LINKFLAGS) $(LIBS) $(FRAMEWORKS) -o build/$(@) build/$<
clean:
	rm -rf build/*.os build/IForwardDistort.dylib
