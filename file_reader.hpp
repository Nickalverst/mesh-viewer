#include <stdio.h>
#include <stdlib.h>

struct objContent
{
    int n_vertices;
    float* vertices;
    int n_faceElements;
    int* faceElements;
    float* normals;
    int n_normals;
};

objContent readfile(char* path);
char *read_file_into_string(const char *filename);