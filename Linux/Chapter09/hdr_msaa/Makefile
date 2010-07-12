MAIN = Block
LIBDIRS = -L/usr/X11R6/lib -L/usr/X11R6/lib64 -L/usr/local/lib
INCDIRS = -I/usr/include -L/usr/local/include -I/usr/include/GL
SRCPATH =    ../../../Src/Chapter15/$(MAIN)/
SHAREDPATH = ../../../Src/GLTools/src/

CC     = gcc
CFLAGS = $(COMPILERFLAGS) -g $(INCDIRS)
LIBS   = -lGL -lGLU -lm

prog : $(MAIN)

$(MAIN).o : $(SRCPATH)$(MAIN).cpp

GLBatch.o		  : $(SHAREDPATH)GLBatch.cpp
glew.o		      : $(SHAREDPATH)glew.cpp
GLShaderManager.o : $(SHAREDPATH)GLShaderManager.cpp
GLTools.o		  : $(SHAREDPATH)GLTools.cpp
GLTriangleBatch.o : $(SHAREDPATH)GLTriangleBatch.cpp
math3d.o		  : $(SHAREDPATH)math3d.cpp

$(MAIN) : $(MAIN).o GLBatch.o glew.o GLShaderManager.o GLTools.o GLTriangleBatch math3d.o
    $(CC) $(CFLAGS) -o $(MAIN) $(LIBDIRS) $(SRCPATH)$(MAIN).cpp $(SHAREDPATH)GLBatch.cpp $(SHAREDPATH)glew.c $(SHAREDPATH)GLShaderManager.cpp $(SHAREDPATH)GLTools.cpp $(SHAREDPATH)GLTriangle.cpp $(SHAREDPATH)math3d.cpp $(LIBS)

clean:
    rm -f *.o
