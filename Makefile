CFLAGS	:=	-O2 -s

ifneq (,$(findstring MINGW,$(shell uname -s)))
	exeext		:= .exe
endif

ifneq (,$(findstring Linux,$(shell uname -s)))
	CFLAGS += -static
endif

TOOLS = bmp2bin$(exeext) raw2c$(exeext)

all: $(TOOLS)

clean:
	rm $(TOOLS)
	
bmp2bin$(exeext): bmp2bin.cpp	
	g++ $< -o $@ $(CFLAGS)

raw2c$(exeext): raw2c.c	
	gcc $< -o $@ $(CFLAGS)

install:
	cp  $(TOOLS) $(PREFIX)
