OS_NAME := $(shell uname -s)

ifeq (Linux,$(OS_NAME))
  CXX     := g++
  EXE_EXT :=
  OBJ_EXT := .o
else # Linux
  CXX     := g++
  EXE_EXT := .exe
  OBJ_EXT := .obj
endif # Linux

all : baddevice$(EXE_EXT)

clean :
	rm -f baddevice$(EXE_EXT) baddevice$(OBJ_EXT)

baddevice$(EXE_EXT) : baddevice$(OBJ_EXT)
	$(CXX) -o $@ baddevice$(OBJ_EXT) -ldl

baddevice$(OBJ_EXT) : baddevice.cpp
	$(CXX) -o $@ -c baddevice.cpp
