CC = g++

GLLIBS = -lglut -lGLEW -lGL

all: mesh2

mesh2: mesh.cpp file_reader.cpp lib/*.cpp
	$(CC) $^ -o $@ $(GLLIBS)

clean:
	rm -f mesh mesh.o