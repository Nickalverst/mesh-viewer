CC = g++

GLLIBS = -lglut -lGLEW -lGL

all: mesh

mesh: mesh.cpp file_reader.cpp lib/utils.cpp
	$(CC) $^ -o $@ $(GLLIBS)

clean:
	rm -f mesh