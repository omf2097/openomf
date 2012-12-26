# A simple Makefile for libShadowDive

LIBNAME=libshadowdive.a
TESTBIN = test.exe

CC=gcc
RM=rm -f
MKDIR=mkdir -p
RMDIR=rmdir
MV=mv
AR=ar

FILES := \
    src/bk.c \
    src/rgba_image.c \
    src/sprite_image.c \
    src/vga_image.c \
    src/animation.c \
    src/internal/reader.c \
    src/internal/writer.c 

TESTMAIN = test/test_main.c
    
LIBDIR=lib
BINDIR=bin
INCDIR=include
OBJDIR=obj

CFLAGS=-I$(INCDIR) -O2 -Wall -s -std=c99

all: 
	$(MKDIR) $(LIBDIR)/
	$(MKDIR) $(OBJDIR)/
	$(MKDIR) $(BINDIR)/
	$(CC) $(CFLAGS) -c $(FILES)
	$(MV) *.o $(OBJDIR)/
	$(AR) rcs $(LIBDIR)/$(LIBNAME) $(OBJDIR)/*.o
	$(CC) -o $(BINDIR)/$(TESTBIN) -I $(INCDIR)/ -l $(LIBDIR)/ -c $(TESTMAIN)
	@echo "All done!"

clean:
	$(RM) $(OBJDIR)/*
	$(RM) $(LIBDIR)/*
	$(RM) $(BINDIR)/*
	$(RMDIR) $(OBJDIR)
	$(RMDIR) $(LIBDIR)
	$(RMDIR) $(BINDIR)