NDKDIR ?= /swiss/tool/nuke-versions/6.3v2/
MYCXX ?= g++
LINK ?= g++
CXXFLAGS ?= -g -c -DUSE_GLEW -I$(NDKDIR)/include -fPIC -msse 
LINKFLAGS ?= -L$(NDKDIR) 
LIBS ?= -lDDImage
LINKFLAGS += -shared


all: IForwardDistort.so

.PRECIOUS : %.os
%.os: src/%.cpp
	$(MYCXX) $(CXXFLAGS) -o build/$(@) $<
%.so: %.os
	$(LINK) $(LINKFLAGS) $(LIBS) -o build/$(@) build/$<
clean:
	rm -rf build/*.os build/IForwardDistort.dylib

