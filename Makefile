ifneq (,$(findstring MINGW,$(shell uname -s)))
	PLATFORM	:=	win32
	exeext		:= .exe
endif


all: bmp2bin$(exeext) raw2c$(exeext)

clean:
	rm *.exe
	
bmp2bin$(exeext): bmp2bin.cpp	
	g++ $< -o $@ -static -O2 -s -D__LITTLE_ENDIAN__

raw2c$(exeext): raw2c.c	
	gcc $< -o $@ -static -O2 -s
