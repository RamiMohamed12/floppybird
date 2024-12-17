CXX = g++
CXXFLAGS = -Wall -g
SFML_LIBS = -lsfml-graphics -lsfml-window -lsfml-system

all: flappy

flappy: main.cpp
    $(CXX) $(CXXFLAGS) main.cpp -o flappy $(SFML_LIBS)

clean:
    rm -f flappy
