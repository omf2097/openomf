# A simple Makefile for omf2097-tools

CC=gcc
RM=rm -f
MKDIR=mkdir -p
RMDIR=rmdir
MV=mv
AR=ar

BKTOOL_MAIN = src/bktool/main.c
AFTOOL_MAIN = src/aftool/main.c
SOUNDTOOL_MAIN = src/soundtool/main.c

SDINC=../libShadowDive/include/
SDLIB=../libShadowDive/lib/
SDLINC=/libs/include/
SDLLIB=/libs/lib/

BINDIR=bin
CFLAGS=-I$(SDINC) -I$(SDLINC) `sdl2-config --cflags` -Wall -std=c99 -ggdb -g3
LDFLAGS=-L$(SDLIB) -L$(SDLLIB) `sdl2-config --libs` -lshadowdive -largtable2

all: 
	$(MKDIR) $(BINDIR)/
	$(CC) -o $(BINDIR)/bktool $(CFLAGS) $(LDFLAGS) $(BKTOOL_MAIN) 
	$(CC) -o $(BINDIR)/aftool $(CFLAGS) $(LDFLAGS) $(AFTOOL_MAIN)
	$(CC) -o $(BINDIR)/soundtool $(CFLAGS) $(LDFLAGS) $(SOUNDTOOL_MAIN)
	$(RM) *.o
	@echo "All done!"

clean:
	$(RM) *.o
	$(RM) $(BINDIR)/bktool.exe
	$(RM) $(BINDIR)/bktool
	$(RM) $(BINDIR)/aftool.exe
	$(RM) $(BINDIR)/aftool
	$(RM) $(BINDIR)/soundtool
	$(RM) $(BINDIR)/soundtool.exe
