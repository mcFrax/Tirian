CXX = g++
CXXFLAGS = -DEXCEPTION_TREE
CFLAGS = $(CXXFLAGS) -O3 -Wall -I$(IDIR)
LDFLAGS = -lSDL -lGL -lGLU -lm -lSDL_ttf -lSDL_image -L . -lTirian 
OBJDIR = Obj/
IDIR = Include/
OBJ = $(OBJDIR)texture.o $(OBJDIR)fontrenderer.o $(OBJDIR)widget.o $(OBJDIR)interface.o $(OBJDIR)engine.o $(OBJDIR)widgets.o
DEP = $(IDIR)gl.hpp $(IDIR)logging.hpp $(IDIR)except.hpp 

everything: all Docs

all: libTirian.a

clean:
	rm -f $(OBJDIR)*.o

Docs: *.cpp $(IDIR)/*.hpp *.dox Doxyfile
	doxygen

Tirian: libTirian.a
	
libTirian.a: $(OBJ)
	ar rc libTirian.a $(OBJ)
	ranlib libTirian.a
	
$(OBJDIR)texture.o:  texture.cpp $(IDIR)texture.hpp $(DEP)
	$(CXX) -c texture.cpp -o $(OBJDIR)texture.o $(CFLAGS)
	
$(OBJDIR)fontrenderer.o:  fontrenderer.cpp $(IDIR)fontrenderer.hpp $(IDIR)texture.hpp $(DEP)
	$(CXX) -c fontrenderer.cpp -o $(OBJDIR)fontrenderer.o $(CFLAGS)

$(OBJDIR)widget.o:  widget.cpp $(IDIR)widget.hpp $(IDIR)texture.hpp $(IDIR)fontrenderer.hpp $(DEP)
	$(CXX) -c widget.cpp -o $(OBJDIR)widget.o $(CFLAGS)

$(OBJDIR)interface.o:  interface.cpp $(IDIR)interface.hpp $(DEP)
	$(CXX) -c interface.cpp -o $(OBJDIR)interface.o $(CFLAGS)

$(OBJDIR)widgets.o:  widgets.cpp $(IDIR)widgets.hpp $(IDIR)widget.hpp
	$(CXX) -c widgets.cpp -o $(OBJDIR)widgets.o $(CFLAGS)

$(OBJDIR)engine.o:  engine.cpp $(IDIR)engine.hpp $(IDIR)widget.hpp
	$(CXX) -c engine.cpp -o $(OBJDIR)engine.o $(CFLAGS)
