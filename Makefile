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
CFLAGS=-I $(SDINC) -I $(SDLINC) -Wall -std=c99 -ggdb -g3
LDFLAGS=-L $(SDLIB) -L $(SDLLIB) -lSDL2 -lSDL2main -lshadowdive -largtable2

all: 
	$(MKDIR) $(BINDIR)/
	$(CC) -o $(BINDIR)/bktool -c $(BKTOOL_MAIN) $(CFLAGS) $(LDFLAGS)
	$(CC) -o $(BINDIR)/aftool -c $(AFTOOL_MAIN) $(CFLAGS) $(LDFLAGS)
	$(CC) -o $(BINDIR)/soundtool -c $(SOUNDTOOL_MAIN) $(CFLAGS) $(LDFLAGS)
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
