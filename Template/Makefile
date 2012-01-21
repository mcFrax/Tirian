CXX = g++
CXXFLAGS = -O3 -Wall
CFLAGS = -I$(IDIR) -I /home/frax/Programowanie/Tirian/Include
LDFLAGS = -lSDL -lGL -lGLU -lm -lSDL_ttf -lSDL_image -pthread -L /home/frax/Programowanie/Tirian -lTirian 
OBJDIR = Obj/
IDIR = Include/
PROG = Template
OBJ = $(OBJDIR)main.o

all: $(PROG)

clean:
	rm -f $(OBJDIR)*.o

$(OBJDIR): $(OBJDIR)main.o
	
$(PROG): $(OBJDIR)*.o /home/frax/Programowanie/Tirian/libTirian.a
	$(CXX) $(OBJ) -o $(PROG) $(CXXFLAGS) $(LDFLAGS)
	
$(OBJDIR)main.o: main.cpp $(IDIR)main.hpp
	$(CXX) -c main.cpp -o $(OBJDIR)main.o $(CXXFLAGS) $(CFLAGS)
	
