ifneq (,$(findstring MINGW,$(shell uname -s)))
	PLATFORM	:=	win32
	exeext		:= .exe
endif

TOOLS = bmp2bin$(exeext) raw2c$(exeext)

all: $(TOOLS)

clean:
	rm *.exe
	
bmp2bin$(exeext): bmp2bin.cpp	
	g++ $< -o $@ -static -O2 -s

raw2c$(exeext): raw2c.c	
	gcc $< -o $@ -static -O2 -s

install:
	cp  --target-directory=$(PREFIX) $(TOOLS)
