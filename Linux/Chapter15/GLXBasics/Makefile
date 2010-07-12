MAIN = GLXBasics
SRCPATH = ../../../Src/Chapter15/$(MAIN)/
SHAREDPATH = ../../../Src/GLTools/src/
SHAREDINCPATH = ../../../Src/GLTools/include/
LIBDIRS = -L/usr/X11R6/lib -L/usr/X11R6/lib64 -L/usr/local/lib
INCDIRS = -I/usr/include -I/usr/local/include -I/usr/include/GL -I$(SHAREDINCPATH)  -I$(SHAREDINCPATH)GL

CC = g++
CFLAGS = $(COMPILERFLAGS) -g $(INCDIRS)
LIBS = -lX11 -lglut -lGL -lGLU -lm

prog : $(MAIN)

$(MAIN).o : $(SRCPATH)$(MAIN).cpp
glew.o    : $(SHAREDPATH)glew.c

$(MAIN) : $(MAIN).o glew.o
	$(CC) $(CFLAGS) -o GLXBasics $(LIBDIRS) $(SRCPATH)$(MAIN).cpp $(SHAREDPATH)glew.c $(LIBS)

clean:
	rm -f *.o
