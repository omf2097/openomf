# A simple Makefile for libShadowDive

LIBNAME=libshadowdive.a

CC=gcc
RM=rm -f
MKDIR=mkdir -p
RMDIR=rmdir
MV=mv
AR=ar

FILES := \
    src/af.c \
    src/bk.c \
    src/rgba_image.c \
    src/sprite_image.c \
    src/vga_image.c \
    src/animation.c \
    src/move.c \
    src/bkanim.c \
    src/sounds.c \
    src/palette.c \
    src/sprite.c \
    src/error.c \
    src/internal/reader.c \
    src/internal/writer.c \
    src/internal/helpers.c

TESTMAIN = test/test_main.c
SOUNDSMAIN = test/sounds_main.c
ROUNDTRIP = test/roundtrip.c
    
LIBDIR=lib
BINDIR=bin
INCDIR=include
OBJDIR=obj

CFLAGS_DBG=-I$(INCDIR) -Wall -std=c99 -ggdb -g3
CFLAGS_REL=-I$(INCDIR) -Wall -std=c99 -s -O2

all: 
	$(MKDIR) $(LIBDIR)/
	$(MKDIR) $(OBJDIR)/
	$(MKDIR) $(BINDIR)/
	$(CC) $(CFLAGS_DBG) -c $(FILES)
	$(MV) *.o $(OBJDIR)/
	$(AR) rcs $(LIBDIR)/$(LIBNAME) $(OBJDIR)/*.o
	$(CC) $(CFLAGS_DBG) -o $(BINDIR)/test $(TESTMAIN) -I $(INCDIR)/ -lshadowdive -L $(LIBDIR)/
	$(CC) $(CFLAGS_DBG) -o $(BINDIR)/roundtrip $(ROUNDTRIP) -I $(INCDIR)/ -lshadowdive -L $(LIBDIR)/
	$(CC) $(CFLAGS_DBG) -o $(BINDIR)/sounds $(SOUNDSMAIN) -I $(INCDIR)/ -lshadowdive -L $(LIBDIR)/
	@echo "All done!"

release: 
	$(MKDIR) $(LIBDIR)/
	$(MKDIR) $(OBJDIR)/
	$(MKDIR) $(BINDIR)/
	$(CC) $(CFLAGS_REL) -c $(FILES)
	$(MV) *.o $(OBJDIR)/
	$(AR) rcs $(LIBDIR)/$(LIBNAME) $(OBJDIR)/*.o
	$(CC) $(CFLAGS_REL) -o $(BINDIR)/test $(TESTMAIN) -I $(INCDIR)/ -lshadowdive -L $(LIBDIR)/
	$(CC) $(CFLAGS_REL) -o $(BINDIR)/roundtrip $(ROUNDTRIP) -I $(INCDIR)/ -lshadowdive -L $(LIBDIR)/
	$(CC) $(CFLAGS_REL) -o $(BINDIR)/sounds $(SOUNDSMAIN) -I $(INCDIR)/ -lshadowdive -L $(LIBDIR)/
	@echo "All done!"
    
clean:
	$(RM) $(OBJDIR)/*
	$(RM) $(LIBDIR)/*
	$(RM) $(BINDIR)/test.exe
	$(RM) $(BINDIR)/test
	$(RM) $(BINDIR)/roundtrip.exe
	$(RM) $(BINDIR)/roundtrip
	$(RM) $(BINDIR)/sounds
	$(RM) $(BINDIR)/sounds.exe
	$(RMDIR) $(OBJDIR)
	$(RMDIR) $(LIBDIR)
