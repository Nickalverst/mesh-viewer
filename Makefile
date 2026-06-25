CC = g++

GLLIBS = -lglut -lGLEW -lGL

all: mesh mesh2

mesh: mesh.cpp file_reader.cpp lib/*.cpp
	$(CC) $^ -o $@ $(GLLIBS)

mesh2: mesh2.cpp file_reader.cpp lib/*.cpp
	$(CC) $^ -o $@ $(GLLIBS)

clean:
	rm -f mesh mesh.o mesh2 mesh2.o